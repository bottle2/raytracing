// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdbool.h>
#include <tgmath.h>

#include "vec3.h"
#include "precision.h"
#include "util.h"

#define OP_DECL(OP, OP_) \
union vec3 vec3_##OP##_vv(union vec3 lhs, union vec3 rhs) { \
    return (union vec3){{lhs.x OP_ rhs.x, lhs.y OP_ rhs.y, lhs.z OP_ rhs.z}}; \
} \
union vec3 vec3_##OP##_vs(union vec3 lhs, dist rhs) { \
    return (union vec3){{lhs.x OP_ rhs, lhs.y OP_ rhs, lhs.z OP_ rhs}}; \
} \
union vec3 vec3_##OP##_sv(dist lhs, union vec3 rhs) { \
    return (union vec3){{lhs OP_ rhs.x, lhs OP_ rhs.y, lhs OP_ rhs.z}}; \
}

OP_DECL(sum, +)
OP_DECL(sub, -)
OP_DECL(mul, *)
OP_DECL(div, /)

#undef OP_DECL

union vec3 vec3_neg(union vec3 op)
{
    return VEC3(-op.x, -op.y, -op.z);
}

union vec3 vec3_unit(union vec3 op)
{
    return div(op, len(op));
}

dist vec3_length(union vec3 op)
{
    return sqrt(lensq(op));
}

dist vec3_length_squared(union vec3 op)
{
    return op.x*op.x + op.y*op.y + op.z*op.z;
}

dist vec3_dot(union vec3 lhs, union vec3 rhs)
{
    return lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
}

union vec3 vec3_cross(union vec3 lhs, union vec3 rhs)
{
    return VEC3(
        lhs.y*rhs.z - lhs.z*rhs.y,
        lhs.z*rhs.x - lhs.x*rhs.z,
        lhs.x*rhs.y - lhs.y*rhs.x
    );
}

union vec3 vec3_random01(void)
{
    return VEC3(random01(), random01(), random01());
}

union vec3 vec3_random_interval(dist min, dist max)
{
    return VEC3(random_interval(min, max), random_interval(min, max), random_interval(min, max));
}

union vec3 vec3_random_in_unit_sphere(void)
{
    while (true)
    {
        union vec3 p = vec3_random_interval(-1,1);
        if (lensq(p) < 1)
            return p;
    }
}

union vec3 vec3_random_in_unit_disk(void)
{
    while (true)
    {
        union vec3 p = VEC3(random_interval(-1,1), random_interval(-1,1), 0);
        if (lensq(p) < 1)
            return p;
    }
}

union vec3 vec3_random_unit_vector(void)
{
    return unit(vec3_random_in_unit_sphere());
}

union vec3 vec3_random_on_hemisphere(union vec3 normal)
{
    union vec3 const on_unit_sphere = vec3_random_unit_vector();
    if (dot(on_unit_sphere, normal) > 0)
        return on_unit_sphere;
    else
        return neg(on_unit_sphere);
}

bool vec3_near_zero(union vec3 it)
{
    dist s = 1e-8;
    return fabs(it.x) < s && fabs(it.y) < s && fabs(it.z) < s;
}

union vec3 vec3_reflect(union vec3 v, union vec3 n)
{
    return sub(v, mul(2 * dot(v,n),n));
}

union vec3 vec3_refract(union vec3 uv, union vec3 n, dist etai_over_etat)
{
    dist cos_theta = fmin(dot(neg(uv), n), 1);
    union vec3 r_out_perp     = mul(etai_over_etat, sum(uv, mul(cos_theta, n)));
    union vec3 r_out_parallel = mul(-sqrt(fabs(1 - lensq(r_out_perp))), n);
    return sum(r_out_perp, r_out_parallel);
}
