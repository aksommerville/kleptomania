#include "game/kleptomania.h"

struct sprite_treasure {
  struct sprite sprite;
  int treasure;
};

#define SPRITE ((struct sprite_treasure*)sprite)

static int _treasure_init(struct sprite *sprite) {
  if (!(SPRITE->treasure=treasure_for_tileid(sprite->tileid))) {
    fprintf(stderr,"Invalid tileid 0x%02x for treasure.\n",sprite->tileid);
    return -1;
  }
  
  // Reject if this treasure is already finished, or if the hero is carrying it.
  if ((SPRITE->treasure>=0)&&(SPRITE->treasure<TREASURE_LIMIT)&&g.treasurev[SPRITE->treasure]) return -1;
  struct sprite *hero=get_hero();
  if (sprite_hero_carrying(hero)==SPRITE->treasure) return -1;
  
  return 0;
}

static void _treasure_update(struct sprite *sprite,double elapsed) {
  sprite_update_gravity(sprite,elapsed);
}

static void _treasure_collide(struct sprite *sprite,struct sprite *other) {
  if (!other) return;
  if (other->type==&sprite_type_hero) {
    if (sprite_hero_carry(other,SPRITE->treasure)>0) {
      sprite->defunct=1;
    }
  }
}

const struct sprite_type sprite_type_treasure={
  .name="treasure",
  .objlen=sizeof(struct sprite_treasure),
  .init=_treasure_init,
  .update=_treasure_update,
  .collide=_treasure_collide,
};

/* Generic model bits.
 */

int treasure_for_tileid(uint8_t tileid) {
  switch (tileid) {
    case 0x90: return NS_treasure_gem;
    case 0x91: return NS_treasure_book;
    case 0x92: return NS_treasure_crown;
    case 0x93: return NS_treasure_avocado;
    case 0x94: return NS_treasure_violin;
    case 0x95: return NS_treasure_sock;
    case 0x96: return NS_treasure_key;
  }
  return 0;
}

uint8_t tileid_for_treasure(int treasure) {
  switch (treasure) {
    case NS_treasure_gem: return 0x90;
    case NS_treasure_book: return 0x91;
    case NS_treasure_crown: return 0x92;
    case NS_treasure_avocado: return 0x93;
    case NS_treasure_violin: return 0x94;
    case NS_treasure_sock: return 0x95;
    case NS_treasure_key: return 0x96;
  }
  return 0;
}

int spriteid_for_treasure(int treasure) {
  switch (treasure) {
    case NS_treasure_gem: return RID_sprite_gem;
    case NS_treasure_book: return RID_sprite_book;
    case NS_treasure_crown: return RID_sprite_crown;
    case NS_treasure_avocado: return RID_sprite_avocado;
    case NS_treasure_violin: return RID_sprite_violin;
    case NS_treasure_sock: return RID_sprite_sock;
    case NS_treasure_key: return RID_sprite_key;
  }
  return 0;
}
