#****h* bz2d/Makefile_bz2d
# NAME
#   Makefile_bz2d
# SYNOPSIS
#   make 
#   make doc
#   make test
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE=bz2d
BINFILE=$(MODULE)
TARGETS=$(BINFILE)
BINOBJECTS = bz2d.o
# link to bzsim library
USEBZSIM=1
# do not remove or comment out the following line
include ../Makefile
# additional compiler flags
CFLAGS += 

install: $(BINDIR)/$(BINFILE) $(MODULELIBDIR)/bz2dbase.dat
doc: $(DOCDIR)/$(MODULE).pdf

$(DOCDIR)/$(MODULE).pdf: $(MODULE).pdf
	$(INSTALL) $(MODULE).pdf $(DOCDIR)/$(MODULE).pdf

$(MODULELIBDIR)/bz2dbase.dat: bz2dbase.dat
	$(INSTALL) bz2dbase.dat $(MODULELIBDIR)/bz2dbase.dat

$(MODULE).pdf: $(MODULE).c Makefile 
	$(ROBODOC) --src $(MODULE).c --doc $(MODULE) --lock --singlefile --latex
	$(LATEX) $(MODULE).tex
	$(LATEX) $(MODULE).tex
	$(DVIPDF) $(MODULE).dvi $(MODULE).pdf

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINOBJECTS) core *~ *.BAK *.mpg *.pdf *.tex \
              *.aux *.idx *.toc *.dvi
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi

.PHONY: test
test: $(TARGETS)
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi
	./$(BINFILE) bz2dbase.dat

.PHONY: demo
demo: $(TARGETS)
	if [ -d ./demo ] ; then $(RMDIR) ./demo ; fi
	cp -f bz2dbase.dat demo.dat
ifneq "$(MYDENSITY)" ""
	echo "Using user defined density $(MYDENSITY)"
	echo "DIFFUSION_SPOT_DENSITY $(MYDENSITY)" >> demo.dat
endif
	echo "OUTPUT_DIR demo" >> demo.dat
	echo "SIMULATION_NAME demo" >> demo.dat
	echo "NX 256" >> demo.dat
	echo "NY 256" >> demo.dat
	echo "SNAP_COLOUR 1" >> demo.dat
	echo "ITERATIONS 100000" >> demo.dat
	echo "SNAP 50" >> demo.dat
	./$(BINFILE) demo.dat
	cd demo && ffmpeg -y -b 1600 -i %010d.jpg ../demo.mp4
	cd demo && ffmpeg -y -b 1600 -i %010d.jpg ../demo.mpg

.PHONY: demos
demos:
	@for d in 0.0100 0.0625 0.0750 0.1500 ; do $(MAKE) demo MYDENSITY=$$d && mv -f demo.mp4 demo_$$d.mp4 && mv -f demo.mpg demo_$$d.mpg ; done

.PHONY: model3
model3: $(TARGETS)
	sh model3.sh

.PHONY: griddemo
griddemo: $(TARGETS)
	if [ -d ./demo ] ; then $(RMDIR) ./demo ; fi
	cp -f bz2dbase.dat demo.dat
	echo "OUTPUT_DIR demo" >> demo.dat
	echo "SIMULATION_NAME demo" >> demo.dat
	echo "DIFFUSION_SPOT_DENSITY 0.1" >> demo.dat
	echo "NX 256" >> demo.dat
	echo "NY 256" >> demo.dat
	echo "ITERATIONS 1" >> demo.dat
	./$(BINFILE) demo.dat

bz2d.o: $(INCDIR)/bzsim.h bz2d.c

#*****

