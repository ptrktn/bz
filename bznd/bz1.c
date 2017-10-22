/****h* bz3d2/bz3d2.c
 * NAME
 *  bz3d2.c
 *  $Id: bz1.c,v 1.18 2005/06/29 15:32:35 petterik Exp $
 * DESCRIPTION
 *  BZ simulation...
 ******
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bzsim.h"

static int MODEL = 0;
static int NX = 0;
static int NY = 0;
static int NZ = 0;
static int NX1;
static int NY1;
static int NZ1;
static int NXNY;
static int N = 0;
static int SNAP;
static int SNAP_DF3;
static int SNAPZ;
static int ITERATIONS;
static float L;
static float TIMESTEP_SAFETY;
static int NPTS;
static int BOUNDARY = 0;
static float SCAL_MIN = 0;
static float SCAL_MAX = 0;
static int SNAP_START = 0;
static float *X1, *Z1, *A1, *X2, *Z2, *A2;
static float dt, dx, dt_o_dx2;
static char *diff_grid;
static char bTurn;
static char OUTPUT_DIR[MAX_PAR_LEN];
static char SIMULATION_NAME[MAX_PAR_LEN];
static int RANDOMSEED = 0;
static int PACEMAKER_RADIUS = 0;
static float P_SRC = 0;
static int XT_PLOT = 0;
static float phi_eff = 0;
static float xmax = FLT_MIN;
static float a_th = 0;
static float xc = 0;

/* Mahara et al. chemical parameters */
/* http://www.ibiblio.org/koine/greek/lessons/alphabet.html */
static float epsilon = 0, q = 0, alpha = 0, f = 0, b = 0, phi = 0, phi2 = 0;
static float D_x = 0, D_z = 0, D_a = 0;
static float x_ini, z_ini, a_ini;

/* internal parameters specific to this model */
float dt_o_epsilon = 0, dt_o_alpha = 0;

static void xdot(float t, float *xin, float *xout, void *data)
{
	float *diffterm = (float *) data; 	/* FIXME */

	xout[0] = ( xin[2] * xin[0] - xin[0] * xin[0] 
				- (f * b * xin[1] + phi_eff) 
				* (xin[0] - q * xin[2])/(xin[0] + q * xin[2]) 
				)/epsilon + diffterm[0];
	xout[1] = xin[2] * xin[0] - b * xin[1] + diffterm[1];
	xout[2] = (0.5 * xin[0] * xin[0] - xin[2] * xin[0] 
			   - (f * b * xin[1] + phi_eff) 
			   * q * xin[2] / (xin[0] + q * xin[2]) 
			   )/alpha + diffterm[2];
}

static void save_xt(char *s, float *x, float smax, int nx, int ny, int nz)
{
	FILE *fp;
	char path[1024];
	int xi = nx / 2, yi = ny / 2, zi, nxny = nx * ny;
	unsigned char *buf = XALLOC(nz, unsigned char);
	float *dat = XALLOC(nz, float);

	for (zi = 0; zi != nz; zi++)
		dat[zi] = (MODEL != 4 ? x[xi + yi * nx + zi * nxny] : x[zi]);

	bzsimScaleFloatArr(dat, buf, nz, SCAL_MIN, smax);

	sprintf(path, "%s/%s.dat", OUTPUT_DIR, s);

	if ((fp = fopen(path, "a"))) {
		fwrite(buf, nz, 1, fp);
		fclose(fp);
	} else
		bzsimPanic("can not write `%s'", path);

	XFREE(buf);
	XFREE(dat);
}

