/****h* bzsim/bzsim.h
 *  NAME
 *    bzsim.h
 *  DESCRIPTION
 *    Header file for bzsim library module.
 *********/

#ifndef __BZSIM_H__
#define __BZSIM_H__

/****f* bzsim/common_includes
 *  NAME
 *    common_includes
 *  DESCRIPTION
 *    All non-standard includes should be put here.
 *  SOURCE
 */

#if !defined(__APPLE__)

#include <linux/limits.h>
#include <values.h>

#else /* __APPLE__ */

/* this should work for Darwin and BSD variants, change as needed */
#include <sys/syslimits.h>
#include <float.h>

#endif

#include <assert.h>

/******/

/****s* bzsim/bzsim_scale_t
 *  NAME
 *    bzsim_scale_t
 *  SOURCE
 */

typedef struct _bzsim_scale_t {
	double          minval;
	double          maxval;
}               bzsim_scale_t;

/***** end bzsim_scale_t */

/****v* bzsim.h/bzsimRandInt
 *  NAME
 *    bzsimRandInt
 *  SYNOPSIS
 *    bzsimRandInt(int imax)
 *  FUNCTION
 *    Return random integer number between 0 and imax-1. The argument maxval
 *    is first truncated to int (just to make sure...).
 *  SOURCE
 */
#define bzsimRandInt(imax) ((int)(((float) ((int)imax))*rand()/(RAND_MAX + 1.0)))
/*****/

/* legacy functions */
int             log_par(char *fname, char *msg, double val);
int             log_msg(char *fname, char *msg);
void            pbc_2D(float *x, float *y, float *z, int nx, int ny);
int             save_grid_image(char *fname, float *x, int NX, int NY, float SCAL_MIN, float SCAL_MAX);
int             set_web_permissions(char *fname);
void            zero_flux_boundaries_2D(float * x, float * y, float * z, int nx, int ny);
void            zero_flux_boundaries_3D(float * x, int nx, int ny, int nz);

/* internal functions, these should remain hidden from users */
void          * _xalloc(size_t n);
void            _xfree(void *ptr);

/* loosely formalised `bzsim' functions */
int             bzsimCopyFile(const char *file1, const char *file2);
void            bzsimDbg(const char *fmt, ...);
int             bzsimExec(char *fmt, ...);
int             bzsimGetElapsedTime(void);
void            bzsimLogElapsedTime(char *fname);
int             bzsimGetPar(char *fname, char *par_name, int par_type, void *par_val);
int			    bzsimGridSnap(void *ptr, int nx, int ny, int nz, int type, int dir, int layer, float scalmin, float scalmax, char *fmt, ...);
void            bzsimInit(char *progname);
unsigned int    bzsimInitRand(unsigned int seedval);
int             bzsimMkdir(char *fname);
void            bzsimNoFluxBC2D(int NX, int NY, int nvar,...);
void            bzsimNoFluxBC3D(int NX, int NY, int NZ, int nvar,...);
int             bzsimScaleFloatArr(float *in_data, unsigned char *out_data, int n, float scal_min, float scal_max);
int             bzsimScaleArr(void *inbuf, int partype, unsigned char *buf, int n, bzsim_scale_t * s);
void            bzsimLogMsg(char *fname, const char *fmt,...);
void            bzsimPanic(const char *fmt, ...);
void            bzsimPBC2D(int nx, int ny, int nvar, ...);
void            bzsimPBC3D(int nx, int ny, int nz, int nvar, ...);
float           bzsimRand(void);
int             bzsimReadData(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt, ...);
unsigned char * bzsimReadJPEG(char *filename, int *nx, int *ny, int *nz);
int             bzsimSaveData(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt, ...);
int             bzsimSaveData2(int nx, int ny, int nz, int type, void *ptr, double minval, double maxval, char *fmt, ...);
int             bzsimSaveDf3(void *ptr, int nx, int ny, int nz, int type, double minval, double maxval, char *fmt,...);
int             bzsimSavePid(char *dir);
int             bzsimRemovePid(char *dir);
int             bzsimSavePGM(char *fname, void *buf, int partype, int nx, int ny, bzsim_scale_t * s);
void            bzsimSetStartTime(void);
void            bzsimSetEndTime(void);
void            bzsimSinkBC2D(int nx, int ny, int nvar, ...);
void            bzsimSinkBC3D(int nx, int ny, int nz, int nvar, ...);
int             bzsimTruncateFile(char *fmt, ...);
int             bzsimWriteJPEG(char *fname, unsigned char *pix_data, int nx, int ny, int depth, int quality);
int             bzsimWritePGM(char *fname, unsigned char *buf, int nx, int ny);

/****v* bzsim.h/BZSIM_X
 *  NAME
 *    BZSIM_X
 *  SOURCE
 */
#define BZSIM_X 0
/************ BZSIM_X */

