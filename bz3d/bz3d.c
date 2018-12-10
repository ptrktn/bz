/* $Id: bz3d.c,v 1.7 2006/05/13 14:04:49 petterik Exp $ */

/****h* bz3d/bz3d.c
 *
 * NAME
 *  bz3d.c
 * DESCRIPTION
 *  3D BZ simulation using bzsim static library. This work is published in:
 *
 *    Petteri Kettunen, Paul D. Bourke, Hajime Hashimoto, Takashi Amemiya, 
 *    Stefan C. Mueller and Tomohiko Yamaguchi, "Computational study of helix
 *    wave formation in active media," Mathematical and Computer Modelling,
 *    volume 41, issue 8/9, 1013-1020, 2005 (doi:10.1016/j.mcm.2004.01.007).
 * 
 *  Multimedia files from this study can be viewed from:
 *    http://web.hpt.jp/petterik/
 ***** 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bzsim.h"

int             N = 0, NX = 128, NY = 128, NZ = 16;
int             NX1, NY1, NZ1, NXNY;
int             SNAP, SNAPZ, PRINT_TIME, UPDATE_GRAPHICS, ITERATIONS, SPIRAL,
	EXPLICIT, SPECIAL_BOUNDARIES, SAVE_DF3, SNAPZ_COMP;
float           X_GLOBAL_MAX;
float           L;
float           TIMESTEP_SAFETY;
float           D_r;
float           eps = (float) 0.02;
float           RANDOM_DENSITY;
float           R_NOISE = (float) 0;
float           DIFFUSION_SPOT_DENSITY = (float) 0;
int             NPTS;
int             spot_cnt = 0;
int             SHOW_R_FIELD = 0;
int             SPECIAL_IC = 0;
int             BOUNDARY = 0;	/* 0 = pbc, 1 = sink */
int             MODEL = 0;
int             R_BOUNDARIES = 0;
int             DIFFZMAXLEN = 0;
float           tf_f = (float) 1.25, tf_q = (float) 0.002, tf_eps = (float) 0.05;
int             SAVE_R = 0, SAVE_RGB = 0, SAVE_PGM = 0, SAVE_JPG = 1;
int             JPEG_COMPRESSION = 100;
float           SCAL_MIN = (float) 0, SCAL_MAX = (float) 0;
int             SNAP_START = 0;
int             SAVE_GRID = 0;
float          *x1, *y_1, *r1, *x2, *y2, *r2, *tmp_ptr;
float           dt, dx, dt_o_dx2, h, dt_o_eps;
float           phai, p1, p2;
float           xyss, x_ss, y_ss;
float           dt_o_tf_eps;
char           *diff_grid, bTurn;
char            OUTPUT_DIR[MAX_PAR_LEN];
char            SIMULATION_NAME[MAX_PAR_LEN];
int             RANDOMSEED = 0;
int             READ_GRID = 0;
char            GRID_FILE[1024];
float          *imagezn;
int             imagezn_n = 0, imagezn_samples = 10 /* 10 */;

