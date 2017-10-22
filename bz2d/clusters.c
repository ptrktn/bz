/* Tobin Fricke's implementation of the
   Hoshen-Kopelman algorithm for
   cluster labeling.

   Copyright (c) September 9, 2000, by Tobin Fricke <tobin@splorg.org>
   Distributed under the terms of the GNU Public License.

   Modified 2002-03-09 Tobin Fricke
   Modified substantially 2004-04-21 by Tobin Fricke

   This program is written in the 1999 standard of the C language (C99).  Older C
   compilers will refuse to compile it.   You can use a C++ compiler, a C99 compiler,
   or you can modify this code to comply with a previous version of the C standard.
   The GCC compiler supports C99 as of version 3.0.  Compile this program with:

   gcc-3.0 -Wall -std=c99 hk.c -o hk

   http://splorg.org/people/tobin/projects/hoshenkopelman/hoshenkopelman.html

   http://www.iki.fi/petterik/doc/timelapseimg/

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

/* Implementation of Union-Find Algorithm */


/* The 'labels' array has the meaning that labels[x] is an alias for the label x; by
   following this chain until x == labels[x], you can find the canonical name of an
   equivalence class.  The labels start at one; labels[0] is a special value indicating
   the highest label already used. */

int *labels;
int n_labels = 0;				/* length of the labels array */

/*  uf_find returns the canonical label for the equivalence class containing x */

int uf_find(int x)
{
	int y = x;
	while (labels[y] != y)
		y = labels[y];

	while (labels[x] != x) {
		int z = labels[x];
		labels[x] = y;
		x = z;
	}
	return y;
}

/*  uf_union joins two equivalence classes and returns the canonical label of the resulting class. */

int uf_union(int x, int y)
{
	return labels[uf_find(x)] = uf_find(y);
}

/*  uf_make_set creates a new equivalence class and returns its label */

int uf_make_set(void)
{
	labels[0]++;
	assert(labels[0] < n_labels);
	labels[labels[0]] = labels[0];
	return labels[0];
}

/*  uf_intitialize sets up the data structures needed by the union-find implementation. */

void uf_initialize(int max_labels)
{
	n_labels = max_labels;
	labels = calloc(sizeof(int), n_labels);
	labels[0] = 0;
}

/*  uf_done frees the memory used by the union-find data structures */

void uf_done(void)
{
	n_labels = 0;
	free(labels);
	labels = 0;
}

/* End Union-Find implementation */

#define max(a,b) (a>b?a:b)
#define min(a,b) (a>b?b:a)

/* print_matrix prints out a matrix that is set up in the "pointer to pointers" scheme
   (aka, an array of arrays); this is incompatible with C's usual representation of 2D
   arrays, but allows for 2D arrays with dimensions determined at run-time */

void print_matrix(int **matrix, int m, int n)
{
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++)
			printf("%3d ", matrix[i][j]);
		printf("\n");
	}
}


/* Label the clusters in "matrix".  Return the total number of clusters found. */

int hoshen_kopelman(int **matrix, int m, int n)
{

	uf_initialize(m * n / 2);

	/* scan the matrix */

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (matrix[i][j]) {	// if occupied ...

				int up = (i == 0 ? 0 : matrix[i - 1][j]);	//  look up  
				int left = (j == 0 ? 0 : matrix[i][j - 1]);	//  look left

				switch (!!up + !!left) {

				case 0:
					matrix[i][j] = uf_make_set();	// a new cluster
					break;

				case 1:		// part of an existing cluster
					matrix[i][j] = max(up, left);	// whichever is nonzero is labelled
					break;

				case 2:		// this site binds two clusters
					matrix[i][j] = uf_union(up, left);
					break;
				}

			}

	/* apply the relabeling to the matrix */

	/* This is a little bit sneaky.. we create a mapping from the canonical labels
	   determined by union/find into a new set of canonical labels, which are 
	   guaranteed to be sequential. */

	int *new_labels = calloc(sizeof(int), n_labels);	// allocate array, initialized to zero

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (matrix[i][j]) {
				int x = uf_find(matrix[i][j]);
				if (new_labels[x] == 0) {
					new_labels[0]++;
					new_labels[x] = new_labels[0];
				}
				matrix[i][j] = new_labels[x];
			}

	int total_clusters = new_labels[0];

	free(new_labels);
	uf_done();

	return total_clusters;
}

