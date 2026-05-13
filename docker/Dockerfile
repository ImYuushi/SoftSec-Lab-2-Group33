# ============================================================
# AFL++ Fuzzing Environment — libpng 1.2.56
# Base: Ubuntu 22.04
# ============================================================
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# ── System dependencies ──────────────────────────────────────
RUN apt-get update && apt-get install -y \
    # Build essentials
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    patch \
    # libpng build dependencies
    zlib1g-dev \
    # AFL++ dependencies
    llvm-14 \
    clang-14 \
    lld-14 \
    # QEMU dependencies (for black-box fuzzing)
    qemu-user \
    qemu-system \
    meson \
    pkg-config \
    libglib2.0-dev \
    libpixman-1-dev \
    python3-dev \
    cargo \
    flex \
    bison \
    texinfo \
    libcap-dev \
    libcapstone-dev \
    libfdt-dev \
    ninja-build \
    # Address Sanitizer & debug tools
    libasan6 \
    gdb \
    valgrind \
    # Python (for afl-plot and helper scripts)
    python3 \
    python3-pip \
    # gnuplot for afl-plot graphs
    gnuplot \
    # Useful utilities
    vim \
    less \
    file \
    && rm -rf /var/lib/apt/lists/*

# Make clang-14 the default compiler
RUN update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-14 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-14 100

# ── Install AFL++ from source ────────────────────────────────
RUN git clone https://github.com/AFLplusplus/AFLplusplus /opt/AFLplusplus && \
    cd /opt/AFLplusplus && \
    make clean && \
    make distrib && \
    make install

RUN cd /opt/AFLplusplus/qemu_mode && \
    ./build_qemu_support.sh

# ── Download libpng 1.2.56 and apply the CRC patch ──────────
RUN wget https://download.sourceforge.net/libpng/libpng-1.2.56.tar.gz -P /opt && \
    tar xf /opt/libpng-1.2.56.tar.gz -C /opt && \
    patch -p0 -d /opt/libpng-1.2.56 \
        < /opt/AFLplusplus/utils/libpng_no_checksum/libpng-nocrc.patch

# ── Build 1: instrumented (AFL++ + ASan) ────────────────────
#    Used for the white-box fuzzing campaign (Team A)
RUN cd /opt/libpng-1.2.56 && \
    CC=clang-14 \
    CXX=clang++-14 \
    ./configure --disable-shared --prefix=/opt/libpng-instrumented && \
    make clean && \
    AFL_USE_ASAN=1 \
    make -j4 \
      CC=afl-clang-fast \
      CXX=afl-clang-fast++ && \
    make install

# ── Build 2: vanilla (gcc, no instrumentation, no ASan) ─────
#    Used for the QEMU-mode fuzzing campaign (Team B)
#    CRC patch is kept so both campaigns explore comparable paths.
RUN cd /opt/libpng-1.2.56 && \
    make distclean && \
    CC=gcc \
    CFLAGS="-g -O1" \
    ./configure --disable-shared --prefix=/opt/libpng-vanilla && \
    make -j$(nproc) && \
    make install

# ── Set up working directory ─────────────────────────────────
WORKDIR /fuzzing

# Copy project files
COPY Makefile /fuzzing
COPY fuzzer-instrumented/src/build.sh /fuzzing/instrumented/src/
COPY fuzzer-instrumented/src/harness.c /fuzzing/instrumented/src/
COPY fuzzer-instrumented/src/run_fuzzer_withFlags.sh /fuzzing/instrumented/src/

# Copy seed corpus and dictionary into image
COPY docker/seeds/ /fuzzing/seeds/
COPY docker/dict/   /fuzzing/dict/

# ── Helpful environment variables ───────────────────────────
ENV AFL_SKIP_CPUFREQ=1 \
    AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_AUTORESUME=1

CMD ["/bin/bash"]
