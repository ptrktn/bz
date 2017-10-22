/* $Id: ueda.c,v 1.2 2005/01/10 02:05:24 petterik Exp $ */

#include <stdio.h>
#include <math.h>

#define B 12.0
#define k 0.10
#define PERIOD 2.0*3.141592653589

static void derivs(double, double *, double *);

int main(int argc, char **argv)
{
    double y[2], dydx[2], yout[2], h, hh, h6, yt[2], dyt[2], dym[2], x, xh;
    int steps, i, j, ppoints, p;

	/* intial values */
    y[0] = 0.0;
    y[1] = 0.0;
    dydx[0] = 0.0000001;
    dydx[1] = 0.0000001;

	/* h=0.01; */
    steps = 50;

	/* redefine h */
    h = PERIOD / ((double) steps);
    hh = h * 0.5;
    h6 = h / 6.0;
    ppoints = 40000;

    for (p = 0; p < ppoints; p++) {
		for (j = 0; j < steps; j++) {
			xh = x + hh;
			for (i = 0; i < 2; i++)
				yt[i] = y[i] + hh * dydx[i];
			derivs(xh, yt, dyt);
			for (i = 0; i < 2; i++)
				yt[i] = y[i] + hh * dyt[i];
			derivs(xh, yt, dym);
			for (i = 0; i < 2; i++) {
				yt[i] = y[i] + h * dym[i];
				dym[i] += dyt[i];
			}
			derivs(x + h, yt, dyt);
			for (i = 0; i < 2; i++) {
				yout[i] = y[i] + h6 * (dydx[i] + dyt[i] + 2.0 * dym[i]);
				y[i] = yout[i];
			}
			x += h;
		}
		printf("%f \t %f\n", y[1], y[0]);
    }

	return 0;
}

static void derivs(double x, double y[], double dydx[])
{
    dydx[0] = B * cos(x) - (y[0] * k + y[1] * y[1] * y[1]);
    dydx[1] = y[0];
}
