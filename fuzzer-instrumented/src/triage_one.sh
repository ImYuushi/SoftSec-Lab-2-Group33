#!/usr/bin/env bash
#Tries to triage one crash
# Usage: ./triage_one.sh [method] [crash_file]

set -euo pipefail

DEFAULT_METHOD="read_image"
METHOD="${1:-$DEFAULT_METHOD}"
CRASH_FILE_PATH="${2}"
CRASH_FILE_NAME=$(basename "$CRASH_FILE_PATH")
OUTPUT_DIR="/fuzzing/instrumented/findings/$METHOD/triage"
mkdir -p "$OUTPUT_DIR"



./fuzz_bins/fuzz_fuzz_$METHOD "$CRASH_FILE_PATH" || true
afl-tmin -i "$CRASH_FILE_PATH" -o "$OUTPUT_DIR/$CRASH_FILE_NAME.minimized.png" -- ./fuzz_bins/fuzz_fuzz_$METHOD @@ || true
ASAN_OPTIONS=symbolize=1 ./fuzz_bins/fuzz_fuzz_$METHOD "$OUTPUT_DIR/$CRASH_FILE_NAME.minimized.png"