static void get_parameters(char *fname)
{
	GET_INT(fname, "MODEL", MODEL);
	GET_INT(fname, "SNAP", SNAP);
	GET_INT(fname, "SNAP_DF3", SNAP_DF3);
	GET_INT(fname, "SNAPZ", SNAPZ);
	GET_INT(fname, "ITERATIONS", ITERATIONS);
	GET_STRING(fname, "OUTPUT_DIR", OUTPUT_DIR);
	GET_STRING(fname, "SIMULATION_NAME", SIMULATION_NAME);
	GET_INT(fname, "NX", NX);
	GET_INT(fname, "NY", NY);
	GET_INT(fname, "NZ", NZ);
	GET_FLOAT(fname, "L", L);
	GET_FLOAT(fname, "TIMESTEP_SAFETY", TIMESTEP_SAFETY);
	GET_INT(fname, "NPTS", NPTS);
	GET_INT(fname, "RANDOMSEED", RANDOMSEED);
	GET_INT(fname, "BOUNDARY", BOUNDARY);
	GET_INT(fname, "SNAP_START", SNAP_START);
	GET_FLOAT(fname, "SCAL_MIN", SCAL_MIN);
	GET_FLOAT(fname, "SCAL_MAX", SCAL_MAX);
	GET_INT(fname, "PACEMAKER_RADIUS", PACEMAKER_RADIUS);
	GET_FLOAT(fname, "P_SRC", P_SRC);
	GET_INT(fname, "XT_PLOT", XT_PLOT);
	/* chemical parameters */
	GET_FLOAT(fname, "x_ini", x_ini);
	GET_FLOAT(fname, "z_ini", z_ini);
	GET_FLOAT(fname, "a_ini", a_ini);
	GET_FLOAT(fname, "a_th", a_th);
	GET_FLOAT(fname, "epsilon", epsilon);
	GET_FLOAT(fname, "q", q);
	GET_FLOAT(fname, "alpha", alpha);
	GET_FLOAT(fname, "f", f);
	GET_FLOAT(fname, "b", b);
	GET_FLOAT(fname, "phi", phi);
	GET_FLOAT(fname, "phi2", phi2);
	GET_FLOAT(fname, "D_x", D_x);
	GET_FLOAT(fname, "D_z", D_z);
	GET_FLOAT(fname, "D_a", D_a);
}

void save_parameters(char *fname)
{
	LOG_PAR(fname, "MODEL", MODEL);
	LOG_PAR(fname, "NX", NX);
	LOG_PAR(fname, "NY", NY);
	LOG_PAR(fname, "NZ", NZ);
	LOG_PAR(fname, "N", N);
	LOG_PAR(fname, "L", L);
	LOG_PAR(fname, "dt", dt);
	LOG_PAR(fname, "D_x stability", 2.0 * D_x * dt / (dx * dx));
	LOG_PAR(fname, "D_z stability", 2.0 * D_z * dt / (dx * dx));
	LOG_PAR(fname, "D_a stability", 2.0 * D_a * dt / (dx * dx));
	LOG_PAR(fname, "dt_safety", TIMESTEP_SAFETY);
	LOG_PAR(fname, "dx", dx);
	LOG_PAR(fname, "snapshot", SNAP);
	LOG_PAR(fname, "DF3 snapshot", SNAP_DF3);
	LOG_PAR(fname, "snapshot z-layer", SNAPZ);
	LOG_PAR(fname, "dt/dx2", dt_o_dx2);
	LOG_PAR(fname, "BOUNDARY", BOUNDARY);
	LOG_PAR(fname, "PACEMAKER_RADIUS", PACEMAKER_RADIUS);
	LOG_PAR(fname, "P_SRC", P_SRC);
	LOG_PAR(fname, "NPTS", NPTS);
	LOG_PAR(fname, "RANDOMSEED", RANDOMSEED);
	LOG_PAR(fname, "SCAL_MIN", SCAL_MIN);
	LOG_PAR(fname, "SCAL_MAX", SCAL_MAX);
	LOG_PAR(fname, "ITERATIONS", ITERATIONS);
	LOG_PAR(fname, "SNAP_START", SNAP_START);
	LOG_PAR(fname, "XT_PLOT", XT_PLOT);
	LOG_PAR(fname, "x_ini", x_ini);
	LOG_PAR(fname, "z_ini", z_ini);
	LOG_PAR(fname, "a_ini", a_ini);
	LOG_PAR(fname, "a_th", a_th);
	LOG_PAR(fname, "epsilon", epsilon);
	LOG_PAR(fname, "q", q);
	LOG_PAR(fname, "alpha", alpha);
	LOG_PAR(fname, "f", f);
	LOG_PAR(fname, "b", b);
	LOG_PAR(fname, "phi", phi);
	LOG_PAR(fname, "phi2", phi2);
	LOG_PAR(fname, "D_x", D_x);
	LOG_PAR(fname, "D_z", D_z);
	LOG_PAR(fname, "D_a", D_a);
}

