#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
 
void handler(int sig)
{
    char *msg;
    switch (sig)
    {
        case SIGINT:  msg = "\nReceived Signal: SIGINT\n";  break;
        case SIGTSTP: msg = "\nReceived Signal: SIGTSTP\n"; break;
        case SIGCONT: msg = "\nReceived Signal: SIGCONT\n"; break;
        default:      msg = "\nReceived Signal\n";          break;
    }
    write(STDOUT_FILENO, msg, strlen(msg));
    signal(sig, SIG_DFL);
    raise(sig);
}
 
int main(int argc, char **argv)
{
    signal(SIGINT, handler);
    signal(SIGTSTP, handler);
    signal(SIGCONT, handler);
 
    while (1)
    {
        sleep(1);
    }
 
    return 0;
}