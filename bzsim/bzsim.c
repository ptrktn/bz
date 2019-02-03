/****h* bzsim/bzsim.c
 * NAME
 *  bzsim.c
 *  $Id: bzsim.c,v 1.18 2006/05/03 13:06:00 petterik Exp $
 * DESCRIPTION
 *  Bag of tricks for BZ simulations.
 * HISTORY
 *  1997-12-19 petterik: Initial version (Tsukuba, Japan).
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>		/* for LONG_MAX */
#include <syslog.h>
#include "bzsim.h"

/****v* bzsim.c/g_end_time
 *  NAME
 *    g_end_time
 *  SYNOPSIS
 *    static time_t g_start_time = 0, g_end_time = 0;
 *  SOURCE
 */
static time_t   g_start_time = 0, g_end_time = 0;
/************ g_end_time */

/****f* bzsim.c/bzsimDbg
 * NAME
 *  bzsimDbg
 * SYNOPSIS
 *  void bzsimDbg(const char *fmt, ...)
 * FUNCTION
 *  Write messages to stdout
 * EXAMPLE
 *  bzsimDbg("time step = %d\n", t);
 * SOURCE
 */

void                bzsimDbg(const char *fmt, ...)
{
	va_list         ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	va_end(ap);
}

/******/

/****f* bzsim.c/bzsimPanic
 * NAME
 *  bzsimPanic
 * SYNOPSIS
 *  void bzsimPanic(const char *fmt, ...)
 * FUNCTION
 *  Some fatal error has occurred. Write panic message to stderr and exit.
 * EXAMPLE
 *  bzsimPanic("can not open file '%s'", filename);
 * SOURCE
 */

void                bzsimPanic(const char *fmt, ...)
{
	va_list         ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

/************ bzsimPanic */

/****f* bzsim.c/bzsimInit
 * NAME
 *  bzsimInit
 * SYNOPSIS
 *  void bzsimInit(char *progname);
 * FUNCTION
 *  Initialise this library. Currently used only to set application log file.
 *  Side effect is that start time is initialised.
 * INPUTS
 *  char * progname - typically argv[0] but can be anything
 * SEE ALSO
 *  bzsimLogMsg, bzsim_globals
 * SOURCE
 */

void bzsimInit(char *str)
{
	/* argument is typically argv[0] but can be anything (not used) */
	bzsimSetStartTime();
}

/************ bzsimInit */

/****f* bzsim.c/bzsimLogMsg
 * NAME
 *  bzsimLogMsg
 * SYNOPSIS
 *  void bzsimLogMsg(char *fname, const char *fmt, ...)
 * INPUTS
 *  char * fname    - logfile name, NULL means use syslog facility is used
 *  const char *fmt - formatter string
 *  ...             - argument list
 * FUNCTION
 *  Log user or application messages to a given file. If filename is NULL, 
 *  message is passed to the system logger.  See configuration file 
 *  /etc/syslog.conf to which file these messages are actually stored.
 * EXAMPLES
 *  bzsimLogMsg(logfile, "old value = %f, new value =%f\n",  val1, val2);
 *  bzsimLogMsg(NULL, "Hello, syslog!");
 * SEE ALSO
 *  man 3 syslog
 * SOURCE
 */

void                bzsimLogMsg(char *fname, const char *fmt,...)
{
	FILE           *fp;
	va_list         ap;
	char            str[1024];

	if (fname == (char *)NULL) {
		va_start(ap, fmt);
		vsprintf(str, fmt, ap);
		va_end(ap);
		syslog(LOG_INFO, str);
	} else {
		fp = fopen(fname, "a");
		if (fp) {
			va_start(ap, fmt);
			vfprintf(fp, fmt, ap);
			va_end(ap);
			fclose(fp);
		} else {
			fprintf(stderr, "bzsimLogMsg: can not open '%s' for writing\n", fname);
		}
	}
}

/************ bzsimLogMsg */

/****f* bzsim.c/bzsimPBC2D
 * NAME
 *  bzsimPBC2D
 * SYNOPSIS
 *  void bzsimPBC2D(int NX, int NY, int nvar, ...)
 * FUNCTION
 *  2D periodic boundary conditions
 * INPUTS
 *  int NX          - number of columns on grid
 *  int NY          - numbers of row on grid
 *  int nvar        - number of arguments
 *  ...             - rest of the arguments
 * EXAMPLES
 *  #define NX 32
 *  #define NY 32
 *  float var1[NX*NY], var2[NX*NY];
 *  ...
 *  bzsimPBC2D(NX, NY, 2, var1, var2);
 * NOTES
 *  Grid pointer type is float. This should be made more general.
 * SOURCE
 */

void bzsimPBC2D(int NX, int NY, int nvar,...)
{
	float          *x, *xn[BZSIM_MAX_ARG];
	int             i, n, xi, yi, N, NX2;
	va_list         ap;

	assert(nvar < BZSIM_MAX_ARG);
	N = NX * NY;
	NX2 = 2 * NX;
	va_start(ap, nvar);
	for (i = 0; i != nvar; i++) {
		xn[i] = va_arg(ap, float *);
	}
	va_end(ap);
	for (yi = 0, xi = 0; xi != NX; xi++) {
		i = xi + yi * NX;
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			x[i] = x[N - NX2 + xi];
		}
	}
	for (yi = NY - 1, xi = 0; xi != NX; xi++) {
		i = xi + yi * NX;
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			x[i] = x[xi + NX];
		}
	}
	for (xi = 0, yi = 0; yi != NY; yi++) {
		i = xi + yi * NX;
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			x[i] = x[i + NX - 2];
		}
	}
	for (xi = NX - 1, yi = 0; yi != NY; yi++) {
		i = xi + yi * NX;
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			x[i] = x[i - NX + 2];
		}
	}
}

/************ bzsimPBC2D */

/****f* bzsim.c/bzsimSinkBC2D
 * NAME
 *  bzsimSinkBC2D
 * SYNOPSIS
 *  void bzsimSinkBC2D(int NX, int NY, int nvar, ...)
 * FUNCTION
 *  2D sink boundary conditions
 * INPUTS
 *  int NX          - number of columns on grid
 *  int NY          - numbers of row on grid
 *  int nvar        - number of arguments
 *  ...             - rest of the arguments
 * EXAMPLES
 *  #define NX 32
 *  #define NY 32
 *  float var1[NX*NY], var2[NX*NY];
 *  ...
 *  bzsimSinkBC2D(NX, NY, 2, var1, var2);
 * NOTES
 *  Grid pointer type is float. This should be made more general.
 * SEE ALSO
 *  bzsimPBC2D
 * SOURCE
 */
