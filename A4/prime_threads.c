#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dynarr.h"

#define MAX_PRIME 881269
#define THREADS 16

int primes[MAX_PRIME];
int happiness[MAX_PRIME];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;				// mutex protects thread_termination conditional variable
static pthread_cond_t thread_termination = PTHREAD_COND_INITIALIZER;

enum t_state {
	THREAD_LIVE,
	THREAD_DEAD,
	THREAD_JOINED
};

typedef struct {
	pthread_t tid;
	int num;
	int sieve_prime;
	enum t_state state;
} thread_struct;

int is_happy(int num) {
	DynIntArr* arr = malloc(sizeof(DynIntArr));
	arr_init(arr, 10);
	
	while(num != 1) {						// Loop until found happy or sad
		int new_num = 0;
		while (num / 10 != 0) {				// While >1 digit
			new_num += pow(num % 10, 2);	// num % 10 = least sig. digit
			num = num / 10;					// Chop off last digit
		}
		new_num += pow(num, 2);
		num = new_num;
		
		int i;								// Check if already hit this #
		for (i=0; i<arr->size; i++)
		{
			if (num == arr->data[i])
			{
				return 0; // is sad
			}
		}
		append(arr, num);
	}
	return 1; // is happy
	arr_free(arr);
	free(arr);
	
	return 1;
}

int get_next_sieve_prime(int start) {
	int m = start + 1;
	while (m <= floor(sqrt(MAX_PRIME)))
	{
	  	if (primes[m-1] == 1) { return m; }		// m is prime, return as next sieve prime
		m++;
	}
	return -1;
}

static void* mark_multiples(void* arg) {
	int j = 2;
	thread_struct* thread_info = (thread_struct*) arg;
	int m = thread_info->sieve_prime;
	
	while (m*j <= MAX_PRIME)
	{
		primes[m*j-1] = 0;
		j++;
	}
	
	pthread_mutex_lock(&mutex);						// Acquire lock since about to terminate.
	thread_info->state = THREAD_DEAD;
	pthread_cond_signal(&thread_termination);		// This lock is only available when main() is blocking on pthread_cond_wait().
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}

static void* find_happiness(void* arg) {
	thread_struct* thread_info = (thread_struct*) arg;
	
	int i;
	for (i=thread_info->num*MAX_PRIME/THREADS + 1; i<=(thread_info->num*MAX_PRIME/THREADS + MAX_PRIME/THREADS); i++) {
		if (primes[i] && is_happy(i+1)) { happiness[i] = 1; }
	}
	
	pthread_mutex_lock(&mutex);
	thread_info->state = THREAD_DEAD;
	pthread_cond_signal(&thread_termination);
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int main() {
	int m = 1;									// Current sieve prime, initialized to 1
	
	pthread_mutex_lock(&mutex);					// Lock thread_termination, only unlock when blocking on pthread_cond_wait
	
	int i;										// Mark all numbers as prime and unhappy
	for (i=0; i<MAX_PRIME; i++) 
	{
		primes[i] = 1;
		happiness[i] = 0;
	}
	
	primes[0] = 0;
	
	thread_struct* t_info;						// Allocate thread structures
	if ((t_info = malloc(sizeof(thread_struct) * THREADS)) == NULL) 
	{
		perror("malloc: ");
		exit(EXIT_FAILURE);
	}
	
	int c;										// Initialize threads and thread structures
	for (c=0; c<THREADS; c++)
	{
		m = get_next_sieve_prime(m);
		t_info[c].state = THREAD_LIVE;
		t_info[c].num = c;
		t_info[c].sieve_prime = m;
		
		if (pthread_create(&t_info[c].tid, NULL, mark_multiples, &t_info[c]) != 0)
		{
			perror("pthread_create: ");
			exit(EXIT_FAILURE);
		}
	}
	
	while ((m = get_next_sieve_prime(m)) != -1)
	{
		pthread_cond_wait(&thread_termination, &mutex);	// Block here until thread_termination signals (ie thread(s) die(s))
		int a = 0;
		while (t_info[a].state == THREAD_LIVE) { a++; }
		if (a >= THREADS)
		{
			fprintf(stderr, "Error. All threads alive but thread_termination signaled.\n");
			exit(EXIT_FAILURE);
		}
		
		pthread_join(t_info[a].tid, NULL);
		
		t_info[a].state = THREAD_LIVE;
		t_info[a].sieve_prime = m;
		
		if (pthread_create(&t_info[a].tid, NULL, mark_multiples, &t_info[a]) != 0) {
			perror("pthread_create: ");
			exit(EXIT_FAILURE);
		}
	}
	
	int d;
	for (d=0; d<THREADS; d++)						// Join all threads to main(), then relaunch them with new task
	{
		while (t_info[d].state != THREAD_DEAD) { pthread_cond_wait(&thread_termination, &mutex); }
		pthread_join(t_info[d].tid, NULL);
		
		t_info[d].num = d;
		t_info[d].state = THREAD_LIVE;
		
		if (pthread_create(&t_info[d].tid, NULL, find_happiness, &t_info[d]) != 0)
		{
			perror("pthread_create: ");
			exit(EXIT_FAILURE);
		}
	}
	
	int numAlive = THREADS;
	while (numAlive > 0) 
	{
		pthread_cond_wait(&thread_termination, &mutex);			// threads may terminate when main() blocks here
		
		int e;
		for (e=0; e<THREADS; e++)
		{
			if (t_info[e].state == THREAD_DEAD)
			{
				pthread_join(t_info[e].tid, NULL);
				t_info[e].state = THREAD_JOINED;
				numAlive--;
			}
		}
	}
	
	int num_happy = 0;
	int y;
	for (y=0; y<MAX_PRIME; y++)
	{
		if (happiness[y])
		{
			num_happy++;
		}
	}
	
	long HSIZE = num_happy*sizeof(unsigned int);
	unsigned int* out_arr = malloc(HSIZE);
	
	int out_fd;
	if ((out_fd = open("happy_primes_threads", O_RDWR | O_CREAT | O_TRUNC, 0755)) == -1)
	{
		perror("error open: ");
		exit(EXIT_FAILURE);
	}
	
	if (ftruncate(out_fd, HSIZE) == -1)
	{
		perror("ftruncate: ");
		exit(EXIT_FAILURE);
	}
	
	if (lseek(out_fd, 0, SEEK_SET) == -1)
	{
		perror("lseek");
		exit(EXIT_FAILURE);
	}
	
	long p = 0;
	int e;
	for (e=0; e<MAX_PRIME; e++)
	{
		if (happiness[e])
		{
			printf("%ld %d\n", p+1, (unsigned int) e+1);
			out_arr[p] = (unsigned int) e+1;
			p++;
		}
	}
	
	if (write(out_fd, (void*) out_arr, HSIZE) == -1)
	{
		perror("write");
		exit(EXIT_FAILURE);
	}
	
	if (close(out_fd) == -1)
	{
		perror("close: ");
		exit(EXIT_FAILURE);
	}
	
}