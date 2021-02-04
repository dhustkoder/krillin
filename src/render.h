#ifndef KRL_RENDER_H_
#define KRL_RENDER_H_
#include "types.h"

void render_init(void);
bool render_update(void);
void render_draw_users(user_t* users, int count);
void render_clear(void);
void render_flush(void);





#endif
