#include "sqlite3.h"
#include "database_connection.h"

int login_handler(const char *json_message) {
    sqlite3 *db = sql_open("test.db");
}