#****h* view3d/Makefile_eledata
# NAME
#   Makefile_view3d
# USAGE
#   make [test]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

RHOST=kuroneko
MODULE=eledata
BINFILE=$(MODULE)
TARGETS=$(BINFILE)
BINOBJECTS = eledata.o
# link to bzsim library
USEBZSIM=1
# link to Open GL
USEGL=0

# do not remove or comment out the following line
include ../Makefile
# additional compiler flags
CFLAGS += 

# install: $(BINDIR)/$(BINFILE) $(MODULELIBDIR)/bz2dbase.dat
# $(MODULELIBDIR)/bz2dbase.dat: bz2dbase.dat
# 	$(INSTALL) bz2dbase.dat $(MODULELIBDIR)/bz2dbase.dat

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINOBJECTS) core *~ *.BAK *.jpg *.pgm


#*****

.PHONY: test
test: $(TARGETS)
	./$(BINFILE) data.csv

.PHONY: mpg
mpg: test
	rcp mkmpg.sh *.pgm $(RHOST):/tmp
	rsh $(RHOST) chmod 744 /tmp/mkmpg.sh
	rsh $(RHOST) /tmp/mkmpg.sh /tmp/avg.mpg /tmp 'avg*.pgm'
	rsh $(RHOST) /tmp/mkmpg.sh /tmp/raw.mpg /tmp 'raw*.pgm'
	rsh $(RHOST) /tmp/mkmpg.sh /tmp/bas.mpg /tmp 'bas*.pgm'
	rsh $(RHOST) /tmp/mkmpg.sh /tmp/sub.mpg /tmp 'sub*.pgm'
	rsh $(RHOST) /tmp/mkmpg.sh /tmp/fnl.mpg /tmp 'fnl*.pgm'
	for i in avg raw bas sub fnl ; do rcp $(RHOST):/tmp/$$i.mpg /tmp; done
	rsh $(RHOST) 'rm -f /tmp/*.pgm; rm -f /tmp/*.pnm'

install:
	@echo this is not here yet








