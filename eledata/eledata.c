/* $Id: eledata.c,v 1.1.1.1 2003/07/05 15:08:18 petterik Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

#ifdef _USEGL
#ifdef _MACOSX
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#endif /* _USEGL */

#include "bzsim.h"

#define MAXLINELEN 102400
#define ELE_NX 8
#define ELE_NY 8
#define MYTOKENS (ELE_NX * ELE_NY + 1) /* time plus number of electrodes */
#define MAXTOKENS 1024
#define MAXLINES 1024
#define PGM_OUTPUT 1 /* 1 = PGM, 0 = JPG */
#define N_COMBINE 4 /* number of samples for calculationg average */
 
static int readFile(char *datafile, float *data, float *t, int n, int maxlines);
static void getMinMax(float *data, int n, float *fmin, float *fmax);
static int saveImg(char *prefix, float *data, int n, int filenum, float fmin, float fmax);
static void calcBas(float *data, float *data2, int cols, int n);
static int calcAvg(float *data, float *data2, int cols, int n, int nframes);
static void calcSub(float *in, float *out, int cols, int n);

#ifdef _USEGL

GLfloat ctrlpoints[6][3] = {
	{ -4.0, -4.0, 0.0}, { -2.0, 4.0, 0.0}, 
	{2.0, -4.0, 0.0}, {4.0, 4.0, 0.0}, {6.0, 8.0, 0.0}, {8.0, 6.0, 0.0}};

void init(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 6, &ctrlpoints[0][0]);
	glEnable(GL_MAP1_VERTEX_3);
}

void display(void)
{
	int i;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

#if 0
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= 30; i++) 
		glEvalCoord1f((GLfloat) i/30.0);
	glEnd();
#else
	glBegin(GL_POINTS);
	for (i = 0; i <= 30; i++) 
		glEvalCoord1f((GLfloat) i/30.0);
	glEnd();
#endif

	glTranslatef(2.5, 0.0, 0.0);
	glBegin(GL_LINE_STRIP);
	for (i = 0; i <= 30; i++) 
		glEvalCoord1f((GLfloat) i/30.0);
	glEnd();


	/* The following code displays the control points as dots. */
	glPointSize(5.0);
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_POINTS);
	for (i = 0; i < 6; i++) 
		glVertex3fv(&ctrlpoints[i][0]);
	glEnd();
	glFlush();
}

void reshape(int w, int h)
{
#define SZ 9.0
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-SZ, SZ, -SZ*(GLfloat)h/(GLfloat)w, 
				SZ*(GLfloat)h/(GLfloat)w, -SZ, SZ);
	else
		glOrtho(-SZ*(GLfloat)w/(GLfloat)h, 
				SZ*(GLfloat)w/(GLfloat)h, -SZ, SZ, -SZ, SZ);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 27:
		exit(0);
		break;
	}
}

#endif /* _USEGL */
	
int main(int argc, char** argv)
{
	float *data, *data2, *data3, *data4, *data5, *t, fmin, fmax;
	int i, n, cols = MYTOKENS - 1, n2;
	
	if (argc != 2) {
		fprintf(stderr, "usage: %s <input CSV file>\n", argv[0]);
		exit(1);
	}

	data = XALLOC((cols * MAXLINES), float);
	data2 = XALLOC((cols * MAXLINES), float);
	data3 = XALLOC((cols * MAXLINES), float);
	data4 = XALLOC((cols * MAXLINES), float);
	data5 = XALLOC((cols * MAXLINES), float);
	t = XALLOC(MAXLINES, float); 

	n = readFile(argv[1], data, t, MYTOKENS, MAXLINES);
	assert(n > 0);

	/* Transform to real voltage. */
	for (i = 0; i < (cols * n); i++) {
		data[i] = pow(10.0, data[i]);
	}

	/* 
	 * Calculate avg for each electrode value. This should cancel out some
	 * random noise. 
	 */
	calcBas(data, data2, cols, n);

	/*
	 * Make a set of subtracted frames. In ideal case this shows only what 
	 * is changed between subsequent frames.
	 */
	calcSub(data2, data3, cols, n);
	
	/* Average N_COMBINE frames from data */
	n2 = calcAvg(data, data4, cols, n, N_COMBINE);

	/* Calculate subtracted frames from the averaged data */
	calcSub(data4, data5, cols, n2);

	/* Write out all the frames */
	getMinMax(data, n*cols, &fmin, &fmax);
	for (i = 0; i < n; i++)
		saveImg("raw", (data + i * cols), cols, i, fmin, fmax);
	getMinMax(data2, n*cols, &fmin, &fmax);
 	for (i = 0; i < n; i++)
		saveImg("bas", (data2 + i * cols), cols, i, fmin, fmax);
	getMinMax(data3, n*cols, &fmin, &fmax);
 	for (i = 0; i < (n-1); i++)
		saveImg("sub", (data3 + i * cols), cols, i, 0, 0);
	getMinMax(data4, n*cols, &fmin, &fmax);
 	for (i = 0; i < n2; i++)
		saveImg("avg", (data4 + i * cols), cols, i, fmin, fmax);
	getMinMax(data5, n*cols, &fmin, &fmax);
 	for (i = 0; i < (n2-1); i++)
		saveImg("fnl", (data5 + i * cols), cols, i, fmin, fmax);

	{
		char *path = "fnl.csv";
		FILE *fp = fopen(path, "wb");
		float dt = 0.1 * N_COMBINE;
		int frame;

		assert(fp);
		for (frame = 0; frame < (n2-1); frame++) {
			fprintf(fp, "%g,", frame*dt);
			for (i = 0; i != cols; i++) {
				fprintf(fp, "%g%s", data5[frame*cols + i], (i == (cols - 1) ? "" : ","));
			}
			fprintf(fp, "\n");
		}
		fclose(fp);
	}


#ifdef _USEGL
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize (1000, 500);
	glutInitWindowPosition (100, 100);
	glutCreateWindow (argv[0]);
	init ();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc (keyboard);
	glutMainLoop();
#endif /* _USEGL */

	XFREE(data);
	XFREE(data2);
	XFREE(data3);
	XFREE(data4);
	XFREE(data5);
	XFREE(t);

	return 0;
}

