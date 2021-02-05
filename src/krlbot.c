#include "krlbot.h"

static krln_socket_t mainsock;
static FILE* nul_file;


static void krlbot_response_printf(krlbot_response_t* resp, const char* str, ...)
{
	va_list vl;
	va_start(vl, str);
	
	const int result_len = vfprintf(nul_file, str, vl);
	if ((result_len + resp->len) > MAX_MSG_LEN)
		return;
	
	vsprintf(resp->msg + resp->len, str, vl);
	resp->len += result_len;
	
	va_end(vl);	
}

static void krlbot_response_send(const krlbot_response_t* resp)
{
	krlnet_socket_send(&mainsock, "PRIVMSG #daddy_dhust :");
	krlnet_socket_send(&mainsock, resp->msg);
	krlnet_socket_send(&mainsock, "\n");
}

static void krlbot_response_clear(krlbot_response_t* resp)
{
	memset(resp, 0x00, sizeof(*resp));
}




static void help_command(actor_t* actor, krlbot_response_t* resp)
{
	krlbot_response_printf(resp, "@%s get some help dude!", actor->nick);
}


static command_clbk_pair_t command_pairs[] = {
	{
		.cmd = "!help",
		.clbk = help_command
	}
};




static pkt_type_t check_pkt_type(void)
{
	if (mainsock.recv_size == 0)
		return PKT_TYPE_NONE;
	
	if (mainsock.recv_size >= MAX_PKT_SIZE)
		return PKT_TYPE_NONE;
	
	const char* twitchbot = ":tmi.twitch.tv";
	const char* pingstr = "PING";
	const char* privmsgstr = "PRIVMSG";
	
	if (memcmp(mainsock.recv_data, twitchbot, strlen(twitchbot)) == 0)
		return PKT_TYPE_NONE;
	
	if (memcmp(mainsock.recv_data, pingstr, strlen(pingstr)) == 0)
		return PKT_TYPE_TWITCH_PING;
	
	if (
		strstr((char*)mainsock.recv_data, privmsgstr) != NULL &&
		strchr((char*)mainsock.recv_data, '!') != NULL &&
		strchr((char*)mainsock.recv_data, ':') != NULL
	) {
		return PKT_TYPE_USER_MSG;
	}
	return PKT_TYPE_NONE;
}


static void process_ping(void)
{
	krlnet_socket_send(&mainsock, "PONG :tmi.twitch.tv\n");
}

static void process_actor_msg(void)
{
	static krlbot_response_t response;
	
	
	const char* nickstart = ((char*)mainsock.recv_data) + 1;
	const char* nickend = strchr(nickstart, '!');
	assert(nickend != NULL);
	
	const char* msgstart = strchr(nickend, ':');
	assert(msgstart != NULL);
	
	++msgstart;
	
	const char* msgend = strchr(msgstart, '\0');
	assert(msgend != NULL);

	
	actor_t* actor = actors_find(nickstart, nickend - nickstart);
	if (actor == NULL) {
		actor = actors_add(nickstart, nickend - nickstart);
		
		if (actor == NULL) {
			printf("ROOM IS FULL\n");
			return;
		}
		
		krlbot_response_clear(&response);
		krlbot_response_printf(&response, "Welcome @%s", actor->nick);
		krlbot_response_send(&response);
	}
	
	if ((get_timer() - actor->cooldown_timer) < actor->cooldown_ms)
		return;

	if (!actors_set_actor_msg(actor, msgstart, msgend - msgstart))
		return;

	printf(":: %s :: -> %s\n", actor->nick, actor->msg);
	
	if (actor->msg[0] != '!')
		return;

	for (int i = 0; i < STATIC_ARRAY_COUNT(command_pairs); ++i) {
		if (memcmp(command_pairs[i].cmd, actor->msg, strlen(command_pairs[i].cmd)) == 0) {
			krlbot_response_clear(&response);
			command_pairs[i].clbk(actor, &response);
			krlbot_response_send(&response);
			break;
		}
	}
}

static void process_packet(void)
{	
	if (mainsock.recv_size == 0)
		return;
	
	pkt_type_t type = check_pkt_type();
	
	printf(
		"\n\n-------------------------- PKT (length %zu, type %d) -------------------------------\n"
		"%s\n"
		"--------------------------------------------------------------------------------------\n",
		mainsock.recv_size,
		type,
		mainsock.recv_data
	);

	switch (type) {
	case PKT_TYPE_TWITCH_PING: process_ping(); break;
	case PKT_TYPE_USER_MSG: process_actor_msg(); break;
	default: break;
	}
}

void krlbot_init(void)
{
	char oauth[80];
	char passcmd[256];
	
	nul_file = fopen("nul", "w");
	assert(nul_file != NULL);
	

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
	fclose(nul_file);
}
