/*  $Id: bz3d2.c,v 1.5 2004/01/30 22:43:12 petterik Exp $ */
/****h* bz3d2/bz3d2.c
 *
 * NAME
 *  bz3d2.c
 * DESCRIPTION
 *  3D BZ demo sandbox for experimenting models and parameters. See bz3d2.dat 
 *  for comments about parameters.
 ******
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bzsim.h"

static int             NX = 0;
static int             NY = 0;
static int             NZ = 0;
static int             NX1;
static int             NY1;
static int             NZ1;
static int             NXNY;
static int             N = 0;
static int             SNAP;
static int             SNAP_DF3;
static int             SNAPZ;
static int             ITERATIONS;
static float           L;
static float           TIMESTEP_SAFETY;
static float           D_r;
static int             NPTS;
static int             BOUNDARY = 0;
static int             MODEL = 0;
static float           tf_f = (float) 1.25;
static float           tf_q = (float) 0.002;
static float           tf_eps = (float) 0.05;
static float           SCAL_MIN = (float) 0;
static float           SCAL_MAX = (float) 0;
static int             SNAP_START = 0;
static float          *X1, *Y1, *R1, *X2, *Y2, *R2;
static float           dt, dx, dt_o_dx2;
static float           x_ss;
static float           dt_o_tf_eps;
static char           *diff_grid;
static char            bTurn;
static char            OUTPUT_DIR[MAX_PAR_LEN];
static char            SIMULATION_NAME[MAX_PAR_LEN];
static int             RANDOMSEED = 0;
static int             PACEMAKER_RADIUS = 0;
static float           P_SINK = (float) 0;
static float           P_SRC = (float) 0;
static int             XT_PLOT = 0;
static int             N_CLUSTER = 1;

static void 	save_xt(float *x, int nx, int ny, int nz)
{
	FILE          *fp;
	char           path[1024];
	int            xi = nx/2, yi = ny/2, zi, nxny = nx * ny;
	unsigned char *buf = XALLOC(nz, unsigned char);
	float         *dat = XALLOC(nz, float);
	
	for (zi = 0; zi != nz; zi++)
		dat[zi] = x[xi + yi * nx + zi * nxny];
		
	bzsimScaleFloatArr(dat, buf, nz, SCAL_MIN, SCAL_MAX);

	sprintf(path, "%s/xt.dat", OUTPUT_DIR);
	if ((fp = fopen(path, "a"))) {
		fwrite(buf, nz, 1, fp);
		fclose(fp);
	}

	XFREE(buf);
	XFREE(dat);
}

static void     my_get_parameters(char *fname)
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
	GET_FLOAT(fname, "D_r", D_r);
	GET_INT(fname, "NPTS", NPTS);
	GET_INT(fname, "RANDOMSEED", RANDOMSEED);
	GET_FLOAT(fname, "TF_F", tf_f);
	GET_FLOAT(fname, "TF_Q", tf_q);
	GET_FLOAT(fname, "TF_EPS", tf_eps);
	GET_INT(fname, "BOUNDARY", BOUNDARY);
	GET_INT(fname, "SNAP_START", SNAP_START);
	GET_FLOAT(fname, "SCAL_MIN", SCAL_MIN);
	GET_FLOAT(fname, "SCAL_MAX", SCAL_MAX);
	GET_INT(fname, "PACEMAKER_RADIUS", PACEMAKER_RADIUS);
	GET_FLOAT(fname, "P_SINK", P_SINK);
	GET_FLOAT(fname, "P_SRC", P_SRC);
	GET_INT(fname, "XT_PLOT", XT_PLOT);
	GET_INT(fname, "N_CLUSTER", N_CLUSTER);
}

void            my_save_parameters(char *fname)
{
	LOG_PAR(fname, "MODEL", MODEL);
	LOG_PAR(fname, "NX", NX);
	LOG_PAR(fname, "NY", NY);
	LOG_PAR(fname, "NZ", NZ);
	LOG_PAR(fname, "N", N);
	LOG_PAR(fname, "L", L);
	LOG_PAR(fname, "dt", dt);
	LOG_PAR(fname, "dt-safety", TIMESTEP_SAFETY);
	LOG_PAR(fname, "dx", dx);
	LOG_PAR(fname, "snapshot", SNAP);
	LOG_PAR(fname, "DF3 snapshot", SNAP_DF3);
	LOG_PAR(fname, "snapshot z-layer", SNAPZ);
	LOG_PAR(fname, "x_ss", x_ss);
	LOG_PAR(fname, "y_ss", x_ss);
	LOG_PAR(fname, "dt/dx2", dt_o_dx2);
	LOG_PAR(fname, "BOUNDARY", BOUNDARY);
	LOG_PAR(fname, "PACEMAKER_RADIUS", PACEMAKER_RADIUS);
	LOG_PAR(fname, "P_SINK", P_SINK);
	LOG_PAR(fname, "P_SRC", P_SRC);
	LOG_PAR(fname, "D_r", D_r);
	LOG_PAR(fname, "NPTS", NPTS);
	LOG_PAR(fname, "RANDOMSEED", RANDOMSEED);
	LOG_PAR(fname, "TF_F", tf_f);
	LOG_PAR(fname, "TF_Q", tf_q);
	LOG_PAR(fname, "TF_EPS", tf_eps);
	LOG_PAR(fname, "SCAL_MIN", SCAL_MIN);
	LOG_PAR(fname, "SCAL_MAX", SCAL_MAX);
	LOG_PAR(fname, "ITERATIONS", ITERATIONS);
	LOG_PAR(fname, "SNAP_START", SNAP_START);
	LOG_PAR(fname, "XT_PLOT", XT_PLOT);
	LOG_PAR(fname, "N_CLUSTER", N_CLUSTER);
}

static void     my_init_params(char *log)
{
	/* Internal numerical parameters */
	N = NX * NY * NZ;
	NX1 = NX - 1;
	NY1 = NY - 1;
	NZ1 = NZ - 1;
	NXNY = NX * NY;
	x_ss = (float) 0;
	dt = (L * L) / (((float) NPTS) * ((float) NPTS));
	dt /= (float) TIMESTEP_SAFETY;
	dx = ((float) L) / ((float) NPTS);
	dt_o_dx2 = dt / (dx * dx);
	dt_o_tf_eps = dt / tf_eps;
}

