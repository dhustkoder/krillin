#ifndef KRL_ACTORS_H_
#define KRL_ACTORS_H_
#include "types.h"

extern void actors_init(void);
extern void actors_term(void);
extern actor_t* actors_find(const char* nick, int nicklen);
extern actor_t* actors_add(const char* nick, int nicklen);
extern bool actors_set_actor_msg(actor_t* actor, const char* msg, int msglen);
extern void actors_update(void);
extern void actors_render(void);




#endif
