set terminal png
set output "tmp.png"
# set nokey
set yrange [0.0:1.0]
set xrange [0:1024]
set xlabel "space"
set ylabel "c(x,t)"

plot "sim1anim.tmp" using 1:2 title "A" with lines, "sim1anim.tmp" using 1:3 title "B" with lines, 	 "sim1anim.tmp" using 1:4 title "U" with lines,	 "sim1anim.tmp" using 1:5 title "V" with lines,	 "sim1anim.tmp" using 1:6 title "W" with lines


