#!/bin/sh

NX=192

set -e

mkmpg () {
	rm -fr tmp
	mkdir tmp
	I=1
	test -d $1
	for i in `find $1 -name '*.jpg' | sort` ; do
		cp -f $i tmp/$I.jpg
		I=$(($I + 1));
	done
	cd tmp
	ffmpeg -y -i %d.jpg ../$2.mpg
	cd ..
	rm -fr tmp
}

mkdir -p model3

for R in 50 40 30 20 10 5; do
	OUT="model3/R_$R"
	rm -fr $OUT
	cp -f bz2dbase.dat model3.dat
	echo "MODEL 3" >> model3.dat
	echo "BOUNDARY 2" >> model3.dat
	echo "NX $NX" >> model3.dat
	echo "NY $NX" >> model3.dat
	echo "R $R" >> model3.dat
	echo "ITERATIONS 500000" >> model3.dat
	echo "SNAP 100" >> model3.dat
	echo "SIMULATION_NAME m3" >> model3.dat
	echo "OUTPUT_DIR $OUT" >> model3.dat
	./bz2d model3.dat
	rm -f model3.dat
	mkmpg $OUT R_$R
done

exit 0
