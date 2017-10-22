#****h* bz3d2/bz3d2_Makefile
# NAME
#   bz3d2_Makefile
#   $Id: Makefile,v 1.3 2004/01/12 15:27:38 petterik Exp $
# USAGE 
#   make [install|test|test2|clean]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = bz3d2
BINFILE     = $(MODULE)
TARGETS     = $(BINFILE)
BINOBJECTS  = bz3d2.o

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

install: $(BINDIR)/$(BINFILE)

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINFILE2) $(BINOBJECTS) core *~ *.BAK *.jpg test.dat
	if [ -d ./res ] ; then $(RMDIR) ./res ; fi

.PHONY: test
test: $(TARGETS)
	$(RMDIR) res
	./$(BINFILE) bz3d2.dat

# testing new study model
.PHONY: test2
test2: $(TARGETS)
	$(RMDIR) res
	$(INSTALL) bz3d2.dat test.dat
	echo "MODEL 4" >> test.dat
	echo "BOUNDARY 1" >> test.dat
	echo "SNAP_DF3 0" >> test.dat
	echo "XT_PLOT 0" >> test.dat
	echo "PACEMAKER_RADIUS 0" >> test.dat
	echo "NX 64" >> test.dat
	echo "NY 64" >> test.dat
	echo "NZ 16" >> test.dat
	echo "SNAPZ 8" >> test.dat
	echo "SNAP 1000" >> test.dat
	echo "P_SRC 0.025" >> test.dat
	echo "TF_F 1.25" >> test.dat
	echo "ITERATIONS 3000000" >> test.dat
	./$(BINFILE) test.dat

bz3d.o: $(INCDIR)/bzsim.h bz3d.c

#*****
