// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

#include "precision.h"
#include "util.h"

dist util_deg2rad(dist deg)
{
    return deg * M_PI / 180.0;
}

// Thanks George Marsaglia.
// http://www.ciphersbyritter.com/NEWS4/RANDC.HTM#36A5FC62.17C9CC33@stat.fsu.edu

#define znew   (z_=36969*(z_&65535)+(z_>>16))
#define wnew   (w=18000*(w&65535)+(w>>16))
#define MWC    ((znew<<16)+wnew )
#define SHR3  (jsr^=(jsr<<17), jsr^=(jsr>>13), jsr^=(jsr<<5))
#define CONG  (jcong=69069*jcong+1234567)
#define KISS  ((MWC^CONG)+SHR3)
#define UNI   (KISS*2.328306e-10)

_Thread_local uint32_t z_=362436069;
_Thread_local uint32_t w=521288629;
_Thread_local uint32_t jsr=123456789;
_Thread_local uint32_t jcong=380116160;

// TODO integer version?

dist random01(void) { return UNI; }

dist random_interval(dist min, dist max)
{
    return min + (max - min) * random01();
}
