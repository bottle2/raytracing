// SPDX-FileCopyrightText: Copyright (c) 2025 Bento Borges Schirmer 
// SPDX-License-Identifier: MIT

#include <stdint.h>

#ifndef BENCHMARK_XS
#define BENCHMARK_XS \
X(START, start) X(FRAME, frame) X(DONE, done) X(CANCEL, cancel)

#define X(L, S) void benchmark_##S(uint64_t);
BENCHMARK_XS
#undef X

extern void (*benchmark_reset)(void);

#endif
