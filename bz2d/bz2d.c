/* $Id: bz2d.c,v 1.9 2006/05/20 17:14:12 petterik Exp $ */

/****h* bz2d/bz2d.c
 * NAME
 *  bz2d.c
 * DESCRIPTION
 *  2D BZ simulation using bzsim static library.
 *  
 *  This study is published in journal article: Petteri Kettunen, Takashi 
 *  Amemiya, Takao Ohmori and Tomohiko Yamaguchi, "Spontaneous spiral
 *  formation in two-dimensional oscillatory media," Physical Review E 60, 
 *  1512-1515, 1999, DOI 10.1103/PhysRevE.60.1512
 *  http://link.aps.org/abstract/PRE/v60/p1512
 * USES
 *  bzsim
 *********/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>

#include "bzsim.h"

/****v* bz2d.c/N
 *  NAME
 *    N
 *  SYNOPSIS
 *    int N = 0, NX = 128, NY = 128;
 *  SOURCE
*/
int             N = 0, NX = 128, NY = 128;
/************ N */

/****v* bz2d.c/NX1
 *  NAME
 *    NX1
 *  SYNOPSIS
 *    int NX1, NY1, NXNY;
 *  SOURCE
 */
int             NX1, NY1, NXNY;
/************ NX1 */

/****v* bz2d.c/SNAP
 *  NAME
 *    SNAP
 *  SYNOPSIS
 *    int SNAP
 *  SOURCE
 */
int             SNAP, ITERATIONS;
/*******/

/****v* bz2d.c/X_GLOBAL_MAX
 *  NAME
 *    X_GLOBAL_MAX
 *  SYNOPSIS
 *    float X_GLOBAL_MAX;
 *  SOURCE
 */
float           X_GLOBAL_MAX;
/************ X_GLOBAL_MAX */

/****v* bz2d.c/L
 *  NAME
 *    L
 *  SYNOPSIS
 *    float L;
 *  SOURCE
*/
float           L;
/************ L */

/****v* bz2d.c/one_m_dt
 *  NAME
 *    one_m_dt
 *  SYNOPSIS
 *    float one_m_dt;
 *  SOURCE
*/
float           one_m_dt;
/************ one_m_dt */

/****v* bz2d.c/one_o_a
 *  NAME
 *    one_o_a
 *  SYNOPSIS
 *    float one_o_a;
 *  SOURCE
*/
float           one_o_a;
/************ one_o_a */

/****v* bz2d.c/TIMESTEP_SAFETY
 *  NAME
 *    TIMESTEP_SAFETY
 *  SYNOPSIS
 *    float TIMESTEP_SAFETY;
 *  SOURCE
*/
float           TIMESTEP_SAFETY;
/************ TIMESTEP_SAFETY */

/****v* bz2d.c/D_r
 *  NAME
 *    D_r
 *  SYNOPSIS
 *    float D_r;
 *  SOURCE
*/
float           D_r;
/************ D_r */

/****v* bz2d.c/b_o_a
 *  NAME
 *    b_o_a
 *  SYNOPSIS
 *    float b_o_a;
 *  SOURCE
*/
float           b_o_a;
/************ b_o_a */

/****v* bz2d.c/eps
 *  NAME
 *    eps
 *  SYNOPSIS
 *    float eps = (float) 0.02;
 *  SOURCE
*/
float           eps = (float) 0.02;
/************ eps */

/****v* bz2d.c/DIFFUSION_SPOT_DENSITY
 *  NAME
 *    DIFFUSION_SPOT_DENSITY
 *  SYNOPSIS
 *    float DIFFUSION_SPOT_DENSITY = (float) 0;
 *  SOURCE
*/
float           DIFFUSION_SPOT_DENSITY = (float) 0;
/************ DIFFUSION_SPOT_DENSITY */

/****v* bz2d.c/NPTS
 *  NAME
 *    NPTS
 *  SYNOPSIS
 *    int NPTS;
 *  SOURCE
*/
int             NPTS;
/************ NPTS */

/****v* bz2d.c/spot_cnt
 *  NAME
 *    spot_cnt
 *  SYNOPSIS
 *    int spot_cnt = 0;
 *  SOURCE
*/
int             spot_cnt = 0;
/************ spot_cnt */

