// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdio.h>

#include <SDL.h>

SDL_Renderer *renderer;
SDL_Window *window = NULL;

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    atexit(SDL_Quit);

    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    window = SDL_CreateWindow(
        "Toy Raytracing",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800, 600,
        SDL_WINDOW_RESIZABLE
    );

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Log("I'm alive\n");

    for (;;)
    {
        for (SDL_Event event; SDL_PollEvent(&event); )
        {
            if (SDL_QUIT == event.type)
            {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
                return 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);

        #define Co rand()*255.0/RAND_MAX
        SDL_SetRenderDrawColor(renderer, Co, Co, Co, 1);
        static SDL_Rect wh = {100,100,100,100};
        SDL_RenderDrawRect(renderer,&wh);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
