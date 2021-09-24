#include "lws.h"
#include <string.h>
#include "libwebsockets.h"
#include "cjson/cJSON.h"
#include "request_handler.h"

int clients = 0;

static void
free_message(void *_msg) {
    Packet *msg = _msg;
    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

RequestType
get_request_type(const char *incoming_data) {
    cJSON *json = cJSON_Parse(incoming_data);
    if (!json) {
        return UNDEFINED_REQUEST;
    }
    cJSON *request_type = cJSON_GetObjectItemCaseSensitive(json, REQUEST_FIELD);
    if (!request_type) {
        cJSON_Delete(json);
        return UNDEFINED_REQUEST;
    }

    RequestType request = UNDEFINED_REQUEST;
    if (strcmp(request_type->valuestring, LOGIN_REQUEST_VALUE) == 0) {
        request = LOGIN_REQUEST;
    } else if (strcmp(request_type->valuestring, REGISTRATION_REQUEST_VALUE) == 0) {
        request = REGISTRATION_REQUEST;
    }
    return request;
}

int write_message(struct lws *connection_info, unsigned char *message, int len) {
    int bytes_sent = lws_write(connection_info, (unsigned char *) message, len, 0);
    if (bytes_sent < len) {
        lwsl_err("ERROR %d writing to ws socket\n", bytes_sent);
        return -1;
    }
    return 0;
}

int
handle_connection(struct lws *connection_info, enum lws_callback_reasons reason,
                  void *user, void *in, size_t len) {

    SessionData *session_data = (SessionData *) user;
    struct lws_vhost *vhost = lws_get_vhost(connection_info);
    const struct lws_protocols *protocols = lws_get_protocol(connection_info);
    VhostData *vhost_data = (VhostData *) lws_protocol_vh_priv_get(vhost, protocols);

    switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
            vhost_data = lws_protocol_vh_priv_zalloc(lws_get_vhost(connection_info),
                                                     lws_get_protocol(connection_info),
                                                     sizeof(VhostData));
            if (!vhost_data)
                return -1;

            vhost_data->context = lws_get_context(connection_info);
            vhost_data->vhost = lws_get_vhost(connection_info);

            /* get the pointers we were passed in pvo */
            vhost_data->interrupted = (int *) lws_pvo_search(
                    (const struct lws_protocol_vhost_options *) in,
                    "interrupted")->value;
            break;

        case LWS_CALLBACK_ESTABLISHED:
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            write_message(connection_info, (unsigned char *) session_data->response_message,
                          strlen(session_data->response_message));
            break;

        case LWS_CALLBACK_RECEIVE:

            // Todo: figure out a better way of receiving connections.
            if (lws_is_first_fragment(connection_info)) {
                //Copy message from received data into session data
                session_data->message = malloc(len);
                strncpy(session_data->message, (char *) in, len);

                // If \0 is not set, garbage data might ensue.
                session_data->message[len] = '\0';
                session_data->message_len = len;
                session_data->message_count++;
            } else {
                session_data->message = realloc(session_data->message, session_data->message_len + len);
                memcpy(session_data->message + session_data->message_len, (char *) in, len);
                session_data->message_len += len;
                session_data->message_count++;
            }

            if (lws_is_final_fragment(connection_info)) {
                session_data->response_message = (char *) malloc(256 + LWS_PRE);
                snprintf(session_data->response_message + LWS_PRE, 256, "This is the response.");
                session_data->response_message += LWS_PRE;

                lwsl_user("Message on receive : %s\n", session_data->message);

                // Handle request
                RequestType request = get_request_type(session_data->message);
                if (request == LOGIN_REQUEST) {
                    ErrMsg *err = (ErrMsg*)malloc(sizeof(ErrMsg));
                    if (login_handler(session_data->message, err)) {
                        lwsl_user("Error handling login.\n");
                    }
                    free_err_msg(err);
                } else if (request == REGISTRATION_REQUEST) {
                    ErrMsg *err = (ErrMsg*)malloc(sizeof(ErrMsg));
                    //TODO: Finish refactoring this part with error messages.
                    registration_handler(session_data->message, err)
                    if (err->kind != ERR_NONE) {
                        lwsl_warn("Error handling registration.\n");
                    }
                    free_err_msg(err);
                } else if (request == UNDEFINED_REQUEST) {
                    lwsl_user("Missing / incorrect request field / cannot parse json. Dropping.");
                    break;
                }
                lws_callback_on_writable(connection_info);
                clients++;
                lwsl_notice("Served clients : %d\n", clients);
            }

            break;

        case LWS_CALLBACK_CLOSED:

            if (!session_data->response_message) {
                free(session_data->response_message);
            }
            if (!session_data->message) {
                free(session_data->message);
            }
            lws_cancel_service(lws_get_context(connection_info));
            break;

        default:
            break;
    }

    return 0;
}