#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const int size = 10;

void log_data(double* data, int fd) {
  char* str = malloc(100 * sizeof(char));
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      sprintf(str, "%2.5f", data[x+y*size]);
      strcat(str, " ");
      if (write(fd, str, sizeof(char) * strlen(str)) == -1)
	{
	  perror("write to arfile: ");
	  exit(EXIT_FAILURE);
	}
    }
  }
}

int main() {
  double dx = 0.03; //m
  double dy = 0.03; //m
  double dt = 0.0003; //s
  double tau = 0.0003;
  double epsilonbar = 0.01;
  double mu = 1.0;
  double k = 4;
  double delta = 0.02;
  double anisotropy = 4.0;
  double alpha = 0.9;
  double gamma = 10.0;
  double Teq = 1.0;
  int r = 3;

  int fd;
  if ((fd = open("data.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH)) == -1)
    {
      perror("open failed: ");
      exit(EXIT_FAILURE);
    }

  printf("fd start: %d\n", fd);

  int arrsize = size*size;
  double T[arrsize];
  double ep[arrsize];
  double dep_dt[arrsize];
  double phi[arrsize];
  double dphi_dx[arrsize];
  double dphi_dy[arrsize];
  double lap_T[arrsize];
  double lap_phi[arrsize];
  double phi_new[arrsize];
  double T_new[arrsize];
  double phi_grad_angle[arrsize];
  double t = 0;
  double t_final = 100.0;

  for (int y = 0; y<size; y++) {
    for (int x = 0; x<size; x++) {
      T[x+y*size] = 0;
      ep[x+y*size] = 0;
      dep_dt[x+y*size] = 0;
      phi[x+y*size] = 0;
      dphi_dx[x+y*size] = 0;
      dphi_dy[x+y*size] = 0;
      lap_T[x+y*size] = 0;
      lap_phi[x+y*size] = 0;
      phi_new[x+y*size] = 0;
      T_new[x+y*size] = 0;
      phi_grad_angle[x+y*size] = 0;

      if ((x-size/2.0)*(x-size/2.0) + (y-size/2.0)*(y-size/2.0) < r*r) {
	phi[x+y*size] = 1.0;
      }
    }
  }
  log_data(phi, fd);

  close(fd);
}
