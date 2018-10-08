/****h* revival/pointskg.c
 * NAME
 *  pointskg.c
 * DESCRIPTION
 *  SKG model DOI: 10.1021/jp9832721
 *  Eqs E6-E8
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bzsim.h"

static  float x1ini = 0.01;
static	float x2ini = 0.0;
static	float x3ini = 0.01;

static float  galpha = 1000000,
	gbeta = 77.4,
	ggamma = 1102, 
	gdelta = 2719, 
	gf = 0.776;


#define x(i) (xin[i-1])
#define xdot(i) (xout[i-1])
#define realsqrt(x) sqrtf(x)

static void Fdot(float t, float *xin, float *xout, void *data)
{
 xdot(1) = x(2) * (gbeta - galpha * x(1)) + x(3) * (ggamma * realsqrt(x(1)) + 1.0) - gdelta * x(1) * x(1);
  xdot(2) = 1.0 - x(2) * (gbeta + galpha * x(1));
  xdot(3) = gf - x(3) * (ggamma * realsqrt(x(1)) + 1.0); 
}


int main(int argc, char **argv)
{

	int t;
	float x0[3], xout[3], h = 1/10000.0;

	x0[0] = x1ini;
	x0[1] = x2ini;
	x0[2] = x3ini;

	for (t = 0; t*h < 1.0; t++) {
		bzsimRK4(t*h, x0, xout, h, 3, NULL, Fdot);

		printf("%f %f %f %f\n", t*h, xout[0], xout[1], xout[2]);

		memcpy(x0, xout, 3 * sizeof(float));
	}

	return 0;
}



