// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <assert.h>
#include <stdatomic.h>
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

#ifdef __STDC_NO_ATOMICS__
#error We are fucked.
#endif

_Static_assert(2 == ATOMIC_CHAR_LOCK_FREE, "Fuck no atomic char");
_Static_assert(2 == ATOMIC_SHORT_LOCK_FREE, "Fuck no atomic short");

#include "benchmark.h"
#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

enum STATE { STATE_RUN, STATE_STOP, STATE_DIE };
char _Atomic state;

bool is_parallel = false;

static struct camera camera;
static bool is_linear = true;

struct canvas canvas = {.fill = CANVAS_FILL_LINEAR};

// XXX
// Yeah.
// se trancado, então pintor tá rodando
// se destrancou, então dormiu e tá esperando condição
// estado só se aplica enquanto tá pintando, são mensagens.
// quando acorda, precisa estar tudo definido, coisa inválida inaceitável

SDL_cond *go;
SDL_mutex *hold;
static int painter(void *data)
{
    (void)data;

    TRY(-1 == SDL_LockMutex(hold));

    while (1)
    {
        // Fuck headers. XXX FUCK
        // fuck.........
        void eval_pixel(struct camera camera[static 1], int i, int j);

        if (is_linear)
        {
            uint64_t start = SDL_GetTicks64();
            if (!is_parallel)
            {
                for (int j = 0; j < camera.image_height; j++)
                {
                    for (int i = 0; i < camera.image_width; i++)
                    {
                        switch (state)
                        {
                            case STATE_RUN:  /* Do nothing. */   break;
                            case STATE_STOP: goto done;          break;
                            case STATE_DIE:  goto finish;        break;
                            default:         assert(!"invalid"); break;
                        }

                        eval_pixel(&camera, i, j);
			canvas.progress[j]++;
                    }
                }
            }
            else
            {
#ifdef __EMSCRIPTEN__
                #pragma omp parallel for
                for (int j = 0; j < camera.image_height; j++)
                {
                    for (int i = 0; i < camera.image_width; i++)
                    {
                        switch (state)
                        {
                            case STATE_RUN:  /* Do nothing. */   break;
                            case STATE_STOP: goto skip;          break;
                            case STATE_DIE:  goto skip;        break;
                            default:         assert(!"invalid"); break;
                        }

                        eval_pixel(&camera, i, j);
			canvas.progress[j]++;
                    }
skip:;
                }
#else
                #pragma omp parallel for schedule(dynamic)
                for (int j = 0; j < camera.image_height; j++)
                {
                    for (int i = 0; i < camera.image_width; i++)
                    {
                        switch (state)
                        {
                            case STATE_RUN:  /* Do nothing. */   break;
                            case STATE_STOP: goto skip;          break;
                            case STATE_DIE:  goto skip;        break;
                            default:         assert(!"invalid"); break;
                        }

                        eval_pixel(&camera, i, j);
			canvas.progress[j]++;
                    }
skip:;
                }
#endif
                if (STATE_DIE == state)
                    goto finish;
            }
            SDL_Log("%s took %" PRIu64 " ms\n", is_parallel ? "par." : "seq.", SDL_GetTicks64() - start);
        }

#if 0
        for (int i = 0; i < 1000; i++)
        {
            switch (state)
            {
                case STATE_RUN:  /* Do nothing. */   break;
                case STATE_STOP: goto done;          break;
                case STATE_DIE:  goto finish;        break;
                default:         assert(!"invalid"); break;
            }
#if 0
            SDL_Log("working %d\n", i);
#endif
            SDL_Delay(5);
        }
#endif
done:
#if 0
	SDL_Log("sleeping...\n");
#endif
        SDL_CondWait(go, hold);
#if 0
        SDL_Log("woke up!\n");
#endif
    }
finish:
#if 0
    SDL_Log("dying...\n");
#endif

    TRY(-1 == SDL_UnlockMutex(hold));
    SDL_DestroyCond(go);
    SDL_DestroyMutex(hold);

    return 0;
}

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

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    (void)data;
    if (x >= output_width || y >= output_height) return;
    canvas.pixels[x + y * output_width] = SDL_MapRGB(pf, r, g, b);
    canvas.used[x + y * output_width] = true;
}

SDL_Window *window = NULL;

int scene_i = 2;

static dist yaw = 0; // Degrees. Look to your left and right
static dist pitch_angle = 0; // Degrees. Look upwards downwards

static int framerates[] = {6, 12, 24, 30, 60, 90, 120, 144};
static int const n_framerate = sizeof (framerates) / sizeof (*framerates);
static int framerate_i = 3;
static dist framerate = 1000 / (dist)30;
int work = 100;

struct user
{
    union vec3 offset;
    union vec3 looking;
    int framerate;
};

static bool step = true;

static int samples_per_pixel = 1;

static bool is_change = false;
static bool is_setup = false;
static bool is_resize = false;
// XXX readability/10
int setup_param_1; // XXX Closure/10
int resize_param_width;
int resize_param_height;

void change(void)
{
    state = STATE_STOP;
    is_change = true;
}

void change_old(void)
{
    SDL_Log("should've cleaned\n");
    memset(canvas.pixels, 0, sizeof (pixel) * output_width * output_height);
    memset(canvas.used, 0, sizeof (bool) * output_width * output_height);
    camera.samples_per_pixel = samples_per_pixel;
    camera_init(&camera);

    // IRC told me it is safe to memset array of atomic if its access is locked!
    // XXX could be a source of bug, pay attention!!
    memset(canvas.progress, 0, sizeof (*canvas.progress) * output_height);

    step = true;
}

