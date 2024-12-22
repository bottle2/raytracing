#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "camera.h"
#include "color.h"
#include "scene.h"
#include "util.h"

#define TRY(IT) \
if ((IT)) { SDL_LogError(SDL_LOG_CATEGORY_ERROR, __FILE__ ":%d: %s\n", __LINE__, SDL_GetError()); \
            exit(EXIT_FAILURE); } else (void)0

SDL_Renderer *renderer;
uint32_t pixel_format = 0;
SDL_Texture *texture = NULL;

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

bool *used = NULL;
SDL_Point *pending = NULL;
int pending_i = 0;
int pending_n = 0;
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
    unsigned char *pipi = data;
    pipi += x * pf->BytesPerPixel + y * pitch;
    *(uint32_t *)pipi = SDL_MapRGB(pf, r, g, b);
    used[x + y * window_width] = true;
    pending[pending_n++] = (SDL_Point){x,y};
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

    TRY(!(renderer = SDL_CreateRenderer(window, -1, 0)));

    {
        SDL_RendererInfo info = {0};
        TRY(SDL_GetRendererInfo(renderer, &info));
	SDL_Log("%s", info.name);
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
    TRY(!(texture = SDL_CreateTexture(renderer,
                    pixel_format, SDL_TEXTUREACCESS_STREAMING,
                    window_width, window_height)));
#define FILL_THING
#ifdef FILL_THING
    used = UNCHECKED(calloc(window_width * window_height, sizeof (bool)));
    pending = UNCHECKED(malloc(sizeof (SDL_Point) * window_width * window_height));
#endif
    // TODO Deallocate shit. Double check shit.

    int max_burst = 0;
    int n_targets_reset = 0;
    int n_device_reset = 0;
    int n_device_lost = 0;

    for (bool is_playing = true, start = true, step = false; is_playing; )
    {
        static bool is_linear = true;
        static bool is_horizontal = true;
        static float offset_x = 0;
        static float offset_z = 0;

        int burst = 0;

        for (SDL_Event event; SDL_PollEvent(&event); )
        {
            if (SDL_QUIT == event.type)
                is_playing = false;
            else if (SDL_KEYDOWN == event.type && 'q' == event.key.keysym.sym)
                TRY(SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT}) < 0);
	    else if (window_resize_maybe(event, &window_width, &window_height))
            {
#ifdef FILL_THING
                SDL_DestroyTexture(texture);
                TRY(!(texture = SDL_CreateTexture(renderer,
                                pixel_format, SDL_TEXTUREACCESS_STREAMING,
                                window_width, window_height)));
                // TODO WTF how about realloc HMMM ?
                used = UNCHECKED(realloc(used, window_width * window_height * sizeof (bool)));
		memset(used, 0, sizeof (bool) * window_width * window_height);
                pending = UNCHECKED(realloc(pending, sizeof (SDL_Point) * window_width * window_height));
                pending_i = 0;
                pending_n = 0;
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
            else if (SDL_KEYDOWN == event.type && 'h' == event.key.keysym.sym)
            {
                is_horizontal = true;
                start = true;
            }
            else if (SDL_KEYDOWN == event.type && 'f' == event.key.keysym.sym)
            {
                is_horizontal = false;
                start = true;
            }
            else if (SDL_RENDER_TARGETS_RESET == event.type)
            {
                SDL_DestroyTexture(texture);
                TRY(!(texture = SDL_CreateTexture(renderer,
                                pixel_format, SDL_TEXTUREACCESS_STREAMING,
                                window_width, window_height)));
                burst++;
                n_targets_reset++;
            }
            else if (SDL_RENDER_DEVICE_RESET == event.type)
            {
                SDL_DestroyTexture(texture);
                TRY(!(texture = SDL_CreateTexture(renderer,
                                pixel_format, SDL_TEXTUREACCESS_STREAMING,
                                window_width, window_height)));

                burst++;
                n_device_reset++;
            }
#if 0
            else if (SDL_RENDER_DEVICE_LOST == event.type)
            {
                burst++;
                n_device_lost++;
            }
#endif
        }

        if (burst > max_burst)
            max_burst = burst;

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

            #define DESIRED_FRAME 1000 / (dist)12
            uint64_t timeout = SDL_GetTicks64() + DESIRED_FRAME;
            while (SDL_GetTicks64() < timeout)
                step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, 100);

            // Do some shit. lazy ass stupid bitch thing.

            if (is_horizontal)