/****v* bzsim.h/BZSIM_Y
 *  NAME
 *    BZSIM_Y
 *  SOURCE
 */
#define BZSIM_Y 1
/************ BZSIM_Y */

/****v* bzsim.h/BZSIM_Z
 *  NAME
 *    BZSIM_Z
 *  SOURCE
 */
#define BZSIM_Z 2
/************ BZSIM_Z */

/****v* bzsim.h/PARTYPE_INT
 *  NAME
 *    PARTYPE_INT
 *  SOURCE
 */
#define PARTYPE_INT 0
/************ PARTYPE_INT */

/****v* bzsim.h/BZSIM_INT
 *  NAME
 *    BZSIM_INT
 *  SOURCE
 */
#define BZSIM_INT 0
/************ BZSIM_INT */

/****v* bzsim.h/PARTYPE_FLOAT
 *  NAME
 *    PARTYPE_FLOAT
 *  SOURCE
*/
#define PARTYPE_FLOAT 1
/************ PARTYPE_FLOAT */

/****v* bzsim.h/BZSIM_FLOAT
 *  NAME
 *    BZSIM_FLOAT
 *  SOURCE
 */
#define BZSIM_FLOAT 1
/************ BZSIM_FLOAT */

/****v* bzsim.h/PARTYPE_DOUBLE
 *  NAME
 *    PARTYPE_DOUBLE
 *  SOURCE
 */
#define PARTYPE_DOUBLE 2
/************ PARTYPE_DOUBLE */

/****v* bzsim.h/BZSIM_DOUBLE
 *  NAME
 *    BZSIM_DOUBLE
 *  SOURCE
 */
#define BZSIM_DOUBLE 2
/************ BZSIM_DOUBLE */

/****v* bzsim.h/PARTYPE_STRING
 *  NAME
 *    PARTYPE_STRING
 *  SOURCE
*/
#define PARTYPE_STRING 3
/************ PARTYPE_STRING */

/****v* bzsim.h/BZSIM_STRING
 *  NAME
 *    BZSIM_STRING
 *  SOURCE
 */
#define BZSIM_STRING 3
/************ BZSIM_STRING */

/****v* bzsim.h/BZSIM_CHAR
 *  NAME
 *    BZSIM_CHAR
 *  SOURCE
 */
#define BZSIM_CHAR 4
/************ BZSIM_CHAR */

/****v* bzsim.h/MAX_LINE_LEN
 *  NAME
 *    MAX_LINE_LEN
 *  SOURCE
 */
#define MAX_LINE_LEN 1024
/************ MAX_LINE_LEN */

/****v* bzsim.h/MAX_PAR_LEN
 *  NAME
 *    MAX_PAR_LEN
 *  SOURCE
 */
#define MAX_PAR_LEN 128
/************ MAX_PAR_LEN */

/****v* bzsim.h/PERMISSIONS
 *  NAME
 *    PERMISSIONS
 *  SOURCE
 */
#define PERMISSIONS (S_IRWXU | S_IROTH | S_IXOTH | S_IRGRP | S_IXGRP)
/************ PERMISSIONS */

/****v* bzsim.h/BZSIM_LOG
 *  NAME
 *    BZSIM_LOG
 *  SOURCE
 */
#define BZSIM_LOG ((char *) NULL)
/************ BZSIM_LOG */

/****v* bzsim.h/BZSIM_OK
 *  NAME
 *    BZSIM_OK
 *  SOURCE
 */
#define BZSIM_OK 0
/************ BZSIM_OK */

/****v* bzsim.h/BZSIM_ERROR
 *  NAME
 *    BZSIM_ERROR
 *  SOURCE
 */
#define BZSIM_ERROR 1
/************ BZSIM_ERROR */

/****v* bzsim.h/BZSIM_BW
 *  NAME
 *    BZSIM_BW
 *  SOURCE
 */
#define BZSIM_BW 1
/************ BZSIM_BW */

/****v* bzsim.h/BZSIM_COLOUR
 *  NAME
 *    BZSIM_COLOUR
 *  SOURCE
*/
#define BZSIM_COLOUR 3
/************ BZSIM_COLOUR */

/****v* bzsim.h/BZSIM_MAX_ARG
 *  NAME
 *    BZSIM_MAX_ARG
 *  SOURCE
 */
#define BZSIM_MAX_ARG 16
/************ BZSIM_MAX_ARG */

/****v* bzsim.h/BZSIM_DATA_8BIT_BINARY
 *  NAME
 *    BZSIM_DATA_8BIT_BINARY
 *  SOURCE
 */
#define BZSIM_DATA_8BIT_BINARY 0 
/************ BZSIM_DATA_8BIT_BINARY */

/****v* bzsim.h/BZSIM_DATA_BYTE
 *  NAME
 *    BZSIM_DATA_BYTE
 *  SOURCE
*/
#define BZSIM_DATA_BYTE   0
/************ BZSIM_DATA_BYTE */

