#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    pid_t pid1, pid2;

    fprintf(stderr, "(parent_process>forking...)\n");
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    //run "ls -ls"
    pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid1 == 0) {
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        fprintf(stderr, "(child1>going to execute cmd: ls -ls)\n");
        execlp("ls", "ls", "-ls", (char*)NULL);
        perror("execlp ls");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);
    fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");
    close(pipefd[1]);

    pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid2 == 0) {
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        fprintf(stderr, "(child2>going to execute cmd: wc)\n");
        execlp("wc", "wc", (char*)NULL);
        perror("execlp wc");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");
    close(pipefd[0]);

    fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    // Exit
    fprintf(stderr, "(parent_process>exiting...)\n");
    return 0;
}
