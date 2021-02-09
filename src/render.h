#ifndef KRL_RENDER_H_
#define KRL_RENDER_H_
#include "krl.h"


void render_init(void);
void render_term(void);
bool render_poll_events(void);
void render_draw_actors(actor_t* actors, int count);
void render_play_dialog_sfx(actor_t* actor);

void render_draw_msg_stack(
	const msg_stack_entry_t* entries,
	size_t count
);

void render_clear(void);
void render_flush(void);



#endif