static void getMinMax(float *data, int n, float *fmin, float *fmax)
{
	int i;

	*fmin = FLT_MAX;
	*fmax = FLT_MIN;

	for (i = 0; i < n; i++) {
		*fmin = (data[i] < *fmin ? data[i] : *fmin);
		*fmax = (data[i] > *fmax ? data[i] : *fmax);
	}
	
	/*fprintf(stderr, "fmin = %f fmax = %f\n", *fmin, *fmax);*/
}

static int readFile(char *datafile, float *data, float *t, int cols, int maxlines)
{
	int nline = 0, nx = cols - 1;
	FILE *fp = fopen(datafile, "rb");

	if (!fp) {
		perror("fopen");
		exit(1);
	}

	while (!feof(fp)) {		
		char buffer[MAXLINELEN];
		char *retval = fgets(buffer, sizeof(buffer), fp);
		char *p = buffer;
		char *tokens[MAXTOKENS];
		int i, num_tokens = 0, bytes;

		if (retval == NULL)
			break;

		assert(nline < maxlines);

		buffer[MAXLINELEN-1] = 0;
		bytes = strlen(buffer);
		if (buffer[bytes-1] == '\n') {
			buffer[bytes-1] = 0;
		}

		while (p != NULL && *p != 0) {
			tokens[num_tokens] = strsep(&p, ",");
			num_tokens++;
			assert(num_tokens < MAXTOKENS);
		}

		/* if the first token is number, this is valid row of data */
		if (num_tokens == cols && sscanf(tokens[0], "%g", &t[nline]) == 1) { 
			for (i = 1 ; i < num_tokens; i++) {
				int status;

				status = sscanf(tokens[i], "%g", &data[i + nline*nx - 1]);
				assert(status == 1);
			}
			nline++;
		}		
	} /* while (!feof(fp)) */
	fclose(fp);
	
	return nline;
}

static int saveImg(char *prefix, float *data, int n, int filenum, float fmin, float fmax)
{
	unsigned char *buf = XALLOC(n, unsigned char);
	char fname[1024];

#if PGM_OUTPUT
	sprintf(fname, "%s%08d.pgm", prefix, filenum);
#else
	sprintf(fname, "%s%08d.jpg", prefix, filenum);
#endif

	/* use dynamic scaling */
	bzsimScaleFloatArr(data, buf, n, fmin, fmax);

#if PGM_OUTPUT
	bzsimWritePGM(fname, buf, ELE_NX, ELE_NY);
#else
	bzsimWriteJPEG(fname, buf, ELE_NX, ELE_NY, BZSIM_BW, 100);
#endif

	XFREE(buf);

	return 0;
}

static void calcBas(float *data, float *data2, int cols, int n)
{
	int i, j, idx;
	float *avg;

	avg = XALLOC(cols, float); 

	for (j = 0; j < cols; j++) {
		avg[j] = 0;
	}
	for (i = 0, idx = 0; i < n; i++) {
		for (j = 0; j < cols; j++, idx++) {
			avg[j] += data[idx];
		}
	}
	for (j = 0; j < cols; j++) {
		avg[j] /= (float) n;
	}
	for (i = 0, idx = 0; i < n; i++) {
		for (j = 0; j < cols; j++, idx++) {
			data2[idx] = data[idx] - avg[j];
		}
	}
	XFREE(avg);
}

static void calcSub(float *in, float *out, int cols, int n)
{
	int i, j, idx = 0;
	float *tmp = XALLOC(cols, float); 

	assert(n > 1);
	/* load first frame */
	memcpy(tmp, in, cols*sizeof(float));
	for (i = 1; i < n; i++) {
		for (j = 0; j < cols; j++, idx++) {
			out[idx] = tmp[j] - in[j + i*cols];
		}
		memcpy(tmp, (in + i*cols), cols*sizeof(float));
	}
	XFREE(tmp);
}

static int calcAvg(float *data, float *data2, int cols, int n, int nframes)
{
	int i = 0, j, navg, ntotal, idx = 0;
	
	ntotal = n/nframes;
	assert(ntotal > 0);
	memset(data2, 0, cols*n*sizeof(float));
	for (navg = 0; navg < ntotal; navg++) {
		for (j = 0; j < nframes; j++) {
			for (i = 0; i < cols; i++) {
				data2[i + navg*cols] += data[idx++];
			}
		}
	}

	return ntotal;
}
