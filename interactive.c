#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#include "camera.h"
#include "color.h"
#include "scene.h"

#define TRY(IT) \
if ((IT)) { SDL_LogError(SDL_LOG_CATEGORY_ERROR, __FILE__ ":%d: %s\n", __LINE__, SDL_GetError()); \
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

SDL_PixelFormat *pf = NULL;
int pitch = -1;

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= window_width || y >= window_height) return;
#if 0
    SDL_Surface *surface = data;
#endif
#if 0
    *(uint32_t *)(((unsigned char *)surface->pixels) + x * surface->format->BytesPerPixel + y * surface->pitch) = SDL_MapRGB(surface->format, r, g, b);
#else
    unsigned char *pipi = data + x * pf->BytesPerPixel + y * pitch;
    *(uint32_t *)pipi = SDL_MapRGB(pf, r, g, b);
#endif
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

    SDL_Renderer *renderer;
    TRY(!(renderer = SDL_CreateRenderer(window, -1, 0)));

    uint32_t pixel_format = 0;
    {
        SDL_RendererInfo info = {0};
        TRY(SDL_GetRendererInfo(renderer, &info));
        SDL_assert(info.num_texture_formats > 0);
        // XXX We picking the first pixel format. But we should search
        // for the best?
        pixel_format = info.texture_formats[0];
    }

    TRY(!(pf = SDL_AllocFormat(pixel_format)));
#if 0
    SDL_Surface *surface = NULL;
    TRY(!(surface = SDL_GetWindowSurface(window)));
#endif
    SDL_Texture *texture = NULL;
    TRY(!(texture = SDL_CreateTexture(renderer,
                    pixel_format, SDL_TEXTUREACCESS_STREAMING,
                    window_width, window_height)));
#if 0
    bool *used = UNCHECKED(calloc(window_width * window_height, sizeof (bool)));
#endif
    // TODO Deallocate shit. Double check shit.

    for (bool is_playing = true, start = true, step = false; is_playing; )
    {
        static bool is_linear = true;
        static float offset_x = 0;
        static float offset_z = 0;

        for (SDL_Event event; SDL_PollEvent(&event); )
        {
            if (SDL_QUIT == event.type)
                is_playing = false;
            else if (SDL_KEYDOWN == event.type && 'q' == event.key.keysym.sym)
                TRY(SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT}) < 0);
	    else if (window_resize_maybe(event, &window_width, &window_height))
            {
                SDL_DestroyTexture(texture);
                TRY(!(texture = SDL_CreateTexture(renderer,
                                pixel_format, SDL_TEXTUREACCESS_STREAMING,
                                window_width, window_height)));
#if 0
                free(used);
                used = UNCHECKED(calloc(window_width * window_height, sizeof (bool)));
#endif
#if 0
                TRY(!(surface = SDL_GetWindowSurface(window)));
#endif
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'i' == event.key.keysym.sym)
            {
                is_linear = true;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'r' == event.key.keysym.sym)
            {
                is_linear = false;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'd' == event.key.keysym.sym)
            {
                offset_x++;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'a' == event.key.keysym.sym)
            {
                offset_x--;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'w' == event.key.keysym.sym)
            {
                offset_z++;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 's' == event.key.keysym.sym)
            {
                offset_z--;
                start = true;
            }
        }

	// XXX check if sdl has lalalal
	// XXX check if lock needed
	// XXX add fullscreen

	static struct camera camera;

        if (start)
        {
#if 0
            SDL_FillRect(surface, NULL, 0);
#endif
            camera = scenes[2].camera;
#if 0
	    camera.vfov = 90;
	    camera.samples_per_pixel = 10;
	    camera.max_depth = 10;
#endif
	    camera.samples_per_pixel = 1;
            camera.image_width = window_width;
            camera.desired_aspect_ratio = window_width / (dist)window_height;
#if 0
	    camera.udata = surface;
#endif
	    camera.log = false;
	    camera.set_pixel = set_pixel;
	    camera.world = &scenes[2].world;

            camera.lookfrom.x += offset_x;
            camera.lookfrom.z += offset_z;

            camera_init(&camera);

	    start = false;
	    step = true;
	}

	if (step)
	{
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
            SDL_RenderClear(renderer);
#if 0
            TRY(SDL_LockSurface(surface));
#endif
            void *pixels = NULL;
            TRY(SDL_LockTexture(texture, NULL, &pixels, &pitch));
            memset(pixels, 0, pitch * window_height); 
            camera.udata = pixels;

            #define DESIRED_FRAME 1000 / (dist)24
            uint64_t timeout = SDL_GetTicks64() + DESIRED_FRAME;
            while (SDL_GetTicks64() < timeout)
                step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, 100);

            // Do some shit. lazy ass stupid bitch thing.

#if 0
            for (int j = 0; j < window_height; j++)
            {
                for (int i = 0; i < window_width; i++)
                {
                }
            }

            memset(used, 0, sizeof (bool) * window_height * window_width);
#endif

#if 0
            SDL_UnlockSurface(surface);
#else
            SDL_UnlockTexture(texture);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
#endif
	}

        SDL_RenderPresent(renderer);
#if 0
        SDL_UpdateWindowSurface(window);
#endif
    }

    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    return 0;
}
