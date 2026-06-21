#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFERSIZE 10000

typedef struct virus {
    unsigned short SigSize;
    unsigned char* VirusName;
    unsigned char* Sig;
} virus;

typedef struct link link;

struct link {
    link* nextVirus;
    virus* vir;
};

void printVirus(virus* virus, FILE* output);
virus* readVirus(FILE* file);
void list_print(link* virus_list, FILE* output);
void list_free(link* virus_list);
link* list_append(link* virus_list, virus* data);
void detect_virus(char* buffer, unsigned int size, link* virus_list);
void neutralize_virus(char* fileName, int signatureOffset);

link* virusList = NULL;
char suspectedFile[256] = {0};
int isBigEndian = 0;

void printVirus(virus* v, FILE* output) {
    fprintf(output, "Virus name: %s\n", v->VirusName);
    fprintf(output, "Virus size: %u\n", v->SigSize);
    fprintf(output, "signature:\n");
    for (int i = 0; i < v->SigSize; i++) {
        fprintf(output, "%02X", v->Sig[i]);
        if ((i + 1) % 20 == 0 || i == v->SigSize - 1)
            fprintf(output, "\n");
        else
            fprintf(output, " ");
    }
    fprintf(output, "\n");
}

virus* readVirus(FILE* file) {
    virus* v = malloc(sizeof(virus));
    if (!v) return NULL;

    if (fread(&v->SigSize, 1, 2, file) != 2) {
        free(v);
        return NULL;
    }

    if (isBigEndian)
        v->SigSize = (v->SigSize >> 8) | (v->SigSize << 8);

    v->VirusName = calloc(17, 1);
    if (!v->VirusName) {
        free(v);
        return NULL;
    }
    if (fread(v->VirusName, 1, 16, file) != 16) {
        free(v->VirusName);
        free(v);
        return NULL;
    }

    v->Sig = malloc(v->SigSize);
    if (!v->Sig) {
        free(v->VirusName);
        free(v);
        return NULL;
    }
    if (fread(v->Sig, 1, v->SigSize, file) != v->SigSize) {
        free(v->Sig);
        free(v->VirusName);
        free(v);
        return NULL;
    }

    return v;
}

link* list_append(link* virus_list, virus* data) {
    link* new_node = malloc(sizeof(link));
    if (!new_node) return virus_list;
    new_node->vir = data;
    new_node->nextVirus = virus_list;
    return new_node;
}

void list_print(link* virus_list, FILE* output) {
    while (virus_list) {
        printVirus(virus_list->vir, output);
        virus_list = virus_list->nextVirus;
    }
}

void list_free(link* virus_list) {
    while (virus_list) {
        link* next = virus_list->nextVirus;
        free(virus_list->vir->Sig);
        free(virus_list->vir->VirusName);
        free(virus_list->vir);
        free(virus_list);
        virus_list = next;
    }
}

void detect_virus(char* buffer, unsigned int size, link* virus_list) {
    int found = 0;
    for (int i = 0; i < size; i++) {
        link* curr = virus_list;
        while (curr) {
            virus* v = curr->vir;
            if (v->SigSize > 0 && i + v->SigSize <= size && memcmp(buffer + i, v->Sig, v->SigSize) == 0) {
                printf("Virus found!\nLocation: %d\nName: %s\nSize: %u\n\n",
                       i, v->VirusName, v->SigSize);
                found = 1;
            }
            curr = curr->nextVirus;
        }
    }
    if (!found)
        printf("No viruses detected.\n");
}

void neutralize_virus(char* fileName, int signatureOffset) {
    FILE* f = fopen(fileName, "r+");
    if (!f) {
        perror("Error opening file to neutralize");
        return;
    }
    fseek(f, signatureOffset, SEEK_SET);
    char ret = 0xC3;
    fwrite(&ret, 1, 1, f);
    fclose(f);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <suspected file>\n", argv[0]);
        return 1;
    }
    strncpy(suspectedFile, argv[1], sizeof(suspectedFile) - 1);
    int done = 0;
    while (!done) {
        printf("Menu:\n");
        printf("1) Load signatures\n");
        printf("2) Print signatures\n");
        printf("3) Detect viruses\n");
        printf("4) Fix file\n");
        printf("5) Quit\n");
        printf("Enter option: ");
        char input[10];
        if (!fgets(input, sizeof(input), stdin)) break;

        int choice = 0;
        sscanf(input, "%d", &choice);
        switch (choice) {
            case 1: {
                printf("Enter signature file name: ");
                char sigFile[256];
                if (!fgets(sigFile, sizeof(sigFile), stdin)) break;
                sigFile[strcspn(sigFile, "\n")] = '\0';

                FILE* file = fopen(sigFile, "rb");
                if (!file) {
                    perror("Failed to open signature file");
                    break;
                }

                char magic[4];
                if (fread(magic, 1, 4, file) != 4) {
                    fprintf(stderr, "Failed to read magic number.\n");
                    fclose(file);
                    break;
                }

                if (strncmp(magic, "VIRL", 4) == 0) {
                    isBigEndian = 0;
                } else if (strncmp(magic, "VIRB", 4) == 0) {
                    isBigEndian = 1;
                } else {
                    fprintf(stderr, "Invalid magic number.\n");
                    fclose(file);
                    break;
                }

                // Replace any previously loaded signatures (avoid duplicates on reload)
                list_free(virusList);
                virusList = NULL;

                while (1) {
                    virus* v = readVirus(file);
                    if (!v) break;
                    virusList = list_append(virusList, v);
                }

                fclose(file);
                break;
            }
            case 2:
                list_print(virusList, stdout);
                break;
            case 3: {
                FILE* file = fopen(suspectedFile, "rb");
                if (!file) {
                    fprintf(stderr, "cannot open %s\n", suspectedFile);
                    break;
                }
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);
                if (fsize < 0) { fclose(file); break; }
                char* buffer = malloc(fsize);
                if (!buffer) { fclose(file); break; }
                size_t readBytes = fread(buffer, 1, fsize, file);
                fclose(file);
                detect_virus(buffer, readBytes, virusList);
                free(buffer);
                break;
            }
            case 4: {
                FILE* file = fopen(suspectedFile, "rb");
                if (!file) {
                    fprintf(stderr, "cannot open %s\n", suspectedFile);
                    break;
                }
                fseek(file, 0, SEEK_END);
                long fsize = ftell(file);
                fseek(file, 0, SEEK_SET);
                if (fsize < 0) { fclose(file); break; }
                char* buffer = malloc(fsize);
                if (!buffer) { fclose(file); break; }
                size_t readBytes = fread(buffer, 1, fsize, file);
                fclose(file);

                for (size_t i = 0; i < readBytes; i++) {
                    link* curr = virusList;
                    while (curr) {
                        if (curr->vir->SigSize > 0 &&
                            i + curr->vir->SigSize <= readBytes &&
                            memcmp(buffer + i, curr->vir->Sig, curr->vir->SigSize) == 0) {
                            neutralize_virus(suspectedFile, i);
                        }
                        curr = curr->nextVirus;
                    }
                }
                free(buffer);
                break;
            }
            case 5:
                list_free(virusList);
                done = 1;
                break;
            default:
                printf("Invalid option.\n");
                break;
        }
    }
    return 0;
}