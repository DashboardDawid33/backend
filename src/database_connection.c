#include "database_connection.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include "cjson/cJSON.h"
#include "libwebsockets.h"

/**
 *
 * @param filename : Name of the database file to open.
 * @return Must be destroyed with sqlite3_close() after use. Returns null on error.
 */
sqlite3* sql_open(const char* filename) {
    sqlite3* db;
    if(sqlite3_open("test.db", &db)){
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

/**
 * \brief Runs SQL query against a database.
 *
 * @param db : sqlite database to query.
 * @param sql : sql query to run.
 * @param callback : to run the results of the query against.
 * @param first : variable of the callback.
 *
 * The callback function :
 * @param void* : callback variable passed as the last variable in sql_exec().
 * @param int : amount of columns.
 * @param char** : array of strings of the values stored in the db.
 * @param char** : array of the column names.
 *
 * @return Returns non-zero value on error.
 */
int sql_exec(sqlite3* db, char* sql, int (*callback)(void *, int, char **, char **), void *callback_var) {
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, callback_var, &zErrMsg);
    if(rc != SQLITE_OK){
        return 1;
    } else {
        return 0;
    }
}

/**
 * @param db database to insert data into.
 * @param username
 * @param password
 * @return return s
 */
int store_login_in_db(sqlite3* db, const char* username, const char* password) {
    char *sql_const = "INSERT INTO USERS (USERNAME, PASSWORD) VALUES(%s, %s);";
    const int sql_const_length = 50; // This is excluding formatting char's like %s
    const int total_length = sql_const_length + strlen(username) + strlen(password);

    char *sql = (char*)malloc(total_length);
    snprintf(sql, total_length, sql_const, username, password);
    sql_exec(db,sql,NULL,NULL);
    free(sql);
    return 0;
}

int check_if_user_exists(sqlite3* db, const char* username, const char* password) {
    char *sql = (char*)malloc(256);
    lwsl_user("%s, %s\n", username, password);
    snprintf(sql,256, "insert into users (username,password) values(%s, %s);", username, password);
}
