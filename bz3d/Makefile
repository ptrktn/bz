#****h* bz3d/Makefile
# NAME
#   bz3d Makefile
# USAGE 
#   make [install|test|clean]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = bz3d
BINFILE     = $(MODULE)
BINFILE2    = slice3d
TARGETS     = $(BINFILE) $(BINFILE2)
BINOBJECTS  = bz3d.o
BINOBJECTS2 = slice3d.o

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

install: $(BINDIR)/$(BINFILE) $(BINDIR)/$(BINFILE2) \
         $(MODULELIBDIR)/bz3dbase.dat

$(MODULELIBDIR)/bz3dbase.dat: bz3dbase.dat
	$(INSTALL) bz3dbase.dat $(MODULELIBDIR)/bz3dbase.dat

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINFILE2) rtime *.o core *~ *.BAK *.jpg *.df3
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi

.PHONY: test
test: $(TARGETS)
	./$(BINFILE) bz3dbase.dat

#TIPDATS=new018_3_x1_98900000.dat new018_3_x2_98900000.dat
TIPDATS=new018_2_x1_3500000.dat new018_2_x2_3500000.dat
#TIPDATS=new018_3_x1_88900000.dat new018_3_x2_88900000.dat

.PHONY: test2
test2: $(BINFILE2)
	./$(BINFILE2) $(TIPDATS) > tip.dat
	cat tip.dat | $(TCLSH) xyz2df3.tcl --dat2inc > tip.inc
	povray +Itip.pov tip.ini > /dev/null 2>&1
	$(CONVERT) -rotate 90 -font Helvetica -pointsize 24 -pen white -crop 300x150+0+75 -draw "text 10,20 test2" tip.tga JPG:tip2.jpg
	montage -mode concatenate -tile 2x2 lap.png xy.png tip2.jpg JPG:tip.jpg 
	$(RM) tip.tga tip2.jpg

xx:
	cat tmp.dat | ./slice3d --stdin > tip.dat
	cat tip.dat | $(TCLSH) xyz2df3.tcl --dat2inc > tip.inc
	povray +Itip.pov tip.ini > /dev/null 2>&1
	$(CONVERT) -rotate 90 -font Helvetica -pointsize 24 -pen white -crop 300x150+0+75 -draw "text 10,20 test2" tip.tga JPG:tip2.jpg
	montage -mode concatenate -tile 2x2 lap.png xy.png tip2.jpg JPG:tip.jpg 
	$(RM) tip.tga tip2.jpg

TIPFILES = t2tip1.dat t2tip6.dat t2tip7.dat t2tip8.dat t2tip9.dat \
           t2tip10.dat t2tip11.dat
TIPPNGS  = t2tip1.dat.png t2tip6.dat.png \
           t2tip7.dat.png t2tip8.dat.png \
           t2tip9.dat.png t2tip10.dat.png t2tip11.dat.png

.PHONY: tip
tip:
	for i in $(TIPFILES) ; do \
		tclsh xyz2df3.tcl $$i tip.df3 ; \
		povray +Itip.pov tip.ini ; \
		$(CONVERT) -rotate 90 -font Helvetica -pointsize 24 -pen white -crop 300x150+0+75 -draw "text 10,20 $$i" tip.tga PNG:$$i.png ; \
		$(RM) tip.tga ; \
		done
	 montage -mode concatenate -tile 2x4 $(TIPPNGS) tip.png
	$(RM) t2*dat*.png

bz3d.o: $(INCDIR)/bzsim.h bz3d.c
slice3d.o: $(INCDIR)/bzsim.h bz3d.c

.PHONY: rt
rt: rtime
	ls -1 /var/tmp/*.jpg | head -200 | ./rtime 0.333333 5 5 > rtime1.dat
	ls -1 /var/tmp/*.jpg | tail -200 | ./rtime 0.333333 5 5 > rtime2.dat
	find /var/tmp/new018_4 -name '*.jpg' | sort | ./rtime 0.333333 5 5 > rtime.dat

# "fingering" demo -- plot r species
.PHONY: fingering
fingering: $(TARGETS)
	rm -fr f.dat fingering
	cp bz3dbase.dat f.dat
	echo "NX 32" >> f.dat
	echo "NY 32" >> f.dat
	echo "NZ 32" >> f.dat
	echo "READ_GRID 0" >> f.dat
	echo "SAVE_DF3 2" >> f.dat
	echo "ITERATIONS 1000" >> f.dat
	echo "OUTPUT_DIR fingering" >> f.dat
	echo "SIMULATION_NAME demo" >> f.dat
	./bz3d f.dat
	@for i in `(cd fingering && ls -1 *.df3)` ; do cp -f fingering/$$i fingering.df3 && povray +Ifingering.pov fingering.ini && convert -rotate 90 fingering.tga fingering/$$i.jpg ; done
	rm -f f.dat fingering.tga fingering.df3

#*****

.PHONY: homework
homework: $(BINFILE) homework2017.dat
	./$(BINFILE) homework2017.dat
