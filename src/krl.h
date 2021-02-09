#ifndef KRL_H_
#define KRL_H_
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <SDL.h>

/*
	BASIC TYPES AND UTILITIES
*/

typedef Uint32 timer_t;
#define get_timer() SDL_GetTicks()


typedef struct rgba32 {
	uint8_t r, g, b, a;
} rgba32_t;

typedef struct vec2 {
	float x, y;
} vec2_t;

typedef struct rect {
	vec2_t pos;
	vec2_t size;
} rect_t;


#define VEC2(...) ((vec2_t){ __VA_ARGS__ })
#define RECT(...) ((rect_t){ __VA_ARGS__ })

#define STATIC_ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

#define ARRAY_RM_MEMMOVE(array, current_count, rm_start_idx, rm_count) { \
	assert(((rm_start_idx) + (rm_count)) <= (current_count)); \
	void* overwrite_start_addr_ = (array) + (rm_start_idx); \
	void* reading_start_addr_ = (array) + (rm_start_idx) + (rm_count); \
	size_t size_ = ((uint8_t*)((array) + (current_count))) - ((uint8_t*)reading_start_addr_); \
	memmove(overwrite_start_addr_, reading_start_addr_, size_); \
}



#define VEC2_OP(a, op, b) ((vec2_t) {     \
	.x = (a).x op (b).x,                  \
	.y = (a).y op (b).y,                  \
})

#define VEC2_OP_B(a, op, n) ((vec2_t) { \
	.x = (a).x op (n),                  \
	.y = (a).y op (n),                  \
})


#define VEC2X_CLAMP(type, v, minx, miny, maxx, maxy) ((type) {          \
	.x = (v).x < (minx) ? (minx) : (v).x > (maxx) ? (maxx) : (v).x,     \
	.y = (v).y < (miny) ? (miny) : (v).y > (maxy) ? (maxy) : (v).y,     \
})


#define VEC2_ADD(a, b)  VEC2_OP(a, +, b)
#define VEC2_SUB(a, b)  VEC2_OP(a, -, b)
#define VEC2_MUL(a, b)  VEC2_OP(a, *, b)
#define VEC2_DIV(a, b)  VEC2_OP(a, /, b)
#define VEC2_DIV_B(a, n) VEC2_OP_B(a, /, n)

#define VEC2_CLAMP(v, minx, miny, maxx, maxy) VEC2X_CLAMP(vec2_t, v, minx, miny, maxx, maxy)
#define VEC2_SCALE(base, scale) VEC2_OP_B(base, *, scale)
#define VEC2_CMP(a, b) (((a).x + (a.y)) - ((b).x + (b.y)))

#define VEC2F_DIV_B(a, n)  VEC2F_OP_B(a, /, n)


#define VEC2_IN_RECT(v, r) (             \
	(v).x >= (r).pos.x &&                \
	(v).x <= ((r).pos.x + (r).size.x) && \
	(v).y >= (r).pos.y &&                \
	(v).y <= ((r).pos.y + (r).size.y)    \
)

#define VEC2_IS_ZERO(v) (((int)(v).x) == 0 && ((int)(v).y) == 0)



#define RECT_CENTER(r) ((vec2_t) {     \
	.x = ((r).pos.x + (r).size.x) / 2, \
	.y = ((r).pos.y + (r).size.y) / 2  \
})





/* 
	RENDER TYPES AND LIMITS
*/

#define WINDOW_W (500)
#define WINDOW_H (570)





/*
	ACTORS TYPES AND LIMITS
*/

#define MAX_ACTORS         (1024)
#define MAX_NICK_LEN       (16)
#define MAX_MSG_LEN        (86)
#define MSG_STACK_COUNT    (8)

#include "krlss.h"

KRL_STATIC_STRING_DEFINE(nick, MAX_NICK_LEN);
KRL_STATIC_STRING_DEFINE(msg, MAX_MSG_LEN);

typedef struct msg_stack_entry {
	krlss_nick_t nick;
	krlss_msg_t msg;
	rgba32_t color;
} msg_stack_entry_t;

typedef enum character_id {
	CHARACTER_ID_KRILLIN,
	CHARACTER_ID_TURTLE,
	CHARACTER_ID_MAX_IDS
} character_id_t;

typedef enum actor_action {
	ACTOR_ACTION_STANDING,
	ACTOR_ACTION_ARRIVING,
	ACTOR_ACTION_DEPARTING,
	ACTOR_ACTION_DELETE
} actor_action_t;

typedef struct actor {
	krlss_nick_t nick;
	krlss_msg_t msg;
	character_id_t char_id;
	actor_action_t action;
	rgba32_t color;
	float speed;
	float move_progress;
	vec2_t pos;
	vec2_t vel;
	vec2_t lerp_points[4];
	timer_t depart_ms;
	timer_t cooldown_ms;
	timer_t depart_timer;
	timer_t cooldown_timer;
} actor_t;



/*
	KRLBOT TYPES AND LIMITS
*/

typedef enum cmd_handler_type {
	CMD_HANDLER_TYPE_STR,
	CMD_HANDLER_TYPE_FN
} cmd_handler_type_t;

typedef void(*cmd_handler_fn_t)(actor_t* a, const char* line, size_t len);

typedef struct cmd_handler {
	cmd_handler_type_t type;
	union {
		cmd_handler_fn_t fn;
		const char* str;
	};
} cmd_handler_t;


#define CMD_HANDLER_STR(val)  {.type = CMD_HANDLER_TYPE_STR, .str = val}
#define CMD_HANDLER_FN(val)   {.type = CMD_HANDLER_TYPE_FN, .fn = val}



#endif