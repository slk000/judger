#ifndef JUDGER_JUDGER_COMPILE_CMD_H
#define JUDGER_JUDGER_COMPILE_CMD_H

#include <stdio.h>

const char * CP_C[] = { "gcc", "Main.c", "-O2","-o", "Main", "-fno-asm", "-Wall",
        "-lm", "--static", "-std=c99", "-DONLINE_JUDGE", NULL };
const char * CP_CPP[] = { "g++", "Main.cc", "-O2", "-o", "Main", "-fno-asm", "-Wall",
        "-lm", "--static", "-std=c++0x", "-DONLINE_JUDGE", NULL };

const char **CP_CMD[] = {CP_C, CP_CPP, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#endif