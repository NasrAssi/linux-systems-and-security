# Low-Level Systems & Security — C and x86 Assembly

A portfolio of systems-programming projects written in **C** and **32-bit x86 assembly** on Linux. The work goes down to the ELF binary format, process control, dynamic loading, and low-level security tooling — implemented from scratch against the raw syscall/ABI level rather than with high-level libraries.

## What's inside

| Project | What it does | Key concepts |
|---|---|---|
| [**elf-analyzer**](elf-analyzer) | Interactive tool that parses 32-bit ELF files — header, section table, symbol table — and **merges two relocatable objects** into one. | ELF format, `mmap`, section/symbol tables |
| [**elf-loader**](elf-loader) | A user-space program loader that maps an ELF's `PT_LOAD` segments into memory with correct permissions and transfers control to the entry point. | `mmap`/`mprotect`, program headers, paging, ABI hand-off |
| [**antivirus-scanner**](antivirus-scanner) | Signature-based scanner: loads a virus-signature database (big/little-endian), detects matches in a file, and **neutralizes** them. | binary parsing, endianness, linked lists, file patching |
| [**custom-shell**](custom-shell) | A Unix shell with `fork`/`exec`, pipelines, I/O redirection, background jobs, `cd`, and signal control. | process control, pipes, `dup2`, signals |
| [**x86-assembly**](x86-assembly) | Hand-written x86 assembly implementing a 16-bit **LFSR** pseudo-random generator with hex-string parsing, calling into libc. | NASM, x86 ABI, calling libc, LFSR |
| [**elf-binary-tools**](elf-binary-tools) | A `hexeditplus` memory/file hex-editor plus a program used for **gdb binary patching**, with ELF-format diagrams. | hex editing, gdb, binary patching, ELF layout |

## Skills demonstrated

- **Languages:** C (C99), x86 assembly (NASM)
- **Systems:** Linux syscalls, `fork`/`exec`/`wait`, pipes, `dup2`, signals, `mmap`/`mprotect`, file I/O
- **Binary / ELF:** ELF32 headers, program & section headers, symbol & relocation tables, dynamic loading, the x86 calling convention
- **Tooling:** GCC (`-m32`), NASM, GNU Make, GDB, Linux

## Building & running

All projects target **32-bit x86 Linux**. On a 64-bit host install the multilib toolchain:

```bash
sudo apt-get install gcc-multilib nasm gdb make   # Debian / Ubuntu
```

Then, in any project folder:

```bash
make          # build
make clean    # remove build artifacts
```

Each project has its own README with run instructions and example sessions.

## Note on academic integrity

These projects began as university systems-lab assignments and are published here as a personal portfolio of my own work. If you are currently taking a similar course, please **do not copy** — submitting this as your own violates academic-integrity rules, and plagiarism tools flag public repositories.
