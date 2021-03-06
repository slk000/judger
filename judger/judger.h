#ifndef JUDGER_JUDGER_JUDGER_H
#define JUDGER_JUDGER_JUDGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dirent.h>

#include "../consts.h"
#include "../db.h"
#include "compile_cmd.h"

#ifdef ___i386
#include "suscall_32.h"
#else
#include "syscall_64.h"
#endif

#define QUERY_SUBMISSION_INFO \
    "SELECT problemset_problem.id,time_limit,memory_limit,is_spj,code_lang,source_code \
    FROM `judger_submission` \
    INNER JOIN `problemset_problem`\
    ON judger_submission.problem_id=problemset_problem.id \
    WHERE judger_submission.id=%d"
#define QUERY_UPDATE_SUBMISSION \
    "UPDATE judger_submission \
    SET judge_result=%d, \
    used_time=%d, \
    used_memory=%d, \
    judge_datetime='%s' \
    WHERE id=%d"
#define QUERY_INSERT_COMPILE_INFO \
    "INSERT INTO `judger_compileinfo`(submission_id, compile_info) \
    VALUES(%d,\'%%s\')"
#define QUERY_DELETE_COMPILE_INFO \
    "DELETE FROM `judger_compileinfo` \
    WHERE submission_id=%d"
struct Submission{
    int id;
    int box_id;
    int problem_id;
    int time_limit, used_time;
    int memory_limit, used_memory;
    int is_spj;
    enum Code_Lang code_lang;
    enum Judge_Result judge_result;
    // char source_code[BUFFER_SIZE];
};

int *CALLS[] = {LANG_CV, LANG_CV};

int get_submission_info(struct Submission *s);

int update_submission(const struct Submission *s);

int create_source_file(const char *code, enum Code_Lang lang);

int compile(struct Submission *s);

int update_compile_info(const struct Submission *s);

int prepare_run_env(int lang);

int prepare_test_files(const char *case_dir, const char *filename);

int test_cases(struct Submission *s);

int is_file_ext(const char *filename, const char *ext);

int System(const char *format, ...);

int get_allowed_syscalls(int lang, int *arr);
#endif