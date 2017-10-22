#!/bin/sh

cd /var/tmp/anim

#while true ; do 
	i=`cat idx`
	f=`ls -1tr /var/tmp/anim/new018_5 | grep _x1_ | head -1`
	d=`ls -1tr /var/tmp/anim/new018_5 | grep _x2_ | head -1`
	j=`ls -1tr /var/tmp/anim/new018_5 | grep jpg | head -1`
#	f=`ls -1tr /var/tmp/anim/new018_5 | grep _x1_ | tail -1`
	f=/var/tmp/anim/new018_5/$f
	d=/var/tmp/anim/new018_5/$d
	j=/var/tmp/anim/new018_5/$j
	if [ -f $f ] ; then 
		/home/petterik/bin/makedf3 -z $f > snap.df3 2> /dev/null
		/usr/local/bin/povray +Idf3.pov df3.ini > /dev/null 2>&1
		if [ $? != 0 ] ; then echo "povray failed $i" ; exit 1; fi
	#convert -rotate 90 -crop 600x200+0+200 df3.tga JPG:$i.jpg
		convert -rotate 90 -crop 300x100+0+100 df3.tga JPG:$i.jpg
		if [ $? != 0 ] ; then echo "convert failed $i" ; exit 1; fi
		rm -f idx df3.tga snap.df3 $f $d $j
		echo "processed $f ==> $i.jpg"
		i=`expr $i \+ 1`
		echo $i > idx
	fi
#	exit 1
#done

exit 0
