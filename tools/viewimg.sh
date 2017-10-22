#!/bin/sh
#****h* tools/viewimg.sh
# NAME
#   viewimg.sh
#   $Id: viewimg.sh,v 1.1 2004/12/15 05:00:29 petterik Exp $
# USAGE 
#   viewimg.sh <image file> [<image file>...]
# DESCRIPTION
#   Display image on the screen. Platform-specific application is
#   checked at the runtime.
# EXAMPLE
#   viewimg.sh data.png data2.png
# RETURN VALUE
#   0 if successful, non-zero in case of error
# SOURCE
#

VIEWER=""

find_viewer () {
	for x in eog xv gimp open ; do
		if test -x "`which $x`" ; then
			VIEWER=$x
			return
		fi
	done

	echo "ERROR: no image viewer application found"
	exit 1
}

# check that args are files
for f in $* ; do
	if ! test -f $f ; then
		echo "ERROR: argument $f is not a regular file"
		exit 1
	fi
done

find_viewer

set -e

$VIEWER $* > /dev/null 2>&1 &

exit 0

#*****
