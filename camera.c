#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "camera.h"
#include "color.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"
#include "util.h"

static union vec3 ray_color(struct ray ray, int depth, struct hittable *world);
static struct ray get_ray(struct camera *camera, int i, int j);
static union vec3 pixel_sample_square(union vec3 pixel_delta_u, union vec3 pixel_delta_v);
static union vec3 defocus_disk_sample(union vec3 center, union vec3 defocus_disk_u, union vec3 defocus_disk_v);

void camera_init(struct camera *camera) {
    // Calculate the image height.

    camera->image_height = camera->image_width / camera->desired_aspect_ratio;

    assert(camera->image_height >= 1);

    // Camera.

    dist const theta           = util_deg2rad(camera->vfov);
    dist const h               = tan(theta/2);
    dist const viewport_height = 2 * h * camera->focus_dist;

    dist const actual_aspect_ratio = camera->image_width / (dist)camera->image_height;
    dist const viewport_width      = viewport_height * actual_aspect_ratio;

    camera->center = camera->lookfrom;

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    union vec3 w = unit(sub(camera->lookfrom, camera->lookat));
    union vec3 u = unit(cross(camera->vup, w));
    union vec3 v = cross(w, u);

    // Calculate the vectors across the horizontal and down vertical viewport edges.

    union vec3 const viewport_u = mul(viewport_width, u);        // Vector across viewport horizontal edge.
    union vec3 const viewport_v = mul(viewport_height, neg(v)); // Vector across viewport vertical edge.

    // Calculate the horizontal and vertical delta vectors from pixel to pixel.

    camera->pixel_delta_u = div(viewport_u, camera->image_width );
    camera->pixel_delta_v = div(viewport_v, camera->image_height);

    // Calculate the location of the upper left pixel.
    
    // XXX Consider shader dialect.
    union vec3 const viewport_upper_left = sub(sub(sub(camera->center, mul(camera->focus_dist, w)), div(viewport_u, 2)), div(viewport_v, 2));
    camera->pixel00_loc         = sum(viewport_upper_left, mul(0.5, sum(camera->pixel_delta_u, camera->pixel_delta_v)));

    // Calculate the camera defocus disk basis vectors.
    dist const defocus_radius = camera->focus_dist * tan(util_deg2rad(camera->defocus_angle / 2));
    camera->defocus_disk_u = mul(u, defocus_radius);
    camera->defocus_disk_v = mul(v, defocus_radius);

    // State machine stuff.
    camera->i = 0;
    camera->j = 0;
    camera->sample = 0;
}

bool camera_step_linear(struct camera camera[static 1], int n)
{
    for (; camera->j < camera->image_height; camera->j++)
    {
        if (camera->log)
            fprintf(stderr, "Scanlines remaining: %d\n", camera->image_height - camera->j);

        for (; camera->i < camera->image_width; camera->i++)
        {
            if (0 == camera->sample)
                camera->pixel_color = VEC3(0,0,0);

            for (; camera->sample < camera->samples_per_pixel; camera->sample++)
            {
                if (0 == n)
                    return false;
                else if (n > 0)
                    n--;

                struct ray r = get_ray(camera, camera->i, camera->j);
                camera->pixel_color = sum(
                    camera->pixel_color,
                    ray_color(r, camera->max_depth, camera->world)
                );
            }

	    camera->sample = 0;

            camera->set_pixel(
                camera->udata,
                camera->i,
                camera->j,
                COLOR_BLOW(camera->pixel_color, camera->samples_per_pixel)
            );
        }

	camera->i = 0;
    }

    return true;
}

// XXX Code copied from `camera_step_linear`, kinda sus.
bool camera_step_random(struct camera camera[static 1], int n)
{
    assert(n >= 0);

    while (n > 0)
    {
        if (0 == camera->sample)
        {
            camera->i = random01() * camera->image_width; 
            camera->j = random01() * camera->image_height; 
            camera->pixel_color = VEC3(0,0,0);
        }

        for (; camera->sample < camera->samples_per_pixel; camera->sample++)
        {
            if (0 == n)
                return false;
            else
                n--;

            struct ray r = get_ray(camera, camera->i, camera->j);
            camera->pixel_color = sum(
                camera->pixel_color,
                ray_color(r, camera->max_depth, camera->world)
            );
        }

        camera->set_pixel(
            camera->udata,
            camera->i,
            camera->j,
            COLOR_BLOW(camera->pixel_color, camera->samples_per_pixel)
        );

        camera->sample = 0;
    }

    return false;
}

static union vec3 ray_color(struct ray ray, int depth, struct hittable *world)
{
    if (depth <= 0)
        return COLOR(0,0,0);

    struct hit_record rec;

    if (hittable_hit(world, ray, INTERVAL(0.001, HUGE_VAL), &rec))
    {
        struct ray scattered;
        union vec3 attenuation;

        if (rec.mat->scatter(rec.mat, ray, rec, &attenuation, &scattered))
            return mul(attenuation, ray_color(scattered, depth - 1, world));
        return COLOR(0, 0, 0);

#if 0
#if 0 // Primitive.
        union vec3 direction = vec3_random_on_hemisphere(rec.normal);
#else // Lambertian distribution.
        union vec3 direction = sum(rec.normal, vec3_random_unit_vector());
#endif
#if 0 // Just paint the normal.
        return mul(0.5, sum(rec.normal, 1));
#else
        return mul(0.5, ray_color(RAY(rec.p, direction), depth - 1, world));
#endif
#endif
    }

    union vec3 unit_direction = vec3_unit(ray.dir);
    dist a = 0.5 * (unit_direction.y + 1.0);
    return sum(mul(1.0 - a, COLOR(1,1,1)), mul(a, COLOR(0.5, 0.7, 1.0)));
}

static struct ray get_ray(struct camera *camera, int i, int j)
{
    // Get a randomly-sampled camera ray for the pixel at location i,j, originating from the camera defocus disk.

    union vec3 const pixel_center  = sum(sum(camera->pixel00_loc, mul(i, camera->pixel_delta_u)), mul(j, camera->pixel_delta_v));
#if 0
    union vec3 const pixel_center = @camera->pixel00_loc
                                  + i * camera->pixel_delta_u
                                  + j * camera->pixel_delta_v@;
#endif // XXX proposed @@ delimiters for gay dialect
    union vec3 const pixel_sample  = sum(pixel_center, pixel_sample_square(camera->pixel_delta_u, camera->pixel_delta_v));
    union vec3 const ray_origin    = camera->defocus_angle <= 0 ? camera->center : defocus_disk_sample(camera->center, camera->defocus_disk_u, camera->defocus_disk_v);
    union vec3 const ray_direction = sub(pixel_sample, ray_origin);
    return RAY(ray_origin, ray_direction);
}

static union vec3 pixel_sample_square(union vec3 pixel_delta_u, union vec3 pixel_delta_v)
{
    dist px = -0.5 + random01();
    dist py = -0.5 + random01();

    return sum(mul(px, pixel_delta_u), mul(py, pixel_delta_v));
}

static union vec3 defocus_disk_sample(union vec3 center, union vec3 defocus_disk_u, union vec3 defocus_disk_v)
{
    // Returns a random point in the camera defocus disk.
    union vec3 p = vec3_random_in_unit_disk();
    return sum(center, sum(mul(p.x, defocus_disk_u), mul(p.y, defocus_disk_v)));
}
