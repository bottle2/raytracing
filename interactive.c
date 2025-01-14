// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>
#include <tgmath.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#define LOOP(ITER) emscripten_set_main_loop((ITER), 0, 1)
#define LOOP_END emscripten_cancel_main_loop(); return
#else
static bool is_playing = true;
#define LOOP(ITER) while (is_playing) (ITER)()
#define LOOP_END is_playing = false; return
#endif

#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

bool is_parallel = false;

static SDL_threadID tid;

SDL_Renderer *renderer;
SDL_Texture *texture = NULL;

static int window_width  = 800;
static int window_height = 600;

static bool used[4000 * 4000] = {0};
int pitch = -1;

struct canvas canvas = {CANVAS_FILL_LINEAR, (pixel[4000*4000]){0}};

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    (void)data;
    if (x >= window_width || y >= window_height) return;
    canvas.pixels[x + y * window_width] = SDL_MapRGB(pf, r, g, b);
    used[x + y * window_width] = true;
}

SDL_Window *window = NULL;

static struct camera camera;
int scene_i = 2;

static dist yaw = 0; // Degrees. Look to your left and right
static dist pitch_angle = 0; // Degrees. Look upwards downwards
#if 0
static int framerates[] = {6, 12, 24, 30, 60, 90, 120, 144};
static int framerate_i = 0;
static int framerate = 12;
#endif

// TODO we have to add physics, framerate independent motion and configuration
// that is, if it is lagging, settings are changed independently from
// framerate

struct user
{
    union vec3 offset;
    union vec3 looking;
    int framerate;
};

#if 0
static void change(void)
{
#if 0
    camera.vfov = 90;
    camera.samples_per_pixel = 10;
    camera.max_depth = 10;
#endif
#if 0
    camera.udata = surface;
#endif

#if 0
    union vec3 forward = sub(camera.lookat, camera.lookfrom);

    {
        dist new_x = forward.x * cos(yaw) - forward.z * sin(yaw);
        dist new_z = forward.x * sin(yaw) - forward.z * cos(yaw);
        forward.x = new_x;
        forward.z = new_z;
    }
#endif

#if 0
    camera.lookat = @;
#endif

}
#endif

static bool step = true;

static int samples_per_pixel = 1;

void change(void)
{
    memset(canvas.pixels, 0, sizeof (pixel) * window_width * window_height);
    memset(used, 0, sizeof (bool) * window_width * window_height);
    camera.samples_per_pixel = samples_per_pixel;
    camera_init(&camera);
    step = true;
}

static void setup(int i)
{
    camera = scenes[i].camera;
    camera.samples_per_pixel = samples_per_pixel;
    camera.image_width = window_width;
    camera.desired_aspect_ratio = window_width / (dist)window_height;
    camera.log = false;
    camera.set_pixel = set_pixel;
    camera.world = &scenes[i].world;
    camera.udata = NULL;

    camera.defocus_angle = 0;
    camera.max_depth = 20;

    change();

    {
        union vec3 forward = unit(sub(camera.lookat, camera.lookfrom));
        yaw = atan2(forward.z, forward.x);
        pitch_angle = -asin(forward.y);
    }
}

static bool is_linear = true;
static bool is_horizontal = true;

