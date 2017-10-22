CC=cc
#CFLAGS= -O2 -DVISUAL
CFLAGS= -O3 -mips4 -64
VLIBS= #-lgl 
MLIBS= -lm
OPTS=

MATRIX= matrix.o rgb.o

matrix : $(MATRIX)
	$(CC) $(CFLAGS) -o matrix $(MATRIX) $(MLIBS) $(VLIBS)

matrix.o : matrix.c
	$(CC) $(CFLAGS) -c -o matrix.o matrix.c

flame.o : flame.c
rgb.o : rgb.c rgb.h

clean:
	rm -f *.o *~ core 
        
