/* $Id: fkn.c,v 1.11 2005/08/26 11:49:58 petterik Exp $ */

/****h* bz3d2/bz3d2.c
 * NAME
 *  bz3d2.c
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
static float *X1, *X2, *X3, *X4, *X5, *X12, *X22, *X32, *X42, *X52;
static float dt, dx, dt_o_dx2;
static char *diff_grid;
static char bTurn;
static char OUTPUT_DIR[MAX_PAR_LEN];
static char SIMULATION_NAME[MAX_PAR_LEN];
static int RANDOMSEED = 0;
static int PACEMAKER_RADIUS = 0;
static float P_SRC = 0;
static int XT_PLOT = 0;
static float xmax = FLT_MIN;
static float max[5];

/* FKN */
/* http://www-aig.jpl.nasa.gov/public/mls/cellerator/notebooks/Oregonator.html */
float H = 0;
float F = 0; /* 2h */
float k1c, k2c, k3c, k4c, k5c, K1, K2, K3, K4, K5;
float x1_ini, x2_ini, x3_ini, x4_ini, x5_ini;
float D_1, D_2, D_3, D_4, D_5;

/* internal parameters specific to this model */
float F2;
float dx2;
static float dc1, dc2, dc3, dc4, dc5;

static float xscalo(float val, float maxval)
{
	if (val > maxval)
		return 1;
	return val/maxval;
}

static void xdot(float t, float *x, float *xout, void *data)
{
	float kinet1 = K1 * x[0] * x[4];
	float kinet2 = K2 * x[2] * x[4];
	float kinet3 = K3 * x[0] * x[2];
	float kinet4 = K4 * x[2] * x[2];
	float kinet5 = K5 * x[1] * x[3];
	 

	/*
	  See (M1)-(M5) from:
	  author="J.J.~Tyson and P.C.~Fife",
	  year="1980",
	  title="Target patterns in a realistic model of the
	  {Belousov--Zhabotinskii} reaction",
	  journal="J.~Chem. Phys.",
	  volume="73",
	  pages="2224--2237"
	*/

	/* A: BrO3- */
	xout[0] = -kinet1 - kinet3 + kinet4;

	/* B: bromomalonic acid */
	xout[1] = -kinet5;

	/* U: acticator HBrO2 */
	xout[2] = kinet1 - kinet2 + kinet3 - 2 * kinet4;

	/* V: catalyst i.e Ce */
	xout[3] = 2 * kinet3 - kinet5;

	/* W: inhibitor Br- */
	xout[4] = -kinet1 - kinet2 + F2 * kinet5;
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
	GET_FLOAT(fname, "D_1", D_1);
	GET_FLOAT(fname, "D_2", D_2);
	GET_FLOAT(fname, "D_3", D_3);
	GET_FLOAT(fname, "D_4", D_4);
	GET_FLOAT(fname, "D_5", D_5);
	GET_FLOAT(fname, "x1_ini", x1_ini);
	GET_FLOAT(fname, "x2_ini", x2_ini);
	GET_FLOAT(fname, "x3_ini", x3_ini);
	GET_FLOAT(fname, "x4_ini", x4_ini);
	GET_FLOAT(fname, "x5_ini", x5_ini);
	GET_FLOAT(fname, "F", F);
	GET_FLOAT(fname, "H", H);
	GET_FLOAT(fname, "k1c", k1c);
	GET_FLOAT(fname, "k2c", k2c);
	GET_FLOAT(fname, "k3c", k3c);
	GET_FLOAT(fname, "k4c", k4c);
	GET_FLOAT(fname, "k5c", k5c);
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
	LOG_PAR(fname, "D_1 stability", 2.0 * D_1 * dt / (dx * dx));
	LOG_PAR(fname, "D_2 stability", 2.0 * D_2 * dt / (dx * dx));
	LOG_PAR(fname, "D_3 stability", 2.0 * D_3 * dt / (dx * dx));
	LOG_PAR(fname, "D_4 stability", 2.0 * D_4 * dt / (dx * dx));
	LOG_PAR(fname, "D_5 stability", 2.0 * D_5 * dt / (dx * dx));
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
	LOG_PAR(fname, "D_1", D_1);
	LOG_PAR(fname, "D_2", D_2);
	LOG_PAR(fname, "D_3", D_3);
	LOG_PAR(fname, "D_4", D_4);
	LOG_PAR(fname, "D_5", D_5);
	LOG_PAR(fname, "x1_ini", x1_ini);
	LOG_PAR(fname, "x2_ini", x2_ini);
	LOG_PAR(fname, "x3_ini", x3_ini);
	LOG_PAR(fname, "x4_ini", x4_ini);
	LOG_PAR(fname, "x5_ini", x5_ini);
	LOG_PAR(fname, "F", F);
	LOG_PAR(fname, "H", H);
	LOG_PAR(fname, "dx2", dx2);
	LOG_PAR(fname, "k1c", k1c);
	LOG_PAR(fname, "k2c", k2c);
	LOG_PAR(fname, "k3c", k3c);
	LOG_PAR(fname, "k4c", k4c);
	LOG_PAR(fname, "k5c", k5c);
	LOG_PAR(fname, "K1", K1);
	LOG_PAR(fname, "K2", K2);
	LOG_PAR(fname, "K3", K3);
	LOG_PAR(fname, "K4", K4);
	LOG_PAR(fname, "K5", K5);
}

