#include "common.h"

#include <setjmp.h>
#include <cmocka.h>

#include "network.h"
#include "commands/znc.h"

static int
setup(void **state)
{
	net_send = net_send_fake;
	UNUSED_PARAM(state);
	return 0;
}

static void
sendsExpected_test1(void **state)
{
	cmd_znc("SetBuffer #ChatZone 300");
	assert_string_equal(g_sent, "PRIVMSG *status :SetBuffer #ChatZone 300");
	UNUSED_PARAM(state);
}

static void
sendsExpected_test2(void **state)
{
	cmd_znc("*status setBUFFER #ChatZone 300");
	assert_string_equal(g_sent, "PRIVMSG *status :setBUFFER #ChatZone 300");
	UNUSED_PARAM(state);
}

static void
sendsExpected_test3(void **state)
{
	cmd_znc("*ControlPanel help");
	assert_string_equal(g_sent, "PRIVMSG *controlpanel :help");
	UNUSED_PARAM(state);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(sendsExpected_test1),
		cmocka_unit_test(sendsExpected_test2),
		cmocka_unit_test(sendsExpected_test3),
	};

	return cmocka_run_group_tests(tests, setup, NULL);
}