/* This procedure checks to see that any occupied neighbors of an occupied site
   have the same label. */

void check_labelling(int **matrix, int m, int n, int *hist)
{
	int N, S, E, W;
	int *hist2 = (int *) malloc(m * n * sizeof(int));

	assert(hist2);

	for (int i = 0; i < m * n; i++)
		hist[i] = hist2[i] = 0;

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (matrix[i][j]) {
				N = (i == 0 ? 0 : matrix[i - 1][j]);
				S = (i == m - 1 ? 0 : matrix[i + 1][j]);
				E = (j == n - 1 ? 0 : matrix[i][j + 1]);
				W = (j == 0 ? 0 : matrix[i][j - 1]);

				assert(N == 0 || matrix[i][j] == N);
				assert(S == 0 || matrix[i][j] == S);
				assert(E == 0 || matrix[i][j] == E);
				assert(W == 0 || matrix[i][j] == W);

				hist2[matrix[i][j]]++;
			}

	for (int i = 0; i < m * n; i++)
		if (hist2[i])
			hist[hist2[i]]++;

	free(hist2);
}


/*
 */

float rmin(int **matrix, int m, int n, int i, int j)
{
	int rmax = (m > n ? m : n) / 2 + 1;
	int rmin = m + n;

	for (int r = 1; r <= rmax; r++) {
		char found = 0;
		int i0 = i - r > 0 ? i - r : 0;
		int j0 = j - r > 0 ? j - r : 0;
		int i1 = i + r <= m ? i + r : m;
		int j1 = j + r <= n ? j + r : n;

		assert(i1 <= m);
		assert(j1 <= n);

		for (int ii = i0; ii < i1; ii++) {
			for (int jj = j0; jj < j1; jj++) {
				if (matrix[ii][jj] && i != ii && j != jj) {
					found = 1;
					int di = i - ii;
					int dj = j - jj;
					float radius = sqrtf(di * di + dj * dj);
					if (radius < rmin)
						rmin = radius;
				}
			}
		}
		
		if (found)
			return rmin;
			
	}

	return rmin;
}


/*
 */

float average_rmin(int **matrix, int m, int n)
{
	float ravg = 0;
	int cnt = 0;

	for (int i = 0; i < m; i++)
		for (int j = 0; j < n; j++)
			if (matrix[i][j]) {
				ravg += rmin(matrix, m, n, i, j);
				cnt++;
			}

	return ravg / cnt;
}


