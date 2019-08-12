#!/bin/bash

test -x "$(command -v convert)" || {
	echo "ERROR: ImageMagick convert is required"
	exit 1
}

FNAME="$1"
TITLE="$2"

declare -a title=( $TITLE )

test -f "$FNAME" || exit 1

NCOLS=$(egrep -v '^#' $FNAME | head -1 | awk '{ print NF }')

ID=$(echo $(basename $FNAME) | sed 's/\..*$//')_$(date +%Y%m%d%H%M)_$(printf "%05d" $RANDOM)

xplot() {
	local fname=$1
	local col=$2
	local j=$(( $col -1 ))
	local tmp=$(mktemp --tmpdir=/tmp tmp.XXXXX.ps)
	local f=${ID}_x$(printf "%02d" $j).jpg

	echo "set term postscript ; set output '$tmp' ; set title '$(echo $ID | sed s:_:\\\\_:g)' ; plot '$fname' u 1:$col w l title 'x(${j}) ${title[$j]}'" | gnuplot > /dev/null 2>&1 || exit 1
	convert -density 300 $tmp -flatten -background white -resize 1024 -rotate 90 $f
	rm -f $tmp
}

maxjobs=$(nproc)

(( maxjobs-- ))

for i in $(seq 2 $NCOLS) ; do
	j=$(( $i -1 ))
	xplot $FNAME $i &

	while [ $(jobs -p | wc -l) -gt $maxjobs ] ; do
		sleep 10
	done
done

wait

exit 0


