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
        exit(1);
    }
    get_submission_info(&submission);
    

    if(!compile(&submission)){
        submission.judge_result = JR_COMPERR;
        update_compile_info(&submission);
        goto judge_done;
    }

    test_cases(&submission);
    if (JR_RUNERR == submission.judge_result){
        update_run_info(&submission);
        goto judge_done;
    }
    

judge_done:
    update_submission(&submission);
    printf("submission %d done\n", submission.id);
    System("rm %s/*", box_dir);
    exit(1);
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
            s->used_memory/1024,
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

int update_run_info(const struct Submission *s){
    // 删除已有的信息
    sprintf(sql, QUERY_DELETE_RUN_INFO, s->id);
    execute_sql(sql);

    // 先将查询里的id填好，后面的%%s变成%s供下面填信息用
    sprintf(sql, QUERY_INSERT_RUN_INFO, s->id);

    FILE *fp = fopen("error.out", "r");
    char run_info[BUFFER_SIZE << 7], *end = run_info;
    char escaped_info[BUFFER_SIZE << 8];
    char long_sql[BUFFER_SIZE << 7];
    // 限制读取信息的大小
    while (end-run_info <= (BUFFER_SIZE << 6) && 
           fgets(end, BUFFER_SIZE << 1, fp) ){
        end += strlen(end);
    }
    fclose(fp);

    mysql_real_escape_string(gDB_CONN, escaped_info, run_info, end-run_info);

    sprintf(long_sql, sql, escaped_info);
    execute_sql(long_sql);
}

int prepare_run_env(int lang){

    return 1;
}

int prepare_test_files(const char *case_dir, const char *case_name){
    char file_path[BUFFER_SIZE];
    sprintf(file_path, "%s/%s", case_dir, case_name);
    int len = strlen(file_path);
    file_path[len-3] = '\0';

    int ret = 1;
    ret &= System("cp '%s.in' data.in", file_path);
    ret &= System("cp '%s.out' data.out", file_path);

    return ret;
}

