#!/usr/bin/env bash
#Runs the fuzzer for a given method for a given duration
# Usage: ./run_fuzzer.sh [method] [duration]

set -euo pipefail

DEFAULT_METHOD="read_image"
METHOD="${1:-$DEFAULT_METHOD}"
DURATION="${2:-100}"
SEEDS_DIR="/fuzzing/seeds"
OUT_DIR="/fuzzing/instrumented/findings/$METHOD"
BINARY="/fuzzing/instrumented/src/fuzz_bins/fuzz_fuzz_$METHOD"
DICTS="/fuzzing/dict/png.dict"
PLOT_DIR="/fuzzing/instrumented/plot-output/$METHOD"
mkdir -p "$OUT_DIR"
mkdir -p PLOT_DIR
afl-fuzz -V "$DURATION" -i "$SEEDS_DIR" -o "$OUT_DIR" -x "$DICTS" -- "$BINARY" @@ 
afl-plot  "$OUT_DIR/default" "$PLOT_DIR"
set -e

cleanup() {
  echo "[+] fixing ownership..."
  chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} /fuzzing/instrumented/findings || true
  chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} /fuzzing/instrumented/plot_output || true
}

trap cleanup EXIT

exec "$@"
