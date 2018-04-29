#include "judger.h"

struct Submission submission = {0, 0, 0, 0, CL_C, 0,};
char sql[BUFFER_SIZE];
char box_dir[BUFFER_SIZE];
int main(int argc, char *argv[]){
    if (argc < 3) exit(1);

    submission.id = atoi(argv[1]);
    submission.box_id = atoi(argv[2]);

    sprintf(box_dir, "/home/judge/run%d", submission.box_id);
    setuid(1001);
    chdir(box_dir);

    printf("judge submission %d at box %d\n", submission.id, submission.box_id);
    if (!connect_mysql()){
        printf("connect db failed\n");
        exit(0);
    }
    get_submission_info(&submission);

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