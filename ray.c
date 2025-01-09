// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include "ray.h"
#include "vec3.h"

union vec3 ray_at(struct ray ray, dist t)
{
    return sum(ray.orig, mul(t, ray.dir));
}
