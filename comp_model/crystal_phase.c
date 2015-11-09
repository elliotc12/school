#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const double M_PI =  3.14159265358979323846;
const int size = 100;

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
  sprintf(str, "\n");
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
  int r = 50;

  int fd;
  if ((fd = open("data.txt", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH)) == -1)
    {
      perror("open failed: ");
      exit(EXIT_FAILURE);
    }

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
  double t_final = 0.005;

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

  int ym, yp, xm, xp;
  double dep2_dx, dep2_dy, term1, term2, term3, m;
  for (t=0; t<t_final; t+=dt) {
    printf("t=%f\n", t);
    for (int y = 0; y<size; y++) {
      for (int x = 0; x<size; x++) {
	ym = y-1;
	yp = y+1;
	xm = x-1;
	xp = x+1;
	
	if (ym==-1) ym = size-1;
	if (xm==-1) xm = size-1;
	if (yp==size) ym = 0;
	if (xp==size) xm = 0;

	dphi_dx[x+y*size] = (phi[xp+y*size] - phi[xm+y*size]) / dx;
	dphi_dy[x+y*size] = (phi[x+yp*size] - phi[x+ym*size]) / dy;

	lap_phi[x+y*size] = (2.0*(phi[xm+ym*size] + phi[xm+y*size] + phi[xm+yp*size] + phi[x+ym*size] + phi[x+yp*size] + phi[xp+ym*size] + phi[xp+y*size] + phi[xp+yp*size]) - 12.0*phi[x+y*size]) / (3.0*dx*dy);
	lap_T[x+y*size] = (2.0*(T[xm+ym*size] + T[xm+y*size] + T[xm+yp*size] + T[x+ym*size] + T[x+yp*size] + T[xp+ym*size] + T[xp+y*size] + T[xp+yp*size]) - 12.0*T[x+y*size]) / (3.0*dx*dy);

	if (dphi_dx[x+y*size] == 0) {
	  if (dphi_dy[x+y*size] < 0)
	    phi_grad_angle[x+y*size] = -0.5 * M_PI;
	  else if (dphi_dy[x+y*size] > 0)
	    phi_grad_angle[x+y*size] =  0.5 * M_PI;
	}
	else {
	  phi_grad_angle[x+y*size] = atan2(dphi_dy[x+y*size], dphi_dx[x+y*size]);
	}

	ep[x+y*size] = epsilonbar*(1.0 + delta * cos(anisotropy*phi_grad_angle[x+y*size]));

	dep_dt[x+y*size] = -epsilonbar*anisotropy*delta*sin(anisotropy*phi_grad_angle[x+y*size]);

	dep2_dx = (ep[xp+y*size]*ep[xp+y*size] - ep[xm+y*size]*ep[xm+y*size]) / dx;
	dep2_dy = (ep[x+yp*size]*ep[x+yp*size] - ep[x+ym*size]*ep[x+ym*size]) / dy;

	term1 = (ep[x+yp*size]*dep_dt[x+yp*size]*dphi_dx[x+yp*size]
			- ep[x+ym*size]*dep_dt[x+ym*size]*dphi_dx[x+ym*size]) / dy;
	
	term2 = (ep[xp+y*size]*dep_dt[xp+y*size]*dphi_dx[xp+y*size]
			- ep[xm+y*size]*dep_dt[xm+y*size]*dphi_dx[xm+y*size]) / dx;

	term3 = dep2_dx*dphi_dx[x+y*size] + dep2_dy*dphi_dy[x+y*size];

	m = alpha/M_PI * atan(gamma*(Teq-T[x+y*size]));

	phi_new[x+y*size] = phi[x+y*size] + (term1 + term2 + ep[x+y*size]*ep[x+y*size]
		     *lap_phi[x+y*size] + term3 + phi[x+y*size]*(1.0-phi[x+y*size])
		     *(phi[x+y*size] - 0.5 - m)) * dt / tau;

	T_new[x+y*size] = T[x+y*size] + lap_T[x+y*size]*dt + k*(phi[x+y*size] /*-phiold?*/);
      }
    }
    
    for (int y = 0; y<size; y++) {
      for (int x = 0; x<size; x++) {
	phi[x+y*size] = phi_new[x+y*size];
	T[x+y*size] = T_new[x+y*size];
      }
    }
    log_data(phi, fd);
  }

  close(fd);
}
