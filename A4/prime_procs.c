#include <limits.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#include "dynarr.h"

#define MAX_PRIME 881269
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
	long MSIZE = MAX_PRIME * sizeof(int);
	
	long i;
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
	long MSIZE = MAX_PRIME * sizeof(int);
	long j = 2;
	
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
	long m = 1;									// Current sieve prime, initialized to 1
	long MSIZE = MAX_PRIME * sizeof(int);
	int child_pids[PROCESSES];
	
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
	
	long a;
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
		wait(NULL);									// Block until a process terminates
		
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
	if ((out_fd = open("happy_primes_procs", O_RDWR | O_CREAT | O_TRUNC, 0755)) == -1)
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
	
	munmap(primes, MSIZE);
	munmap(happiness, MSIZE);
	
}