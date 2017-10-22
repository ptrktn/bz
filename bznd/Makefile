#****h* bznd/bznd_Makefile
# NAME
#   bznd_Makefile
#   $Id: Makefile,v 1.27 2005/08/25 22:27:29 petterik Exp $
# USAGE 
#   make [install|test|test3|test4|xtest|clean]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = bz1
BINFILE     = $(MODULE)
BINFILE2    = fkn
TARGETS     = $(BINFILE)
BINOBJECTS  = bz1.o
BINOBJECTS2 = fkn.o

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

# test build?
ifeq "$(TESTFLAG)" "1"
CFLAGS += -D__TEST
endif

install: $(BINDIR)/$(BINFILE)

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINFILE2) $(BINOBJECTS) $(BINOBJECTS) \
          core *~ *.BAK *.jpg *.png \
          test.dat point.dat pointrk4 pointrk4.dat nohup.out
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	if [ -d ./index ] ; then $(RMDIR) ./index ; fi

# model test
.PHONY: test
test: $(TARGETS)
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	./$(BINFILE) bzndbase.dat > test.dat
	if [ -x "`which eog`" ] ; then (eog ./res &) ; \
		else echo "see directory \`res' for results" ; fi
#	$(JPGIND) res
#	@if [ -x "`which mozilla`" ] ; then mozilla index/index.html ; \
#		else echo "see index/index.html" ; fi

.PHONY: point
point:
	rm -f point.jpg
	$(OCTAVE) point
	$(BINDIR)/viewimg.sh point.jpg

.PHONY: rk4plot
rk4plot: pointrk4
	./pointrk4 > pointrk4.dat
	(echo 'set terminal png'; echo 'set output "pointrk4.png"'; echo 'plot "pointrk4.dat" using 1:2 with lines, "pointrk4.dat" using 1:3 with lines, "pointrk4.dat" using 1:4 with lines') | gnuplot
	(eog pointrk4.png &)


# point model test
.PHONY: pointx
pointx: fkn
	cp -f bzndbase.dat test.dat
	echo "MODEL 2" >> test.dat	
	echo "ITERATIONS 5000000" >> test.dat
	echo "SNAP 100" >> test.dat
	echo "phi 0" >> test.dat
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	./fkn test.dat > point.dat
	-(echo 'set terminal png'; echo 'set output "point.png"'; echo 'plot "point.dat" using 1:2 with lines, "point.dat" using 1:3 with lines, "point.dat" using 1:4 with lines') | gnuplot
	(eog point.png &)
	convert -quality 95 point.png point.jpg
	rm -f test.dat

# the main result is space-time plot
.PHONY: test3
test3: $(TARGETS)
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	$(INSTALL) bzndbase.dat test.dat
	echo "MODEL 3" >> test.dat
	echo "phi 0" >> test.dat
	echo "phi2 0" >> test.dat
	echo "BOUNDARY 2" >> test.dat
	echo "SNAP_DF3 0" >> test.dat
	echo "SNAP 0" >> test.dat
	echo "PACEMAKER_RADIUS 0" >> test.dat
	echo "NX 6" >> test.dat
	echo "NY 6" >> test.dat
	echo "NZ 96" >> test.dat
	echo "SNAPZ 8" >> test.dat
	echo "ITERATIONS 2000000" >> test.dat
	echo "XT_PLOT 5000" >> test.dat
	echo "TIMESTEP_SAFETY 100" >> test.dat
	./$(BINFILE) test.dat
	sh -c 'eog res/xt.png res/at.png > /dev/null 2>1 &'

# 1D space-time plot
.PHONY: test4
test4: $(TARGETS)
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	$(INSTALL) bzndbase.dat test.dat
	echo "MODEL 4" >> test.dat
	echo "NPTS 5" >> test.dat
	echo "D_x 1" >> test.dat
	echo "D_z 0" >> test.dat
	echo "D_a 0.5" >> test.dat
	echo "phi 0" >> test.dat
	echo "phi2 0" >> test.dat
	echo "BOUNDARY 2" >> test.dat
	echo "SNAP_DF3 0" >> test.dat
	echo "SNAP 0" >> test.dat
	echo "PACEMAKER_RADIUS 0" >> test.dat
	echo "NZ 256" >> test.dat
	echo "SNAPZ 8" >> test.dat
	echo "ITERATIONS 750000" >> test.dat
	echo "XT_PLOT 1000" >> test.dat
	echo "TIMESTEP_SAFETY 20" >> test.dat
	echo "SCAL_MAX 0.2" >> test.dat
	echo "a_ini 0.10" >> test.dat
	time ./$(BINFILE) test.dat
	$(BINDIR)/viewimg.sh res/at.jpg res/xt.jpg res/00_space_time.jpg

.PHONY: fkntest
fkntest: fkn
	rm -fr res
	$(INSTALL) bzndbase.dat fkntest.dat
	echo "MODEL 2" >> fkntest.dat
	./fkn fkntest.dat > fkn.dat
	$(BINDIR)/plot.sh fkn.dat 5 fkn.png
	$(BINDIR)/viewimg.sh fkn.png

.PHONY: fknsim
fknsim: fkn
	rm -fr res fknsim.dat
	$(INSTALL) bzndbase.dat fknsim.dat
	echo "SIMULATION_NAME fkn" >> fknsim.dat
	echo "MODEL 5" >> fknsim.dat
	echo "NZ 1024" >> fknsim.dat
	echo "NPTS 5" >> test.dat
	echo "SNAP_DF3 0" >> fknsim.dat
	echo "SNAP 0" >> fknsim.dat
	echo "ITERATIONS 15000000" >> fknsim.dat
	echo "TIMESTEP_SAFETY 50" >> fknsim.dat
	echo "XT_PLOT 1000" >> fknsim.dat
	echo "SCAL_MAX 0.0011" >> fknsim.dat
#	echo "k5c 0.1" >> fknsim.dat
	time $(NICE) ./fkn fknsim.dat
	$(BINDIR)/viewimg.sh res/at.jpg res/xt.jpg res/00_space_time.jpg

.PHONY: fkn2
fkn2:
	$(OCTAVE) point_fkn
	$(BINDIR)/viewimg.sh point_fkn.jpg

.PHONY: sim1anim
sim1anim: fkn
	./fkn sim1anim.dat

# internal test
.PHONY: xtest
xtest:
	$(MAKE) clean
	$(MAKE) TESTFLAG=1
	./$(BINFILE)

#*****
