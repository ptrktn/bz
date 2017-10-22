/****h* bzsim/random_test
 *  NAME
 *    random_test
 *  SYNOPSIS
 *    random_test <number of samples>
 *  FUNCTION
 *    Test driver for bzsimRand function.
 *  SOURCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bzsim.h"

int             main(int argc, char *argv[])
{
	float           x, sum = 0;
	int             i, n;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <number of samples>\n", argv[0]);
		exit(1);
	}
	n = atoi(argv[1]);
	init_random(0);
	for (i = 0; i != n; i++) {
		sum += random_variable();
	}

	printf("%d %f %f\n", n, sum, sum / n);
	return 0;
}

/************ main */
