#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const int size = 10;

void log_data(double data[size][size], int fd) {
  printf("size: ", );
  char* str;
  printf("%s, fd: %d\n", str, fd);
  if (write(fd, str, sizeof(char) * strlen(str)) == -1)
    {
      perror("write to arfile: ");
      exit(EXIT_FAILURE);
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
  int r = 10;

  int fd;
  if ((fd = open("data.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH)) == -1)
    {
      perror("open failed: ");
      exit(EXIT_FAILURE);
    }

  printf("fd start: %d\n", fd);

  double T[size-1][size-1];
  double ep[size-1][size-1];
  double dep_dt[size-1][size-1];
  double phi[size-1][size-1];
  double dphi_dx[size-1][size-1];
  double dphi_dy[size-1][size-1];
  double lap_T[size-1][size-1];
  double lap_phi[size-1][size-1];
  double phi_new[size-1][size-1];
  double T_new[size-1][size-1];
  double phi_grad_angle[size-1][size-1];
  double t = 0;
  double t_final = 100.0;

  for (int x = 0; x<size; x++) {
    for (int y = 0; y<size; y++) {
      T[x][y] = 0;
      ep[x][y] = 0;
      dep_dt[x][y] = 0;
      phi[x][y] = 0;
      dphi_dx[x][y] = 0;
      dphi_dy[x][y] = 0;
      lap_T[x][y] = 0;
      lap_phi[x][y] = 0;
      phi_new[x][y] = 0;
      T_new[x][y] = 0;
      phi_grad_angle[x][y] = 0;

      if ((x-size/2.0)*(x-size/2.0) + (y-size/2.0)*(y-size/2.0) < r*r) {
	phi[x][y] = 1.0;
      }
    }
  }
  log_data(phi, fd);

  close(fd);
}
