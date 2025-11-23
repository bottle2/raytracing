# SPDX-FileCopyrightText: Copyright (C) 2024 Bento Borges Schirmer 
# SPDX-License-Identifier: MIT

RGH=https://raw.githubusercontent.com
SOKOL=$(RGH)/floooh/sokol-samples/d91015d455409f20fc1b376fae1b29e0cce1e9ef
TENCENT_NCNN=$(RGH)/Tencent/ncnn/9e11dac7d17aae3c600d30de471b57478459a624

TARGET=html5_omp_san/index.html html5_omp_opt/index.html \
       native_omp_opt/interactive native_omp_debug/interactive

all:interactive batch

interactive:native_omp_opt/interactive
	cp native_omp_opt/interactive $@

batch:native_omp_opt/batch
	cp native_omp_opt/batch $@

full:$(TARGET)

image.png:batch
	./batch | gm convert - $@

include dep.mk

clean:$(TREES_CLEAN)
	rm -f image.png *.zip

tags:
	ctags *.c *.h
	ctags -a -R --c-kinds=dept "$$(sdl2-config --prefix)/include/SDL2"

raytracing.zip:html5_omp_opt/index.html
	7z a raytracing.zip html5_omp_opt/index.{html,js,wasm}

shell.html:
	curl $(SOKOL)/webpage/shell.html > $@
simpleomp.cpp:
	curl $(TENCENT_NCNN)/src/simpleomp.cpp > $@

ACKNOWLEDGMENTS:ACKNOWLEDGMENTS.m4
	m4 -DSOKOL='$(SOKOL)' $< > $@

dep.mk:dep.m4
	m4 dep.m4 > $@

include $(OBJ_FULL:.o=.d)

.SUFFIXES: .c .d .cpp

.c.d:
	sh convert.sh cc $<
.cpp.d:
	sh convert.sh c++ $<