static void     my_check_parameters(void)
{
	assert(NX && NY && NZ);
	assert(SNAPZ >= 0 && SNAPZ < NZ);
	assert(PACEMAKER_RADIUS);
	assert(PACEMAKER_RADIUS < NX/2 && PACEMAKER_RADIUS < NY/2);
	assert(tf_f != (float) 1);
}

static void random_cluster(unsigned char *grid, int x, int y, int z)
{
	int ntry = 0, i = x + y * NX + z * NXNY, filled = 1;
	
	if (!x || !y || !z || x == NX1 || y == NY1 || z == NZ1)
		return; /* mind the borders */

	assert(N_CLUSTER <= 27);
	grid[i] = 0;

	while (filled < N_CLUSTER) {
		int dx = bzsimRandInt(3) - 1;
		int dy = bzsimRandInt(3) - 1;
		int dz = bzsimRandInt(3) - 1;
		
		i = x + dx + (y + dy)* NX + (z + dz)*NXNY;
		if (grid[i] != 0 && bzsimRand() < 0.5) {
			grid[i] = 0;
			filled++;
		}
		
		ntry++;
		assert(ntry < 1000); /* safeguard; this is all about luck... */
	}
}

/* fill diffusion spot grid and set variable initial conditions */
static void     my_init_grid(char *log)
{
	int             cnt = 0, xi, yi, zi, i;

	/* grid on */
	memset(diff_grid, 1, N);

	/* UGLY: RANDOMSEED is int, function returns unsigned int */
	bzsimLogMsg(log, "SEEDVALUE %d\nSEEDUSED %u\n", RANDOMSEED, bzsimInitRand(RANDOMSEED));

	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0; yi != NY; yi++) {
			for (xi = 0; xi != NX; xi++) {
				i = xi + yi * NX + zi * NXNY;
				switch (MODEL) {
				case 1:
					/* isotropic medium */					
					break;
				case 2:
					if (zi > NZ/3 && bzsimRand() < P_SINK)
						diff_grid[i] = 0;
					break;
				case 3: 
					{
						int h = 20;
						if (zi > h && zi < 2*h && bzsimRand() < P_SINK) {
							if (N_CLUSTER == 1)
								diff_grid[i] = 0;
							else
								random_cluster(diff_grid, xi, yi, zi);
						}
					}
					break;
				case 4:
					/* random diff sources all over the volume */
					/* practically the same as bz2d but in 3D */
					diff_grid[i] = ((bzsimRand() < P_SRC) ? 1 : 0);
					break;
				default:
					bzsimPanic("undefined model: %d", MODEL);
					break;
				}	/* end switch(MODEL) */
			}	/* end xi */
		}	/* end yi */
	}		/* end zi */

	for (i = 0, cnt = 0; i != N; i++)
		if (diff_grid[i])
			cnt++;

	if (MODEL == 4) 
		bzsimLogMsg(log, "SRCS %d\nGRID_POINTS %d\nACTUAL_DENSITY %f\n", cnt, N, cnt / (float) N);
	else
		bzsimLogMsg(log, "SINKs %d\nGRID_POINTS %d\nACTUAL_DENSITY %f\n", cnt, N, ((float) cnt) / (N));

	/* other variables */
	x_ss = (tf_q * (tf_f + 1)) / (tf_f - 1);
	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0; yi != NY; yi++) {
			for (xi = 0; xi != NX; xi++) {
				i = xi + yi * NX + zi * NXNY;
				R1[i] = R2[i] = (float) 0;
				X1[i] = X2[i] = x_ss;
				Y1[i] = Y2[i] = x_ss;
			}	/* end xi */
		}		/* end yi */
	}			/* end zi */

	{
		unsigned char *tmp = XALLOC(N, unsigned char);

		for (i = 0; i != N; i++)
			tmp[i] = (diff_grid[i] == 0 ? 255 : 0);
		bzsimSaveDf3(tmp, NX, NY, NZ, BZSIM_DATA_BYTE, 0, 0, "%s/grid.df3", OUTPUT_DIR);
		XFREE(tmp);
	}
}

