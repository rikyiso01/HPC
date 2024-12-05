#!/usr/bin/env bash

set -euo pipefail

source /opt/intel/oneapi/setvars.sh

COMPILED_FILE=./mandelbrot
SOURCE_FILE=./mandelbrot.cpp
PROJECT_DIR=./advisor
OUT=./data
PROGRAM_OUTPUT=/dev/null

function compile(){
    iterations="$1"
    shift
    icpx "-DITERATIONS=$iterations" "$@" -o $COMPILED_FILE $SOURCE_FILE
}

function survey(){
    advixe-cl -collect survey --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
}
function roofline(){
    advixe-cl -collect roofline --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
}
function snapshot(){
    advixe-cl -snapshot --project-dir $PROJECT_DIR "$OUT/$1"
}



rm -rf $OUT
rm -rf $PROJECT_DIR
mkdir -p $OUT

advixe-cl -create-project --project-dir $PROJECT_DIR test

compile 700 -g
roofline
snapshot "hotspot"

compile 700 -O3
