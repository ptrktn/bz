#****h* view3d/Makefile_view3d
# NAME
#   Makefile_view3d
# USAGE
#   make [test] 
# SEE ALSO
#   Toplevel Makefile
# SOURCE

MODULE=view3d
BINFILE=$(MODULE)
TARGETS=$(BINFILE)
BINOBJECTS = view3d.o
# link to bzsim library
USEBZSIM=1
# link to Open GL
USEOPENGL=1

# do not remove or comment out the following line
#include ../Makefile
# additional compiler flags
#CFLAGS += 

# install: $(BINDIR)/$(BINFILE) $(MODULELIBDIR)/bz2dbase.dat
# $(MODULELIBDIR)/bz2dbase.dat: bz2dbase.dat
# 	$(INSTALL) bz2dbase.dat $(MODULELIBDIR)/bz2dbase.dat

# .PHONY: clean
# clean:
# 	$(RM) $(BINFILE) $(BINOBJECTS) core *~ *.BAK
# 	if [ -d ./res ] ; then $(RMDIR) ./res ; fi

# .PHONY: test
# test: $(TARGETS)
# 	./$(BINFILE) bz2dbase.dat

# bz2d.o: $(INCDIR)/bzsim.h bz2d.c

#*****


CC = c++
CFLAGS = -O2 -Wall -ansi 

# OpenGL frameworks (Mac OS X)
# ----------------------------
ifeq "$(OSTYPE)" "darwin"
CFLAGS += -D_MACOSX
LOADLIBES = -framework OpenGL -framework GLUT -lm -lobjc 
else
# default to GNU/Linux - ugly but works...
LOADLIBES = -lglut -lm
endif

CFLAGS +=
OBJ=view3d.o

.c.o :
	$(CC) -c $(CFLAGS) $(NOPTION) $*.c

view3d: $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJ) -o view3d $(LOADLIBES)

view3d.o   :view3d.c Makefile

clean:
	rm -f core *~ .*~ *.log *.dvi *.aux view3d *.o 

test: view3d
	./view3d x1.dat x2.dat

install:
	@echo this is not here yet








