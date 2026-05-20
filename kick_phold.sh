#!/bin/bash

set -euo pipefail

# Detect total CPU threads and compute 50%
TOTAL_THREADS=$(nproc)
T50=$(( TOTAL_THREADS / 2 ))
# Ensure minimums of 1
[[ $T50 -lt 1 ]] && T50=1
THREADS=($T50)

RUN=1
WARMUP=10
DURATION=60

LOOKAHEAD=(0.5)
OBJECTS=(1024)
M=(1)
P_SHIFT=(6)

SIM=./bin/PARSIR-simulator

# --- Helper ---
die() { echo "Error: $*" >&2; exit 1; }

parse_output() {
    awk '
    /^SPEC_WINDOWS:/      {sw=$2}
    /^ROLLBACKS:/         {rb=$2}
    /^TOTAL_EVENTS:/      {te=$2}
    /^COMMITTED_EVENTS:/  {ce=$2}
    /^FILTERED_EVENTS:/   {fe=$2}
    END {printf "%s,%s,%s,%s,%s\n",sw,rb,te,ce,fe}'
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
    make -C build "$target" BENCHMARK=1 "${args[@]}" WARMUP=$WARMUP DURATION=$DURATION DEBUG=1 >/dev/null \
        || die "make failed for target '$target' with args: ${args[*]}"

    local ckpt_type=${target#*_}

    for ((i=0; i<RUN; i++)); do
        echo "Run $((i+1))/$RUN : $target ${args[*]}"

        output=$($SIM) \
            || die "Simulator failed on run $((i+1))/$RUN for target '$target' with args: ${args[*]}"

        parsed=$(printf "%s\n" "$output" | parse_output) \
            || die "parse_output failed on run $((i+1))/$RUN for target '$target'"

        [[ -n "$parsed" ]] \
            || die "parse_output returned empty result on run $((i+1))/$RUN for target '$target' — check simulator output format"

        params=$(format_params "${args[@]}") \
            || die "format_params failed for args: ${args[*]}"

        echo "$ckpt_type,$params,$parsed" >> "$csv" \
            || die "Failed to write to CSV '$csv'"
    done
}

# --- CSV header ---
echo "CKPT_TYPE,THREADS,SPEC_WINDOW,OBJECTS,M,P_SHIFT,SPEC_WINDOWS,ROLLBACKS,TOTAL_EVENTS,COMMITTED_EVENTS,FILTERED_EVENTS" > phold.csv \
    || die "Failed to create phold.csv"

# --- Simulation runs ---
for t in "${THREADS[@]}"; do
for l in "${LOOKAHEAD[@]}"; do
for o in "${OBJECTS[@]}"; do
for m in "${M[@]}"; do
for p in "${P_SHIFT[@]}"; do
    run_series phold_grid_ckpt phold.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
    run_series phold_grid_ckpt_save phold.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
    run_series phold_chunk_ckpt phold.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
    run_series phold_full_ckpt phold.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
    run_series phold_mmap_mv phold.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
done
done
done
done
done

cd build
make clean