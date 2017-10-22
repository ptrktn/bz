set nokey
set xlabel 'p'
set ylabel 'S'
set terminal postscript eps enhanced
set output 'savg.eps'
plot 'savg.dat' using 1:2, 'savg.dat' using 1:2 smooth bezier
set terminal png
set output 'savg.png'
replot
