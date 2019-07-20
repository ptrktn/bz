#!/bin/bash

test -x "$(command -v convert)" || {
	echo "ERROR: ImageMagick convert is required"
	exit 1
}

FNAME="$1"

# the second argument is not really needed
FMT=${2-"eps"}

test -f "$FNAME" || exit 1

NCOLS=$(egrep -v '^#' $FNAME | head -1 | awk '{ print NF }')
ID=$(echo $(basename $FNAME) | sed 's/\..*$//')_$(date +%Y%m%d%H%M%S)_$(printf "%05d" $RANDOM)

for i in $(seq 2 $NCOLS) ; do
	rm -f tmp.eps
	j=$(( $i -1 ))
	f=${ID}_x$(printf "%02d" $j).$FMT
	echo "set term $FMT ; set output 'tmp.eps' ; plot '$FNAME' u 1:$i w l title '${x}${i}'" | gnuplot || exit 1
	convert -density 300 tmp.eps -flatten -background white -resize 1024 $f
	rm -f tmp.eps
done

exit 0


