#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>


// make global variables static so they can't interfere with other files
static const char* MANAGE_SOCKNAME = "/tmp/CS344_manage_sock";

static void* g_data;
static int g_data_len;

static pthread_mutex_t death_mut;
static pthread_cond_t death_cond;

static pthread_mutex_t data_mut;
static pthread_cond_t data_cond;

long long compute_flops() {
  printf("starting compute flops\n");
  int* buf = malloc(40000);
  long long i = 0;
  int j, c, sum;  // i is the candidate perfect int. j is the possible
		  // divisor. sum is the sum of all divisors. c makes
		  // sure we do 1000 perfect calculations before
		  // checking time.
  int curtime = time(NULL);
  while (time(NULL) < curtime + 10) {
    for (c=0; c<1000; c++) {
      sum = 0;
      for (j=2; j<i; j++) {
	if (i % j == 0)
	  { sum += j; }
      }
      if (sum == i)
	{ buf[i%40000] = 1; }
      i++;
    }
  }
  free(buf);
  return (i*i/2 - 2*i) / 10;       // Return # of operations done per second
  
}

void find_perfect_nums(int start, int end, int* nums) {
  int i,j, sum;
  for (i=start; i<=end; i++)
  {
    sum = 0;
    for (j=2; j<i; j++)
    {
      if (i % j == 0)
	{ sum += j; }
    }
    if (sum == i)
      { nums[i-start] = 1; }
  }
}

void* manage_communicate(void* arg) {
  // Open active socket, connect to manage.py's bound socket, then read from the pipe until a message is received. Signal to the main thread that something happened.
  // Things to communicate:
  // First wait for main to cond_signal its done with performance testing. Open a socket with manage, send this information
  // Get work, use a conditional variable to tell main to start working?
  // send data when done, then go back to previous line
  // Listen for kill signal, then raise the terminate signal

  struct sockaddr_un manage_sock_struct;
  int s;
  int READ_BUF_MAX = 1000;
  char read_buf[READ_BUF_MAX];
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  
  manage_sock_struct.sun_family = AF_UNIX;

  strcpy(manage_sock_struct.sun_path, MANAGE_SOCKNAME);

  s = connect(sfd, (const struct sockaddr*) &manage_sock_struct, sizeof(struct sockaddr_un));
  if (s == -1)
    { perror("creating manage socket"); exit(EXIT_FAILURE); }
  
  pthread_mutex_lock(&data_mut);
    
  s = write(sfd, (char*) g_data, sizeof(char) * g_data_len);
  if (s == -1)
    { perror("writing to socket");  exit(EXIT_FAILURE);  }

  pthread_mutex_unlock(&data_mut);

  s = read(sfd, read_buf, READ_BUF_MAX);
  if (s == -1)
    { perror("reading from socket");  exit(EXIT_FAILURE);  }

  printf("compute response: %s\n", read_buf);

  char* start_range = strchr(read_buf, ':') + 2;
  char* mid_range = strchr(start_range, '-') + 1;
  char* end_range = strchr(mid_range, '"');

  char start[100];
  char end[100];
  snprintf(start, (size_t) (mid_range - start_range - 1), "%s", start_range);
  snprintf(end, end_range - mid_range, "%s", mid_range);
  int s = strtol(start, NULL, 10);
  int e = strtol(end, NULL, 10);
    
  s = pthread_mutex_lock(&death_mut);
  printf("Function manage_communicate() run.\n");
  pthread_cond_signal(&death_cond);
  s = pthread_mutex_unlock(&death_mut);
  
  exit(EXIT_SUCCESS);
}


int main() {
  // create new thread to block in manage_communicate, then block until
  int s;
  pthread_t manage_thread;

  s = pthread_mutex_init(&death_mut, NULL);
  if (s == -1)
    { perror("initializing death mutex");  exit(EXIT_FAILURE);  }

  s = pthread_mutex_init(&data_mut, NULL);
  if (s == -1)
    { perror("initializing data mutex");  exit(EXIT_FAILURE);  }
  
  s = pthread_cond_init(&death_cond, NULL);
  if (s == -1)
    { perror("initializing death conditional variable"); exit(EXIT_FAILURE);  }

  s = pthread_cond_init(&data_cond, NULL);
  if (s == -1)
    { perror("initializing data conditional variable"); exit(EXIT_FAILURE);  }

  pthread_mutex_lock(&death_mut);
  pthread_mutex_lock(&data_mut);
  pthread_create(&manage_thread, NULL, manage_communicate, NULL);  // Begin communication thread

  long long r = compute_flops();
  int pid = (int) getpid();

  char outbuf[1000];
  sprintf(outbuf, "{\"sender\":\"compute\",\"pid\":\"%d\",\"flops\":\"%lld\"}", pid, r);

  g_data = (void*) outbuf;
  g_data_len = strlen(outbuf);

  pthread_mutex_unlock(&data_mut);
  
  pthread_cond_wait(&death_cond, &death_mut);     // Wait for thread to terminate
  pthread_mutex_unlock(&death_mut);
  
  s = pthread_cond_destroy(&death_cond);
  if (s == -1)
    { perror("destroy death conditional variable"); exit(EXIT_FAILURE);  }

  s = pthread_cond_destroy(&data_cond);
  if (s == -1)
    { perror("destroy data conditional variable"); exit(EXIT_FAILURE);  }

  s = pthread_mutex_destroy(&death_mut);
  if (s == -1)
    {  perror("destroy death mutex"); exit(EXIT_FAILURE);  }

  s = pthread_mutex_destroy(&data_mut);
  if (s == -1)
    {  perror("destroy data mutex"); exit(EXIT_FAILURE);  }

  exit(EXIT_SUCCESS);
}
