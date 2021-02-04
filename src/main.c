#include "render.h"
#include "krlnet.h"

static krln_socket_t mainsock;

static int user_count;
static user_t userpool[MAX_USERS];

static user_t* userpool_find(const char* nick)
{
	user_t* user = NULL;
	for (int i = 0; i < user_count; ++i) {
		if (strcmp(userpool[i].nick, nick) == 0) {
			user = &userpool[i];
			break;
		}
	}
	return user;
}

static user_t* userpool_add(const char* nick)
{
	if (user_count >= MAX_USERS)
		return NULL;
	user_t* user = &userpool[user_count++];
	strcpy(user->nick, nick);
	user->color = (rgba32_t) {
		rand() % 0xFF,
		rand() % 0xFF,
		rand() % 0xFF,
		0xFF
	};
	user->pos.x = rand() % (WINDOW_W - 48);
	user->pos.y = rand() % (WINDOW_H - 48);
	user->char_id = rand() % CHARACTER_ID_MAX_IDS;
	return user;
}


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

static void process_user_msg(void)
{
	// process and update userpool	
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
	
	user_t* user = userpool_find(nick);
	if (user == NULL) {
		user = userpool_add(nick);
		if (user == NULL) {
			printf("cannot add you %s </3", nick);
			return;
		}
		krlnet_socket_send(&mainsock, "PRIVMSG #daddy_dhust :Welcome @");
		krlnet_socket_send(&mainsock, nick);
		krlnet_socket_send(&mainsock, "\n");
	}
	
	if ((get_timer() - user->cooldown_timer) < COOLDOWN_TIME)
		return;

	memcpy(user->msg, msgstart, msglen);
	user->msg[msglen] = '\0';

	// print message
	user->cooldown_timer = get_timer();
	printf(":: %s :: -> %s\n", user->nick, user->msg);
}

static void process_packet(void)
{	
	if (mainsock.recv_size == 0)
		return;

	/*
	   printf(
	   "-------------------------- DATA (length %zu) -------------------------------\n"
	   "%s\n"
	   "----------------------------------------------------------------------------\n",
	   mainsock.recv_size,
	   mainsock.recv_data
	   );*/

	switch (check_pkt_type()) {
	case PKT_TYPE_TWITCH_PING: process_ping(); break;
	case PKT_TYPE_USER_MSG: process_user_msg(); break;
	default: break;
	}
}

static void krl_init(void)
{
	char oauth[80];
	char passcmd[256];

	memset(passcmd, 0x00, sizeof(passcmd));
	
	FILE *f = fopen(".auth", "r+");
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

static void krl_update(void)
{
	krlnet_socket_recv(&mainsock);
	process_packet();
	
	
	render_draw_users(userpool, user_count);
	
	
	for (int i = 0; i < user_count; ++i) {
		user_t* user = userpool + i;
		if ((get_timer() - user->cooldown_timer) < COOLDOWN_TIME) {
			render_draw_dialog(user);
		}
	}
}


static void krl_term(void)
{
	krlnet_socket_term(&mainsock);
	krlnet_term();
}



int main(void)
{
	printf("Hello Krillin\n");
	
	render_init();
	krl_init();
	
	while (render_update()) {
		render_clear();
		krl_update();
		render_flush();
	}
	
	krl_term();
	
	return 0;
}

