#ifndef JUDGER_WATCHER_WATCHER_H
#define JUDGER_WATCHER_WATCHER_H

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <mysql.h>

#include "daemon.h"
#include "../db.h"

#define MAX_JUDGING_CNT (3)
#define MAX_SUBMISSION_CNT 100

#define _TO_STRING(x) #x
#define TO_STRING(x) _TO_STRING(x)
#define QUERY_NEW_SUBMISSION \
    "SELECT id FROM `judger_submission` \
    WHERE judge_result=0 or judge_result=1 \
    ORDER BY judge_result ASC, id ASC"
#define QUERY_LIMIT(x) " LIMIT " TO_STRING(x)

// 准备退出watcher
void on_exit(int /*arg*/);

// 在数组里查找目标，返回下标或-1
inline int get_index_in_arr(int *arr, int n, int target);

// 获取提交，存入submission_queue中，返回个数
int get_submission_ids(uint *submission_queue);

// 为每个提交运行judger
void get_and_judge(void);

// 执行judger
void run_judger(int submission_id, int box_index);

#endif