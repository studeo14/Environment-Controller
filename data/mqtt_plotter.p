# for use with gnuplot

set datafile separator "|"
plot for [col=1:2] 'monitor_data.dat' using 0:col with lines
pause -1