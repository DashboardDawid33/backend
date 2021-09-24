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
    //TODO: Generate UUID. https://stackoverflow.com/questions/51053568/generating-a-random-uuid-in-c
    char *sql_const = "INSERT INTO USERS (UUID ,USERNAME, PASSWORD)"
                      "VALUES(1, %s, %s);";
    const int sql_const_length = strlen(sql_const);
    const int total_length = sql_const_length + strlen(username) + strlen(password);
    char *sql = (char *) malloc(total_length);
    snprintf(sql, total_length, sql_const, username, password);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL)) {
        free(sql);
        return 1;
    }
    free(sql);
    return 0;
}

static int get_password(void *callback_var, int argc, char **argv, char **azColName) {
    if (argc > 0) {
        if (argc > 1)
            lwsl_err("More than one password returned in sql request.\n");

        strcpy((char*) callback_var, argv[0]);
    } else {
        lwsl_user("Password does not exist in database\n");
    }
    return 0;
}

int validate_user(sqlite3 *db, const char *username, const char *password) {
    //TODO: Sanitise SQL.
    const char *query =
            "SELECT (PASSWORD) "
            "FROM USERS "
            "WHERE USERNAME=%s";
    const int sql_const_length = 54;
    const int total_length = sql_const_length + strlen(username) + strlen(password);
    char *sql = (char *) malloc(total_length);
    snprintf(sql, total_length, query, username);

    //TODO: Figure out a way of receiving variable length passwords (probably with char**).
    char *db_password = malloc(100);
    if (sqlite3_exec(db, sql, &get_password, db_password, NULL)) {
        lwsl_user("SQL query and callback failed.\n");
        free(sql);
        return 1;
    } else {
        if (!db_password) {
            free(sql);
            return 1;
        }
    }

    //TODO: Find better way for removing quotes from password.
    int len = strlen(password);
    char *pass = malloc(len);
    memcpy(pass, password, len - 1);
    pass[len - 1] = '\0';
    lwsl_user("Is %s == %s ?\n", pass + 1, db_password);
    if(strcmp(pass + 1, db_password) == 0)
        lwsl_user("CORRECT PASSWORD\n");
    else
        lwsl_user("WRONG PASSWORD\n");
    free(pass);


    free(db_password);
    return 0;
}
