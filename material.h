// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef MATERIAL_H
#define MATERIAL_H

#include <stdbool.h>

#include "color.h"
#include "hit_record.h"
#include "ray.h"
#include "vec3.h"

typedef bool scatter(
    struct material *mat,
    struct ray r_in,
    struct hit_record rec,
    union vec3 *attenuation,
    struct ray *scattered
);

struct material
{
    union vec3 albedo;
    dist fuzz; // TODO should be 1 or less
    dist ir;
    scatter *scatter;
};

scatter material_scatter_lambertian;
scatter material_scatter_metal;
scatter material_scatter_dielectric;

#define MATERIAL_LAMBERTIAN(C)  {C, .scatter = material_scatter_lambertian}
#define MATERIAL_METAL(C, FUZZ) {C, FUZZ, .scatter = material_scatter_metal}
#define MATERIAL_DIELECTRIC(IR) {.ir = IR, material_scatter_dielectric}

#define MATERIAL_NEW_LAMBERTIAN(...) (struct material)MATERIAL_LAMBERTIAN(__VA_ARGS__)
#define MATERIAL_NEW_METAL(     ...) (struct material)MATERIAL_METAL     (__VA_ARGS__)
#define MATERIAL_NEW_DIELECTRIC(...) (struct material)MATERIAL_DIELECTRIC(__VA_ARGS__)

#endif
