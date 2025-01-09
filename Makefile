# SPDX-FileCopyrightText: Copyright (C) 2024 Bento Borges Schirmer 
# SPDX-License-Identifier: MIT

CFLAGS=-std=c18 -Wpedantic -Wall -Wextra -O3 -march=native -flto
#CC=zig cc
#CFLAGS=-std=c18 -Wpedantic -Wall -Wextra -g3 -fsanitize=undefined,address
#CFLAGS=-std=c18 -Wpedantic -Wall -Wextra -g3
OBJECT=camera.o hit_record.o hittable.o interval.o material.o ray.o scene.o sphere.o util.o vec3.o
LDLIBS=

SOKOL=https://raw.githubusercontent.com/floooh/sokol-samples/d91015d455409f20fc1b376fae1b29e0cce1e9ef

TARGET=batch interactive

all:$(TARGET)

image.png:batch
	./batch | gm convert - $@

batch:$(OBJECT) batch.c
	$(CC) $(CFLAGS) -o $@ batch.c $(OBJECT) $(LDLIBS)

interactive:$(OBJECT) canvas.c interactive.c
	$(CC) $(CFLAGS) $$(pkg-config --cflags SDL2) -o $@ canvas.c interactive.c $(OBJECT) -L/clang64/lib/ $$(pkg-config --libs SDL2)

basic.h:precision.h
	touch $@
batch.c:color.h scene.h
	touch $@
camera.c:camera.h
	touch $@
camera.h:hittable.h
	touch $@
canvas.c:canvas.h util.h
	touch $@
color.h:vec3.h
	touch $@
interactive.c:camera.h color.h scene.h
	touch $@
material.c:material.h vec3.h
	touch $@
ray.c:ray.h vec3.h
	touch $@
ray.h:vec3.h
	touch $@
vec3.c:vec3.h precision.h
	touch $@
vec3.h:basic.h precision.h
	touch $@

clean:
	rm -f image.png $(TARGET) $(OBJECT) index.* *.zip

tags:
	ctags *.c *.h
	ctags -a -R --c-kinds=dept "$$(sdl2-config --prefix)/include/SDL2"

#EMSCRIPTEN_FLAGS=-gsource-map -g3 -Og -fsanitize=address,undefined
EMSCRIPTEN_FLAGS=-DSDL_ASSERT_LEVEL=0 -Oz -flto

raytracing.zip:
	emcc -std=c18 -DM_PI=3.14159265358979323846 \
		$(EMSCRIPTEN_FLAGS) \
		-sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=655360000  \
		$$(find . -name '*.c' ! -name batch.c ! -name simple.c) \
		--use-port=sdl2 -o index.html --shell-file=shell.html
	7z a raytracing.zip index.{html,js,wasm}

shell.html:
	curl $(SOKOL)/webpage/shell.html

ACKNOWLEDGMENTS:ACKNOWLEDGMENTS.m4
	m4 -DSOKOL='$(SOKOL)' $< > $@