void bzsimSinkBC2D(int NX, int NY, int nvar,...)
{
	float          *x, *xn[BZSIM_MAX_ARG];
	int             i, n, xi, yi, N1, NX1;
	va_list         ap;

	assert(nvar < BZSIM_MAX_ARG);
	N1 = NX * NY - 1;
	NX1 = NX - 1;
	va_start(ap, nvar);
	for (i = 0; i != nvar; i++) {
		xn[i] = va_arg(ap, float *);
	}
	va_end(ap);
	/* top & bottom */
	for (xi = 0; xi != NX; xi++) {
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			/* yi = 0; i = xi + yi * NX; -> i = xi */
			x[xi] = (float) 0;
			/* last row is the NX elems */
			x[N1 - xi] = (float) 0;
		}
	}
	/* left & right */
	for (xi = 0, yi = 0; yi != NY; yi++) {
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			/* xi = 0, i = xi + yi * NX; -> i = yi * NX */
			i = yi * NX;
			x[i] = (float) 0;
			x[i + NX1] = (float) 0;
		}
	}
}

/************ bzsimSinkBC2D */

/****f* bzsim.c/bzsimSinkBC3D
 * NAME
 *  bzsimSinkBC3D
 * SYNOPSIS
 *  void bzsimSinkBC3D(int NX, int NY, int NZ, int nvar, ...)
 * FUNCTION
 *  3D sink boundary conditions
 * INPUTS
 *  int NX          - number of columns on grid
 *  int NY          - numbers of row on grid
 *  int NZ          - numbers of layers on grid
 *  int nvar        - number of arguments
 *  ...             - rest of the arguments
 * EXAMPLES
 *  #define NX 32
 *  #define NY 32
 *  #define NZ 32
 *  float var1[NX*NY*NZ], var2[NX*NY*NZ];
 *  ...
 *  bzsimSinkBC3D(NX, NY, NZ, 2, var1, var2);
 * NOTES
 *  Grid pointer type is float. This should be made more general.
 * SEE ALSO
 *  bzsimPBC3D
 * SOURCE
 */

void bzsimSinkBC3D(int NX, int NY, int NZ, int nvar,...)
{
	float          *x, *xn[BZSIM_MAX_ARG];
	int             i, n, zi, xi, yi, NXNY1, NXNY, N1, NX1;
	va_list         ap;

	assert(nvar < BZSIM_MAX_ARG);
	NXNY = NX * NY;
	NXNY1 = NXNY - NX;
	NX1 = NX - 1;
	N1 = (NXNY * NZ) - 1;
	va_start(ap, nvar);
	for (i = 0; i != nvar; i++) {
		xn[i] = va_arg(ap, float *);
	}
	va_end(ap);
	for (zi = 0; zi != NZ; zi++) {
		for (yi = 0, xi = 0; xi != NX; xi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				/* yi = 0; i = xi + yi * NX; -> i = xi */
				i = xi + /* yi * NX + */ zi * NXNY;	/* TODO */
				x[i] = (float) 0;
				/* last row is the NX elems */
				x[NXNY1 + i] = (float) 0;
			}	/* end n */
		}		/* end xi 0...NX */
		for (xi = 0, yi = 0; yi != NY; yi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				/* xi = 0, i = xi + yi * NX; -> i = yi * NX */
				i = /* xi + */ yi * NX + zi * NXNY;	/* TODO */
				x[i] = (float) 0;
				x[i + NX1] = (float) 0;
			}	/* end n */
		}		/* end yi 0...NY */
	}			/* end zi 0...NZ */
	/* bottom&top layers */
	for (i = 0; i != NXNY; i++) {
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			x[i] = (float) 0;
			x[N1 - i] = (float) 0;
		}		/* end n */
	}			/* end i...NXNY */
}

/************ bzsimSinkBC3D */

/****f* bzsim.c/bzsimNoFluxBC3D
 * NAME
 *  bzsimNoFluxBC3D
 * SYNOPSIS
 *  void bzsimNoFluxBC3D(int NX, int NY, int NZ, int nvar, ...)
 * FUNCTION
 *  3D no-flux boundary conditions
 * INPUTS
 *  int NX          - number of columns on grid
 *  int NY          - numbers of row on grid
 *  int NZ          - numbers of layers on grid
 *  int nvar        - number of arguments
 *  ...             - rest of the arguments
 * EXAMPLES
 *  #define NX 32
 *  #define NY 32
 *  #define NZ 32
 *  float var1[NX*NY*NZ], var2[NX*NY*NZ];
 *  ...
 *  bzsimNoFluxBC3D(NX, NY, NZ, 2, var1, var2);
 * NOTES
 *  Grid pointer type is float. This should be made more general.
 * SEE ALSO
 *  bzsimPBC3D, bzsimSinkBC3D,
 * SOURCE
 */

void bzsimNoFluxBC3D(int nx, int ny, int nz, int nvar,...)
{
	float          *x, *xn[BZSIM_MAX_ARG];
	int             i, n, zi, xi, yi, nxny1, nxny, n1, nx1;
	va_list         ap;

	assert(nvar < BZSIM_MAX_ARG);
	nxny = nx * ny;
	nxny1 = nxny - nx;
	nx1 = nx - 1;
	n1 = (nxny * nz) - 1;
	va_start(ap, nvar);
	for (i = 0; i != nvar; i++) {
		xn[i] = va_arg(ap, float *);
	}
	va_end(ap);
	/* zero-flux in x & y direction */
	for (zi = 0; zi != nz; zi++) {
		yi = 0;
		for (xi = 0; xi != nx; xi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i + nx];
			}	/* end n */
		}		/* end xi */
		yi = ny - 1;
		for (xi = 0; xi != nx; xi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i - nx];
			}
		}
		xi = 0;
		for (yi = 0; yi != ny; yi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i + 1];
			}
		}
		xi = nx - 1;
		for (yi = 0; yi != ny; yi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i - 1];
			}
		}
	}
	/* top & bottom */
	zi = 0;
	for (yi = 0; yi != ny; yi++) {
		for (xi = 0; xi != nx; xi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				zi = 0;
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i + nxny];
				zi = nz - 1;
				i = xi + yi * nx + zi * nxny;
				x[i] = x[i - nxny];
			}
		}
	}
}