/****v* bzsim.h/BZSIM_DATA_INT
 *  NAME
 *    BZSIM_DATA_INT
 *  SOURCE
 */
#define BZSIM_DATA_INT    1
/************ BZSIM_DATA_INT */

/****v* bzsim.h/BZSIM_DATA_UINT
 *  NAME
 *    BZSIM_DATA_UINT
 *  SOURCE
 */
#define BZSIM_DATA_UINT   2
/************ BZSIM_DATA_UINT */

/****v* bzsim.h/BZSIM_DATA_FLOAT
 *  NAME
 *    BZSIM_DATA_FLOAT
 *  SOURCE
*/
#define BZSIM_DATA_FLOAT  3
/************ BZSIM_DATA_FLOAT */

/****v* bzsim.h/BZSIM_DATA_DOUBLE
 *  NAME
 *    BZSIM_DATA_DOUBLE
 *  SOURCE
 */
#define BZSIM_DATA_DOUBLE 4
/************ BZSIM_DATA_DOUBLE */

/*
 * Macros for reading parameters.
 */

/****v* bzsim.h/GET_INT
 *  NAME
 *    GET_INT
 *  SOURCE
 */
#define GET_INT(f,n,p) (bzsimGetPar(f, n, BZSIM_INT, (void *)&p));
/************ GET_INT */

/****v* bzsim.h/GET_FLOAT
 *  NAME
 *    GET_FLOAT
 *  SOURCE
 */
#define GET_FLOAT(f,n,p) (bzsimGetPar(f, n, BZSIM_FLOAT, (void *)&p));
/************ GET_FLOAT */

/****v* bzsim.h/GET_DOUBLE
 *  NAME
 *    GET_DOUBLE
 *  SOURCE
 */
#define GET_DOUBLE(f,n,p) (bzsimGetPar(f, n, BZSIM_DOUBLE, (void *)&p));
/************ GET_DOUBLE */

/****v* bzsim.h/GET_STRING
 *  NAME
 *    GET_STRING
 *  SOURCE
 */
#define GET_STRING(f,n,p) (bzsimGetPar(f, n, BZSIM_STRING, (void *)&p));
/************ GET_STRING */

/****v* bzsim.h/LOG_PAR
 *  NAME
 *    LOG_PAR
 *  SOURCE
 */
#define LOG_PAR(f,n,v) (log_par(f,n,(double) v))
/************ LOG_PAR */

/****v* bzsim.h/LOG_STR
 *  NAME
 *    LOG_STR
 *  SOURCE
*/
#define LOG_STR(f,n,v) (bzsimLogMsg(f,"%s %s\n",n,v))
/************ LOG_STR */

/****v* bzsim.h/CLEAN_FILE
 *  NAME
 *    CLEAN_FILE
 *  SOURCE
 */
#define CLEAN_FILE(f) (log_clean(f))
/************ CLEAN_FILE */

/****v* bzsim.h/BZSIM_SHELL
 *  NAME
 *    BZSIM_SHELL
 *  SOURCE
*/
#define BZSIM_SHELL "/bin/sh"
/************ BZSIM_SHELL */

/****v* bzsim.h/XALLOC
 *  NAME
 *    XALLOC
 *  SYNOPSIS
 *    XALLOC(n, t)
 *  FUNCTION
 *    Do approriate casting and pass the call to _xalloc which actually
 *    carries out the real work.
 *  EXAMPLE
 *    unsigned char * buf = XALLOC(512, unsigned char); 
 *  SEE ALSO
 *    XFREE
 *  SOURCE
 */
#define XALLOC(n, t) (t *) _xalloc(n * sizeof(t))
/*****/

/****v* bzsim.h/XFREE
 *  NAME
 *    XFREE
 *  FUNCTION
 *    Free allocated memory. Call passed on to _xfree function.
 *  SEE ALSO
 *    XALLOC
 *  SOURCE
 */
#define XFREE(p) _xfree(p)
/*****/

/****v* bzsim.h/LAPL1D
 *  NAME
 *    LAPL1D
 *  DESCRIPTION
 *    Three-point Laplacian mask in 1D.
 *  SOURCE
 */

#define LAPL1D(x, i) (x[i - 1] + x[i + 1] - 2.0 * x[i])

/*****/

/****v* bzsim.h/LAPL3D
 *  NAME
 *    LAPL3D
 *  DESCRIPTION
 *    Seven-point Laplacian mask in 3D.
 *  SOURCE
 */

#define LAPL3D(x, i, nx, nxny) (x[i - 1] + x[i + 1] + x[i + nx] + x[i - nx] \
                                + x[i + nxny] + x[i - nxny] - 6.0 * x[i])

/*****/

void bzsimRK4(float t, float *x0, float *xout, float h, int n, void *data,
			  void (*xdot)(float, float *, float *, void *));

#endif				/* __BZSIM_H__ */

