#include "database_utils.h"
#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>
#include "cjson/cJSON.h"
#include "libwebsockets.h"

sqlite3 *sql_open(const char *filename) {
    sqlite3 *db;
    if (sqlite3_open("test.db", &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

int store_login_in_db(sqlite3 *db, const char *username, const char *password) {
    char *sql_const = "INSERT INTO USERS (USERNAME, PASSWORD)"
                      "VALUES(%s, %s);";
    const int sql_const_length = 50; // This is excluding formatting char's like %s
    const int total_length = sql_const_length + strlen(username) + strlen(password);
    char *sql = (char *) malloc(total_length);
    snprintf(sql, total_length, sql_const, username, password);

    if (sqlite3_exec(db, sql, callback, NULL, NULL)) {
        free(sql);
        return 1;
    }
    free(sql);
    return 0;
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

static int get_password(void *callback_var, int argc, char **argv, char **azColName) {
    int len = strlen(argv[0]);
    callback_var = (char*) malloc(len);
    strcpy((char *) callback_var, argv[0]);
    lwsl_user("Password in database : %s\n", argv[0]);
    return 0;
}

int validate_user(sqlite3 *db, const char *username, const char *password) {
    const char *query =
            "SELECT (PASSWORD) "
            "FROM USERS "
            "WHERE USERNAME=%s";
    const int sql_const_length = 54;
    const int total_length = sql_const_length + strlen(username) + strlen(password);
    char *sql = (char *) malloc(total_length);
    snprintf(sql, total_length, query, username);

    void *db_password = NULL;
    //TODO: Figure out why db_password does not get passed to the callback function.
    if (sqlite3_exec(db, sql, &get_password, db_password, NULL)) {
        lwsl_user("SQL query and callback failed.\n");
        free(sql);
        return 1;
    } else {
        lwsl_user("Password in database : %s\n", db_password);
    }

    //Technically not necessary, just a precaution.
    if (!db_password) {
        free(sql);
        return 1;
    }


    free(db_password);
    return 0;
}