static void     my_get_parameters(char *fname)
{
	GET_INT(fname, "MODEL", MODEL);
	GET_INT(fname, "SNAP", SNAP);
	GET_INT(fname, "SNAPZ", SNAPZ);
	GET_INT(fname, "SNAPZ_COMP", SNAPZ_COMP);
	GET_INT(fname, "ITERATIONS", ITERATIONS);
	GET_STRING(fname, "OUTPUT_DIR", OUTPUT_DIR);
	GET_STRING(fname, "SIMULATION_NAME", SIMULATION_NAME);
	GET_INT(fname, "UPDATE_GRAPHICS", UPDATE_GRAPHICS);
	GET_INT(fname, "READ_GRID", READ_GRID);
	GET_STRING(fname, "GRID_FILE", GRID_FILE);
	GET_INT(fname, "NX", NX);
	GET_INT(fname, "NY", NY);
	GET_INT(fname, "NZ", NZ);
	GET_FLOAT(fname, "L", L);
	GET_FLOAT(fname, "EPS", eps);
	GET_FLOAT(fname, "TIMESTEP_SAFETY", TIMESTEP_SAFETY);
	GET_FLOAT(fname, "D_r", D_r);
	GET_FLOAT(fname, "RANDOM_DENSITY", RANDOM_DENSITY);
	GET_INT(fname, "DIFFZMAXLEN", DIFFZMAXLEN);
	GET_INT(fname, "NPTS", NPTS);
	GET_INT(fname, "RANDOMSEED", RANDOMSEED);
	GET_FLOAT(fname, "TF_F", tf_f);
	GET_FLOAT(fname, "TF_Q", tf_q);
	GET_FLOAT(fname, "TF_EPS", tf_eps);
	GET_INT(fname, "BOUNDARY", BOUNDARY);
	GET_FLOAT(fname, "DIFFUSION_SPOT_DENSITY", DIFFUSION_SPOT_DENSITY);
	GET_INT(fname, "SAVE_RGB", SAVE_RGB);
	GET_INT(fname, "SAVE_PGM", SAVE_PGM);
	GET_INT(fname, "SAVE_JPG", SAVE_JPG);
	GET_INT(fname, "SAVE_GRID", SAVE_GRID);
	GET_INT(fname, "SAVE_R", SAVE_R);
	GET_INT(fname, "JPEG_COMPRESSION", JPEG_COMPRESSION);
	GET_INT(fname, "SNAP_START", SNAP_START);
	GET_FLOAT(fname, "SCAL_MIN", SCAL_MIN);
	GET_FLOAT(fname, "SCAL_MAX", SCAL_MAX);
	GET_INT(fname, "SAVE_DF3", SAVE_DF3);
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
	LOG_PAR(fname, "eps", eps);
	LOG_PAR(fname, "snapshot", SNAP);
	LOG_PAR(fname, "snapshot z-layer", SNAPZ);
	LOG_PAR(fname, "snapshot z-layer composite", SNAPZ_COMP);
	LOG_PAR(fname, "x_ss/y_ss", x_ss);
	LOG_PAR(fname, "dt/dx2", dt_o_dx2);
	LOG_PAR(fname, "BOUNDARY", BOUNDARY);
	LOG_PAR(fname, "BOUNDARIES", SPECIAL_BOUNDARIES);
	LOG_PAR(fname, "R_BOUNDARIES", R_BOUNDARIES);
	LOG_PAR(fname, "R_NOISE", R_NOISE);
	LOG_PAR(fname, "READ_GRID", READ_GRID);
	LOG_STR(fname, "GRID_FILE", GRID_FILE);
	LOG_PAR(fname, "D_r", D_r);
	LOG_PAR(fname, "random density", RANDOM_DENSITY);
	LOG_PAR(fname, "NPTS", NPTS);
	LOG_PAR(fname, "show r field", SHOW_R_FIELD);
	LOG_PAR(fname, "special initial conditions", SPECIAL_IC);
	LOG_PAR(fname, "diffusion density", DIFFUSION_SPOT_DENSITY);
	LOG_PAR(fname, "DIFFZMAXLEN", DIFFZMAXLEN);
	LOG_PAR(fname, "RANDOMSEED", RANDOMSEED);
	LOG_PAR(fname, "TF_F", tf_f);
	LOG_PAR(fname, "TF_Q", tf_q);
	LOG_PAR(fname, "TF_EPS", tf_eps);
	LOG_PAR(fname, "SAVE_R", SAVE_R);
	LOG_PAR(fname, "SAVE_DF3", SAVE_DF3);
	LOG_PAR(fname, "SAVE_GRID", SAVE_GRID);
	LOG_PAR(fname, "JPEG_COMPRESSION", JPEG_COMPRESSION);
	LOG_PAR(fname, "SCAL_MIN", SCAL_MIN);
	LOG_PAR(fname, "SCAL_MAX", SCAL_MAX);
	LOG_PAR(fname, "SNAP_START", SNAP_START);
}

static void     my_init_params(char *logfile)
{
	/* Internal numerical parameters */
	N = NX * NY * NZ;
	NX1 = NX - 1;
	NY1 = NY - 1;
	NZ1 = NZ - 1;
	NXNY = NX * NY;
	x_ss = y_ss = (float) 0;
	dt = (L * L) / (((float) NPTS) * ((float) NPTS));
	dt /= (float) TIMESTEP_SAFETY;
	dt_o_eps = dt / eps;
	dx = ((float) L) / ((float) NPTS);
	dt_o_dx2 = dt / (dx * dx);
	dt_o_tf_eps = dt / tf_eps;
}

