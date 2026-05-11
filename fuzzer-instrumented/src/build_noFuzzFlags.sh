#!/usr/bin/env bash
# Builds one binary per fuzzing flag.
# Runs inside the AFL++ fuzzing Docker container.
# Usage: ./build_fuzz_targets.sh [source.c]

set -euo pipefail

SRC="${1:-fuzz_png.c}"
OUT_DIR="Binaries"


mkdir -p "$OUT_DIR"


BIN="$OUT_DIR/fuzz_noFlag"
echo -n "Building $BIN  ... "

afl-clang-fast "$SRC"                           \
    -I/opt/libpng-instrumented/include          \
    -L/opt/libpng-instrumented/lib              \
    -lpng12 -lz -lm                             \
    -fsanitize=address                          \
    -g -O1                                      \
    -Wl,-rpath,/opt/libpng-instrumented/lib     \
    -o "$BIN"

echo "OK"


echo "------------------------------"
echo "Done. Binaries in $OUT_DIR/:"
ls -lh "$OUT_DIR"/