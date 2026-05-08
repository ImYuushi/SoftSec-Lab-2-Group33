#!/bin/bash


for bin in fuzz_bins/*; do
    echo -n "Testing $bin ... "
    $bin test.png && echo "OK" || echo "FAILED (exit $?)"
done

echo "this is not a png" > bad.png
for bin in fuzz_bins/*; do
    echo -n "Testing $bin with bad input ... "
    $bin bad.png && echo "OK (handled gracefully)" || echo "CRASHED (exit $?)"
done