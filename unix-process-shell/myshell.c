#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include "LineParser.h"

#define LINE_SIZE 2048
#define HISTLEN 20

#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process {
    cmdLine* cmd;
    pid_t pid;
    int status;
    struct process* next;
} process;

char* history[HISTLEN];
int history_size = 0;
process* proc_list = NULL;

void add_to_history(const char *cmd) {
    if (history_size < HISTLEN) {
        history[history_size++] = strdup(cmd);
    } else {
        free(history[0]);
        memmove(&history[0], &history[1], sizeof(char*) * (HISTLEN - 1));
        history[HISTLEN - 1] = strdup(cmd);
    }
}

void print_history() {
    for (int i = 0; i < history_size; i++)
        printf("%d: %s\n", i + 1, history[i]);
}

process* new_process(cmdLine* c, pid_t pid) {
    process* p = malloc(sizeof(process));
    p->cmd = c;
    p->pid = pid;
    p->status = RUNNING;
    p->next = NULL;
    return p;
}

void addProcess(process** list, cmdLine* c, pid_t pid) {
    process* p = new_process(c, pid);
    p->next = *list;
    *list = p;
}

void updateProcessList(process** list) {
    for (process *p = *list; p; p = p->next) {
        int wstatus = 0;
        pid_t r = waitpid(p->pid, &wstatus, WNOHANG | WUNTRACED);
        if (r == p->pid) {
            if (WIFEXITED(wstatus) || WIFSIGNALED(wstatus)) {
                p->status = TERMINATED;
            } else if (WIFSTOPPED(wstatus)) {
                p->status = SUSPENDED;
            } else if (WIFCONTINUED(wstatus)) {
                p->status = RUNNING;
            }
        }
    }
}

void printProcessList(process** list) {
    updateProcessList(list);
    printf("PID\t\tSTATUS\t\tCOMMAND\n");
    process *p = *list, *prev = NULL;
    while (p) {
        char *status_str;
        switch (p->status) {
            case RUNNING:   status_str = "Running"; break;
            case SUSPENDED: status_str = "Suspended"; break;
            default:        status_str = "Terminated"; break;
        }
        printf("%d\t\t%s\t\t%s\n", p->pid, status_str, p->cmd->arguments[0]);

        if (p->status == TERMINATED) {
            process *to_free = p;
            if (prev) prev->next = p->next;
            else *list = p->next;
            freeCmdLines(to_free->cmd);
            free(to_free);
            p = (prev) ? prev->next : *list;
        } else {
            prev = p;
            p = p->next;
        }
    }
}

void freeProcessList(process* list) {
    while (list) {
        process* next = list->next;
        freeCmdLines(list->cmd);
        free(list);
        list = next;
    }
}

int handle_builtin(cmdLine* cmd) {
    if (strcmp(cmd->arguments[0], "quit") == 0) {
        exit(0);
    }
    if (strcmp(cmd->arguments[0], "hist") == 0) {
        print_history();
        return 1;
    }
    if (strcmp(cmd->arguments[0], "procs") == 0) {
        printProcessList(&proc_list);
        return 1;
    }
    if (!strcmp(cmd->arguments[0], "halt") ||
        !strcmp(cmd->arguments[0], "wakeup") ||
        !strcmp(cmd->arguments[0], "ice")) {
        if (cmd->argCount < 2) {
            fprintf(stderr, "Usage: %s <pid>\n", cmd->arguments[0]);
            return 1;
        }
        pid_t pid = atoi(cmd->arguments[1]);
        if (pid <= 0) {
            fprintf(stderr, "%s: invalid pid '%s'\n", cmd->arguments[0], cmd->arguments[1]);
            return 1;
        }
        int sig = 0;
        if (!strcmp(cmd->arguments[0], "halt")) sig = SIGTSTP;
        if (!strcmp(cmd->arguments[0], "wakeup")) sig = SIGCONT;
        if (!strcmp(cmd->arguments[0], "ice")) sig = SIGINT;
        if (kill(pid, sig) == -1) {
            perror("kill");
        }

        // If wakeup command, update the process status to RUNNING manually
        if (!strcmp(cmd->arguments[0], "wakeup")) {
            process* p = proc_list;
            while (p) {
                if (p->pid == pid) {
                    p->status = RUNNING;
                    break;
                }
                p = p->next;
            }
        }
        return 1;
    }
    return 0;
}

