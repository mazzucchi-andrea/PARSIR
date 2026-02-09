set datafile separator comma
set datafile missing "NaN"

set terminal png size 1200,800 font 'Arial,16'

set title "Throughput vs Threads"
set xlabel "Threads"
set ylabel "Throughput"

set yrange [0:*]

set key outside right
set grid

COL_CKPT      = 1
COL_THREADS   = 2
COL_LOOKAHEAD = 3
COL_OBJECTS   = 4
COL_TPUT      = 5
COL_TPUT_DELTA= 6

ckpt_types  = "grid_ckpt chunk_ckpt chunk_full_ckpt"
objects_lst = "1024 4096"
lookaheads  = "0.25 0.5 1.0"

# ---------- styles ----------
set style data yerrorlines

# point types per lookahead
pt_la(l) = (l == 0.25 ? 7 : \
            l == 0.5  ? 5 : \
            l == 1.0  ? 9 : 7)

ls_unique(idx) = idx  # gnuplot will auto-cycle styles

# Loop over objects to generate one plot per objects value
do for [o in objects_lst] {

    # Set output file per objects value
    set output sprintf("plots/phold/throughput_obj%s.png", o)

    plot for [c in ckpt_types] for [l in lookaheads] 'phold_bench.csv' using \
        (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_LOOKAHEAD) == real(l) ? column(COL_THREADS) : 1/0): \
        (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT) : 1/0): \
        (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT_DELTA) : 1/0) \
        with yerrorlines lw 2 pt pt_la(real(l)) ps 1.4 title sprintf("%s, la=%s", c, l) noenhanced

    unset output
}

set datafile separator comma
set datafile missing "NaN"

set terminal png size 1200,800 font 'Arial,16'

set title "Throughput vs Threads"
set xlabel "Threads"
set ylabel "Throughput"

set yrange [0:*]

set key outside right
set grid

COL_CKPT      = 1
COL_THREADS   = 2
COL_LOOKAHEAD = 3
COL_OBJECTS   = 4
COL_TA        = 5
COL_TPUT      = 6
COL_TPUT_DELTA= 7

ckpt_types  = "grid_ckpt chunk_ckpt chunk_full_ckpt"
objects_lst = "1024 4096"
lookaheads  = "0.25 0.5 1.0"
ta          = "0.4 0.1"


 # Loop over ta and objects to generate one plot per (object, ta) tuple
do for [t in ta] {
    do for [o in objects_lst] {
        # Set output file per (object, ta) tuple
        set output sprintf("plots/pcs/throughput_obj%s_ta%s.png", o, t)

        plot for [c in ckpt_types] for [l in lookaheads] 'pcs_bench.csv' using \
            (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_THREADS) : 1/0): \
            (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT) : 1/0): \
            (strcol(COL_CKPT) eq c && int(column(COL_OBJECTS)) == int(o) && column(COL_TA) == real(t) && column(COL_LOOKAHEAD) == real(l) ? column(COL_TPUT_DELTA) : 1/0) \
            with yerrorlines lw 2 pt pt_la(real(l)) ps 1.4 title sprintf("%s, la=%s", c, l) noenhanced

        unset output
    }
}