static void init_params(char *log)
{
	/* Internal numerical parameters */
	if (MODEL == 4) 
		NY = NX = 1;

	N = NX * NY * NZ;
	NX1 = NX - 1;
	NY1 = NY - 1;
	NZ1 = NZ - 1;
	NXNY = NX * NY;
	dt = (L * L) / (((float) NPTS) * ((float) NPTS));
	dt /= (float) TIMESTEP_SAFETY;
	dx = L / (float) NPTS;
	dt_o_dx2 = dt / (dx * dx);
	dt_o_epsilon = dt / epsilon;
	dt_o_alpha = dt / alpha;
	xc = x_ini / 1000;
}

static void check_parameters(void)
{
	assert(NX && NY && NZ);
	assert(SNAPZ >= 0 && SNAPZ < NZ);
	assert(PACEMAKER_RADIUS < NX / 2 && PACEMAKER_RADIUS < NY / 2);
}

/* fill diffusion spot grid and set variable initial conditions */
static void init_grid(char *log)
{
	int cnt = 0, xi, yi, zi, i;

	/* grid on */
	memset(diff_grid, 1, N);

	/* FIXME: RANDOMSEED is int, function returns unsigned int */
	bzsimLogMsg(log, "SEEDVALUE %d\nSEEDUSED %u\n", 
				RANDOMSEED, bzsimInitRand(RANDOMSEED));

	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0; yi != NY; yi++) {
			for (xi = 0; xi != NX; xi++) {
				i = xi + yi * NX + zi * NXNY;
				switch (MODEL) {
				case 1:
				case 3:
				case 4:
					break;
				default:
					bzsimPanic("undefined model: %d", MODEL);
					break;
				}				/* end switch(MODEL) */
			}					/* end xi */
		}						/* end yi */
	}							/* end zi */

	for (i = 0, cnt = 0; i != N; i++)
		if (diff_grid[i])
			cnt++;

	/* other variables */
	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0; yi != NY; yi++) {
			for (xi = 0; xi != NX; xi++) {
				i = xi + yi * NX + zi * NXNY;
				switch (MODEL) {
				case 4:
					/* catalyst is uniform in space and does not diffuse */
					i = zi;
					X1[i] = X2[i] = 0;
					Z1[i] = Z2[i] = z_ini;
					A1[i] = A2[i] = 0;
					break;
				case 3:
					X1[i] = X2[i] = x_ini;
					Z1[i] = Z2[i] = z_ini;
					A1[i] = A2[i] = 0;
					break;
				default:
					X1[i] = X2[i] = x_ini;
					Z1[i] = Z2[i] = z_ini;
					A1[i] = A2[i] = a_ini;
					break;
				} /* end switch(MODEL) */
			}					/* end xi */
		}						/* end yi */
	}							/* end zi */
}

