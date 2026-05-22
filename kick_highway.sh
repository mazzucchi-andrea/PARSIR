#!/bin/bash

set -euo pipefail

# Detect total CPU threads and compute 50%
TOTAL_THREADS=$(nproc)
T50=$((TOTAL_THREADS / 2))
# Ensure minimums of 1
[[ $T50 -lt 1 ]] && T50=1
THREADS=($T50)

RUN=1
WARMUP=10
DURATION=60

LOOKAHEAD=(0.5)
OBJECTS=(1024)

SIM=./bin/PARSIR-simulator

# --- Helper ---
die() {
	echo "Error: $*" >&2
	exit 1
}

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
	IFS=,
	echo "${out[*]}"
}

run_series() {
	local target=$1
	local csv=$2
	shift 2
	local args=("$@")

	echo "Compiling $target ${args[*]}"
	make -C build "$target" "${args[@]}" WARMUP=$WARMUP DURATION=$DURATION BENCHMARK=1 DEBUG=1 >/dev/null ||
		die "make failed for target '$target' with args: ${args[*]} WARMUP=$WARMUP DURATION=$DURATION BENCHMARK=1 DEBUG=1"

	local ckpt_type=${target#*_}

	local filtered_args=()
	if [[ "$target" == "highway_mmap_mv" ]]; then
		for arg in "${args[@]}"; do
			if [[ "$arg" == MMAP_MV_PAGE_SIZE=* ]]; then
				local page_size="${arg#*=}"
				ckpt_type="${ckpt_type}_${page_size}"
				# Don't add to filtered_args - we're excluding it from CSV params
			else
				filtered_args+=("$arg")
			fi
		done
	else
		filtered_args=("${args[@]}")
	fi

	for ((i = 0; i < RUN; i++)); do
		echo "Run $((i + 1))/$RUN : $target ${args[*]} WARMUP=$WARMUP DURATION=$DURATION BENCHMARK=1 DEBUG=1"

		output=$($SIM) ||
			die "Simulator failed on run $((i + 1))/$RUN for target '$target' with args: ${args[*]}"

		parsed=$(printf "%s\n" "$output" | parse_output) ||
			die "parse_output failed on run $((i + 1))/$RUN for target '$target'"

		[[ -n "$parsed" ]] ||
			die "parse_output returned empty result on run $((i + 1))/$RUN for target '$target' — check simulator output format"

		params=$(format_params "${filtered_args[@]}") ||
			die "format_params failed for args: ${args[*]}"

		echo "$ckpt_type,$params,$parsed" >>"$csv" ||
			die "Failed to write to CSV '$csv'"
	done
}

# --- CSV header ---
echo "CKPT_TYPE,THREADS,SPEC_WINDOW,OBJECTS,SPEC_WINDOWS,ROLLBACKS,TOTAL_EVENTS,COMMITTED_EVENTS,FILTERED_EVENTS" >highway.csv ||
	die "Failed to create highway.csv"

# --- Simulation runs ---
for t in "${THREADS[@]}"; do
	for l in "${LOOKAHEAD[@]}"; do
		for o in "${OBJECTS[@]}"; do
			run_series highway_grid_ckpt highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o
			run_series highway_grid_ckpt_save highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o
			run_series highway_chunk_ckpt highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o
			run_series highway_full_ckpt highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o
			run_series highway_mmap_mv highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o MMAP_MV_PAGE_SIZE=256
			run_series highway_mmap_mv highway.csv THREADS=$t LOOKAHEAD=$l OBJECTS=$o MMAP_MV_PAGE_SIZE=4096
		done
	done
done

cd build
make clean
