#!/usr/bin/env bash
#Runs the fuzzer for a given method for a given duration
# Usage: ./run_fuzzer.sh [method] [duration]

set -euo pipefail

DEFAULT_METHOD="read_image"
METHOD="${1:-$DEFAULT_METHOD}"
DURATION="${2:-100}"
SEEDS_DIR="/fuzzing/seeds"
OUT_DIR="/fuzzing/instrumented/findings/$METHOD"
BINARY="./fuzz_bins/fuzz_fuzz_$METHOD"

afl-fuzz -V "$DURATION" -i "$SEEDS_DIR" -o "$OUT_DIR" -- "$BINARY" @@ 
