/* $Id: slice3d.c,v 1.9 2004/01/12 15:27:38 petterik Exp $ */
/****h* bz3d/slice3d.c
 *
 * NAME
 *  slice3d.c
 * DESCRIPTION
 *  Various internal routines to post-process simulation data. Many of these
 *  were different attempts to locate filament from 3D data. This will be
 *  not maintained but portions of code may migrate to other functions or
 *  modules case by case.
 *****
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bzsim.h"

#define _DEBUG_ 0
#define MAXZ    128 /* UGLY */

static int tip[MAXZ*3]; /* z, x, y */
static int tipptr = 0;

/*
x:       y:
-1 0 1   -1 -2 -1
-2 0 2    0  0  0
-1 0 1    1  2  1
*/

#if 0

static void bzsimKernelSobel(float *sx, float *sy, float *v, 
							 int x, int y, int nx, int ny)
{
	int i = x + y * nx;

	*sx = -v[i - nx - 1] + v[i - nx +1]
		-2.0 * v[i - 1] + 2.0 * v[i + 1]
		-v[i + nx - 1] + v[i + nx +1];
	*sy = -v[i - nx - 1] - 2.0 * v[i - nx] - v[i - nx +1]
		+ v[i + nx - 1] + 2.0 * v[i + nx] + v[i + nx +1];
}

#endif /* 0 */

static void bzsimCross3D(float *x, float *y, float *z, 
						 float x0, float x1, float x2, 
						 float y0, float y1, float y2)
{
	*x = x1 * y2 - x2 * y1;
	*y = x0 * y2 - x2 * y0;
	*z = x0 * y1 - x1 * y0; 
}

static void bzsimZuckerHummel(float *gx, float *gy, float *gz, float *v, int x, int y, int z, int nx, int ny, int nz)
{
	
#define V(i,j,k) (v[(i) + (j)*nx + (k)*nxny])

	float k1 = sqrt( 3.0 ) / 3.0;
	float k2 = sqrt( 2.0 ) / 2.0;
	/* http://www.artree.com/rakesh/papers/spie-01.pdf */
	int nxny = nx * ny;
	/* int i = x + y * nx + z * nxny; */
	
	*gx = V(x+1,y,z) - V(x-1,y,z) 
		+ k1 * ( V(x+1,y+1,z+1) - V(x-1,y+1,z+1) + V(x+1,y-1,z+1) 
			   - V(x-1,y-1,z+1) + V(x+1,y+1,z-1) - V(x-1,y+1,z-1) 
			   + V(x+1,y-1,z-1) - V(x-1,y-1,z-1)) 
		+ k2 * ( V(x+1,y,z+1) - V(x-1,y,z+1) + V(x+1,y+1,z+1) 
				 - V(x-1,y+1,z+1) + V(x+1,y,z-1) - V(x-1,y,z-1) 
				 + V(x+1,y-1,z) - V(x-1,y-1,z));
	*gy = V(x,y+1,z) - V(x,y-1,z) 
		+ k1 * ( V(x-1,y+1,z-1) - V(x-1,y-1,z+1) + V(x+1,y-1,z+1) 
				 - V(x+1,y-1,z+1) + V(x-1,y+1,z-1) - V(x-1,y-1,z-1) 
				 + V(x+1,y+1,z-1) - V(x+1,y-1,z-1)) 
		+ k2 * ( V(x,y+1,z+1) - V(x-1,y,z+1) + V(x+1,y+1,z) 
				 - V(x+1,y+1,z) + V(x,y+1,z-1) - V(x,y-1,z-1) 
				 + V(x+1,y-1,z) - V(x+1,y-1,z));
	*gz = V(x,y,z+1) - V(x,y,z-1) 
		+ k1 * (V(x-1,y+1,z+1) - V(x-1,y+1,z-1) + V(x+1,y+1,z+1) 
				- V(x+1,y+1,z-1) + V(x-1,y-1,z+1) - V(x-1,y-1,z-1) 
				+ V(x+1,y-1,z+1) - V(x+1,y-1,z-1)) 
		+ k2 * ( V(x,y+1,z+1) - V(x,y+1,z+1) + V(x,y-1,z+1) 
				 - V(x,y-1,z-1) + V(x+1,y,z+1) - V(x+1,y,z-1) 
				 + V(x-1,y,z+1) - V(x-1,y,z-1));
#undef V
}

