#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const double m_pi =  3.14159265358979323846;
const int size = 1000;
const int animate = 1;
const int skiprate = 250;
const int debug = 0;

void log_data(double* data, int fd) {
  char* str = malloc(10 * sizeof(char));
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      sprintf(str, "%2.4f", data[x+y*size]);
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
  double dt = 0.0001; //s
  double t_final = 0.7; //s
  
  double tau = 0.0003;
  double delta_bar = 0.01; // Average thickness of layer (?)
  double F = 0.5; // Latent heat of fusion
  double mu = 0.02; // Strength of anisotropy
  double anisotropy = 6.0;
  double beta = 0.9; // n-shifting coefficient
  double eta = 10.0; // n-shifting coefficient
  double TM = 1.0;
  double T0 = 0.0;
  double theta_0 = 1.57;
  int r = 3;

  int fd;
  if ((fd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IWOTH | S_IROTH)) == -1)
    {
      perror("open failed: ");
      exit(EXIT_FAILURE);
    }

  int arrsize = size*size;
  double* delta = malloc(sizeof(double) * arrsize);
  double* delta_dt = malloc(sizeof(double) * arrsize);
  double* u = malloc(sizeof(double) * arrsize);
  double* phi = malloc(sizeof(double) * arrsize);
  double* dphi_dx = malloc(sizeof(double) * arrsize);
  double* dphi_dy = malloc(sizeof(double) * arrsize);
  double* lap_u = malloc(sizeof(double) * arrsize);
  double* lap_phi = malloc(sizeof(double) * arrsize);
  double* phi_new = malloc(sizeof(double) * arrsize);
  double* u_new = malloc(sizeof(double) * arrsize);
  double* phi_grad_angle = malloc(sizeof(double) * arrsize);
  double t = 0;

  for (int y = 0; y<size; y++) {
    for (int x = 0; x<size; x++) {
      u[x+y*size] = -1;       // u0 = (T0-TM) / (TM-T0) = -1
      delta[x+y*size] = 0;
      delta_dt[x+y*size] = 0;
      phi[x+y*size] = 0;
      dphi_dx[x+y*size] = 0;
      dphi_dy[x+y*size] = 0;
      lap_u[x+y*size] = 0;
      lap_phi[x+y*size] = 0;
      phi_new[x+y*size] = 0;
      u_new[x+y*size] = 0;
      phi_grad_angle[x+y*size] = 0;

      if ((x-size/2.0)*(x-size/2.0) + (y-size/2.0)*(y-size/2.0) < r*r) {
	phi[x+y*size] = 1.0;
      }
    }
  }

  if (animate) log_data(phi, fd);

  int ym, yp, xm, xp;
  double ddelta2_dx, ddelta2_dy, term1, term2, term3, n;
  for (t=0; t<t_final; t+=dt) {
    if ((int)(t/dt) % (int)(t_final/(dt*10)) == 0) printf("step %g of %g\n", t/dt, t_final/dt);
    for (int y = 0; y<size; y++) {
      for (int x = 0; x<size; x++) {
	ym = y-1;
	yp = y+1;
	xm = x-1;
	xp = x+1;
	
	if (ym==-1) ym = size-1;
	if (xm==-1) xm = size-1;
	if (yp==size) yp = 0;
	if (xp==size) xp = 0;

	dphi_dx[x+y*size] = (phi[xp+y*size] - phi[xm+y*size]) / dx;
	dphi_dy[x+y*size] = (phi[x+yp*size] - phi[x+ym*size]) / dy;

	lap_phi[x+y*size] = ((phi[xm+ym*size] +
			      phi[xm+y*size] +
			      phi[xm+yp*size] +
			      phi[x+ym*size] +
			      phi[x+yp*size] +
			      phi[xp+ym*size] +
			      phi[xp+y*size] +
			      phi[xp+yp*size]) - 8.0*phi[x+y*size]) / (3.0*dx*dy);
	
	lap_u[x+y*size] = ((u[xm+ym*size] +
			    u[xm+y*size] +
			    u[xm+yp*size] +
			    u[x+ym*size] +
			    u[x+yp*size] +
			    u[xp+ym*size] +
			    u[xp+y*size] +
			    u[xp+yp*size]) - 8.0*u[x+y*size]) / (3.0*dx*dx);

	phi_grad_angle[x+y*size] = atan2(dphi_dy[x+y*size], dphi_dx[x+y*size]);

	delta[x+y*size] = delta_bar*(1.0 + mu * cos(anisotropy*(theta_0 - phi_grad_angle[x+y*size])));

	delta_dt[x+y*size] = -delta_bar*anisotropy*mu*sin(anisotropy*phi_grad_angle[x+y*size]);

	ddelta2_dx = (delta[xp+y*size]*delta[xp+y*size] - delta[xm+y*size]*delta[xm+y*size]) / (2*dx);
	ddelta2_dy = (delta[x+yp*size]*delta[x+yp*size] - delta[x+ym*size]*delta[x+ym*size]) / (2*dy);

	term1 = -(delta[xp+y*size]*delta_dt[xp+y*size]*dphi_dy[xp+y*size]
		  - delta[xm+y*size]*delta_dt[xm+y*size]*dphi_dy[xm+y*size]) / (2*dx);

	term2 = (delta[x+yp*size]*delta_dt[x+yp*size]*dphi_dx[x+yp*size]
		 - delta[x+ym*size]*delta_dt[x+ym*size]*dphi_dx[x+ym*size]) / (2*dy);

	term3 = ddelta2_dx*dphi_dx[x+y*size] + ddelta2_dy*dphi_dy[x+y*size];

	n = beta/m_pi * atan(-eta*(TM-T0)*u[x+y*size]);

	phi_new[x+y*size] = phi[x+y*size] + (term1 + term2 + term3 +
	       delta[x+y*size]*delta[x+y*size]*lap_phi[x+y*size] +
               phi[x+y*size]*(1.0-phi[x+y*size])*(phi[x+y*size] - 0.5 + n)) * dt / tau;
	
	u_new[x+y*size] = u[x+y*size] + lap_u[x+y*size]*dt + F*(phi_new[x+y*size] - phi[x+y*size]);
	/* if (x==3 && y==3) printf("%2.2g\n", lap_T[x+y*size]*dt); */
      }
    }
    
    for (int y = 0; y<size; y++) {
      for (int x = 0; x<size; x++) {
	if (debug) printf("%2.4g ", lap_u[x+y*size]*dt);
	//if (debug) printf("%2.4g ", u[x+y*size]);
	//if (debug) printf("%2.4g ", phi[x+y*size]);
    	phi[x+y*size] = phi_new[x+y*size];
    	u[x+y*size] = u_new[x+y*size];
      }
      if (debug) printf("\n");
    }
    if (debug) printf("\n");
    if (animate && (int)(t/dt) % skiprate == 0) log_data(phi, fd);
  }
  log_data(phi, fd);
  close(fd);
}
