# Linux Systems & Security

A suite of low-level systems-programming projects written in **C** and **32-bit x86 assembly** on Linux. The work reaches down to the ELF binary format, process control, dynamic loading, multi-precision arithmetic, and security tooling — all implemented from scratch against the raw syscall/ABI level, with no high-level frameworks.

**Stack:** C (C99) · x86 assembly (NASM) · GNU Make · GNU ld linker scripts · GDB · Linux (32-bit / i386)

## Projects

| Project | Summary | Key concepts |
|---|---|---|
| [**elf-loader**](elf-loader) | A user-space program loader that maps an ELF's `PT_LOAD` segments into memory with correct permissions and transfers control to the entry point. | `mmap`/`mprotect`, program headers, paging, ABI hand-off |
| [**elf-analyzer**](elf-analyzer) | Interactive ELF inspector that parses headers, section tables, and symbol tables — and merges two relocatable objects — with no third-party libraries. | ELF format, `mmap`, section/symbol tables |
| [**signature-based-antivirus**](signature-based-antivirus) | Loads a virus-signature database (big/little-endian), detects byte-sequence matches in a target file, and neutralizes them by patching the binary. | binary parsing, endianness, linked lists, file patching |
| [**unix-process-shell**](unix-process-shell) | A Unix shell with process forking/execution, pipelines, I/O redirection, background jobs, and signal control. | `fork`/`exec`, pipes, `dup2`, signals |
| [**x86-bignum-arithmetic**](x86-bignum-arithmetic) | Hand-written x86 assembly performing multi-precision (big-number) arithmetic on length-prefixed byte arrays, with an LFSR random generator. | NASM, x86 ABI, carry propagation, LFSR, libc calls |
| [**elf-manipulation-suite**](elf-manipulation-suite) | A `hexeditplus` memory/file hex-editor plus a gdb binary-patching target, with annotated ELF-format diagrams. | hex editing, raw file syscalls, gdb, ELF layout |

## Skills demonstrated

- **Languages:** C (C99), x86 assembly (NASM)
- **Operating systems:** Linux syscalls, `fork`/`exec`/`wait`, pipes, `dup2`, signals, `mmap`/`mprotect`, raw file I/O
- **Binary / ELF internals:** ELF32 headers, program & section headers, symbol & relocation tables, dynamic loading, linker scripts, the x86 calling convention
- **Tooling:** GCC (`-m32`), NASM, GNU Make, GDB

## Building & running

All projects target **32-bit x86 Linux**. On a 64-bit host, install the multilib toolchain:

```bash
sudo apt-get install gcc-multilib nasm gdb make   # Debian / Ubuntu
```

Then, in any project folder:

```bash
make          # build
make clean    # remove build artifacts
```

Each project has its own README with run instructions, example sessions, and the concepts it demonstrates.

---

<sub>These projects originated in an academic systems-programming setting and are shared as a personal portfolio of my own work. Please don't submit them as your own — doing so violates academic-integrity policies, and plagiarism tools scan public repositories.</sub>
