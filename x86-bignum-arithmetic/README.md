# x86 Big-Number Arithmetic

A hand-written **32-bit x86 assembly** program (NASM) that performs **multi-precision (arbitrary-length) integer arithmetic** — the kind of routine that underpins cryptography and bignum libraries — built entirely at the register level and linked against libc.

## Number representation
A number is a **length-prefixed byte array**: the first byte holds the length, followed by that many data bytes (little-endian limbs). This lets the program handle integers far larger than a single machine register.

## What it implements (`multi.s`)
- **`getmulti`** — reads a hex string from stdin, counts the digits, pads to an even length, allocates a buffer with `malloc`, and converts each hex pair into a byte (`hexchar_to_nibble`).
- **`add_multi`** — adds two multi-precision numbers limb-by-limb with **carry propagation**, sizing the result to the larger operand (`MaxMin`).
- **`print_multi`** — prints a number as a hex byte sequence via `printf`.
- **`rand_num`** — a 16-bit **LFSR** (`STATE = 0xACE1`, tap `MASK = 0xB400`) that generates pseudo-random numbers.
- Uses `.data`, `.bss`, `.rodata`, and `.text` sections and `extern` linkage to `printf`, `puts`, `malloc`, `fgets`, and `exit`.

## Build & run
```bash
make            # nasm -f elf32 + gcc -m32  ->  ./multi
./multi
```
Enter hex digits (no spaces) when prompted; the program builds the number, performs the arithmetic, and prints the results.

## Concepts
NASM, the x86 cdecl calling convention, calling libc from assembly, multi-precision addition with carry, length-prefixed buffers, and LFSR pseudo-random generation.
