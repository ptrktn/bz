/*
matrixg.c
degrading polymer
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "rgb.h"

#ifdef VISUAL
#include <gl/gl.h>
#define P_MAX 1.0
#define DP_MAX 1.0
#define H_MAX 1.0
#define D_MAX 1.0
#define RGB 3
#define UPDATE_G 100
#endif

#define K1    1.0
#define KR1   0.0

#define BETA            10

#define L               8
#define NX              32
#define NY              10

#define TIME_END        5500000
#define SNAP            0
#define DIR             "./tmp/"
#define NAME            "Fivemil"
#define PRINT_TIME      (TIME_END/10)
#define MINMAX         1
#define PRINT_PROFILES (TIME_END/5)
#define TIME_PLOT       0
#define RELEASE_RATE    100



#define Alpha 15
#define Beta 5
/* f2(dp,p)=Gamma*f1(dp,p) */
#define Gamma 0.5



/* function for the nonlinear diffusion term */
float
f1 (float dp, float p)
{
  return exp (-Alpha * dp - Beta * p);
/* return 1.0; */
}

/* don't touch to the definitions below */
/* you are not expected to understand this */
#define NX1             (NX-1)
#define NY1             (NY-1)
#define N               (NX*NY)
#define INDEX           (x+y*NX)
#define NY2             NY+NY
#define NY3             NY+NY+NY

