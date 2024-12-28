#include <stdint.h>

#include <SDL.h>

#include "canvas.h"
#include "util.h"

uint32_t pixel_format;
SDL_PixelFormat *pf = NULL;

static void death(void) { SDL_FreeFormat(pf); }

void canvas_setup(SDL_Renderer *renderer)
{
    SDL_RendererInfo info = {0};
    TRY(SDL_GetRendererInfo(renderer, &info));
    SDL_Log("%s", info.name);
    SDL_assert(info.num_texture_formats > 0);
    // XXX We picking the first pixel format. But we should search
    // for the best?
    pixel_format = info.texture_formats[0];
    TRY(!(pf = SDL_AllocFormat(pixel_format)));
    SDL_assert(BITS_PER_PIXEL == pf->BitsPerPixel);
    atexit(death);
}
