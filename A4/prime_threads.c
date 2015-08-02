#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRIME 100000
#define THREADS 16

int primes[MAX_PRIME];
int happiness[MAX_PRIME];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;				// mutex protects thread_termination conditional variable
static pthread_cond_t thread_termination = PTHREAD_COND_INITIALIZER;

enum t_state {
	THREAD_LIVE,
	THREAD_DEAD
};

typedef struct {
	pthread_t tid;
	int num;
	int sieve_prime;
	enum t_state state;
} thread_struct;


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
	
	while (m*j < MAX_PRIME)
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
	printf("I'm a new thread. I'm finding happiness between %d and %d\n", 
		thread_info->num*MAX_PRIME/THREADS + 1, thread_info->num*MAX_PRIME/THREADS + MAX_PRIME/THREADS);
		
	int i;
	for (i=thread_info->num*MAX_PRIME/THREADS + 1; i<=(thread_info->num*MAX_PRIME/THREADS + MAX_PRIME/THREADS); i++) {
		if (is_happy(i)) { happiness[i] = 1; printf("%d is happy\n", i); }
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
		pthread_cond_wait(&thread_termination, &mutex);	// Block here until thread_termination signals (ie a thread dies)
		
		int a = 0;
		while (t_info[a].state == THREAD_LIVE) { a++; }
		if (a >= THREADS)
		{
			fprintf(stderr, "Error. All threads alive but thread_termination signalled.\n");
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
	for (d=0; d<THREADS; d++)
	{	
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
		pthread_cond_wait(&thread_termination, &mutex);			// threads terminate when main() blocks here
		int e;
		for (e=0; e<THREADS; e++)
		{
			if (t_info[e].state == THREAD_DEAD)
			{
				pthread_join(t_info[e].tid, NULL);
				numAlive--;
			}
		}
	}
	
	//join all threads
	
	
	pthread_mutex_unlock(&mutex);
	pthread_cond_destroy(&thread_termination);
}


//-Make THREADS threads, each with MAX_PRIME/THREADS range. Have them go through and run is_happy() on every element that is prime.
//-Have them store this info in an array.
//-Then write all this stuff to the scratch file.