static void     my_check_parameters(void)
{
	assert((DIFFZMAXLEN + 3) < NZ);
	assert(SNAPZ < NZ);
}

/* fill diffusion spot grid and set variable initial conditions */
static void     my_init_grid(char *logfile)
{
	int             xi, yi, zi, i;

	/* null grid */
	memset(diff_grid, 0, N);

	if (READ_GRID != 0) {
		bzsimReadData(NX, NY, NZ, BZSIM_DATA_BYTE, diff_grid, 0, 0, "%s", GRID_FILE);
	} else {
		/* RANDOMSEED is int, function returns unsigned int */
		bzsimLogMsg(logfile, "SEEDVALUE %d\nSEEDUSED %u\n", RANDOMSEED, bzsimInitRand(RANDOMSEED));
		for (zi = 0; zi != NZ; zi++) {
			for (yi = 0; yi != NY; yi++) {
				for (xi = 0; xi != NX; xi++) {
					i = xi + yi * NX + zi * NXNY;
					switch (MODEL) {
					case 3:
						/* fprintf(stderr, "%f\n", bzsimRand()); */
						/* z = 0: model gel roughness */
						if (zi == 0 && (bzsimRand() < DIFFUSION_SPOT_DENSITY)) {
							int             zi2,
							                zlen = (int) (floor((bzsimRand() * DIFFZMAXLEN) + 0.5));
							for (zi2 = 0; zi2 != zlen; zi2++) {
								diff_grid[xi + yi * NX + (0 + zi2) * NXNY] = 1;
								spot_cnt++;
							}
						}
						break;
					case 2:
						/*
						 * linear density gradient
						 * along x-axis
						 */
						if (bzsimRand() < (xi / (float) NX)) {
							/* Diffusion spot */
							diff_grid[i] = 1;
							spot_cnt++;
						} else {
							diff_grid[i] = 0;
						}
						break;
					case 1:
						/* random spatial */
						if (bzsimRand() < DIFFUSION_SPOT_DENSITY) {
							/* Diffusion spot */
							diff_grid[i] = 1;
							spot_cnt++;
						} else {
							diff_grid[i] = 0;
						}
						break;
					default:
						fprintf(stderr, "MODEL undefined (%d)\n", MODEL);
						exit(1);
						break;
					}	/* end switch(MODEL) */
				}	/* end xi */
			}	/* end yi */
		}		/* end zi */
	}			/* !READ_GRID */
	if (SAVE_GRID) {
		bzsimSaveData(NX, NY, NZ, BZSIM_DATA_BYTE, diff_grid, 0, 0, "%s/%s_grid.dat", OUTPUT_DIR, SIMULATION_NAME);
	}
	/* other variables */
	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0; yi != NY; yi++) {
			for (xi = 0; xi != NX; xi++) {
				i = xi + yi * NX + zi * NXNY;
				x1[i] = x_ss;
				x2[i] = x_ss;
				y_1[i] = y_ss;
				y2[i] = y_ss;
				r1[i] = (float) 0;
				r2[i] = (float) 0;
				x_ss = y_ss = (tf_q * (tf_f + 1)) / (tf_f - 1);
				x1[i] = x2[i] = x_ss;
				y_1[i] = y2[i] = y_ss;
			}	/* end xi */
		}		/* end yi */
	}			/* end zi */
}

static void     my_write_grid(char *fname, char *buf, int nx, int ny)
{
	int             i, n;
	unsigned char  *buf2;

	n = nx * ny;
	buf2 = XALLOC(n, unsigned char);
	for (i = 0; i != n; i++) {
		buf2[i] = (buf[i] == 0 ? 255 : 0);
	}
	bzsimWriteJPEG(fname, buf2, NX, NY, BZSIM_BW, 100);
	XFREE(buf2);
}

