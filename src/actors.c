#include "actors.h"






static int actor_count;
static actor_t actor_pool[MAX_USERS];

actor_t* actors_find(const char* nick)
{
	actor_t* actor = NULL;
	for (int i = 0; i < actor_count; ++i) {
		if (strcmp(actor_pool[i].nick, nick) == 0) {
			actor = &actor_pool[i];
			break;
		}
	}
	return actor;
}

actor_t* actors_add(const char* nick)
{
	if (actor_count >= MAX_USERS)
		return NULL;
	
	actor_t* actor = &actor_pool[actor_count++];
	strcpy(actor->nick, nick);
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


void actors_update(void)
{
	
}

void actors_render(void)
{
	render_draw_users(actor_pool, actor_count);
	
	for (int i = 0; i < actor_count; ++i) {
		actor_t* actor = actor_pool + i;
		if ((get_timer() - actor->cooldown_timer) < COOLDOWN_TIME) {
			render_draw_dialog(actor);
			break;
		}
	}
}