#ifndef BACKEND_DATABASE_UTILS_H
#define BACKEND_DATABASE_UTILS_H
#include <sqlite3.h>

/**
 *
 * @param filename : Name of the database file to open.
 * @return Must be destroyed with sqlite3_close() after use. Returns null on error.
 */
sqlite3* sql_open(const char* filename);

/**
 * @param db database to insert data into.
 * @param username
 * @param password
 * @return Return non-zero value on error.
 */
int store_login_in_db(sqlite3* db, const char* username, const char* password);
/**
 * @param db database to insert data into.
 * @param username
 * @param password
 * @return Return non-zero value on error / validation failure.
 */
int validate_user(sqlite3* db, const char* username, const char* password);

#endif //BACKEND_DATABASE_UTILS_H