/****v* bz2d.c/BOUNDARY
 *  NAME
 *    BOUNDARY
 *  SYNOPSIS
 *    int BOUNDARY = 0;
 *  SOURCE
*/
int             BOUNDARY = 0;	/* 0 = pbc, 1 = sink */
/************ BOUNDARY */

/****v* bz2d.c/MODEL
 *  NAME
 *    MODEL
 *  SYNOPSIS
 *    int MODEL = 0;
 *  SOURCE
*/
int             MODEL = 0;
/************ MODEL */

/****v* bz2d.c/tf_eps
 *  NAME
 *    tf_eps
 *  SYNOPSIS
 *    float tf_f = (float) 1.25, tf_q = (float) 0.002, tf_eps = (float) 0.05;
 *  SOURCE
*/
float           tf_f = (float) 1.25, tf_q = (float) 0.002, tf_eps = (float) 0.05;
/************ tf_eps */

/****v* bz2d.c/CALCULATE_MEAN_R
 *  NAME
 *    CALCULATE_MEAN_R
 *  SYNOPSIS
 *    int CALCULATE_MEAN_R = 0;
 *  SOURCE
*/
int             CALCULATE_MEAN_R = 0;
/************ CALCULATE_MEAN_R */

/****v* bz2d.c/SAVE_JPG
 *  NAME
 *    SAVE_JPG
 *  SYNOPSIS
 *    int SAVE_R = 0, SAVE_RGB = 0, SAVE_PGM = 0, SAVE_JPG = 1;
 *  SOURCE
*/
int             SAVE_R = 0, SAVE_RGB = 0, SAVE_PGM = 0, SAVE_JPG = 1;
/************ SAVE_JPG */

/****v* bz2d.c/JPEG_COMPRESSION
 *  NAME
 *    JPEG_COMPRESSION
 *  SYNOPSIS
 *    int JPEG_COMPRESSION = 100;
 *  SOURCE
*/
int             JPEG_COMPRESSION = 100;
/************ JPEG_COMPRESSION */

/****v* bz2d.c/SCAL_MAX
 *  NAME
 *    SCAL_MAX
 *  SYNOPSIS
 *    float SCAL_MIN = (float) 0, SCAL_MAX = (float) 0;
 *  SOURCE
*/
float           SCAL_MIN = (float) 0, SCAL_MAX = (float) 0;
/************ SCAL_MAX */

/****v* bz2d.c/SNAP_START
 *  NAME
 *    SNAP_START
 *  SYNOPSIS
 *    int SNAP_START = 0;
 *  SOURCE
*/
int             SNAP_START = 0;
/************ SNAP_START */

/****v* bz2d.c/SAVE_GRID
 *  NAME
 *    SAVE_GRID
 *  SYNOPSIS
 *    int SAVE_GRID = 0;
 *  SOURCE
*/
int             SAVE_GRID = 0;
/************ SAVE_GRID */

/****v* bz2d.c/r1
 *  NAME
 *    r1
 *  SYNOPSIS
 *    float *x1, *y_1, *r1, *x2, *y2, *r2, *tmp_ptr;
 *  SOURCE
*/
float          *x1, *y_1, *r1, *x2, *y2, *r2, *tmp_ptr;
/************ r1 */

/****v* bz2d.c/dt
 *  NAME
 *    dt
 *  SYNOPSIS
 *    float dt, dx, dt_o_dx2, h, dt_o_eps;
 *  SOURCE
*/
float           dt, dx, dt_o_dx2, h, dt_o_eps;
/************ dt */

/****v* bz2d.c/p1
 *  NAME
 *    p1
 *  SYNOPSIS
 *    float phai, p1, p2;
 *  SOURCE
*/
float           phai, p1, p2;
/************ p1 */

/****v* bz2d.c/x_ss
 *  NAME
 *    x_ss
 *  SYNOPSIS
 *    float xyss, x_ss, y_ss;
 *  SOURCE
*/
float           xyss, x_ss, y_ss;
/************ x_ss */

/****v* bz2d.c/dt_o_tf_eps
 *  NAME
 *    dt_o_tf_eps
 *  SYNOPSIS
 *    float dt_o_tf_eps;
 *  SOURCE
*/
float           dt_o_tf_eps;
/************ dt_o_tf_eps */

