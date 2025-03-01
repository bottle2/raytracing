// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "hittable.h"

#define N_SCENE 3

extern struct scene
{
    struct camera   camera;
    struct hittable world;
} scenes[N_SCENE];

void scenes_init(void);

#endif
