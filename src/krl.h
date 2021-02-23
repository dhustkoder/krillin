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


#define clamp(x, min, max) ((x) < (min) ? (min) : (x) > (max) ? (max) : (x))


/*
	VPAD
*/

extern void virtual_pad_init(void);
extern void virtual_pad_term(void);
extern void virtual_pad_update(void);



/*
	ACTORS
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
	CHARACTER_ID_CHICHI,
	CHARACTER_ID_MAX_IDS
} character_id_t;

typedef enum actor_action {
	ACTOR_ACTION_STANDING,
	ACTOR_ACTION_ARRIVING,
	ACTOR_ACTION_DEPARTING,
	ACTOR_ACTION_DELETE
} actor_action_t;

typedef enum animation_id {
	ANIMATION_ID_ARRIVING,
	ANIMATION_ID_STANDING,
	ANIMATION_ID_DEPARTING,
	ANIMATION_ID_MAX_IDS,
} animation_id_t;

typedef struct animation_frames {
	const rect_t* rects;
	size_t count;
	timer_t ms;
} animation_frames_t;

typedef struct animation {
	animation_frames_t frames;
	size_t idx;
	timer_t timer;
} animation_t;

typedef struct actor {
	krlss_nick_t nick;
	krlss_msg_t msg;
	character_id_t char_id;
	actor_action_t action;
	animation_t animation;
	rgba32_t color;
	float speed;
	float move_progress;
	vec2_t pos;
	vec2_t vel;
	vec2_t lerp_points[4];
	timer_t cooldown_ms;
	timer_t depart_timer;
	timer_t cooldown_timer;
} actor_t;


extern void actors_init(void);
extern void actors_term(void);
extern actor_t* actors_find(const char* nick, size_t nicklen);
extern actor_t* actors_add(const char* nick, size_t nicklen);
extern bool actors_set_actor_msg(actor_t* actor, const char* msg, size_t msglen);
extern void actors_update(void);
extern void actors_render(void);


/* 
	RENDER
*/

#define WINDOW_W (500)
#define WINDOW_H (570)

typedef enum {
	COMMON_SFX_ID_LANDING,
	COMMON_SFX_ID_IDCOUNT
} common_sfx_id_t;

extern void render_init(void);
extern void render_term(void);
extern bool render_poll_events(void);
extern void render_draw_actors(actor_t* actors, int count);
extern void render_play_dialog_sfx(actor_t* actor);
extern void render_play_common_sfx(common_sfx_id_t id);

extern void render_draw_msg_stack(
	const msg_stack_entry_t* entries,
	size_t count
);

extern void render_clear(void);
extern void render_flush(void);




/*
	KRLNET
*/

extern void krlnet_init(void);
extern void krlnet_term(void);
extern void krlnet_connect(const char* url, uint16_t port);
extern void krlnet_write(const char* fmt, ...);
extern size_t krlnet_readline(char* buffer, size_t maxlen);


/*
	KRLBOT
*/

typedef enum cmd_handler_type {
	CMD_HANDLER_TYPE_STR,
	CMD_HANDLER_TYPE_FN
} cmd_handler_type_t;

typedef struct command_info {
	actor_t* actor;
	const char* cmd;
	size_t cmd_len;
	const char* args;
	size_t args_len;
} cmd_info_t;

typedef void(*cmd_handler_fn_t)(cmd_info_t* info);

typedef struct cmd_handler {
	cmd_handler_type_t type;
	const char* cmd;
	union {
		cmd_handler_fn_t fn;
		const char* str;
	};
} cmd_handler_t;


#define CMD_HANDLER_STR(cmdstr, val)  {.cmd = cmdstr, .type = CMD_HANDLER_TYPE_STR, .str = val}
#define CMD_HANDLER_FN(cmdstr, val)   {.cmd = cmdstr, .type = CMD_HANDLER_TYPE_FN, .fn = val}

#define VPAD_DEFAULT_HANDLERS_COUNT (9)
const cmd_handler_t vpad_default_handlers[VPAD_DEFAULT_HANDLERS_COUNT];



extern void krlbot_init(void);
extern void krlbot_update(void);
extern void krlbot_term(void);



#endif
