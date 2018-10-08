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
BINOBJECTS2 = fkn.o

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

XDATE=$(shell date +%Y%m%d)

install:: $(BINDIR)/skg.m

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINFILE2) $(BINOBJECTS) $(BINOBJECTS) \
          core *~ *.BAK *.jpg *.png \
          plot.mat nohup.out
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

.PHONY: plot
plot:
	rm -f skg.jpg
	time octave skg.m
	(killall gpicview > /dev/null 2>&1) || true
	xdg-open skg.jpg

# internal test
.PHONY: xtest
xtest:
	$(MAKE) clean
	$(MAKE) TESTFLAG=1
	./$(BINFILE)

#*****
