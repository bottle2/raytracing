// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>
#include <tgmath.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

#ifndef USER_FULLSCREEN
#error Define to either SDL_WINDOW_FULLSCREEN or SDL_WINDOW_FULLSCREEN_DESKTOP
#endif

#define LOOP(ITER) emscripten_set_main_loop((ITER), 0, 1)
#define LOOP_END emscripten_cancel_main_loop(); return
#else
static bool is_playing = true;
#define LOOP(ITER) while (is_playing) (ITER)()
#define LOOP_END is_playing = false; return
#endif

#include "benchmark.h"
#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

bool is_parallel = false;

static SDL_threadID tid;

SDL_Renderer *renderer;

static int window_width  = 800;
static int window_height = 600;
static int output_width  = 800;
static int output_height = 600;
static int pad_x = 0;
static int pad_y = 0;
#define IN_RECT  (SDL_Rect){0, 0, output_width, output_height}
#define OUT_RECT (SDL_Rect){pad_x, pad_y, output_width, output_height}

struct canvas canvas = {.fill = CANVAS_FILL_LINEAR};

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    (void)data;
    if (x >= output_width || y >= output_height) return;
    canvas.pixels[x + y * output_width] = SDL_MapRGB(pf, r, g, b);
    canvas.used[x + y * output_width] = true;
}

SDL_Window *window = NULL;

static struct camera camera;
int scene_i = 2;

static dist yaw = 0; // Degrees. Look to your left and right
static dist pitch_angle = 0; // Degrees. Look upwards downwards

static int framerates[] = {6, 12, 24, 30, 60, 90, 120, 144};
static int const n_framerate = sizeof (framerates) / sizeof (*framerates);
static int framerate_i = 3;
static dist framerate = 1000 / (dist)24;

struct user
{
    union vec3 offset;
    union vec3 looking;
    int framerate;
};

static bool step = true;

static int samples_per_pixel = 1;

void change(void)
{
    memset(canvas.pixels, 0, sizeof (pixel) * output_width * output_height);
    memset(canvas.used, 0, sizeof (bool) * output_width * output_height);
    camera.samples_per_pixel = samples_per_pixel;
    camera_init(&camera);
    step = true;
}

static void setup(int i)
{
    camera = scenes[i].camera;
    camera.samples_per_pixel = samples_per_pixel;
    camera.image_width = output_width;
    camera.desired_aspect_ratio = output_width / (dist)output_height;
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
        uint64_t timeout = SDL_GetTicks64() + framerate;
        while (SDL_GetTicks64() < timeout && step)
            step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, 100);

        (step ? benchmark_frame : benchmark_done)(SDL_GetTicks64());
    }

    int pitch;
    void *pixels = NULL;
    TRY(SDL_LockTexture(canvas.texture, &IN_RECT, &pixels, &pitch));

    if (output_width * pf->BytesPerPixel == pitch)
        (void)memcpy(pixels, canvas.pixels, pitch * output_height);
    else for (int j = 0; j < output_height; j++)
        (void)memcpy(((unsigned char *)(pixels)) + j * pitch, canvas.pixels + j * output_width, pitch);

    if (step && !is_linear) {
        if (is_horizontal)
            for (int j = 0; j < output_height; j++)
            {
                uint8_t prev_r;
                uint8_t prev_g;
                uint8_t prev_b;
                int last = -1;
                #define FILL(FROM, TO, GEN) \
                for (int k = (FROM); k < (TO); k++) \
                    PIXEL_INDEX(pixels, k, j, pitch) = SDL_MapRGB(pf, GEN##_r, GEN##_g, GEN##_b)

                for (int i = 0; i < output_width; i++)
                {
                    if (!canvas.used[i + j * output_width])
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
                    FILL(last, output_width, prev);
                #undef FILL
            }
        else // Do the radial
        {
            int pending_n = 0;
            (void)memcpy(canvas.used2, canvas.used, output_width * output_height);

            #define FILL(CMP, I, J) \
            if ((CMP) && !canvas.used2[(I) + (J)*output_width]++) \
            {   PIXEL_INDEX(pixels,(I),(J),pitch) = SDL_MapRGB(pf,r,g,b); \
                canvas.pending[pending_n] = (SDL_Point){(I),(J)}; \
                pending_n = (pending_n+1) % (output_width*output_height); }

            #define FILL_4() do { \
                uint8_t r, g, b; \
                SDL_GetRGB(PIXEL_INDEX(pixels, x, y, pitch), pf, &r, &g, &b); \
                FILL(x-1 >= 0, x-1, y) \
                FILL(x+1 < output_width, x+1, y) \
                FILL(y-1 >= 0, x, y-1) \
                FILL(y+1 < output_height, x, y+1) } while (0)

            for (int y = 0; y < output_height; y++)
                for (int x = 0; x < output_width; x++)
                    if (canvas.used[x + y * output_width])
                        FILL_4();

            for (
                int pending_i = 0;
                pending_i != pending_n;
                pending_i = (pending_i + 1) % (output_width * output_height)
            ) {
                int x = canvas.pending[pending_i].x;
                int y = canvas.pending[pending_i].y;
                FILL_4();
            }

            #undef FILL_4
            #undef FILL
        }
    }

    SDL_UnlockTexture(canvas.texture);

    SDL_RenderCopy(renderer, canvas.texture, &IN_RECT, &OUT_RECT);

    SDL_RenderPresent(renderer);
}