/************ bzsimNoFluxBC3D */

/****f* bzsim.c/bzsimPBC3D
 * NAME
 *  bzsimPBC3D
 * SYNOPSIS
 *  void bzsimPBC3D(int NX, int NY, int NZ, int nvar, ...)
 * FUNCTION
 *  3D periodic boundary conditions
 * INPUTS
 *  int NX          - number of columns on grid
 *  int NY          - numbers of row on grid
 *  int NZ          - numbers of layers on grid
 *  int nvar        - number of arguments
 *  ...             - rest of the arguments
 * EXAMPLES
 *  #define NX 32
 *  #define NY 32
 *  #define NZ 32
 *  float var1[NX*NY*NZ], var2[NX*NY*NZ];
 *  ...
 *  bzsimPBC3D(NX, NY, NZ, 2, var1, var2);
 * NOTES
 *  Grid pointer type is float. This should be made more general.
 * SEE ALSO
 *  bzsimSinkBC3D
 * SOURCE
 */

void bzsimPBC3D(int nx, int ny, int nz, int nvar,...)
{
	float          *x, *xn[BZSIM_MAX_ARG];
	int             i, n, i2, zi, yi, nxny, nx2, ny1, nz1, nxm2, n2;
	va_list         ap;
	size_t          size_tmp;

	assert(nvar < BZSIM_MAX_ARG);

	nxny = nx * ny;
	n2 = nxny * (nz - 2);
	ny1 = ny - 1;
	nz1 = nz - 1;
	nx2 = 2 * nx;
	nxm2 = nx - 2;
	size_tmp = (nx - 2) * sizeof(float);
	
	/* pick up pointers from va_arg */
	va_start(ap, nvar);
	for (i = 0; i != nvar; i++) {
		xn[i] = va_arg(ap, float *);
	}
	va_end(ap);
	
	/* x & y direction for all z */
	for (zi = 1; zi != nz1; zi++) {
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			i = 1 + zi * nxny;
			i2 = nxny - nx2 + i;
			memcpy(x + i, x + i2, size_tmp); 
			i2 += nx;
			i += nx;
			memcpy(x + i2, x + i, size_tmp); 
		}

		for (yi = 1; yi != ny1; yi++) {
			for (n = 0; n != nvar; n++) {
				x = xn[n];
				i = yi * nx + zi * nxny;
				i2 = i + nxm2;
				x[i] = x[i2];
				i2++;
				i++;
				x[i2] = x[i];
			}
		}

		/* corners, yes, corners must be accounted for */
		for (n = 0; n != nvar; n++) {
			x = xn[n];
			i = /* xi + yi * nx + */ zi * nxny;
			i2 = i + nxny - nx - 2;
			x[i] = x[i2];
			i2 = i + nxny - nx2 + 1;
			i += nx - 1;
			x[i] = x[i2];
			i2 = zi * nxny + nx + 1;
			i = (zi + 1) * nxny - 1;
			x[i] = x[i2];
			i = (zi + 1) * nxny - nx;
			i2 = zi * nxny + nx2 - 2;
			x[i] = x[i2];
		}
	}

	/* top & bottom */
	size_tmp = nxny * sizeof(float);
	for (n = 0; n != nvar; n++) {
		x = xn[n];
		memcpy(x, x + n2, size_tmp);
		memcpy(x + n2 + nxny, x + nxny, size_tmp);
	}
}

/******/

/****f* bzsim.c/pbc_2D
 *  NAME
 *    pbc_2D
 *  SYNOPSIS
 *    pbc_2D(float *x, float *y, float *z, int NX, int NY)
 *  SOURCE
*/
void                pbc_2D(float *x, float *y, float *z, int NX, int NY)
{
	int             N = NX * NY, xi, yi, i;

	yi = 0;
	for (xi = 0; xi != NX; xi++) {
		i = xi + yi * NX;
		x[i] = x[N - NX - NX + xi];
		y[i] = y[N - NX - NX + xi];
		z[i] = z[N - NX - NX + xi];
	}
	yi = NY - 1;
	for (xi = 0; xi != NX; xi++) {
		i = xi + yi * NX;
		x[i] = x[xi + NX];
		y[i] = y[xi + NX];
		z[i] = z[xi + NX];
	}
	xi = 0;
	for (yi = 0; yi != NY; yi++) {
		i = xi + yi * NX;
		x[i] = x[i + NX - 2];
		y[i] = y[i + NX - 2];
		z[i] = z[i + NX - 2];
	}
	xi = NX - 1;
	for (yi = 0; yi != NY; yi++) {
		i = xi + yi * NX;
		x[i] = x[i - NX + 2];
		y[i] = y[i - NX + 2];
		z[i] = z[i - NX + 2];
	}
	/* end of periodic boundaries */
}

/************ pbc_2D */

/****f* bzsim.c/zero_flux_boundaries_2D
 *  NAME
 *    zero_flux_boundaries_2D
 *  SYNOPSIS
 *    zero_flux_boundaries_2D(float *x, float *y, float *z,
 *  FUNCTION
 *    Apply Neumann boundary conditions to three-variable system. 
 *  SOURCE
 */

void zero_flux_boundaries_2D(float *x, float *y, float *z, int nx, int ny)
{
	int             xi, yi, i;

	yi = 0;
	for (xi = 0; xi != nx; xi++) {
		i = xi + yi * nx;
		x[i] = x[i + nx];
		y[i] = y[i + nx];
		z[i] = z[i + nx];
	}
	yi = ny - 1;
	for (xi = 0; xi != nx; xi++) {
		i = xi + yi * nx;
		x[i] = x[i - nx];
		y[i] = y[i - nx];
		z[i] = z[i - nx];
	}
	xi = 0;
	for (yi = 0; yi != ny; yi++) {
		i = xi + yi * nx;
		x[i] = x[i + 1];
		y[i] = y[i + 1];
		z[i] = z[i + 1];
	}
	xi = nx - 1;
	for (yi = 0; yi != ny; yi++) {
		i = xi + yi * nx;
		x[i] = x[i - 1];
		y[i] = y[i - 1];
		z[i] = z[i - 1];
	}
}

/************ zero_flux_boundaries_2D */

/****f* bzsim.c/zero_flux_boundaries_3D
 *  NAME
 *    zero_flux_boundaries_3D
 *  SYNOPSIS
 *    void zero_flux_boundaries_3D(float *x, int nx, int ny, int nz)
 *  SOURCE
 */

