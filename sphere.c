// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <tgmath.h>

#include "hit_record.h"
#include "interval.h"
#include "ray.h"
#include "sphere.h"

bool sphere_hit(
    struct sphere     *sphere,
    struct ray         ray,
    struct interval    ray_t,
    struct hit_record *rec
) {
    union vec3 const oc = sub(ray.orig, sphere->center);
    dist const      a       = lensq(ray.dir);
    dist const half_b       = dot(oc, ray.dir);
    dist const      c       = lensq(oc) - sphere->radius*sphere->radius;
    dist const discriminant = half_b*half_b - a*c;

    if (discriminant < 0)
        return false;

    dist const sqrtd = sqrt(discriminant);

    dist root = (-half_b - sqrtd) / a;

    if (!interval_surrounds(ray_t, root))
    {
        root = (-half_b + sqrtd) / a;
        if (!interval_surrounds(ray_t, root))
            return false;
    }

    rec->t = root;
    rec->p = ray_at(ray, rec->t);
    union vec3 outward_normal = div(sub(rec->p, sphere->center), sphere->radius);
    hit_record_set_face_normal(rec, ray, outward_normal);
    rec->mat = sphere->mat;

    return true;
}
