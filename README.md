# Crocante

[Ray Tracing in One Weekend](https://raytracing.github.io/books/raytracinginoneweekend.html)
teaches basic C++ and basic raytracing. However, I followed it in C,
which is my favorite programming language, then I turned it into an
interactive graphical application using [SDL2](https://libsdl.org/).
Because rendering the entire picture in a couple miliseconds is hardly
likely, my current approach is to calculate a couple random pixels at
a time, then use what I know to fill the remainder of the window with
some color.

This project exists for my personal education, thus contributions are
not accepted, but I appreciate criticism.

## Building

Run one of:

    $ make image.png
    $ make interactive
    $ make raytracing.zip

Further configuration is done altering the Makefile.

I use C11, particularly `_Generic`, `_Thread_local` and anonymous
aggregates, but I'm targeting C18 because I'm told it is a bugfix of
C11.

Development occurs in [MSYS2](https://www.msys2.org/), so I dunno if the
[`Makefile`](Makefile) or code is adequate in other environments.

The primary build target is HTML5, which is compiled with
[Emscripten](https://emscripten.org/). Actually, because sanitizers
are unrealiable on Windows, I'm also using HTML5 builds to debug
problems. Most tests occur with a native Windows build.

## Code overview

There are two main entry points:

1. [`batch.c`](batch.c): relies only on standard library, printing
   [PPM](https://en.wikipedia.org/wiki/Netpbm)
2. [`interactive.c`](interactive.c): the SDL2 graphical application

The type `dist` is defined in [`precision.h`](precision.h), which is
some real type, and we let `tgmath.h` pick the correct functions.
This `typedef` is so simple, maybe a compiler-defined macro would do.

Type `struct material` of [`material.h`](material.h) has three
specializations: Lambertian, metallic and dielectric. Each has
specific fields and behavior for method `scatter`. A single `struct`
is used for all subtypes, as they are likely to share common fields,
and function-like macros correctly set it up. A `union` might be used
in the future, tho. For each method of each subtype, functions with
identical signatures are defined. Because users of `struct material`
only care about the behavior, it is unnecessary to distinguish the
actual subtype of instances. Five steps is required to create new
subtypes, being defining the two function-like macro constructors
critical.

Type `struct hittable` of [`hittable.h`](hittable.h) has arrays of
each of its subtypes and of itself, and the method `hittable_hit`
calls methods following the pattern `*_hit` for every element of all
arrays, according to their type. The only subtype now is `struct
sphere` of [`sphere.h`](sphere.h). Maintenance requires two steps:
create a new subtype using function-like macro `X` in the list
`HITTABLE_XS` and implement it.

[`vec3.h`](vec3.h) has some function-like macros `sum`, `sub`, `mul`
and `div` that employ `_Generic`, so that any parameter can be either
a scalar type or a `union vec3`. In order to handle when both
parameters are scalar, [`basic.h`](basic.h) define basic arithmetic
for scalars as functions. Likewise, some shorthand macros are defined
for other vector operations, so that it is easier to type while not
polluting the namespace. 

Examples from the tutorial are kept at [`scene.h`](scene.h). There is
no dynamic memory allocation, so no cleanup function is provided.

## Future directions:

Tutorials to follow:

- [Ray Tracing: The Next Week](https://raytracing.github.io/books/RayTracingTheNextWeek.html)
- [Ray Tracing: The Rest of Your Life](https://raytracing.github.io/books/RayTracingTheRestOfYourLife.html)
- [Ray Tracing in Curved Space Time](http://locklessinc.com/articles/raytracing/)
- [Recursive Make Considered Harmful](https://grosskurth.ca/bib/1997/miller.pdf)
- [Measuring Performance](https://catlikecoding.com/unity/tutorials/basics/measuring-performance/#2)

Essential:

- Add an interactive HUD
- Mobile control
- FPS measurement and display
- Finish keyboard control
- Reliable mouse ungrab in HTML5
- Physics based movement
- Branding
- Benchmark option

Other:

- Improve resizing efficiency
- Implement `if` OpenMP clause in [`simpleomp.cpp`](simpleomp.cpp)
- Fix non-parallel build (remove `is_parallel` and related controls)
- Built-in option to save render without printing 
- Cinematic camera for kiosk mode
- Cross-compilation to macOS and Linux
- Builds for Windows
- Reliable fullscreen
- Editor for new scenes
- Sharing scenes as platform-specific user creations
- More shapes, such quadric surfaces
- Leverage some existing math library rather than NIH syndrome 
- More types such as unit vector
- Better Makefile

Geometry math expressions are to be simplified by employing ordinary
operators between dollar signs, then have a program translate them
into calls such as `sum`, `sub`, `mul` and `div`, which are already
defined in [`vec3.h`](vec3.h), only guaranteeing precedence and
associativity, but unaware of types.

## License

This project Crocante is [MIT licensed](LICENSE), and uses some
third-party code, see [ACKNOWLEDGMENTS](ACKNOWLEDGMENTS).
