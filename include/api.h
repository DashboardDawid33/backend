#ifndef BACKEND_API_H
#define BACKEND_API_H
#define REQUEST_TYPE_JSON "request_type"
#define REQUEST_TYPE_REGISTRATION_JSON "register"
#define REQUEST_TYPE_LOGIN_JSON "login"
#define USERNAME_JSON "username"
#define PASSWORD_JSON "password"

typedef enum RequestType {LOGIN_REQUEST, REGISTRATION_REQUEST, UNDEFINED} RequestType;

struct LoginRequest {
    char *username;
    char *password;
};
struct LoginResponse {
    int is_authenticated;
    char *response;
};
#endif //BACKEND_API_H
