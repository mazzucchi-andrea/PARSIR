#!/bin/bash

declare -a LOOKAHEAD=(0.25 0.5 1.0)
declare -a PHOLD_OBJECTS=(1024)
declare -a M=(1 100) 
declare -a PCS_OBJECTS=(1024)
declare -a MIT=(0.4 0.1)
declare -a THREADS=(10 20 40)

RUN=5

if ! command -v gnuplot &> /dev/null
then
    echo "Error: Gnuplot could not be found."
    echo "Please install gnuplot to run this script."
    exit 1
fi

rm -r plots

if [ ! -f "get_phold_data.c" ]; then
    echo "Error: C script 'get_phold_data.c' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

if [ ! -f "plot_phold.gp" ]; then
    echo "Error: Gnuplot template 'plot_phold.gp' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

if [ ! -f "phold_bench.csv" ]; then
    echo "Error: phold benchmark data 'phold_bench.csv' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

mkdir -p plots/phold

for l in ${LOOKAHEAD[@]};
do
    for o in ${PHOLD_OBJECTS[@]};
    do
        for m in ${M[@]};
        do
            echo "Generate plots for Speculation Window = $l, Objects =$o, M = $m"
            for t in ${THREADS[@]};
            do
                gcc -O3 get_phold_data.c -o get_data -lm -DTHREADS=$t -DSPEC=$l -DOBJECTS=$o -DM=$m
                ./get_data
            done
            gnuplot -c plot_phold.gp $m $l
            rm phold_plot_data.csv;
        done
    done
done

if [ ! -f "get_pcs_data.c" ]; then
    echo "Error: C script 'get_pcs_data.c' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

if [ ! -f "plot_pcs.gp" ]; then
    echo "Error: Gnuplot template 'plot_pcs.gp' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

if [ ! -f "pcs_bench.csv" ]; then
    echo "Error: pcs benchmark data 'pcs_bench.csv' not found!"
    echo "Please make sure it's in the same directory as this script."
    exit 1
fi

mkdir -p plots/pcs

for l in ${LOOKAHEAD[@]};
do
    for o in ${PCS_OBJECTS[@]};
    do
        for ta in ${MIT[@]};
        do
            echo "Generate plots for Speculation Window = $l, Objects =$o, MIT = $ta"
            for t in ${THREADS[@]};
            do
                gcc -O3 get_pcs_data.c -o get_data -lm -DTHREADS=$t -DSPEC=$l -DOBJECTS=$o -DMIT=$ta -DRUN=$RUN
                ./get_data
            done
            gnuplot -c plot_pcs.gp $ta $l
            rm pcs_plot_data.csv;
        done
    done
done

rm get_data