/****v* bz2d.c/bTurn
 *  NAME
 *    bTurn
 *  SYNOPSIS
 *    char *diff_grid, bTurn;
 *  SOURCE
*/
char           *diff_grid, bTurn;
/************ bTurn */

/****v* bz2d.c/OUTPUT_DIR
 *  NAME
 *    OUTPUT_DIR
 *  SYNOPSIS
 *    char OUTPUT_DIR[MAX_PAR_LEN];
 *  SOURCE
*/
char            OUTPUT_DIR[MAX_PAR_LEN];
/************ OUTPUT_DIR */

/****v* bz2d.c/SIMULATION_NAME
 *  NAME
 *    SIMULATION_NAME
 *  SYNOPSIS
 *    char SIMULATION_NAME[MAX_PAR_LEN];
 *  SOURCE
*/
char            SIMULATION_NAME[MAX_PAR_LEN];
/************ SIMULATION_NAME */

float R = 0;

#define CALC_MIN_MAX

#ifdef CALC_MIN_MAX
float xmax = -1e22, ymax = -1e22;
#endif

int SNAP_COLOUR = 0;

/****f* bz2d.c/my_get_parameters
 *  NAME
 *    my_get_parameters
 *  SYNOPSIS
 *    static void my_get_parameters(char *fname)
 *  SOURCE
 */

static void     my_get_parameters(char *fname)
{
	GET_INT(fname, "MODEL", MODEL);
	GET_INT(fname, "SNAP", SNAP);
	GET_INT(fname, "SNAP_COLOUR", SNAP_COLOUR);
	GET_INT(fname, "ITERATIONS", ITERATIONS);
	GET_STRING(fname, "OUTPUT_DIR", OUTPUT_DIR);
	GET_STRING(fname, "SIMULATION_NAME", SIMULATION_NAME);
	GET_INT(fname, "NX", NX);
	GET_INT(fname, "NY", NY);
	GET_FLOAT(fname, "L", L);
	GET_FLOAT(fname, "EPS", eps);
	GET_FLOAT(fname, "TIMESTEP_SAFETY", TIMESTEP_SAFETY);
	GET_FLOAT(fname, "D_r", D_r);
	GET_INT(fname, "NPTS", NPTS);
	GET_FLOAT(fname, "TF_F", tf_f);
	GET_FLOAT(fname, "TF_Q", tf_q);
	GET_FLOAT(fname, "TF_EPS", tf_eps);
	GET_INT(fname, "BOUNDARY", BOUNDARY);
	GET_FLOAT(fname, "DIFFUSION_SPOT_DENSITY", DIFFUSION_SPOT_DENSITY);
	GET_INT(fname, "CALCULATE_MEAN_R", CALCULATE_MEAN_R);
	GET_INT(fname, "SAVE_RGB", SAVE_RGB);
	GET_INT(fname, "SAVE_PGM", SAVE_PGM);
	GET_INT(fname, "SAVE_JPG", SAVE_JPG);
	GET_INT(fname, "SAVE_R", SAVE_R);
	GET_INT(fname, "JPEG_COMPRESSION", JPEG_COMPRESSION);
	GET_INT(fname, "SNAP_START", SNAP_START);
	GET_FLOAT(fname, "SCAL_MIN", SCAL_MIN);
	GET_FLOAT(fname, "SCAL_MAX", SCAL_MAX);
	GET_FLOAT(fname, "R", R);
}

/************ my_get_parameters */

