CFLAGS=-Wpedantic -Wall -Wextra -O3 -march=native -flto
OBJECT=camera.o hit_record.o hittable.o interval.o material.o ray.o scene.o sphere.o util.o vec3.o
LDLIBS=

TARGET=batch interactive

all:$(TARGET)

image.png:batch
	./batch | gm convert - $@

batch:$(OBJECT) batch.c
	$(CC) $(CFLAGS) -o $@ batch.c $(OBJECT) $(LDLIBS)

interactive:$(OBJECT) interactive.c
	$(CC) $$(pkg-config --cflags SDL2) -o $@ interactive.c $(OBJECT) $$(pkg-config --libs SDL2)

basic.h:precision.h
	touch $@
batch.c:color.h scene.h
	touch $@
camera.c:camera.h
	touch $@
camera.h:hittable.h
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

raytracing.zip:
	emcc -Os -sASYNCIFY_STACK_SIZE=262144 -sASYNCIFY -sSINGLE_FILE $$(find . -name '*.c' ! -name batch.c) --use-port=sdl2 -o index.html --shell-file=shell.html
	7z a raytracing.zip index.html #index.{html,js,wasm}
