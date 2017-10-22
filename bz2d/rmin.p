set nokey
set xlabel 'p'
set ylabel 'r_{ min }'
set terminal postscript eps enhanced
set output 'rmin.eps'
plot 'rmin.dat' using 1:2
set terminal png
set output 'rmin.png'
replot
