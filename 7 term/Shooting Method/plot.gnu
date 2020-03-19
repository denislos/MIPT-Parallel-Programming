set terminal png enhanced font "Consolas, 9"
set output "plot_1_process.png"

set xlabel "t"
set xrange [0:3 * 3.14]
set ylabel "y"
set yrange [-3:10]
set grid


plot "shooting_data.dat" title "", "final_data.dat" title ""#, "true_function.dat" title ""
