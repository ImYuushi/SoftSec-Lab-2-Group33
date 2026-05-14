# Lab 2 Software Security Group 33

This repository contains our fuzzing setup for `libpng` using AFL++.

We run two campaign types:

1. **Instrumented / white-box fuzzing** using AFL++ compile-time instrumentation and AddressSanitizer.
2. **Binary-only / black-box fuzzing** using AFL++ QEMU mode against an uninstrumented binary.

The target library is `libpng`.

## Start the Docker Environment

Build the image:

```bash
cd docker
docker compose build
```

Start the container:

```bash
docker compose up -d
```

Enter the container:

```bash
docker compose exec fuzzer bash
```

Stop the container:

```bash
docker compose down
```

## Using the Makefile

Build all fuzzing binaries:

```bash
make build
```

Run all instrumented AFL++ campaigns:

```bash
make fuzz
```

Run individual instrumented campaigns:

```bash
make fuzz_read_image
make fuzz_progressive
make fuzz_transforms
```

Run the QEMU / binary-only campaigns:

```bash
make fuzz-qemu
```

Clean generated outputs:

```bash
make clean
```
