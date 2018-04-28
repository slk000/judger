#include "db.h"

MYSQL *gDB_CONN = NULL;
const unsigned int gDB_TIMEOUT = 30;
const char *gDB_HOST = "localhost";
const char *gDB_USER = "root";
const char *gDB_PASSWD = "";
const char *gDB_NAME = "oj";
const unsigned int gDB_PORT = 3306;
MYSQL_RES *DB_RES;
MYSQL_ROW DB_ROW;
// int main(){

//     return 0;
// }

int connect_mysql(){
    if (gDB_CONN) return 1;
    gDB_CONN = mysql_init(NULL);
    mysql_options(gDB_CONN, MYSQL_OPT_CONNECT_TIMEOUT, &gDB_TIMEOUT);

    if (mysql_real_connect(
        gDB_CONN, gDB_HOST, gDB_USER, gDB_PASSWD, gDB_NAME, 
        gDB_PORT, NULL, 0)){
        printf("%s", mysql_error(gDB_CONN));
        return 1;
    }
    else {
        printf("connect fail: %s\n", mysql_error(gDB_CONN));
        return 0;
    }
}
void close_mysql(void){
    mysql_close(gDB_CONN);
}
int execute_sql(const char *sql){
    uint sql_length = strlen(sql);
    if (0 == mysql_real_query(gDB_CONN, sql, sql_length)) { 
        // query查询成功返回0
        printf("query %s success\n", sql);
        return 1;
    }
    else {
        printf("query %s fail: \n", sql, mysql_error(gDB_CONN));
        return 0;
    }
}