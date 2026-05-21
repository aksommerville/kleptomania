#include "game/kleptomania.h"

struct sprite_hero {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_hero*)sprite)

/* Cleanup.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  return 0;
}

/* Gravity events.
 */
 
static void _hero_landed(struct sprite *sprite,double velocity) {
  fprintf(stderr,"%s %.03f\n",__func__,velocity);
}

static void _hero_falling(struct sprite *sprite) {
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  if (!(g.input&EGG_BTN_UP)) sprite_update_gravity(sprite,elapsed);
  //XXX highly temporary
  const double speed=8.0; // m/s
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: sprite->xform=EGG_XFORM_XREV; sprite_move(sprite,-speed*elapsed,0.0); break;
    case EGG_BTN_RIGHT: sprite->xform=0; sprite_move(sprite,speed*elapsed,0.0); break;
  }
  switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
    case EGG_BTN_UP: sprite_move(sprite,0.0,-speed*elapsed); break;
    case EGG_BTN_DOWN: sprite_move(sprite,0.0,speed*elapsed); break;
  }
}

/* Render.
 */
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
  uint8_t tileid=sprite->tileid;
  graf_tile(&g.graf,x,y,tileid,sprite->xform);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,tileid-0x10,sprite->xform);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
  .landed=_hero_landed,
  .falling=_hero_falling,
};

/* Get the global hero sprite if there is one.
 */
 
struct sprite *get_hero() {
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->type!=&sprite_type_hero) continue;
    if (sprite->defunct) continue;
    return sprite;
  }
  return 0;
}