/****f* bz2d.c/my_save_parameters
 *  NAME
 *    my_save_parameters
 *  SYNOPSIS
 *    void my_save_parameters(char *fname)
 *  SOURCE
*/
void            my_save_parameters(char *fname)
{
	LOG_PAR(fname, "MODEL", MODEL);
	LOG_PAR(fname, "NX", NX);
	LOG_PAR(fname, "NY", NY);
	LOG_PAR(fname, "N", N);
	LOG_PAR(fname, "L", L);
	LOG_PAR(fname, "dt", dt);
	LOG_PAR(fname, "dt-safety", TIMESTEP_SAFETY);
	LOG_PAR(fname, "dx", dx);
	LOG_PAR(fname, "eps", eps);
	LOG_PAR(fname, "snapshot", SNAP);
	LOG_PAR(fname, "x_ss/y_ss", x_ss);
	LOG_PAR(fname, "dt/dx2", dt_o_dx2);
	LOG_PAR(fname, "BOUNDARY", BOUNDARY);
	LOG_PAR(fname, "D_r", D_r);
	LOG_PAR(fname, "NPTS", NPTS);
	LOG_PAR(fname, "diffusion density", DIFFUSION_SPOT_DENSITY);
	LOG_PAR(fname, "TF_F", tf_f);
	LOG_PAR(fname, "TF_Q", tf_q);
	LOG_PAR(fname, "TF_EPS", tf_eps);
	LOG_PAR(fname, "CALCULATE_MEAN_R", CALCULATE_MEAN_R);
	LOG_PAR(fname, "SAVE_R", SAVE_R);
	LOG_PAR(fname, "JPEG quality level", JPEG_COMPRESSION);
	LOG_PAR(fname, "SCAL_MIN", SCAL_MIN);
	LOG_PAR(fname, "SCAL_MAX", SCAL_MAX);
	LOG_PAR(fname, "SNAP_START", SNAP_START);
	LOG_PAR(fname, "SNAP_COLOUR", SNAP_COLOUR);
	LOG_PAR(fname, "R", R);
}

/************ my_save_parameters */

/****f* bz2d.c/my_init_params
 *  NAME
 *    my_init_params
 *  SYNOPSIS
 *    static void my_init_params()
 *  SOURCE
 */

static void     my_init_params()
{
	/* Internal numerical parameters */
	N = NX * NY;
	NX1 = NX - 1;
	NY1 = NY - 1;
	NXNY = NX * NY;
	x_ss = y_ss = (float) 0;
	dt = (L * L) / (((float) NPTS) * ((float) NPTS));
	dt /= (float) TIMESTEP_SAFETY;
	dt_o_eps = dt / eps;
	one_m_dt = ((float) 1) - dt;
	dx = ((float) L) / ((float) NPTS);
	dt_o_dx2 = dt / (dx * dx);
	dt_o_tf_eps = dt / tf_eps;
}

/************ my_init_params */

/****f* bz2d.c/my_init_grid
 *  NAME
 *    my_init_grid
 *  SYNOPSIS
 *    static void my_init_grid()
 *  SOURCE
 */

static void     my_init_grid()
{
	int             xi, yi, i;

	/* diffusion spot grid */
	bzsimInitRand(0);
	for (yi = 0; yi != NY; yi++) {
		for (xi = 0; xi != NX; xi++) {
			i = xi + yi * NX;
			switch (MODEL) {
			case 3:
				/* "plane and dot" */
				if (5 > xi) {
					diff_grid[i] = 1;
				} else {
					int dx = NX/2 - xi, dy = NY/2 - yi;
					float r = sqrtf(dx*dx + dy*dy);
					diff_grid[i] = r <= R ? 1 : 0;
				}
				break;
			case 2:
				if (bzsimRand() < (xi / (float) NX)) {
					/* Diffusion spot */
					diff_grid[i] = 1;
					spot_cnt++;
				} else {
					diff_grid[i] = 0;
				}
				break;
			default:
				if (bzsimRand() < DIFFUSION_SPOT_DENSITY) {
					/* Diffusion spot */
					diff_grid[i] = 1;
					spot_cnt++;
				} else {
					diff_grid[i] = 0;
				}
				break;
			}	/* end switch(MODEL) */
		}
	}

	/* other variables */
	for (yi = 0; yi != NY; yi++) {
		for (xi = 0; xi != NX; xi++) {
			i = xi + yi * NX;
			x1[i] = x_ss;
			x2[i] = x_ss;
			y_1[i] = y_ss;
			y2[i] = y_ss;
			r1[i] = (float) 0;
			r2[i] = (float) 0;
			x_ss = y_ss = (tf_q * (tf_f + 1)) / (tf_f - 1);
			x1[i] = x2[i] = x_ss;
			y_1[i] = y2[i] = y_ss;
		}		/* end xi */
	}			/* end yi */
}

/************ my_init_grid */

