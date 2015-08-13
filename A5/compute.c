#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

const char* MANAGE_SOCKNAME = "/tmp/CS344_manage_sock";

pthread_mutex_t mut;
pthread_cond_t cond;

long compute_flops() {
  printf("starting compute flops\n");
  int* buf = malloc(40000);
  int i = 0;
  int j, sum;
  int curtime = time(NULL);
  while (time(NULL) < curtime + 10) {
    sum = 0;
    for (j=2; j<i; j++) {
      if (i % j == 0)
	{ sum += j; }
    }
    if (sum == i)
      { buf[i%40000] = 1; }
    i++;
  }
  free(buf);
  printf("ending compute flops\n");
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
  //char read_buf[100];
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  
  manage_sock_struct.sun_family = AF_UNIX;

  strcpy(manage_sock_struct.sun_path, MANAGE_SOCKNAME);

  s = connect(sfd, (const struct sockaddr*) &manage_sock_struct, sizeof(struct sockaddr_un));
  if (s == -1)
  {
    perror("creating manage socket");
    exit(EXIT_FAILURE);
  }

  long r = compute_flops();
  int pid = (int) getpid();

  char outbuf[1000];
  sprintf(outbuf, "{\"sender\":\"compute\",\"pid\":\"%d\",\"flops\":\"%ld\"}", pid, r);

  printf("string: %s\n", outbuf);
  
  s = write(sfd, outbuf, sizeof(outbuf));
  if (s == -1)
  {
    perror("reading from socket");
    exit(EXIT_FAILURE);
  }
  
  s = pthread_mutex_lock(&mut);
  printf("Function manage_communicate() run.\n");
  pthread_cond_signal(&cond);
  s = pthread_mutex_unlock(&mut);
  
  exit(EXIT_SUCCESS);
}


int main() {
  // create new thread to block in manage_communicate, then block until
  int s;
  pthread_t manage_thread;

  s = pthread_mutex_init(&mut, NULL);
  if (s == -1) {
    perror("initializing mutex");
    exit(EXIT_FAILURE);
  }
  
  s = pthread_cond_init(&cond, NULL);
  if (s == -1) {
    perror("initializing conditional variable");
    exit(EXIT_FAILURE);
  }

  s = pthread_mutex_lock(&mut);
  
  pthread_create(&manage_thread, NULL, manage_communicate, NULL);

  pthread_cond_wait(&cond, &mut);     // Wait for thread to terminate

  pthread_mutex_unlock(&mut);

  s = pthread_cond_destroy(&cond);
  if (s == -1) {
    perror("destroy conditional variable");
    exit(EXIT_FAILURE);
  }
  
  s = pthread_mutex_destroy(&mut);
  if (s == -1) {
    perror("destroy mutex");
    exit(EXIT_FAILURE);
  }
  
  exit(EXIT_SUCCESS);
}
