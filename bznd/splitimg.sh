#!/bin/sh
# Split large space-time image to smaller images.
# $Id: splitimg.sh,v 1.2 2004/12/17 23:31:08 petterik Exp $

set -e
set -x

IN=$1
BODY=$2
DIV=$3

BODY=${BODY:-split}
DIV=${DIV:-10}

DIMS=`identify -format "%w %h" $IN 2>&1`
X=`echo $DIMS | awk '{print $1}'`
Y=`echo $DIMS | awk '{print $2}'`

LY=$(($Y / $DIV))

echo "$BODY $X $Y $LY"

I=0

while [ $I -lt $DIV ] ; do
	echo $I
	OY=$(($I * $LY))
	SER=`printf %05d $I`
	convert -crop "${X}x${LY}+0+${OY}" $IN ${BODY}${SER}.jpg
	I=$(($I + 1))
done

exit 0