void
main (int argc, char *argv[])
{
#ifdef VISUAL
  short blackvec[RGB] = { 0, 0, 0 };
  long vert[2];
  short rgbvec[RGB];
#endif


  float m0, mt;
  FILE *rfile;
  float yn = 0.0, yn1;

  float sum;
  float *d, *d2, *h, *h2, *dp, *dp2, *p, *p2,
    k1 = K1, kr1 = KR1, kterm1, kterm2, dterm, hterm, dxyi, dpxyi, xyi,
    dxy1, dxy2, dxy3, dxy4, dt, dx, fij, hij, gij, dt_o_dx2;
  int i, x, y, t = 0;
  char fname[300];
  int j;
  FILE *file;
#if MINMAX
  float dmin = 1.0e22, dmax = -1.0e22, hmin = 1.0e22, hmax = -1.0e22,
    dpmin = 1.0e22, dpmax = -1.0e22, pmin = 1.0e22, pmax = -1.0e22;
#endif
/* allocate memory for arrays */
  d = malloc (N * sizeof (float));
  d2 = malloc (N * sizeof (float));
  h = malloc (N * sizeof (float));
  h2 = malloc (N * sizeof (float));
  dp = malloc (N * sizeof (float));
  dp2 = malloc (N * sizeof (float));
  p = malloc (N * sizeof (float));
  p2 = malloc (N * sizeof (float));

  if (!d || !d2 || !h || !h2 || !dp || !dp2 || !p || !p2)
    {
      fprintf (stderr, "out of memory\n");
      exit (1);




    }

  dx = ((float) L) / ((float) NX);
  dt = (((dx * dx) / (4.0 * 1.0))) / ((float) BETA);
  dt_o_dx2 = (dt) / (dx * dx);

  fprintf (stderr, "NX   = %d\n", NX);
  fprintf (stderr, "NY   = %d\n", NY);
  fprintf (stderr, "L    = %d\n", L);
  fprintf (stderr, "E.Z. = %d\n", TIME_END);
  fprintf (stderr, "dx   = %g\n", dx);
  fprintf (stderr, "dt   = %g\n", dt);
  fprintf (stderr, "%d bytes\n", 8 * N * sizeof (float));

#ifdef VISUAL
  foreground ();
  prefsize (NX, 4 * NY);
  winopen ("matrix");
  RGBmode ();
  gconfig ();
  c3s (blackvec);
  clear ();
#endif

/* initial conditions */
  for (y = 0; y < NY; y++)
    for (x = 0; x < NX; x++)
      {
	i = INDEX;
	if (x < NX / 2)
	  {
	    p[i] = 0.0;
	    d[i] = 0.0;
	    h[i] = 0.0;
	    dp[i] = 1.0;
	  }
	else
	  {
	    p[i] = 0.0;
	    d[i] = 0.0;
	    h[i] = 1.00;
	    dp[i] = 0.0;
	  }
/* if (x<NX/4) {dp[i]=0.5; p[i]=0.5;} else {dp[i]=0.0; p[i]=0.5;} */
	p2[i] = d2[i] = h2[i] = dp2[i] = 0.0;
      }



/* initial amount in matrix */
  for (y = 0; y < NY; y++)
    for (x = 0; x < NX; x++)
      {
	i = INDEX;
	m0 += dp[i];
      }
  sprintf (fname, "rate%s.dat", NAME);
  rfile = fopen (fname, "w");
  if (!rfile)
    exit (1);






/*
                T I M E     L O O P     S T A R T S
 */
  t = 0;
  while (t++ < TIME_END)
    {

/* non-flux boundaries */
      y = 0;
      for (x = 0; x < NX; x++)
	{
	  i = INDEX;
	  h[i] = h[i + NX];
	  d[i] = d[i + NX];
	}
      y = NY1;
      for (x = 0; x < NX; x++)
	{
	  i = INDEX;
	  h[i] = h[i - NX];
	  d[i] = d[i - NX];
	}
      x = 0;
      for (y = 0; y < NY; y++)
	{
	  i = INDEX;
	  h[i] = h[i + 1];
	  d[i] = d[i + 1];
	}
      x = NX1;
      for (y = 0; y < NY; y++)
	{
	  i = INDEX;
	  h[i] = h[i - 1];
	  /* sink for the drug */
	  d[i] = 0.0;
	}

/* end of non-flux boundaries */

/* sweep */
      for (y = 1; y < NY1; y++)
	for (x = 1; x < NX1; x++)
	  {
	    i = INDEX;

/* kinetics */
	    kterm1 = k1 * h[i] * dp[i];
	    kterm2 = kr1 * h[i] * p[i] * d[i];
/* d[D]/dt */
	    fij = kterm1 - kterm2;
/* d[DP]/dt */
	    hij = -fij;
/* d[P]/dt */
	    gij = fij;

/* calculate D(dp,p) */
	    dxyi = f1 (dp[i], p[i]);
	    dxy1 = 0.5 * (f1 (dp[i + 1], p[i + 1]) + dxyi);
	    dxy2 = 0.5 * (f1 (dp[i - 1], p[i - 1]) + dxyi);
	    dxy3 = 0.5 * (f1 (dp[i + NX], p[i + NX]) + dxyi);
	    dxy4 = 0.5 * (f1 (dp[i - NX], p[i - NX]) + dxyi);
/* diffusion of h */
	    xyi = h[i];
	    hterm = dxy1 * (h[i + 1] - xyi) - dxy2 * (xyi - h[i - 1])
	      + dxy3 * (h[i + NX] - xyi) - dxy4 * (xyi - h[i - NX]);
/* hterm=h[i-1]+h[i+1]+h[i+NX]+h[i-NX]-4.0*h[i];  */
/* diffusion of drug */
	    xyi = d[i];
	    dxy1 *= Gamma;
	    dxy2 *= Gamma;
	    dxy3 *= Gamma;
	    dxy4 *= Gamma;
	    dterm = dxy1 * (d[i + 1] - xyi) - dxy2 * (xyi - d[i - 1])
	      + dxy3 * (d[i + NX] - xyi) - dxy4 * (xyi - d[i - NX]);

/* here comes Euler */
	    dp2[i] = dp[i] + dt * hij;
	    p2[i] = p[i] + dt * gij;
	    d2[i] = d[i] + dt * fij + dt_o_dx2 * dterm;
	    h2[i] = h[i] + dt_o_dx2 * hterm;

	  }			/* end sweep */

/* swap variables */
      for (i = 0; i < N; i++)
	{
	  if (p2[i] > 0.0)
	    p[i] = p2[i];
	  else
	    p[i] = 0.0;
	  if (h2[i] > 0.0)
	    h[i] = h2[i];
	  else
	    h[i] = 0.0;
	  if (d2[i] > 0.0)
	    d[i] = d2[i];
	  else
	    d[i] = 0.0;
	  if (dp2[i] > 0.0)
	    dp[i] = dp2[i];
	  else
	    dp[i] = 0.0;
#if MINMAX
	  if (d[i] < dmin)
	    dmin = d[i];
	  else if (d[i] > dmax)
	    dmax = d[i];
	  if (h[i] < hmin)
	    hmin = h[i];
	  else if (h[i] > hmax)
	    hmax = h[i];
	  if (p[i] < pmin)
	    pmin = p[i];
	  else if (p[i] > pmax)
	    pmax = d[i];
	  if (dp[i] < dpmin)
	    dpmin = dp[i];
	  else if (dp[i] > dpmax)
	    dpmax = dp[i];
#endif
	}			/* end swap */

#if PRINT_TIME
      if (!(t % PRINT_TIME))
	fprintf (stderr, "%-10d\n", t);
#endif

#if SNAP
      if (!(t % SNAP))
	{
	  sprintf (fname, "%s%s.%d.rgb", DIR, NAME, (long) t + 100000000);
	  write_rgb2of (fname, P_VAR, NX, NY, 0);
	}
#endif


#ifdef VISUAL
      if (!(t % UPDATE_G))
	{
	  /* free drug */
	  for (y = 0; y < NY; y++)
	    for (x = 0; x < NX; x++)
	      {
		j = (int) (255.0 * d[INDEX] / D_MAX);
		bgnpoint ();
		rgbvec[0] = j;
		rgbvec[1] = j;
		rgbvec[2] = j;
		c3s (rgbvec);
		vert[0] = x;
		vert[1] = y;
		v2i (vert);
		endpoint ();
	      }
	  /* H+ */
	  for (y = 0; y < NY; y++)
	    for (x = 0; x < NX; x++)
	      {
		j = (int) (255.0 * h[INDEX] / H_MAX);
		bgnpoint ();
		rgbvec[0] = j;
		rgbvec[1] = j;
		rgbvec[2] = j;
		c3s (rgbvec);
		vert[0] = x;
		vert[1] = NY + y;
		v2i (vert);
		endpoint ();
	      }
	  /* bound drug + polymer */
	  for (y = 0; y < NY; y++)
	    for (x = 0; x < NX; x++)
	      {
		j = (int) ((255.0 * dp[INDEX]) / DP_MAX);
		bgnpoint ();
		rgbvec[0] = j;
		rgbvec[1] = j;
		rgbvec[2] = j;
		c3s (rgbvec);
		vert[0] = x;
		vert[1] = NY2 + y;
		v2i (vert);
		endpoint ();
	      }
	  /* free polymer */
	  for (y = 0; y < NY; y++)
	    for (x = 0; x < NX; x++)
	      {
		j = (int) ((255.0 * p[INDEX]) / P_MAX);
		bgnpoint ();
		rgbvec[0] = j;
		rgbvec[1] = j;
		rgbvec[2] = j;
		c3s (rgbvec);
		vert[0] = x;
		vert[1] = NY3 + y;
		v2i (vert);
		endpoint ();
	      }
	}
#endif

#if PRINT_PROFILES
      if (!(t % PRINT_PROFILES))
	{
	  /* here we print the concentration d(x) */
	  sprintf (fname, "dp%s%d.dat", NAME, (int) t + 1000000);
	  file = fopen (fname, "w");
	  if (!file)
	    exit (1);
	  /* print the profile from middle along the y-direction */
	  y = NY / 2;
	  for (x = 0; x < NX; x++)
	    fprintf (file, "%f %f\n", (float) x * dx, dp[INDEX]);
	  fclose (file);

	  /* here we print the concentration d(x) */
	  sprintf (fname, "p%s%d.dat", NAME, (int) t + 1000000);
	  file = fopen (fname, "w");
	  if (!file)
	    exit (1);
	  /* print the profile from middle along the y-direction */
	  y = NY / 2;
	  for (x = 0; x < NX; x++)
	    fprintf (file, "%f %f\n", (float) x * dx, p[INDEX]);
	  fclose (file);

	  /* here we print the concentration d(x) */
	  sprintf (fname, "h%s%d.dat", NAME, (int) t + 1000000);
	  file = fopen (fname, "w");
	  if (!file)
	    exit (1);
	  /* print the profile from middle along the y-direction */
	  y = NY / 2;
	  for (x = 0; x < NX; x++)
	    fprintf (file, "%f %f\n", (float) x * dx, h[INDEX]);
	  fclose (file);

	  /* here we print the concentration d(x) */
	  sprintf (fname, "d%s%d.dat", NAME, (int) t + 1000000);
	  file = fopen (fname, "w");
	  if (!file)
	    exit (1);
	  /* print the profile from middle along the y-direction */
	  y = NY / 2;
	  for (x = 0; x < NX; x++)
	    fprintf (file, "%f %f\n", (float) x * dx, d[INDEX]);
	  fclose (file);
	}
#endif

#if TIME_PLOT
      if (!(t % TIME_PLOT))
	{
	  sum = 0.0;
	  for (y = 0; y < NY; y++)
	    for (x = NX / 2; x < NX; x++)
	      sum += d[INDEX];
	  sum /= (float) NX *NY * dx * dx;
	  sum *= 2.0;
	  fprintf (stdout, "%f %f\n", (float) dt * t, sum);
	  fflush (stdout);
	}
#endif



      if (!(t % RELEASE_RATE))
	{
	  mt = 0.0;
	  for (y = 0; y < NY; y++)
	    for (x = 0; x < NX / 2; x++)
	      {
		i = INDEX;
		mt += dp[i];
		mt += d[i];
	      }
	  yn1 = ((m0 - mt) / m0);
/* fprintf(rfile,"%f %f\n",(float)dt*t, (yn1-yn)/dt); */
	  fprintf (rfile, "%f %f\n", (float) dt * t, yn1);
	  fflush (rfile);
	  yn = yn1;
	}

    }				/* end t */


#if MINMAX
  fprintf (stderr, "dmin=%f dmax=%f\n", dmin, dmax);
  fprintf (stderr, "hmin=%f hmax=%f \n", hmin, hmax);
  fprintf (stderr, "dpmin=%f dpmax=%f \n", dpmin, dpmax);
  fprintf (stderr, "pmin=%f pmax=%f \n", pmin, pmax);
#endif



/* free allocated memory */
  free (d);
  free (p);
  free (h);
  free (dp);
  free (d2);
  free (p2);
  free (h2);
  free (dp2);


#ifdef VISUAL
  sleep (10);
  gexit ();
#endif

}				/* end main */
