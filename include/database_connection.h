#ifndef BACKEND_DATABASE_CONNECTION_H
#define BACKEND_DATABASE_CONNECTION_H
#include <sqlite3.h>

sqlite3* sql_open(const char* filename);
void sql_exec(sqlite3 *db,
          char *sql,
          int (*callback)(void*,int,char**,char**),
          void *);

#endif //BACKEND_DATABASE_CONNECTION_H
