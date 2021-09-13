#ifndef BACKEND_LWS_PROTOCOL_H
#define BACKEND_LWS_PROTOCOL_H
#include <stdlib.h>
#include <libwebsockets.h>

#define RING_DEPTH 4096

/* one of these created for each message */

typedef struct Message_t {
    void *payload; /* is malloc'd */
    size_t len;
    char binary;
    char first;
    char final;
} Message;

typedef struct SessionData_t {
    struct lws_ring *ring;
    uint32_t msglen;
    uint32_t tail;
    uint8_t completed:1;
    uint8_t flow_controlled:1;
    uint8_t write_consume_pending:1;
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

#endif //BACKEND_LWS_PROTOCOL_H