static void init_params(char *log)
{
	/* Internal numerical parameters */
	if (MODEL == 5) 
		NY = NX = 1;

	N = NX * NY * NZ;
	NX1 = NX - 1;
	NY1 = NY - 1;
	NZ1 = NZ - 1;
	NXNY = NX * NY;
	dt = (L * L) / (((float) NPTS) * ((float) NPTS));
	dt /= (float) TIMESTEP_SAFETY;
	dx = L / (float) NPTS;
	dx2 = dx * dx;
	dt_o_dx2 = dt / dx2;
	dc1 = D_1 * dt_o_dx2;
	dc2 = D_2 * dt_o_dx2;
	dc3 = D_3 * dt_o_dx2;
	dc4 = D_4 * dt_o_dx2;
	dc5 = D_5 * dt_o_dx2;

	K1 = k1c * H * H;
	K2 = k2c * H;
	K3 = k3c * H;
	K4 = k4c;
	K5 = k5c;

	F2 = F / 2;

	/* these values are obtained by 'make pointx' */
	max[0] = 0.330000;
	max[1] = 0.320000;
	max[2] = 0.001005;
	max[3] = 0.032879;
	max[4] = 0.004490;
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
				case 5:
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
				case 5:
					/* catalyst is uniform in space and does not diffuse */
					i = zi;
					X1[i] = X12[i] = 0;
					X2[i] = X22[i] = 0;
					X3[i] = X32[i] = 0;
					X4[i] = X42[i] = x4_ini;
					X5[i] = X52[i] = 0;
					break;
				default:
					bzsimPanic("undefined model: %d", MODEL);
					break;
				} /* end switch(MODEL) */
			}					/* end xi */
		}						/* end yi */
	}							/* end zi */
}

static void do_step(float t, 
					float *x1, float *x2, float *x3, float *x4, float *x5,
					float *x1s, float *x2s, float *x3s, float *x4s, float *x5s)
{
	int i, zi;
	float xin[5], xout[5];

	switch (MODEL) {
	case 5:
		/* 1D spatial model */
		/* left boundary is open to reactants */
		x1[0] = x1[1] = x1_ini;
		x2[0] = x2[1] = x2_ini;
		x3[0] = x3[1] = x3_ini;
		x4[0] = x4[1] = x4_ini;
		x5[0] = x5[1] = x5_ini;
#if 0
		/* right open */
		x1[NZ1] = x1[NZ1 - 1] = x1_ini;
		x2[NZ1] = x2[NZ1 - 1] = x2_ini;
		x3[NZ1] = x3[NZ1 - 1] = x3_ini;
		x4[NZ1] = x4[NZ1 - 1] = x4_ini;
		x5[NZ1] = x5[NZ1 - 1] = x5_ini;
#else
		/* right closed */
		x1[NZ1] = x1[NZ1 - 1];
		x2[NZ1] = x2[NZ1 - 1];
		x3[NZ1] = x3[NZ1 - 1];
		x4[NZ1] = x4[NZ1 - 1];
		x5[NZ1] = x5[NZ1 - 1];
#endif

		break;
	default:
		bzsimPanic("undefined model type: %d", MODEL);
		break;
	}					/* end switch(MODEL) */

	/* MODEL == 5 -- 1D spatial model */
	for (zi = 1; zi != NZ1; zi++) {
		/* reactive term */
		
		xin[0] = x1[zi];
		xin[1] = x2[zi];
		xin[2] = x3[zi];
		xin[3] = x4[zi];
		xin[4] = x5[zi];

		bzsimRK4(t, xin, xout, dt, 5, NULL, xdot);
					
		/* diffusion term */
		xout[0] += dc1 * LAPL1D(x1, zi);
		xout[1] += dc2 * LAPL1D(x2, zi);
		xout[2] += dc3 * LAPL1D(x3, zi);
		xout[3] += dc4 * LAPL1D(x4, zi);
		xout[4] += dc5 * LAPL1D(x5, zi);

		for (i = 0; i != 5; i++)
			assert(finitef(xout[i]));
		
		/* Check for negative concentrations. */
		x1s[zi] = (xout[0] < 0 ? 0 : xout[0]);
		x2s[zi] = (xout[1] < 0 ? 0 : xout[1]);
		x3s[zi] = (xout[2] < 0 ? 0 : xout[2]);
		x4s[zi] = (xout[3] < 0 ? 0 : xout[3]);
		x5s[zi] = (xout[4] < 0 ? 0 : xout[4]);

		if (x3s[zi] > xmax)
			xmax = x3s[zi];
	}
} /* end do_step */

