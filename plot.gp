set datafile separator comma
set datafile missing "NaN"
set terminal png size 1200,800 font 'Arial,26'
set key outside right
set grid

set style fill solid 0.4

COL_CKPT      = 1
COL_THREADS   = 2
COL_LOOKAHEAD = 3
COL_OBJECTS   = 4
COL_M         = 5
COL_TPUT      = 6
COL_TPUT_DELTA= 7
COL_EPOCHS    = 8
COL_RB        = 9

ckpt_types  = "grid_ckpt chunk_ckpt full_ckpt"
m_values    = "1 1000"
spec_windows  = "0.25 0.5 1.0"

set style data yerrorlines

# Define point styles for each checkpoint type
set style line 1 lw 2 pt 6 ps 3.0  # grid_ckpt: filled circle
set style line 2 lw 2 pt 4 ps 3.0  # chunk_ckpt: filled square
set style line 3 lw 2 pt 8 ps 3.0  # full_ckpt: filled triangle

# Loop over objects to generate one plot per objects value
do for [m in m_values] {
    do for [l in spec_windows] {
        set title sprintf("Phold model (M = %s, Speculative Window %s)", m, l)
        set xlabel "Threads"
        set ylabel "Throughput (events/s)"
        set output sprintf("plots/phold/throughput_obj1024_spec_windows%s_m%s.png", l, m)
        plot for [c in ckpt_types] 'phold_bench.csv' using \
        (strcol(COL_CKPT) eq c && int(column(COL_M)) == int(m) && \
        column(COL_LOOKAHEAD) == real(l) ? column(COL_THREADS) : 1/0): \
        (strcol(COL_CKPT) eq c && int(column(COL_M)) == int(m) && \
        column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT) : 1/0): \
        (strcol(COL_CKPT) eq c && int(column(COL_M)) == int(m) && \
        column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT_DELTA) : 1/0) \
        with yerrorlines ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) \
        title sprintf("%s", c) noenhanced
        unset output
    }
}

COL_CKPT      = 1
COL_THREADS   = 2
COL_LOOKAHEAD = 3
COL_OBJECTS   = 4
COL_TA        = 5
COL_TPUT      = 6
COL_TPUT_DELTA= 7
COL_EPOCHS    = 8
COL_RB        = 9

ckpt_types  = "grid_ckpt chunk_ckpt full_ckpt"
spec_windows  = "0.25 0.5 1.0"
ta          = "0.4 0.1"

do for [t in ta] {
    do for [l in spec_windows] {
        set title sprintf("PCS model (MIT = %s, Speculative Window %s)", t, l)
        set xlabel "Threads"
        set ylabel "Throughput (events/s)"
        set output sprintf("plots/pcs/throughput_obj1024_spec_windows%s_mit%s.png", l, t)
        plot for [c in ckpt_types] 'pcs_bench.csv' using \
        (strcol(COL_CKPT) eq c && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_THREADS) : 1/0): \
        (strcol(COL_CKPT) eq c && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT) : 1/0): \
        (strcol(COL_CKPT) eq c && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT_DELTA) : 1/0) \
        with yerrorlines ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) title sprintf("%s", c) noenhanced
        unset output
    }
}

set key outside right
set grid

set style data linespoints

do for [m in m_values] {
    do for [l in spec_windows] {
        set title sprintf("Phold model (M = %s, Speculative Window %s)", m, l)
        set xlabel "Threads"
        set ylabel "Rollbacks per Epoch"
        set output sprintf("plots/phold/rollbacks_per_epoch_obj1024_spec_windows%s_m%s.png", l, m)
        set autoscale y
             safe_ratio(rb, ep) = (rb == 0 ? 0 : rb / (ep == 0 ? 1 : ep))
        plot for [c in ckpt_types] 'phold_bench.csv' using \
    (strcol(COL_CKPT) eq c && \
     int(column(COL_M)) == int(m) && \
     strcol(COL_LOOKAHEAD) eq l \
        ? column(COL_THREADS) : 1/0) : \
    (strcol(COL_CKPT) eq c && \
     int(column(COL_M)) == int(m) && \
     strcol(COL_LOOKAHEAD) eq l \
        ? safe_ratio(column(COL_RB), column(COL_EPOCHS)) \
        : 1/0) \
    ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) \
    title sprintf("%s", c) noenhanced
        
        unset output
    }
}

do for [t in ta] {
    do for [l in spec_windows] {
        set title sprintf("PCS model (MIT = %s, Speculative Window %s)", t, l)
        set xlabel "Threads"
        set ylabel "Rollbacks per Epoch"
        set output sprintf("plots/pcs/rollbacks_per_epoch_obj1024_spec_windows%s_mit%s.png", l, t)
        set autoscale y

        plot for [c in ckpt_types]  'pcs_bench.csv' using \
            (strcol(COL_CKPT) eq c && int(column(COL_TA)) == int(t) && \
            column(COL_LOOKAHEAD) == real(l)  ? column(COL_THREADS) : 1/0): \
            (strcol(COL_CKPT) eq c && int(column(COL_TA)) == int(t) && \
            column(COL_LOOKAHEAD) == real(l) && column(COL_EPOCHS) > 0 ? (column(COL_RB) / column(COL_EPOCHS)) : 1/0) \
            ls (c eq 'grid_ckpt' ? 1 : (c eq 'chunk_ckpt' ? 2 : 3)) title sprintf("%s", c) noenhanced

        unset output
    }
}