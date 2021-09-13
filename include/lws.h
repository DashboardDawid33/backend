#ifndef BACKEND_LWS_H
#define BACKEND_LWS_H
int lws_start(const int argc, const char **argv);
void sigint_handler(int sig);
#endif //BACKEND_LWS_H
