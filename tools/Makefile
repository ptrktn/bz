#****h* tools/tools_Makefile
# NAME
#   tools_Makefile
#   $Id: Makefile,v 1.2 2004/12/15 05:00:29 petterik Exp $
# USAGE 
#   make [install|clean|test]
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE      = tools
BINFILE     = 
TARGETS     = $(BINFILE)
BINOBJECTS  = 

# link to bzsim library
USEBZSIM=0

# do not remove or comment out the following line
include ../Makefile

# additional compiler flags
CFLAGS += 

install: $(BINDIR)/plot.sh $(BINDIR)/viewimg.sh

$(BINDIR)/plot.sh: plot.sh
	install -m 755 $< $@

$(BINDIR)/viewimg.sh: viewimg.sh
	install -m 755 $< $@

.PHONY: test
test:
	echo "not implemented"

.PHONY: clean
clean:
	$(RM) core *~ *.BAK

#*****
