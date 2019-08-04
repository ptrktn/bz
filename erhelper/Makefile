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
	./erhelper.py lv.txt

.PHONY: clean
clean:
	$(RM) core *~ *.BAK octave-workspace

#*****