static void     do_pacemaker(float *x, float *y, float *r, int pixradius)
{
	int             xi, yi, zi = 2;
	int             x0 = NX/2, y0 = NY/2;
	
	for (yi = 1; yi != NY1; yi++) {
		for (xi = 1; xi != NX1; xi++) {
			int dx = xi - x0;
			int dy = yi - y0;
			
			if (sqrt(dx*dx + dy*dy) < pixradius)
				x[xi + yi * NX + zi * NXNY] = 1.0;
		}
	}
	
}

static void     do_step(float *x, float *y, float *r, float *xs, float *ys, float *rs)
{
	int             i, xi, yi, zi;
	float           tmp_x, tmp_y, tmp_r, tmp_xi, lap_x, lap_r;

	/* boundary conditions */
	switch (BOUNDARY) {
	case 1:
		bzsimSinkBC3D(NX, NY, NZ, 3, x, y, r);
		break;
	case 2:
		bzsimNoFluxBC3D(NX, NY, NZ, 3, x, y, r);
		break;
	default:
		/* bzsimPBC3D(NX, NY, NZ, 3, x, y, r); */
		bzsimPanic("undefined boundary type %d", BOUNDARY);
		break;
	}			/* end switch(BOUNDARY) */

	if (PACEMAKER_RADIUS > 0)
		do_pacemaker(x, y, r, PACEMAKER_RADIUS);

	for (i = 0; i != N; i++) {
		switch (MODEL) {
		case 4:
			/* Special case for 'diffusion spots' -- see bz2d.c */
			if (diff_grid[i])
				r[i] = (float) 1;
			break;
		default:
			if (diff_grid[i]) {
				r[i] = (float) 1;
			} else /*if (MODEL != 3)*/ {
				r[i] = (float) 0;
			}
			break;
		} /* end switch(MODEL) */
	}

	/* Calculate each grid point */
	for (zi = 1; zi != NZ1; zi++) {
		for (yi = 1; yi != NY1; yi++) {
			for (xi = 1; xi != NX1; xi++) {
				i = xi + yi * NX + zi * NXNY;
				/* seven-point Laplacian mask */
				lap_x = x[i - 1] + x[i + 1] + x[i + NX] + x[i - NX] + x[i + NXNY] + x[i - NXNY] - 6.0 * x[i];
				lap_r = r[i - 1] + r[i + 1] + r[i + NX] + r[i - NX] + r[i + NXNY] + r[i - NXNY] - 6.0 * r[i];
				/* dynamics */
				tmp_xi = x[i] * r[i];
				tmp_x = tmp_xi + dt_o_tf_eps * (tmp_xi * (1.0 - tmp_xi) - (tf_f * y[i] * (tmp_xi - tf_q)) / (tmp_xi + tf_q)) + lap_x * dt_o_dx2;
				tmp_y = y[i] + dt * (tmp_xi - y[i]);
				/* diffusion of reactants */
				tmp_r = r[i] + D_r * dt_o_dx2 * lap_r;
				/*
				 * Check for negative concentrations. Not very scientific but
				 * may help to overcome some numerical instabilities.
				 */
				xs[i] = (tmp_x < 0 ? (float) 0 : tmp_x);
				ys[i] = (tmp_y < 0 ? (float) 0 : tmp_y);
				rs[i] = (tmp_r < 0 ? (float) 0 : tmp_r);
			}	/* end xi */
		}		/* end yi */
	}			/* end zi */
}				/* end do_step */


