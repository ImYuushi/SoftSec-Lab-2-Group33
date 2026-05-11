#!/bin/bash


for bin in fuzz_bins/*; do
    for img in /fuzzing/seeds/*; do
        echo -n "Testing $bin ... "
        $bin img && echo "OK" || echo "FAILED (exit $?)"
    done
done

echo "this is not a png" > bad.png
for bin in fuzz_bins/*; do
    echo -n "Testing $bin with bad input ... "
    $bin bad.png && echo "OK (handled gracefully)" || echo "CRASHED (exit $?)"
done