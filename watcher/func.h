#ifndef JUDGER_WATCHER_FUNC_H
#define JUDGER_WATCHER_FUNC_H

#include <stdio.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while (0);\

#endif