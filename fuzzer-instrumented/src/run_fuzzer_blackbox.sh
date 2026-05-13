#!/usr/bin/env bash
# Runs AFL++ in QEMU blackbox mode
# Usage: ./run_fuzzer_qemu.sh [method] [duration]

set -euo pipefail

DEFAULT_METHOD="read_image"
METHOD="${1:-$DEFAULT_METHOD}"
DURATION="${2:-100}"

SEEDS_DIR="/fuzzing/seeds"

# Separate findings directory for QEMU campaign
OUT_DIR="/fuzzing/instrumented/findings-qemu/$METHOD"

# Separate afl-plot output
PLOT_DIR="/fuzzing/instrumented/plot-output-qemu/$METHOD"

# Blackbox binary
BINARY="/fuzzing/instrumented/src/fuzz_bins/blackbox_fuzz_fuzz_$METHOD"

DICTS="/fuzzing/dict/png.dict"

mkdir -p "$OUT_DIR"
mkdir -p "$PLOT_DIR"

echo "[+] Starting QEMU-mode fuzzing"
echo "[+] Method    : $METHOD"
echo "[+] Duration  : $DURATION seconds"
echo "[+] Binary    : $BINARY"
echo "[+] Findings  : $OUT_DIR"
echo "[+] Plot dir  : $PLOT_DIR"

# QEMU mode enabled with -Q
afl-fuzz \
    -Q \
    -V "$DURATION" \
    -i "$SEEDS_DIR" \
    -o "$OUT_DIR" \
    -x "$DICTS" \
    -- "$BINARY" @@

# Generate afl-plot graphs
afl-plot "$OUT_DIR/default" "$PLOT_DIR"

cleanup() {
    echo "[+] fixing ownership..."

    chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} \
        /fuzzing/instrumented/findings-qemu || true

    chown -R ${HOST_UID:-1000}:${HOST_GID:-1000} \
        /fuzzing/instrumented/plot-output-qemu || true
}

trap cleanup EXIT