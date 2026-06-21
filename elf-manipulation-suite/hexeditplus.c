#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MEM_BUF_SIZE 10000

// Global state
char debug_mode = 0;
char display_mode = 0; // 0 = hexadecimal, 1 = decimal
char file_name[128] = "";
int unit_size = 1;
unsigned char mem_buf[MEM_BUF_SIZE];
size_t mem_count = 0;

// Format strings for printing units
static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "Unknown unit\n", "%#x\n"};
static char* dec_formats[] = {"%hhd\n", "%hd\n", "Unknown unit\n", "%d\n"};

// Function prototypes
void toggle_debug_mode();
void set_file_name();
void set_unit_size();
void load_into_memory();
void toggle_display_mode();
void memory_display();
void save_into_file();
void memory_modify();
void quit_program();

// Menu definition
struct menu_option { char* name; void (*func)(); };

struct menu_option menu[] = {
    {"Toggle Debug Mode", toggle_debug_mode},
    {"Set File Name",      set_file_name},
    {"Set Unit Size",      set_unit_size},
    {"Load Into Memory",   load_into_memory},
    {"Toggle Display Mode",toggle_display_mode},
    {"Memory Display",     memory_display},
    {"Save Into File",     save_into_file},
    {"Memory Modify",      memory_modify},
    {"Quit",               quit_program},
    {NULL,                  NULL}
};

int main() {
    while (1) {
        if (debug_mode) {
            fprintf(stderr, "Debug: file_name='%s', unit_size=%d, mem_count=%zu\n",
                    file_name, unit_size, mem_count);
        }
        printf("Choose action:\n");
        for (int i = 0; menu[i].name; i++) {
            printf("%d - %s\n", i, menu[i].name);
        }
        printf("Option: ");
        int choice;
        if (scanf("%d", &choice) != 1) { printf("Invalid input\n"); exit(1); }
        getchar();
        int num_options = 0;
        while (menu[num_options].name) num_options++;
        if (choice >= 0 && choice < num_options && menu[choice].func) {
            menu[choice].func();
        } else {
            printf("Invalid choice\n");
        }
        printf("\n");
    }
    return 0;
}

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    printf("Debug flag now %s\n", debug_mode ? "on" : "off");
}

void set_file_name() {
    char input[256];
    printf("Enter file name: ");
    if (fgets(input, sizeof(input), stdin) == NULL) return;
    input[strcspn(input, "\n")] = '\0';
    strncpy(file_name, input, sizeof(file_name)-1);
    if (debug_mode) fprintf(stderr, "Debug: file name set to '%s'\n", file_name);
}

void set_unit_size() {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    if (scanf("%d", &size) != 1) { printf("Invalid input\n"); return; }
    getchar();
    if (size == 1 || size == 2 || size == 4) {
        unit_size = size;
        if (debug_mode) fprintf(stderr, "Debug: set unit size to %d\n", unit_size);
    } else {
        printf("Error: invalid unit size\n");
    }
}

void load_into_memory() {
    if (file_name[0] == '\0') { printf("Error: file name is empty\n"); return; }
    unsigned int location;
    int length;
    printf("Please enter <location> <length>\n");
    if (scanf("%x %d", &location, &length) != 2) { printf("Invalid input\n"); getchar(); return; }
    getchar();
    if (length < 0 || length > MEM_BUF_SIZE / unit_size) {
        printf("Error: invalid length (max %d units for unit size %d)\n", MEM_BUF_SIZE / unit_size, unit_size);
        return;
    }
    ssize_t to_read = (ssize_t)length * unit_size;
    if (debug_mode) fprintf(stderr,
        "Debug: file_name='%s', location=0x%x, length=%d\n",
        file_name, location, length);
    int fd = open(file_name, O_RDONLY);
    if (fd < 0) { perror("open"); return; }
    if (lseek(fd, location, SEEK_SET) == (off_t)-1) { perror("lseek"); close(fd); return; }
    ssize_t rd = read(fd, mem_buf, to_read);
    if (rd < 0) { perror("read"); close(fd); return; }
    mem_count = rd / unit_size;
    printf("Loaded %zu units into memory\n", mem_count);
    close(fd);
}

void toggle_display_mode() {
    display_mode = !display_mode;
    printf("Display flag now %s, %s representation\n",
        display_mode ? "on" : "off",
        display_mode ? "decimal" : "hexadecimal");
}

void memory_display() {
    unsigned int addr;
    int length;
    printf("Enter address and length\n");
    if (scanf("%x %d", &addr, &length) != 2) { printf("Invalid input\n"); getchar(); return; }
    getchar();
    char *start = (addr == 0 ? (char*)mem_buf : (char*)addr);
    if (display_mode) {
        printf("Decimal ========\n");
        for (int i = 0; i < length; i++) {
            unsigned int val = 0;
            memcpy(&val, start + i * unit_size, unit_size);
            printf(dec_formats[unit_size-1], val);
        }
    } else {
        printf("Hexadecimal ========\n");
        for (int i = 0; i < length; i++) {
            unsigned int val = 0;
            memcpy(&val, start + i * unit_size, unit_size);
            printf(hex_formats[unit_size-1], val);
        }
    }
}

void save_into_file() {
    if (file_name[0] == '\0') { printf("Error: file name is empty\n"); return; }
    unsigned int source, target;
    int length;
    printf("Please enter <source-address> <target-location> <length>\n");
    if (scanf("%x %x %d", &source, &target, &length) != 3) { printf("Invalid input\n"); getchar(); return; }
    getchar();
    if (debug_mode) fprintf(stderr,
        "Debug: source=0x%x, target=0x%x, length=%d\n",
        source, target, length);
    int fd = open(file_name, O_RDWR);
    if (fd < 0) { perror("open"); return; }
    if (lseek(fd, target, SEEK_SET) == (off_t)-1) { perror("lseek"); close(fd); return; }
    char *src_ptr = (source == 0 ? (char*)mem_buf : (char*)source);
    ssize_t to_write = (ssize_t)length * unit_size;
    ssize_t wr = write(fd, src_ptr, to_write);
    if (wr < 0) { perror("write"); close(fd); return; }
    close(fd);
}

void memory_modify() {
    unsigned int location, val;
    printf("Please enter <location> <val>\n");
    if (scanf("%x %x", &location, &val) != 2) { printf("Invalid input\n"); getchar(); return; }
    getchar();
    if (location > (unsigned int)(MEM_BUF_SIZE - unit_size)) {
        printf("Error: location out of bounds\n");
        return;
    }
    if (debug_mode) fprintf(stderr, "Debug: location=0x%x, val=0x%x\n", location, val);
    memcpy(mem_buf + location, &val, unit_size);
}

void quit_program() {
    if (debug_mode) printf("quitting\n");
    exit(0);
}