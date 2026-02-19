#!/usr/bin/env bash
set -euo pipefail

LOOKAHEAD=(0.25 0.5 1.0)
THREADS=(10 20 40)
RUN=5
WARMUP=10
DURATION=60

PHOLD_OBJECTS=(1024)
M=(1 100)

PCS_OBJECTS=(1024)
MIT=(0.4 0.1)

SIM=./bin/PARSIR-simulator

parse_output() {
awk '
/^MEAN_TOT_THROUGHPUT:/ {tot=$2}
/^MEAN_COM_THROUGHPUT:/ {com=$2}
/^EPOCHS:/ {ep=$2}
/^ROLLBACKS:/ {rb=$2}
/^TOTAL_EVENTS:/ {te=$2}
/^COMMITTED_EVENTS:/ {ce=$2}
/^FILTERED_EVENTS:/ {fe=$2}
END {printf "%s,%s,%s,%s,%s,%s,%s\n",tot,com,ep,rb,te,ce,fe}'
}

format_params() {
    local out=()
    for kv in "$@"; do
        out+=("${kv#*=}")
    done
    IFS=,; echo "${out[*]}"
}

run_series() {
    local target=$1
    local csv=$2
    shift 2
    local args=("$@")

    echo "Compiling $target ${args[*]}"
    make -C build "$target" BENCHMARK=1 "${args[@]}" WARMUP=$WARMUP DURATION=$DURATION >/dev/null

    ckpt_type=${target#*_}

    for ((i=0;i<RUN;i++)); do
        echo "Run $((i+1))/$RUN : $target ${args[*]}"
        output=$($SIM)
        parsed=$(printf "%s\n" "$output" | parse_output)
        params=$(format_params "${args[@]}")
        echo "$ckpt_type,$params,$parsed" >> "$csv"
    done
}

# ---------------- PHOLD ----------------
echo "CKPT_TYPE,THREADS,SPEC_WINDOW,OBJECTS,M,MEAN_TOT_THROUGHPUT,MEAN_COM_THROUGHPUT,EPOCHS,ROLLBACKS,TOTAL_EVENTS,COMMITTED_EVENTS,FILTERED_EVENTS" > phold_bench.csv

for t in "${THREADS[@]}"; do
for l in "${LOOKAHEAD[@]}"; do
for o in "${PHOLD_OBJECTS[@]}"; do
for m in "${M[@]}"; do

    run_series phold_grid_ckpt phold_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m
    run_series phold_chunk_ckpt phold_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m
    run_series phold_full_ckpt phold_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m

done
done
done
done

# ---------------- PCS ----------------
echo "CKPT_TYPE,THREADS,SPEC_WINDOW,OBJECTS,MIT,MEAN_TOT_THROUGHPUT,MEAN_COM_THROUGHPUT,EPOCHS,ROLLBACKS,TOTAL_EVENTS,COMMITTED_EVENTS,FILTERED_EVENTS" > pcs_bench.csv

for t in "${THREADS[@]}"; do
for l in "${LOOKAHEAD[@]}"; do
for o in "${PCS_OBJECTS[@]}"; do
for ta in "${MIT[@]}"; do

    run_series pcs_grid_ckpt pcs_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o MIT=$ta
    run_series pcs_chunk_ckpt pcs_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o MIT=$ta
    run_series pcs_full_ckpt pcs_bench.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o MIT=$ta

done
done
done
done

bash generate_plot.sh
