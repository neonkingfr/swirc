#include "base64.h"
#include "common.h"

#include "../config.h"
#include "../irc.h"
#include "../libUtils.h"
#include "../network.h"
#include "../strHand.h"
#include "../strdup_printf.h"

#include "auth.h"

static bool
build_auth_message(char **msg)
{
    const char *username = Config("sasl_username");
    const char *password = Config("sasl_password");

    if (Strings_match(username, "") || Strings_match(password, "")) {
	*msg = NULL;
	return false;
    }

    char *msg_unencoded = Strdup_printf("%s%c%s%c%s",
	username, '\0', username, '\0', password);
    size_t len = strlen(username) * 2;
    len += 2;
    len += strlen(password);
    *msg = xmalloc(1000);
    if (b64_encode((uint8_t *) msg_unencoded, len, *msg, 1000) == -1) {
	free(msg_unencoded);
	free(*msg);
	*msg = NULL;
	return false;
    }
    free(msg_unencoded);
    return true;
}

static void
abort_authentication()
{
    net_send("AUTHENTICATE *");
    net_send("CAP END");
}

void
event_authenticate(struct irc_message_compo *compo)
{
    if (Strings_match(compo->params, "+")) {
	char *msg = NULL;

	if (!build_auth_message(&msg)) {
	    abort_authentication();
	    return;
	}

	net_send("AUTHENTICATE %s", msg);
	free(msg);
    }
}
