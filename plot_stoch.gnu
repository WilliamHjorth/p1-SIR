set terminal wxt size 1200,800 enhanced persist
set xlabel 'Days'
set ylabel 'Individuals'
set grid
set key top right
set title 'Stochastic Model (50 Replicates)'
set style line 1 lc rgb '#40008000' lt 1 lw 1
set style line 2 lc rgb '#40FFA500' lt 1 lw 1
set style line 3 lc rgb '#40FF0000' lt 1 lw 1
set style line 4 lc rgb '#40800080' lt 1 lw 1
set style line 5 lc rgb '#400000FF' lt 1 lw 1
plot 'data_stoch.txt' index 0 using 1:2 w l ls 1 title 'S', '' index 0 using 1:3 w l ls 2 title 'E', '' index 0 using 1:4 w l ls 3 title 'I', '' index 0 using 1:6 w l ls 5 title 'R' , \
for [i=1:49] 'data_stoch.txt' index i using 1:2 w l ls 1 notitle, for [i=1:49] '' index i using 1:3 w l ls 2 notitle, for [i=1:49] '' index i using 1:4 w l ls 3 notitle, for [i=1:49] '' index i using 1:6 w l ls 5 notitle