int test_cases(struct Submission *s){
    s->judge_result = JR_RUN;
    update_submission(s);

    int allowed_syscalls[BUFFER_SIZE];
    memset(allowed_syscalls, 0, sizeof(allowed_syscalls));
    char str_case_dir[BUFFER_SIZE];
    sprintf(str_case_dir, "/home/judge/data/%d", s->problem_id);
    DIR *case_dir = opendir(str_case_dir);
    if (!case_dir) return 0;
    dirent *dir_ent;

    s->judge_result = JR_ACCEPT;

    int case_cnt = 0;
    for (; JR_ACCEPT == s->judge_result && 
          (dir_ent = readdir(case_dir)); case_cnt++){
        if (!is_file_ext(dir_ent->d_name, "in")) continue;
        printf("%s\n", dir_ent->d_name);
        prepare_test_files(str_case_dir, dir_ent->d_name);
        get_allowed_syscalls(s->code_lang, allowed_syscalls);

        pid_t user_pid = fork();
        if (0 == user_pid){
            run_user_program(s);
        }
        else{
            watch_user_program(s, user_pid, allowed_syscalls);
            if (s->used_time > s->time_limit*1000){
                s->judge_result = JR_TIMELIM;
            }
            else if (s->used_memory > s->memory_limit * 1048576){
                s->judge_result = JR_MEMLIM;
            }
            else if (JR_ACCEPT == s->judge_result){
                if (s->is_spj){}
                else{
                    s->judge_result = compare_file("user.out", "data.out");
                }
            }
        }

    }
    if (JR_RUNERR == s->judge_result){
        // update run error info
    }
    // else 
    closedir(case_dir);
    update_submission(s);
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

int get_allowed_syscalls(int lang, int *arr){
    int *call = &CALLS[lang][0];
    
    for (int *call=&CALLS[lang][0]; (*call)>=0; call++){
        // printf("allowed %d\n", *call);
        arr[*call] = 1;
    }
    return 1;
}

int run_user_program(struct Submission *s){
	ptrace(PTRACE_TRACEME, 0, NULL, NULL);

    freopen("data.in", "r", stdin);
    freopen("user.out", "w", stdout);
    freopen("error.out", "a+", stderr);
    
    struct rlimit limit;
    limit.rlim_cur = limit.rlim_max = s->time_limit/1000 + 1;
    setrlimit(RLIMIT_CPU, &limit);

    limit.rlim_cur = 1048576*s->memory_limit/2 * 3;
    limit.rlim_max = 1048576*s->memory_limit * 2;
    setrlimit(RLIMIT_AS, &limit);

    limit.rlim_cur = limit.rlim_max = 1048576 << 6;
    setrlimit(RLIMIT_STACK, &limit);

    limit.rlim_cur = 1048576<<5;
    limit.rlim_max = 1048576<<5 + 1048576;
    setrlimit(RLIMIT_FSIZE, &limit);
    // TODO: other lang
    alarm(0);
	alarm(s->time_limit * 10);

    execl("./Main", "./Main", NULL);
    exit(0);
}

int watch_user_program(struct Submission *s, pid_t pid, const int *allowed_syscalls){
    int peak_memory = -1;
    int current_peak_memory;
    int status;


    struct user_regs_struct reg;
    struct rusage ruse;

    while(true){
        wait4(pid, &status, 0, &ruse);
        current_peak_memory = get_proc_status(pid, "VmPeak:") << 10; // KB to B

        if (current_peak_memory > peak_memory){
            peak_memory = current_peak_memory;
        }
        s->used_memory = peak_memory;///1024;
        if (peak_memory > s->memory_limit * 1048576){
            s->judge_result = JR_MEMLIM;
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }

        if (WIFEXITED(status)){// 如果子进程正常退出，返回一个非零值
            break; 
        }
        if (/*lang*/ get_file_size("error.out")){
            s->judge_result = JR_RUNERR;
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }
        if (!s->is_spj && 
            get_file_size("user.out")> get_file_size("data.out") * 2 + 1024){
            s->judge_result = JR_OUTPUTLIM;
            ptrace(PTRACE_KILL, pid, NULL, NULL);
            break;
        }
        int exit_code = WEXITSTATUS(status); // 提取子进程正常退出时的返回值
        if ( (s->code_lang >= 3 && exit_code == 17) ||
             exit_code == 5 || 
             exit_code == 0){
			//continue
        }
		else {
            if (JR_ACCEPT == s->judge_result){
                switch (exit_code){
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        s->judge_result = JR_TIMELIM;
                        break;
                    case SIGXFSZ:
                        s->judge_result = JR_OUTPUTLIM;
                        break;
                    default:
                        s->judge_result = JR_RUNERR;

                }
                rtlog(strsignal(exit_code));
            }
            ptrace(PTRACE_KILL, pid, NULL, NULL);
			break;
        }
        if (WIFSIGNALED(status)){ // 若子进程异常terminate
            int exit_signal = WTERMSIG(status); // 获取使其terminate的信号编号
            if (JR_ACCEPT == s->judge_result){
                switch (exit_signal){
                    case SIGCHLD:
                    case SIGALRM:
                        alarm(0);
                    case SIGKILL:
                    case SIGXCPU:
                        s->judge_result = JR_TIMELIM;
                        break;
                    case SIGXFSZ:
                        s->judge_result = JR_OUTPUTLIM;
                        break;
                    default:
                        s->judge_result = JR_RUNERR;
                }
                rtlog(strsignal(exit_signal));
            }
            break;
        }

        // 检查系统调用
        ptrace(PTRACE_GETREGS, pid, NULL, &reg);
        printf("call: %d\n", reg.SYSCALL_REG);
        if (!allowed_syscalls[reg.SYSCALL_REG]){
            s->judge_result = JR_RUNERR;
            char error[BUFFER_SIZE];
            sprintf(error, "NOT ALLOWED SYSCALL %d\n", reg.SYSCALL_REG);
            rtlog(error);
            ptrace(PTRACE_KILL, pid, NULL, NULL);
        }
        // 4023
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }
    s->used_time = (ruse.ru_utime.tv_sec * 1000 + ruse.ru_utime.tv_usec / 1000)
            +(ruse.ru_stime.tv_sec * 1000 + ruse.ru_stime.tv_usec / 1000);
    return 1;
}
