#include "lws.h"

int main(int argc, const char **argv)
{
    return lws_start(argc, argv);
}

/*
cJSON *json = cJSON_Parse(upload_data);
cJSON *name = cJSON_GetObjectItemCaseSensitive(json,"user");
cJSON *password = cJSON_GetObjectItemCaseSensitive(json, "password");
char *sql = (char*)malloc(128);
snprintf(sql,128, "insert into users (name,password) values(\"%s\", \"%s\");",name->valuestring,password->valuestring);

sqlite3 *db = sql_open("test.db");
sql_exec(db,sql,NULL,NULL);

sqlite3_close(db);
cJSON_Delete(json);
 */