void zero_flux_boundaries_3D(float *x, int nx, int ny, int nz)
{
	int             xi, yi, zi, i, nxny;

	nxny = nx * ny;

	/* Zero-flux in x & y direction */
	for (zi = 0; zi != nz; zi++) {
		yi = 0;
		for (xi = 0; xi != nx; xi++) {
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i + nx];
		}

		yi = ny - 1;
		for (xi = 0; xi != nx; xi++) {
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i - nx];
		}

		xi = 0;
		for (yi = 0; yi != ny; yi++) {
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i + 1];
		}

		xi = nx - 1;
		for (yi = 0; yi != ny; yi++) {
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i - 1];
		}
	}
	/* Top & bottom */
	zi = 0;
	for (yi = 0; yi != ny; yi++) {
		for (xi = 0; xi != nx; xi++) {
			zi = 0;
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i + nxny];
			zi = nz - 1;
			i = xi + yi * nx + zi * nxny;
			x[i] = x[i - nxny];
		}
	}
}

/************ zero_flux_boundaries_3D */

/****f* bzsim.c/bzsimGetPar
 * NAME
 *  bzsimGetPar
 * SYNOPSIS
 *  int bzsimGetPar(char *fname, char *par_name, int par_type, void *par_val)
 * FUNCTION
 *  Read parameters from given input file. The input file format follows
 *  the following syntax. Lines starting with '#' characters are comments
 *  and parameters are defined as ASCII strings as space-separated
 *  name-value pairs. Each line (terminated with LF) can contain at most
 *  one parameter definition.
 *
 *  Each time the function is called the file is opened and each line
 *  read and parsed. Thus in case the requested parameter is defined
 *  in same file more than once, the latest definition will be used.
 * INPUTS
 *  char * fname    - parameter file name
 *  char * par_name - string presenting the parameter
 *  int par_type    - valid values are BZSIM_INT, BZSIM_FLOAT,
 *                    BZSIM_DOUBLE, BZSIM_STRING (bzsim.h)
 *  void * par_val  - pointer to which parsed parameter value is stored
 * EXAMPLES
 *  Consider parameter file called 'test.dat':
 *
 *  # This is comment line, following lines contain definitions
 *  WRKDIR /home/users/tux/wrk
 *  INTEGER_PARAMETER 12345
 *  Float_value 1.23456
 *  ...
 *
 *  void foo(void)
 *  {
 *      int ival;
 *      float fval;
 *      char wrkdir[PATH_MAX];
 *      bzsimGetPar("test.dat", "WRKDIR", BZSIM_STRING, (void*)&wrkdir);
 *      bzsimGetPar("test.dat", "INTEGER_PARAMETER", BZSIM_INT, (void*)&ival);
 *      bzsimGetPar("test.dat", "Float_value", BZSIM_FLOAT, (void*)&fval);
 *      ...
 *  }
 *
 * SEE ALSO
 *  bzsim.h, GET_INT, GET_FLOAT, GET_DOUBLE, GET_STRING
 * SOURCE
 */

int bzsimGetPar(char *fname, char *par_name, int par_type, void *par_val)
{
	int             retval = BZSIM_ERROR;
	FILE           *filep;
	char            line_str[MAX_LINE_LEN], name_str[MAX_LINE_LEN], val_str[MAX_LINE_LEN];

	filep = fopen(fname, "r");
	if (!filep) {
		fprintf(stderr, "bzsimGetPar: can not open file '%s' for reading\n", fname);
		exit(1);
	}
	/* Read the file line by line */
	while (fgets(line_str, MAX_LINE_LEN, filep) != NULL) {
		/*
		 * Line read into line_str[]. Try to read the parameter name
		 * and value pair out from it.
		 */
		if (sscanf(line_str, "%s%s", name_str, val_str) == 2) {
			/* Compare par_name and the name read from the file */
			if (strcmp(par_name, name_str) == 0) {
				/* Name matches the asked one */
				switch (par_type) {
				case BZSIM_INT:
					*((int *) par_val) = atoi(val_str);
					retval = BZSIM_OK;
					break;
				case BZSIM_FLOAT:
					*((float *) par_val) = (float) (atof(val_str));
					retval = BZSIM_OK;
					break;
				case BZSIM_DOUBLE:
					*((double *) par_val) = (double) (atof(val_str));
					retval = BZSIM_OK;
					break;
				case BZSIM_STRING:
					strcpy((char *) par_val, val_str);
					retval = BZSIM_OK;
					break;
				default:
					break;
				}	/* end switch(par_type) */
			}
		}
	}			/* end fgets */
	fclose(filep);
	if (retval == BZSIM_ERROR) {
		fprintf(stderr, "bzsimGetPar: reading parameter %s failed\n", par_name);
		exit(1);	/* should this be flagged out? */
	}
	return retval;
}

/************ bzsimGetPar */

/****f* bzsim.c/bzsimCopyFile
 * NAME
 *  bzsimCopyFile
 * SYNOPSIS
 *  int bzsimCopyFile(const char *file1, const char *file2)
 * FUNCTION
 *  Copy file1 to file2.
 * RETURN VALUE
 *  BZSIM_OK if successful, BZSIM_ERROR otherwise.
 * BUGS
 *  Copying byte-by-byte is very inefficient. Should not use this
 *  for large files.
 * SOURCE
 */

int bzsimCopyFile(const char *file1, const char *file2)
{
	FILE           *ifp, *ofp;
	int             c;

	ifp = fopen(file1, "rb");
	ofp = fopen(file2, "wb");
	if (ifp && ofp) {
		while ((c = getc(ifp)) != EOF) {
			fprintf(ofp, "%c", c);
		}
		fclose(ifp);
		fclose(ofp);
		/* set_web_permissions(fname); */
		return BZSIM_OK;
	}
	return BZSIM_ERROR;
}

/************ bzsimCopyFile */

/****f* bzsim.c/log_par
 *  NAME
 *    log_par
 *  SYNOPSIS
 *    log_par(char *fname, char *msg, double val)
 *  SOURCE
 */
int log_par(char *fname, char *msg, double val)
{
	static char     tmp[256];
	sprintf(tmp, "%s %g\n", msg, val);
	return (log_msg(fname, tmp));
}

/************ log_par */

/****f* bzsim.c/log_msg
 *  NAME
 *    log_msg
 *  SYNOPSIS
 *    log_msg(char *fname, char *msg)
 *  SOURCE
 */
