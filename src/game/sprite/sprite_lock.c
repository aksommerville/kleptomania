#include "game/kleptomania.h"

struct sprite_lock {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_lock*)sprite)

static int _lock_init(struct sprite *sprite) {
  if (g.treasurev[NS_treasure_key]) return -1;
  return 0;
}

static void _lock_update(struct sprite *sprite,double elapsed) {
  //TODO Is the hero unlocking us?
}

static void _lock_collide(struct sprite *sprite,struct sprite *other) {
  if (!other) return;
  if (other->type==&sprite_type_hero) {
    if (sprite_hero_carrying(other)==NS_treasure_key) {
      g.treasurev[NS_treasure_key]=1;
      sprite_hero_carry(other,0);
      kl_sound(RID_sound_deliver);
      sprite->defunct=1;
    }
  }
}

static void _lock_render(struct sprite *sprite,int x,int y) {
  // Our reference position is the southwest tile, and that's also our tileid.
  graf_set_image(&g.graf,sprite->imageid);
  graf_tile(&g.graf,x,y,sprite->tileid,0);
  graf_tile(&g.graf,x+NS_sys_tilesize,y,sprite->tileid+0x01,0);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,sprite->tileid-0x10,0);
  graf_tile(&g.graf,x+NS_sys_tilesize,y-NS_sys_tilesize,sprite->tileid-0x0f,0);
}

const struct sprite_type sprite_type_lock={
  .name="lock",
  .objlen=sizeof(struct sprite_lock),
  .init=_lock_init,
  .update=_lock_update,
  .render=_lock_render,
  .collide=_lock_collide,
};
