#include <time.h>




extern float frame_delta;


static int actor_count;
static actor_t actor_pool[MAX_ACTORS];
static msg_stack_entry_t msg_stack[MSG_STACK_COUNT];


static const rect_t chichi_arriving_frames[] = {
	{ { 0, 0 },    { 40, 62 } },
	{ { 41, 0 },   { 38, 62 } },
	{ { 80, 0 },   { 38, 62 } },
	{ { 117, 0 },  { 38, 62 } },
	{ { 158, 32 }, { 38, 47 } },
	{ { 203, 32 }, { 38, 47 } }
};

static const rect_t chichi_standing_frames[] = {
	{ {  0, 79 },  { 22, 61 } },
	{ { 25, 79 }, { 22, 61 } },
	{ { 54, 79 },   { 22, 61 } },
	{ { 79, 79 },   { 22, 61 } },
	{ { 102, 79 },   { 22, 61 } },
	{ { 127, 79 },   { 22, 61 } },
	{ { 156, 79 },   { 22, 61 } },
	{ { 181, 79 },   { 22, 61 } }
};

static const rect_t chichi_departing_frames[] = {
	{ { 0, 171 },  { 37, 47 } },
	{ { 42, 157 },   { 33, 61 } },
	{ { 80, 140 },   { 33, 61 } },
	{ { 117, 140 },  { 33, 61 } },
	{ { 154, 140 }, { 33, 61 } }
};

static const animation_frames_t char_animations_frames[CHARACTER_ID_MAX_IDS][ANIMATION_ID_MAX_IDS] = {
	[CHARACTER_ID_CHICHI] = {
		[ANIMATION_ID_ARRIVING] = {
			.rects = chichi_arriving_frames,
			.count = STATIC_ARRAY_COUNT(chichi_arriving_frames),
			.ms = (1000 / STATIC_ARRAY_COUNT(chichi_arriving_frames))
		},
		[ANIMATION_ID_STANDING] = {
			.rects = chichi_standing_frames,
			.count = STATIC_ARRAY_COUNT(chichi_standing_frames),
			.ms = 250
		},
		[ANIMATION_ID_DEPARTING] = {
			.rects = chichi_departing_frames,
			.count = STATIC_ARRAY_COUNT(chichi_departing_frames),
			.ms = (2000 / STATIC_ARRAY_COUNT(chichi_departing_frames))
		},
	}
};



void actors_init(void)
{
	srand(time(NULL));
}

void actors_term(void)
{
}

actor_t* actors_find(const char* nick, size_t nicklen)
{
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	actor_t* actor = NULL;
	for (int i = 0; i < actor_count; ++i) {
		if (krlss_ncmp_cstr(actor_pool[i].nick, nick, nicklen) == 0) {
			actor = &actor_pool[i];
			break;
		}
	}
	return actor;
}

actor_t* actors_add(const char* nick, size_t nicklen)
{
	nicklen = nicklen > MAX_NICK_LEN ? MAX_NICK_LEN : nicklen;
	
	if (actor_count >= MAX_ACTORS) {
		printf("ACTOR POOL IS FULL\n");
		return NULL;
	}
	
	actor_t* actor = &actor_pool[actor_count++];
	
	actor->char_id = rand() % CHARACTER_ID_MAX_IDS;
	
	krlss_assign_ex(&actor->nick, nick, nicklen);
	
	actor->color = (rgba32_t) {
		127 + (rand() % (255 - 127)),
		127 + (rand() % (255 - 127)),
		127 + (rand() % (255 - 127)),
		0xFF
	};
	
	actor->animation.frames = char_animations_frames[actor->char_id][ANIMATION_ID_ARRIVING];
	actor->animation.timer = get_timer();
	actor->animation.idx = 0;
	
	const vec2_t start_pos = VEC2((80 + rand() % (WINDOW_W - 80)), -100);
	const vec2_t end_pos = VEC2(28 + (rand() % 465), 406 + (rand() % 12));
	actor->action = ACTOR_ACTION_ARRIVING;
	actor->depart_timer = get_timer();
	actor->speed = 218.0f;
	actor->vel = VEC2(0, 0);
	actor->pos = start_pos;
	actor->move_progress = 0.0f;
	actor->lerp_points[0] = start_pos;
	actor->lerp_points[3] = end_pos;
	actor->lerp_points[1] = VEC2(start_pos.x < (WINDOW_W / 2.0f) ? WINDOW_W + 100 : 0, end_pos.y / 4.0f);
	actor->lerp_points[2] = VEC2(start_pos.x < (WINDOW_W / 2.0f) ? -100 : WINDOW_W, end_pos.y / 2.0f);
	
	render_play_common_sfx(COMMON_SFX_ID_LANDING);	
	return actor;
}