int main(int argc, char **argv)
{

	int m, n, iters;

	m = n = 512;
	iters = 50;

	int **matrix = (int **) calloc(m, sizeof(int *));
	assert(matrix);

	int *sumhist = (int *) malloc(m * n * sizeof(int));
	assert(sumhist);

	for (int i = 0; i < m; i++) {
		matrix[i] = (int *) calloc(n, sizeof(int));
		assert(matrix[i]);
	}

	int *hist = (int *) malloc(m * n * sizeof(int));
	assert(hist);

	unlink("savg.dat");
	unlink("rmin.dat");

	float pstart = 0.02;
	float pend = 0.24;
	float pstep = 0.02;
	int xmax = -1;
	int ymax = -1;

	srand(((unsigned int) ((time(NULL) % 2) + getpid())));

	for (float p = pstart; p <= pend; p += pstep) {

		int clusters = 0;
		float rmin = 0;

		memset(sumhist, 0, m * n * sizeof(int));

		for (int iter = 0; iter < iters; iter++) {

			for (int i = 0; i < m; i++) {
				for (int j = 0; j < n; j++) {
					float x = rand() / (RAND_MAX + 1.0);
					matrix[i][j] = x < p ? 1 : 0;
				}
			}

			clusters += hoshen_kopelman(matrix, m, n);

			check_labelling(matrix, m, n, hist);

			rmin += average_rmin(matrix, m, n);

			if (m < 20 && n < 20)
				print_matrix(matrix, m, n);

			for (int i = 0; i < m * n; i++) {
				sumhist[i] += hist[i];
				if (hist[i]) {
					if (i > xmax)
						xmax = i;
					if (hist[i] > ymax)
						ymax = hist[i];
				}
			}
		}						/* iter = 0 ... iters */


		/* write out cluster size distribution */

		{
			char fname[64];
			sprintf(fname, "clusters_%.3f.dat", p);

			FILE *fp = fopen(fname, "w");
			assert(fp);

			for (int i = 0; i < m * n; i++)
				if (sumhist[i])
					fprintf(fp, "%d %f\n", i, sumhist[i] / ((float) iters));

			fclose(fp);
		}

		/* average cluster size for this value of p */

		float savg = 0;
		for (int i = 0; i < m * n; i++)
			savg += i * sumhist[i];
		savg /= (float) clusters;

		{
			FILE *fp;

			fp = fopen("savg.dat", "a");
			assert(fp);
			fprintf(fp, "%f %f\n", p, savg);
			fclose(fp);

			fp = fopen("rmin.dat", "a");
			assert(fp);
			fprintf(fp, "%f %f\n", p, rmin / iters);
			fclose(fp);
		}

	}							/* p = pstart ... pend */


	/* create plots */

	for (float p = pstart; p <= pend; p += pstep) {
		char fname[64], eps[64];
		sprintf(fname, "clusters_%.3f.dat", p);
		sprintf(eps, "clusters_%.3f.eps", p);

		FILE *fp = fopen("clusters.p", "w");
		assert(fp);

		fprintf(fp,
				"set logscale y\n"
				"set tics out\n"
				"set terminal postscript eps enhanced\n"			   
				"set xrange[0:%d]\n"
				"set nokey\n",
				xmax + 1 /*, ymax + 1 */ );
		fprintf(fp, "set title 'p = %.2f'\n", p);
		fprintf(fp, "set output '%s'\n", eps);
		fprintf(fp, "plot '%s' using 1:2 with impulses\n", fname);
		fclose(fp);

		int status = system("gnuplot clusters.p");
		assert(status == 0);

		unlink("clusters.p");
	}


	/* plot savg */

	{
		FILE *fp = fopen("savg.p", "w");
		assert(fp);
		fprintf(fp,
				"set nokey\n"
				"set xlabel 'p'\n"
				"set ylabel 'S'\n"
				"set terminal postscript eps enhanced\n"
				"set output 'savg.eps'\n"
				"plot 'savg.dat' using 1:2, 'savg.dat' using 1:2 smooth bezier\n"
				"set terminal png\n"
				"set output 'savg.png'\n"
				"replot\n"
				);
		fclose(fp);
		int status = system("gnuplot savg.p");
		assert(0 == status);
	}


	/* plot rmin */

	{
		FILE *fp = fopen("rmin.p", "w");
		assert(fp);
		fprintf(fp,
				"set nokey\n"
				"set xlabel 'p'\n"
				"set ylabel 'r_{ min }'\n"
				"set terminal postscript eps enhanced\n"
				"set output 'rmin.eps'\n"
				"plot 'rmin.dat' using 1:2\n"
				"set terminal png\n"
				"set output 'rmin.png'\n"
				"replot\n"
				);
		fclose(fp);
		int status = system("gnuplot rmin.p");
		assert(0 == status);
	}

	for (int i = 0; i < m; i++)
		free(matrix[i]);
	free(matrix);
	free(hist);
	free(sumhist);

	return 0;
}
