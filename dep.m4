divert(-1)

# TODO use diversions
# Tiered attributes that, one fixed at construction, other reseted
# Artifact construtor that zeroes shit
# Would be cool to have one recipe that generates an hierarchy of folders.
# Another function that is common to native and html5_common

define(`ID',`translit(`$1',/.,__)')
define(`ID_CONTENTS',`REV_`'ID(`$1')`'___contents')

define(`CAT',`define(`$1',defn(`$1')``'$2')')

define(`APPEND',`ifelse(index(defn(`$1'),`X(``$2'')'),-1,`CAT(`$1',`X(``$2'')')')')
define(`EXPAND',`pushdef(`X',`$2')$1`'popdef(`X')')

define(`ADDS',`
  APPEND(`ALL',`$1.o')
  define(ID_CONTENTS($1.o),)
  ADD(`$1.o')
  define(ID($1.o)_deps,`$1$2')
  define(ID($1.o)___3,` -c $1$2 -o PREFIX/$1.o')
')
define(`ADDC',`ADDS(`$1',`.c')')
define(`ADDCXX',`ADDS(`$1',`.cpp')')

define(`INSERT_REV2',`APPEND(ID_CONTENTS(`$1'),PREFIX/`$1')')
define(`INSERT_REV',`ifelse(index($1,.o),-1,,`INSERT_REV2($1)')')

define(`PEEL',`ifdef(`$1',$1)')

define(`AS_RECIPE',`PREFIX/$1:ID($1)_deps
	PEEL(ID($1)`'___1)`'PEEL(ID($1)`'___2)`'PEEL(ID($1)`'___3)`'PEEL(ID($1)`'___4)
')

define(`RESET',`define(ID($1)`'___1,)define(ID($1)`'___2,)define(ID($1)`'___4,)')

define(`TREE',`
  EXPAND(`REFERENCED',defn(`RESET'))
  define(`PREFIX',`$1')
  define(`REFERENCED',`')
  define(`EXTRASS',`')
  $2
  EXPAND(`REFERENCED',defn(`INSERT_REV'))
  APPEND(`CLEANING',clean_`'ID(`$1'))
  divert(0)dnl
`'EXPAND(`REFERENCED',defn(`AS_RECIPE'))
`'clean_`'ID(`$1'):
`'	rm -f`'EXPAND(`REFERENCED',` \
	PREFIX/$'`1')`'EXPAND(`EXTRASS',` \
	PREFIX/$'`1')
`'divert(-1)
')

define(`EMPTY',`define(`WORK',)')
define(`ADD',`PRODUCT(`$1')APPEND(`WORK',`$1')')
define(`FOR',`EXPAND(`group___$1',defn(`ADD'))')

define(`GROUP',`define(`group___$1',WORK)')

define(`DEPFOR',`PRODUCT(`$1')EXPAND(`WORK',`CAT(ID($1)`'_deps,` 'PREFIX/$'`1)')')
define(`PRODUCT',`APPEND(`REFERENCED',`$1')')
define(`EXTRA',`APPEND(`EXTRASS',`$1')')

define(`CC',`EXPAND(`WORK',`define(ID($'`1)`'___1,$1)')')
define(`CFLAG',`EXPAND(`WORK',`CAT(ID($'`1)`'___2,` '$1)')')

EMPTY
ADDC(`camera')   ADDC(`hit_record')
ADDC(`hittable') ADDC(`interval')
ADDC(`material') ADDC(`ray')
ADDC(`scene')    ADDC(`sphere')
ADDC(`util')     ADDC(`vec3')
GROUP(`common')

EMPTY
ADDC(`canvas') ADDC(`interactive') ADDC(`benchmark')
GROUP(`interactive')
EMPTY
ADDCXX(`simpleomp')
ADDC(`batch')

define(`native',`
 define(`interactive_deps',)
 define(`batch_deps',)

 EMPTY
 FOR(`interactive')
  CFLAG(`$$(pkg-config --cflags SDL2)')

 EMPTY
  ADD(`interactive.o')
  CFLAG(`-DUSER_FULLSCREEN=SDL_WINDOW_FULLSCREEN')
  ADD(`camera.o')
  ADD(`interactive')
  ADD(`batch.o')
  ADD(`batch')
  CFLAG(`-fopenmp')

 EMPTY
  FOR(`common')
  DEPFOR(`batch')
  FOR(`interactive')
  DEPFOR(`interactive')
  ADD(`interactive')
  ADD(`batch')
  ADD(`batch.o')
  GROUP(`all')
  CC(`cc -std=c18')
  CFLAG(`-Wpedantic -Wall -Wextra')

 EMPTY
  ADD(`interactive')
  CFLAG(`$$(pkg-config --cflags --libs SDL2)')
  define(`interactive___3',` -o' PREFIX/interactive interactive_deps)

 EMPTY
  ADD(`batch.o')
  DEPFOR(`batch')
  define(`batch___3',` -o' PREFIX/batch batch_deps)
')

define(`html5_common',`
 define(`index_html_deps',)
 EMPTY
  ADD(`scene.o')
  ADD(`util.o')
  CFLAG(`-DM_PI=3.14159265358979323846')

 EMPTY
  ADD(`interactive.o')
  CFLAG(`-DUSER_FULLSCREEN=SDL_WINDOW_FULLSCREEN_DESKTOP')

 EMPTY
  ADD(`canvas.o')
  CFLAG(`-DLIMIT_WIDTH=4000 -DLIMIT_HEIGHT=3000')

 EMPTY
  FOR(`interactive')
  CFLAG(`--use-port=sdl2')

 EMPTY
  ADD(`simpleomp.o')
  CC(`em++')
  CFLAG(`-DNCNN_SIMPLEOMP=1')
  CFLAG(`-fno-rtti')
  CFLAG(`-fno-exceptions')

 EMPTY
  ADD(`index.html')
  CC(`emcc')
  CFLAG(`-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency+1')
  CFLAG(`-sINITIAL_MEMORY=655360000')
  CFLAG(`--use-port=sdl2')
  CFLAG(`--shell-file=shell.html')

 EMPTY
  FOR(`common')
  FOR(`interactive')
  CC(`emcc')

  ADD(`simpleomp.o')

  DEPFOR(`index.html')
  ADD(`index.html')
  GROUP(`all')
  CFLAG(`-pthread')
  CFLAG(`-Wpedantic -Wall -Wextra')
  define(`index_html___3',` -o' PREFIX/index.html index_html_deps)
  EXTRA(`index.js')
  EXTRA(`index.wasm')

  EMPTY
   ADD(`interactive.o')
   ADD(`camera.o')
   ADD(`index.html')
   CFLAG(`-fopenmp')
')

define(`opt',`
 EMPTY
 FOR(`all')
 CFLAG(`-Oz -march=```native''' -flto')
')

define(`debug',`
 EMPTY
 FOR(`all')
 CFLAG(`-g3')
')

define(`coverage',`
 EMPTY
 FOR(`all')
 CFLAG(`--```coverage'''')
')

dnl # TODO: MC/DC coverage (varies to GCC and Clang)

define(`pgo_collect',`
 EMPTY
 FOR(`all')
 CFLAG(`-fprofile-generate')
')

define(`pgo_apply',`
 EMPTY
 FOR(`all')
 CFLAG(`-fprofile-use=native_omp_pgo_collect')
')

define(`html5_opt',`
 EMPTY
 FOR(`all')
 CFLAG(`-Oz -flto -sDISABLE_EXCEPTION_CATCHING=1')
')

define(`html5_san',`
 EMPTY
 FOR(`all')
 CFLAG(`-gsource-map -g3 -Og -fsanitize=address,undefined,bounds')
')

TREE(`html5_omp_san',`html5_common`'html5_san')
TREE(`html5_omp_opt',`html5_common`'html5_opt')
TREE(`native_omp_opt',`native`'opt')
TREE(`native_omp_debug',`native`'debug')
TREE(`native_omp_coverage',`native`'coverage')
TREE(`native_omp_pgo_collect',`native`'opt`'pgo_collect')
TREE(`native_omp_pgo',`native`'opt`'pgo_apply')
divert(0)dnl
OBJ_FULL =EXPAND(`ALL',` \
 $1')
TREES_CLEAN =EXPAND(`CLEANING',` $1')
EXPAND(`ALL',`REV_`'ID($1) =EXPAND(ID_CONTENTS($1),` \
 $'`1')
')
divert(-1)
# vim: nowrap
