/* $Id: bz2d2.c,v 1.3 2003/12/21 15:03:19 petterik Exp $ */
/*
  origical: bz2d2.c
  ~~~~~
  Oregenator; three variables; Bar-type system;
  compilation with graphics:
  cc -O2 -mips2 -DVISUAL -o bar bar.c rgb.c -lm -lgl
  compilation without graphics:
  cc -O2 -mips2 -o bar bar.c rgb.c -lm 
  compilation in hermes:
  cc -O3 -mips4 -64 -o bar bar.c rgb.c -lm -lgl
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "bzsim.h"

#define A 0.2					/* BrO3 */
#define B 0.1					/* 0.2 MA + BrMA */
#define H 0.36					/* H+ */
#define K1 (H*H*2.0)
#define K2 (H*3000000.0)
#define K3 (42.0*H)
#define K4 3000.0
#define K5 1.0
#define ORE_D 0.000015

#define FHAT      2.45			/* 2.381 */
#define FHAT2     3.0
#define Q ((2.0*K1*K4)/(K2*K3))
#define EPS ((K5*B)/(K3*A))
#define EPSPRIME  ((2.0*K4*K5*B)/(K2*K3*A))
#define ORE_T_U  (1.0/(K5*B))
#define ORE_S_U  sqrt(ORE_D/(K5*B))

#define DXY               1
#define D_U               1.0
#define D_W               1.0
#if 1
#define D_U_A             (D_U/1.0)
#define D_W_A             (D_W/1.0)
#define D_U_B             (D_U/15.0)
#define D_W_B             (D_W/15.0)
#else
#define D_U_A             (D_U/10.0)
#define D_W_A             (D_W/10.0)
#define D_U_B             (D_U/1.0)
#define D_W_B             (D_W/1.0)
#endif

#define BETA            20

#if 0
#define L               (64)	/* 50 */
#define NX 		       (256)
#define NY 	           (256)
#define ENDZEIT 	3000000
#define SNAP 		1000
#else
#define L               (64)	/* 50 */
#define NX 		       (128)
#define NY 	           (128)
#define ENDZEIT 	6000
#define SNAP 		200
#endif

#define DIR             "./tmp/"
#define NAME            "bz2d2"
#define MIN_MAX         0
#define P_VAR           v
#define PSICRIT         (80.0*0.00027)
#define PSI             0.00027
#define BOUNDS          0
#define SAVE_FILE       "uvw.data"
#define LOAD_INITIAL    0
#define LOAD_FILE       "ini.256.128"
#define MAKE_SPIRAL     500 /*5000 ok for 512 */ /* 600 nice */ /* 500 ok */ /*150*/	/* 500 */
#define MAKE_SPIRAL_T   1.5

#define ILLUMI_A          1
#define ILLUMI_A_AMP      0.05	/* 0.1 ==> drift out of the scene */
#define ILLUMI_A_F        0.05
#define PERIODIC_Y        0 /* PBC in y-direction */
#define PULSING           0 /* 150 */

/* 
   if you want 5-point laplacian leave 1 the the 
   line below. otherwise set it to zero and
   program uses the 9-point formulae
*/
#define FIVE_POINT_LAPLACIAN       1
#define PERTURBATION               0.2	/* 0.075 */
#define PRINT_TIME                 0
#define MIDDLE_SECTION             0

/* don't touch to the definitions below */
/* you are not expected to understand this */
#define NX1 		(NX-1)
#define NX2 		(NX/2)
#define NY1 		(NY-1)
#define N  		(NX*NY)
#define INDEX 		(x+y*NX)

float v1[NX*NY], v2[NX*NY];
int gov = 0;

