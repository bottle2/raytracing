#ifndef VEC_H
#define VEC_H

#include <stdbool.h>

#include "basic.h"
#include "precision.h"

union vec3
{
    struct { dist x; dist y; dist z; };
    struct { dist r; dist g; dist b; };
};

union vec3 vec3_neg           (union vec3 op);
union vec3 vec3_unit          (union vec3 op);
dist       vec3_length        (union vec3 op);
dist       vec3_length_squared(union vec3 op);

dist       vec3_dot  (union vec3 lhs, union vec3 rhs);
union vec3 vec3_cross(union vec3 lhs, union vec3 rhs);

union vec3 vec3_random01(void);
union vec3 vec3_random_interval(dist min, dist max);
union vec3 vec3_random_unit_vector(void);
union vec3 vec3_random_on_hemisphere(union vec3 normal);
union vec3 vec3_random_in_unit_sphere(void);
union vec3 vec3_random_in_unit_disk(void);

bool vec3_near_zero(union vec3 it);

union vec3 vec3_reflect(union vec3  v, union vec3 n);
union vec3 vec3_refract(union vec3 uv, union vec3 n, dist etai_over_etat);

#define OP_DECL(OP) \
union vec3 vec3_##OP##_vv(union vec3 lhs, union vec3 rhs); \
union vec3 vec3_##OP##_vs(union vec3 lhs, dist       rhs); \
union vec3 vec3_##OP##_sv(dist       lhs, union vec3 rhs)

OP_DECL(sum);
OP_DECL(sub);
OP_DECL(mul);
OP_DECL(div);

#undef OP_DECL

#define GOP(OP, LHS, RHS) _Generic((LHS), \
  union vec3: _Generic((RHS), union vec3: vec3_##OP##_vv, default: vec3_##OP##_vs) \
, default:    _Generic((RHS), union vec3: vec3_##OP##_sv, default: basic_##OP))((LHS), (RHS))

// Clean interface

#define sum(LHS, RHS) GOP(sum, (LHS), (RHS))
#define sub(LHS, RHS) GOP(sub, (LHS), (RHS))
#define mul(LHS, RHS) GOP(mul, (LHS), (RHS))
#define div(LHS, RHS) GOP(div, (LHS), (RHS))

#define VEC3(...) (union vec3){{__VA_ARGS__}}
#define VEC3_C(...) {{__VA_ARGS__}}
#define VEC3_L(...) VEC3(__VA_ARGS__)

#define VEC3_FMT "%f %f %f"
#define VEC3_BLOW(IT) (double)(IT).x, (double)(IT).y, (double)(IT).z

#define neg   vec3_neg
#define unit  vec3_unit
#define len   vec3_length
#define lensq vec3_length_squared

#define dot   vec3_dot
#define cross vec3_cross

#endif
