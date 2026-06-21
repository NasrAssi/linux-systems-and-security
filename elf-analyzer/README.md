# ELF Analyzer (`myELF`)

An interactive command-line tool for inspecting and manipulating **32-bit ELF** object files — similar in spirit to `readelf`, but written entirely from scratch in C with **no third-party libraries**, reading the raw binary layout directly via `mmap`.

## Features
- **Examine ELF header** — validates the magic number and prints the entry point, section-header table offset, and counts.
- **List section headers** — name, type, address, offset, and size for every section.
- **Dump the symbol table** — symbol names (resolved through the string tables), values, and the sections they belong to.
- **Merge two relocatable objects** into a single combined ELF.
- Toggle debug output. Files are mapped with `mmap` for zero-copy access, and up to two files can be loaded at once.

## Build & run
```bash
make
./myELF
```
You'll get a numbered menu (examine file, print section names, print symbols, check/merge files, quit). To get a 32-bit ELF to inspect, compile any C file:
```bash
gcc -m32 -c hello.c -o hello.o
```
then choose **Examine ELF File** and enter `hello.o`.

## Concepts
ELF32 layout (`Elf32_Ehdr`, `Elf32_Shdr`, `Elf32_Sym`), section & string tables, `mmap`-based file access, and a function-pointer menu dispatch table — all without linking against any ELF helper library.
