#ifndef BACKEND_REQUEST_HANDLER_H
#define BACKEND_REQUEST_HANDLER_H
#define REQUEST_FIELD "request_type"
#define LOGIN_REQUEST_VALUE "LOGIN"
#define REGISTRATION_REQUEST_VALUE "REGISTER"
#define USERNAME_FIELD "username"
#define PASSWORD_FIELD "password"

typedef enum RequestType {LOGIN_REQUEST, REGISTRATION_REQUEST, UNDEFINED_REQUEST} RequestType;

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

/**
 * @param json_message Json to be parsed
 * @return Non-zero return on failure.
 * @authors Dawid Sobczak
 **/
int login_handler(const LoginData *request);

/**
 * @param request Registration request to process.
 * @return Non-zero return on failure.
 * @authors Dawid Sobczak
 */
int registration_handler(const RegistrationData *request);

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
#endif //BACKEND_REQUEST_HANDLER_H
