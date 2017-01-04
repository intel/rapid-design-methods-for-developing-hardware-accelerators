set term png
set output dir . "/data.png"

set title dir
set key top left
set ylabel "Read Bandwidth Fraction"
set xlabel "Read Response Buffer Positions"
set xtics 32,32,256

y0 = 1.0
x0 = 100
m = 0.01

y(x) = x < x0 ? y0 + m*(x-x0) : y0

data_fn = dir . "/data.txt"

fit y(x) data_fn via x0, y0, m

set yrange [0:1]

set arrow from x0+32,y0-0.2 to x0,y0 lw 0 lc 4 
set label at x0+32,y0-0.2 sprintf("%.1f,%.3f", x0, y0 )

plot [0:256] data_fn with points pointtype 4, y(x) with lines
