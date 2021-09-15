#ifndef BACKEND_LWS_PROTOCOL_H
#define BACKEND_LWS_PROTOCOL_H
#include <stdlib.h>
#include <libwebsockets.h>

#define RING_DEPTH 4096

/* one of these created for each message */
typedef struct lws_dll2 lws_dll2;

typedef struct Packet_t {
    void *payload; /* is malloc'd */
    size_t len;
    char binary;
    char first;
    char final;
    lws_dll2 list;
} Packet;

typedef struct Response_t {
    char *message;
}Response;

typedef struct SessionData_t {
    char *message;
    int message_len;
    char *response_message;
    uint32_t message_count;
} SessionData;

typedef struct VhostData_t {
    struct lws_context *context;
    struct lws_vhost *vhost;

    int *interrupted;
    int *options;
} VhostData;

int
handle_connection(struct lws *connection_info, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len);
int initialize_connection(struct lws *connection_info, VhostData *vhost_data, void *in);

#endif //BACKEND_LWS_PROTOCOL_H