int log_msg(char *fname, char *msg)
{
	FILE           *filep;

	filep = fopen(fname, "a");
	if (!filep) {
		fprintf(stderr, "can not open file %s for writing.", fname);
		return 0;
	}
	fprintf(filep, "%s", msg);
	fclose(filep);
	set_web_permissions(fname);
	return 1;
}

/************ log_msg */

/****f* bzsim/bzsimTruncateFile
 * NAME
 *  bzsimTruncateFile
 * SYNOPSIS
 *  int bzsimTruncateFile(char *fmt, ...)
 * FUNCTION
 *  Make file zero length. This may be useful when re-running same simulations.
 *  Note that bzsimLogMsg appends to file.
 * RETURNS
 *  BZSIM_OK if successful
 * SEE ALSO
 *  bzsimLogMsg
 * SOURCE
 */

int bzsimTruncateFile(char *fmt, ...)
{
	FILE           *fp;
	char            fname[1024];
	va_list         ap;

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);

	if (!(fp = fopen(fname, "w"))) {
		fprintf(stderr, "can not open file %s for writing.", fname);
		return BZSIM_ERROR;
	}

	fclose(fp);

	return BZSIM_OK;
}

/************ bzsimTruncateFile */

/****f* bzsim/bzsimInitRand
 * NAME
 *  bzsimInitRand
 * SYNOPSIS
 *  unsigned int bzsimInitRand(unsigned int seedval);
 * FUNCTION
 *  Initialise random number generator. If given seedval is zero,
 *  value from time() function will be used.
 * RETURNS
 *  Value of seed used. This can be then saved in the calling function.
 * INPUTS
 *  seedval -
 * SEE ALSO
 *  man random, bzsimRand
 * SOURCE
 */
unsigned int bzsimInitRand(unsigned int seedval)
{
	unsigned int    seed;

	if (seedval == 0) {
		seed = (((unsigned int) (time(NULL))) ^ ((unsigned int) getpid()));
		srandom(seed);
		return seed;
	}
	srandom(seedval);
	return seedval;
}
/************ bzsimInitRand */

/****f* bzsim/bzsimRand
 * NAME
 *  bzsimRand
 * SYNOPSIS
 *  float bzsimRand();
 * RETURNS
 *  Random floating-point value between 0 and 1.
 * SEE ALSO
 *  man random, bzsimRand
 * SOURCE
 */

float bzsimRand(void)
{
	return (((float) (random())) / RAND_MAX);
}

/***************/

/****f* bzsim.c/set_web_permissions
 *  NAME
 *    set_web_permissions
 *  SYNOPSIS
 *    set_web_permissions(char *fname)
 *  SOURCE
 */

int set_web_permissions(char *fname)
{
	return (chmod(fname, PERMISSIONS));
}

/************ set_web_permissions */

/****f* bzsim.c/bzsimMkdir
 *  NAME
 *    bzsimMkdir
 *  SYNOPSIS
 *    bzsimMkdir(char *fname)
 *  SOURCE
 */

int bzsimMkdir(char *fname)
{
	if ((mkdir(fname, PERMISSIONS)) == 0) {
		set_web_permissions(fname);
		return BZSIM_OK;
	}
	return BZSIM_ERROR;
}

/************ bzsimMkdir */

/****f* bzsim.c/write_dat
 *  NAME
 *    write_dat
 *  SYNOPSIS
 *    write_dat(char *fname, float *x, int nx, int ny)
 *  SOURCE
 */