static void do_step(float t, float *x, float *z, float *a, float *xs, float *zs, float *as)
{
	int i, xi, yi, zi;
	float tmp_x, tmp_z, tmp_a, xin[3], xout[3];


	if (MODEL != 4) {
		/* 3D boundary conditions */
		switch (BOUNDARY) {
		case 1:
			bzsimSinkBC3D(NX, NY, NZ, 3, x, z, a);
			break;
		case 2:
			bzsimNoFluxBC3D(NX, NY, NZ, 3, x, z, a);
			break;
		default:
			/* bzsimPBC3D(NX, NY, NZ, 3, x, y, r); */
			bzsimPanic("undefined boundary type %d", BOUNDARY);
			break;
		}							/* end switch(BOUNDARY) */
	}

	switch (MODEL) {
	case 4:
		/* 1D spatial model */
		/* left and right boundaries are open to reactants */
		x[0] = x[1] = x_ini;
		z[0] = z[1] = z_ini;
		a[0] = a[1] = a_ini;
		x[NZ1] = x[NZ1 - 1] = x_ini;
		z[NZ1] = z[NZ1 - 1] = z_ini;
		a[NZ1] = a[NZ1 - 1] = a_ini;
		/* "dark" conditions */
		break;
	case 3:
		/* diffusion source at z = 1 */
		for (zi = 1, yi = 1; yi != NY1; yi++)
			for (xi = 1; xi != NX1; xi++)
				a[xi + yi * NX + zi * NXNY] = a_ini;
		phi_eff = 0; /* "dark" conditions */
		break;
	case 1:
		/* spatially homegeneous */
		break;
	default:
		bzsimPanic("undefined model type: %d", MODEL);
		break;
	}						/* end switch(MODEL) */

	if (MODEL != 4) {
		/* Calculate each grid point */
		for (zi = 1; zi != NZ1; zi++) {
			for (yi = 1; yi != NY1; yi++) {
				/* TEST: phi_eff = yi < NY/2 ? phi : phi2; */
				for (xi = 1; xi != NX1; xi++) {					
					i = xi + yi * NX + zi * NXNY;
					
					/* reactive term */
					xin[0] = x[i];
					xin[1] = z[i];
					xin[2] = a[i];
					bzsimRK4(t, xin, xout, dt, 3, NULL, xdot);
					
					tmp_x = xout[0] + D_x * dt_o_dx2 * LAPL3D(x, i, NX, NXNY);
					tmp_z = xout[1] + D_z * dt_o_dx2 * LAPL3D(z, i, NX, NXNY);
					tmp_a = xout[2] + D_a * dt_o_dx2 * LAPL3D(a, i, NX, NXNY);
					
					/* Check for negative concentrations. */
					xs[i] = (tmp_x < 0 ? (float) 0 : tmp_x);
					zs[i] = (tmp_z < 0 ? (float) 0 : tmp_z);
					as[i] = (tmp_a < 0 ? (float) 0 : tmp_a);
				}					/* end xi */
			}						/* end yi */
		}							/* end zi */
	} else {
		
		/* MODEL == 4 -- 1D spatial model */
		for (zi = 1; zi != NZ1; zi++) {
			float diffterm[3] = {0, 0, 0};

			/* effective illumination parameter */
			phi_eff = phi;

#if 1
			/* reactive term */

			xin[0] = x[zi];
			xin[1] = z[zi];
			xin[2] = a[zi];


			/* FIXME */
#if 0
			diffterm[0] = D_x * LAPL1D(x, zi)/(dx*dx);
			diffterm[1] = D_z * LAPL1D(z, zi)/(dx*dx);
			diffterm[2] = D_a * LAPL1D(a, zi)/(dx*dx);
#endif

			/* FIXME */
			/* reactive term with a threshold guard */
			/* this is a numerical safeguard */
			if (xin[0] > xc && xin[2] > a_th) {
				bzsimRK4(t, xin, xout, dt, 3, diffterm, xdot);
				tmp_x = xout[0];
				tmp_z = xout[1];
				tmp_a = xout[2];
			} else {
				/* too low propragator concentration */
				tmp_x = xin[0];
				tmp_z = xin[1];
				tmp_a = xin[2];
			}

			/* diffusion term */
			tmp_x += dt * D_x * LAPL1D(x, zi)/(dx*dx);
			tmp_z += dt * D_z * LAPL1D(z, zi)/(dx*dx);
			tmp_a += dt * D_a * LAPL1D(a, zi)/(dx*dx);

#ifndef __APPLE__
			assert(finitef(tmp_x));
			assert(finitef(tmp_z));
			assert(finitef(tmp_a));
#endif /* ! __APPLE__ */

#else
			/* debug code: to be removed */
			tmp_x = x[zi] + dt * D_x * LAPL1D(x, zi)/(dx*dx);
			tmp_z = z[zi] + dt * D_z * LAPL1D(z, zi)/(dx*dx);
			tmp_a = a[zi] + dt * D_a * LAPL1D(a, zi)/(dx*dx);

			assert(finite(tmp_x));
			assert(finite(tmp_z));
			if (! finite(tmp_a) )
				printf("%d %f %f %f %f\n", zi, t, a[0], a[1], a[2]);
			assert(finite(tmp_a));

			/* reactive term */
			xin[0] = x[zi];
			xin[1] = z[zi];
			xin[2] = a[zi];

			bzsimRK4(t, xin, xout, dt, 3, diffterm, xdot);

			{
				int i;

				for (i = 0; i != 3; i++)
					if (! finite(xout[i]) )
						xout[i] = xin[i];
			}

			assert(finite(xout[0]));
			assert(finite(xout[1]));
			assert(finite(xout[2]));

			tmp_x += xout[0];
			tmp_z += xout[1];
			tmp_a += xout[2];

#endif

			/* Check for negative concentrations. */
			xs[zi] = (tmp_x < 0 ? (float) 0 : tmp_x);
			zs[zi] = (tmp_z < 0 ? (float) 0 : tmp_z);
			as[zi] = (tmp_a < 0 ? (float) 0 : tmp_a);

			if (xs[zi] > xmax)
				xmax = xs[zi];
		}		
	}
} /* end do_step */

