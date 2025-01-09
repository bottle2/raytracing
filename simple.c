// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#define TRY(IT) \
if ((IT)) { SDL_LogError(SDL_LOG_CATEGORY_ERROR, \
            __FILE__ ":%d: %s\n", __LINE__, SDL_GetError()); \
            exit(EXIT_FAILURE); } else (void)0

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    atexit(SDL_Quit);

    TRY(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Point *nothing = NULL;

    TRY(!(window = SDL_CreateWindow(
        "Resize and texture test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        SDL_WINDOW_RESIZABLE
    )));

    TRY(!(renderer = SDL_CreateRenderer(window, -1, 0)));

    uint32_t pixel_format;
    {
        SDL_RendererInfo info = {0};
        TRY(SDL_GetRendererInfo(renderer, &info));
	SDL_Log("%s", info.name);
        SDL_assert(info.num_texture_formats > 0);
        pixel_format = info.texture_formats[0];
    }

    TRY(!(texture = SDL_CreateTexture(renderer,
                    pixel_format, SDL_TEXTUREACCESS_STREAMING,
                    800, 600)));
    for (;;)
    {
        for (SDL_Event event; SDL_PollEvent(&event); )
        {
            bool once = false;
            char **start = (char*[]){"Start of burst\n", ""};
            if (SDL_QUIT == event.type)
                goto end;
            else if (SDL_RENDER_TARGETS_RESET == event.type)
                SDL_Log("%sRender targets reset\n", start[once++]);
            else if (SDL_RENDER_DEVICE_RESET == event.type)
                SDL_Log("%sDevice reset\n", start[once++]);
            else if (SDL_WINDOWEVENT == event.type)
            {
                if (SDL_WINDOWEVENT_RESIZED == event.window.event)
                    SDL_Log("%sWindow resized\n", start[once++]);
                else if (SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event)
                    SDL_Log("%sSize changed\n", start[once++]);
                else continue;
		if (nothing) free(nothing);
		nothing = malloc(sizeof (SDL_Point) * event.window.data1 * event.window.data2);
                SDL_DestroyTexture(texture);
                TRY(!(texture = SDL_CreateTexture(renderer,
                                pixel_format, SDL_TEXTUREACCESS_STREAMING,
                                event.window.data1, event.window.data2)));
            }
            // SDL_RENDER_DEVICE_LOST is not present in SDL2.
        }

        SDL_SetRenderDrawColor(renderer, rand() / (float)RAND_MAX * 100, 0, 0, 1);
        SDL_RenderClear(renderer);

        void *pixels;
        int pitch;
        TRY(SDL_LockTexture(texture, NULL, &pixels, &pitch));
        unsigned char *pipixels = pixels;
        for (int i = 0; i < 100; i++)
            pipixels[rand() / (float)RAND_MAX * pitch * 

        SDL_RenderPresent(renderer);
    }

end:
    if (nothing) free(nothing);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    return 0;
}
