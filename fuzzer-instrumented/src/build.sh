#!/usr/bin/env bash
# Builds one binary per fuzzing flag.
# Runs inside the AFL++ fuzzing Docker container.
# Usage: ./build_fuzz_targets.sh [source.c]

set -euo pipefail

SRC="${1:-harness.c}"
OUT_DIR="fuzz_bins"

FUZZ_FLAGS=(
    FUZZ_READ_IMAGE
    FUZZ_PROGRESSIVE
    FUZZ_TRANSFORMS
)

mkdir -p "$OUT_DIR"

echo "Source : $SRC"
echo "Output : $OUT_DIR/"
echo "------------------------------"

for FLAG in "${FUZZ_FLAGS[@]}"; do
    BIN="$OUT_DIR/fuzz_${FLAG,,}"
    echo -n "Building $BIN  (-D$FLAG) ... "

    afl-clang-fast "$SRC"                           \
        -D"$FLAG"                                   \
        -I/opt/libpng-instrumented/include          \
        -L/opt/libpng-instrumented/lib              \
        -lpng12 -lz -lm                             \
        -fsanitize=address                          \
        -g -O1                                      \
        -Wl,-rpath,/opt/libpng-instrumented/lib     \
        -o "$BIN"

    echo "OK"
done

echo "------------------------------"
echo "Done. Binaries in $OUT_DIR/:"
ls -lh "$OUT_DIR"/