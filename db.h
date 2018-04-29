#ifndef JUDGER_DB_H
#define JUDGER_DB_H

#include <stdio.h>
#include <string.h>

#include <mysql.h>

// https://blog.csdn.net/mantis_1984/article/details/53571758
extern MYSQL *gDB_CONN;
extern const unsigned int gDB_TIMEOUT;
extern const char *gDB_HOST;
extern const char *gDB_USER;
extern const char *gDB_PASSWD;
extern const char *gDB_NAME;
extern const unsigned int gDB_PORT;
extern MYSQL_RES *DB_RES;
extern MYSQL_ROW DB_ROW;


int connect_mysql(void);
void close_mysql(void);
int execute_sql(const char *sql);

#endif