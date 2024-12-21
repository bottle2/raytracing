#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "scene.h"

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    (void)data;
    (void)x;
    (void)y;
    printf(COLOR_FMT "\n", (int)r, (int)g, (int)b);
}

int main(int argc, char *argv[])
{
    scenes_init();

    struct scene *scene = scenes + (1 == argc ? 0 : atoi(argv[1]));

    struct camera batch = scene->camera;

    batch.world = &scene->world;
    batch.udata = NULL;
    batch.set_pixel = set_pixel;
    batch.log = true;

    camera_init(&batch);

    printf("P3\n%d %d\n255\n", batch.image_width, batch.image_height);

    camera_step_linear(&batch, -1);

    fprintf(stderr, "Done\n");

    return 0;
}
