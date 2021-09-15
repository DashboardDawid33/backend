#ifndef BACKEND_API_H
#define BACKEND_API_H
#define REQUEST_FIELD "request_type"
#define LOGIN_REQUEST_VALUE "LOGIN"
#define REGISTRATION_REQUEST_VALUE "REGISTER"
#define USERNAME_FIELD "username"
#define PASSWORD_FIELD "password"

typedef enum RequestType {LOGIN_REQUEST, REGISTRATION_REQUEST, UNDEFINED} RequestType;

typedef struct LoginRequest_t {
    char *username;
    char *password;
}LoginRequest;
typedef struct LoginResponse {
    int is_authenticated;
    char *response;
}LoginResponse;

typedef struct RegistrationRequest_t {
    char *username;
    char *password;
}RegistrationRequest;
typedef struct RegistrationResponse_t {
    int successful;
    char *response;
}RegistrationResponse;
#endif //BACKEND_API_H
