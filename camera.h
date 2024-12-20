#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include <stdint.h>

#include "hittable.h"
#include "precision.h"

struct camera
{
    // User-defined.

    dist desired_aspect_ratio;
    int  image_width;
    int  samples_per_pixel;
    int  max_depth;
    dist vfov;
    union vec3 lookfrom;
    union vec3 lookat;
    union vec3 vup;

    dist defocus_angle;
    dist focus_dist;

    // User data
    struct hittable *world;
    void *udata;
    void (*set_pixel)(void *, int x, int y, uint8_t r, uint8_t g, uint8_t b);
    bool log;

    // Private.

    union vec3 pixel00_loc;
    union vec3 center;
    union vec3 pixel_delta_u;
    union vec3 pixel_delta_v;
    union vec3 defocus_disk_u;
    union vec3 defocus_disk_v;
    int image_height;

    // Private state-machine state.

    int i;
    int j;
    int sample;
    union vec3 pixel_color;
};

void camera_init(struct camera *camera);
bool camera_step(struct camera *camera, int n);

#endif