static void render(void)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
    SDL_RenderClear(renderer);

    if (step)
    {
        #define DESIRED_FRAME 1000 / (dist)24
        uint64_t timeout = SDL_GetTicks64() + DESIRED_FRAME;
        while (SDL_GetTicks64() < timeout)
            step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, 100);
    }

        void *pixels = NULL;
        TRY(SDL_LockTexture(texture, NULL, &pixels, &pitch));

        if (window_width * pf->BytesPerPixel == pitch)
            (void)memcpy(pixels, canvas.pixels, pitch * window_height);
        else for (int j = 0; j < window_height; j++)
            (void)memcpy(((unsigned char *)(pixels)) + j * pitch, canvas.pixels + j * window_width, pitch);

        // Do some shit. lazy ass stupid bitch thing.

    if (step && !is_linear) {
        if (is_horizontal)
            for (int j = 0; j < window_height; j++)
            {
                uint8_t prev_r;
                uint8_t prev_g;
                uint8_t prev_b;
                int last = -1;
                #define FILL(FROM, TO, GEN) \
                for (int k = (FROM); k < (TO); k++) \
                    PIXEL_INDEX(pixels, k, j, pitch) = SDL_MapRGB(pf, GEN##_r, GEN##_g, GEN##_b)

                for (int i = 0; i < window_width; i++)
                {
                    if (!used[i + j * window_width])
                        continue;
                    // Copied from callback.
                    uint8_t curr_r;
                    uint8_t curr_g;
                    uint8_t curr_b;
                    SDL_GetRGB(PIXEL_INDEX(pixels, i, j, pitch), pf, &curr_r, &curr_g, &curr_b);

                    if (-1 == last) FILL(0, i, curr);
                    else
                    {
                        FILL(last, (last + i) / 2, prev);
                        FILL((last + i) / 2, i, curr);
                    }

                    prev_r = curr_r;
                    prev_g = curr_g;
                    prev_b = curr_b;

                    last = i;
                }
                if (last != -1)
                    FILL(last, window_width, prev);
                #undef FILL
            }
        else // Do the radial
        {
            static SDL_Point pending[4000 * 4000];
            static bool used2[4000 * 4000];
            int pending_n = 0;
            (void)memcpy(used2, used, window_width * window_height);

            #define FILL(CMP, I, J) \
            if ((CMP) && !used2[(I) + (J)*window_width]++) \
            {   PIXEL_INDEX(pixels,(I),(J),pitch) = SDL_MapRGB(pf,r,g,b); \
                pending[pending_n] = (SDL_Point){(I),(J)};                \
                pending_n = (pending_n+1) % (window_width*window_height); }

            #define FILL_4() do { \
                uint8_t r, g, b; \
                SDL_GetRGB(PIXEL_INDEX(pixels, x, y, pitch), pf, &r, &g, &b); \
                FILL(x-1 >= 0, x-1, y) \
                FILL(x+1 < window_width, x+1, y) \
                FILL(y-1 >= 0, x, y-1) \
                FILL(y+1 < window_height, x, y+1) } while (0)

            for (int y = 0; y < window_height; y++)
                for (int x = 0; x < window_width; x++)
                    if (used[x + y * window_width])
                        FILL_4();

            for (
                int pending_i = 0;
                pending_i != pending_n;
                pending_i = (pending_i + 1) % (window_width * window_height)
            ) {
                int x = pending[pending_i].x;
                int y = pending[pending_i].y;
                FILL_4();
            }

            #undef FILL_4
            #undef FILL
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);
}

// TODO WTF how about realloc HMMM ?
static void resize(int width, int height)
{
    int w;
    int h;
    SDL_GetRendererOutputSize(renderer, &w, &h);
    SDL_Log("called once %d %d %d %d\n", width, height, w, h);
    if (window_width != width || window_height != height)
    {
        window_width = width;
        window_height = height;

        SDL_DestroyTexture(texture);
        TRY(!(texture = SDL_CreateTexture(renderer,
                        pixel_format, SDL_TEXTUREACCESS_STREAMING,
                        window_width, window_height)));
#if 0
        setup(scene_i);
#else
        camera.image_width = window_width;
        camera.desired_aspect_ratio = window_width / (dist)window_height;
        change();
#endif
        step = true;
    }

}

// https://discourse.libsdl.org/t/is-it-thread-safe-to-render-inside-event-watcher/49409
static int filter(void *userdata, SDL_Event *event)
{
    (void)userdata;

    if (SDL_WINDOWEVENT == event->type)
    {
        switch (event->window.event)
        {
            case SDL_WINDOWEVENT_EXPOSED:
                SDL_assert(SDL_GetThreadID(NULL) == tid);
                render();
                return 0;
            break;
            case SDL_WINDOWEVENT_RESIZED: // Fall through.
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                if (SDL_GetThreadID(NULL) == tid)
                {
                    resize(event->window.data1, event->window.data2);
                    return 0;
                }
            break;
            default:
                // Do nothing.
            break;
        }
    }

    return 1;
}

