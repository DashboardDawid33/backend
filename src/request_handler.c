#include "sqlite3.h"
#include "database_utils.h"
#include "cjson/cJSON.h"
#include <stdlib.h>
#include <string.h>
#include "libwebsockets.h"
#include "request_handler.h"

//TODO : Somehow merge the freeing functions.

void free_registration_request(const RegistrationData *data) {
    free(data->username);
    free(data->password);
    free((void *) data);
}

void free_login_request(const LoginData *data) {
    free(data->username);
    free(data->password);
    free((void *) data);
}

void free_err_msg(const ErrMsg *err) {
    if (!err->msg)
        free(err->msg);
    free(err);
}

RegistrationData *
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

    RegistrationData *request = (RegistrationData *) malloc(sizeof(RegistrationData));
    if (!request) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request, 0, sizeof(LoginData));

    int username_len = strlen(username->valuestring);
    request->username = (char *) malloc(username_len);
    if (!request->username) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->username, 0, username_len);
    request->username = cJSON_Print(username);

    int password_len = strlen(password->valuestring);
    request->password = (char *) malloc(strlen(password->valuestring));
    if (!request->password) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->password, 0, password_len);
    request->password = cJSON_Print(password);

    cJSON_Delete(json);
    return request;
}

LoginData *
parse_json_login(const char *json_message) {
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

    LoginData *request = (LoginData *) malloc(sizeof(LoginData));
    if (!request) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request, 0, sizeof(LoginData));

    int username_len = strlen(username->valuestring);
    request->username = (char *) malloc(username_len);
    if (!request->username) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->username, 0, username_len);
    request->username = cJSON_Print(username);

    int password_len = strlen(password->valuestring);
    request->password = (char *) malloc(strlen(password->valuestring));
    if (!request->password) {
        lwsl_err("OOM, exiting...");
        return NULL;
    }
    memset(request->password, 0, password_len);
    request->password = cJSON_Print(password);

    cJSON_Delete(json);
    return request;
}

int registration_handler(char* json_message, ErrorKind *err) {
    RegistrationData *registration_data = NULL;
    if (!(registration_data = parse_json_registration(json_message))) {
        lwsl_user("Cannot parse json registration.\n");
        return 1;
    }

    sqlite3 *db = sql_open("test.db");
    if (!db) {
        free_registration_request(registration_data);
        return 1;
    }

    if (store_login_in_db(db, registration_data->username, registration_data->password)) {
        sqlite3_close(db);
        free_registration_request(registration_data);
        return 1;
    }
    free_registration_request(registration_data);
    sqlite3_close(db);
    return 0;
}

int login_handler(const char* json_message, ErrorKind *err) {
    LoginData *login_data = NULL;
    if ((login_data = parse_json_login(json_message))) {
        lwsl_user("Cannot parse json login.\n");
        return 1;
    }

    sqlite3 *db = sql_open("test.db");
    if (!db) {
        free_login_request(login_data);
        return 1;
    }

    lwsl_user("Attempting to validate user %s : %s\n", login_data->username, login_data->password);
    if (validate_user(db, login_data->username, login_data->password)){
        lwsl_user("Cannot validate user\n");
        free_login_request(login_data);
        sqlite3_close(db);
        return 1;
    } else {

    }
    free_login_request(login_data);
    sqlite3_close(db);
    return 0;
}