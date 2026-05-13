FROM aflplusplus/aflplusplus:stable

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    autoconf \
    automake \
    libtool \
    make \
    gcc \
    g++ \
    clang \
    lld \
    gdb \
    wget \
    curl \
    ca-certificates \
    patch \
    zlib1g-dev \
    qemu-user \
    python3 \
    python3-pip \
    gnuplot \
    file \
    vim \
    less \
    && rm -rf /var/lib/apt/lists/*

# Download libpng 1.2.56 and apply AFL++ no-CRC patch.
RUN wget https://download.sourceforge.net/libpng/libpng-1.2.56.tar.gz -P /opt && \
    tar xf /opt/libpng-1.2.56.tar.gz -C /opt && \
    patch -p0 -d /opt/libpng-1.2.56 \
      < /AFLplusplus/utils/libpng_no_checksum/libpng-nocrc.patch

# Build 1: AFL-instrumented, no ASan.
RUN cd /opt/libpng-1.2.56 && \
    CC=afl-clang-fast \
    CXX=afl-clang-fast++ \
    CFLAGS="-g -O1" \
    LDFLAGS="" \
    ./configure --disable-shared --prefix=/opt/libpng-afl && \
    make -j$(nproc) && \
    make install

# Build 2: AFL-instrumented + ASan.
RUN cd /opt/libpng-1.2.56 && \
    make distclean && \
    CC=afl-clang-fast \
    CXX=afl-clang-fast++ \
    CFLAGS="-fsanitize=address -g -O1" \
    LDFLAGS="-fsanitize=address" \
    ./configure --disable-shared --prefix=/opt/libpng-afl-asan && \
    make -j$(nproc) && \
    make install

# Build 3: vanilla, no instrumentation, no ASan.
# Used for QEMU mode.
RUN cd /opt/libpng-1.2.56 && \
    make distclean && \
    CC=gcc \
    CFLAGS="-g -O1" \
    ./configure --disable-shared --prefix=/opt/libpng-vanilla && \
    make -j$(nproc) && \
    make install

WORKDIR /fuzzing

ENV AFL_SKIP_CPUFREQ=1 \
    AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1 \
    AFL_AUTORESUME=1

CMD ["/bin/bash"]
