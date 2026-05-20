#ifndef EGG_GAME_MAIN_H
#define EGG_GAME_MAIN_H

#include "egg/egg.h"
#include "util/stdlib/egg-stdlib.h"
#include "util/graf/graf.h"
#include "util/font/font.h"
#include "util/res/res.h"
#include "util/text/text.h"
#include "egg_res_toc.h"
#include "shared_symbols.h"
#include "map.h"

#define FBW 320
#define FBH 176

extern struct g {

  // res.c
  void *rom;
  int romc;
  struct rom_entry *resv;
  int resc,resa;
  struct map *mapv; // All maps by id.
  int mapc,mapa;
  struct map **planev; // LRTB, coords of first is (planex,planey). Nulls permitted.
  int planex,planey,planew,planeh;
  uint8_t *physicsv; // 256*physicsc, indexed by (rid-1).
  int physicsc,physicsa;
  
  struct graf graf;
  
  struct map *map;
} g;

// res.c
int res_init();
int res_search(int tid,int rid);
int res_get(void *dstpp,int tid,int rid);

// camera.c
void render_game();

#endif
