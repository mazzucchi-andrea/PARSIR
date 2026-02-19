set datafile separator comma
set datafile missing "NaN"
set terminal png size 1200,800 font 'Arial,26'
set key inside right top
set grid

COL_CKPT            = 1
COL_THREADS         = 2
COL_SPEC_WINDOW     = 3
COL_OBJECTS         = 4
COL_M               = 5
COL_TPUT            = 6
COL_TPUT_DELTA      = 7
COL_RB_SPEC         = 8
COL_RB_SPEC_DELTA   = 9

ckpt_types  = "grid_ckpt chunk_ckpt full_ckpt"

set style data yerrorlines

# Define point styles for each checkpoint type
set style line 1 lw 2 pt 6 ps 3.0  # grid_ckpt: filled circle
set style line 2 lw 2 pt 4 ps 3.0  # chunk_ckpt: filled square
set style line 3 lw 2 pt 8 ps 3.0  # full_ckpt: filled triangle

set title sprintf("Phold (M = %s, Speculative Window %s)", ARG1, ARG2)
set xlabel "Threads"
set ylabel "Throughput (events/s)"
set output sprintf("plots/phold/throughput_obj1024_spec_windows%s_m%s.png", ARG2, ARG1)
plot for [c in ckpt_types] 'phold_plot_data.csv' using \
    (strcol(COL_CKPT) eq c ? column(COL_THREADS) : 1/0): \
    (strcol(COL_CKPT) eq c ? column(COL_TPUT) : 1/0): \
    (strcol(COL_CKPT) eq c ? column(COL_TPUT_DELTA) : 1/0) \
    with yerrorlines ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) title (c eq 'grid_ckpt' ? 'grid ckpt' : (c eq 'chunk_ckpt' ? 'chunk ckpt' : 'full ckpt'))
unset output

set title sprintf("Phold (M = %s, Speculative Window %s)", ARG1, ARG2)
set xlabel "Threads"
set ylabel "Rollbacks per Speculation Window"
set output sprintf("plots/phold/rollbacks_per_epoch_obj1024_spec_windows%s_m%s.png", ARG2, ARG1)

plot for [c in ckpt_types] 'phold_plot_data.csv' using \
    (strcol(COL_CKPT) eq c ? column(COL_THREADS) : 1/0) : \
    (strcol(COL_CKPT) eq c ? column(COL_RB_SPEC) : 1/0) : \
    (strcol(COL_CKPT) eq c ? column(COL_RB_SPEC_DELTA) : 1/0) \
    with yerrorlines \
    ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) \
    title (c eq 'grid_ckpt' ? 'grid ckpt' : (c eq 'chunk_ckpt' ? 'chunk ckpt' : 'full ckpt'))

unset output
