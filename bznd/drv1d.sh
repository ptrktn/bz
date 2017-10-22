#!/bin/sh
# $Id: drv1d.sh,v 1.4 2004/12/18 11:42:49 petterik Exp $
# simulation driver for 1D spatial model

RESTOP=model4
REMHOST=tt
REMDIR=public_html/bznd

set -e

if ! test -d ./$RESTOP ; then 
	mkdir -p ./$RESTOP
fi

# base config file

rm -f $RESTOP/base.dat
cp -f bzndbase.dat $RESTOP/base.dat

echo "MODEL 4" >> $RESTOP/base.dat
echo "D_x 1" >> $RESTOP/base.dat
echo "D_z 0" >> $RESTOP/base.dat
echo "D_a 0.5" >> $RESTOP/base.dat
echo "phi 0" >> $RESTOP/base.dat
echo "phi2 0" >> $RESTOP/base.dat
echo "BOUNDARY 2" >> $RESTOP/base.dat
echo "SNAP_DF3 0" >> $RESTOP/base.dat
echo "SNAP 0" >> $RESTOP/base.dat
echo "PACEMAKER_RADIUS 0" >> $RESTOP/base.dat
echo "NZ 1024" >> $RESTOP/base.dat
echo "SNAPZ 8" >> $RESTOP/base.dat
echo "ITERATIONS 20000000" >> $RESTOP/base.dat
echo "XT_PLOT 1000" >> $RESTOP/base.dat
echo "TIMESTEP_SAFETY 20" >> $RESTOP/base.dat
echo "SCAL_MAX 0.5" >> $RESTOP/base.dat

sim1() {
	for AINI in 0.25 0.20 0.15 0.10 0.05 0.02; do
		ODIR=${RESTOP}/model4_sim1_a_ini_${AINI}
		cp -f $RESTOP/base.dat $RESTOP/sim_${AINI}.dat
		echo "OUTPUT_DIR ${ODIR}" >> $RESTOP/sim_${AINI}.dat
		echo "a_ini ${AINI}" >> $RESTOP/sim_${AINI}.dat
		./bz1 $RESTOP/sim_${AINI}.dat
		rm -f $RESTOP/sim_${AINI}.dat
		rsh $REMHOST "if ! test -d $REMDIR ; then mkdir -p $REMDIR; fi"
		rcp -r ${ODIR} ${REMHOST}:${REMDIR}
	done
}

sim2() {
	for AINI in 0.10 ; do
		for DA in 0.25 0.50 0.75 1.00 1.25 1.50; do
			ODIR=${RESTOP}/m4_sim2_a_ini_${AINI}_d_a_${DA}
			PARF=${ODIR}.dat
			cp -f $RESTOP/base.dat $PARF
			echo "ITERATIONS 5000000" >> $PARF
			echo "OUTPUT_DIR ${ODIR}" >> $PARF
			echo "NZ 512" >> $PARF
			echo "NPTS 5" >> $PARF
			echo "SCAL_MAX 0.2" >> $PARF
			echo "a_ini ${AINI}" >> $PARF
			echo "D_a ${DA}" >> $PARF
			./bz1 $PARF
			rm -f $PARF
			sh ./splitimg.sh ${ODIR}/00_space_time.jpg snap_ 10
			jpgind -x 5 -y 2 -T "sim2 a_ini ${AINI} D_a ${DA}" snap_*
			mv index $ODIR
			rm -fr snap_*
			rsh $REMHOST "if ! test -d $REMDIR ; then mkdir -p $REMDIR; fi"
			rcp -r ${ODIR} ${REMHOST}:${REMDIR}
		done
	done
}

sim3() {
	for AINI in 0.10 ; do
		for F in 0.6 0.7 0.9 1.0 1.1 1.2 1.3 1.4 1.5; do
			ODIR=${RESTOP}/m4_sim3_a_ini_${AINI}_f_${F}
			PARF=${ODIR}.dat
			cp -f $RESTOP/base.dat $PARF
			echo "ITERATIONS 5000000" >> $PARF
			echo "OUTPUT_DIR ${ODIR}" >> $PARF
			echo "NZ 512" >> $PARF
			echo "NPTS 5" >> $PARF
			echo "SCAL_MAX 0.2" >> $PARF
			echo "a_ini ${AINI}" >> $PARF
			echo "f ${F}" >> $PARF
			./bz1 $PARF
			rm -f $PARF
			sh ./splitimg.sh ${ODIR}/00_space_time.jpg snap_ 10
			jpgind -x 5 -y 2 -T "sim3 a_ini ${AINI} f ${F}" snap_*
			mv index $ODIR
			rm -fr snap_*
			rsh $REMHOST "if ! test -d $REMDIR ; then mkdir -p $REMDIR; fi"
			rcp -r ${ODIR} ${REMHOST}:${REMDIR}
		done
	done
}

sim4() {
	for AINI in 0.10 ; do
		ODIR=${RESTOP}/m4_sim4_a_ini_${AINI}
		PARF=${ODIR}.dat
		cp -f $RESTOP/base.dat $PARF
		echo "ITERATIONS 20000000" >> $PARF
		echo "OUTPUT_DIR ${ODIR}" >> $PARF
		echo "NZ 256" >> $PARF
		echo "NPTS 5" >> $PARF
		echo "SCAL_MAX 0.2" >> $PARF
		echo "a_ini ${AINI}" >> $PARF
		./bz1 $PARF
		rm -f $PARF
		sh ./splitimg.sh ${ODIR}/00_space_time.jpg snap_ 10
		jpgind -x 5 -y 2 -T "sim4 a_ini ${AINI} NZ 256" snap_*
		mv index $ODIR
		rm -fr snap_*
		rsh $REMHOST "if ! test -d $REMDIR ; then mkdir -p $REMDIR; fi"
		rcp -r ${ODIR} ${REMHOST}:${REMDIR}
	done
}

# build (just in case)
make -s

# vary initial A concentration
# sim1

# vary D_a
# sim2

# vary excitability
# sim3

# small length
sim4

exit 0
