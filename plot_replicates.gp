set xlabel 'Time'
set ylabel 'Number of individuals'
set title 'Stochastic SIR Model - 50 Replicates'
set grid
set key off
set style line 1 lc rgb '#8000FF00' lt 1 lw 1
set style line 2 lc rgb '#80FF0000' lt 1 lw 1
set style line 3 lc rgb '#800000FF' lt 1 lw 1
plot for [i=0:49] 'sir_replicates.txt' index i using 1:2 with lines ls 1, \
     for [i=0:49] 'sir_replicates.txt' index i using 1:3 with lines ls 2, \
     for [i=0:49] 'sir_replicates.txt' index i using 1:4 with lines ls 3
pause -1 'Press enter to close'
