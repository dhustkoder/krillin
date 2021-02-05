#ifndef KRL_RENDER_H_
#define KRL_RENDER_H_
#include "types.h"

void render_init(void);
void render_term(void);
bool render_poll_events(void);
void render_draw_users(actor_t* users, int count);
void render_play_dialog_sfx(actor_t* user);
void render_draw_dialog(actor_t* user);
void render_clear(void);
void render_flush(void);





#endif