bool actors_set_actor_msg(actor_t* actor, const char* msg, size_t msglen)
{
	msglen = msglen > MAX_MSG_LEN ? MAX_MSG_LEN : msglen;
	
	if (actor->action == ACTOR_ACTION_DEPARTING)
		return false;
	
	if ((get_timer() - actor->cooldown_timer) < actor->cooldown_ms) {
		printf("could not set msg\n");
		return false;
	}

	krlss_assign_ex(&actor->msg, msg, msglen);
	
	actor->cooldown_ms = 2500;
	actor->cooldown_timer = get_timer();
	actor->depart_timer = get_timer();
	
	ARRAY_RM_MEMMOVE(
		msg_stack,
		MSG_STACK_COUNT,
		0,
		1
	);
	
	krlss_assign(&msg_stack[MSG_STACK_COUNT - 1].nick, actor->nick);
	krlss_assign(&msg_stack[MSG_STACK_COUNT - 1].msg, actor->msg);
	msg_stack[MSG_STACK_COUNT - 1].color = actor->color;
	
	return true;
}


static inline float vui_lerp(float to, float from, float t) { return (to - from) * t + from; }
static vec2_t vui_cubic_bezier_curve_interp(const vec2_t points[4], float progress) 
{
	vec2_t tmp_buf[4];
	memcpy(tmp_buf, points, sizeof(vec2_t) * 4);

	size_t number_of_points = 4;
	while (number_of_points > 1) {
		for (size_t i = 0; i < number_of_points - 1; ++i) {
			tmp_buf[i].x = vui_lerp(tmp_buf[i + 1].x, tmp_buf[i].x, progress);
			tmp_buf[i].y = vui_lerp(tmp_buf[i + 1].y, tmp_buf[i].y, progress);

		}
		number_of_points -= 1;
	}

	return tmp_buf[0];
}

static void arriving_action_update(actor_t* actor)
{
	actor->move_progress += 1.0f * frame_delta;
	if (actor->move_progress >= 1.0f) {
		actor->move_progress = 1.0f;
		actor->action = ACTOR_ACTION_STANDING;
		actor->animation.frames = char_animations_frames[actor->char_id][ANIMATION_ID_STANDING];
		actor->animation.idx = 0;
		actor->animation.timer = get_timer();
	}
	actor->pos = vui_cubic_bezier_curve_interp(actor->lerp_points, actor->move_progress);
}

static void departing_action_update(actor_t* actor)
{
	actor->move_progress += (1.0f * frame_delta) / 2.0f;
	if (actor->move_progress >= 1.0f) {
		actor->move_progress = 1.0f;
		actor->action = ACTOR_ACTION_DELETE;
	}
	actor->pos = vui_cubic_bezier_curve_interp(actor->lerp_points, actor->move_progress);
}

static void standing_action_update(actor_t* a)
{
	if ((get_timer() - a->depart_timer) > 60 * 1000) {
		a->lerp_points[0] = a->pos;
		a->lerp_points[1] = VEC2(rand() % (250), a->pos.y - (rand() % 250));
		a->lerp_points[2] = VEC2(WINDOW_W - (rand() % 250), a->pos.y - 250 + (rand() % 350));
		a->lerp_points[3] = VEC2(a->pos.x, -80);
		a->move_progress = 0.0f;
		a->action = ACTOR_ACTION_DEPARTING;
		a->animation.frames = char_animations_frames[a->char_id][ANIMATION_ID_DEPARTING];
		a->animation.idx = 0;
		a->animation.timer = get_timer();
		return;
	}
}



void actors_update(void)
{
	timer_t now = get_timer();
	for (int i = 0; i < actor_count; ++i) {
		actor_t* a = &actor_pool[i];
		if ((now - a->animation.timer) >= a->animation.frames.ms) {
			a->animation.idx += 1;
			a->animation.idx %= a->animation.frames.count;
			a->animation.timer = now;
		}
		switch (a->action) {
		case ACTOR_ACTION_ARRIVING: arriving_action_update(a); break;
		case ACTOR_ACTION_STANDING: standing_action_update(a); break;
		case ACTOR_ACTION_DEPARTING: departing_action_update(a); break;
		default: break;
		}
	}
	
	for (int i = 0; i < actor_count; ++i) {
		actor_t* a = &actor_pool[i];
		if (a->action == ACTOR_ACTION_DELETE) {
			ARRAY_RM_MEMMOVE(actor_pool, actor_count, i, 1);
			--i;
			--actor_count;
			continue;
		}
	}

}

void actors_render(void)
{
	render_draw_actors(actor_pool, actor_count);
	render_draw_msg_stack(&msg_stack[0], MSG_STACK_COUNT);
}
