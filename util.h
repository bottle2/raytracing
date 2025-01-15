// SPDX-FileCopyrightText: Copyright (c) 2024 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#ifndef UTIL_H
#define UTIL_H

#include "precision.h"

dist util_deg2rad(dist deg);

// Returns a random real in (0,1).
// TODO It was supposed to be [0,1).
dist random01(void);

// Returns a random real in (min,max).
// TODO It was supposed to be [min,max).
dist random_interval(dist min, dist max);

#define UNCHECKED(...) (__VA_ARGS__)
#define KLUDGE(...) (__VA_ARGS__)
#define NIL

#define TRY_(IT, STR) if ((IT)) { \
    SDL_LogError( \
        SDL_LOG_CATEGORY_ERROR, \
        __FILE__ ":%d: %s\n", __LINE__, (STR) \
    ); exit(EXIT_FAILURE); } else (void)0

#define TRY( IT) TRY_((IT), SDL_GetError())
#define TRYE(IT) TRY_((IT), strerror(errno))
// E as in errno

#define MIN(A, B) ((A) < (B) ? (A) : (B))

#endif
