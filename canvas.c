// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL.h>

#include "canvas.h"
#include "util.h"

#ifdef LIMIT_WIDTH
#define ADJUST_WIDTH(W) \
if ((W) > (LIMIT_WIDTH)) (W) = (LIMIT_WIDTH); else (void)0
#else
#define ADJUST_WIDTH(...) (void)0
#endif

#ifdef LIMIT_HEIGHT
#define ADJUST_HEIGHT(H) \
if ((H) > (LIMIT_HEIGHT)) (H) = (LIMIT_HEIGHT); else (void)0
#else
#define ADJUST_HEIGHT(...) (void)0
#endif

static uint32_t pixel_format;
SDL_PixelFormat *pf = NULL;
int max_width;
int max_height;

static void death(void) { SDL_FreeFormat(pf); }

void canvas_setup(SDL_Renderer *renderer)
{
    SDL_RendererInfo info = {0};
    TRY(SDL_GetRendererInfo(renderer, &info));
    SDL_Log("%s", info.name);
    SDL_assert(info.num_texture_formats > 0);
    // XXX We picking the first pixel format. But should we search
    // for the best?
    pixel_format = info.texture_formats[0];
    max_width = info.max_texture_width;
    max_height = info.max_texture_height;
    ADJUST_WIDTH(max_width);
    ADJUST_HEIGHT(max_height);
    SDL_Log("max is %d %d", max_width, max_height);
    TRY(!(pf = SDL_AllocFormat(pixel_format)));
    SDL_assert(BITS_PER_PIXEL == pf->BitsPerPixel);
    atexit(death);
}

void canvas_init(struct canvas canvas[static 1], SDL_Renderer *renderer)
{
    #define X(T, N, M) \
    TRYE(!((canvas->N) = malloc(max_width*max_height * sizeof (*(canvas->N)) * (M))))
    CANVAS_XS;
    #undef X
    canvas->used2 = canvas->used + max_width*max_height;
    canvas_create_texture(canvas, renderer);
    TRYE(!(canvas->progress = malloc(sizeof (*canvas->progress) * max_height)));
}

void canvas_end(struct canvas canvas[static 1])
{
    #define X(T, N, M) free(canvas->N)
    CANVAS_XS;
    #undef X
    SDL_DestroyTexture(canvas->texture);
}

void canvas_create_texture(struct canvas canvas[static 1], SDL_Renderer *renderer)
{
    TRY(!(canvas->texture = SDL_CreateTexture(
        renderer,
        pixel_format, SDL_TEXTUREACCESS_STREAMING,
        max_width, max_height
    )));
}
