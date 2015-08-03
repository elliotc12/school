#include <math.h>
#include <stdio.h>

#define MAX_PRIME 1000000
#define PROCESSES 16

int get_next_sieve_prime(int start) {
	int m = start + 1;
	while (m <= floor(sqrt(MAX_PRIME)))
	{
	  	if (primes[m-1] == 1) { return m; }		// m is prime, return as next sieve prime
		m++;
	}
	return -1;
}

void mark_multiples(int m) {
	printf("new process where m=%d\n", m);
	int j = 2;
	
	while (m*j <= MAX_PRIME)
	{
		j++;
	}
	
	exit(EXIT_SUCCESS);
}


int main() {
	int m = 1;									// Current sieve prime, initialized to 1
	
	int primes[MAX_PRIME];
	int happiness[MAX_PRIME];
	
	int i;										// Mark all numbers as prime and unhappy
	for (i=0; i<MAX_PRIME; i++) 
	{
		primes[i] = 1;
		happiness[i] = 0;
	}
	primes[0] = 0;
	
	int c;										// Initialize threads and thread structures
	for (c=0; c<PROCESSES; c++)
	{
		m = get_next_sieve_prime(m);
		
		int p = fork();
		if (p == 0)
		{
			mark_multiples(m);
		} 
		else if (p == -1)
		{
			perror("fork: ");
			exit(EXIT_FAILURE);
		} 
		else
		{
			continue;
		}
		
		//fork process, send child to mark_multiples with m as parameter. Have it mark shared memory array.
	}
	
	/*
	while ((m = get_next_sieve_prime(m)) != -1)
	{
		//wait() for child to die? or use a SIGCHLD handler to wait for kids to die, decrement in handler
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
	
	
	/*
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
	
	int fd;
	if ((fd = open("happy_primes", O_WRONLY | O_CREAT | O_TRUNC, 0755)) == -1)
	{
		perror("error open: ");
		exit(EXIT_FAILURE);
	}
	
	int e;
	int p = 1;
	for (e=0; e<MAX_PRIME; e++)
	{
		if (happiness[e])
		{
			char buf[100];
			sprintf(buf, "%d", (unsigned int) e+1);			// Make this print what it's actually supposed to print
			write(fd, buf, strlen(buf));
			p++;
		}
	}
	
	if (close(fd) == -1)
	{
		perror("close: ");
		exit(EXIT_FAILURE);
	}
	
	*/
}