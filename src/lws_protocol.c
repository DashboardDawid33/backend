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
    Packet *msg = _msg;

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
    cJSON *request_type = cJSON_GetObjectItemCaseSensitive(json, REQUEST_FIELD);
    if(!request_type) {
        cJSON_Delete(json);
        return UNDEFINED;
    }

    RequestType request = UNDEFINED;
    if(strcmp(request_type->valuestring, LOGIN_REQUEST_VALUE) == 0) {
        request = LOGIN_REQUEST;
    } else if (strcmp(request_type->valuestring, REGISTRATION_REQUEST_VALUE) == 0) {
        request = REGISTRATION_REQUEST;
    }
    return request;
}

int write_message(struct lws *connection_info, unsigned char *message, int len) {
    int bytes_sent = lws_write(connection_info, (unsigned char *)message, len, 0);
    if (bytes_sent < len) {
        lwsl_err("ERROR %d writing to ws socket\n", bytes_sent);
        return -1;
    }
}

int
handle_connection(struct lws *connection_info, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len)
{
    SessionData *session_data = (SessionData *)user;
    struct lws_vhost *vhost = lws_get_vhost(connection_info);
    const struct lws_protocols *protocols = lws_get_protocol(connection_info);
    VhostData *vhost_data = (VhostData*)lws_protocol_vh_priv_get(vhost, protocols);

    Packet incoming_message;
    int m, flags;

    switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
            initialize_connection(connection_info, vhost_data, in);
            break;

        case LWS_CALLBACK_ESTABLISHED:
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            write_message(connection_info, session_data->response_message, strlen(session_data->response_message));
            break;

        case LWS_CALLBACK_RECEIVE:

            if (lws_is_first_fragment(connection_info)) {
                session_data->message = malloc(len);
                memset(session_data->message, 0, len);
                memcpy(session_data->message, in, len);
            } else {
                session_data->message = realloc(session_data->message, session_data->message_len + len);
                memcpy(session_data->message + session_data->message_len, (char *)in, len);
                session_data->message_len += len;
            }

            if (lws_is_final_fragment(connection_info)) {
                session_data->response_message = (char *)malloc(256 + LWS_PRE);
                snprintf(session_data->response_message + LWS_PRE,256, "This is the response.");
                session_data->response_message += LWS_PRE;

                // Handle request
                RequestType request = get_request_type(session_data->message);
                if (request == LOGIN_REQUEST) {
                    if(login_handler(session_data->message)){
                        lwsl_user("Error handling login.\n");
                    }
                } else if (request == REGISTRATION_REQUEST) {
                    if(registration_handler(session_data->message)) {
                        lwsl_warn("Error handling registration.\n");
                    }
                }

                lws_callback_on_writable(connection_info);
            }

            break;

        case LWS_CALLBACK_CLOSED:
            lwsl_user("LWS_CALLBACK_CLOSED\n");
            if (!session_data->response_message) {
                free(session_data->response_message);
            }
            if(!session_data->message) {
                free(session_data->message);
            }
            lws_cancel_service(lws_get_context(connection_info));
            break;

        default:
            break;
    }

    return 0;
}