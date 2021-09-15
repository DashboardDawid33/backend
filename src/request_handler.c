#include "sqlite3.h"
#include "database_connection.h"
#include "api.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include "libwebsockets.h"

static void free_login_request(RegistrationRequest *req) {
    free(req->username);
    free(req->password);
    free(req);
}

/*
 * Return value needs to be freed by caller.
 */
static RegistrationRequest *
parse_json_registration(const char *json_message) {
    lwsl_user("%s\n", json_message);
    cJSON *json = cJSON_Parse(json_message);
    if (!json) {
        return NULL;
    }
    cJSON *username = cJSON_GetObjectItem(json, USERNAME_FIELD);
    cJSON *password = cJSON_GetObjectItem(json, PASSWORD_FIELD);
    if (!username || !password) {
        cJSON_Delete(json);
        return NULL;
    }

    RegistrationRequest *request = (RegistrationRequest *)malloc(sizeof(LoginRequest));
    if (!request) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request, 0, sizeof(LoginRequest));

    int username_len = strlen(username->valuestring);
    request->username = (char*)malloc(username_len);
    if (!request->username) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->username, 0, username_len);
    request->username = cJSON_Print(username);

    int password_len = strlen(password->valuestring);
    request->password = (char*)malloc(strlen(password->valuestring));
    if (!request->password) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->password, 0, password_len);
    request->password = cJSON_Print(password);

    cJSON_Delete(json);
    return request;
}


int registration_handler(const char *json_message) {
    sqlite3 *db = sql_open("test.db");
    if (!db) {
        return 1;
    }
    RegistrationRequest *request = NULL;
    if (!(request = parse_json_registration(json_message))) {
        sqlite3_close(db);
        return 1;
    }
    if (store_login_in_db(db,request->username,request->password)){
        sqlite3_close(db);
        free_login_request(request);
        return 1;
    }
    free_login_request(request);
    sqlite3_close(db);
    return 0;
}

int login_handler(const char *json_message) {
    sqlite3 *db = sql_open("test.db");
    return 0;
}