static void     do_step(float *x, float *y, float *r, float *xs, float *ys, float *rs)
{
	int             i, xi, yi, zi;
	float           tmp_x, tmp_y, tmp_r, tmp_xi, lap_x, lap_r;

	/* boundary conditions */
	switch (BOUNDARY) {
	case 2:
		bzsimNoFluxBC3D(NX, NY, NZ, 3, x, y, r);
		break;
	case 1:
		bzsimSinkBC3D(NX, NY, NZ, 3, x, y, r);
		break;
	default:
		/* bzsimPBC3D(NX, NY, NZ, 3, x, y, r); */
		exit(1);
		break;
	}			/* end switch(BOUNDARY) */

	/* Special case for 'diffusion spots' */
	for (i = 0; i != N; i++) {
		if (diff_grid[i]) {
			r[i] = (float) 1;
		}
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
	char           *parfile, fname[1024], logfile[1024];

	if (argc != 2) {
		fprintf(stderr, "usage: %s <parameter file>\n", argv[0]);
		exit(1);
	}
	bzsimInit(argv[0]);	/* application log file becomes bz3d.log */
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
	my_init_params(logfile);
	my_save_parameters(logfile);
	/* allocate memory for arrays */
	x1 = XALLOC(N, float);
	y_1 = XALLOC(N, float);
	x2 = XALLOC(N, float);
	y2 = XALLOC(N, float);
	r1 = XALLOC(N, float);
	r2 = XALLOC(N, float);

	
	/* analysis of core evolution */
	if (0 != SNAPZ_COMP) {
		int i;

		imagezn = XALLOC(N, float);	 	

		for (i = 0; i != N; i++)
			imagezn[i] = 0;
	}

	diff_grid = XALLOC(N, char);
	bzsimLogMsg(logfile, "float array bulk space: %d bytes\n", (int) (6 * N * sizeof(float)));
	bzsimLogMsg(logfile, "char array bulk space: %d bytes\n", (int) (1 * N * sizeof(float)));
	my_init_grid(logfile);
	my_check_parameters();
	sprintf(fname, "%s/%s_grid.jpg", OUTPUT_DIR, SIMULATION_NAME);
	my_write_grid(fname, diff_grid, NX, NY);
	bzsimLogMsg(logfile, "SPOTS %d / GRID POINTS %d = ACTUAL DENSITY %f\n", spot_cnt, N, ((float) spot_cnt) / (N));

	if (0 == spot_cnt) {
		fprintf(stderr, "ERROR: no spots\n");
		exit(1);
	}
	
	/* T I M E     L O O P     S T A R T S  */
	bTurn = 0;
	t = 0;
	while (++t <= ITERATIONS) {
		/* Switch pointers according to flag bTurn. */
		bTurn = !bTurn;
		if (bTurn)
			do_step(x1, y_1, r1, x2, y2, r2);
		else
			do_step(x2, y2, r2, x1, y_1, r1);

		if (SNAP != 0 && t > SNAP_START && (t % SNAP) == 0) {
			if (-1 == SNAPZ) {
				float *tmp = XALLOC(N, float);
				int xi, yi, zi, i;

				memcpy(tmp, x1, N * sizeof(float));

				for (yi = 0, zi = 0; zi < NZ; zi++) {
					for (xi = 0; xi < NX; xi++) {
						i = xi + yi * NX + zi * NXNY;
						tmp[i] = SCAL_MAX;
					}
				}
				
				sprintf(fname, "%s/%s_%010d", OUTPUT_DIR, SIMULATION_NAME, t);
				save_grid_image(fname, tmp, NX, NY*NZ, SCAL_MIN, SCAL_MAX);

				XFREE(tmp);
			} else {
				sprintf(fname, "%s/%s_%010d", OUTPUT_DIR, SIMULATION_NAME, t);
				save_grid_image(fname, (x1 + (SNAPZ * NXNY)), NX, NY, SCAL_MIN, SCAL_MAX);
			// FIXME bzsimSaveData2(NX, NY, NZ, BZSIM_DATA_FLOAT, x1, SCAL_MIN, SCAL_MAX, "%s/%s_x1_%010d.dat", OUTPUT_DIR, SIMULATION_NAME, t);
			}
#if 0
			/* 3D render hook */
			system("./snap.sh");
#endif
		}

		if (0 != SNAPZ_COMP && t > SNAP_START && 0 == (t % SNAPZ_COMP)) {
			int xi, yi, zi;

			imagezn_n++;

#if 0
			memset(tmp, 0, NX * NY * sizeof(float));

			for (yi = 0; yi < NY; yi++) {
				for (xi = 0; xi < NX; xi++) {
					tmp[xi + yi * NX] = 0;
				}
			}
#endif		   

			for (zi = 0; zi < NZ; zi++) {
				for (yi = 0; yi < NY; yi++) {
					for (xi = 0; xi < NX; xi++) {
						int i = xi + yi * NX + zi * NXNY;
						//int j = xi + yi * NX; 
						//tmp[j] = x1[i] > tmp[j] ? x1[i] : tmp[j];
						//tmp[j] = x1[i] > 0.5 ? 1 : tmp[j];
						//tmp[j] = x1[i] > 0.4 ? 1 : tmp[j];
						// superimpose
						// FIXME
						//imagezn[i] = x1[i] > imagezn[i] ? x1[i] : imagezn[i];
						// average
						imagezn[i] += x1[i];
					}
				}
			}			

			if (0 == (imagezn_n % imagezn_samples)) {
				//float *tmp = XALLOC(NX * NY, float);

#if 1
				int i;

				for (i = 0; i != N; i++)
					imagezn[i] /= (float) imagezn_samples;
#endif
				
				sprintf(fname, "%s/%s_%010d_z_comp", OUTPUT_DIR, SIMULATION_NAME, t);
				save_grid_image(fname, imagezn, NX, NY*NZ, SCAL_MIN, SCAL_MAX);
								
				//save_grid_image(fname, tmp, NX, NY, SCAL_MIN, SCAL_MAX);
				//XFREE(tmp);
			}
				
		}
			
		if (SAVE_DF3 && 0 == (t % SAVE_DF3)) {
			/* save only r whose values are from 0 to 1 */
			float *tmp = XALLOC(N, float);
			int xi, yi, zi, i;

			for (zi = 0; zi != NZ; zi++) {
				for (yi = 0; yi != NY; yi++) {
					for (xi = 0; xi != NX; xi++) {
						i = xi + yi * NX + zi * NXNY;
						if (0 == xi ||
							0 == yi ||
							0 == zi ||
							NX1 == xi ||
							NY1 == yi ||
							NZ1 == zi)
							tmp[i] = 0;
						else
							tmp[i] = r1[i];
					}
				}
			}

			i = bzsimSaveDf3(tmp, NX, NY, NZ, BZSIM_DATA_FLOAT,
							 0, 1, "%s/%010d.df3", OUTPUT_DIR, t);
			if (BZSIM_OK != i)
				bzsimPanic("main: writing DF3 failed %d", t);

			XFREE(tmp);
		} /* end SAVE DF3 */

	}			/* end TIME LOOP */

	bzsimSaveData(NX, NY, NZ, BZSIM_DATA_FLOAT, x1, SCAL_MIN, SCAL_MAX, "%s/%s_x1_final.dat", OUTPUT_DIR, SIMULATION_NAME);
	bzsimSaveData(NX, NY, NZ, BZSIM_DATA_FLOAT, r1, SCAL_MIN, SCAL_MAX, "%s/%s_r1_final.dat", OUTPUT_DIR, SIMULATION_NAME);

	/* Free the allocated memory. */
	XFREE(x1);
	XFREE(x2);
	XFREE(y_1);
	XFREE(y2);
	XFREE(r1);
	XFREE(r2);

	if (-1 == SNAPZ) {
		char *cmd = XALLOC(8192, char);
		
		sprintf(cmd, "test -x $(command -v montage) && montage %s/%s_??????????.jpg -geometry +1+1 %s/%s_slices.jpg", OUTPUT_DIR, SIMULATION_NAME, OUTPUT_DIR, SIMULATION_NAME);
		system(cmd);

		bzsimLogMsg(logfile, "%s%s", cmd, "\n");
		
		XFREE(cmd);
	}

	if (0 != SNAPZ_COMP) {
		char *cmd = XALLOC(8192, char);
		
		sprintf(cmd, "test -x $(command -v montage) && montage %s/%s_??????????_z_comp.jpg -tile x1 -geometry +1+1 %s/%s_comp.jpg", OUTPUT_DIR, SIMULATION_NAME, OUTPUT_DIR, SIMULATION_NAME);
		system(cmd);

		bzsimLogMsg(logfile, "%s%s", cmd, "\n");

		XFREE(cmd);
		XFREE(imagezn); 
	}
	
	/* Timing info */
	bzsimSetEndTime();
	bzsimLogElapsedTime(logfile);

	return 0;
}

