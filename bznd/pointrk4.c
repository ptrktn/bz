/****h* bznd/pointrk4.c
 * NAME
 *  pointrk4.c
 *  $Id: pointrk4.c,v 1.3 2004/10/23 02:13:04 petterik Exp $
 * DESCRIPTION
 *  Mahara et al. model, point system.
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bzsim.h"

static  float xini = 0.007979;
static	float zini = 0.02109;
static	float aini = 0.24914;
static	float epsilon = 0.12;
static	float q = 0.0008;
static	float alpha = 71.94;
static	float f = 0.8;
static	float b = 0.1;
static	float phi = 0;

#define x(i) (xin[i-1])
#define xdot(i) (xout[i-1])

static void Fdot(float t, float *xin, float *xout, void *data)
{
  xdot(1) = ( x(3) * x(1) - x(1) * x(1) 
			 - (f * b * x(2) + phi) 
			 * (x(1) - q * x(3))/(x(1) + q * x(3)) 
			 )/epsilon;
  xdot(2) = x(3) * x(1) - b * x(2);
  xdot(3) = (0.5 * x(1) * x(1) - x(3) * x(1) 
			 - (f * b * x(2) + phi) 
			 * q * x(3) / (x(1) + q * x(3)) 
			 )/alpha;
}


int main(int argc, char **argv)
{

	int t;
	float x0[3], xout[3], h = 0.0001;

	x0[0] = xini;
	x0[1] = zini;
	x0[2] = aini;

	for (t = 0; t*h < 500.0; t++) {
		bzsimRK4(t*h, x0, xout, h, 3, NULL, Fdot);

		if ((t % 1000) == 0)
			printf("%f %f %f %f\n", t*h, xout[0], xout[1], xout[2]);

		memcpy(x0, xout, 3 * sizeof(float));
	}

	return 0;
}