void execute_pipeline(cmdLine* left, cmdLine* right) {
    if (left->arguments[0] == NULL || right->arguments[0] == NULL) {
        fprintf(stderr, "invalid pipe command\n");
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) { perror("pipe"); return; }

    pid_t pid1 = fork();
    if (pid1 < 0) { perror("fork"); close(pipefd[0]); close(pipefd[1]); return; }
    if (pid1 == 0) {
        // left command: stdout -> pipe (its input may still be redirected)
        if (left->inputRedirect) freopen(left->inputRedirect, "r", stdin);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(left->arguments[0], left->arguments);
        perror("execvp");
        exit(1);
    }

    close(pipefd[1]);                       // parent closes write end

    pid_t pid2 = fork();
    if (pid2 < 0) { perror("fork"); close(pipefd[0]); waitpid(pid1, NULL, 0); return; }
    if (pid2 == 0) {
        // right command: stdin <- pipe (its output may still be redirected)
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        if (right->outputRedirect) freopen(right->outputRedirect, "w", stdout);
        execvp(right->arguments[0], right->arguments);
        perror("execvp");
        exit(1);
    }

    close(pipefd[0]);                       // parent closes read end

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void execute_cmd(cmdLine* pCmd) {
    if (pCmd->arguments[0] == NULL) return;   // nothing to run (e.g. redirect-only line)

    if (pCmd->next != NULL) {                 // pipeline: pCmd | pCmd->next
        execute_pipeline(pCmd, pCmd->next);
        return;
    }

    if (handle_builtin(pCmd)) return;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        if (pCmd->inputRedirect) freopen(pCmd->inputRedirect, "r", stdin);
        if (pCmd->outputRedirect) freopen(pCmd->outputRedirect, "w", stdout);
        execvp(pCmd->arguments[0], pCmd->arguments);
        perror("execvp");
        exit(1);
    }

    if (pCmd->blocking) {
        // Foreground: wait here; main frees this cmd. It is NOT put in proc_list.
        waitpid(pid, NULL, 0);
        return;
    }

    // Background: proc_list takes ownership of pCmd (freed when it terminates).
    addProcess(&proc_list, pCmd, pid);
}

int main() {
    char buf[LINE_SIZE];

    printf("myshell> ");
    fflush(stdout);

    while (fgets(buf, LINE_SIZE, stdin)) {
        buf[strcspn(buf, "\n")] = '\0';

        if (buf[0] == '\0') {
            printf("myshell> ");
            fflush(stdout);
            continue;
        }

        if (!strcmp(buf, "!!")) {
            if (history_size == 0) {
                printf("No commands in history.\n");
                printf("myshell> ");
                fflush(stdout);
                continue;
            }
            strcpy(buf, history[history_size - 1]);
            printf("%s\n", buf);
        } else if (buf[0] == '!' && isdigit(buf[1])) {
            int idx = atoi(buf + 1) - 1;
            if (idx < 0 || idx >= history_size) {
                printf("No such command in history.\n");
                printf("myshell> ");
                fflush(stdout);
                continue;
            }
            strcpy(buf, history[idx]);
            printf("%s\n", buf);
        }

        add_to_history(buf);

        cmdLine* cmd = parseCmdLines(buf);
        if (!cmd) {
            printf("myshell> ");
            fflush(stdout);
            continue;
        }

        execute_cmd(cmd);
        if (cmd->next != NULL || cmd->blocking)
            freeCmdLines(cmd); // pipelines & foreground cmds freed here; background freed after termination

        printf("myshell> ");
        fflush(stdout);
    }

    for (int i = 0; i < history_size; i++) free(history[i]);
    freeProcessList(proc_list);

    return 0;
}
