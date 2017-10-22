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
apt-get upgrade

apt-get install libjpeg62-turbo libjpeg62-turbo-dev gnuplot gnuplot-doc \
	indent indent-doc libfftw3-3 libfftw3-dev libfftw3-doc \
        imagemagick xine-ui \
	octave octave-doc povray \
	gsl-bin libgsl0ldbl libgsl0-dev

exit 0
