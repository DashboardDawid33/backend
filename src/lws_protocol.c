#include "lws_protocol.h"
#include <string.h>
#include "libwebsockets.h"
#include <assert.h>

static void
free_message(void *_msg)
{
    Message *msg = _msg;

    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

int
handle_connection(struct lws *connection_info, enum lws_callback_reasons reason,
                             void *user, void *in, size_t len)
{
    SessionData *session_data = (SessionData *)user;

    struct lws_vhost *vhost = lws_get_vhost(connection_info);
    const struct lws_protocols *protocols = lws_get_protocol(connection_info);

    VhostData *vhost_data = (VhostData*)lws_protocol_vh_priv_get(vhost, protocols);

    const Message *pmsg;
    Message amsg;
    int m, n, flags;

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
            vhost_data->interrupted = (int *)lws_pvo_search(
                    (const struct lws_protocol_vhost_options *)in,
                    "interrupted")->value;
            break;

        case LWS_CALLBACK_ESTABLISHED:
            /* generate a block of output before travis times us out */
            lwsl_warn("LWS_CALLBACK_ESTABLISHED\n");
            session_data->ring = lws_ring_create(sizeof(Message), RING_DEPTH,
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

            pmsg = lws_ring_get_element(session_data->ring, &session_data->tail);
            if (!pmsg) {
                lwsl_user(" (nothing in ring)\n");
                break;
            }

            flags = lws_write_ws_flags(
                    pmsg->binary ? LWS_WRITE_BINARY : LWS_WRITE_TEXT,
                    pmsg->first, pmsg->final);


            m = lws_write(connection_info, ((unsigned char *)pmsg->payload) +
                               LWS_PRE, pmsg->len, (enum lws_write_protocol)flags);
            if (m < (int)pmsg->len) {
                lwsl_err("ERROR %d writing to ws socket\n", m);
                return -1;
            }

            lwsl_user(" wrote %d: flags: 0x%x first: %d final %d\n",
                      m, flags, pmsg->first, pmsg->final);

            break;

        case LWS_CALLBACK_RECEIVE:

            lwsl_user("LWS_CALLBACK_RECEIVE: %4d (rpp %5d, first %d, "
                      "last %d, bin %d, msglen %d (+ %d = %d))\n",
                      (int)len, (int)lws_remaining_packet_payload(connection_info),
                      lws_is_first_fragment(connection_info),
                      lws_is_final_fragment(connection_info),
                      lws_frame_is_binary(connection_info), session_data->msglen, (int)len,
                      (int)session_data->msglen + (int)len);

            amsg.first = (char)lws_is_first_fragment(connection_info);
            amsg.final = (char)lws_is_final_fragment(connection_info);
            amsg.binary = (char)lws_frame_is_binary(connection_info);

            n = (int)lws_ring_get_count_free_elements(session_data->ring);
            if (!n) {
                lwsl_user("dropping!\n");
                break;
            }

            if (amsg.final)
                session_data->msglen = 0;
            else
                session_data->msglen += (uint32_t)len;

            amsg.len = len;
            /* notice we over-allocate by LWS_PRE */

            amsg.payload = malloc(LWS_PRE + len);
            if (!amsg.payload) {
                lwsl_user("OOM: dropping\n");
                break;
            }

            memcpy((char *)amsg.payload + LWS_PRE, in, len);
            const char* out = amsg.payload + LWS_PRE;

            lwsl_user("%s\n", (char *)amsg.payload + LWS_PRE);
            if (!lws_ring_insert(session_data->ring, &amsg, 1)) {
                free_message(&amsg);
                lwsl_user("dropping!\n");
                break;
            }
            lws_callback_on_writable(connection_info);

            /*
            if (n < 3 && !session_data->flow_controlled) {
                session_data->flow_controlled = 1;
                lws_rx_flow_control(connection_info, 0);
            }
            */

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