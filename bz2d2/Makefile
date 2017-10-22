#****h* bz2d2/Makefile_bz2d2
# NAME
#   Makefile_bz2d2
# SYNOPSIS
#   make 
#   make doc
#   make test
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE=bz2d2
BINFILE=$(MODULE)
TARGETS=$(BINFILE)
BINOBJECTS = bz2d2.o
# link to bzsim library
USEBZSIM=1
# do not remove or comment out the following line
include ../Makefile
# additional compiler flags
CFLAGS += 

install: $(BINDIR)/$(BINFILE) 

#$(MODULELIBDIR)/bz2d2base.dat

$(MODULELIBDIR)/bz2d2base.dat: bz2d2base.dat
	$(INSTALL) bz2d2base.dat $(MODULELIBDIR)/bz2d2base.dat

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINOBJECTS) core *~ *.BAK *.png *.jpg *.gif
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi

.PHONY: test
test: $(TARGETS)
	$(RM) *.gif *.jpg
	./$(BINFILE) > $(BINFILE).dat
	mogrify -format gif *.jpg
	ffmpeg -y -b 800 -i %d.gif $(BINFILE).mpg
	$(RM) *.jpg *.gif

.PHONY: data
data: $(TARGETS)
	touch data.dat
	tclsh period.tcl >> data.dat

.PHONY: plot
plot:
	gnuplot bz2d2.gp
	gimp period.png

plot2:
	(echo 'set terminal png'; echo 'set output "tmp.png"'; echo 'plot "bz2d2.dat" using 1:2 with lines, "bz2d2.dat" using 1:3 with lines') | gnuplot
	nautilus tmp.png

iplot:
	(echo 'set terminal png'; echo 'set output "tmp.png"'; echo 'plot "bz2d2.dat" using 1:4 with lines') | gnuplot
	nautilus tmp.png

cplot: corr
	$(RM) cc.dat
	./corr < bz2d2.dat > cc.dat
	(echo 'set terminal png'; echo 'set output "tmp.png"'; echo 'plot "cc.dat" with lines') | gnuplot
	nautilus tmp.png

bz2d2.o: $(INCDIR)/bzsim.h bz2d2.c

corr: corr.o
	$(CC) -o $@ $< -lfftw3 -lm

fft: fft.o
	$(CC) -o $@ $< -lfftw3 -lm

#*****

