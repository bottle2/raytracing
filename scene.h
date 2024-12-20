#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "hittable.h"

extern struct scene
{
    struct camera   camera;
    struct hittable world;
} scenes[3];

void scenes_init(void);

#endif
