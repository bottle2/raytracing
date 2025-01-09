// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef RAY_H
#define RAY_H

#include "vec3.h"

struct ray
{
    union vec3 orig;
    union vec3 dir;
};

#define RAY(...) (struct ray){__VA_ARGS__}

union vec3 ray_at(struct ray ray, dist t);

#endif
