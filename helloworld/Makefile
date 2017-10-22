#****h* helloworld/helloworld_Makefile
# NAME
#   helloworld_Makefile
#   $Id: Makefile,v 1.1 2004/09/30 12:30:48 petterik Exp $
# USAGE 
#   make [install|test|clean]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = helloworld
BINFILE     = $(MODULE)
TARGETS     = $(BINFILE)
BINOBJECTS  = $(MODULE).o

# link to bzsim library
USEBZSIM=1

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

# test build?
ifeq "$(TESTFLAG)" "1"
CFLAGS += -D__TEST
endif

install: $(BINDIR)/$(BINFILE)

.PHONY: test
test: $(TARGETS)
	./$(BINFILE)

.PHONY: clean
clean:
	$(RM) $(BINFILE) $(BINOBJECTS) core *~ *.BAK

#*****
