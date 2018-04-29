#include "watcher.h"

int IS_EXIT_WATCHER = 0;

int main(void){
    create_daemon();

    signal(SIGQUIT, on_exit);
    signal(SIGKILL, on_exit);
    signal(SIGTERM, on_exit);

    connect_mysql();
    while(!IS_EXIT_WATCHER){
        get_and_judge();
        sleep(3);
    }

    return 0;
}

void on_exit(int arg){
    printf("exit");
    mysql_free_result(DB_RES); 
    close_mysql();
    IS_EXIT_WATCHER = 1;
}

inline int get_index_in_arr(int *arr, int n, int target){
    int i = 0;
    for (; i < n && arr[i] != target; i++);
    return i == n ? -1 : i;
}

// 获取提交，存入submission_queue中，返回个数
int get_submission_ids(uint *submission_queue){
    if (!execute_sql(QUERY_NEW_SUBMISSION QUERY_LIMIT(MAX_SUBMISSION_CNT)))
        return 0;

    int cnt = 0;
    memset(submission_queue, 0, sizeof(submission_queue));
    DB_RES = mysql_store_result(gDB_CONN);
    
    while(DB_ROW = mysql_fetch_row(DB_RES)){
        submission_queue[cnt] = atoi(DB_ROW[0]);
        cnt++;
    }
    
    return cnt;
}

// 为每个提交运行judger
void get_and_judge(void){
    uint submissions[MAX_SUBMISSION_CNT];
    static pid_t judgers_box[100];// 各个judger进程
    static int now_judging_cnt = 0;// 当前运行+未回收的judger个数
    int submissions_cnt = get_submission_ids(submissions);
    
    printf("got %d submissions\n", submissions_cnt);
    if (0 == submissions_cnt) return;

    for (int i = 0; i<submissions_cnt && submissions[i] > 0; i++){
        uint submission_id = submissions[i];

        int idle_box_index;
        if (now_judging_cnt < MAX_JUDGING_CNT){
            // 找到一个空闲的位置
            idle_box_index = get_index_in_arr(judgers_box, MAX_JUDGING_CNT, 0);
        }
        else{
            // 或者等待已有的子进程终止
            pid_t ended_p = waitpid(-1, NULL, 0);
            now_judging_cnt--;
            idle_box_index = get_index_in_arr(judgers_box, MAX_JUDGING_CNT, ended_p);
        }

        now_judging_cnt++;
        judgers_box[idle_box_index] = fork();
        if (0 == judgers_box[idle_box_index]){
            run_judger(submission_id, idle_box_index);
            exit(0);
        }
    }

    pid_t recycle = 0;
    while((recycle = waitpid(-1, NULL, WNOHANG)) > 0){
        now_judging_cnt--;
        int index = get_index_in_arr(judgers_box, MAX_JUDGING_CNT, recycle);
        judgers_box[index] = 0;
    }
}

// 执行judger
void run_judger(int submission_id, int box_index){
    char str_id[100], str_index[100];
    sprintf(str_id, "%d", submission_id);
    sprintf(str_index, "%d", box_index);
    execl("../judger/judger", "../judger/judger", str_id, str_index, NULL);
}