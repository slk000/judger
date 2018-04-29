#ifndef JUDGER_JUDGER_JUDGER_H
#define JUDGER_JUDGER_JUDGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../consts.h"
#include "../db.h"

#define QUERY_SUBMISSION_INFO \
    "SELECT problemset_problem.id,time_limit,memory_limit,is_spj,code_lang,source_code \
    FROM `judger_submission` \
    INNER JOIN `problemset_problem`\
    ON judger_submission.problem_id=problemset_problem.id \
    WHERE judger_submission.id=%d"
struct Submission{
    int id;
    int box_id;
    int problem_id;
    int time_limit, time_used;
    int memory_limit, memory_used;
    int is_spj;
    enum Code_Lang code_lang;

    // char source_code[BUFFER_SIZE];
};

int get_submission_info(struct Submission *s);

int create_source_file(const char *code, enum Code_Lang lang);

#endif