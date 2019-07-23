#****h* revival/revival_Makefile
# NAME
#   revival_Makefile
# USAGE 
#   make [install|fplot|plot|clean]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = revival
BINFILE     = $(MODULE)
BINFILE2    = fkn
TARGETS     = $(BINFILE)
BINOBJECTS  = pointskg.o
BINOBJECTS2 = 
BINFILE2    = pointskg
OCTAVEBIN   = octave --no-gui

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

XDATE=$(shell date +%Y%m%d)
XDATETIME=$(shell date +%Y%m%d%H%M)

install:: $(BINDIR)/skg.m

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINFILE2) $(BINOBJECTS) $(BINOBJECTS2) \
          core *~ *.BAK *.jpg *.png *.pyc \
          plot.mat nohup.out octave-core octave-workspace
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	if [ -d ./index ] ; then $(RMDIR) ./index ; fi

.PHONY: fplot
fplot:
	rm -f skg.jpg plot.mat
	chmod 755 skg.m
	for i in $(shell seq 0.5 0.05 1) ; do \
		rm -f $(XDATE)_skg_f_$$i.jpg ; \
		./skg.m $$i ; \
		mv skg.jpg $(XDATE)_skg_f_$$i.jpg ; \
	done

.PHONY: BrO30plot
BrO30plot:
	rm -f skg2.jpg plot.mat
	chmod 755 skg2.m
	for i in $(shell seq 0.010 0.010 0.300) ; do \
		rm -f $(XDATE)_skg_BrO30_$$i.jpg ; \
		./skg2.m $$i ; \
		mv skg2.jpg $(XDATE)_skg_BrO30_$$i.jpg ; \
	done

.PHONY: BrO30plotpp
BrO30plotpp:
	rm -f skg2pp.jpg plot.mat
	chmod 755 skg2pp.m
	for i in $(shell seq 0.010 0.010 0.300) ; do \
		rm -f skg2pp.jpg plot.mat $(XDATE)_skg2_pp_BrO30_$$i.jpg $(XDATE)_skg2_BrO30_$$i.jpg ; \
		./skg2pp.m $$i || exit 1 ; \
		mv skg2pp.jpg $(XDATE)_skg2_pp_BrO30_$${i}.jpg ; \
        mv skg2.jpg $(XDATE)_skg2_BrO30_$$i.jpg ; \
	done
	rm -f plot.mat $(XDATE)_skg2_BrO30.jpg $(XDATE)_skg2_BrO30_pp.jpg
	montage $(XDATE)_skg2_pp_BrO30_*.jpg -geometry +1+1 $(XDATE)_skg2_pp_BrO30.jpg 
	montage $(XDATE)_skg2_BrO30_*.jpg -geometry +1+1 $(XDATE)_skg2_BrO30.jpg 

.PHONY: BrCHDplotpp
BrCHDplotpp:
	rm -f skg3pp.jpg plot.mat skg3.jpg
	chmod 755 skg3pp.m
	for i in $(shell seq 0.002 0.002 0.01) $(shell seq 0.01 0.005 0.05); do \
		rm -f skg3.jpg skg3pp.jpg plot.mat $(XDATE)_skg3_pp_BrCHD0_$$i.jpg $(XDATE)_skg3_BrCHD0_$$i.jpg ; \
		./skg3pp.m $$i || exit 1 ; \
		mv skg3pp.jpg $(XDATE)_skg3_pp_BrCHD0_$${i}.jpg ; \
        mv skg3.jpg $(XDATE)_skg3_BrCHD0_$$i.jpg ; \
	done
	rm -f plot.mat $(XDATE)_skg3_BrCHD0.jpg $(XDATE)_skg3_BrCHD0_pp.jpg
	montage $(XDATE)_skg3_pp_BrCHD0_*.jpg -geometry +1+1 $(XDATE)_skg3_pp_BrCHD0.jpg 
	montage $(XDATE)_skg3_BrCHD0_*.jpg -geometry +1+1 $(XDATE)_skg3_BrCHD0.jpg 


# [BrO3^-]_0: 0.01-0.30, step 0.01
# [BrCHD]_0: 0.001-0.030, step 0.001
.PHONY: BrO3BrCHDplotpp
BrO3BrCHDplotpp:
	rm -f skg3pp.jpg plot.mat skg3.jpg
	chmod 755 skg3pp.m
	for BrCHD in $(shell seq 0.001 0.001 0.030) ; do \
	for BrO3 in $(shell seq 0.01 0.01 0.30) ; do \
        XSIM=BrO30_$${BrO3}_BrCHD0_$${BrCHD} ; \
		rm -f skg3.jpg skg3pp.jpg plot.mat $(XDATE)_*$${XSIM}.jpg ; \
		./skg3pp.m $$BrCHD $$BrO3 || exit 1 ; \
		mv skg3pp.jpg $(XDATE)_skg3_pp_$${XSIM}.jpg ; \
		mv skg3.jpg $(XDATE)_skg3_$${XSIM}.jpg ; \
	done ; done
	rm -f plot.mat $(XDATE)_skg3_BrO30_BrCHD0.jpg $(XDATE)_skg3_BrO30_BrCHD0_pp.jpg
	montage $(XDATE)_skg3_pp_BrO30_*_BrCHD0_*.jpg -geometry +1+1 -tile 30 $(XDATE)_skg3_pp_BrO30_BrCHD0.jpg 
	montage $(XDATE)_skg3_BrO30_*_BrCHD0_*.jpg -geometry +1+1 -tile 30 $(XDATE)_skg3_BrO30_BrCHD0.jpg 

