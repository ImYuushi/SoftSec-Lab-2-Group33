#!/usr/bin/env bash
#Tries to triage one crash
# Usage: ./triage.sh [method]

set -euo pipefail

DEFAULT_METHOD="read_image"
METHOD="${1:-$DEFAULT_METHOD}"
CRASH_DIR="../findings/$METHOD/default/crashes"

for CRASH_FILE in "$CRASH_DIR"/id:*; do
    bash ./triage_one.sh "$METHOD" "$CRASH_FILE" || true
done