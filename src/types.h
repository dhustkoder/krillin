#ifndef KRL_TYPES_H_
#define KRL_TYPES_H_
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>
#include <SDL.h>

#define WINDOW_W (500)
#define WINDOW_H (570)

#define MAX_USERS    (1024)
#define MAX_NICK_LEN (16)
#define MAX_MSG_LEN  (256)
#define MAX_PKT_SIZE (1024)


typedef Uint32 timer_t;
#define get_timer() SDL_GetTicks()




typedef enum character_id {
	CHARACTER_ID_KRILLIN,
	CHARACTER_ID_TURTLE,
	CHARACTER_ID_MAX_IDS
} character_id_t;

typedef enum packet_types {
	PKT_TYPE_NONE,
	PKT_TYPE_TWITCH_PING,
	PKT_TYPE_USER_MSG
} pkt_type_t;

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

typedef struct actor {
	char nick[MAX_NICK_LEN + 1];
	char msg[MAX_MSG_LEN + 1];
	character_id_t char_id;
	rgba32_t color;
	vec2_t pos;
	timer_t cooldown_ms;
	timer_t msg_display_ms;
	timer_t cooldown_timer;
	timer_t msg_display_timer;
} actor_t;


typedef struct krln_socket {
	uint64_t handle;
	uint8_t* recv_data;
	size_t recv_size;
	size_t recv_reserved;
} krln_socket_t;

typedef struct krlbot_response {
	char msg[MAX_MSG_LEN + 1];
	int len;
} krlbot_response_t;

typedef void(*command_clbk_fn_t)(actor_t* actor, krlbot_response_t* resp);

typedef struct command_clbk_pair {
	const char* cmd;
	command_clbk_fn_t clbk;
} command_clbk_pair_t;


#define VEC2(...) ((vec2_t){ __VA_ARGS__ })
#define RECT(...) ((rect_t){ __VA_ARGS__ })
#define STATIC_ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))










#endif