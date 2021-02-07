#include "krlbot.h"


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
	
	actors_set_actor_msg(a, msg_start, msg_end - msg_start);
	
	printf(":: %s :: => %s\n", a->nick, a->msg);
	
	krlnet_write("PRIVMSG #daddy_dhust :Hello @%s I love you LOL\n", a->nick);
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
