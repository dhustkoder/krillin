#include "krl.h"

#define MAX_IRC_LINE_LEN    (680)


static const cmd_handler_t cmd_handlers[] = {
	CMD_HANDLER_STR("help", "Commands are: !today and... thats it for now :|"),
	CMD_HANDLER_STR("today", "Trying to get back to work")
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

static const cmd_handler_t* command_find_handler(
	const char* cmdstr, 
	int cmdlen,
	const cmd_handler_t* handlers,
	size_t handler_count
)
{
	const cmd_handler_t* handler = NULL;
	for (size_t i = 0; i < handler_count; ++i) {
		if (strlen(handlers[i].cmd) == cmdlen) {
			if (memcmp(handlers[i].cmd, cmdstr, cmdlen) == 0) {
				handler = &handlers[i];
				break;
			}
		}
	}
	return handler;
}

static bool command_try_handle(actor_t* a, const char* line, size_t len)
{
	assert(*line == '!');
	const char* cmd_start = line + 1;
	const char chars[] = {' ', '\r', '\n', '\0'};
	const char* cmd_end = strchr_n(cmd_start, chars, STATIC_ARRAY_COUNT(chars));
	assert(cmd_end != NULL);
	size_t cmd_len = cmd_end - cmd_start;
	const char* args_start = cmd_end + 1;
	const char* args_end = strchr_n(args_start, &chars[1], STATIC_ARRAY_COUNT(chars) - 1);
	assert(args_end != NULL);
	
	const cmd_handler_t* handler = NULL;
	handler = command_find_handler(
		cmd_start,
		cmd_len,
		cmd_handlers,
		STATIC_ARRAY_COUNT(cmd_handlers)
	); 	
	if (handler == NULL) {
		handler = command_find_handler(
			cmd_start,
			cmd_len,
			vpad_default_handlers,
			VPAD_DEFAULT_HANDLERS_COUNT	
		); 
	}
	
	if (handler == NULL)
		return false;
	
	if (handler->type == CMD_HANDLER_TYPE_STR) {
		krlnet_write("PRIVMSG #daddy_dhust :%s\n", handler->str);
	} else {
		cmd_info_t cmd_info = {
			.actor    = a,
			.cmd      = cmd_start,
			.cmd_len  = cmd_len,
			.args     = args_start,
			.args_len = args_end - args_start
		};
		handler->fn(&cmd_info);
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
		"JOIN #daddy_dhust\n",
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

