#****h* bzsim/Makefile_bzsim
# NAME
#   Makefile_bzsim
# SYNOPSIS
#   make 
#   make test
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE=bzsim
LIBFILE=lib$(MODULE).a
TARGETS=$(LIBFILE)
LIBOBJECTS = bzsim.o jpg.o pgm.o
BINFILE=jpegtile
USEJPG=1

# End of configurable options.
# Do not remove or comment out the following line
include ../Makefile

#CFLAGS += -D__DEBUG__ 

# The following rule is mandatory
install: $(LIBDIR)/$(LIBFILE) $(INCDIR)/$(MODULE).h

clean:
	$(RM) $(LIBOBJECTS) $(LIBFILE) core *~ *.BAK

bzsim.o: bzsim.h bzsim.c
jpg.o: bzsim.h jpg.c
pgm.o: bzsim.h pgm.c

#*****