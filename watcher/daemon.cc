#include "daemon.h"

void create_daemon(void){
    pid_t pid;
    pid = fork();
    if (pid == -1)
        ERR_EXIT("fork error");
    if (pid > 0) // parent exits
        exit(EXIT_SUCCESS);
    if (setsid() == -1)
        ERR_EXIT("setsid error");
    
    chdir("/home/judger/");

    int i;
    for (i = 0; i < 3; i++){
        close(i);
        open("/dev/null", O_RDWR);
        dup(0);
        dup(0);
    }
    umask(0);
    return ;
}