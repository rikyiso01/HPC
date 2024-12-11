#!/usr/bin/env bash

source /opt/intel/oneapi/setvars.sh

set -euo pipefail

COMPILED_FILE=./heat
SOURCE_FILE=./heat.c
PROJECT_DIR=./advisor
OUT=./data
PROGRAM_OUTPUT=/dev/null

function compile(){
    size="$1"
    shift
    steps="$1"
    shift
    echo "Compiling with" "SIZE=$size" "STEPS=$steps" "$@"
    icx "-DSIZE=$size" "-DSTEPS=$steps" "$@" -o $COMPILED_FILE $SOURCE_FILE
}

function survey(){
    echo "Creating survey"
    advixe-cl -collect survey --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
    snapshot "$1"
    measure "$1"
}
function roofline(){
    echo "Creating roofline"
    advixe-cl -collect roofline --project-dir $PROJECT_DIR $COMPILED_FILE $PROGRAM_OUTPUT
    snapshot "$1"
    measure "$1"
}
function snapshot(){
    echo "Creating snapshot"
    rm -rf "$OUT/$1.advixeexpz"
    advixe-cl -snapshot --project-dir $PROJECT_DIR "$OUT/$1"
}

function measure(){
    echo "Executing"
    time $COMPILED_FILE $PROGRAM_OUTPUT | tee "$OUT/$1"
}

function title(){
    echo "-----------"
    echo "$1"
    echo
}
