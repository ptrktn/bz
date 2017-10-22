# ----------------------------------- #
# This is the Makefile for EZ-Scroll. #
# ----------------------------------- #

# NX, NY, NZ defined?
# -------------------
# If you wish to specify NX, NY, NZ (the number of grid points in each
# direction) at compile time, then uncomment the following and set the values
# to what you want.  They need not all be set.  Those that are set will be
# ignored in task.dat.

# NOPTION = -DNX=61 -DNY=61 -DNZ=61

# -------------------
# Choose the compiler
# -------------------

# EZ-Scroll conforms to the ANSI C standard. It will compile with cc (e.g.
# SGI's C) or gcc (GNU C).  Use at least the -O2 compiler option, the others
# are optional options.  If you use the warning options -Wall or -fullwarn
# (which you should use if you modify the code) then you may get some
# warnings. These are probably OK.

# Uncomment below for SGI cc
# --------------------------
#CC = cc 
#CFLAGS = -O2 -ansi -n32 -mips4

# Uncomment below for GNU cc
# --------------------------
#CC = cc 
#CFLAGS = -O2 -Wall -ansi 

# Uncomment below for Mac OS X
# ---------------------------- 
CC = cc
CFLAGS = -O2 -Wall -ansi 

#-D_MACOSX

# ----------------------------------------------------
# Set the libraries
# Note: In addition to specify the libraries, you 
# might need to specify the location of the 
# GL and X libraries explicitly with something like
# LDFLAGS = -L/usr/X11R6/lib 
# ----------------------------------------------------

# OpenGL with X
# -------------
#LOADLIBES = -lGL -lX11 -lXext -lm 

# Mesa with X
# ------------
#LOADLIBES = -lMesaGL -lX11 -lXext -lm 

# OpenGL without X
# ----------------
CFLAGS += -D_GLONLY
LOADLIBES = -lGL -lglut -lm

# OpenGL frameworks (Mac OS X)
# ----------------------------
#CFLAGS += -D_GLONLY
#LOADLIBES = -framework OpenGL -framework GLUT -lm -lobjc 

# -----------------------------------------------
# Probably you wont need to change anything below.
# ------------------------------------------------

OBJ=ezscroll.o ezstep3d.o ezgraph3d.o ezmarching.o

.c.o :
	$(CC) -c $(CFLAGS) $(NOPTION) $*.c

ezscroll: $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJ) -o ezscroll $(LOADLIBES)


# Dependencies
# ------------

ezscroll.o   :ezscroll.h Makefile
ezstep3d.o   :ezscroll.h ezstep3d.h Makefile
ezgraph3d.o  :ezscroll.h ezgraph3d.h ezmarching.h Makefile
ezmarching.o :ezscroll.h ezgraph3d.h ezmarching.h Makefile

clean:	tidy
	rm -f core *~ .*~ 	
	rm -f *.log *.dvi *.aux *.pgm *.jpg

tidy:
	rm -f ezscroll *.o 
	rm -f fc.dat
