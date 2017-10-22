/****h* helloworld/helloworld.c
 * NAME
 *  helloworld.c
 *  $Id: helloworld.c,v 1.1 2004/09/30 12:30:48 petterik Exp $
 * DESCRIPTION
 *  Example program.
 ******
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "bzsim.h"

int main(int argc, char **argv)
{
	bzsimDbg("Hello, World!\n");

	return 0;
}
