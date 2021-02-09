#include "krlbot.h"

#define MAX_IRC_LINE_LEN    (680)




static const char* cmd_strings[] = {
	"help",
	"today"
};

static const cmd_handler_t cmd_handlers[] = {
	CMD_HANDLER_STR("Commands are: !today and... thats it for now :|"),
	CMD_HANDLER_STR("Trying to get back to work")
};

static const char* strchr_n(const char* str, const char chars[], size_t n)
{	
	for (size_t i = 0; i < n; ++i) {
		const char* found = strchr(str, chars[i]);
		if (found != NULL)
			return found;
	}
	
	return NULL;
}

static bool command_try_handle(actor_t* a, const char* line, size_t len)
{
	assert(*line == '!');
	const char* start = line + 1;
	const char chars[] = {' ', '\r', '\n'};
	const char* end = strchr_n(start, chars, STATIC_ARRAY_COUNT(chars));
	assert(end != NULL);
	size_t cmd_len = end - start;
	
	const cmd_handler_t* handler = NULL;
	
	for (size_t i = 0; i < STATIC_ARRAY_COUNT(cmd_strings); ++i) {
		if (strlen(cmd_strings[i]) == cmd_len) {
			if (memcmp(cmd_strings[i], start, cmd_len) == 0) {
				handler = &cmd_handlers[i];
				break;
			}
		}
	}
	
	if (handler == NULL)
		return false;
	
	if (handler->type == CMD_HANDLER_TYPE_STR) {
		krlnet_write("PRIVMSG #daddy_dhust :%s\n", handler->str);
	} else {
		handler->fn(a, line, len);
	}
	
	return true;
}

static void process_irc_privmsg(const char* line, size_t len)
{
	printf(":: PROCESSING PRIVMSG :: => %s\n", line);
	
	const char* nick_start = strchr(line, ':');
	assert(nick_start != NULL);
	++nick_start;
	
	const char* nick_end = strchr(line, '!');
	assert(nick_end != NULL);
	
	actor_t* a = actors_find(nick_start, nick_end - nick_start);
	if (a == NULL) {
		a = actors_add(nick_start, nick_end - nick_start);
		if (a == NULL) {
			return;
		}
	}
	
	const char* msg_start = strchr(nick_end, ':');
	assert(msg_start != NULL);
	++msg_start;
	const char* msg_end = strchr(msg_start, '\n');
	assert(msg_end != NULL);
	
	if (!actors_set_actor_msg(a, msg_start, msg_end - msg_start))
		return;
	
	printf(":: %s :: => %s\n", a->nick.data, a->msg.data);
	
	if (*msg_start == '!') {
		if (!command_try_handle(a, msg_start, msg_end - msg_start)) {
			krlnet_write(
				"PRIVMSG #daddy_dhust :@%s wtf youre trying to do? try !help\n",
				a->nick.data
			);
		}
	}
}

static void process_irc_ping(const char* line, size_t len)
{
	printf(":: PROCESSING PING :: => %s", line);
	krlnet_write("PONG :tmi.twitch.tv\n");
}

static void process_irc_line(const char* line, size_t len)
{	
	if (strstr(line, "PRIVMSG") != NULL) {
		process_irc_privmsg(line, len);
	} else if (strstr(line, "PING") != NULL) {
		process_irc_ping(line, len);
	}
}

void krlbot_init(void)
{
	char oauth[80];
	
	FILE *f = fopen(".oauth", "r+");
	assert(f != NULL);
	fscanf(f, "%s", oauth);
	fclose(f);
	
	krlnet_connect("irc.chat.twitch.tv", 6667);
	
	krlnet_write(
		"PASS %s\n"
		"NICK daddy_krillin_bot\n"
		"JOIN #trainwreckstv\n",
		oauth
	);
}

void krlbot_update(void)
{
	static char irc_line[MAX_IRC_LINE_LEN + 1];
	size_t len = krlnet_readline(irc_line, MAX_IRC_LINE_LEN);
	if (len > 0) {
		process_irc_line(irc_line, len);
	}
	
}

void krlbot_term(void)
{	
}

