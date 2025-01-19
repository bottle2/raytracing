# SPDX-FileCopyrightText: Copyright (C) 2024 Bento Borges Schirmer 
# SPDX-License-Identifier: MIT

CFLAGS=-std=c18 -fopenmp -Wpedantic -Wall -Wextra -O3 -march=native -flto
#CC=zig cc
#CFLAGS=-std=c18 -fopenmp -Wpedantic -Wall -Wextra -g3 -fsanitize=undefined,address
#CFLAGS=-std=c18 -fopenmp -Wpedantic -Wall -Wextra -g3
OBJECT=camera.o hit_record.o hittable.o interval.o material.o ray.o scene.o sphere.o util.o vec3.o
LDLIBS=

SOKOL=https://raw.githubusercontent.com/floooh/sokol-samples/d91015d455409f20fc1b376fae1b29e0cce1e9ef
TENCENT_NCNN=https://raw.githubusercontent.com/Tencent/ncnn/9e11dac7d17aae3c600d30de471b57478459a624

TARGET=batch interactive

all:$(TARGET)

image.png:batch
	./batch | gm convert - $@

batch:$(OBJECT) batch.c
	$(CC) $(CFLAGS) -o $@ batch.c $(OBJECT) $(LDLIBS)

interactive:$(OBJECT) canvas.c interactive.c benchmark.c
	$(CC) $(CFLAGS) $$(pkg-config --cflags SDL2) -o $@ \
	-DUSER_FULLSCREEN=SDL_WINDOW_FULLSCREEN \
	benchmark.c canvas.c interactive.c $(OBJECT) $$(pkg-config --libs SDL2)

basic.h:precision.h
	touch $@
batch.c:color.h scene.h
	touch $@
benchmark.c:benchmark.h
	touch $@
camera.c:camera.h
	touch $@
camera.h:hittable.h
	touch $@
canvas.c:canvas.h util.h
	touch $@
color.h:vec3.h
	touch $@
interactive.c:benchmark.h camera.h color.h scene.h
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
	rm -f image.png $(TARGET) $(OBJECT) index.* *.zip simpleomp.o

tags:
	ctags *.c *.h
	ctags -a -R --c-kinds=dept "$$(sdl2-config --prefix)/include/SDL2"

#EMSCRIPTEN_FLAGS=-gsource-map -g3 -Og -fsanitize=address,undefined -fopenmp -pthread
EMSCRIPTEN_FLAGS=-Oz -flto -fopenmp -pthread -sDISABLE_EXCEPTION_CATCHING=1

raytracing.zip:interactive.c simpleomp.o
	emcc -std=c18 -DM_PI=3.14159265358979323846 \
		-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency $(EMSCRIPTEN_FLAGS) \
		-sINITIAL_MEMORY=655360000 \
		-DLIMIT_WIDTH=4000 -DLIMIT_HEIGHT=3000 \
		-DUSER_FULLSCREEN=SDL_WINDOW_FULLSCREEN_DESKTOP \
		$$(find . -name '*.c' ! -name batch.c) simpleomp.o \
		--use-port=sdl2 -o index.html --shell-file=shell.html
	7z a raytracing.zip index.{html,js,wasm}

simpleomp.o:simpleomp.cpp cpu.h platform.h simpleomp.h
	em++ -DNCNN_SIMPLEOMP=1 -fno-rtti -fno-exceptions $(EMSCRIPTEN_FLAGS) -c $<

shell.html:
	curl $(SOKOL)/webpage/shell.html > $@
simpleomp.cpp:
	curl $(TENCENT_NCNN)/src/simpleomp.cpp > $@

ACKNOWLEDGMENTS:ACKNOWLEDGMENTS.m4
	m4 -DSOKOL='$(SOKOL)' $< > $@
