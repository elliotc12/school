#include <math.h>
#include <stdio.h>

#define MAX_PRIME 100
#define THREADS 16

int primes[MAX_PRIME];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

enum t_state {
	THREAD_UNSTARTED,
	THREAD_LIVE,
	THREAD_DEAD
};

struct {
	pthread_t tid;
	int sieve_prime;
	enum t_state state;
} t_info*;




int get_next_sieve_prime() {
	int i;
	for (i=0; i<MAX_PRIME; i++)
	{
		primes[i] = 1;
	}
	
	primes[0] = 0;
	int m = 2;
	while (m <= floor(sqrt(MAX_PRIME)))
	{
	  	if (primes[m-1] == 1) { launch_thread(m); }	// is prime, launch thread and restart
		m++;
	}
	
	int k;
	for (k=0; k<MAX_PRIME; k++)
	{
		if (primes[k-1] == 1) printf("prime: %d\n", k);
	}
	return 0;
}

void cull_sieve(void* arg) {
	int j = 2;
		
	while (m*j < MAX_PRIME)
	{
		pthread_mutex_lock(&mutex);
		primes[m*j-1] = 0;
		pthread_mutex_unlock(&mutex);
		j++;
	}
}

int main() {
	int m = 2;
	int t_count = 0;

	if (t_info = malloc(sizeof(*t_info) * THREADS) == NULL) 
	{
		perror("calloc: ");
		exit(EXIT_FAILURE);
	}
	
	while (m <= floor(sqrt(MAX_PRIME)))
	{
		m = get_next_sieve_prime();
		if (t_count <= THREADS)
		{
			int a = 0;
			while (t_info[a].state == THREAD_DEAD) { a++; }
			t_info[a].state = THREAD_LIVE;
			t_info[a].sieve_prime = m;
			pthread_create(&t_info[a].tid, NULL, cull_sieve, NULL);
			t_count++;
			break;
		}
		else
		{
			int a = 0;
			while (t_info[a].state != THREAD_DEAD) { a++; }
			pthread_join(&t_info[a].tid, NULL);
			t_info[a].state = THREAD_LIVE;
			t_info[a].sieve_prime = m;
			pthread_create(&t_info[a].tid, NULL, cull_sieve, NULL);
		}
		//get next element
		//if t_count < THREADS launch new thread, starting @ lowest idx
		//else find a terminated thread, join it & relaunch it
	}
	
	
}




