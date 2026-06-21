#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// Declare startup function from startup.s (provided by lab)
extern void startup(void (*entry)(), int argc, char **argv);

// Iterate all program headers
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int, void *), void *arg) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr->e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "Not a valid ELF file.\n");
        return -1;
    }
    Elf32_Phdr *phdr = (Elf32_Phdr *)((char *)map_start + ehdr->e_phoff);
    for (int i = 0; i < ehdr->e_phnum; i++) {
        func(&phdr[i], i, arg);
    }
    return 0;
}

// Print header (task 1b)
void print_phdr_full(Elf32_Phdr *phdr, int i, void *arg) {
    printf("Program header %d:\n", i);
    char *type;
    switch(phdr->p_type) {
        case PT_NULL: type = "NULL"; break;
        case PT_LOAD: type = "LOAD"; break;
        case PT_DYNAMIC: type = "DYNAMIC"; break;
        case PT_INTERP: type = "INTERP"; break;
        case PT_NOTE: type = "NOTE"; break;
        case PT_PHDR: type = "PHDR"; break;
        default: type = "UNKNOWN"; break;
    }
    printf("Type: %s\n", type);
    printf("Offset:   0x%06x\n", phdr->p_offset);
    printf("VirtAddr: 0x%08x\n", phdr->p_vaddr);
    printf("PhysAddr: 0x%08x\n", phdr->p_paddr);
    printf("FileSiz:  0x%05x\n", phdr->p_filesz);
    printf("MemSiz:   0x%05x\n", phdr->p_memsz);
    printf("Flags: ");
    if (phdr->p_flags & PF_R) printf("R");
    if (phdr->p_flags & PF_W) printf("W");
    if (phdr->p_flags & PF_X) printf("E");
    printf("\n");
    printf("Align:    0x%x\n", phdr->p_align);
}

// Loader function for Task 2b
void load_segment(Elf32_Phdr *phdr, int i, void *map_start) {
    if (phdr->p_type != PT_LOAD) return;

    int prot = 0;
    if (phdr->p_flags & PF_R) prot |= PROT_READ;
    if (phdr->p_flags & PF_W) prot |= PROT_WRITE;
    if (phdr->p_flags & PF_X) prot |= PROT_EXEC;

    size_t page_size = sysconf(_SC_PAGESIZE);
    size_t aligned_vaddr = phdr->p_vaddr & ~(page_size -1);
    size_t offset_in_page = phdr->p_vaddr - aligned_vaddr;
    size_t map_length = phdr->p_memsz + offset_in_page;
    map_length = (map_length + page_size -1) & ~(page_size -1);

    /* Map writable first so we can copy the segment contents in; a text
       segment has no PF_W, so writing into a read-only mapping would segfault. */
    void *mapped = mmap((void *)aligned_vaddr, map_length, prot | PROT_WRITE,
                        MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS, -1, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    memcpy((char *)mapped + offset_in_page,
           (char *)map_start + phdr->p_offset,
           phdr->p_filesz);

    if (phdr->p_memsz > phdr->p_filesz) {
        memset((char *)mapped + offset_in_page + phdr->p_filesz,
               0,
               phdr->p_memsz - phdr->p_filesz);
    }

    /* Restore the segment's intended protection now that it is populated. */
    if (!(prot & PROT_WRITE)) {
        if (mprotect(mapped, map_length, prot) == -1) {
            perror("mprotect");
            exit(1);
        }
    }

    printf("Loaded segment %d at %p, length 0x%zx\n", i, (void *)aligned_vaddr, map_length);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("fopen");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (filesize < 0) {
        perror("ftell");
        fclose(file);
        return 1;
    }

    void *map_start = malloc(filesize);
    if (!map_start) {
        perror("malloc");
        fclose(file);
        return 1;
    }

    if (fread(map_start, 1, filesize, file) != (size_t)filesize) {
        fprintf(stderr, "Failed to read file\n");
        free(map_start);
        fclose(file);
        return 1;
    }
    fclose(file);

    printf("Program headers:\n");
    foreach_phdr(map_start, print_phdr_full, NULL);

    printf("\nLoading program segments into memory...\n");
    foreach_phdr(map_start, load_segment, map_start);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)map_start;
    printf("Entry point: 0x%08x\n", ehdr->e_entry);
    printf("Transferring control to loaded program...\n");
    startup((void (*)())ehdr->e_entry, argc - 1, argv + 1);


    // Remove jump for now
    // printf("Jumping to loaded program...\n");
    // void (entry)() = (void()())ehdr->e_entry;
    // entry();

    free(map_start);
    return 0;
}