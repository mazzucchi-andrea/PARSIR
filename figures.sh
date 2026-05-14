#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# Parse command line arguments
MODE="both"
if [[ $# -eq 1 ]]; then
    if [[ "$1" == "phold" || "$1" == "pcs" ]]; then
        MODE="$1"
    else
        echo "Usage: $0 [phold|pcs]"
        echo "  phold - Generate only PHOLD plots"
        echo "  pcs   - Generate only PCS plots"
        echo "  (no argument) - Generate both"
        exit 1
    fi
fi

LOOKAHEAD=(0.25 0.5 1.0)

# Detect total CPU threads and compute 25%, 50%, 100%
TOTAL_THREADS=$(nproc)
T25=$(( TOTAL_THREADS / 4 ))
T50=$(( TOTAL_THREADS / 2 ))
# Ensure minimums of 1
[[ $T25 -lt 1 ]] && T25=1
[[ $T50 -lt 1 ]] && T50=1
THREADS=($T25 $T50 $TOTAL_THREADS)

RUN=5
WARMUP=10
DURATION=60

OBJECTS=(128 1024)
M=(1 10 100)  
P_SHIFT=(4 5 6 7 8)

MIT=(0.4 0.1)

COMMITTED=1

# PHOLD plots generation
if [[ "$MODE" == "phold" || "$MODE" == "both" ]]; then
    echo "Generating PHOLD plots..."
    rm -rf plots/phold
    mkdir -p plots/phold
    gcc -O3 get_phold_data.c -o get_phold_data -lm
    for l in "${LOOKAHEAD[@]}"; do
        for o in "${OBJECTS[@]}"; do
            for m in "${M[@]}"; do
                for p in "${P_SHIFT[@]}"; do
                    for t in "${THREADS[@]}"; do
                        ./get_phold_data -r $RUN -t $t -s $l -o $o -m $m -p $p -c $COMMITTED
                    done 
                    gnuplot -c plot_phold.gp $l $o $m $p
                    rm phold_plot_data.csv;
                done          
            done
        done
    done
    rm -f get_phold_data
    cd plots/phold
    montage \
        throughput_m_1_spec_window_0.25_obj1024.png \
        throughput_m_1_spec_window_0.5_obj1024.png \
        throughput_m_1_spec_window_1.0_obj1024.png \
        rollbacks_m_1_spec_window_0.25_obj1024.png \
        rollbacks_m_1_spec_window_0.5_obj1024.png \
        rollbacks_m_1_spec_window_1.0_obj1024.png \
        throughput_m_100_spec_window_0.25_obj1024.png \
        throughput_m_100_spec_window_0.5_obj1024.png \
        throughput_m_100_spec_window_1.0_obj1024.png \
        rollbacks_m_100_spec_window_0.25_obj1024.png \
        rollbacks_m_100_spec_window_0.5_obj1024.png \
        rollbacks_m_100_spec_window_1.0_obj1024.png \
        -tile 3x4 -geometry +2+2 fig6.png
    cd ../..
    echo "PHOLD plots generated successfully!"
fi
 
# PCS plots generation
if [[ "$MODE" == "pcs" || "$MODE" == "both" ]]; then
    echo "Generating PCS plots..."
    rm -rf plots/pcs
    mkdir -p plots/pcs
    gcc -O3 get_pcs_data.c -o get_pcs_data -lm
    for l in "${LOOKAHEAD[@]}"; do
        for o in "${OBJECTS[@]}"; do
            for m in "${MIT[@]}"; do
                for t in "${THREADS[@]}"; do
                    ./get_pcs_data -r $RUN -t $t -s $l -o $o -m $m -c $COMMITTED
                done 
                gnuplot -c plot_pcs.gp $m $l
                rm pcs_plot_data.csv;          
            done
        done
    done
    rm -f get_pcs_data
    cd plots/pcs
    montage \
        throughput_mit_0.4_spec_window_0.25_obj1024.png \
        throughput_mit_0.4_spec_window_0.5_obj1024.png \
        throughput_mit_0.4_spec_window_1.0_obj1024.png \
        throughput_mit_0.1_spec_window_0.25_obj1024.png \
        throughput_mit_0.1_spec_window_0.5_obj1024.png \
        throughput_mit_0.1_spec_window_1.0_obj1024.png \
        -tile 3x2 -geometry +2+2 fig7.png
    cd ../..
    echo "PCS plots generated successfully!"
fi
 
echo "Done!"