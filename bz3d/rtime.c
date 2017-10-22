/* $Id: rtime.c,v 1.2 2004/01/10 11:19:00 petterik Exp $ */
/****h* bz3d/slicer.c
 *
 * NAME
 *  rtime.c
 * DESCRIPTION
 *  Simple program to read pixel value from series of JPEG image files.
 * SOURCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>		/* for stat()   */
#include "bzsim.h"

int 
main(int argc, char *argv[])
{
	int             x0, y0, nx, ny, nz, n = 0;
	char            f[PATH_MAX];
	unsigned char  *buf;
	struct stat     st;
	float           dt;

	if (argc != 4)
		bzsimPanic("usage: %s <dt> <x0> <y0>", argv[0]);

	dt = atof(argv[1]);
	x0 = atoi(argv[2]);
	y0 = atoi(argv[3]);

	while (fgets(f, PATH_MAX, stdin) != NULL) {
		f[strlen(f) - 1] = 0;
		if (stat(f, &st) >= 0 && st.st_size > 0) {
			if (!(buf = bzsimReadJPEG(f, &nx, &ny, &nz)))
				bzsimPanic("reading %s failed", f);
			ASSERT(nz == 1 && x0 < nx && y0 < ny);
			printf("%f %d\n", n * dt, buf[x0 + y0 * nx]);
			XFREE(buf);
			n++;
		}
	}

	return 0;
}

/**** end SOURCE */
