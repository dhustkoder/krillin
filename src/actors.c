#include <time.h>
#include "actors.h"



extern float frame_delta;


static int actor_count;
static actor_t actor_pool[MAX_USERS];

void actors_init(void)
{
	srand(time(NULL));
}

void actors_term(void)
{
}

actor_t* actors_find(const char* nick, size_t nicklen)
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

actor_t* actors_add(const char* nick, size_t nicklen)
{
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	if (actor_count >= MAX_USERS) {
		printf("ACTOR POOL IS FULL\n");
		return NULL;
	}
	
	actor_t* actor = &actor_pool[actor_count++];
	memcpy(actor->nick, nick, nicklen);
	actor->nick[nicklen] = '\0';
	actor->color = (rgba32_t) {
		rand() % 0xFF,
		rand() % 0xFF,
		rand() % 0xFF,
		0xFF
	};
	actor->action = ACTOR_ACTION_ARRIVING;
	actor->depart_ms = 10 * 1000;
	actor->depart_timer = get_timer();
	actor->speed = 218.0f;
	actor->vel = VEC2(0, 0);
	actor->pos = VEC2(rand() % WINDOW_W, -100);
	actor->dest = VEC2(28 + (rand() % 465), 406 + (rand() % 12));
	actor->char_id = rand() % CHARACTER_ID_MAX_IDS;
	return actor;
}

bool actors_set_actor_msg(actor_t* actor, const char* msg, size_t msglen)
{
	msglen = msglen > MAX_MSG_LEN ? MAX_MSG_LEN : msglen;
	
	if (actor->action == ACTOR_ACTION_DEPARTING)
		return false;
	
	if ((get_timer() - actor->cooldown_timer) < actor->cooldown_ms) {
		printf("could not set msg\n");
		return false;
	}
	
	memcpy(actor->msg, msg, msglen);
	actor->msg[msglen] = '\0';
	actor->cooldown_ms = 1000;
	actor->cooldown_timer = get_timer();
	actor->msg_display_ms = 4000 + (strlen(actor->msg) * 50);
	actor->msg_display_timer = get_timer();
	
	render_play_dialog_sfx(actor);
	
	return true;
}

static void actor_move(actor_t* a, vec2_t dest)
{
	a->vel = VEC2_SUB(dest, a->pos);
	a->vel = vec2_norm(a->vel);
	a->pos = VEC2_ADD(a->pos, VEC2_SCALE(a->vel, a->speed * frame_delta));
}


static void arriving_action_update(actor_t* actor)
{
	
	vec2_t distance = VEC2_SUB(actor->dest, actor->pos);
	
	if (VEC2_IS_ZERO(distance)) {
		actor->action = ACTOR_ACTION_STANDING;
		return;
	}
	
	float mul = 0.6;
	float x = distance.y * mul;
	printf("%.2f %.2f\n", actor->pos.x, actor->pos.y);
	printf("%.2f %.2f\n", actor->dest.x, actor->dest.y);
	
	actor_move(actor, VEC2(actor->dest.x + x, actor->dest.y));
}

static void standing_action_update(actor_t* a)
{
	if ((get_timer() - a->depart_timer) > a->depart_ms) {
		a->action = ACTOR_ACTION_DEPARTING;
		return;
	}
}

static void departing_action_update(actor_t* a)
{
	vec2_t dest = VEC2(a->pos.x, -120);
	if (VEC2_IS_ZERO(VEC2_SUB(dest, a->pos))) {
		a->action = ACTOR_ACTION_DELETE;
		return;
	}
	actor_move(a, dest);
}

void actors_update(void)
{
	for (int i = 0; i < actor_count; ++i) {
		actor_t* a = &actor_pool[i];
		switch (a->action) {
		case ACTOR_ACTION_ARRIVING: arriving_action_update(a); break;
		case ACTOR_ACTION_STANDING: standing_action_update(a); break;
		case ACTOR_ACTION_DEPARTING: departing_action_update(a); break;
		default: break;
		}
	}
	
	for (int i = 0; i < actor_count; ++i) {
		actor_t* a = &actor_pool[i];
		if (a->action == ACTOR_ACTION_DELETE) {
			ARRAY_RM_MEMMOVE(actor_pool, actor_count, i, 1);
			--i;
			--actor_count;
			continue;
		}
	}
	
	//printf("actor_count: %d\n", actor_count);
}

void actors_render(void)
{
	render_draw_actors(actor_pool, actor_count);
	
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