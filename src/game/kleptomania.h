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
#include "sprite/sprite.h"

#define FBW 320
#define FBH 176

#define TRANSITION_TIME 0.333

#define TRANSITION_FADE       1
#define TRANSITION_PAN_LEFT   2
#define TRANSITION_PAN_RIGHT  3
#define TRANSITION_PAN_UP     4
#define TRANSITION_PAN_DOWN   5

#define TREASURE_LIMIT 8

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
  
  struct map *map; // REQUIRED
  int safex,safey; // Column and row where hero entered the current map. Will respawn here on death.
  int input,pvinput;
  int treasurev[TREASURE_LIMIT]; // Indexed by NS_treasure_*. Nonzero if this treasure has been delivered.
  double deathtime; // Counts up after dying.
  
  // sprite.c
  struct sprite **spritev;
  int spritec,spritea;
  
  // camera.c
  int bgbits,txbits; // Framebuffer-sized textures for static background and transition prior frame.
  int transition; // TRANSITION_*
  double txclock; // Counts down.
  int bgbits_dirty;
  int sortd;
  double screenshake; // Counts down.
  
} g;

// main.c
void kl_sound(int rid);

// res.c
int res_init();
int res_search(int tid,int rid);
int res_get(void *dstpp,int tid,int rid);

// camera.c
int camera_init_scratches();
void render_game();
void camera_draw_txbits(int include_hero);

/* game.c
 * game_reset() to clear the whole state and start a new game.
 * game_load_map() to reset volatile things and schedule transition to this new map.
 * game_reset and game_load_map will tolerate null (g.map); other functions need not.
 * DO NOT reset or load map during a sprite update cycle. We'll create a deferral mechanism.
 */
int game_reset();
int game_load_map(int rid);
void check_transitions(double elapsed);
int spawn_villagers(); // Normally happens during game_load_map(), but you can poke it manually when everybody's gone.

int treasure_for_tileid(uint8_t tileid);
uint8_t tileid_for_treasure(int treasure);
int spriteid_for_treasure(int treasure);

#endif
