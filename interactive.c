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
#include "color.h"
#include "scene.h"
#include "util.h"
#include "vec3.h"

static SDL_threadID tid;

#ifndef BITS_PER_PIXEL
#define BITS_PER_PIXEL 32
#endif
#if 32 == BITS_PER_PIXEL
#define PIXEL_INDEX(PIXELS, I, J, PITCH) \
	(((uint32_t *)(((unsigned char *)(PIXELS)) + (J) * (PITCH)))[(I)])
#else
#error Not implemented
#endif

#define TRY(IT) \
if ((IT)) { SDL_LogError(SDL_LOG_CATEGORY_ERROR, __FILE__ ":%d: %s\n", __LINE__, SDL_GetError()); \
            exit(EXIT_FAILURE); } else (void)0

SDL_Renderer *renderer;
uint32_t pixel_format = 0;
SDL_Texture *texture = NULL;

static int window_width  = 800;
static int window_height = 600;

static bool used[4000 * 4000] = {0};
static SDL_Point pending[4000 * 4000] = {0};
int pending_i = 0;
int pending_n = 0;
SDL_PixelFormat *pf = NULL;
int pitch = -1;

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= window_width || y >= window_height) return;
    unsigned char *pipi = data;
    pipi += x * pf->BytesPerPixel + y * pitch;
    *(uint32_t *)pipi = SDL_MapRGB(pf, r, g, b);
    used[x + y * window_width] = true;
    pending[pending_n++] = (SDL_Point){x,y};
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

static void setup(int i)
{
    camera = scenes[i].camera;
    camera.samples_per_pixel = 1;
    camera.image_width = window_width;
    camera.desired_aspect_ratio = window_width / (dist)window_height;
    camera.log = false;
    camera.set_pixel = set_pixel;
    camera.world = &scenes[i].world;
    camera_init(&camera);

    {
        union vec3 forward = unit(sub(camera.lookat, camera.lookfrom));
        yaw = util_rad2deg(atan2(forward.z, forward.x));
        pitch_angle = util_rad2deg(-asin(forward.y));
    }
}

static bool step = true;
static bool is_linear = true;
static bool is_horizontal = true;

static void render(void)
{
    if (step)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
        SDL_RenderClear(renderer);

        void *pixels = NULL;
        TRY(SDL_LockTexture(texture, NULL, &pixels, &pitch));
        memset(pixels, 0, pitch * window_height); 
        camera.udata = pixels;

        #define DESIRED_FRAME 1000 / (dist)24
        uint64_t timeout = SDL_GetTicks64() + DESIRED_FRAME;
        while (SDL_GetTicks64() < timeout)
            step = !(is_linear ? camera_step_linear : camera_step_random)(&camera, 100);

        // Do some shit. lazy ass stupid bitch thing.

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
            for (; pending_i != pending_n; pending_i = (pending_i + 1) % (window_width * window_height))
            {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                int x = pending[pending_i].x;
                int y = pending[pending_i].y;
                SDL_GetRGB(PIXEL_INDEX(pixels, x, y, pitch), pf, &r, &g, &b);
                #define FILL(CMP, I, J) \
                if ((CMP) && !used[(I) + (J)*window_width]++) \
                {   PIXEL_INDEX(pixels,(I),(J),pitch) = SDL_MapRGB(pf,r,g,b); \
                    pending[pending_n] = (SDL_Point){(I),(J)};                \
                    pending_n = (pending_n+1) % (window_width*window_height); }
                FILL(x-1 >= 0, x-1, y)
                FILL(x+1 < window_width, x+1, y)
                FILL(y-1 >= 0, x, y-1)
                FILL(y+1 < window_height, x, y+1)
                #undef FILL
            }
            // Lala
            pending_i = 0;
            pending_n = 0;
        }

        memset(used, 0, sizeof (bool) * window_height * window_width);

        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }

    SDL_RenderPresent(renderer);
}