int depth = 20;

static void setup(int i)
{
    state = STATE_STOP;
    is_setup = true;
    setup_param_1 = i;
}

static void setup_old(int i)
{
    camera = scenes[i].camera;
    camera.samples_per_pixel = samples_per_pixel;
    camera.image_width = output_width;
    camera.desired_aspect_ratio = output_width / (dist)output_height;
    camera.log = false;
    camera.set_pixel = set_pixel;
    camera.world = &scenes[i].world;
    camera.udata = NULL;

#if 0
    camera.defocus_angle = 0;
#endif
    camera.max_depth = depth;

    change_old();

    {
        union vec3 forward = unit(sub(camera.lookat, camera.lookfrom));
        yaw = atan2(forward.z, forward.x);
        pitch_angle = -asin(forward.y);
    }
}

static bool is_horizontal = true;

static void render(void)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
    SDL_RenderClear(renderer);

    if (step)
    {
#if 0
        int omp_get_num_threads(void);
#endif
        if (!is_linear)
        {
        uint64_t timeout = SDL_GetTicks64() + framerate;
        while (SDL_GetTicks64() < timeout && step)
            step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, work);

        (step ? benchmark_frame : benchmark_done)(SDL_GetTicks64());
        }
        else
        {
            // TODO Do it (what?)
            // XXX IS linear
        }
    }

    int pitch;
    void *pixels = NULL;
    TRY(SDL_LockTexture(canvas.texture, &IN_RECT, &pixels, &pitch));

    // XXX protect for now.
    if (!is_linear)
    {
        if (output_width * pf->BytesPerPixel == pitch)
            (void)memcpy(pixels, canvas.pixels, pitch * output_height);
        else for (int j = 0; j < output_height; j++)
            (void)memcpy(((unsigned char *)(pixels)) + j * pitch, canvas.pixels + j * output_width, pitch);
    }
    else // XXX DANGER
    {
        for (int j = 0; j < output_height; j++)
        {
            (void)memcpy(((unsigned char *)(pixels)) + j * pitch, canvas.pixels + j * output_width, canvas.progress[j] * 4); // FUCK portability. FUCK ME. FUCK
        }
    }

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

#if 0
    if (!is_linear)
#endif
        SDL_RenderCopy(renderer, canvas.texture, &IN_RECT, &OUT_RECT);

    SDL_RenderPresent(renderer);
}

static void resize(int width, int height)
{
    state = STATE_STOP;
    is_resize = true;
    resize_param_width = width;
    resize_param_height = height;
}

static void resize_old(int width, int height)
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
        change_old();
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
            state = STATE_DIE;
            TRY(-1 == SDL_LockMutex(hold)); // wait
            TRY(-1 == SDL_UnlockMutex(hold)); // okay
            TRY(SDL_CondSignal(go) != 0);

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
                    #define BENCHMARK_SAMPLES 500
                    samples_per_pixel = BENCHMARK_SAMPLES;
		    depth = 50;
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
                case 'z': if (work > 10) work -= 10; break;
                case 'x': if (work < 100) work += 10; break;
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
                    SDL_Log("framrate is %d and work is %d\n", framerates[framerate_i], work);
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
                case 't': is_parallel = true; change(); break;
                case 'y': is_parallel = false; change(); break;
		case 'u':
                {
                    extern bool is_uv;
                    is_uv = !is_uv;
                    change();
                }
                break;
                case 'g':
                    state = STATE_STOP;
                break;
                case 'i':
                {
                    // TODO this will be the thing that will reload scene for some reason.
                    int cause;
                    TRY(-1 == (cause = SDL_TryLockMutex(hold)));
                    if (0 == cause)
                    {
                        // locked, is sleeping
                        TRY(-1 == SDL_UnlockMutex(hold));
                        state = STATE_RUN;
                        TRY(SDL_CondSignal(go) != 0);
                    }
                    else
                    {
                        // is running
                        ; // Do nothing.
                    }
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

    if (STATE_STOP == state)
    {
        int cause;
        TRY(-1 == (cause = SDL_TryLockMutex(hold)));
        if (0 == cause)
        {
            if (is_setup)
            {
                is_setup = false;
                is_change = false;
                setup_old(setup_param_1);
            }
            if (is_resize)
            {
                is_resize = false;
                is_change = false;
                resize_old(resize_param_width, resize_param_height);
            }
            if (is_change)
            {
                is_change = false;
                change_old();
            }

            TRY(-1 == SDL_UnlockMutex(hold));
            state = STATE_RUN;
            TRY(SDL_CondSignal(go) != 0);
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

    atomic_init(&state, STATE_RUN);

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

    setup_old(2);

    SDL_SetEventFilter(filter, NULL);

    benchmark_reset = change;

    SDL_Log("before cond\n");
    TRY(!(go = SDL_CreateCond()));
    SDL_Log("before mutex\n");
    TRY(!(hold = SDL_CreateMutex()));
    SDL_Thread *t;
    SDL_Log("before create thread\n");
    TRY(!(t = SDL_CreateThread(painter, "painter", NULL)));
    SDL_Log("before deatach\n");
    SDL_DetachThread(t);

    SDL_Log("I'm alive\n");

    LOOP(iter);

    return 0;
}
