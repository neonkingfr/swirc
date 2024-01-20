/* Event 367 (RPL_BANLIST) and 368 (RPL_ENDOFBANLIST)
   Copyright (C) 2016-2024 Markus Uhlin. All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

   - Neither the name of the author nor the names of its contributors may be
     used to endorse or promote products derived from this software without
     specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE. */

#include "common.h"

#include <stdexcept>
#include <string>
#include <time.h>

#include "../dataClassify.h"
#include "../errHand.h"
#include "../irc.h"
#include "../printtext.h"
#include "../strHand.h"
#include "../theme.h"

#include "banlist.h"

#define TM_STRUCT_MSG "unable to retrieve tm structure"

static std::string
get_channel(const char *channel)
{
	std::string str(LEFT_BRKT);

	(void) str.append(COLOR1).append(channel).append(TXT_NORMAL);
	(void) str.append(RIGHT_BRKT);

	return str;
}

static std::string
get_mask(const char *mask)
{
	std::string str("");

	(void) str.append(COLOR4).append(mask).append(TXT_NORMAL);

	return str;
}

static void
feeds_written_4(PPRINTTEXT_CONTEXT ctx, char *params)
{
	char		*channel, *mask, *issuer, *seconds;
	char		*issuer_name, *issuer_userhost;
	char		*state1 = const_cast<char *>("");
	char		*state2 = const_cast<char *>("");
	char		 buf[500] = { '\0' };
	char		 tbuf[100] = { '\0' };
	struct tm	 result = { 0 };

	/* recipient */
	(void) strtok_r(params, "\n", &state1);

	if ((channel = strtok_r(NULL, "\n", &state1)) == NULL)
		throw std::runtime_error("unable to get channel");
	else if ((mask = strtok_r(NULL, "\n", &state1)) == NULL)
		throw std::runtime_error("unable to get mask");
	else if ((issuer = strtok_r(NULL, "\n", &state1)) == NULL)
		throw std::runtime_error("unable to get issuer");
	else if ((seconds = strtok_r(NULL, "\n", &state1)) == NULL)
		throw std::runtime_error("unable to get seconds");
	else if (sw_strcpy(buf, issuer, ARRAY_SIZE(buf)) != 0)
		throw std::runtime_error("cannot copy issuer");
	else if (!is_numeric(seconds))
		throw std::runtime_error("seconds not a number");

	if ((ctx->window = window_by_label(channel)) == NULL)
		ctx->window = g_status_window;
	if ((issuer_name = strtok_r(buf, "!", &state2)) == NULL)
		throw std::runtime_error("unable to get issuer name");
	if ((issuer_userhost = strtok_r(NULL, "!", &state2)) == NULL)
		issuer_userhost = const_cast<char *>("");

	const time_t date_of_issue = static_cast<time_t>(strtol(seconds, NULL,
	    10));

#if defined(UNIX)
	if (localtime_r(&date_of_issue, &result) == NULL)
		throw std::runtime_error("localtime_r: " TM_STRUCT_MSG);
#elif defined(WIN32)
	if (localtime_s(&result, &date_of_issue) != 0)
		throw std::runtime_error("localtime_s: " TM_STRUCT_MSG);
#endif

	if (strftime(tbuf, ARRAY_SIZE(tbuf), "%c", &result) == 0)
		throw std::runtime_error("strftime: zero return");

	printtext(ctx, "%s: %s issued by %s%s%s %s%s%s %s%s%s",
	    get_channel(channel).c_str(), get_mask(mask).c_str(),
	    COLOR2, issuer_name, TXT_NORMAL,
	    LEFT_BRKT, issuer_userhost, RIGHT_BRKT,
	    LEFT_BRKT, trim(tbuf), RIGHT_BRKT);
}

static void
feeds_written_2(PPRINTTEXT_CONTEXT ctx, char *params)
{
	char *channel, *mask;
	char *last = const_cast<char *>("");

	(void) strtok_r(params, "\n", &last);

	if ((channel = strtok_r(NULL, "\n", &last)) == NULL)
		throw std::runtime_error("unable to get channel");
	else if ((mask = strtok_r(NULL, "\n", &last)) == NULL)
		throw std::runtime_error("unable to get mask");

	if ((ctx->window = window_by_label(channel)) == NULL)
		ctx->window = g_status_window;

	printtext(ctx, "%s: %s", get_channel(channel).c_str(),
	    get_mask(mask).c_str());
}

