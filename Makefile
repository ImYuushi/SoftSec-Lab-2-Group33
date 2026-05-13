CC_AFL=afl-clang-fast
CC_GCC=gcc

AFL_FLAGS=-fsanitize=address -g -O1
PNG_INST_INC=/opt/libpng-instrumented/include
PNG_INST_LIB=/opt/libpng-instrumented/lib

PNG_VAN_INC=/opt/libpng-vanilla/include
PNG_VAN_LIB=/opt/libpng-vanilla/lib

SRC=fuzzer-instrumented/src

all: build

build:
	./instrumented/src/build.sh

fuzz:
	./instrumented/src/run_fuzzer_withFlags.sh read_image 1800
	./instrumented/src/run_fuzzer_withFlags.sh progressive 1800
	./instrumented/src/run_fuzzer_withFlags.sh transforms 1800

fuzz_progressive:
	./instrumented/src/run_fuzzer_withFlags.sh progressive 1800

clean:
	rm -rf fuzz_bins
	rm -rf findings
	rm -rf plot-output
	rm -rf afl_output