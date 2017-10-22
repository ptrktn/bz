#!/bin/sh
#****h* tools/plot.sh
# NAME
#   plot.sh
#   $Id: plot.sh,v 1.3 2004/12/15 05:00:29 petterik Exp $
# USAGE 
#   plot.sh <input data file> <number of y columns> <output PNG file>
# DESCRIPTION
#   Wrapper script to plot time series from an
#   ASCII file. First column in a row specifies x axis value
#   followed by y value or values. I.e. the data format is:
#
#     x1 y11 y12 y13 ...
#     x2 y21 y22 y23 ...
#     x3 y31 y32 y33 ...
#     ...
# EXAMPLE
#   echo "1 2" > data.dat
#   echo "2 3" >> data.dat
#   ./plot.sh data.dat 2 data.png 
#   rm data.dat
#   viewimg.sh data.png
# SEE ALSO
#   Toplevel Makefile
# USES
#   gnuplot
# SOURCE
#

INF=$1
NCOLS=$2
OUTF=$3
TMPF=/tmp/plot${USER}$$

set -e

echo "set terminal png" > $TMPF
echo "set output \"${OUTF}\"" >> $TMPF
echo -n "plot " >> $TMPF

NCOL=1

while [ $NCOL -le $NCOLS ] ; do
	COL=$(($NCOL + 1))
	echo -n "\"$INF\" using 1:${COL} with lines" >> $TMPF
	if [ $NCOL -lt $NCOLS ] ; then
		echo -n ", " >> $TMPF
	else
		echo "" >> $TMPF
	fi
	NCOL=$(($NCOL + 1))
done

gnuplot $TMPF

rm -f $TMPF

exit 0

#*****
