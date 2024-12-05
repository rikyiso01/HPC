#!/usr/bin/env bash


source /opt/intel/oneapi/setvars.sh

set -euo pipefail

COMPILED_FILE=./mandelbrot
SOURCE_FILE=./mandelbrot.cpp
PROJECT_DIR=./advisor
OUT=./data
PROGRAM_OUTPUT=/dev/null

function compile(){
    iterations="$1"
    shift
    echo "Compiling with" "ITERATION=$iterations" "$@"
    icpx "-DITERATIONS=$iterations" -qopenmp "$@" -o $COMPILED_FILE $SOURCE_FILE
}

function survey(){
    echo "Creating survey"
    advixe-cl -collect survey --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
}
function roofline(){
    echo "Creating roofline"
    advixe-cl -collect roofline --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
}
function snapshot(){
    echo "Creating snapshot"
    advixe-cl -snapshot --project-dir $PROJECT_DIR "$OUT/$1"
}

function measure(){
    echo "Executing"
    time $COMPILED_FILE $PROGRAM_OUTPUT
}

function title(){
    echo "-----------"
    echo "$1"
    echo
}



# rm -rf $OUT
rm -rf $PROJECT_DIR
mkdir -p $OUT

advixe-cl -create-project --project-dir $PROJECT_DIR test

compile ROOFLINE -g
roofline
snapshot "hotspot"

compile $ROOFLINE -qopt-report=3 -xHost -O3
mv ./mandelbrot.optrpt $OUT


title "Best sequential"

compile $BEST_SEQUENTIAL -xHost -O3
measure best-sequential-normal

compile -xHost -O3 -ffast-math
measure best-sequential-fast-math

compile $BEST_SEQUENTIAL -xHost -O3 -ffast-math -ipo
measure best-sequential-fast-math-ipo

compile $BEST_SEQUENTIAL -xHost -O3 -ipo
measure best-sequential-ipo