int main(int argc, char *argv[])
{
	int t = 0;
	char *parfile, fname[1024], log[1024];

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
		float x0[5], xout[5];
		int i;

		for (i = 0; i != 5; i++)
			max[i] = -1;

		x0[0] = x1_ini;
		x0[1] = x2_ini;
		x0[2] = x3_ini;
		x0[3] = x4_ini;
		x0[4] = x5_ini;

		while (t*dt < 2000.0) {
			bzsimRK4(t*dt, x0, xout, dt, 5, NULL, xdot);
			
			for (i = 0; i != 5; i++)
				if (xout[i] > max[i])
					max[i] = xout[i];

			if ((t % SNAP) == 0)
				printf("%f %f %f %f %f %f\n", t*dt, xout[0], xout[1], 
					   xout[2], xout[3], xout[4]);
			memcpy(x0, xout, 5 * sizeof(float));
			t++;
		}

		for (i = 0; i != 5; i++)
			fprintf(stderr, "max[%d] = %f\n", i, max[i]);

		exit(0);
	}

	if (XT_PLOT) {
		bzsimTruncateFile("%s/xt.dat", OUTPUT_DIR);
		bzsimTruncateFile("%s/at.dat", OUTPUT_DIR);
	}

	/* allocate memory for arrays */
	X1 = XALLOC(N, float);
	X2 = XALLOC(N, float);
	X3 = XALLOC(N, float);
	X4 = XALLOC(N, float);
	X5 = XALLOC(N, float);
	X12 = XALLOC(N, float);
	X22 = XALLOC(N, float);
	X32 = XALLOC(N, float);
	X42 = XALLOC(N, float);
	X52 = XALLOC(N, float);
	diff_grid = XALLOC(N, char);
	bzsimLogMsg(log, "float array bulk space: %d bytes\n",
				(int) (10 * N * sizeof(float)));
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
			do_step(tim, X1, X2, X3, X4, X5, X12, X22, X32, X42, X52);
		else
			do_step(tim, X12, X22, X32, X42, X52, X1, X2, X3, X4, X5);

		if (SNAP > 0 && t >= SNAP_START && (t % SNAP) == 0) {
			bzsimGridSnap(X1, NX, NY, NZ, BZSIM_FLOAT, BZSIM_Z, NZ / 2, 
						  SCAL_MIN, SCAL_MAX, "%s/%010d.jpg", OUTPUT_DIR, t);
			bzsimLogMsg(log, "t = %10.5f %s/%010d.jpg\n", tim, OUTPUT_DIR, t);
		}

		if (SNAP_DF3 > 0 && t >= SNAP_START && (t % SNAP_DF3) == 0) {
			if (BZSIM_OK != bzsimSaveDf3(X1, NX, NY, NZ, BZSIM_DATA_FLOAT,
										 SCAL_MIN, SCAL_MAX, "%s/%010d.df3",
										 OUTPUT_DIR, t))
				bzsimPanic("main: writing DF3 failed %d", t);
		}

		if (XT_PLOT > 0 && (t % XT_PLOT) == 0) {
			save_xt("xt", X3, SCAL_MAX, NX, NY, NZ);
			save_xt("at", X1, x1_ini, NX, NY, NZ);

			if (5 == MODEL) {
				static int idx = 0;
				FILE *fp;
				
				fp = fopen("sim1anim.tmp", "w");
				if (fp) {
					int i;
					char cmd[64];

					for (i = 0; i != NZ; i++)						
						fprintf(fp, "%d %f %f %f %f %f\n",
								i,
								xscalo(X1[i], max[0]),
								xscalo(X2[i], max[1]),
								xscalo(X3[i], max[2]),
								xscalo(X4[i], max[3]),
								xscalo(X5[i], max[4]));
					
					fclose(fp);

					system("gnuplot sim1anim.gp");
					sprintf(cmd, "mv -f tmp.png %s/%010d.png",
							OUTPUT_DIR, idx);
					system(cmd);

					sprintf(cmd, "mv -f sim1anim.tmp %s/%010d.dat",
							OUTPUT_DIR, idx);
					system(cmd);
					idx++;
				}
			}
		}

#if 1
		if (0 == (t % 1000))
			printf("%6.2f%% %f %f %f %f %f %f\n",
				   100.0 * t/(float) ITERATIONS,
				   t*dt, X1[N/2], X2[N/2], X3[N/2], 
				   X4[N/2], X5[N/2]);
#endif

	}							/* end TIME LOOP */

	/* Free the allocated memory. */
	XFREE(X1);
	XFREE(X2);
	XFREE(X3);
	XFREE(X4);
	XFREE(X5);
	XFREE(X12);
	XFREE(X22);
	XFREE(X32);
	XFREE(X42);
	XFREE(X52);
	XFREE(diff_grid);

	bzsimLogMsg(log, "end time %f\n", dt * ITERATIONS);
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
