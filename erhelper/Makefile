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
USEMATH=1

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

lsoda: lsoda.c erhelper.h

.PHONY: test-lsoda
test-lsoda:
	rm -f erhelper.h erhelper.mat
	./erhelper.py examples/brusselator.txt
	$(MAKE) lsoda
	./lsoda > erhelper.mat
	../tools/xplot.sh erhelper.mat

.PHONY: lsoda-sim
lsoda-sim: lsoda
	rm -f erhelper.mat
	./lsoda > erhelper.mat

.PHONY: lsoda-dat
lsoda-dat:
	rm -f ~/sk.zip
	rm -f *.jpg
	../tools/xplot.sh erhelper.mat 'HOBr Br2 Br HBrO2 BrO3 H2BrO2 Br2O4 BrO2 H2Q Ox Red CHD CHDE BrCHD CHED'
	zip -mq ~/sk erhelper*jpg
	zip -qrj ~/sk lsoda.c erhelper.h erhelper.m examples

.PHONY: clean
clean:
	$(RM) core *~ *.BAK octave-workspace *.jpg *.mat *.m *.h lsoda

#*****
