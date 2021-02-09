#ifndef KRL_ACTORS_H_
#define KRL_ACTORS_H_
#include "krl.h"


extern void actors_init(void);
extern void actors_term(void);
extern actor_t* actors_find(const char* nick, size_t nicklen);
extern actor_t* actors_add(const char* nick, size_t nicklen);
extern bool actors_set_actor_msg(actor_t* actor, const char* msg, size_t msglen);
extern void actors_update(void);
extern void actors_render(void);




#endif
