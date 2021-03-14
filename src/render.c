#include <SDL.h>
#include <SDL_mixer.h>
#include "stb_image.h"
#include "krl.h"

#define BPFNT_BIG_TABLE_W (160)
#define BPFNT_BIG_TABLE_H (192)

#define BPFNT_BIG_CHAR_W (10)
#define BPFNT_BIG_CHAR_H (12)

#define BPFNT_BIG_CHAR_DRAW_W (8)
#define BPFNT_BIG_CHAR_DRAW_H (10)

#define BPFNT_BIG_H_CHARS (BPFNT_BIG_TABLE_W / BPFNT_BIG_CHAR_W)
#define BPFNT_BIG_V_CHARS (BPFNT_BIG_TABLE_H / BPFNT_BIG_CHAR_H)

#define SDLRECT(rect) ((SDL_Rect){.x=rect.pos.x, .y=rect.pos.y,.w=rect.size.x,.h=rect.size.y})

#define BKG_FRAMES (6)
#define BKG_FRAME_DELAY (50)
#define BKG_W (WINDOW_W)
#define BKG_H (WINDOW_H)


static SDL_Window* win;
static SDL_Renderer* rend;
static SDL_Texture* target_tex;
static SDL_Texture* bpf_big_tex;
static SDL_Texture* sprites_tex;
static SDL_Texture* bkg_pic_tex;

static timer_t bkg_timer;
static int bkg_frame_idx;

static const char* common_sfx_files[COMMON_SFX_ID_IDCOUNT] = {
	[COMMON_SFX_ID_LANDING] = "landing.ogg",
};

static Mix_Chunk* common_sfx_chunks[COMMON_SFX_ID_IDCOUNT];


static SDL_Texture* load_png(const char* file)
{
	int w, h, c;
	void* data;
	
	data = stbi_load(
		file,
		&w,
		&h,
		&c,
		STBI_rgb_alpha
	);
	
	SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormatFrom(
		data,
		w,
		h,
		32,
		4 * w,
		SDL_PIXELFORMAT_RGBA32
	);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
	
	assert(tex != NULL);
	
	SDL_FreeSurface(surf);
	stbi_image_free(data);
	
	return tex;
}

void render_init(void)
{
	int ret;
	ret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER);
	assert(ret == 0);
	
	ret = Mix_OpenAudio(
		MIX_DEFAULT_FREQUENCY,
		MIX_DEFAULT_FORMAT,
		MIX_DEFAULT_CHANNELS,
		4096
	);
	
	assert(ret == 0);

	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	win = SDL_CreateWindow(
		"KrillinBot",
		dm.w - WINDOW_W,
		0,
		WINDOW_W,
		WINDOW_H,
		SDL_WINDOW_BORDERLESS
	);
	assert(win != NULL);
	
	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	assert(rend != NULL);
	
	target_tex = SDL_CreateTexture(
		rend,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_TARGET,
		WINDOW_W,
		WINDOW_H
	);
	
	SDL_SetRenderTarget(rend, target_tex);
	
	sprites_tex = load_png("sprites.png");
	bpf_big_tex = load_png("bpfnt_big.png");
	bkg_pic_tex = load_png("bkg.png");
	
	SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
	
	
	for (size_t i = 0; i < STATIC_ARRAY_COUNT(common_sfx_files); ++i) {
		common_sfx_chunks[i] = Mix_LoadWAV(common_sfx_files[i]);
		assert(common_sfx_chunks[i] != NULL);
	}

	
	bkg_timer = get_timer();
}

void render_term(void)
{
	for (size_t i = 0; i < STATIC_ARRAY_COUNT(common_sfx_chunks); ++i)
		Mix_FreeChunk(common_sfx_chunks[i]);
	
	Mix_CloseAudio();
	
	SDL_DestroyTexture(bkg_pic_tex);
	SDL_DestroyTexture(bpf_big_tex);
	SDL_DestroyTexture(sprites_tex);
	SDL_DestroyTexture(target_tex);
	SDL_DestroyRenderer(rend);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

bool render_poll_events(void)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT: return false;
		case SDL_KEYDOWN:
			actors_add("test", 4);
			break;
		}
	}
	return true;
}

