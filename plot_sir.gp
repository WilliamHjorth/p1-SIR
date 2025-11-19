set xlabel 'Time'
set ylabel 'Population'
set title 'Stochastic SIR Model'
set grid
set key right top
plot 'sir_data.txt' using 1:2 with lines lw 2 linecolor rgb 'green' title 'S', \
     'sir_data.txt' using 1:3 with lines lw 2 linecolor rgb 'red' title 'I', \
     'sir_data.txt' using 1:4 with lines lw 2 linecolor rgb 'blue' title 'R'
pause -1 'Press enter to close'
