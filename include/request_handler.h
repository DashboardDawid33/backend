#ifndef BACKEND_REQUEST_HANDLER_H
#define BACKEND_REQUEST_HANDLER_H
#define REQUEST_FIELD "request_type"
#define LOGIN_REQUEST_VALUE "LOGIN"
#define REGISTRATION_REQUEST_VALUE "REGISTER"
#define USERNAME_FIELD "username"
#define PASSWORD_FIELD "password"

typedef enum RequestType {LOGIN_REQUEST, REGISTRATION_REQUEST, UNDEFINED_REQUEST} RequestType;
typedef enum ErrorKind { ERR_FAILED_TO_PARSE_JSON, ERR_NONE } ErrorKind;

typedef struct LoginData_t {
    char *username;
    char *password;
    int is_authenticated;
    char *response;
}LoginData;

typedef struct RegistrationData_t {
    char *username;
    char *password;
    int successful;
    char *response;
}RegistrationData;

typedef struct ErrMsg {
    char* msg;
    ErrorKind kind;
} ErrMsg;

/**
 * @param json_message Json to be parsed
 * @param err Error message. err.kind == NONE on success.
 * @return Non-zero return on failure.
 * @authors Dawid Sobczak
 **/
int login_handler(const char* json_message, ErrorKind *err);

/**
 * @param request Registration request to process.
 * @return Non-zero return on failure.
 * @authors Dawid Sobczak
 */
int registration_handler(char* json_message, ErrorKind *err);

//TODO : Somehow merge the parsing functions.
/**
 * @param Json to be parsed
 * @return Return value needs to be freed by caller.
 * @author Dawid Sobczak
 **/
RegistrationData *
parse_json_registration(const char *json_message);

/**
 * @param Json to be parsed
 * @return Return value needs to be freed by caller.
 * @author Dawid Sobczak
 **/
LoginData *
parse_json_login(const char *json_message);

//TODO : Somehow merge the freeing functions.
void free_registration_request(const RegistrationData *req);
void free_login_request(const LoginData *req);
void free_err_msg(const ErrMsg *err);
#endif //BACKEND_REQUEST_HANDLER_H
