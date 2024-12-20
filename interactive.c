#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#include "camera.h"
#include "color.h"
#include "scene.h"

#define TRY(IT) \
if ((IT)) { SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError()); \
            exit(EXIT_FAILURE); } else (void)0

static inline bool window_resize_maybe(SDL_Event event, int *width, int *height)
{
    if (event.type != SDL_WINDOWEVENT) return false;
    if (SDL_WINDOWEVENT_RESIZED      != event.window.event &&
        SDL_WINDOWEVENT_SIZE_CHANGED != event.window.event) return false;

    *width  = event.window.data1;
    *height = event.window.data2;

    return true;
}

static int window_width  = 800;
static int window_height = 600;

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= window_width || y >= window_height) return;
    SDL_Surface *surface = data;
    *(uint32_t *)(((unsigned char *)surface->pixels) + x * surface->format->BytesPerPixel + y * surface->pitch) = SDL_MapRGB(surface->format, r, g, b);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    scenes_init();

    atexit(SDL_Quit);

    TRY(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window *window = NULL;

    TRY(!(window = SDL_CreateWindow(
        "Toy Raytracing",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_RESIZABLE
    )));
    // TODO Deal with Apple's high-DPI stuff.

    SDL_Surface *surface = NULL;
    TRY(!(surface = SDL_GetWindowSurface(window)));

    for (bool is_playing = true, start = true, step = false; is_playing; )
    {
        for (SDL_Event event; SDL_PollEvent(&event); )
        {
            if (SDL_QUIT == event.type)
                is_playing = false;
            else if (SDL_KEYDOWN == event.type && 'q' == event.key.keysym.sym)
                TRY(SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT}) < 0);
	    else if (window_resize_maybe(event, &window_width, &window_height))
            {
                TRY(!(surface = SDL_GetWindowSurface(window)));
                start = true;
            }
        }

	// XXX check if sdl has lalalal
	// XXX check if lock needed
	// XXX add fullscreen

	static struct camera camera;

        if (start)
        {
            camera = scenes[2].camera;
#if 0
	    camera.vfov = 90;
	    camera.samples_per_pixel = 10;
	    camera.max_depth = 10;
#endif
            camera.image_width = window_width;
            camera.desired_aspect_ratio = window_width / (dist)window_height;
	    camera.udata = surface;
	    camera.log = false;
	    camera.set_pixel = set_pixel;
	    camera.world = &scenes[2].world;

            camera_init(&camera);

	    start = false;
	    step = true;
	}

	if (step)
	{
            TRY(SDL_LockSurface(surface));

            #define DESIRED_FRAME 1000 / (dist)24
            uint64_t timeout = SDL_GetTicks64() + DESIRED_FRAME;
            while (SDL_GetTicks64() < timeout)
                step = !camera_step(&camera, 100);

            SDL_UnlockSurface(surface);
	}

        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    return 0;
}
