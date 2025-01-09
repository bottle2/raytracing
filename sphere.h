// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef SPHERE_H
#define SPHERE_H

#include <stdbool.h>

#include "hit_record.h"
#include "interval.h"
#include "material.h"
#include "ray.h"
#include "vec3.h"

struct sphere
{
    union vec3 center;
    dist radius;
    struct material *mat;
};

bool sphere_hit(
    struct sphere     *sphere,
    struct ray         ray,
    struct interval    ray_t,
    struct hit_record *rec
);

#endif
