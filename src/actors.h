#ifndef KRL_ACTORS_H_
#define KRL_ACTORS_H_
#include "types.h"


extern actor_t* actors_find(const char* nick);
extern actor_t* actors_add(const char* nick);
extern void actors_update(void);
extern void actors_render(void);




#endif
