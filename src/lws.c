#include <string.h>
#include "libwebsockets.h"
#include "lws_protocol.h"
#include <signal.h>
#define PORT 8888

static struct lws_protocols protocols[] = {
        {
            "websocket-server",
            handle_connection,
            sizeof(SessionData),
            1024,
            0, NULL, 0
	    },
        { NULL, NULL, 0, 0 } /* terminator */
};

static int interrupted;

/* pass pointers to shared vars to the protocol */

static const struct lws_protocol_vhost_options pvo_interrupted = {
        NULL,
        NULL,
        "interrupted",		/* pvo name */
        (void *)&interrupted	/* pvo value */
};

static const struct lws_protocol_vhost_options pvo = {
        NULL,				/* "next" pvo linked-list */
        &pvo_interrupted,		/* "child" pvo linked-list */
        "websocket-server",	/* protocol name we belong to on this vhost */
        ""				/* ignored */
};

void sigint_handler(int sig)
{
    interrupted = 1;
}

int lws_start(int argc, const char **argv)
{
    signal(SIGINT, sigint_handler);

    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
    lws_set_log_level(logs, NULL);

    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
    info.port = PORT;
    info.protocols = protocols;
    info.pvo = &pvo;
    info.pt_serv_buf_size = 32 * 1024;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 |
                   LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;

    context = lws_create_context(&info);
    if (!context) {
        lwsl_err("lws init failed\n");
        return 1;
    }

    int n = 0;
    while (n >= 0 && !interrupted)
        n = lws_service(context, 0);

    lws_context_destroy(context);

    lwsl_user("Completed %s\n", interrupted == 2 ? "OK" : "failed");

    return interrupted != 2;
}
