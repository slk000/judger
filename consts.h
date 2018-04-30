#ifndef JUDGER_CONST_H
#define JUDGER_CONST_H

enum Judge_Result {
    JR_WAIT = 0,
    JR_REJUDGE = 1,
    JR_COMPILE = 2,
    JR_RUN = 3,
    JR_ACCEPT = 4,
    JR_PREERR = 5,
    JR_WRONG = 6,
    JR_TIMELIM = 7,
    JR_MEMLIM = 8,
    JR_OUTPUTLIM = 9,
    JR_RUNERR = 10,
    JR_COMPERR = 11,
    JR_OTHER = 12
};

enum Code_Lang {
    CL_C = 0,
    CL_CPP = 1,
    CL_Pascal = 2,
    CL_Java = 3,
    CL_Ruby = 4,
    CL_Bash = 5,
    CL_Python = 6,
    CL_PHP = 7,
    CL_Perl = 8,
    CL_CS = 9,
    CL_ObjC = 10,
    CL_FreeBasic = 11,
    CL_Text = 12,
    CL_Other = 13,
    CL_Lua = 14
};

const char LANG_EXT[][8] = { 
    "c", "cc"
};

const int BUFFER_SIZE = 512;

#endif