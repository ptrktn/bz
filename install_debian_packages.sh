#!/bin/sh
# $Id: install_debian_packages.sh,v 1.5 2005/06/27 12:32:29 petterik Exp $

if [ ! -x "`which apt-get`" ] ; then
	echo "ERROR: no apt-get found"
	exit 1
fi
 
if [ "`id -u`" != "0" ] ; then
	echo "ERROR: you need to be root to run this script (try \`su - root')"
	exit 1
fi

set -e

apt-get update

apt-get install libjpeg62 libjpeg62-dev robodoc gnuplot gnuplot-doc \
	indent indent-doc fftw3 fftw3-dev fftw3-doc imagemagick xine-ui \
	octave2.1 octave2.1-doc octave-forge \
	gsl-bin gsl-doc-pdf libgsl0 libgsl0-dev

exit 0