static void resize(int width, int height)
{
    if (window_width != width || window_height != height)
    {
        window_width = width;
        window_height = height;

        // Letterbox
        if (window_width <= max_width && window_height <= max_height)
        {
            pad_x = 0;
            pad_y = 0;
            output_width = width;
            output_height = height;
        }
        else
        {
            output_width = width > max_width ? max_width : width;
            output_height = height > max_height ? max_height : height;
            pad_x = (window_width - output_width) / 2;
            pad_y = (window_height - output_height) / 2;
        }

        camera.image_width = output_width;
        camera.desired_aspect_ratio = output_width / (dist)output_height;
        change();
        step = true;
    }
}

// https://discourse.libsdl.org/t/is-it-thread-safe-to-render-inside-event-watcher/49409
static int filter(void *userdata, SDL_Event *event)
{
    (void)userdata;

    if (SDL_WINDOWEVENT == event->type
            && SDL_WINDOWEVENT_EXPOSED == event->window.event)
    {
        SDL_assert(SDL_GetThreadID(NULL) == tid);
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        resize(w, h);
        render();
    }

    return 1;
}

static void iter(void)
{
    for (SDL_Event event; SDL_PollEvent(&event); )
    {
        if (SDL_QUIT == event.type)
        {
            canvas_end(&canvas);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
            LOOP_END;
        }
        else if (SDL_WINDOWEVENT == event.type && (SDL_WINDOWEVENT_RESIZED == event.window.event
                || SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event))
            resize(event.window.data1, event.window.data2);
	else if (SDL_KEYDOWN == event.type)
        {
            void omp_set_num_threads(int);
            static bool is_fullscreen = false;
            switch (event.key.keysym.sym)
            {
                case SDLK_ESCAPE:
                    if (!is_fullscreen)
                        SDL_SetRelativeMouseMode(false);
                    else
                    {
                        SDL_SetWindowFullscreen(window, 0);
                        is_fullscreen = false;
                    }
#if 0
                    SDL_ShowCursor(SDL_ENABLE);
                    SDL_SetWindowMouseGrab(window, false);
#endif
                break;
                case 'f':
                fullscreen:
                {
                    SDL_DisplayMode dm;
                    SDL_GetDisplayMode(0, 0, &dm);
                    SDL_SetWindowDisplayMode(window, &dm);
                }
                    SDL_SetWindowFullscreen(window, USER_FULLSCREEN);
                    is_fullscreen = true;
                break;
                case 'b':
                    #define BENCHMARK_SAMPLES 10
                    samples_per_pixel = BENCHMARK_SAMPLES;
                    is_linear = true;
                    benchmark_start(SDL_GetTicks64());
                    goto fullscreen;
                break;

                case 'c':
                    if (framerate_i > 0)
                        framerate = 1000 / (dist)framerates[--framerate_i];
                break;
                case 'v':
                    if (framerate_i < n_framerate - 1)
                        framerate = 1000 / (dist)framerates[++framerate_i];
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
                    SDL_Log("framrate is %d\n", framerates[framerate_i]);
                    SDL_Log("%.3f %.3f", yaw, pitch_angle);
                    SDL_Log("%d %d %d %d %d %d\n", pad_x, pad_y, output_width, output_height, window_width, window_height);
                    #pragma omp parallel
                    {
                    int omp_get_num_threads(void);
                    SDL_Log("%d\n", omp_get_num_threads());
                    }
                    {
                        SDL_Rect v;
                        SDL_RenderGetViewport(renderer, &v);
                        SDL_Log("viewport %d %d\n", v.w, v.h);
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
            canvas_create_texture(&canvas, renderer);
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
    canvas_init(&canvas, renderer);

    setup(2);

    SDL_SetEventFilter(filter, NULL);

    benchmark_reset = change;

    LOOP(iter);

    return 0;
}
