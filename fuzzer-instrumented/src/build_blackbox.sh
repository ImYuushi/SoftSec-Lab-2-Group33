#!/usr/bin/env bash
set -euo pipefail

SRC="${1:-harness.c}"
OUT_DIR="fuzz_bins"

FUZZ_FLAGS=(
    FUZZ_READ_IMAGE
    FUZZ_PROGRESSIVE
    FUZZ_TRANSFORMS
)

cd "$(dirname "$0")"

mkdir -p "$OUT_DIR"

echo "Source : $SRC"
echo "Output : $OUT_DIR/"
echo "------------------------------"

for FLAG in "${FUZZ_FLAGS[@]}"; do
    BIN="$OUT_DIR/blackbox_fuzz_${FLAG,,}"

    echo -n "Building $BIN (-D$FLAG) ... "

    # Regular compiler, NO AFL instrumentation
    gcc "$SRC"                                   \
        -D"$FLAG"                                  \
        -I/opt/libpng-vanilla/include                      \
        -L/opt/libpng-vanilla/lib                          \
        -lpng12 -lz -lm                            \
        -O2                                     \
        -Wl,-rpath,/opt/libpng-vanilla/lib                 \
        -o "$BIN"

    echo "OK"
done

echo "------------------------------"
echo "Done. Binaries in $OUT_DIR/:"
ls -lh "$OUT_DIR"/