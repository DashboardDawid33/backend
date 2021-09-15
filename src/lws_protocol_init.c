#include "libwebsockets.h"
#include "lws_protocol.h"

int initialize_connection(struct lws *connection_info, VhostData *vhost_data, void *in) {
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
    return 0;
}