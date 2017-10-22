# $Id: bz2d2.gp,v 1.1 2003/08/11 02:22:21 petterik Exp $

set terminal png
set xlabel "Da/Db"
set ylabel "Ta/Tb"
set output "period.png"
plot "data.dat" using 1:2:3 with yerrorbars, \
     "data.dat" using 1:2 smooth unique