/* event_banlist: 367

   Examples:
     :irc.server.com 367 <recipient> <channel> <mask> irc.server.com <time>
     :irc.server.com 367 <recipient> <channel> <mask> <nick>!<user>@<host>
                         <time>
     :irc.server.com 367 <recipient> <channel> <mask>

   Other events with the same semantics:
     - 346 (RPL_INVITELIST)
     - 348 (RPL_EXCEPTLIST) */
void
event_banlist(struct irc_message_compo *compo)
{
	PRINTTEXT_CONTEXT	ctx;

	printtext_context_init(&ctx, g_status_window, TYPE_SPEC1, true);

	try {
		int	feeds_written;

		if ((feeds_written = strFeed(compo->params, 4)) == 4) {
			feeds_written_4(&ctx, compo->params);
		} else if (feeds_written == 2) {
			feeds_written_2(&ctx, compo->params);
		} else {
			throw std::runtime_error("unexpected number of feeds "
			    "written");
		}
	} catch (const std::bad_alloc &e) {
		err_exit(ENOMEM, "event_banlist(%s): error: %s", compo->command,
		    e.what());
	} catch (const std::runtime_error &e) {
		ctx.spec_type = TYPE_SPEC1_FAILURE;

		printtext(&ctx, "event_banlist(%s): error: %s", compo->command,
		    e.what());
	} catch (...) {
		err_log(0, "event_banlist(%s): error: unknown exception",
		    compo->command);
	}
}

/*
 * event_quietlist: 728 (Undocumented in the RFC)
 *
 * Examples:
 *   :irc.server.com 728 <recipient> <channel> q <mask> irc.server.com <time>
 */
void
event_quietlist(struct irc_message_compo *compo)
{
	try {
		char	*cp;

		if ((cp = strstr(compo->params, " q ")) == NULL)
			throw std::runtime_error("cannot find substring");
		else if (strings_match(&cp[3], ""))
			throw std::runtime_error("parse error");
		(void) memmove(cp, &cp[2], strlen(&cp[2]) + 1);
		event_banlist(compo);
	} catch (const std::runtime_error &e) {
		err_log(0, "event_quietlist(%s): %s", compo->command, e.what());
	}
}

/* event_eof_banlist: 368

   Examples:
     :irc.server.com 368 <recipient> <channel> :End of Channel Ban List

   Other events that uses this function:
     :irc.server.com 347 <recipient> <channel> :End of Channel Invite List
     :irc.server.com 349 <recipient> <channel> :End of Channel Exception List */
void
event_eof_banlist(struct irc_message_compo *compo)
{
	PRINTTEXT_CONTEXT	ctx;

	printtext_context_init(&ctx, g_status_window, TYPE_SPEC1, true);

	try {
		char	*channel, *msg;
		char	*state = const_cast<char *>("");

		if (strFeed(compo->params, 2) != 2)
			throw std::runtime_error("strFeed");

		(void) strtok_r(compo->params, "\n", &state); /* recipient */

		if ((channel = strtok_r(NULL, "\n", &state)) == NULL ||
		    (msg = strtok_r(NULL, "\n", &state)) == NULL) {
			throw std::runtime_error("unable to retrieve event "
			    "components");
		}

		if ((ctx.window = window_by_label(channel)) == NULL)
			ctx.window = g_status_window;

		if (*msg == ':')
			msg++;
		if (*msg)
			printtext(&ctx, "%s", msg);
	} catch (const std::runtime_error &e) {
		ctx.spec_type = TYPE_SPEC1_FAILURE;

		printtext(&ctx, "event_eof_banlist(%s): error: %s",
		    compo->command, e.what());
	}
}

/*
 * event_eof_quietlist: 729 (Undocumented in the RFC)
 *
 * Examples:
 *   :irc.server.com 729 <recipient> <channel> q :End of Channel Quiet List
 */
void
event_eof_quietlist(struct irc_message_compo *compo)
{
	try {
		char	*cp;

		if ((cp = strstr(compo->params, " q ")) == NULL)
			throw std::runtime_error("cannot find substring");
		else if (strings_match(&cp[3], ""))
			throw std::runtime_error("parse error");
		(void) memmove(cp, &cp[2], strlen(&cp[2]) + 1);
		event_eof_banlist(compo);
	} catch (const std::runtime_error &e) {
		err_log(0, "event_eof_quietlist(%s): %s", compo->command,
		    e.what());
	}
}