void render_draw_actors(actor_t* actors, int count)
{
	for (int i = 0; i < count; ++i) {
		SDL_Rect src = SDLRECT(actors[i].animation.frames.rects[actors[i].animation.idx]);
		SDL_Rect dst = {
			.x = actors[i].pos.x - (48 / 2.0f),
			.y = actors[i].pos.y - (48 / 2.0f),
			.w = actors[i].animation.frames.rects[actors[i].animation.idx].size.x,
			.h = actors[i].animation.frames.rects[actors[i].animation.idx].size.y
		};
		SDL_RenderCopy(
			rend,
			sprites_tex,
			&src,
			&dst
		);
	}
}



void render_play_dialog_sfx(actor_t* user)
{
//	Mix_PlayChannel(-1, character_sfx_chunks[user->char_id], 0);
}

void render_play_common_sfx(common_sfx_id_t id)
{
	Mix_PlayChannel(-1, common_sfx_chunks[id], 0);
}

static void render_print_text_char(char c, SDL_Rect dst)
{
	const rect_t table_pos = RECT(
		VEC2(
			(c % BPFNT_BIG_H_CHARS) * BPFNT_BIG_CHAR_W,
			(c / BPFNT_BIG_H_CHARS) * BPFNT_BIG_CHAR_H
		),
		VEC2(
			BPFNT_BIG_CHAR_DRAW_W,
			BPFNT_BIG_CHAR_DRAW_H
		)
	);
	SDL_RenderCopy(
		rend,
		bpf_big_tex,
		&SDLRECT(table_pos),
		&dst
	);
}

static void render_text(const char* str, rect_t dest)
{
	SDL_Rect text_dst = {
		.x = dest.pos.x,
		.y = dest.pos.y,
		.w = BPFNT_BIG_CHAR_DRAW_W,
		.h = BPFNT_BIG_CHAR_DRAW_H
	};
	
	const char* p = str;
	int lines = 0;
	while (*p != '\0') {
		const char c = *p++;
		if (
			c == '\n' ||
			c == '\r'
		) {
			continue;
		}
		
		render_print_text_char(c, text_dst);
		text_dst.x += BPFNT_BIG_CHAR_DRAW_W;
		if ((text_dst.x + BPFNT_BIG_CHAR_DRAW_W) >= dest.size.x) {
			render_print_text_char('-', text_dst);
			
			if (*p == ' ')
				++p;
			
			text_dst.x = 0;
			text_dst.y += BPFNT_BIG_CHAR_DRAW_H + 2;
			++lines;
			if (lines > 1)
				break;
		}
	}
}

void render_draw_msg_stack(
	const msg_stack_entry_t* entries,
	size_t count
)
{
	rect_t dest = RECT(VEC2(0, 0), VEC2(WINDOW_W, (count + 1) * (BPFNT_BIG_CHAR_DRAW_H + 2) * 2));
	SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xA0);
	SDL_RenderFillRect(rend, &SDLRECT(dest));
	SDL_SetRenderDrawColor(rend, 0xFF, 0xFF, 0xFF, 0xFF);
	dest.pos.y = count * (BPFNT_BIG_CHAR_DRAW_H + 2) * 2;
	
	const msg_stack_entry_t* entry = &entries[count - 1];
	for (size_t i = 0; i<count; ++i, --entry) {
		if (entry->nick.len == 0 || entry->msg.len == 0)
			continue;
		
		SDL_SetTextureColorMod(bpf_big_tex, entry->color.r, entry->color.g, entry->color.b);
		render_text(entry->nick.data, dest);
		dest.pos.x += entry->nick.len * BPFNT_BIG_CHAR_DRAW_W;
		render_text(": ", dest);
		dest.pos.x += strlen(": ") * BPFNT_BIG_CHAR_DRAW_W;
		render_text(entry->msg.data, dest);
		dest.pos.y -= (BPFNT_BIG_CHAR_DRAW_H + 2) * 2;
		dest.pos.x = 0;
	}
}


void render_clear(void)
{
	SDL_SetRenderTarget(rend, target_tex);
	
	SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderClear(rend);
	
	SDL_Rect bkg_rect = { .y = 0, .w = BKG_W, .h = BKG_H };
	if ((get_timer() - bkg_timer) > BKG_FRAME_DELAY) {
		bkg_frame_idx = (bkg_frame_idx + 1) % BKG_FRAMES;
		bkg_timer = get_timer();
	}
	
	bkg_rect.x = bkg_frame_idx * BKG_W;
	
	SDL_RenderCopy(rend, bkg_pic_tex, &bkg_rect, NULL);
}

void render_flush(void)
{
	SDL_SetRenderTarget(rend, NULL);
	SDL_RenderCopy(rend, target_tex, NULL, NULL);
	SDL_RenderPresent(rend);
}