static float * readGrid(char *path, int *x, int *y, int *z, float *vmin, float *vmax)
{
	FILE *fp;
	int i, n;
	float *res, fMin = 1.0e22, fMax = -1.0e22;

	if (!(fp = fopen(path, "rb")))
		bzsimPanic("opening '%s' failed", path);

	if (3 != fscanf(fp, "%d%d%d", x, y, z))
		bzsimPanic("data header read error");

	n = *x * *y * *z;
	res = XALLOC(n, float);

	for (i = 0; i != n; i++) {
		float f;

		if (fscanf(fp, "%f", &f) != 1)
			bzsimPanic("data read error (%d)", i);		
		res[i] = f;
		if (f < fMin) fMin = f;
		if (f > fMax) fMax = f;
	}

	fclose(fp);
	
	*vmax = fMax;
	*vmin = fMax;

#if _DEBUG_
	fprintf(stderr, "%s: min = %f max = %f\n", path, fMin, fMax);
#endif /* _DEBUG_ */

	return res;
}

#if 0 
static int save_grid_image2(char *fname, float *x, int NX, int NY, float minval, float maxval)
{
	unsigned char  *buf;
	int             res;
	char            newname[1024];

	buf = XALLOC(NX * NY, unsigned char);
	if (!buf) {
		fprintf(stderr, "save_grid_image: out of memomy (%d bytes)\n", (int) NX * NY);
		return BZSIM_ERROR;
	}
	bzsimScaleFloatArr(x, buf, NX * NY, minval, maxval);
	sprintf(newname, "%s.jpg", fname);
	res = bzsimWriteJPEG(newname, buf, NX, NY, BZSIM_BW, 100);
	XFREE(buf);

	return res;
}
#endif /* 0 */

static unsigned char * getZSlice(float *x, int NX, int NY, int NZ, int which, float minval, float maxval)
{
	unsigned char  *buf;

	buf = XALLOC(NX * NY, unsigned char);
	bzsimScaleFloatArr((x + which * NX * NY), buf, NX * NY, minval, maxval);

	return buf;
}

#if 0
static float bzsimLapl5x5(float *v, int x, int y, int nx)
{
	int i = x + y * nx;
	return (24.0*v[i] + v[i-1] + v[i+1] + v[i-2] + v[i+2]
			+ v[i-nx] + v[i+nx] + v[i-2*nx] + v[i+2*nx]
			+ v[i-nx-1] + v[i-nx-2] + v[i-2*nx-1] + v[i-2*nx-2]
			+ v[i+nx-1] + v[i+nx-2] + v[i+2*nx-1] + v[i+2*nx-2]
			+ v[i-nx+1] + v[i-nx+2] + v[i-2*nx+1] + v[i-2*nx+2]
			+ v[i+nx+1] + v[i+nx+2] + v[i+2*nx+1] + v[i+2*nx+2]);
}
#endif /* 0 */

static int bzsimFindTipLapl(int *tx, int *ty, float *v, int nx, int ny)
{
	int i, x, y, n = nx * ny;
	unsigned char *buf;
	float lmin = FLT_MAX, *lap;

	buf = XALLOC(n, unsigned char);
	lap = XALLOC(n, float);

	bzsimScaleFloatArr(v, buf, n, (float) 0, (float) 0);

	for (i = 0; i != n; i++) {
		/* hard-coded threshold: remember this */
		if (buf[i] < 200)
			buf[i] = 0;
		else
			buf[i] = 0xff;

		v[i] = buf[i];
	}

	for (y = 2; y != (ny - 3); y++) {
		for (x = 2; x != (nx - 3); x++) {			
			i = x + y * nx;
			lap[i] = (-8.0 * v[i] 
					  + v[i - 1]
					  + v[i + 1] 
					  + v[i + nx]
					  + v[i - nx] 
					  + v[i - nx -1]
					  + v[i - nx + 1] 
					  + v[i + nx - 1]
					  + v[i + nx + 1])/8.0;

			if (lap[i] < lmin) {
				lmin = lap[i];
				*tx = x;
				*ty = y;
			}
		} /* x */
	} /* y */
	
#if 1
	{
		char f[NAME_MAX];
		static int lapcnt = 0;

		sprintf(f, "lap%05d.jpg", lapcnt++);
		bzsimScaleFloatArr(lap, buf, n, 0, 0);
		for (x = *tx, y = 0; y != ny; y++)
			buf[x + y * nx] = 0xff;
		for (y = *ty, x = 0; x != nx; x++)
			buf[x + y * nx] = 0xff;
		bzsimWriteJPEG(f, buf, nx, ny, BZSIM_BW, 90);		
	}
#endif

	XFREE(buf);
	XFREE(lap);

	return BZSIM_OK;
}

