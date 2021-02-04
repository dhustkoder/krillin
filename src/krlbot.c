#include "krlbot.h"




static void help_command(actor_t* actor, char* response)
{
	strcat(response, "@");
	strcat(response, actor->nick);
	strcat(response, " ");
	strcat(
		response,
		"get some help dude..."
	);
}


static command_clbk_pair_t command_pairs[] = {
	{
		.cmd = "!help",
		.clbk = help_command
	}
};



static krln_socket_t mainsock;

static pkt_type_t check_pkt_type(void)
{
	if (mainsock.recv_size == 0)
		return PKT_TYPE_NONE;
	
	if (mainsock.recv_size >= MAX_PKT_SIZE)
		return PKT_TYPE_NONE;
	
	const char* twitchbot = ":tmi.twitch.tv";
	const char* pingstr = "PING";
	
	if (memcmp(mainsock.recv_data, twitchbot, strlen(twitchbot)) == 0)
		return PKT_TYPE_NONE;
	
	if (memcmp(mainsock.recv_data, pingstr, strlen(pingstr)) == 0)
		return PKT_TYPE_TWITCH_PING;
	
	if (strchr(mainsock.recv_data, '!') != NULL)
		return PKT_TYPE_USER_MSG;
	
	return PKT_TYPE_NONE;
}


static void process_ping(void)
{
	krlnet_socket_send(&mainsock, "PONG :tmi.twitch.tv\n");
}

static void process_actor_msg(void)
{
	// process and update actorpool	
	char nick [ MAX_NICK_LEN + 1 ]; // Crashed by SpookyCookieMonster
	
	const char* nickstart = mainsock.recv_data + 1;
	const char* nickend = strchr(nickstart, '!');
	assert(nickend != NULL);
	
	const char* msgstart = strchr(nickend, ':');
	assert(msgstart != NULL);
	
	++msgstart;
	
	const char* msgend = strchr(msgstart, '\0');
	assert(msgend != NULL);
	
	int nicklen = nickend - nickstart;
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	int msglen = msgend - msgstart;
	msglen = msglen > MAX_MSG_LEN ? MAX_MSG_LEN : msglen;
	
	memcpy(nick, nickstart, nicklen);
	nick[nicklen] = '\0';
	
	actor_t* actor = actors_find(nick);
	if (actor == NULL) {
		actor = actors_add(nick);
		if (actor == NULL) {
			printf("cannot add you %s </3", nick);
			return;
		}
		krlnet_socket_send(&mainsock, "PRIVMSG #daddy_dhust :Welcome @");
		krlnet_socket_send(&mainsock, nick);
		krlnet_socket_send(&mainsock, "\n");
	}
	
	if ((get_timer() - actor->cooldown_timer) < COOLDOWN_TIME)
		return;

	memcpy(actor->msg, msgstart, msglen);
	actor->msg[msglen] = '\0';

	// print message
	actor->cooldown_timer = get_timer();
	printf(":: %s :: -> %s\n", actor->nick, actor->msg);
	
	if (actor->msg[0] != '!')
		return;
	
	static char response[MAX_MSG_LEN + 1];
	memset(response, 0, sizeof(response));
	for (int i = 0; i < STATIC_ARRAY_COUNT(command_pairs); ++i) {
		if (memcmp(command_pairs[i].cmd, actor->msg, strlen(command_pairs[i].cmd)) == 0) {
			command_pairs[i].clbk(actor, response);
			if (strlen(response) > 0) {
				krlnet_socket_send(&mainsock, "PRIVMSG #daddy_dhust :");
				krlnet_socket_send(&mainsock, response);
				krlnet_socket_send(&mainsock, "\n");
			}
			break;
		}
	}
}

static void process_packet(void)
{	
	if (mainsock.recv_size == 0)
		return;

	
	   printf(
		   "-------------------------- DATA (length %zu) -------------------------------\n"
		   "%s\n"
		   "----------------------------------------------------------------------------\n",
		   mainsock.recv_size,
		   mainsock.recv_data
	   );

	switch (check_pkt_type()) {
	case PKT_TYPE_TWITCH_PING: process_ping(); break;
	case PKT_TYPE_USER_MSG: process_actor_msg(); break;
	default: break;
	}
}

void krlbot_init(void)
{
	char oauth[80];
	char passcmd[256];

	memset(passcmd, 0x00, sizeof(passcmd));
	
	FILE *f = fopen(".oauth", "r+");
	assert(f != NULL);
	fscanf(f, "%s", oauth);
	fclose(f);
	
	strcat(passcmd, "PASS ");
	strcat(passcmd, oauth);
	strcat(passcmd, "\n");
	
	krlnet_init();
	krlnet_socket_init(&mainsock, "irc.chat.twitch.tv", 6667);
	krlnet_socket_send(&mainsock, passcmd);
	krlnet_socket_send(&mainsock, "NICK daddy_dhust\n");
	krlnet_socket_send(&mainsock, "JOIN #daddy_dhust\n");
	
	krlnet_socket_send(&mainsock, "PRIVMSG #daddy_dhust :Initializing Krillin Bot\n");
	
	srand(time(NULL));
	
	printf("Krillin Initialized\n");
}

void krlbot_update(void)
{
	krlnet_socket_recv(&mainsock);
	process_packet();
}


void krlbot_term(void)
{
	krlnet_socket_term(&mainsock);
	krlnet_term();
}
