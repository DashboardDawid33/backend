#include "lws_protocol.h"
#include <string.h>
#include "libwebsockets.h"
#include "cjson/cJSON.h"
#include "sqlite3.h"
#include "database_connection.h"
#include "api.h"
#include <assert.h>
#include "util.h"
#include "request_handler.h"

static void
free_message(void *_msg)
{
    RawMessage *msg = _msg;

    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

RequestType
get_request_type(const char *incoming_data) {
    cJSON *json = cJSON_Parse(incoming_data);
    if(!json) {
        return UNDEFINED;
    }
    cJSON *request_type = cJSON_GetObjectItemCaseSensitive(json, REQUEST_TYPE_JSON);
    if(!request_type) {
        cJSON_Delete(json);
        return UNDEFINED;
    }

    RequestType request = UNDEFINED;
    if(strcmp(request_type->valuestring, REQUEST_TYPE_LOGIN_JSON) == 0) {
        request = LOGIN_REQUEST;
    } else if (strcmp(request_type->valuestring, REQUEST_TYPE_REGISTRATION_JSON) == 0) {
        request = REGISTRATION_REQUEST;
    }
    return request;
}

int
handle_connection(struct lws *connection_info, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len)
{
    SessionData *session_data = (SessionData *)user;
    struct lws_vhost *vhost = lws_get_vhost(connection_info);
    const struct lws_protocols *protocols = lws_get_protocol(connection_info);
    VhostData *vhost_data = (VhostData*)lws_protocol_vh_priv_get(vhost, protocols);

    const Response *response_message;
    RawMessage incoming_message;
    int m, n, flags;

    switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
            initialize_connection(connection_info, vhost_data, in);
            break;

        case LWS_CALLBACK_ESTABLISHED:
            lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
            session_data->ring = lws_ring_create(sizeof(RawMessage), RING_DEPTH,
                                        free_message);
            if (!session_data->ring)
                return 1;
            session_data->tail = 0;
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:

            lwsl_user("LWS_CALLBACK_SERVER_WRITEABLE\n");

            if (session_data->write_consume_pending) {
                /* perform the deferred fifo consume */
                lws_ring_consume_single_tail(session_data->ring, &session_data->tail, 1);
                session_data->write_consume_pending = 0;
            }

            response_message = lws_ring_get_element(session_data->ring, &session_data->tail);
            if (!response_message) {
                lwsl_user(" (nothing in ring)\n");
                break;
            }

            flags = lws_write_ws_flags(
                    response_message->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
                    response_message->first, response_message->final);


            m = lws_write(connection_info, ((unsigned char *)response_message->payload) +
                               LWS_PRE, response_message->len, (enum lws_write_protocol)flags);
            if (m < (int)response_message->len) {
                lwsl_err("ERROR %d writing to ws socket\n", m);
                return -1;
            }

            lwsl_user(" wrote %d: flags: 0x%x first: %d final %d\n",
                      m, flags, response_message->first, response_message->final);

            break;

        case LWS_CALLBACK_RECEIVE:

            lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
                      "last %d, bin %d, msglen %d (+ %d = %d))\n",
                      (int)len, (int)lws_remaining_packet_payload(connection_info),
                      lws_is_first_fragment(connection_info),
                      lws_is_final_fragment(connection_info),
                      lws_frame_is_binary(connection_info), session_data->msglen, (int)len,
                      (int)session_data->msglen + (int)len);

            incoming_message.first = (char)lws_is_first_fragment(connection_info);
            incoming_message.final = (char)lws_is_final_fragment(connection_info);
            incoming_message.binary = (char)lws_frame_is_binary(connection_info);

            n = (int)lws_ring_get_count_free_elements(session_data->ring);
            if (!n) {
                lwsl_user("dropping!\n");
                break;
            }

            if (incoming_message.final)
                session_data->msglen = 0;
            else
                session_data->msglen += (uint32_t)len;

            incoming_message.len = len;
            /* notice we over-allocate by LWS_PRE */

            incoming_message.payload = malloc(LWS_PRE + len);
            if (!incoming_message.payload) {
                lwsl_user("OOM: dropping\n");
                break;
            }

            memcpy((char *)incoming_message.payload + LWS_PRE, in, len);
            const char* message = (char*) (incoming_message.payload + LWS_PRE);

            RequestType request = get_request_type(message);
            if (request == LOGIN_REQUEST) {
                if(login_handler(message)){
                    lwsl_user("Error handling login.");
                }
            } else if (request == REGISTRATION_REQUEST) {
                if(registration_handler(message)) {
                    lwsl_user("Error handling registration.");
                }
            }

            if (!lws_ring_insert(session_data->ring, &incoming_message, 1)) {
                free_message(&incoming_message);
                lwsl_user("dropping!\n");
                break;
            }
            lws_callback_on_writable(connection_info);

            break;

        case LWS_CALLBACK_CLOSED:
            lwsl_user("LWS_CALLBACK_CLOSED\n");
            lws_ring_destroy(session_data->ring);

            if (!*vhost_data->interrupted)
                *vhost_data->interrupted = 1 + session_data->completed;

            lws_cancel_service(lws_get_context(connection_info));
            break;

        default:
            break;
    }

    return 0;
}