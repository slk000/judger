#include "judger.h"

struct Submission submission = {0, 0, 0, 0, -1, 0, -1, 0, CL_Other, JR_OTHER};
char sql[BUFFER_SIZE];
char box_dir[BUFFER_SIZE];
int main(int argc, char *argv[]){
    if (argc < 3) exit(1);

    submission.id = atoi(argv[1]);
    submission.box_id = atoi(argv[2]);

    sprintf(box_dir, "/home/judge/run%d", submission.box_id);
    setuid(1001);
    setgid(1001);
    setresuid(1001, 1001, 1001);
    chdir(box_dir);

    printf("judge submission %d at box %d\n", submission.id, submission.box_id);
    if (!connect_mysql()){
        printf("connect db failed\n");
        exit(0);
    }
    get_submission_info(&submission);
    

    if(!compile(&submission)){
        submission.judge_result = JR_COMPERR;
        update_compile_info(&submission);
        goto judge_done;
    }



judge_done:
    update_submission(&submission);
    sleep(5);
    printf("submission %d done\n", submission.id);
    return 0;
}

int get_submission_info(struct Submission *s){
    sprintf(sql, QUERY_SUBMISSION_INFO, s->id);
    if (execute_sql(sql) && 
        (DB_ROW = mysql_fetch_row(DB_RES)) ){
        
        s->problem_id = atoi(DB_ROW[0]);
        s->time_limit = atoi(DB_ROW[1]);
        s->memory_limit = atoi(DB_ROW[2]);
        s->is_spj = ('1' == DB_ROW[3][0]);
        s->code_lang = (enum Code_Lang)atoi(DB_ROW[4]);

        create_source_file(DB_ROW[5], s->code_lang);
        mysql_free_result(DB_RES);
        
        return 1;
    }
    else{
        printf("get info failed\n");
    }
    return 0;
}

int update_submission(const struct Submission *s){
    char time_buffer[BUFFER_SIZE];
    time_t timep;  
    struct tm *p_tm;  
    timep = time(NULL);
    p_tm = gmtime(&timep);  // UTC时间
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", p_tm);  
    sprintf(sql, QUERY_UPDATE_SUBMISSION, 
            s->judge_result, 
            s->used_time,
            s->used_memory,
            time_buffer,
            s->id);
    return execute_sql(sql);
}

int create_source_file(const char *code, enum Code_Lang lang){
    char filename[BUFFER_SIZE];
    sprintf(filename, "Main.%s", LANG_EXT[lang]);
    FILE *fp = fopen(filename, "w");
    if (!fp){
        printf("create file failed\n");
        return 0;
    }
    fprintf(fp, "%s", code);
    fclose(fp);
    return 1;
}

int compile(struct Submission *s){
    s->judge_result = JR_COMPILE;
    update_submission(s);
    pid_t pid = fork();
    if (0 == pid){
        // set_limit();
        freopen("compile_info.txt", "w", stderr);
        // TODO: other lang
        // freopen("compile_info.txt", "w", stdout);
        const char *compiler = CP_CMD[s->code_lang][0];
        const char **args = CP_CMD[s->code_lang];
        execvp(compiler, (char * const *)args);
    }
    else{
        int status = 0;
        waitpid(pid, &status, 0);
        // TODO: other lang
        printf("compile status :%d\n", status);
        return !status;
    }
}


int update_compile_info(const struct Submission *s){
    // 删除已有的信息
    sprintf(sql, QUERY_DELETE_COMPILE_INFO, s->id);
    execute_sql(sql);

    // 先将查询里的id填好，后面的%%s变成%s供下面填信息用
    sprintf(sql, QUERY_INSERT_COMPILE_INFO, s->id);

    FILE *fp = fopen("compile_info.txt", "r");
    char compile_info[BUFFER_SIZE << 7], *end = compile_info;
    char escaped_info[BUFFER_SIZE << 8];
    char long_sql[BUFFER_SIZE << 7];
    // 限制读取信息的大小
    while (end-compile_info <= (BUFFER_SIZE << 6) && 
           fgets(end, BUFFER_SIZE << 1, fp) ){
        end += strlen(end);
    }
    fclose(fp);

    mysql_real_escape_string(gDB_CONN, escaped_info, compile_info, end-compile_info);

    sprintf(long_sql, sql, escaped_info);
    execute_sql(long_sql);
}

int prepare_run_env(int lang){

    return 1;
}

int prepare_test_files(struct Submission *s){

}

int test_cases(struct Submission *s){
    char str_case_dir[BUFFER_SIZE];
    sprintf(str_case_dir, "/home/judge/data/%d", s->problem_id);
    DIR *case_dir = opendir(str_case_dir);
    if (!case_dir) return 0;
    dirent *dir_ent;

    s->judge_result = JR_ACCEPT;

    while(JR_ACCEPT == s->judge_result && 
          (dir_ent = readdir(case_dir)) ){
        if (!is_file_ext(dir_ent->d_name, "in")) continue;
        
    }
}

int is_file_ext(const char *filename, const char *ext){
    int filename_len = strlen(filename);
    int ext_len = strlen(ext);
    if (filename_len <= ext_len) return 0;
    // char *extp = s+filename_len-1;
    // for(;extp >= s && *expt != '.';expt--) ;
    char *extp = (char *)filename + filename_len - ext_len;
    if (extp-1<filename || '.' != *(extp-1)) return 0;
    return strcmp(extp, ext) == 0;
}