int main(int argc, char *argv[])
{
	int t = 0;
	char *parfile, fname[1024], log[1024];

#ifdef __TEST
	/* internal test code: this has nothing to do with simulation */
	{
#define MY_X 5
#define MY_Y 5
#define MY_Z 5
		int i, xi, yi, zi;
		float *x = XALLOC(MY_X * MY_Y * MY_Z, float);

		for (i = 1, zi = 1; zi != MY_Z - 1; zi++)
			for (yi = 1; yi != MY_Y - 1; yi++)
				for (xi = 1; xi != MY_X - 1; xi++)
					x[xi + yi * MY_X + zi * MY_X * MY_Y] = i++;

		bzsimPBC3D(MY_X, MY_Y, MY_Z, 1, x);

		for (zi = 0; zi != MY_Z; zi++) {
			bzsimDbg("zi = %d\n", zi);
			for (yi = 0; yi != MY_Y; yi++) {
				for (xi = 0; xi != MY_X; xi++) {
					bzsimDbg("%3d ", xi + yi * MY_X + zi * MY_X * MY_Y);
				}
				bzsimDbg("     ");
				for (xi = 0; xi != MY_X; xi++) {
					bzsimDbg("%3d ", (int) (x[xi + yi * MY_X + zi * MY_X * MY_Y]));
				}
				bzsimDbg("\n");
			}
		}

		XFREE(x);
		exit(0);
#undef MY_X
#undef MY_Y
#undef MY_Z
	}
#endif							/* test code */

	if (argc != 2) {
		fprintf(stderr, "usage: %s <parameter file>\n", argv[0]);
		exit(1);
	}

	bzsimInit(argv[0]);			/* application log file becomes bz3d2.log */
	/* read in the model parameters */
	get_parameters(parfile = argv[1]);
	check_parameters();

	/* create directory if it doesn't exists */
	bzsimMkdir(OUTPUT_DIR);

	/* make copy of the parameter file */
	sprintf(fname, "%s/%s.input", OUTPUT_DIR, SIMULATION_NAME);
	bzsimCopyFile(parfile, fname);

	/* log specific to this simulation */
	sprintf(log, "%s/%s.log", OUTPUT_DIR, SIMULATION_NAME);
	bzsimTruncateFile(log);
	init_params(log);
	save_parameters(log);

	if (MODEL == 2) {
		/* simulate dimensionless (point) system */
		float x0[3], xout[3];

		x0[0] = x_ini;
		x0[1] = z_ini;
		x0[2] = a_ini;
		phi_eff = phi;

		while (t*dt < 500.0) {
			bzsimRK4(t*dt, x0, xout, dt, 3, NULL, xdot);
			if ((t % SNAP) == 0)
				printf("%f %f %f %f\n", t*dt, xout[0], xout[1], xout[2]);
			memcpy(x0, xout, 3 * sizeof(float));
			t++;
		}

		exit(0);
	}

	if (XT_PLOT) {
		bzsimTruncateFile("%s/xt.dat", OUTPUT_DIR);
		bzsimTruncateFile("%s/at.dat", OUTPUT_DIR);
	}

	/* allocate memory for arrays */
	X1 = XALLOC(N, float);
	Z1 = XALLOC(N, float);
	A1 = XALLOC(N, float);
	X2 = XALLOC(N, float);
	Z2 = XALLOC(N, float);
	A2 = XALLOC(N, float);
	diff_grid = XALLOC(N, char);
	bzsimLogMsg(log, "float array bulk space: %d bytes\n",
				(int) (6 * N * sizeof(float)));
	bzsimLogMsg(log, "char array bulk space: %d bytes\n",
				(int) (1 * N * sizeof(float)));
	init_grid(log);
	bzsimSavePid(OUTPUT_DIR);

	/* T I M E   L O O P   S T A R T S */
	bTurn = 0;
	t = 0;
	while (++t <= ITERATIONS) {
		float tim = t * dt;

		/* Switch pointers according to flag bTurn. */
		bTurn = !bTurn;
		if (bTurn)
			do_step(tim, X1, Z1, A1, X2, Z2, A2);
		else
			do_step(tim, X2, Z2, A2, X1, Z1, A1);

		if (SNAP > 0 && t >= SNAP_START && (t % SNAP) == 0) {
			bzsimGridSnap(X1, NX, NY, NZ, BZSIM_FLOAT, BZSIM_Z, NZ / 2, SCAL_MIN, SCAL_MAX, "%s/%010d.jpg", OUTPUT_DIR, t);
			bzsimLogMsg(log, "t = %10.5f %s/%010d.jpg\n", tim, OUTPUT_DIR, t);
		}

		if (SNAP_DF3 > 0 && t >= SNAP_START && (t % SNAP_DF3) == 0) {
			if (BZSIM_OK != bzsimSaveDf3(X1, NX, NY, NZ, BZSIM_DATA_FLOAT, SCAL_MIN, SCAL_MAX, "%s/%010d.df3", OUTPUT_DIR, t))
				bzsimPanic("main: writing DF3 failed %d", t);
		}

		if (XT_PLOT > 0 && (t % XT_PLOT) == 0) {
			save_xt("xt", X1, SCAL_MAX, NX, NY, NZ);
			save_xt("at", A1, a_ini, NX, NY, NZ);
		}

#if 0
		if (0 == (t % 1000))
			printf("%f %f %f %f\n", t*dt, X1[N/2], Z1[N/2], A1[N/2]);
#endif /* 0 */

	}							/* end TIME LOOP */

	/* Free the allocated memory. */
	XFREE(X1);
	XFREE(X2);
	XFREE(Z1);
	XFREE(Z2);
	XFREE(A1);
	XFREE(A2);
	XFREE(diff_grid);

	
	bzsimLogMsg(log, "xmax %f\n", xmax);

	/* Timing info */
	bzsimSetEndTime();
	bzsimLogMsg(log, "elapsed wall-clock time: %d seconds\n",
				bzsimGetElapsedTime());

	if (XT_PLOT > 0) {
		int sz = ITERATIONS / XT_PLOT;

		bzsimExec("convert -quality 99 -depth 8 -size %dx%d gray:%s/xt.dat JPG:%s/xt.jpg", NZ, sz, OUTPUT_DIR, OUTPUT_DIR);
		bzsimExec("convert -equalize %s/xt.jpg %s/00_space_time.jpg", OUTPUT_DIR, OUTPUT_DIR);
		bzsimExec("convert -depth 8 -size %dx%d gray:%s/at.dat JPG:%s/at.jpg", NZ, sz, OUTPUT_DIR, OUTPUT_DIR);
	}

	bzsimRemovePid(OUTPUT_DIR);

	return 0;
}