#ifdef FILL_THING
            for (int j = 0; j < window_height; j++)
            {
                uint8_t prev_r;
                uint8_t prev_g;
                uint8_t prev_b;
                int last = -1;
                for (int i = 0; i < window_width; i++)
                {
                    if (!used[i + j * window_width])
                        continue;
                    // Copied from callback.
                    unsigned char *pipi = pixels;
                    pipi += i * pf->BytesPerPixel + j * pitch;
                    uint8_t curr_r;
                    uint8_t curr_g;
                    uint8_t curr_b;
                    SDL_GetRGB(*(uint32_t *)pipi, pf, &curr_r, &curr_g, &curr_b);

                    // Ugh
                    if (-1 == last)
                    {
                        for (int k = 0; k < i; k++)
                            *(uint32_t *)(((unsigned char *)pixels) + k * pf->BytesPerPixel + j * pitch) = SDL_MapRGB(pf, curr_r, curr_g, curr_b);
                    }
                    else
                    {
                        for (int k = last; k < (last + i) / 2; k++)
                            *(uint32_t *)(((unsigned char *)pixels) + k * pf->BytesPerPixel + j * pitch) = SDL_MapRGB(pf, prev_r, prev_g, prev_b);
                        for (int k = (last + i) / 2; k < i; k++)
                            *(uint32_t *)(((unsigned char *)pixels) + k * pf->BytesPerPixel + j * pitch) = SDL_MapRGB(pf, curr_r, curr_g, curr_b);
                    }

                    prev_r = curr_r;
                    prev_g = curr_g;
                    prev_b = curr_b;

                    last = i;
                }
                for (; last < window_width; last++)
                    *(uint32_t *)(((unsigned char *)pixels) + last * pf->BytesPerPixel + j * pitch) = SDL_MapRGB(pf, prev_r, prev_g, prev_b);
            }
            else // Do the radial
            {
                for (; pending_i != pending_n; pending_i = (pending_i + 1) % (window_width * window_height))
                {
                    uint8_t r;
                    uint8_t g;
                    uint8_t b;
                    unsigned char *pipi = pixels;
                    pipi += pending[pending_i].x * pf->BytesPerPixel + pending[pending_i].y * pitch;
                    SDL_GetRGB(*(uint32_t *)pipi, pf, &r, &g, &b);
                    // I feel dirty
                    int x = pending[pending_i].x;
                    int y = pending[pending_i].y;
                    if (x > 0 && !used[x - 1 + y * window_width]++)
                    {
                        *(uint32_t *)(((unsigned char *)pixels) + (x-1) * pf->BytesPerPixel + y * pitch) = SDL_MapRGB(pf, r, g, b);
                        pending[pending_n = (pending_n + 1) % (window_width * window_height)] = (SDL_Point){x-1, y};
                    }
                    if (x < window_width - 1 && !used[x+1 + y * window_width]++)
                    {
                        *(uint32_t *)(((unsigned char *)pixels) + (x+1) * pf->BytesPerPixel + y * pitch) = SDL_MapRGB(pf, r, g, b);
                        pending[pending_n = (pending_n + 1) % (window_width * window_height)] = (SDL_Point){x+1, y};
                    }
                    if (y > 0 && !used[x + (y-1) * window_width]++)
                    {
                        *(uint32_t *)(((unsigned char *)pixels) + x * pf->BytesPerPixel + (y-1) * pitch) = SDL_MapRGB(pf, r, g, b);
                        pending[pending_n = (pending_n + 1) % (window_width * window_height)] = (SDL_Point){x, y-1};
                    }
                    if (y < window_height - 1 && !used[x + (y+1) * window_width]++)
                    {
                        *(uint32_t *)(((unsigned char *)pixels) + x * pf->BytesPerPixel + (y+1) * pitch) = SDL_MapRGB(pf, r, g, b);
                        pending[pending_n = (pending_n + 1) % (window_width * window_height)] = (SDL_Point){x, y+1};
                    }
                }
                // Lala
                pending_i = 0;
                pending_n = 0;
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
	emscripten_sleep(0);
#if 0
        SDL_UpdateWindowSurface(window);
#endif
    }

    SDL_Log("Longest burst: %d\nTargets reset count: %d\nDevice reset count: %d\nDevice lost count: %d\n", max_burst, n_targets_reset, n_device_reset, n_device_lost);

    SDL_DestroyWindow(window);

    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    return 0;
}