// TODO WTF how about realloc HMMM ?
static void resize(SDL_Event event[static 1])
{
    SDL_assert(SDL_WINDOWEVENT == event->type);
    SDL_assert(SDL_WINDOWEVENT_RESIZED == event->window.event
            || SDL_WINDOWEVENT_SIZE_CHANGED == event->window.event);

    if (window_width != event->window.data1 || window_height != event->window.data2)
    {
        window_width = event->window.data1;
        window_height = event->window.data2;

        memset(used, 0, sizeof (bool) * window_width * window_height);
        pending_i = 0;
        pending_n = 0;

        SDL_DestroyTexture(texture);
        TRY(!(texture = SDL_CreateTexture(renderer,
                        pixel_format, SDL_TEXTUREACCESS_STREAMING,
                        window_width, window_height)));
        setup(scene_i); step = true;
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
                    resize(event);
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
            SDL_FreeFormat(pf);
            SDL_DestroyTexture(texture);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
            LOOP_END;
            // TODO but will try to render something in the end BROKEN
        }
        else if (SDL_WINDOWEVENT == event.type && (SDL_WINDOWEVENT_RESIZED == event.window.event
                || SDL_WINDOWEVENT_SIZE_CHANGED == event.window.event))
            resize(&event);
	else if (SDL_KEYDOWN == event.type)
        {
            switch (event.key.keysym.sym)
            {
                case 'q':
                    TRY(SDL_PushEvent(&(SDL_Event){.type = SDL_QUIT}) < 0);
                break;
                case SDLK_ESCAPE:
                    SDL_SetRelativeMouseMode(false);
#if 0
                    SDL_ShowCursor(SDL_ENABLE);
                    SDL_SetWindowMouseGrab(window, false);
#endif
                break;
                case 'l': is_linear     = !is_linear;     break;
                case 'h': is_horizontal = !is_horizontal; break;
                case 'd':
                    camera.lookfrom = sum(camera.lookfrom, VEC3(-sin(util_deg2rad(yaw)), 0, cos(util_deg2rad(yaw))));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case 'a':
                    camera.lookfrom = sub(camera.lookfrom, VEC3(-sin(util_deg2rad(yaw)), 0, cos(util_deg2rad(yaw))));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case 'w':
                    camera.lookfrom = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), 0, sin(util_deg2rad(yaw))));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case 's':
                    camera.lookfrom = sub(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), 0, sin(util_deg2rad(yaw))));
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case SDLK_LSHIFT:
                    camera.lookfrom.y--;
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case SDLK_SPACE:
                    camera.lookfrom.y++;
                    camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                    camera_init(&camera);
                break;
                case '1': setup(scene_i = 0); break;
                case '2': setup(scene_i = 1); break;
                case '3': setup(scene_i = 2); break;
                case 'p': SDL_Log("%.3f %.3f", yaw, pitch_angle); break;
                default: goto keep; break;
            }
            step = true;
            keep:;
        }
        else if (SDL_MOUSEBUTTONDOWN == event.type && 1 == event.button.button)
        {
#if 0
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetWindowMouseGrab(window, true);
	    // XXX I've read online that another option is to warp the
	    // mouse to the center at the end of each frame or
	    // something
#else
            SDL_SetRelativeMouseMode(true);
#endif
        }
#if 0
	else if (SDL_RENDER_TARGETS_RESET == event.type || SDL_RENDER_DEVICE_RESET == event.type)
	{
            SDL_DestroyTexture(texture);
            TRY(!(texture = SDL_CreateTexture(renderer,
                            pixel_format, SDL_TEXTUREACCESS_STREAMING,
                            window_width, window_height)));
            setup(scene_i); step = true;
	}
#endif
	else if (SDL_MOUSEMOTION == event.type)
	{
            if (SDL_GetRelativeMouseMode())
            {
                yaw += event.motion.xrel / 2;
                pitch_angle += event.motion.yrel / 2;
                if (pitch_angle > 89) pitch_angle = 89;
                else if (pitch_angle < -89) pitch_angle = -89;
                camera.lookat = sum(camera.lookfrom, VEC3(cos(util_deg2rad(yaw)), sin(util_deg2rad(-pitch_angle)), sin(util_deg2rad(yaw))));
                camera_init(&camera);
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

    scenes_init();

    setup(2);

    atexit(SDL_Quit);

    TRY(SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    tid = SDL_GetThreadID(NULL);

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
    TRY(!(texture = SDL_CreateTexture(renderer,
                    pixel_format, SDL_TEXTUREACCESS_STREAMING,
                    window_width, window_height)));

    SDL_SetEventFilter(filter, NULL);

    LOOP(iter);

    return 0;
}
