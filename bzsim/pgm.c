/****h* bzsim/pgm.c
 *  NAME
 *    pgm.c
 *  DESCRIPTION
 *********/

#include<stdio.h>
#include<stdlib.h>
#include "bzsim.h"

/****f* pgm.c/bzsimSavePGM
 *  NAME
 *    bzsimSavePGM
 *  SYNOPSIS
 *    int bzsimSavePGM(char *fname, void *inbuf, int partype, int nx, int ny, bzsim_scale_t * s)
 *  SOURCE
 */

int             bzsimSavePGM(char *fname, void *inbuf, int partype, int nx, int ny, bzsim_scale_t * s)
{
	unsigned char  *buf;
	int             i = 0;

	buf = (unsigned char *) malloc(nx * ny * sizeof(unsigned char));
	if (buf) {
		bzsimScaleArr(inbuf, partype, buf, (nx * ny), s);
		i = bzsimWritePGM(fname, buf, nx, ny);
		free(buf);
	} else {
		bzsimLogMsg(BZSIM_LOG, "bzsimSavePGM: out of memory (%d bytes)\n", (nx * ny));
	}
	return i;
}

/************ bzsimSavePGM */

/****f* pgm.c/bzsimWritePGM
 *  NAME
 *    bzsimWritePGM
 *  SYNOPSIS
 *    int bzsimWritePGM(char *fname, unsigned char *buf, int nx, int ny)
 *  SOURCE
*/
int             bzsimWritePGM(char *fname, unsigned char *buf, int nx, int ny)
{
	FILE           *fp;
	int             res = BZSIM_ERROR;

	fp = fopen(fname, "wb");
	if (fp) {
		/* Write the PGM header */
		fprintf(fp, "P5\n%d %d\n255\n", nx, ny);
		/* Write the image data */
		if ((fwrite(buf, (nx * ny), 1, fp)) == 1) {
			res = BZSIM_OK;
		}
		fclose(fp);
	} else {
		bzsimLogMsg(BZSIM_LOG, "bzsimWritePGM: writing file '%s' failed\n", fname);

	}
	return res;
}

/************ bzsimWritePGM */