/****f* bz2d.c/my_write_grid
 *  NAME
 *    my_write_grid
 *  SYNOPSIS
 *    static void my_write_grid(char *fname, unsigned char *buf, int nx, int ny)
 *  SOURCE
 */

static void     my_write_grid(char *fname, char *buf, int nx, int ny)
{
	int             i, n;
	unsigned char  *buf2;
	FILE *fp;
	char *cmd;

	n = nx * ny;
	buf2 = XALLOC(n, unsigned char);
	for (i = 0; i != n; i++) {
		buf2[i] = (buf[i] == 0 ? 255 : 0);
	}
	bzsimWriteJPEG(fname, buf2, NX, NY, BZSIM_BW, 100);

	/* Ugly Hack: change .jpg to .dat */
	n = strlen(fname);
	fname[n - 3] = 'd';
	fname[n - 2] = 'a';
	fname[n - 1] = 't';

	fp = fopen(fname, "w");
	fwrite(buf2, nx * ny * sizeof(unsigned char), 1, fp);
	fclose(fp);

	/* convert to pgm ignoring errors */
	cmd = XALLOC((64 + 2 * n), char);
	sprintf(cmd, "convert -depth 8 -size %dx%d gray:%s %s.png > /dev/null 2>&1",
			nx, ny, fname, fname);
	printf("%s\n", cmd);
	system(cmd);

	XFREE(buf2);
	XFREE(cmd);
}

/************ my_write_grid */

/****f* bz2d.c/do_step
 *  NAME
 *    do_step
 *  SYNOPSIS
 *    static void do_step(float *x, float *y, float *r, float *xs, float *ys, float *rs)
 *  SOURCE
 */

static void     do_step(float *x, float *y, float *r, float *xs, float *ys, float *rs)
{
	int             yi, xi, i;
	float           tmp_x, tmp_y, tmp_r, tmp_xi, lap_x, lap_r;

	/* boundary conditions */
	switch (BOUNDARY) {
	case 2:
		zero_flux_boundaries_2D(x, y, r, NX, NY);
		break;
	case 1:
		bzsimSinkBC2D(NX, NY, 3, x, y, r);
		break;
	default:
		bzsimPBC2D(NX, NY, 3, x, y, r);
		break;
	}			/* end switch(BOUNDARY) */

	/* Special case for 'diffusion spots' */
	for (i = 0; i != N; i++) {
		if (diff_grid[i] == 1) {
			r[i] = (float) 1;
		}
	}

	/* Calculate each grid point */
	for (yi = 1; yi != NY1; yi++) {
		for (xi = 1; xi != NX1; xi++) {
			i = xi + yi * NX;
			lap_x = x[i - 1] + x[i + 1] + x[i + NX] + x[i - NX] - 4.0 * x[i];
			lap_r = r[i - 1] + r[i + 1] + r[i + NX] + r[i - NX] - 4.0 * r[i];
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

#ifdef CALC_MIN_MAX
			if (xs[i] > xmax)
				xmax = xs[i];
			if (ys[i] > ymax)
				ymax = ys[i];
#endif

		}		/* end xi */
	}			/* end yi */
}				/* end do_step */

/************ do_step */

inline static int pix(float val)
{
	if (val < 0)
		return 0;
	else if (val > 1.0)
		return 255;
	
	return (int) (255 * val);
}

