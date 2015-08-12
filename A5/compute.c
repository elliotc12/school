#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const char* MANAGE_SOCKNAME = "/tmp/CS344_manage_sock"

void* manage_communicate(void* arg) {
  // Open active socket, connect to manage.py's bound socket, then read from the pipe until a message is received. Signal to the main thread that something happened.
  // Things to communicate:
  // First wait for main to cond_signal its done with performance testing. Open a socket with manage, send this information
  // Get work, use a conditional variable to tell main to start working?
  // send data when done, then go back to previous line
  // Listen for kill signal, then raise the terminate signal

  struct sockaddr_un manage_sock_struct;
  int sfd;
  int s;
  
  memset(&manage_sock_struct, 0, sizeof(struct sockaddr_un));
  manage_sock_struct.sun_family = AF_UNIX;

  strcpy(manage_sock_struct.sun_path, MANAGE_SOCKNAME);

  s = connect(sfd, (struct sockaddr_un*) &manage_sock_struct, sizeof(struct sockaddr_un));
  if (s == -1)
  {
    perror("creating manage socket");
    exit(EXIT_FAILURE);
  }
  
  printf("Function manage_communicate() run.\n");
  exit(EXIT_SUCCESS);
}


int main() {
  // create new thread to block in manage_communicate, then block until 
  pthread_t manage_thread;
  pthread_create(&manage_thread, NULL, manage_communicate, NULL);

  sleep(1);
  
  exit(EXIT_SUCCESS);
}
