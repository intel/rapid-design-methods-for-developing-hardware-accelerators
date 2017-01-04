set term png
set output dir . "/data.png"

set title dir
set key bottom left
set ylabel "Read Bandwidth Fraction"
set xlabel "Read Response Buffer Positions"
set xtics 16,16,64

y0 = 1.0

y(x) = y0

data_fn = dir . "/data.txt"

fit y(x) data_fn via y0

set yrange [0:1]

set arrow from 1+8,y0-0.2 to 1,y0 lw 0 lc 4 
set label at 1+8,y0-0.2 sprintf("%d,%.3f", 1, y0 )

plot [0:64] data_fn with points pointtype 4, y(x) with lines
