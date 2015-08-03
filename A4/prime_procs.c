#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#include "dynarr.h"

#define MAX_PRIME 10000
#define PROCESSES 16

int* primes;
int* happiness;

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

static void* find_happiness(int section) {
	int MSIZE = MAX_PRIME * sizeof(int);
	
	int i;
	for (i = section*MAX_PRIME/PROCESSES + 1; i<=(section*MAX_PRIME/PROCESSES + MAX_PRIME/PROCESSES); i++) {
		if (primes[i] && is_happy(i+1)) { happiness[i] = 1; }
	}
	
	if (msync(happiness, MSIZE, MS_SYNC) == -1)
	{
		perror("mysnc: ");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}

void mark_multiples(int m) {
	int MSIZE = MAX_PRIME * sizeof(int);
	int j = 2;
	
	/*sem_t* num_term_children;
	
	num_term_children = sem_open("num_term_children", 0);
	if (num_term_children == SEM_FAILED)
	{
		perror("sem_open: ");
		exit(EXIT_FAILURE);
	}*/
	
	while (m*j <= MAX_PRIME)
	{
		primes[m*j-1] = 0;
		j++;
	}
	
	if (msync(primes, MSIZE, MS_SYNC) == -1)
	{
		perror("mysnc: ");
		exit(EXIT_FAILURE);
	}
	
	//sem_post(num_term_children);
	exit(EXIT_SUCCESS);
}

int main() {
	int m = 1;									// Current sieve prime, initialized to 1
	int MSIZE = MAX_PRIME * sizeof(int);
	int child_pids[PROCESSES];
	
	sem_t* num_term_children;
	
	num_term_children = sem_open("num_term_children", O_CREAT, S_IRUSR | S_IWUSR, 0);
	if (num_term_children == SEM_FAILED)
	{
		perror("sem_open: ");
		exit(EXIT_FAILURE);
	}
	
	int fd = open("prime_procs_temp_shm", O_RDWR | O_CREAT | O_TRUNC, 0755);
	if (fd == -1)
	{
		perror("open: ");
		exit(EXIT_FAILURE);
	}
	
	int fd_h = open("prime_procs_temp_shm_happiness", O_RDWR | O_CREAT | O_TRUNC, 0755);
	if (fd == -1)
	{
		perror("open: ");
		exit(EXIT_FAILURE);
	}
	
	if (ftruncate(fd, MSIZE) == -1)
	{
		perror("ftruncate: ");
		exit(EXIT_FAILURE);
	}
	
	if (ftruncate(fd_h, MSIZE) == -1)
	{
		perror("ftruncate: ");
		exit(EXIT_FAILURE);
	}
	
	primes = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (primes == MAP_FAILED)
	{
		perror("mmap: ");
		exit(EXIT_FAILURE);
	}
	
	happiness = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_h, 0);
	if (happiness == MAP_FAILED)
	{
		perror("mmap: ");
		exit(EXIT_FAILURE);
	}
	
	if (close(fd) == -1)
	{
		perror("close: ");
		exit(EXIT_FAILURE);
	}
	
	if (close(fd_h) == -1)
	{
		perror("close: ");
		exit(EXIT_FAILURE);
	}
	
	int a;
	for (a=0; a<MAX_PRIME; a++)
	{
		primes[a] = 1;
		happiness[a] = 0;
	}
	primes[0] = 0;
	
	if (msync(primes, MSIZE, MS_SYNC) == -1)
	{
		perror("mysnc: ");
		exit(EXIT_FAILURE);
	}
	
	if (msync(happiness, MSIZE, MS_SYNC) == -1)
	{
		perror("mysnc: ");
		exit(EXIT_FAILURE);
	}
		
	int c;
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
			child_pids[c] = p;
			continue;
		}
	}

	while ((m = get_next_sieve_prime(m)) != -1)
	{
		//sem_wait(num_term_children);							// Block until a process terminates
		
		wait(NULL);
		
		int b = 0;
		while (waitpid(child_pids[b], NULL, WNOHANG) == 0) { b++; }
		if (b >= PROCESSES) {
			fprintf(stderr, "Error. num_term_children incremented but no children exited.\n");
			exit(EXIT_FAILURE);
		}
		
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
			child_pids[b] = p;
			continue;
		}
		
	}
	
	int g;											// Wait for all processes to terminate
	for (g=0; g<PROCESSES; g++)
	{
		waitpid(child_pids[g], NULL, 0);
	}


	int f;
	for (f=0; f<PROCESSES; f++)
	{	
		int p = fork();
		if (p == 0)
		{
			find_happiness(f);
		} 
		else if (p == -1)
		{
			perror("fork: ");
			exit(EXIT_FAILURE);
		} 
		else
		{
			child_pids[f] = p;
			continue;
		}
	}

	int e;											// Wait for all processes to terminate again
	for (e=0; e<PROCESSES; e++)
	{
		waitpid(child_pids[e], NULL, 0);
	}
	
	int d;
	for (d=0; d<MAX_PRIME; d++)
	{
		if (happiness[d] == 1) { printf("%d\n", d+1); }
	}

	
	/*int fd;
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
	
	sem_unlink("num_term_children");
}