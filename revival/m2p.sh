#!/bin/bash

fname=$1

test -f "$fname" || exit 1

# -e 'k[rf]\([0-9]{1,2}\).*=' 
egrep -e '[rf]kinet\([0-9]{1,2}\).*=' \
	  -e 'xdot\([0-9]{1,2}\).*=' \
	  $fname | sed -e 's/[\(\)]//g' -e 's/\^/\*\*/g' \
				   -e 's/#.*//g' -e 's/^\s*//g' -e '/^$/d'
