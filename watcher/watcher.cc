#include "watcher.h"

int IS_EXIT_WATCHER = 0;

int main(void){
    create_daemon();

    signal(SIGQUIT, on_exit);
	signal(SIGKILL, on_exit);
	signal(SIGTERM, on_exit);

    while(!IS_EXIT_WATCHER){
        // TODO:
    }

    return 0;
}

void on_exit(int arg){
    printf("exit");
    IS_EXIT_WATCHER = 1;
}