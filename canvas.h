// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>

#include <SDL.h>

#ifndef BITS_PER_PIXEL
#define BITS_PER_PIXEL 32
#endif
#if 32 == BITS_PER_PIXEL
#define PIXEL_INDEX(PIXELS, I, J, PITCH) \
(((uint32_t *)(((unsigned char *)(PIXELS)) + (J) * (PITCH)))[(I)])
typedef uint32_t pixel;
#else
#error Not implemented
#endif

#define CANVAS_XS \
X(pixel, pixels, 1); X(bool, used, 2); X(SDL_Point, pending, 1)
// TODO I think `pending` can be smaller

struct canvas
{
    enum canvas_fill
    {
        CANVAS_FILL_NONE,
        CANVAS_FILL_LINEAR,
        CANVAS_FILL_RANDOM,
        CANVAS_FILL_RANDOM_HORIONTAL = 6,
        CANVAS_FILL_RANDOM_FLOOD = 10,
    } fill;
    #define X(T, N, M) T *N
    CANVAS_XS;
    #undef X
    bool *used2;
    SDL_Texture *texture;
    _Atomic short *progress;
};

extern SDL_PixelFormat *pf;
extern int max_width;
extern int max_height;
// These are undefined before calling `canvas_setup`.
// These are freed automatically `atexit`.

// MUST be called AFTER `atexit(SDL_Quit)`.
void canvas_setup(SDL_Renderer *renderer);

void canvas_init(struct canvas[static 1], SDL_Renderer *);
void canvas_end (struct canvas[static 1]);
void canvas_create_texture(struct canvas[static 1], SDL_Renderer *);

#endif