void write_dat(char *fname, float *x, int nx, int ny)
{
	FILE           *fp;
	int             xi, yi;
	fp = fopen(fname, "w");
	if (!fp) {
		return;
	}
	for (xi = 0; xi != nx; xi++) {
		for (yi = 0; yi != ny; yi++) {
			fprintf(fp, "%g ", x[xi + nx * yi]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

/************ write_dat */

/****f* bzsim.c/save_grid_image
 *  NAME
 *    save_grid_image
 *  SYNOPSIS
 *    save_grid_image(char *fname, float *x, int NX, int NY, float minval, float maxval)
 *  SOURCE
 */

int save_grid_image(char *fname, float *x, int NX, int NY, float minval, float maxval)
{
	unsigned char  *buf;
	int             res;
	char            newname[1024];

	buf = malloc(NX * NY * sizeof(unsigned char));
	if (!buf) {
		fprintf(stderr, "save_grid_image: out of memomy (%d bytes)\n", (int) NX * NY);
		exit(1);
	}
	bzsimScaleFloatArr(x, buf, NX * NY, minval, maxval);
	sprintf(newname, "%s.jpg", fname);
	res = bzsimWriteJPEG(newname, buf, NX, NY, BZSIM_BW, 100);
	free(buf);
#if 0
	sprintf(newname, "%s.dat", fname);
	write_dat(newname, x, NX, NY);
#endif
	return res;
}

/************ save_grid_image */

/****f* bzsim.c/get_value
 *  NAME
 *    get_value
 *  SYNOPSIS
 *    static double get_value(void *buf, int partype, int i)
 *  SOURCE
 */

static double get_value(void *buf, int partype, int i)
{
	double          res;

	switch (partype) {
	case BZSIM_CHAR:
		res = ((unsigned char *) buf)[i];
		break;
	case BZSIM_INT:
		res = ((int *) buf)[i];
		break;
	case BZSIM_FLOAT:
		res = ((float *) buf)[i];
		break;
	case BZSIM_DOUBLE:
		res = ((double *) buf)[i];
		break;
	default:
		bzsimLogMsg(BZSIM_LOG, "get_value: illegal type %d\n", partype);
		res = BZSIM_ERROR;	/* just to silence pedantic compilers */
		exit(1);
		break;
	}
	return res;
}

/************ get_value */

/****f* bzsim.c/bzsimScaleArr
 *  NAME
 *    bzsimScaleArr
 *  SYNOPSIS
 *    int bzsimScaleArr(void *inbuf, int partype, unsigned char *buf, int n, bzsim_scale_t * s)
 *  SOURCE
 */

int bzsimScaleArr(void *inbuf, int partype, unsigned char *buf, int n, bzsim_scale_t * s)
{
	int             i;
	double          range, maxval = -1e22, minval = 1e22, tmp;

	if (s == (bzsim_scale_t *) NULL) {
		/* use dynamic scaling */
		for (i = 0; i != n; i++) {
			tmp = get_value(inbuf, partype, i);
			if (tmp > maxval)
				maxval = tmp;
			if (tmp < minval)
				minval = tmp;
		}
	} else {
		minval = s->minval;
		maxval = s->maxval;
	}
	range = maxval - minval;
	if (range != (double) 0) {
		for (i = 0; i != n; i++) {
			tmp = get_value(inbuf, partype, i);
			tmp -= minval;
			tmp /= range;
			tmp *= (double) 255;
			assert(tmp < 256);
			buf[i] = (unsigned char) tmp;
		}
	} else {
		for (i = 0; i != n; i++) {
			buf[i] = 0;
		}
	}
	return BZSIM_OK;
}

/************ bzsimScaleArr */

/****f* bzsim.c/bzsimScaleFloatArr
 *  NAME
 *    bzsimScaleFloatArr
 *  SYNOPSIS
 *    int bzsimScaleFloatArr(float *in_data, unsigned char *out_data, int n, float scal_min, float scal_max)
 *  SOURCE
 */

int bzsimScaleFloatArr(float *in_data, unsigned char *out_data, int n, float scal_min, float scal_max)
{
	int             i = 0;
	float           fRange, fmax = -1e22, fmin = 1e22;
	if ((scal_min == (float) 0) && (scal_max == (float) 0)) {
		for (i = 0; i != n; i++) {
			if (in_data[i] > fmax)
				fmax = in_data[i];
			if (in_data[i] < fmin)
				fmin = in_data[i];
		}
	} else {
		fmin = scal_min;
		fmax = scal_max;
	}
	fRange = fmax - fmin;
	if (fRange != (float) 0) {
		float           tmp;
		for (i = 0; i != n; i++) {
			tmp = in_data[i];
			tmp -= fmin;
			tmp /= fRange;
			tmp *= (float) 255;
			out_data[i] = (unsigned char) tmp;
		}
	} else {
		for (i = 0; i != n; i++) {
			out_data[i] = 0;
		}
		return BZSIM_ERROR;
	}
	return BZSIM_OK;
}

/************ bzsimScaleFloatArr */

/****f* bzsim.c/bzsimSetStartTime
 * NAME
 *  bzsimSetStartTime
 * SYNOPSIS
 *  void bzsimSetStartTime(void)
 * FUNCTION
 *  Set global variable.
 * SEE ALSO
 *  bzsimSetEndTime, bzsimGetElapsedTime
 * SOURCE
 */

void bzsimSetStartTime(void)
{
	g_start_time = time(NULL);
}

/************ bzsimSetStartTime */

/****f* bzsim.c/bzsimSetEndTime
 * NAME
 *  bzsimSetEndTime
 * SYNOPSIS
 *  void bzsimSetEndTime(void)
 * FUNCTION
 *  Set global variable.
 * SEE ALSO
 *  bzsimSetStartTime, bzsimGetElapsedTime
 * SOURCE
 */

void bzsimSetEndTime(void)
{
	g_end_time = time(NULL);
}

/************ bzsimSetEndTime */

/****f* bzsim.c/bzsimGetElapsedTime
 * NAME
 *  bzsimGetElapsedTime
 * SYNOPSIS
 *  int bzsimGetElapsedTime(void)
 * FUNCTION
 *  Return time difference from global variables.
 * RETURN VALUE
 *  Time difference in seconds.
 * SEE ALSO
 *  bzsimSetStartTime, bzsimSetEndTime
 * SOURCE
 */

int bzsimGetElapsedTime(void)
{
	return (int) (difftime(g_end_time, g_start_time));
}

/************ bzsimGetElapsedTime */

/****f* bzsim.c/bzsimGetElapsedTime
 * NAME
 *  bzsimGetElapsedTime
 * SYNOPSIS
 *  void bzsimLogElapsedTime(char *logfile)
 * FUNCTION
 *  Log the elapsed wall-clock time between calls to bzsimSetStartTime and 
 *  bzsimSetEndTime.
 * SEE ALSO
 *  bzsimSetStartTime, bzsimSetEndTime
 * SOURCE
 */

void bzsimLogElapsedTime(char *logfile)
{
	time_t t = difftime(g_end_time, g_start_time);
	struct tm *info = gmtime(&t);

	if (t < 86400) {
		bzsimLogMsg(logfile, "elapsed wall-clock time %02d:%02d:%02d\n", info->tm_hour, info->tm_min, info->tm_sec);
	} else {
		int h = t / 86400;
		
		bzsimLogMsg(logfile, "elapsed wall-clock time %d:%02d:%02d\n", h, info->tm_min, info->tm_sec);
	}

}

/************ bzsimLogElapsedTime */

/****f* bzsim.c/bzsimReadData
 * NAME
 *  bzsimReadData
 * SYNOPSIS
 * int bzsimReadData(char *fname, int nx, int ny, int nz,
 *                   void *ptr, float minval, float maxval);
 * FUNCTION
 *  Read data.
 * RETURN VALUE
 *  BZSIM_OK if successfull
 * NOTES
 *  Problems if reading other than bytes.
 * SOURCE
 */

int             bzsimReadData(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt, ...)
{
	int             i, n, nxny;
	unsigned char  *buf, *buf2;
	char            fname[PATH_MAX];
	FILE           *fp;
	va_list         ap;

	nxny = nx * ny;
	n = nxny * nz;

	switch (type) {
	case BZSIM_DATA_FLOAT:
		return BZSIM_ERROR;
		break;
	case BZSIM_DATA_BYTE:
		buf = (unsigned char *) ptr;
		buf2 = XALLOC(nxny, unsigned char);
		break;
	default:
		return BZSIM_ERROR;
		break;
	}

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);
	fp = fopen(fname, "rb");
	if (!fp) {
		return BZSIM_ERROR;
	}
	for (i = 0; i != nz; i++) {
		if (1 != fread(buf2, nxny, 1, fp)) {
			bzsimLogMsg(NULL, "WARNING: short read ('%s', %d)\n", fname, i);
			break;
		}
		memcpy((buf + i * nxny), buf2, nxny);
	}

	fclose(fp);
	XFREE(buf2);

	return BZSIM_OK;
}

/************ bzsimReadData */

/****f* bzsim.c/bzsimSaveData
 * NAME
 *  bzsimSaveData
 * SYNOPSIS
 * int bzsimSaveData(char *fname, int nx, int ny, int nz,
 *                   float *var, float minval, float maxval);
 * FUNCTION
 *  Save data in binary format.
 * RETURN VALUE
 *  BZSIM_OK if successfull
 * SOURCE
 */

int             bzsimSaveData(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt,...)
{
	int             n = nx * ny * nz;
	unsigned char  *buf;
	char            fname[PATH_MAX];
	FILE           *fp;
	va_list         ap;
	int             res = BZSIM_ERROR;

	switch (type) {
	case BZSIM_DATA_FLOAT:
		buf = XALLOC(n, unsigned char);
		bzsimScaleFloatArr((float *) ptr, buf, n, minval, maxval);
		break;
	case BZSIM_DATA_BYTE:
		buf = (unsigned char *) ptr;
		break;
	default:
		return BZSIM_ERROR;
		break;
	} /* end switch(type) */

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);

	if ((fp = fopen(fname, "wb"))) {
		fwrite(buf, n, 1, fp);
		fclose(fp);
		res = BZSIM_OK;
	}

	if (type != BZSIM_DATA_BYTE)
		XFREE(buf);

	return res;
}

/************ bzsimSaveData */

/****f* bzsim.c/bzsimSaveDf3
 * NAME
 *  bzsimSaveDf3
 * SYNOPSIS
 * int bzsimSaveDf3(char *fname, int nx, int ny, int nz,
 *                   float *var, float minval, float maxval);
 * FUNCTION
 *  Save data in povray df3 density file format.
 * RETURN VALUE
 *  BZSIM_OK if successfull
 * SEE ALSO
 *  http://astronomy.swin.edu.au/~pbourke/povray/df3/
 * SOURCE
 */

int             bzsimSaveDf3(void *ptr, int nx, int ny, int nz, int type, double minval, double maxval, char *fmt, ...)
{
	int             n = nx * ny * nz;
	unsigned char  *buf;
	char            fname[PATH_MAX];
	FILE           *fp;
	va_list         ap;
	int             res = BZSIM_ERROR;

	assert(ptr);
#if 0 /* __APPLE__ has problems with these asserts */
	assert(((nx > 0) && (nx =< (0xffff))));
	assert((ny > 0) && (ny < 65536));
	assert(nz > 0 && nz =< 0xffff);
#endif

	switch (type) {
	case BZSIM_DATA_FLOAT:
		buf = XALLOC(n, unsigned char);
		bzsimScaleFloatArr((float *) ptr, buf, n, minval, maxval);
		break;
	case BZSIM_DATA_BYTE:
		buf = (unsigned char *) ptr;
		break;
	default:
		return BZSIM_ERROR;
		break;
	} /* end switch(type) */

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);

	if ((fp = fopen(fname, "wb"))) {
		fputc(nx >> 8, fp);
		fputc(nx & 0xff, fp);
		fputc(ny >> 8, fp);
		fputc(ny & 0xff, fp);
		fputc(nz >> 8, fp);
		fputc(nz & 0xff, fp);
		fwrite(buf, n, 1, fp);
		fclose(fp);
		res = BZSIM_OK;
	}

	if (type != BZSIM_DATA_BYTE)
		XFREE(buf);
	
	return res;
}

/************ bzsimSaveDf3 */

/****f* bzsim.c/bzsimSaveData2
 * NAME
 *  bzsimSaveData2
 * SYNOPSIS
 *  int bzsimSaveData2(int nx, int ny, int nz, int type, void *ptr, 
 *                     double minval, double maxval, char *fmt, ...)
 * FUNCTION
 *  Save data in ASCII format.
 * RETURN VALUE
 *  BZSIM_OK if successfull
 * SOURCE
 */

int bzsimSaveData2(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt,...)
{
	int             i, 	n = nx * ny * nz;
	char            fname[PATH_MAX];
	FILE           *fp;
	va_list         ap;

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);
	if (!(fp = fopen(fname, "wb")))
		return BZSIM_ERROR;
	
	/* three integers for grid dimensions */
	fprintf(fp, "%d %d %d", nx, ny, nz);
	for (i = 0; i != n; i++) {
		switch (type) {
		case BZSIM_DATA_FLOAT:
			fprintf(fp, " %g", *(((float *) ptr) + i));
			break;
		case BZSIM_DATA_BYTE:
			/* FIXME */
			break;
		default:
			break;
		}
	}			/* end i...n */
	fclose(fp);

	return BZSIM_OK;
}

