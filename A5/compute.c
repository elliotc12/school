#include <pthread.h>
#include <signal.h>
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

long long compute_rate() {
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
      for (j=1; j<i; j++) {
	if (i % j == 0)
	  { sum += j; }
      }
      if (sum == i)
	{ buf[i%40000] = 1; }
      i++;
    }
  }
  free(buf);
  return (i*i/2) / 10;       // Return # of operations done per second
  
}

void find_perfect_nums(long long start, long long end, char* is_perfect) {
  long long i,j, sum;
  for (i=start; i<=end; i++)
  {
    sum = 0;
    for (j=1; j<i; j++)
    {
      if (i % j == 0)
	{ sum += j; }
    }
    if (sum == i)
      { is_perfect[i-start] = '1'; }
  }
}

void* listen_for_termination(void* args) {
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  int READ_BUF_MAX = 1000;
  char read_buf[READ_BUF_MAX];
  struct sockaddr_un manage_sock_struct;

  manage_sock_struct.sun_family = AF_UNIX;
  strcpy(manage_sock_struct.sun_path, MANAGE_SOCKNAME);
  
  int s = connect(sfd, (const struct sockaddr*) &manage_sock_struct, sizeof(struct sockaddr_un));
  if (s == -1)
    { perror("creating term manage socket"); exit(EXIT_FAILURE); }

  char* str = "{\"sender\":\"term\"}";
  
  s = write(sfd, str, strlen(str));
  if (s == -1)
    { perror("writing to socket");  exit(EXIT_FAILURE);  }

  s = read(sfd, read_buf, READ_BUF_MAX);
  if (s == -1)
    { perror("reading from socket");  exit(EXIT_FAILURE);  }

  printf("compute.c exiting due to -k flag.\n");
  
  exit(EXIT_SUCCESS);
}

int main() {
  // create new thread to block in manage_communicate, then block until

  int s;
  pthread_t kill_signal_listener;

  pthread_create(&kill_signal_listener, NULL, listen_for_termination, NULL);  // Begin communication thread

  struct sockaddr_un manage_sock_struct;
  int READ_BUF_MAX = 1000;
  char read_buf[READ_BUF_MAX];
  int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
  
  manage_sock_struct.sun_family = AF_UNIX;

  strcpy(manage_sock_struct.sun_path, MANAGE_SOCKNAME);

  s = connect(sfd, (const struct sockaddr*) &manage_sock_struct, sizeof(struct sockaddr_un));
  if (s == -1)
    { perror("creating manage socket"); exit(EXIT_FAILURE); }
  
  long long r = compute_rate();
  int pid = (int) getpid();

  char outbuf[1000];
  sprintf(outbuf, "{\"sender\":\"compute\",\"pid\":\"%d\",\"flops\":\"%lld\"}", pid, r);

  s = write(sfd, outbuf, sizeof(outbuf));
  if (s == -1)
    { perror("writing to socket");  exit(EXIT_FAILURE);  }

  s = read(sfd, read_buf, READ_BUF_MAX);
  if (s == -1)
    { perror("reading from socket");  exit(EXIT_FAILURE);  }

  char* start_range = strchr(read_buf, ':') + 2;
  char* mid_range = strchr(start_range, '-') + 1;
  char* end_range = strchr(mid_range, '"');

  char start_buf[100];
  char end_buf[100];

  strncpy(start_buf, start_range, mid_range - start_range - 1);
  start_buf[mid_range - start_range - 1] = '\0';
  strncpy(end_buf, mid_range, end_range - mid_range);
  end_buf[end_range - mid_range] = '\0';
 
  
  long long start = strtoll(start_buf, NULL, 10);
  long long end = strtoll(end_buf, NULL, 10);  
  long long perfect_length = end - start;
  char* is_perfect = malloc( (end - start) * sizeof(char) );
  memset(is_perfect, '0', (size_t) (end - start) * sizeof(char));

  find_perfect_nums(start, end, is_perfect);
  
  char data_packet[perfect_length + 13];

  sprintf(data_packet, "{\"data\":\"%s\"}", is_perfect);
  
  s = write(sfd, data_packet, perfect_length + 13);
  if (s == -1)
    { perror("writing to socket");  exit(EXIT_FAILURE);  }
  

  exit(EXIT_SUCCESS);

}
