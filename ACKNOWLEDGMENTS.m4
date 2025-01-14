THIRD-PARTY LIBRARIES

SDL2 is statically linked to the HTML5 build, unmodified.

The HTML5 build has bits of Emscripten.
dnl You'd think they would have a licensing exception typical of
dnl compilers, but nah, they don't. I value my time, so I won't pursue
dnl every single code in use and their licenses.

File shell.html is part of Sokol, licensed under the following terms:

syscmd(curl SOKOL/LICENSE | sed -e 's/^/  /')dnl

File simpleomp.cpp is part of ncnn;
File platform.h was derived from src/platform.h.in of ncnn;
File cpu.h was derived from src/cpu.h of ncnn.

All three files simpleomp.cpp, platform.h and cpu.h are licensed under
the following terms (BSD 3-Clause License):

  Tencent is pleased to support the open source community by making
  ncnn available.
  Copyright (C) 2017 THL A29 Limited, a Tencent company.  All rights
  reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  Neither the name of [copyright holder] nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

LITERATURE

MARSAGLIA, George. 1999. "Random numbers for C: The END?" [online].
Available at:
<http://www.ciphersbyritter.com/NEWS4/RANDC.HTM#36A5FC62.17C9CC33@stat.fsu.edu>.
Access at: January 2025.

SHIRLEY, Peter; BLACK, Trevor David; HOLLASCH, Steve.
2024. "Ray Tracing in One Weekend". [online]. Available at:
<https://raytracing.github.io/books/raytracinginoneweekend.html>.
Access at: April 2024.