.PHONY: BrO3BrCHDsplitplot
BrO3BrCHDsplitplot:
	rm -f $(XDATE)_plot_BrCHD0_*.jpg $(XDATE)_pp_BrCHD0_*.jpg 
	for BrCHD in $(shell seq 0.001 0.001 0.030) ; do \
		montage *_skg3_pp_BrO30_*_BrCHD0_$${BrCHD}.jpg -geometry +1+1 -tile 30 $(XDATE)_pp_BrCHD0_$${BrCHD}.jpg ; \
		montage *_skg3_BrO30_*_BrCHD0_$${BrCHD}.jpg -geometry +1+1 -tile 30 $(XDATE)_plot_BrCHD0_$${BrCHD}.jpg ; \
	done 


#./skg4.m 0.2 0.022 0 0 ==> LC
#./skg4.m 0.04 0.001 0 0 ==> LC
.PHONY: skg4plot
skg4plot:
	rm -f plot.map skg4.jpg
	for i in $(shell seq 0.18 0.02 0.4); do \
		for j in $(shell seq  0.02 0.002 0.04); do \
			k1=`echo "((0.2 - 0.001)/2.5)" | bc -l` ; \
			k2=`echo "((0.022 - 0.001)/2.5)" | bc -l` ; \
			k1=`echo "($$k1 * $$i)/1000" | bc -l` ; \
			k2=`echo "($$k2 * $$j)/1000" | bc -l` ; \
			echo $$k1 $$k2 ; \
			rm -f skg4.jpg skg4x.jpg ; \
			./skg4.m $$i $$j 0.1 0.3 ; \
			ix=`printf "%05d" $$i` ; \
			ij=`printf "%05d" $$j` ; \
			montage skg4.jpg skg4x.jpg -tile 1x2 -geometry +1+1 $(XDATE)_skg4_$${i}_$${j}.jpg ; \
			rm -f skg4.jpg skg4x.jpg ; \
		done ; \
	done
#	echo	./skg4.m 0.2 0.022 0.0796 0.0084
#	echo ./skg4.m 0.2 0.022 0.0796 0.0084
#	echo xdg-open skg4.jpg &


.PHONY: skg4summary
skg4summary:
	./skg4.m 0.2 0.022 0 0 
	montage skg4.jpg skg4x.jpg -tile 1x2 -geometry +1+1 $(XDATE)_LC1.jpg
	./skg4.m 0.04 0.001 0 0
	montage skg4.jpg skg4x.jpg -tile 1x2 -geometry +1+1 $(XDATE)_LC2.jpg
	./skg4.m 0.2 0.022 0.1 0.3
	montage skg4.jpg skg4x.jpg -tile 1x2 -geometry +1+1 $(XDATE)_RV1.jpg
	montage $(XDATE)_LC1.jpg $(XDATE)_LC2.jpg $(XDATE)_RV1.jpg  -tile 3x1 -geometry +1+1 $(XDATE)_summary.jpg

skg4test:
	$(OCTAVEBIN) ./skg4.m 0.2 0.022 0 0 

.PHONY: skg5plot
skg5plot:
	./skg5.m 0.2 0.02 0.1 0.3

.PHONY: plot
plot:
	rm -f skg.jpg
	time octave skg.m
	(killall gpicview > /dev/null 2>&1) || true
	xdg-open skg.jpg

.PHONY: plot2
plot2:
	rm -f skg2.jpg
	time octave skg2.m
	(killall gpicview > /dev/null 2>&1) || true
	xdg-open skg2.jpg

.PHONY: rk4plot
rk4plot: pointskg
	./pointskg > pointrk4.dat
	(echo 'set terminal png'; echo 'set output "pointrk4.png"'; echo 'plot "pointrk4.dat" using 1:2 with lines, "pointrk4.dat" using 1:3 with lines, "pointrk4.dat" using 1:4 with lines') | gnuplot
	rm -f pointrk4.dat

# internal test
.PHONY: xtest
xtest:
	$(MAKE) clean
	$(MAKE) TESTFLAG=1
	./$(BINFILE)

sk::
	rm -f sk.mat
	$(OCTAVEBIN) sk.m > ~/$(XDATETIME).log 2>&1

sk_jac.m: sk.m m2p.sh jac.py
	rm -f tmp.dat tmp.py
	bash m2p.sh sk.m > tmp.dat
	python jac.py tmp.dat > tmp.py
	python tmp.py | sed -E 's/kf([0-9]{1,2})/kf\(\1\)/g' \
		| sed -E 's/kr([0-9]{1,2})/kr\(\1\)/g' \
		| sed -E 's/x([0-9]{1,2})/x\(\1\)/g' > sk_jac.m
	rm -f tmp.dat tmp.py

#*****
