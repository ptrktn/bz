#****h* erhelper/erhelper_Makefile
# NAME
#   erhelper_Makefile
# USAGE 
#   make [install|clean|test]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = erhelper
BINFILE     = erhelper.py
TARGETS     = $(BINFILE)
BINOBJECTS  = 

# link to bzsim library
USEBZSIM=0

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

install: $(BINDIR)/$(BINFILE)

.PHONY: test
test: test-lv

.PHONY: test-lv
test-lv:
	rm -f erhelper.mat erhelper.m
	./erhelper.py examples/lotka-volterra.txt
	$(OCTAVE) erhelper.m
	../tools/xplot.sh erhelper.mat

.PHONY: otest
otest:
	rm -f erhelper.mat erhelper.m
	./erhelper.py examples/oregonator.txt

.PHONY: clean
clean:
	$(RM) core *~ *.BAK octave-workspace *.jpg *.mat *.m

#*****