static void iter(void)
{
    for (SDL_Event event; SDL_PollEvent(&event); )
    {
        if (SDL_QUIT == event.type)
        {
            SDL_SetEventFilter(NULL, NULL);
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
            LOOP_END;
            // TODO but will try to render something in the end BROKEN
        }
        else
		if (SDL_WINDOWEVENT == event.type && (SDL_WINDOWEVENT_RESIZED == event.window.event
                || SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event))
            resize(event.window.data1, event.window.data2);
	else if (SDL_KEYDOWN == event.type)
        {
            void omp_set_num_threads(int);
            switch (event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    SDL_SetRelativeMouseMode(false);
#if 0
                    SDL_ShowCursor(SDL_ENABLE);
                    SDL_SetWindowMouseGrab(window, false);
#endif
                break;
                case 'l': is_linear     = !is_linear;     change(); break;
                case 'h': is_horizontal = !is_horizontal; change(); break;
                case 'n': if (samples_per_pixel > 1) { samples_per_pixel--; change(); } break;
                case 'm': if (samples_per_pixel < 100) { samples_per_pixel++; change(); } break;
                case 'd':
                    camera.lookfrom = sum(camera.lookfrom, VEC3(-sin(yaw), 0, cos(yaw)));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case 'a':
                    camera.lookfrom = sub(camera.lookfrom, VEC3(-sin(yaw), 0, cos(yaw)));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case 'w':
                    camera.lookfrom = sum(camera.lookfrom, VEC3(cos(yaw), 0, sin(yaw)));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case 's':
                    camera.lookfrom = sub(camera.lookfrom, VEC3(cos(yaw), 0, sin(yaw)));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case SDLK_LSHIFT:
                    camera.lookfrom.y--;
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case SDLK_SPACE:
                    camera.lookfrom.y++;
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                    change();
                break;
                case 'r':
                    change();
                break;
                case '1': setup(scene_i = 0); break;
                case '2': setup(scene_i = 1); break;
                case '3': setup(scene_i = 2); break;
                case 'p':
                    SDL_Log("%.3f %.3f", yaw, pitch_angle);
                    #pragma omp parallel
                    {
                    int omp_get_num_threads(void);
                    SDL_Log("%d\n", omp_get_num_threads());
                    }
                break;
                case 't': is_parallel = true; break;
                case 'y': is_parallel = false; break;
		case 'u':
                {
                    extern bool is_uv;
                    is_uv = !is_uv;
                    change();
                }
                break;
                default: goto keep; break;
            }
            step = true;
            keep:;
        }
        else if (SDL_MOUSEBUTTONDOWN == event.type && 1 == event.button.button)
            SDL_SetRelativeMouseMode(true);
	else if (SDL_RENDER_DEVICE_RESET == event.type)
	{
            SDL_DestroyTexture(texture);
            TRY(!(texture = SDL_CreateTexture(renderer,
                            pixel_format, SDL_TEXTUREACCESS_STREAMING,
                            window_width, window_height)));
	}
	else if (SDL_MOUSEMOTION == event.type)
	{
            if (SDL_GetRelativeMouseMode())
            {
                yaw += util_deg2rad(event.motion.xrel / 2);
		// TODO we should wrap around so as not to lose precision
                pitch_angle += util_deg2rad(event.motion.yrel / 2);
                if (pitch_angle > 89) pitch_angle = 89;
                else if (pitch_angle < -89) pitch_angle = -89;
                camera.lookat = sum(camera.lookfrom, VEC3(cos(yaw), sin(-pitch_angle), sin(yaw)));
                change();
            }
	}
    }

    // XXX check if sdl has lalalal
    // XXX check if lock needed
    // XXX add fullscreen

    render();
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

    scenes_init();

    setup(2);

    atexit(SDL_Quit);

    int omp_get_max_threads(void);
    void omp_set_num_threads(int);
    omp_set_num_threads(omp_get_max_threads());

    TRY(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    tid = SDL_GetThreadID(NULL);

    TRY(!(window = SDL_CreateWindow(
        "Toy Raytracing",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        window_width, window_height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    )));
    // TODO Deal with Apple's high-DPI stuff.

    TRY(!(renderer = SDL_CreateRenderer(window, -1, 0)));

    canvas_setup(renderer);

    TRY(!(texture = SDL_CreateTexture(renderer,
                    pixel_format, SDL_TEXTUREACCESS_STREAMING,
                    window_width, window_height)));

    SDL_SetEventFilter(filter, NULL);

    LOOP(iter);

    return 0;
}