int main(int argc, char *argv[])
{
	float *u, *v, *w, *u2, *v2, *w2, eps = EPS, epsprime = EPSPRIME, f = FHAT, q = Q, d = D_U_A, dw = D_W_A, dt_o_dx2, dt, dx, lam1, lam2, lam3, lam4, psi, psi2 = PSI, lam2b, lam4b, fij, gij, hij, mask, maskw, uvss, wss, uthr;
	int i, x, y, t = 0;
	char fname[300];
#if MIN_MAX
	float umin = 999.0, umax = -999.0, vmin = 999.0, vmax = -999.0, wmin = 999.0, wmax = -999.0;
#endif
#if M_S
	FILE *mfile;
	float mdata[NX];
#endif
#if DXY
	float *dxy, xyi;
#endif

	/* allocate memory for arrays */
	u = malloc(N * sizeof(float));
	v = malloc(N * sizeof(float));
	u2 = malloc(N * sizeof(float));
	v2 = malloc(N * sizeof(float));
	w = malloc(N * sizeof(float));
	w2 = malloc(N * sizeof(float));

	if (!u || !v || !u2 || !v2 || !w || !w2) {
		fprintf(stderr, "out of memory\n");
		exit(1);
	}

#if DXY
	dxy = malloc(N * sizeof(float));
	if (!dxy)
		exit(1);
#endif


	uvss = (q * (f + 1.0)) / (f - 1.0);
	uthr = f * q;
	dx = ((float) L) / ((float) NX);
	wss = (f * uvss) / (uvss + q);

	{
		float dmax = (D_U_A > D_U_B ? D_U_A : D_U_B);
#if 0
#if FIVE_POINT_LAPLACIAN
		dt = (((dx * dx) / (4.0 * dw))) / ((float) BETA);
#else
		dt = (((3.0 * dx * dx) / (8.0 * dw))) / ((float) BETA);
#endif
#else
#if FIVE_POINT_LAPLACIAN
		dt = (((dx * dx) / (4.0 * dmax))) / ((float) BETA);
#else
		dt = (((3.0 * dx * dx) / (8.0 * dmax))) / ((float) BETA);
#endif
#endif
	}

	lam1 = dt / eps;
	lam3 = dt / epsprime;
	dt_o_dx2 = dt / (dx * dx);

#if FIVE_POINT_LAPLACIAN
	lam2 = (d * dt) / (dx * dx);
	lam4 = (dw * dt) / (dx * dx);
	lam2b = (D_U_B * dt) / (dx * dx);
	lam4b = (D_W_B * dt) / (dx * dx);
	if (dt > ((dx * dx) / (4.0 * d)))
		printf("dt may be too big\n");
	fprintf(stderr, "5-point\n");
#else
	/* use the nine-point formulae */
	lam2 = (d * dt) / (6.0 * dx * dx);
	lam4 = (dw * dt) / (6.0 * dx * dx);
	lam2b = (D_U_B * dt) / (6.0 * dx * dx);
	lam4b = (D_W_B * dt) / (6.0 * dx * dx);
	if (dt > ((3.0 * dx * dx) / (8.0 * d)))
		printf("dt may be too big\n");
	fprintf(stderr, "9-point\n");
#endif

	fprintf(stderr, "NX   = %d\n", NX);
	fprintf(stderr, "NY   = %d\n", NY);
	fprintf(stderr, "L    = %d\n", L);
	fprintf(stderr, "E.Z. = %d\n", ENDZEIT);
	fprintf(stderr, "dx   = %g\n", dx);
	fprintf(stderr, "dt   = %g\n", dt);
	fprintf(stderr, "eps  = %g\n", eps);
	fprintf(stderr, "eps' = %g\n", epsprime);
	fprintf(stderr, "f    = %g\n", f);
	fprintf(stderr, "q    = %g\n", q);
	fprintf(stderr, "A: du/dw= %g\n", D_U_A / D_W_A);
	fprintf(stderr, "B: du/dw= %g\n", D_U_B / D_W_B);
	fprintf(stderr, "D A/B   = %g\n", D_U_A / D_U_B);
	fprintf(stderr, "uvss = %g\n", uvss);
	fprintf(stderr, "wss  = %g\n", wss);
	fprintf(stderr, "uthr = %g\n", uthr);
	fprintf(stderr, "lam1 = %g\n", lam1);
	fprintf(stderr, "lam2 = %g\n", lam2);
	fprintf(stderr, "lam3 = %g\n", lam3);
	fprintf(stderr, "lam4 = %g\n", lam4);
	fprintf(stderr, "%d bytes\n", 6 * NX * NY * sizeof(float));
	fprintf(stderr, "tu   = %g s\n", ORE_T_U);
	fprintf(stderr, "su   = %g cm\n", ORE_S_U);
	fprintf(stderr, "area = %g cm x %g cm\n", ORE_S_U * dx * NX, ORE_S_U * dx * NY);

#if M_S
	sprintf(fname, "%s%s.ms.data", DIR, NAME);
	mfile = fopen(fname, "w");
	if (!mfile)
		exit(1);
	for (x = 0; x < NX; x++)
		mdata[x] = 0.0;
	error
#endif

	/* initialise */
	for (i = 0; i < N; i++) {
		/* steady-state values */
		u[i] = uvss;
		v[i] = uvss;
		w[i] = wss;
		u2[i] = uvss;
		v2[i] = uvss;
		w2[i] = wss;
	}

#if DXY
	for (y = 0; y < NY; y++)
		for (x = 0; x < NX; x++)
			if (x < NX / 2)
				dxy[INDEX] = D_U_A;
			else
				dxy[INDEX] = D_U_B;
#endif /* DXY */

#if PULSING
	/* null */
#else
#if 1
	/* perturbation (spiral) */
	for (y = 0; y < NY; y++)
		for (x = 0; x < 3; x++)
			u[INDEX] = PERTURBATION;
#else
#if 0
	/* perturbation (target pattern) */
	for (y = NY/2-3; y < NY/2+3; y++)
		for (x = 0; x < 5; x++)
			u[INDEX] = PERTURBATION;
#else
	/* competing spiral pair */
	for (y = 0; y < NY; y++)
		for (x = NX/2-2; x < NX/2+2; x++)
			u[INDEX] = PERTURBATION;
#endif
#endif
#endif /* !PULSING */ 

#if LOAD_INITIAL
	sprintf(fname, "%s%s", DIR, LOAD_FILE);
	file = fopen(fname, "r");
	if (file) {
		for (i = 0; i < N; i++)
			fscanf(file, "%f", &u[i]);
		for (i = 0; i < N; i++)
			fscanf(file, "%f", &v[i]);
		for (i = 0; i < N; i++)
			fscanf(file, "%f", &w[i]);
		fprintf(stderr, "loaded grid from file %s\n", fname);
		fclose(file);
	} else {
		fprintf(stderr, "unable to open file %s for reading\n", fname);
	}
#endif

	/*
	   T I M E     L O O P     S T A R T S
	 */
	t = 0;
	psi2 = 0;
	psi = 0.0;
	while (t++ < ENDZEIT) {

#if PULSING
		if ((t % PULSING) == 0) {
			for (y = NY/2-3; y < NY/2+3; y++)
				for (x = 0; x < 5; x++)
					u[INDEX] = PERTURBATION;
		}
#endif /* PULSING */

#if ILLUMI_A
		float psi_t = ILLUMI_A_AMP * PSICRIT * (1.0 + sin(2.0 * M_PI * ILLUMI_A_F * (t - MAKE_SPIRAL) * dt));
#endif

		/* non-flux boundaries */
#if PERIODIC_Y
		bzsimPBC2D(NX, NY, 2, u, w);
#else
		y = 0;
		for (x = 0; x < NX; x++) {
			i = INDEX;
			u[i] = u[i + NX];
			/* v[i]=v[i+NX]; */
			w[i] = w[i + NX];
		}
		y = NY1;
		for (x = 0; x < NX; x++) {
			i = INDEX;
			u[i] = u[i - NX];
			/* v[i]=v[i-NX]; */
			w[i] = w[i - NX];
		}
#endif /* PERIODIC_Y */

		x = 0;
		for (y = 0; y < NY; y++) {
			i = INDEX;
			u[i] = u[i + 1];
			/* v[i]=v[i+1]; */
			w[i] = w[i + 1];
		}
		x = NX1;
		for (y = 0; y < NY; y++) {
			i = INDEX;
			u[i] = u[i - 1];
			/* v[i]=v[i-1]; */
			w[i] = w[i - 1];
		}

		/* end of non-flux boundaries */

		/* sweep */
		for (y = 1; y < NY1; y++)
			for (x = 1; x < NX1; x++) {
				i = INDEX;

#if MAKE_SPIRAL && (PULSING == 0)
				if (y > NY / 2 && t < MAKE_SPIRAL)
					psi = PSICRIT;
				else
					psi = 0.0;
#else
					psi = 0.0;
#endif

#if ILLUMI_A
				if (t > MAKE_SPIRAL && x < NX / 2)
					psi = psi_t;
#endif							/* ILLUMI_A */

#if BOUNDS && MAKE_SPIRAL
#define BOR 20
				if (t > MAKE_SPIRAL)
					if (x == BOR || x == NX - BOR || y == BOR || y == NY - BOR)
						psi = PSICRIT;
#endif

				/* dynamics */
				fij = w[i] * (q - u[i]) + u[i] - u[i] * u[i];
				gij = u[i] - v[i];
				hij = -q * w[i] - u[i] * w[i] + f * v[i] + psi;

#if DXY
				/* diffusion */
				xyi = u[i];
				mask = dxy[i + 1] * (u[i + 1] - xyi) - dxy[i - 1] * (xyi - u[i - 1])
					+ dxy[i + NX] * (u[i + NX] - xyi) - dxy[i - NX] * (xyi - u[i - NX]);
				xyi = w[i];
				maskw = dxy[i + 1] * (w[i + 1] - xyi) - dxy[i - 1] * (xyi - w[i - 1])
					+ dxy[i + NX] * (w[i + NX] - xyi) - dxy[i - NX] * (xyi - w[i - NX]);
#else
#if FIVE_POINT_LAPLACIAN
				/* 5-point Laplacian mask */
				mask = u[i - 1] + u[i + 1] + u[i + NX] + u[i - NX] - 4.0 * u[i];
				maskw = w[i - 1] + w[i + 1] + w[i + NX] + w[i - NX] - 4.0 * w[i];
#else
				/* 9-point Laplacian mask */
				error; 
				mask = 4.0 * (u[i - 1] + u[i + 1] + u[i + NX] + u[i - NX]) + u[i + NX - 1] + u[i + NX + 1] + u[i - NX - 1] + u[i - NX + 1] - 20.0 * u[i];
				maskw = 4.0 * (w[i - 1] + w[i + 1] + w[i + NX] + w[i - NX]) + w[i + NX - 1] + w[i + NX + 1] + w[i - NX - 1] + w[i - NX + 1] - 20.0 * w[i];
#endif
#endif /* DXY */

				/* here comes Euler */
#if DXY
				u2[i] = u[i] + lam1 * fij + dt_o_dx2 * mask;
				v2[i] = v[i] + gij * dt;
				w2[i] = w[i] + hij * lam3 + dt_o_dx2 * maskw;
#else
				if (x < NX2) {
					u2[i] = u[i] + lam1 * fij + lam2 * mask;
					v2[i] = v[i] + gij * dt;
					w2[i] = w[i] + hij * lam3 + lam4 * maskw;
				} else if (x > NX2) {
					u2[i] = u[i] + lam1 * fij + lam2b * mask;
					v2[i] = v[i] + gij * dt;
					w2[i] = w[i] + hij * lam3 + lam4b * maskw;
				}
				if (x == NX2) {
					u2[i] = u[i] + lam1 * fij + dt_o_dx2 * (D_U_B * (u[i + 1] - u[i]) - D_U_A * (u[i] - u[i - 1]));
					v2[i] = v[i] + gij * dt;
					w2[i] = w[i] + hij * lam3 + dt_o_dx2 * (D_W_B * (w[i + 1] - w[i]) - D_W_A * (w[i] - w[i - 1]));
				}
#endif

			}					/* end sweep */

		/* swap variables */
		for (i = 0; i < N; i++) {
			if (u2[i] > q)
				u[i] = u2[i];
			else
				u[i] = q;
			if (v2[i] > 0.0)
				v[i] = v2[i];
			else
				v[i] = 0.0;
			if (w2[i] > 0.0)
				w[i] = w2[i];
			else
				w[i] = 0.0;
			u2[i] = uvss;
			v2[i] = uvss;
			w2[i] = wss;


#if MIN_MAX
			if (u[i] < umin)
				umin = u[i];
			else if (u[i] > umax)
				umax = u[i];
			if (v[i] < vmin)
				vmin = v[i];
			else if (v[i] > vmax)
				vmax = v[i];
			if (w[i] < wmin)
				wmin = w[i];
			else if (w[i] > wmax)
				wmax = w[i];
#endif

		}						/* end swap */

#if PRINT_TIME
		if (!(t % PRINT_TIME)) {
			x = NX / 2;
			y = NY / 2;
			i = INDEX;
			fprintf(stderr, "%d %f %f %f\n", t, u[i], v[i], w[i]);
		}
#endif

#if 1
		if (!(t % SNAP)) {
			static int frame = 0;
			unsigned char *buf = XALLOC(NX * NY, unsigned char);
			sprintf(fname, "%d.jpg", frame++);

			bzsimScaleFloatArr(P_VAR, buf, NX * NY, 0, 0);
			bzsimWriteJPEG(fname, buf, NX, NY, BZSIM_BW, 95);

			if (gov == 0) {
				memcpy(v1, P_VAR, NX*NY*sizeof(float));
				gov = 1;
			} else {
				int i;
				float sub[NX*NY];
				static int subframe = 0;
				float smin = 1e22, smax = -1e22;
				int mx = 0, my = 0, mix = 0, miy = 0, xi, yi;
				float gmax = -100000;
				unsigned char buf2[N];

				memcpy(v2, P_VAR, NX*NY*sizeof(float));


				for (yi = 0; yi != NY; yi++) {
					for (xi = 0; xi != NX; xi++) {
						i = xi + yi * NX;
						sub[i] = v2[i] - v1[i];						
						if (sub[i] < smin) {
							smin = sub[i];
							mix = xi;
							miy = yi;
						}
						if (sub[i] > smax) {
							smax = sub[i];
							mx = xi;
							my = yi;
						}
					}
				}

#if 1
				/* finding the tip */
				for (yi = 3; yi != (NY1 - 2); yi++) {
					for (xi = 3; xi != NX/2 - 3; xi++) {
						float tmp, gux, guy, gvx, gvy;
						i = xi + yi * NX;

						gux = - u[i - NX - 1]
							+ u[i - NX + 1]
							-2.0 * u[i - 1]
							+2.0 * u[i + 1]
							-u[i + NX -1]
							+ u[i + NX + 1];
						gvx = - v[i - NX -1]
							+ v[i - NX + 1]
							-2.0 * v[i - 1]
							+2.0 * v[i + 1]
							-v[i + NX -1]
							+ v[i + NX + 1];
						guy = -u[i - NX - 1]
							- 2.0 * u[i - NX]
							- u[i - NX + 1]
							+ u[i + NX - 1]
							+ 2.0 * u[i + NX]
							+ u[i + NX + 1];
						gvy = -v[i - NX - 1]
							- 2.0 * v[i - NX]
							- v[i - NX + 1]
							+ v[i + NX - 1]
							+ 2.0 * v[i + NX]
							+ v[i + NX + 1];

#if 0						
						tmp = sqrt(gux*gux + guy*guy)
							* sqrt(gvx*gvx + gvy*gvy)
							* sqrt(1.0 - (gux*gvy + guy*gvy)*(gux*gvy + guy*gvy));
#endif

						tmp = gux * gvy - gvx*guy;
						tmp *= tmp;
						tmp = sqrt(tmp);
						if (gmax < tmp) {
							gmax = tmp;
							mix = xi;
							miy = yi;
						}
					}
				}
				
#endif
				
				bzsimScaleFloatArr(sub, buf, NX * NY, 0, 0);
				memcpy(buf2, buf, N);


				for (xi = mix, yi = 0; yi != NY; yi++)
					buf[xi + yi*NX] = 0xff;
				for (yi = miy, xi = 0; xi != NX; xi++)
					buf[xi + yi*NX] = 0xff;				

				sprintf(fname, "sub%05d.jpg", subframe++);
				bzsimWriteJPEG(fname, buf, NX, NY, BZSIM_BW, 95);
				memcpy(v1, v2, NX*NY*sizeof(float));
				/* fprintf(stderr, "%f %f\n", smin, smax); */
				
			}
			XFREE(buf);						
		}
#endif

#if M_S
		if (!(t % M_S)) {
			y = NY / 2;
			for (x = 0; x < NX; x++) {
				if (mdata[x] < u[i = INDEX])
					mdata[x] = u[i];
			}
		}
#endif

		if (!(t % 33)) {
			printf("%f ", t * dt);
			y = NY / 2;
			x = NX / 2 - 3;
			printf("%f ", v[INDEX]);
			x = NX - 3;
			printf("%f", v[INDEX]);

#if ILLUMI_A
			printf(" %f", psi_t);
#endif

			printf("\n");
			fflush(stdout);
		}

	}							/* end t */

#if MIN_MAX
	printf("umin=%f umax=%f vmin=%f vmax=%f\n", umin, umax, vmin, vmax);
	printf("wmin=%f wmax=%f \n", wmin, wmax);
#endif

#if M_S
	for (x = 0; x < NX; x++)
		fprintf(mfile, "%f %f\n", (float) dx * x, mdata[x]);
	fclose(mfile);
#endif

	printf("DIR  = %s\n", DIR);
	printf("NAME = %s\n", NAME);
	printf("NX   = %d\n", NX);
	printf("NY   = %d\n", NY);
	printf("L    = %d\n", L);
	printf("E.Z. = %d\n", ENDZEIT);
	printf("dx   = %g\n", dx);
	printf("dt   = %g\n", dt);
	printf("eps  = %g\n", eps);
	printf("f    = %g\n", f);
	printf("q    = %g\n", q);
	printf("uvss = %g\n", uvss);
	printf("uthr = %g\n", uthr);

	/* free allocated memory */
	free(u);
	free(v);
	free(w);
	free(u2);
	free(v2);
	free(w2);

#if DXY
	free(dxy);
#endif

	return 0;
}								/* end main */