/************ bzsimSaveData2 */

/****f* bzsim.c/_xfree
 *  NAME
 *    _xfree
 *  SYNOPSIS
 *    void _xfree(void *ptr)
 *  SEE ALSO
 *    XALLOC, XFREE, _xalloc
 *  SOURCE
 */

void _xfree(void *ptr)
{
	if (ptr)
		free(ptr);
}

/************ _xfree */

/****f* bzsim.c/_xalloc
 *  NAME
 *    _xalloc
 *  SYNOPSIS
 *    void * _xalloc(size_t n)
 *  SEE ALSO
 *    XALLOC, XFREE, _xfree
 *  SOURCE
 */

void * _xalloc(size_t n)
{
	void *p;
	
	if (!(p = malloc(n)))
		bzsimPanic("_xalloc: memory allocation failed (%ld)\n", (long int) n);

	memset(p, 0, n); /* null memory */
	
	return p;
}

/************ _xalloc */

/****f* bzsim.c/bzsimSavePid
 *  NAME
 *    bzsimSavePid
 *  SYNOPSIS
 *    void bzsimSavePid(char *dir)
 *  DESCRIPTION
 *    Save process ID to a file. Typically the argument is the simulation
 *    output directory. This is useful if one wants to terminate some
 *    simulation. For instance:
 *        kill `cat results/sim_xyz/pid`
 *  SEE ALSO
 *    bzsimRemovePid
 *  SOURCE
 */

int bzsimSavePid(char *dir)
{
	char path[PATH_MAX];
	FILE *fp;
	int pid;

	assert(PATH_MAX > (strlen(dir) + 4));
	sprintf(path, "%s/pid", dir);
	if (!(fp = fopen(path, "w")))
		bzsimPanic("bzsimSavePid: can not write '%s'", path);
	pid = getpid();
	fprintf(fp, "%d\n", pid);
	fclose(fp);

	return pid;
}

/************ bzsimSavePid */

/****f* bzsim.c/bzsimRemovePid
 *  NAME
 *    bzsimRemovePid
 *  SYNOPSIS
 *    void bzsimRemovePid(char *dir)
 *  DESCRIPTION
 *    Remove process ID file saved by bzsimSavePid.
 *  RETURN VALUE
 *    0 if successful, non-zero if unlink failed
 *  SEE ALSO
 *    bzsimSavePid
 *  SOURCE
 */

