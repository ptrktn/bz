#!/bin/bash

test -x "$(command -v convert)" || {
	echo "ERROR: ImageMagick convert is required"
	exit 1
}

FNAME="$1"

# the second argument is not really needed
FMT=${2-"jpg"}

test -f "$FNAME" || exit 1

NCOLS=$(egrep -v '^#' $FNAME | head -1 | awk '{ print NF }')
ID=$(echo $(basename $FNAME) | sed 's/\..*$//')_$(date +%Y%m%d%H%M%S)_$(printf "%05d" $RANDOM)

for i in $(seq 2 $NCOLS) ; do
	rm -f tmp.ps
	j=$(( $i -1 ))
	f=${ID}_x$(printf "%02d" $j).$FMT
	echo "set term postscript ; set output 'tmp.ps' ; plot '$FNAME' u 1:$i w l title '${x}${j}'" | gnuplot > /dev/null 2>&1 || exit 1
	convert -density 300 tmp.ps -flatten -background white -resize 1024 -rotate 90 $f
	rm -f tmp.ps
done

exit 0


