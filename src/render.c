#include <SDL.h>
#include "render.h"
#include "stb_image.h"

static SDL_Window* win;
static SDL_Renderer* rend;
static SDL_Texture* bpf_big_tex;
static SDL_Texture* character_pics_tex;
static SDL_Texture* bkg_pic;

#define BPFNT_BIG_TABLE_W (160)
#define BPFNT_BIG_TABLE_H (192)

#define BPFNT_BIG_CHAR_W (10)
#define BPFNT_BIG_CHAR_H (12)

#define BPFNT_BIG_H_CHARS (BPFNT_BIG_TABLE_W / BPFNT_BIG_CHAR_W)
#define BPFNT_BIG_V_CHARS (BPFNT_BIG_TABLE_H / BPFNT_BIG_CHAR_H)

#define SDLRECT(rect) ((SDL_Rect){.x=rect.pos.x, .y=rect.pos.y,.w=rect.size.x,.h=rect.size.y})

static rect_t character_pics[] = {
	[CHARACTER_ID_KRILLIN] = {
		VEC2(0, 0),
		VEC2(176, 166)
	},
	[CHARACTER_ID_TURTLE] = {
		VEC2(176, 0),
		VEC2(247, 167)
	}
};



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
	ret = SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
	assert(ret == 0);
	
	win = SDL_CreateWindow(
		"KrillinBot",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_W,
		WINDOW_H,
		SDL_WINDOW_RESIZABLE
	);
	assert(win != NULL);
	
	rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
	assert(rend != NULL);
	
	SDL_SetRenderTarget(rend, NULL);
	
	character_pics_tex = load_png("character_pics.png");
	bpf_big_tex = load_png("bpfnt_big.png");
	bkg_pic = load_png("bkg.png");
}

bool render_update(void)
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT: return false;
		}
	}
	return true;
}

void render_draw_users(user_t* users, int count)
{
	for (int i = 0; i < count; ++i) {
		SDL_Rect rect = {
			.x = users[i].pos.x,
			.y = users[i].pos.y,
			.w = 48,
			.h = 48
		};
		SDL_RenderCopy(
			rend,
			character_pics_tex,
			&SDLRECT(character_pics[users[i].char_id]),
			&rect
		);
	}
}

void render_draw_dialog(user_t* user)
{
	rect_t pic_src = character_pics[user->char_id];
	SDL_Rect pic_dst = {
		.x = 0,
		.y = WINDOW_H - (pic_src.size.y / 1.5f),
		.w = pic_src.size.x / 1.5f,
		.h = pic_src.size.y / 1.5f
	};
	
	SDL_SetRenderDrawColor(rend, 0x00, 0x00, 0x00, 0xFF);
	SDL_RenderFillRect(
		rend,
		&SDLRECT(RECT(pic_dst.x, pic_dst.y, WINDOW_W, WINDOW_H))
	);
	
	SDL_RenderCopy(
		rend,
		character_pics_tex,
		&SDLRECT(pic_src),
		&pic_dst
	);
	
	SDL_Rect text_dst = {
		.x = pic_dst.x + pic_dst.w + 8,
		.y = pic_dst.y + 4,
		.w = BPFNT_BIG_CHAR_W,
		.h = BPFNT_BIG_CHAR_H
	};
	const char* p = user->msg;
	while (*p != '\0') {
		const char c = *p++;
		if (
			c == '\n' ||
			c == '\r'
		) {
			continue;
		}
		const rect_t table_pos = RECT(
			VEC2(
				(c % BPFNT_BIG_H_CHARS) * BPFNT_BIG_CHAR_W,
				(c / BPFNT_BIG_H_CHARS) * BPFNT_BIG_CHAR_H
			),
			VEC2(
				BPFNT_BIG_CHAR_W,
				BPFNT_BIG_CHAR_H
			)
		);
		SDL_RenderCopy(
			rend,
			bpf_big_tex,
			&SDLRECT(table_pos),
			&text_dst
		);
		text_dst.x += BPFNT_BIG_CHAR_W;
		if (text_dst.x >= WINDOW_W)
			text_dst.x = (pic_dst.x + pic_dst.w + 8);
	}
}

void render_clear(void)
{
	SDL_SetRenderDrawColor(rend, 0xFF, 0x00, 0xFF, 0xFF);
	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, bkg_pic, NULL, NULL);
}

void render_flush(void)
{
	SDL_RenderPresent(rend);
}