int bzsimRemovePid(char *dir)
{
	char path[PATH_MAX];

	assert(PATH_MAX > (strlen(dir) + 4));
	sprintf(path, "%s/pid", dir);

	return unlink(path);
}

/************ bzsimSavePid */

/****f* bzsim.c/bzsimGridSnap
 *  NAME
 *    bzsimGridSnap
 *  SYNOPSIS
 *    bzsimGridSnap
 *  TODO
 *    This is locked to float variables only. Make this more general.
 *  SOURCE
 */

int			    bzsimGridSnap(void *ptr, int nx, int ny, int nz, int type, int dir, int layer, float scalmin, float scalmax, char *fmt, ...)
{	
	int             src, xi = 0, yi = 0, zi = 0, i = 0;
	unsigned char  *buf;
	char            fname[PATH_MAX];
	va_list         ap;
	int             res = BZSIM_ERROR;
	int             rx = 0, ry = 0;
	void           *tmp;

#if 0 /* __APPLE__ has problems with these asserts */
	assert(nx > 0 && nx =< 0xffff);
	assert(ny > 0 && ny =< 0xffff);
	assert(nz > 0 && nz =< 0xffff);
#endif

	assert(type == BZSIM_FLOAT);

	/* which layer is kept constant? */
	switch (dir) {
	case BZSIM_X:
		rx = ny;
		ry = nz;
		xi = layer;
		assert(layer < nx);
		break;
	case BZSIM_Y:
		rx = nx;
		ry = nz;
		yi = layer;
		assert(layer < ny);
		break;
	case BZSIM_Z:
		rx = nx;
		ry = ny;
		zi = layer;
		assert(layer < nz);
		break;
	default:
		bzsimPanic("bzsimGridSnap: unknown direction: %d", dir);
		break;
	} /* end switch(dir) */

	buf = XALLOC(rx*ry, unsigned char);
	tmp = XALLOC(rx*ry, float);

	switch (dir) {
	case BZSIM_X:
		for (zi = 0; zi != nz; zi++) {
			for (yi = 0; yi != ny; yi++) {
				src = xi + yi*nx + zi*nx*ny;
				*(((float *) tmp) + i) = *(((float *) ptr) + src);
				i++;
			}
		}
		break;
	case BZSIM_Y:
		for (zi = 0; zi != nz; zi++) {
			for (xi = 0; xi != nx; xi++) {
				src = xi + yi*nx + zi*nx*ny;
				*(((float *) tmp) + i) = *(((float *) ptr) + src);
				i++;
			}
		}
		break;
	case BZSIM_Z:
		for (yi = 0; yi != ny; yi++) {
			for (xi = 0; xi != nx; xi++) {
				src = xi + yi*nx + zi*nx*ny;
				*(((float *) tmp) + i) = *(((float *) ptr) + src);
				i++;
			}
		}
		break;
	default:
		bzsimPanic("bzsimGridSnap: unknown direction: %d", dir);
		break;
	}

	va_start(ap, fmt);
	vsprintf(fname, fmt, ap);
	va_end(ap);

	bzsimScaleFloatArr(tmp, buf, rx*ry, scalmin, scalmax);
	res = bzsimWriteJPEG(fname, buf, rx, ry, BZSIM_BW, 100);

	XFREE(buf);
	XFREE(tmp);

	return res;	
}

/************ bzsimGridSnap */

/****f* bzsim.c/bzsimRK4
 *  NAME
 *    bzsimRK4
 *  SYNOPSIS
 *    bzsimRK4
 *  SOURCE
 */

void bzsimRK4(float t, float *x0, float *xout, float h, int n, void *data,
			  void (*xdot)(float, float *, float *, void *))
{
	float *k1, *k2, *k3, *k4, *x;
	int i;
	
	k1 = XALLOC(n, float);
	k2 = XALLOC(n, float);
	k3 = XALLOC(n, float);
	k4 = XALLOC(n, float);
	x = XALLOC(n, float);

	/* k1 */
	for (i = 0; i != n; i++) 
		x[i] = x0[i];
	(*xdot)(t, x, xout, data);
	for (i = 0; i != n; i++)
		k1[i] = h * xout[i];

	/* k2 */
	for (i = 0; i != n; i++)
		x[i] = x0[i] + 0.5 * k1[i];
	(*xdot)(t, x, xout, data);
	for (i = 0; i != n; i++)
		k2[i] = h * xout[i];
	
	/* k3 */
	for (i = 0; i != n; i++)
		x[i] = x0[i] + 0.5 * k2[i];
	(*xdot)(t, x, xout, data);
	for (i = 0; i != n; i++)
		k3[i] = h * xout[i];

	/* k4 */
	for (i = 0; i != n; i++)
		x[i] = x0[i] + k3[i];
	(*xdot)(t, x, xout, data);
	for (i = 0; i != n; i++)
		k4[i] = h * xout[i];

	/* result */
	for (i = 0; i != n; i++)
		xout[i] = x0[i] + k1[i]/6.0 + k2[i]/3.0 + k3[i]/3.0 + k4[i]/6.0;

	XFREE(k1);
	XFREE(k2);
	XFREE(k3);
	XFREE(k4);
	XFREE(x);
}

/*******/

/****f* bzsim.c/bzsimExec
 * NAME
 *  bzsimExec
 * SYNOPSIS
 *  int bzsimExec(char *fmt, ...)
 * INPUTS
 *  const char *fmt - formatter string
 *  ...             - argument list
 * FUNCTION
 *  Execute external program synchronously.
 * RETURN VALUE
 *  The exit status of child process.
 * EXAMPLES
 *  bzsimExec("convert -depth 8 -size %dx%d gray:%s/xt.dat PNG:%s/xt.png", 
 *             nx, ny, OUTPUT_DIR, OUTPUT_DIR);
 * SOURCE
 */

int                bzsimExec(char *fmt, ...)
{
	va_list         ap;
	char            cmd[1024];
	int ret;

	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	va_end(ap);
	ret = system(cmd);

	return ret;
}

/*******/


float * bzsimReadFloatGrid(char *path, int *x, int *y, int *z, float *vmin, float *vmax)
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
	*vmin = fMin;

#if _DEBUG_
	fprintf(stderr, "%s: min = %f max = %f\n", path, fMin, fMax);
#endif /* _DEBUG_ */

	return res;
}
