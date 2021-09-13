#include "database_connection.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

sqlite3* sql_open(const char* filename) {
    sqlite3* db;
    if(sqlite3_open("test.db", &db)){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

/*
 * Crashes if sql query doesn't succeed.
 *
 * first arg = callback variable
 * second arg = amount of columns
 * third arg = array of strings of the values stored in the db
 * fourth arg = array of the column names.
 */
void sql_exec(sqlite3* db, char* sql, int (*callback)(void *, int, char **, char **), void *callback_var) {
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, callback_var, &zErrMsg);
    if(rc != SQLITE_OK){
        exit(1);
    }
}
