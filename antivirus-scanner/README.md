# Signature-Based Antivirus

A small antivirus engine in C. It loads a database of virus **signatures**, scans a suspect file for any of them, reports every match with its offset, and can **neutralize** an infection by patching the binary.

## Features
- **Loads a signature database** with a magic number that selects byte order: `VIRL` (little-endian) or `VIRB` (big-endian). The signature size field is byte-swapped accordingly.
- Stores signatures in a linked list; each record has a name, size, and raw signature bytes.
- **Print signatures** in a readable hex dump.
- **Detect** — scans the whole file buffer and prints the name, size, and offset of every signature match.
- **Fix** — overwrites the first byte of a detected signature with `0xC3` (x86 `ret`), neutralizing the malicious code in place.

## Build & run
```bash
make
./AntiVirus infected        # 'infected' is a bundled sample file
```
Then use the menu: **Load signatures** (`signatures-L` or `signatures-B`) → **Detect** → **Fix**.

Bundled fixtures: `signatures-L` / `signatures-B` (little/big-endian signature DBs) and `infected` (a sample file containing a signature).

## Concepts
Binary file-format parsing, endianness handling, linked lists with proper allocation/free, `memcmp`-based pattern matching, and in-place file patching with `fopen(..., "r+")`.
