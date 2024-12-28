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
    pixel *pixels;
};

extern uint32_t pixel_format;
extern SDL_PixelFormat *pf;
// These are undefined before calling `canvas_setup`.
// These are freed automatically `atexit`.

// MUST be called AFTER `atexit(SDL_Quit)`.
void canvas_setup(SDL_Renderer *renderer);

#endif
