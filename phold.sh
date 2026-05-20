#!/usr/bin/env bash

# SPDX-FileCopyrightText: 2026 Andrea Mazzucchi <andrea.mazzucchi@tutamail.com>
# SPDX-FileCopyrightText: 2026 Francesco Quaglia <francesco.quaglia@uniroma2.it>
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -euo pipefail

# Configuration
TARGETS=(phold_grid_ckpt phold_chunk_ckpt phold_full_ckpt phold_mmap_mv) # phold_grid_ckpt_save

SPEC_WINDOW=(0.25 0.5 1.0)

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

SIM=./bin/PARSIR-simulator

RUN_ID=0

# Output files
CSV_FILE="phold.csv"
LOG_FILE="benchmark.log"
PROGRESS_FILE=".benchmark_progress"

# Detect if running under nohup (no TTY)
if [ -t 1 ]; then
    # TTY available - use colors
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    NC='\033[0m'
else
    # No TTY - disable colors
    RED=''
    GREEN=''
    YELLOW=''
    NC=''
fi

log() {
    echo "[$(date +%Y-%m-%d\ %H:%M:%S)] $*" | tee -a "$LOG_FILE"
}

error() {
    echo "[ERROR $(date +%Y-%m-%d\ %H:%M:%S)] $*" | tee -a "$LOG_FILE" >&2
}

warn() {
    echo "[WARN $(date +%Y-%m-%d\ %H:%M:%S)] $*" | tee -a "$LOG_FILE"
}

# Validate prerequisites
validate_setup() {
    log "Validating setup..."
    
    if [[ ! -d "build" ]]; then
        error "Build directory not found"
        exit 1
    fi
    
    if ! command -v make &> /dev/null; then
        error "make not found"
        exit 1
    fi
    
    log "Setup validation complete"
}

parse_output() {
    awk '
    /^SPEC_WINDOWS:/ {sw=$2}
    /^ROLLBACKS:/ {rb=$2}
    /^TOTAL_EVENTS:/ {te=$2}
    /^COMMITTED_EVENTS:/ {ce=$2}
    /^FILTERED_EVENTS:/ {fe=$2}
    END {
        if (sw=="" || rb=="" || te=="" || ce=="" || fe=="") {
            print "ERROR,ERROR,ERROR,ERROR,ERROR"
        } else {
            printf "%s,%s,%s,%s,%s\n",sw,rb,te,ce,fe
        }
    }'
}

format_params() {
    local out=()
    for kv in "$@"; do
        out+=("${kv#*=}")
    done
    IFS=,; echo "${out[*]}"
}

# Calculate total number of experiments
calculate_total_experiments() {
    echo $(( ${#OBJECTS[@]} * ${#M[@]} * ${#P_SHIFT[@]} * ${#SPEC_WINDOW[@]} * ${#THREADS[@]} * ${#TARGETS[@]} * RUN ))
}

# Load progress if resuming
load_progress() {
    if [[ -f "$PROGRESS_FILE" ]]; then
        source "$PROGRESS_FILE"
        warn "Resuming from experiment $COMPLETED_EXPERIMENTS"
    else
        COMPLETED_EXPERIMENTS=0
    fi
}

# Save progress with sync to ensure data is written
save_progress() {
    echo "COMPLETED_EXPERIMENTS=$COMPLETED_EXPERIMENTS" > "$PROGRESS_FILE"
    sync "$PROGRESS_FILE" 2>/dev/null || true
}

run_series() {
    local target=$1
    local csv=$2
    shift 2
    local args=("$@")
    local TARGETS=${target#*_}
    
    log "Compiling $target ${args[*]}"
    
    if ! make -C build "$target" BENCHMARK=1 "${args[@]}" WARMUP=$WARMUP DURATION=$DURATION >> "$LOG_FILE" 2>&1; then
        error "Compilation failed for $target ${args[*]}"
        return 1
    fi
    
    for ((i=0; i<RUN; i++)); do
        RUN_ID=$((RUN_ID + 1))
        # Skip already completed runs
        if ((RUN_ID <= COMPLETED_EXPERIMENTS )); then
            log "[SKIP] Run already completed ($RUN_ID/$TOTAL_EXPERIMENTS)"
            continue
        fi
        


        COMPLETED_EXPERIMENTS=$((COMPLETED_EXPERIMENTS + 1))
        local pct=$((COMPLETED_EXPERIMENTS * 100 / TOTAL_EXPERIMENTS))
        
        log "[$COMPLETED_EXPERIMENTS/$TOTAL_EXPERIMENTS - $pct%] Run $((i+1))/$RUN: $target ${args[*]}"
        
        if ! output=$($SIM 2>&1); then
            error "Simulation failed for $target ${args[*]}"
            echo "$TARGETS,$(format_params "${args[@]}"),ERROR,ERROR,ERROR,ERROR,ERROR" >> "$csv"
            continue
        fi
        
        parsed=$(printf "%s\n" "$output" | parse_output)
        
        if [[ "$parsed" == *"ERROR"* ]]; then
            warn "Failed to parse output for $target ${args[*]}"
        fi
        
        params=$(format_params "${args[@]}")
        echo "$TARGETS,$params,$parsed" >> "$csv"
        
        # Flush CSV to disk
        sync "$csv" 2>/dev/null || true
        
        save_progress
    done
}


# Main execution
main() {
    log "=== PARSIR Benchmark Suite ==="
    log "System: $(uname -a)"
    log "CPU cores: $TOTAL_THREADS"
    log "Thread configs: ${THREADS[*]}"
    log "PID: $$"
    log "Output CSV: $CSV_FILE"
    log "Log file: $LOG_FILE"
    
    validate_setup
    
    TOTAL_EXPERIMENTS=$(calculate_total_experiments)
    log "Total experiments to run: $TOTAL_EXPERIMENTS"
    
    load_progress
    
    # Initialize CSV if starting fresh
    if [[ $COMPLETED_EXPERIMENTS -eq 0 ]]; then
        echo "TARGETS,THREADS,SPEC_WINDOW,OBJECTS,M,P_SHIFT,SPEC_WINDOWS,ROLLBACKS,TOTAL_EVENTS,COMMITTED_EVENTS,FILTERED_EVENTS" > "$CSV_FILE"
        log "Created output file: $CSV_FILE"
    fi
    
    local start_time=$(date +%s)

    for o in "${OBJECTS[@]}"; do
    for m in "${M[@]}"; do
    for p in "${P_SHIFT[@]}"; do
    for l in "${SPEC_WINDOW[@]}"; do
    for t in "${THREADS[@]}"; do
    for target in "${TARGETS[@]}"; do
        run_series "$target" "$CSV_FILE" THREADS=$t LOOKAHEAD=$l OBJECTS=$o M=$m P_SHIFT=$p
    done
    done
    done
    done
    done
    done

    local end_time=$(date +%s)
    local duration=$((end_time - start_time))
    
    log "=== Benchmark Complete ==="
    log "Total time: $((duration / 3600))h $((duration % 3600 / 60))m $((duration % 60))s"
    log "Results saved to: $CSV_FILE"
    log "Log saved to: $LOG_FILE"
    
    rm -f "$PROGRESS_FILE"
}

# Cleanup function
cleanup() {
    local exit_code=$?
    if [[ $exit_code -ne 0 ]]; then
        error "Benchmark interrupted at experiment $COMPLETED_EXPERIMENTS/$TOTAL_EXPERIMENTS"
        save_progress
        log "Progress saved. Resume by running the script again."
    fi
    exit $exit_code
}

# Trap errors and cleanup
trap cleanup INT TERM EXIT

main