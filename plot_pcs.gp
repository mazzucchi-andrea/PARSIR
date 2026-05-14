# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

set datafile separator comma
set datafile missing "NaN"
set terminal png size 1200,800 font 'Arial,26' noenhanced
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
set style line 1 lw 2 pt 6 ps 3.0  # grid_ckpt: open circle
set style line 2 lw 2 pt 4 ps 3.0  # chunk_ckpt: open square
set style line 3 lw 2 pt 8 ps 3.0  # full_ckpt: open triangle

set title sprintf("PCS (MIT = %s, Speculative Window %s)", ARG1, ARG2)
set xlabel "Threads"
set ylabel "Throughput (events/s)"
set output sprintf("plots/pcs/throughput_mit_%s_spec_window_%s_obj1024.png", ARG1, ARG2)
plot for [i=1:words(ckpt_types)] 'pcs_plot_data.csv' using \
    (strcol(COL_CKPT) eq word(ckpt_types,i) ? column(COL_THREADS) : 1/0): \
    (strcol(COL_CKPT) eq word(ckpt_types,i) ? column(COL_TPUT) : 1/0): \
    (strcol(COL_CKPT) eq word(ckpt_types,i) ? column(COL_TPUT_DELTA) : 1/0) \
    with yerrorlines ls i title word(ckpt_types,i)

unset output