static void findTip(float *v1, float *v2, int nx, int ny, int nz, float vmax)
{
	unsigned char  *buf1 = XALLOC(nx * ny, unsigned char);
	int x, y, z, tipx = -1, tipy = -1, nxny = nx * ny;
	float maxgrad = (float) 0;
	char f[NAME_MAX];
	float *v = XALLOC(nxny, float);

	for (z = 1; z != (nz-2); z++) { 
		float *gr = XALLOC(nxny, float);
		int tx = tipx, ty = tipy, go = 0;

		maxgrad = (float) 0;

		bzsimScaleFloatArr((v1 + z * nxny), buf1, nxny, 0.0, 1.0);
		memcpy(v, (v1 + z * nxny), nxny * sizeof(float));
		bzsimFindTipLapl(&tipx, &tipy, v, nx, ny);

		if (tx > -1) {
#if 0
			int dx = abs(tx - tipx);
			int dy = abs(ty - tipy);
			float d = sqrt(dx*dx + dy*dy);

			if (z == 29 || z == 30 || z == 31) {
				tipx = tx;
				tipy = ty;
			}

			if (0 && (dx > 5 || dy > 5)) {
				//fprintf(stderr, "z %d: dx %d dy %d: newx %d newy %d oldx %d oldy %d\n", z, dx, dy, tipx, tipy, tx, ty);
				tipx = tx;
				tipy = ty;
				go = 0;
				fprintf(stderr, "%d %f\n", z, d);
			}
#endif

		}

#if _DEBUG_
		fprintf(stderr, "starting search using: %d %d\n", tipx, tipy);
#endif /* _DEBUG_ */

		if (go) {
			for (y = 2; y != (ny - 2); y++) {
				for (x = 2; x != (nx - 2); x++) {
					float tmp, gux, guy, guz, gvx, gvy, gvz, gx, gy, gz;
					
					if (abs(x-tx) <= 1 && abs(y-ty) <= 1) {
						bzsimZuckerHummel(&gux, &guy, &guz, v1, x, y, z, nx, ny, nz);
						bzsimZuckerHummel(&gvx, &gvy, &gvz, v2, x, y, z, nx, ny, nz);
						bzsimCross3D(&gx, &gy, &gz, gux, guy, guz, gvx, gvy, gvz);
						
						tmp = sqrt(gx*gx + gy*gy + gz*gz);
						gr[x + y * nx] = tmp;
						if (tmp > maxgrad) {
							maxgrad = tmp;
							tipx = x;
							tipy = y;
						}
					} /* abs(x-tipx) && abs(y-tipy) */
				} /* x */
			} /* y */
		} /* go */


		//fprintf(stderr, "%d %d %d %d\n", tipx, tipy, tx2, ty2);

		tip[tipptr++] = z;
		tip[tipptr++] = tipx;
		tip[tipptr++] = tipy;

		for (x = tipx, y = 0; y != ny; y++)
			buf1[x + y * nx] = 0xff;

		for (y = tipy, x = 0; x != nx; x++)
			buf1[x + y * nx] = 0xff;

		sprintf(f, "v1%03d.jpg", z);
		bzsimWriteJPEG(f, buf1, nx, ny, BZSIM_BW, 90);
		
		XFREE(gr);
	} /* z */

	XFREE(buf1);
	XFREE(v);
}