/****f* bz2d.c/main
 *  NAME
 *    main
 *  SYNOPSIS
 *    int main(int argc, char *argv[])
 *  SOURCE
*/
int             main(int argc, char *argv[])
{
	int             t;
	char           *parfile, fname[1024], logfile[1024];

	if (argc != 2) {
		fprintf(stderr, "usage: %s <parameter file>\n", argv[0]);
		exit(1);
	}
	bzsimInit(argv[0]);	/* application log file becomes bz2d.log */
	/* read in the model parameters */
	my_get_parameters(parfile = argv[1]);
	/* create directory if it doesn't exists */
	bzsimMkdir(OUTPUT_DIR);
	/* make copy of the parameter file */
	sprintf(fname, "%s/%s.input", OUTPUT_DIR, SIMULATION_NAME);
	bzsimCopyFile(parfile, fname);
	/* logfile specific to this simulation */
	sprintf(logfile, "%s/%s.log", OUTPUT_DIR, SIMULATION_NAME);
	bzsimTruncateFile(logfile);
	my_init_params();
	my_save_parameters(logfile);
	/* allocate memory for arrays */
	x1 = XALLOC(N, float);
	y_1 = XALLOC(N, float);
	x2 = XALLOC(N, float);
	y2 = XALLOC(N, float);
	r1 = XALLOC(N, float);
	r2 = XALLOC(N, float);
	diff_grid = XALLOC(N, char);
	bzsimLogMsg(logfile, "float array bulk space: %d bytes\n", (int) (6 * N * sizeof(float)));
	bzsimLogMsg(logfile, "char array bulk space: %d bytes\n", (int) (1 * N * sizeof(float)));
	my_init_grid();
	sprintf(fname, "%s/%s_grid.jpg", OUTPUT_DIR, SIMULATION_NAME);
	my_write_grid(fname, diff_grid, NX, NY);
	if (3 != MODEL)
		bzsimLogMsg(logfile, "SPOTS %d / GRID POINTS %d = ACTUAL DENSITY %f\n",
					spot_cnt, NX * NY, ((float) spot_cnt) / (NX * NY));

	/* T I M E     L O O P     S T A R T S */
	bTurn = 0;
	t = 0;
	while (++t <= ITERATIONS) {
		/* switch pointers according to flag bTurn */
		bTurn = !bTurn;
		if (bTurn) {
			do_step(x1, y_1, r1, x2, y2, r2);
		} else {
			do_step(x2, y2, r2, x1, y_1, r1);
		}

		if (SNAP != 0 && t > SNAP_START && 0 == (t % SNAP)) {
			static int frame = 1;
			unsigned char *img = XALLOC(3 * NX * NY, unsigned char);
			int i, x, y, tmp;
			float scaled, red, green, blue;

#define XMAX 0.8
#define YMAX 0.3
#define XTHRESHOLD 0.50

			for (i = 0, y = 0; y != NY; y++) {
				for (x = 0; x != NX; x++) {
					int idx = x + y * NX;
			   
					switch (SNAP_COLOUR) {
						
					case 1:
						if (x1[idx] > XTHRESHOLD) {
							/* propagator = green */
							img[i++] = 0;
							img[i++] = pix(x1[idx]/XMAX);
							img[i++] = 0;
						} else { 
							/* this snippet is lifted from D Barkley's EZSpiral */
							scaled = y_1[idx] / YMAX;
							red   = 1.0 - scaled * scaled;
							green = scaled * scaled / 2.0;
							blue  = scaled * scaled;
								
							img[i++] = pix(red);
							img[i++] = pix(green);
							img[i++] = pix(blue);
						}
						break;

					case 0:
					default:
						/* propagator: excited is mapped to white */
						tmp = pix(x1[idx]/XMAX);						
						img[i++] = tmp;
						img[i++] = tmp;
						img[i++] = tmp;
						break;


					} /* end switch(SNAP_COLOUR) */
				}
			}

			sprintf(fname, "%s/%010d.jpg", OUTPUT_DIR, frame++);			
			bzsimLogMsg(logfile, "snapshot %s %f\n", fname, t * dt);				
			bzsimWriteJPEG(fname, img, NX, NY, 3, 100);				
			XFREE(img);

		} /* end SNAP */

		if (CALCULATE_MEAN_R && 0 == (t % CALCULATE_MEAN_R)) {
			int i;
			float rsum = 0;

			for (i = 0; i < N; i++)
				rsum += r1[i];

			bzsimLogMsg(logfile, "ravg %d %f\n", t, rsum/N);
		} /* end CALCULATE_MEAN_R */

	}			/* end TIME LOOP */

	/* free the allocated memory */
	XFREE(x1);
	XFREE(x2);
	XFREE(y_1);
	XFREE(y2);
	XFREE(r1);
	XFREE(r2);

#ifdef CALC_MIN_MAX
	bzsimLogMsg(logfile, "xmax = %f\nymax = %f\n", xmax, ymax);
#endif

	/* Timing info */
	bzsimSetEndTime();
	bzsimLogMsg(logfile, "elapsed time %d seconds\n", bzsimGetElapsedTime());

	return 0;
}

/************ main */
