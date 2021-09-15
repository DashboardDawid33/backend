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

//TODO : Somehow merge the parsing functions.

/**
 * @param Json to be parsed
 * @return Return value needs to be freed by caller.
 * @author Dawid Sobczak
 **/
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

/**
 * @param Json to be parsed
 * @return Return value needs to be freed by caller.
 * @author Dawid Sobczak
 **/
static RegistrationRequest *
parse_json_login(const char *json_message) {

    return 1;
}

int registration_handler(const char *json_message) {
    RegistrationRequest *request = NULL;
    if (!(request = parse_json_registration(json_message))) {
        return 1;
    }
    sqlite3 *db = sql_open("test.db");
    if (!db) {
        free_login_request(request);
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
    LoginRequest *request = NULL;
    if (!(request = parse_json_login(json_message))) {
        return 1;
    }
    sqlite3 *db = sql_open("test.db");
    if (!db) {

        free_login_request(request);
        return 1;
    }
    free_login_request(request);
    sqlite3_close(db);
    return 0;
    return 0;
}