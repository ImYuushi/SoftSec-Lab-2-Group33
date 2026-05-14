#!/usr/bin/env bash
# Runs AFL++ for Q8 exec-speed comparison across three configurations:
#   (1) no sanitizer + fork mode
#   (2) ASan         + fork mode
#   (3) ASan         + persistent mode
#
# Each run uses -V (time limit in seconds) so exec/s is stable and comparable.
# Results and afl-plot output are written to separate findings directories.
#
# Usage: ./run_fuzzer_q8.sh [duration_seconds]
# Default duration: 60 seconds per configuration (enough to get stable exec/s)

set -euo pipefail

DURATION="${1:-300}"
SEEDS_DIR="/fuzzing/seeds"
DICT="/fuzzing/dict/png.dict"
BIN_DIR="/fuzzing/instrumented/src/fuzz_bins"
FINDINGS_BASE="/fuzzing/instrumented/findings/q8"
PLOT_BASE="/fuzzing/instrumented/plot-output/q8"

# Binary paths (built by build_q8.sh)
BIN_NOSAN="$BIN_DIR/fuzz_nosan_fork"
BIN_ASAN_FORK="$BIN_DIR/fuzz_asan_fork"
BIN_ASAN_PERSISTENT="$BIN_DIR/fuzz_asan_persistent"

# ── Helpers ───────────────────────────────────────────────────────────────────

run_config() {
    local name="$1"
    local binary="$2"
    local extra_flags="${3:-}"          # e.g. -t 5000 for persistent mode

    local out_dir="$FINDINGS_BASE/$name"
    local plot_dir="$PLOT_BASE/$name"
    mkdir -p "$out_dir" "$plot_dir"

    echo ""
    echo "========================================"
    echo "  Config: $name"
    echo "  Binary: $binary"
    echo "  Duration: ${DURATION}s"
    echo "========================================"

    # Wipe any previous run so afl-fuzz doesn't refuse to start
    rm -rf "${out_dir:?}/default"

    # shellcheck disable=SC2086
    afl-fuzz -V "$DURATION"         \
        -i "$SEEDS_DIR"             \
        -o "$out_dir"               \
        -x "$DICT"                  \
        $extra_flags                \
        -- "$binary" @@

    echo ""
    echo "[+] Generating plot for $name ..."
    afl-plot "$out_dir/default" "$plot_dir" || true

    echo "[+] Exec speed summary for $name:"
    grep "execs_per_sec" "$out_dir/default/fuzzer_stats" 2>/dev/null \
        || echo "    (fuzzer_stats not found)"
}

# ── Run all three configurations ──────────────────────────────────────────────

echo "Q8 — Instrumentation Depth and Performance"
echo "Running each config for ${DURATION}s ..."

# (1) No sanitizer + fork mode
run_config "nosan_fork" "$BIN_NOSAN"

# (2) ASan + fork mode
run_config "asan_fork" "$BIN_ASAN_FORK"

# (3) ASan + persistent mode
#     -t sets a generous per-execution timeout (ms); persistent loops are fast
run_config "asan_persistent" "$BIN_ASAN_PERSISTENT" "-t 5000+"

# ── Print side-by-side exec/s summary ─────────────────────────────────────────

echo ""
echo "========================================"
echo "  Q8 Exec Speed Summary"
echo "========================================"
for cfg in nosan_fork asan_fork asan_persistent; do
    stats="$FINDINGS_BASE/$cfg/default/fuzzer_stats"
    if [[ -f "$stats" ]]; then
        speed=$(grep "execs_per_sec" "$stats" | awk '{print $3}')
        printf "  %-25s : %s exec/s\n" "$cfg" "$speed"
    else
        printf "  %-25s : stats file not found\n" "$cfg"
    fi
done
echo "========================================"

# ── Fix ownership ─────────────────────────────────────────────────────────────

cleanup() {
    echo "[+] Fixing ownership ..."
    chown -R "${HOST_UID:-1000}:${HOST_GID:-1000}" \
        /fuzzing/instrumented/findings || true
    chown -R "${HOST_UID:-1000}:${HOST_GID:-1000}" \
        /fuzzing/instrumented/plot-output || true
}
trap cleanup EXIT