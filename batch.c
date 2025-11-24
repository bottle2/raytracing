// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>

#include "color.h"
#include "scene.h"

static struct pixel { unsigned char r, g, b; } pixels[4000][4000];

static void set_pixel(void *data, int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
    (void)data;
    pixels[x][y] = (struct pixel){r,g,b};
}

bool is_parallel = true;

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

    for (int y = 0; y < batch.image_height; y++)
        for (int x = 0; x < batch.image_width; x++)
        {
            struct pixel p = pixels[x][y];
            printf(COLOR_FMT "\n", p.r, p.g, p.b);
        }

    fprintf(stderr, "Done\n");

    return 0;
}
