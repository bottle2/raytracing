#include "camera.h"
#include "hittable.h"
#include "material.h"
#include "scene.h"
#include "sphere.h"
#include "vec3.h"
#include "util.h"

struct scene scenes[3] = {0};

void scenes_init(void)
{
    // Image.

#if 0
    struct camera camera = {
        .desired_aspect_ratio = 16.0 / 9.0,
        .image_width          = 400, //1200,
        .samples_per_pixel    = 100, //10,
        .max_depth            = 50,
        .vfov                 = 20,
        .lookfrom             = VEC3(-2,2,1), //VEC3(13, 2, 3),
        .lookat               = VEC3(0,0,-1), //VEC3( 0, 0, 0),
        .vup                  = VEC3( 0, 1, 0),
        .defocus_angle        = 0,//0.6,
        .focus_dist           = 10.1//10
    };

    scenes[0].camera = scenes[1].camera = scenes[2].camera = camera;
#else
    scenes[0].camera = (struct camera)
    {
        .desired_aspect_ratio = 16.0 / 9.0,
        .image_width          = 400, //1200,
        .samples_per_pixel    = 100, //10,
        .max_depth            = 50,
        .vfov                 = 20,
        .lookfrom             = VEC3(-2,2,1), //VEC3(13, 2, 3),
        .lookat               = VEC3(0,0,-1), //VEC3( 0, 0, 0),
        .vup                  = VEC3( 0, 1, 0),
        .defocus_angle        = 0,//0.6,
        .focus_dist           = 10.1//10
    };

    scenes[1].camera = (struct camera)
    {
        .desired_aspect_ratio = 16.0 / 9.0,
        .image_width          = 400,
        .samples_per_pixel    = 100,
        .max_depth            = 50,
        .vfov                 = 90,
        .lookfrom             = VEC3(0, 0, 0),
        .lookat               = VEC3( 0, 0, -1),
        .vup                  = VEC3( 0, 1, 0),
        .defocus_angle        = 0,//0.6,
        .focus_dist           = 10.1//10
    };

    scenes[2].camera = (struct camera)
    {
        .desired_aspect_ratio = 16.0 / 9.0,
        .image_width          = 1200,
        .samples_per_pixel    = 100,
        .max_depth            = 10,
        .vfov                 = 20,
        .lookfrom             = VEC3(13, 2, 3),
        .lookat               = VEC3( 0, 0, 0),
        .vup                  = VEC3( 0, 1, 0),
        .defocus_angle        = 0.6,
        .focus_dist           = 10
    };
#endif

    // World of first scene.

    {
        static struct material material_ground = MATERIAL_LAMBERTIAN(COLOR_C(0.8, 0.8, 0.0));
#if 0
        static struct material material_center = MATERIAL_LAMBERTIAN(COLOR_C(0.7, 0.3, 0.3));
        static struct material material_left   = MATERIAL_METAL     (COLOR_C(0.8, 0.8, 0.8), 0.3);
#else
        static struct material material_center = MATERIAL_LAMBERTIAN(COLOR_C(0.1, 0.2, 0.5));
        static struct material material_left   = MATERIAL_DIELECTRIC(1.5);
#endif
        static struct material material_right  = MATERIAL_METAL     (COLOR_C(0.8, 0.6, 0.2), 0.0);

        static struct sphere spheres[5] = {
            {VEC3_C( 0, -100.5, -1), 100  , &material_ground},
            {VEC3_C( 0,    0  , -1),   0.5, &material_center},
            {VEC3_C(-1,    0  , -1),   0.5, &material_left  },
            {VEC3_C(-1,    0  , -1),  -0.4, &material_left  },
            {VEC3_C( 1,    0  , -1),   0.5, &material_right },
        };

        struct hittable world = {
            .spheres = spheres,
            .n_sphere = 5,
        };

        scenes[0].world = world;
    }

    // World of second scene.

    {
        dist const R = cos(M_PI/4);

        static struct material material_left  = MATERIAL_LAMBERTIAN(COLOR_C(0,0,1));
        static struct material material_right = MATERIAL_LAMBERTIAN(COLOR_C(1,0,0));

        static struct sphere spheres[2];

	spheres[0] = (struct sphere){VEC3(-R, 0, -1), R, &material_left };
	spheres[1] = (struct sphere){VEC3( R, 0, -1), R, &material_right};

        struct hittable world = {
            .spheres = spheres,
            .n_sphere = 2,
        };

        scenes[1].world = world;
    }
    
    // World of third scene.

    {
        #define SPHERE_MAX 1 + 22*22 + 3
        static struct sphere spheres[SPHERE_MAX];

        struct hittable world = {.spheres = spheres, .n_sphere = 1,};

        static struct material ground_material = MATERIAL_LAMBERTIAN(VEC3_C(0.5, 0.5, 0.5));
        world.spheres[0] = (struct sphere){VEC3(0, -1000, 0), 1000, &ground_material};

        static struct material materials[22*22];
        for (int a = -11; a < 11; a++)
        {
            for (int b = -11; b < 11; b++)
            {
                dist choose_mat = random01();
                union vec3 center = VEC3(a + 0.9 * random01(), 0.2, b + 0.9*random01());

                if (len(sub(center, VEC3(4, 0.2, 0))) > 0.9)
                {
                    struct material *sphere_material = materials + world.n_sphere;

                    if (choose_mat < 0.8)
                    {
                        // Diffuse.
                        union vec3 albedo = mul(vec3_random01(), vec3_random01());
                        *sphere_material = materials[world.n_sphere] = MATERIAL_NEW_LAMBERTIAN(albedo);
                    }
                    else if (choose_mat < 0.95)
                    {
                        // Metal.
                        union vec3 albedo = vec3_random_interval(0.5, 1);
                        dist fuzz = random_interval(0, 0.5);
                        *sphere_material = MATERIAL_NEW_METAL(albedo, fuzz);
                    }
                    else
                    {
                        // Glass
                        *sphere_material = MATERIAL_NEW_DIELECTRIC(1.5);
                    }

                    world.spheres[world.n_sphere++] = (struct sphere){center, 0.2, sphere_material};
                }
            }
        }

        static struct material material1 = MATERIAL_DIELECTRIC(1.5);
        world.spheres[world.n_sphere++] = (struct sphere){VEC3(0,1,0),1, &material1};

        static struct material material2 = MATERIAL_LAMBERTIAN(COLOR_C(0.4, 0.2, 0.1));
        world.spheres[world.n_sphere++] = (struct sphere){VEC3(-4,1,0),1, &material2};

        static struct material material3 = MATERIAL_METAL(COLOR_C(0.7, 0.6, 0.5), 0.0);
        world.spheres[world.n_sphere++] = (struct sphere){VEC3(4,1,0),1, &material3};

        scenes[2].world = world;
    }
}
