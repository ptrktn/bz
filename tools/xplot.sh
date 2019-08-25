#!/bin/bash

GNUPLOTBIN=${GNUPLOT-gnuplot}
HAVE_GS=0
HAVE_IM=0
HAVE_JPEG_TERM=0

preflight() {

	# gnuplot is mandatory
	test -x "$(command -v $GNUPLOTBIN)"
	if [ 0 -ne $? ] ; then
		echo "ERROR: gnuplot is required"
		exit 1
	fi

	echo "set term" | $GNUPLOTBIN 2>&1 | grep -wq jpeg
	if [ 0 -eq $? ] ; then
		HAVE_JPEG_TERM=1
	fi

	test -x "$(command -v gs)"
	if [ 0 -eq $? ] ; then
		HAVE_GS=1
	fi

	test -x "$(command -v convert)"
	if [ 0 -eq $? ] ; then
		HAVE_IM=1
	fi

	if [ $HAVE_JPEG_TERM -eq 0 -a $HAVE_GS -eq 0 -a $HAVE_IM -eq 0 ] ; then
		echo "ERROR: Ghostscript or ImageMagik is required"
		exit 1
	fi

}

preflight

FNAME="$1"
TITLE="$2"

declare -a title=( $TITLE )

test -f "$FNAME" || exit 1

NCOLS=$(egrep -v '^#' $FNAME | head -1 | awk '{ print NF }')

ID=$(echo $(basename $FNAME) | sed 's/\..*$//')_$(date +%Y%m%d%H%M)_$(printf "%05d" $RANDOM)

xplot() {
	local fname=$1
	local col=$2
	local title=$3
	local j=$(( $col - 1 ))
	local f=${ID}_x$(printf "%02d" $j).jpg
	local ptitle=$(echo $ID | sed 's:_: :g')

	if [ $HAVE_JPEG_TERM -eq 0 ] ; then
		local tmp=$(mktemp --tmpdir=/tmp tmp.XXXXX.ps)
		echo "set term postscript ; set output '$tmp' ; set title '$ptitle' ; plot '$fname' u 1:$col w l title 'x(${j}) $title'" | $GNUPLOTBIN > /dev/null 2>&1 || exit 1
		if [ $HAVE_GS -ne 0 ] ; then
			gs -q -dSAFER -dBATCH -dNOPAUSE -sDEVICE=jpeg -r300 \
			   -dTextAlphaBits=4 -dGraphicsAlphaBits=4 -dMaxStripSize=8192 \
			   -sOutputFile=$f -dAutoRotatePages=/None \
			   -c '<</Orientation 3>> setpagedevice' -f $tmp || exit 1
		else
			convert -density 300 $tmp -flatten -background white -resize 1024 -rotate 90 $f || exit 1
		fi
		rm -f $tmp
	else
		echo "set term jpeg size 1280,960 ; set output '$f' ; set title '$ptitle' ; plot '$fname' u 1:$col w l ls 1 title 'x(${j}) $title'" | $GNUPLOTBIN > /dev/null 2>&1 || exit 1
	fi
}

maxjobs=$(nproc)

(( maxjobs-- ))

for i in $(seq 2 $NCOLS) ; do
	j=$(( $i - 2))
	xplot $FNAME $i ${title[$j]} &
	while [ $(jobs -p | wc -l) -gt $maxjobs ] ; do
		sleep 10
	done
done

wait

exit 0