int             main(int argc, char *argv[])
{
	int             t;
	char           *parfile, fname[1024], log[1024];

#if 0 /* test code */
	{
#define MY_X 5
#define MY_Y 5
#define MY_Z 5
		int i, xi, yi, zi;
		float *x = XALLOC(MY_X*MY_Y*MY_Z, float);

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
#endif /* test code */

	if (argc != 2) {
		fprintf(stderr, "usage: %s <parameter file>\n", argv[0]);
		exit(1);
	}

	bzsimInit(argv[0]);	/* application log file becomes bz3d2.log */
	/* read in the model parameters */
	my_get_parameters(parfile = argv[1]);
	my_check_parameters();

	/* create directory if it doesn't exists */
	bzsimMkdir(OUTPUT_DIR);

	/* make copy of the parameter file */
	sprintf(fname, "%s/%s.input", OUTPUT_DIR, SIMULATION_NAME);
	bzsimCopyFile(parfile, fname);

	/* log specific to this simulation */
	sprintf(log, "%s/%s.log", OUTPUT_DIR, SIMULATION_NAME);
	bzsimTruncateFile(log);
	my_init_params(log);
	my_save_parameters(log);

	if (XT_PLOT)
		bzsimTruncateFile("%s/xt.dat", OUTPUT_DIR);

	/* allocate memory for arrays */
	X1 = XALLOC(N, float);
	Y1 = XALLOC(N, float);
	X2 = XALLOC(N, float);
	Y2 = XALLOC(N, float);
	R1 = XALLOC(N, float);
	R2 = XALLOC(N, float);
	diff_grid = XALLOC(N, char);
	bzsimLogMsg(log, "float array bulk space: %d bytes\n", (int) (6 * N * sizeof(float)));
	bzsimLogMsg(log, "char array bulk space: %d bytes\n", (int) (1 * N * sizeof(float)));
	my_init_grid(log);
	bzsimSavePid(OUTPUT_DIR);

	/* T I M E     L O O P     S T A R T S  */
	bTurn = 0;
	t = 0;
	while (++t <= ITERATIONS) {
		/* Switch pointers according to flag bTurn. */
		bTurn = !bTurn;
		if (bTurn)
			do_step(X1, Y1, R1, X2, Y2, R2);
		else
			do_step(X2, Y2, R2, X1, Y1, R1);

		if (SNAP > 0 && t >= SNAP_START && (t % SNAP) == 0) {
			bzsimGridSnap(X1, NX, NY, NZ, BZSIM_FLOAT, BZSIM_Z, NZ/2, SCAL_MIN, SCAL_MAX, "%s/%010d.jpg", OUTPUT_DIR, t);
		}
		if (SNAP_DF3 > 0 && t >= SNAP_START && (t % SNAP_DF3) == 0) {
			if (BZSIM_OK != bzsimSaveDf3(X1, NX, NY, NZ, BZSIM_DATA_FLOAT, SCAL_MIN, SCAL_MAX, "%s/%010d.df3", OUTPUT_DIR, t))
				bzsimPanic("main: writing DF3 failed %d", t);
		}
		if (XT_PLOT > 0 && (t % XT_PLOT) == 0)
			save_xt(X1, NX, NY, NZ);
	}			/* end TIME LOOP */

	/* Free the allocated memory. */
	XFREE(X1);
	XFREE(X2);
	XFREE(Y1);
	XFREE(Y2);
	XFREE(R1);
	XFREE(R2);

	/* Timing info */
	bzsimSetEndTime();
	bzsimLogMsg(log, "elapsed time %d seconds\n", bzsimGetElapsedTime());

	if (XT_PLOT) {
		char           buf[1024];
		int            sz = ITERATIONS/XT_PLOT;

		sprintf(buf, "convert -depth 8 -size %dx%d gray:%s/xt.dat PNG:%s/xt.png", NZ, sz, OUTPUT_DIR, OUTPUT_DIR);
		system(buf);
	}

	return 0;
}