static void tipData(void)
{
	int i, z;
	
	printf("%d %f %f\n", tip[0], (tip[1]+tip[4])/2.0, (tip[2]+tip[5])/2.0);
#if 0
	assert(tipptr > 6);
	/* three-point moving average */
	for (z = 1; z != (tipptr/3 - 1); z++) {
		i = 3 * z;
		printf("%d %f %f\n", tip[i], (tip[i+1]+tip[i-2]+tip[i+4])/3.0,
			   (tip[i+2]+tip[i-1]+tip[i+5])/3.0);
	}
#else
	assert(tipptr > 15);
	/* five-point moving average */
	for (z = 2; z != (tipptr/3 - 2); z++) {
		i = 3 * z;
		printf("%d %f %f\n", tip[i], 
			   (tip[i+1]+tip[i-2]+tip[i+4]+tip[i-5]+tip[i+7])/5.0,
			   (tip[i+2]+tip[i-1]+tip[i+5]+tip[i-4]+tip[i+8])/5.0);
	}
	printf("%d %f %f", tip[tipptr-6], (tip[tipptr-8]+tip[tipptr-5])/2.0, 
		   (tip[tipptr-7]+tip[tipptr-4])/2.0);
#endif

	printf("%d %f %f", tip[tipptr-3], (tip[tipptr-2]+tip[tipptr-5])/2.0, 
		   (tip[tipptr-1]+tip[tipptr-4])/2.0);
}

int             main(int argc, char *argv[])
{
	int             i, x, y, z, nx, ny, nz;
	float          *v, vmin, vmax;
	unsigned char *sum;

	if (strcmp("--stdin", argv[1]) == 0) {
		char line[128];

		while (fgets(line, 128, stdin) != NULL) {
			if (sscanf(line, "%d%d%d", &z, &x, &y) != 3)
				bzsimPanic("read error: %s", line);
			tip[tipptr++] = z;
			tip[tipptr++] = x;
			tip[tipptr++] = y;
		}

		if ((tipptr % 3) != 0) 
			bzsimPanic("data length error: %d", tipptr);

		tipData();

		exit(0);
	}

	if (argc < 2) {
		fprintf(stderr, "usage: %s <bzsim data file>\n", argv[0]);
		exit(1);
	}

	if (!(v = readGrid(argv[1], &nx, &ny, &nz, &vmin, &vmax)))
		bzsimPanic("reading '%s' failed", argv[1]);

	if (argc == 3) {
		float *v2 = readGrid(argv[2], &nx, &ny, &nz, &vmin, &vmax);

		if (!v2)
			bzsimPanic("reading '%s' failed", argv[2]);
		
		findTip(v, v2, nx, ny, nz, vmax);
		/*system("mogrify -format jpg -resize 1000% gr*.jpg");*/
		system("montage -mode concatenate -tile 8x12 v????.jpg PNG:xy.png");
		system("montage -mode concatenate -tile 8x12 lap*.jpg PNG:lap.png");
		system("rm -f lap*.jpg");

		for (i = 0; i < tipptr; ) {
			z = tip[i++];
			x = tip[i++];
			y = tip[i++];
			printf("%d %d %d\n", z, x, y);
		}

		exit(0);
	}

	sum = XALLOC(nx*ny, unsigned char);
	memset(sum, 0, nx*ny);

	for (z = 0; z != nz; z++) {
		char fname[1024];
		unsigned char *buf = getZSlice(v, nx, ny, nz, z, (float) 0, (float) 1);


		sprintf(fname, "z%03d.jpg", z);
		bzsimWriteJPEG(fname, buf, nx, ny, BZSIM_BW, 90);

		if ((z % 25) == 0) {
			int x, y;

			for (y = 0; y < ny; y++) {
				for (x = 0; x < nx; x++) {
					int i = x + y * nx;
					if (buf[i] > 128) 
						sum[i] = buf[i];
				}
			}
		}
		XFREE(buf);
	}

	bzsimWriteJPEG("s.jpg", sum, nx, ny, BZSIM_BW, 90);

	XFREE(v);
	XFREE(sum);
	z = system("mogrify -format jpg -resize 1000% *.jpg");
	if (z == -1)
		bzsimPanic("fork failed");
	else if (z != 0)
		bzsimPanic("scaling failed");

	return 0;
}

