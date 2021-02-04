#include <time.h>
#include "actors.h"






static int actor_count;
static actor_t actor_pool[MAX_USERS];

void actors_init(void)
{
	srand(time(NULL));
}

void actors_term(void)
{
}

actor_t* actors_find(const char* nick, int nicklen)
{
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	actor_t* actor = NULL;
	for (int i = 0; i < actor_count; ++i) {
		if (memcmp(actor_pool[i].nick, nick, nicklen) == 0) {
			actor = &actor_pool[i];
			break;
		}
	}
	return actor;
}

actor_t* actors_add(const char* nick, int nicklen)
{
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	if (actor_count >= MAX_USERS)
		return NULL;
	
	actor_t* actor = &actor_pool[actor_count++];
	memcpy(actor->nick, nick, nicklen);
	actor->nick[nicklen] = '\0';
	actor->color = (rgba32_t) {
		rand() % 0xFF,
		rand() % 0xFF,
		rand() % 0xFF,
		0xFF
	};
	actor->pos.x = rand() % (WINDOW_W - 48);
	actor->pos.y = rand() % (WINDOW_H - 48);
	actor->char_id = rand() % CHARACTER_ID_MAX_IDS;
	return actor;
}

bool actors_set_actor_msg(actor_t* actor, const char* msg, int msglen)
{
	msglen = msglen > MAX_MSG_LEN ? MAX_MSG_LEN : msglen;
	
	if ((get_timer() - actor->cooldown_timer) < actor->cooldown_ms) {
		printf("could not set msg\n");
		return false;
	}
	
	memcpy(actor->msg, msg, msglen);
	actor->msg[msglen] = '\0';
	actor->cooldown_ms = 8000;
	actor->cooldown_timer = get_timer();
	actor->msg_display_ms = 1000 + (strlen(actor->msg) * 50);
	actor->msg_display_timer = get_timer();
	
	return true;
}

void actors_update(void)
{
	
}

void actors_render(void)
{
	render_draw_users(actor_pool, actor_count);
	
	actor_t* display_msg_actor = NULL;
	for (int i = 0; i < actor_count; ++i) {
		actor_t* actor = actor_pool + i;
		if ((get_timer() - actor->msg_display_timer) < actor->msg_display_ms) {
			if (display_msg_actor == NULL)
				display_msg_actor = actor;
			else if (actor->msg_display_timer < display_msg_actor->msg_display_timer)
				display_msg_actor = actor;
		}
	}
	
	if (display_msg_actor != NULL) {
		render_draw_dialog(display_msg_actor);
	}
}