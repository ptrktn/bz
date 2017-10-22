#!/bin/sh
# $Id: mkmpg.sh,v 1.1.1.1 2003/07/05 15:08:18 petterik Exp $

MPG=$1
DIR=$2
FILERG=$3
MINPIX=$4
PARF=${DIR}/$$.par

usage () {
	echo "usage: mkmpg.sh <mpgfile> <filedir> <file pattern> [<min size>]"
        echo "example:"
        echo "./mkmpg.sh /tmp/bas.mpg /tmp 'bas*.pgm'"
	exit 1
}

ppmtompegpar () {
	MPG=$1
	FILES=$2
	IFORMAT=$3

	cat <<EOF > ${PARF}
PATTERN I
OUTPUT $MPG
INPUT_DIR $DIR
INPUT
\`ls -1 ${FILES}\`
END_INPUT
GOP_SIZE 4
PIXEL FULL
PQSCALE 1
BQSCALE 1
IQSCALE 1
RANGE 2
PSEARCH_ALG SUBSAMPLE
BSEARCH_ALG SIMPLE
INPUT_CONVERT *
BASE_FILE_FORMAT $IFORMAT
SLICES_PER_FRAME 1
REFERENCE_FRAME ORIGINAL
EOF
}

if ( [ "$FILERG" = "" ] || [ "$DIR" = "" ] || [ "$MPG" = "" ] ) ; then
	usage
fi

if [ "$MINPIX" = "" ] ; then
	 MINPIX=64
fi

FILE=`find $DIR -name "$FILERG" 2> /dev/null | head -1`
NX=`pnmfile $FILE | awk '{print $4;}'`
NY=`pnmfile $FILE | awk '{print $6;}'`

if ( [ $NX -lt $MINPIX ] || [ $NY -lt $MINPIX ] ) ; then
	# conversion needed
	for i in `find $DIR -name "$FILERG" 2> /dev/null` ; do
		pnmscale -xsize=$MINPIX -ysize=$MINPIX $i > $i.pnm
	done
	ppmtompegpar ${MPG} "${FILERG}.pnm" PNM
	
else
	echo err
	exit 1
	ppmtompegpar ${MPG} ${FILERG} PGM
fi

ppmtompeg -realquiet ${PARF}

rm -f ${PARF}

exit 0
