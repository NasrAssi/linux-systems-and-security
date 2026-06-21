# x86 Assembly — LFSR Generator

A hand-written **32-bit x86 assembly** program (NASM syntax) that reads a string of hex digits, runs a 16-bit **Linear-Feedback Shift Register (LFSR)** pseudo-random generator over the data, and prints results — all while calling into the C library (`malloc`, `printf`, `fgets`).

## Highlights
- Pure assembly with a proper `main` prologue/epilogue and the **cdecl calling convention** for libc calls.
- A 16-bit LFSR (`STATE = 0xACE1`, tap `MASK = 0xB400`) — a classic hardware-style pseudo-random sequence generator.
- Manual hex-string parsing into a byte buffer, dynamic allocation with `malloc`, and formatted output with `printf`/`puts`.
- Demonstrates `.data`, `.bss`, `.rodata`, and `.text` section usage and `extern` linkage against libc.

## Build & run
```bash
make            # nasm -f elf32 + gcc -m32
./multi
```

## Concepts
NASM, the x86 (cdecl) ABI, calling C library functions from assembly, LFSR pseudo-random generation, and manual buffer/string handling at the register level.
