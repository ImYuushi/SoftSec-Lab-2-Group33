#!/usr/bin/env bash
# Builds three binaries for Q8 exec-speed comparison.
# Configurations:
#   (1) No sanitizer  + fork mode      -> fuzz_bins/fuzz_nosan_fork
#   (2) ASan          + fork mode      -> fuzz_bins/fuzz_asan_fork
#   (3) ASan          + persistent mode -> fuzz_bins/fuzz_asan_persistent
#
# Usage: ./build_q8.sh [source.c] [persistent_source.c]
# Defaults: harness.c and harness_persistent.c

set -euo pipefail

SRC="${1:-harness.c}"
SRC_PERSISTENT="${2:-harness_persistent.c}"
OUT_DIR="fuzz_bins"

cd "$(dirname "$0")"
mkdir -p "$OUT_DIR"

echo "Fork-mode source      : $SRC"
echo "Persistent-mode source: $SRC_PERSISTENT"
echo "Output dir            : $OUT_DIR/"
echo "------------------------------"

# ── (1) No sanitizer + fork mode ─────────────────────────────────────────────
BIN_NOSAN="$OUT_DIR/fuzz_nosan_fork"
echo -n "Building $BIN_NOSAN  (no sanitizer, fork mode) ... "
afl-clang-fast "$SRC"                       \
    -DFUZZ_READ_IMAGE                       \
    -I/opt/libpng-afl/include               \
    -L/opt/libpng-afl/lib                   \
    -lpng12 -lz -lm                         \
    -g -O1                                  \
    -Wl,-rpath,/opt/libpng-afl/lib          \
    -o "$BIN_NOSAN"
echo "OK"

# ── (2) ASan + fork mode ──────────────────────────────────────────────────────
BIN_ASAN_FORK="$OUT_DIR/fuzz_asan_fork"
echo -n "Building $BIN_ASAN_FORK  (ASan, fork mode) ... "
afl-clang-fast "$SRC"                       \
    -DFUZZ_READ_IMAGE                       \
    -I/opt/libpng-afl-asan/include          \
    -L/opt/libpng-afl-asan/lib              \
    -lpng12 -lz -lm                         \
    -fsanitize=address                      \
    -g -O1                                  \
    -Wl,-rpath,/opt/libpng-afl-asan/lib     \
    -o "$BIN_ASAN_FORK"
echo "OK"

# ── (3) ASan + persistent mode ───────────────────────────────────────────────
BIN_ASAN_PERSISTENT="$OUT_DIR/fuzz_asan_persistent"
echo -n "Building $BIN_ASAN_PERSISTENT  (ASan, persistent mode) ... "
afl-clang-fast "$SRC_PERSISTENT"            \
    -DFUZZ_READ_IMAGE                       \
    -I/opt/libpng-afl-asan/include          \
    -L/opt/libpng-afl-asan/lib              \
    -lpng12 -lz -lm                         \
    -fsanitize=address                      \
    -g -O1                                  \
    -Wl,-rpath,/opt/libpng-afl-asan/lib     \
    -o "$BIN_ASAN_PERSISTENT"
echo "OK"

echo "------------------------------"
echo "Done. Binaries in $OUT_DIR/:"
ls -lh "$OUT_DIR"/fuzz_nosan_fork "$OUT_DIR"/fuzz_asan_fork "$OUT_DIR"/fuzz_asan_persistent