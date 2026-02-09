#!/bin/bash

declare -a THREADS=(2 4 8)
declare -a LOOKAHEAD=(0.25 0.5 1.0)
declare -a PHOLD_OBJECTS=(1024 4096)
declare -a PCS_OBJECTS=(1024 4096)
declare -a TA=(0.4 0.1)

PERIOD=5
SAMPLES=5

# --- Error Checking ---
if ! command -v gnuplot &> /dev/null
then
    echo "Error: Gnuplot could not be found."
    echo "Please install gnuplot to run this script."
    exit 1
fi

if [ ! -f "plot.gp" ]; then
    echo "Error: Gnuplot template 'plot.gp' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

mkdir -p plots/phold
mkdir -p plots/pcs

# Phold Benchmark

rm phold_bench.csv
echo "CKPT_TYPE,THREADS,LOOKAHEAD,OBJECTS,THROHGHPUT_MEAN,THROHGHPUT_CI,EPOCHS,ROLLBACKS,FILTERED" > phold_bench.csv

for t in ${THREADS[@]};
do
    for l in ${LOOKAHEAD[@]};
    do
        for o in ${PHOLD_OBJECTS[@]};
        do
            make -C build phold_grid_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o PERIOD=$PERIOD SAMPLES=$SAMPLES
            output=$(./bin/PARSIR-simulator) 
            throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
            throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
            epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
            rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
            filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
            echo "grid_ckpt,$t,$l,$o,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> phold_bench.csv
            make -C build phold_chunk_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o PERIOD=$PERIOD SAMPLES=$SAMPLES
            output=$(./bin/PARSIR-simulator) 
            throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
            throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
            epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
            rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
            filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
            echo "chunk_ckpt,$t,$l,$o,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> phold_bench.csv
            make -C build phold_chunk_full_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o PERIOD=$PERIOD SAMPLES=$SAMPLES
            output=$(./bin/PARSIR-simulator) 
            throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
            throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
            epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
            rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
            filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
            echo "chunk_full_ckpt,$t,$l,$o,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> phold_bench.csv
        done
    done
done

# PCS Benchmark

rm pcs_output.csv
echo "CKPT_TYPE,THREADS,LOOKAHEAD,OBJECTS,TA,THROHGHPUT_MEAN,THROHGHPUT_CI,EPOCHS,ROLLBACKS,FILTERED" > pcs_bench.csv

for t in ${THREADS[@]};
do
    for l in ${LOOKAHEAD[@]};
    do
        for o in ${PCS_OBJECTS[@]};
        do
            for ta in ${TA[@]};
            do
                make -C build pcs_grid_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o TA=$ta PERIOD=$PERIOD SAMPLES=$SAMPLES
                output=$(./bin/PARSIR-simulator) 
                throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
                throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
                epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
                rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
                filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
                echo "grid_ckpt,$t,$l,$o,$ta,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> pcs_bench.csv
                make -C build pcs_chunk_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o TA=$ta PERIOD=$PERIOD SAMPLES=$SAMPLES
                output=$(./bin/PARSIR-simulator) 
                throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
                throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
                epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
                rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
                filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
                echo "chunk_ckpt,$t,$l,$o,$ta,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> pcs_bench.csv
                make -C build pcs_chunk_full_ckpt BENCHMARK=1 THREADS=$t LOOKAHEAD=$l OBJECTS=$o TA=$ta PERIOD=$PERIOD SAMPLES=$SAMPLES
                output=$(./bin/PARSIR-simulator) 
                throughput_mean=$(awk '/^THROHGHPUT_MEAN:/ {print $2}' <<< "$output")
                throughput_ci=$(awk '/^THROHGHPUT_CI:/ {print $2}' <<< "$output")
                epochs=$(awk '/^EPOCHS:/ {print $2}' <<< "$output")
                rollbacks=$(awk '/^ROLLBACKS:/ {print $2}' <<< "$output")
                filtered=$(awk '/^FILTERED_EVENTS:/ {print $2}' <<< "$output")
                echo "chunk_full_ckpt,$t,$l,$o,$ta,$throughput_mean,$throughput_ci,$epochs,$rollbacks,$filtered" >> pcs_bench.csv
            done
        done
    done
done

gnuplot plot.gp
