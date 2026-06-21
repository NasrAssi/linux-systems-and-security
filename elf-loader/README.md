# ELF Loader

A user-space **program loader**: given a statically-linked 32-bit ELF executable, it parses the program headers, maps each loadable segment into memory at the correct virtual address with the correct permissions, and then jumps to the program's entry point — doing by hand what the kernel's loader does.

![Loaded object memory layout](loaded-object.jpg)

## How it works
1. `foreach_phdr` walks the ELF program-header table and validates the magic number.
2. For every `PT_LOAD` segment, `load_segment`:
   - computes the page-aligned virtual address and mapping length,
   - `mmap`s the region (`MAP_FIXED | MAP_ANONYMOUS`), copies the segment's file contents in, and zero-fills the `.bss` tail (`memsz > filesz`),
   - then `mprotect`s the region back to its intended `R/W/X` flags (mapped writable first so a read-only text segment can be populated).
3. A small assembly stub (`startup.s`) sets up `argc`/`argv` and transfers control to the loaded program's entry point.

## Build & run
```bash
make                 # gcc -m32 loader.c + nasm startup.s  ->  ./loader
./loader loadme      # loads and runs the bundled sample ELF
```
`loadme` is a small 32-bit ELF included so you can watch the loader run end-to-end. It is linked at a **non-default base address** (see `linking_script`, a GNU `ld` script that sets the load address to `0x04048000`) so it doesn't overlap the loader's own `0x08048000` address space.

## Concepts
ELF program headers, `mmap`/`mprotect`, page alignment, the `filesz` vs `memsz` (`.bss`) distinction, linker scripts / load addresses, and the x86 calling convention for handing control to a freshly loaded image.
