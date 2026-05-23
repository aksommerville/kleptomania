#include "game/kleptomania.h"

struct sprite_treasure {
  struct sprite sprite;
  int treasure;
};

#define SPRITE ((struct sprite_treasure*)sprite)

static int _treasure_init(struct sprite *sprite) {
  switch (sprite->tileid) {
    case 0x90: SPRITE->treasure=NS_treasure_gem; break;
    case 0x91: SPRITE->treasure=NS_treasure_book; break;
    case 0x92: SPRITE->treasure=NS_treasure_crown; break;
    case 0x93: SPRITE->treasure=NS_treasure_avocado; break;
    case 0x94: SPRITE->treasure=NS_treasure_violin; break;
    case 0x95: SPRITE->treasure=NS_treasure_sock; break;
    default: {
        fprintf(stderr,"Invalid tileid 0x%02x for treasure.\n",sprite->tileid);
        return -1;
      }
  }
  return 0;
}

static void _treasure_update(struct sprite *sprite,double elapsed) {
  sprite_update_gravity(sprite,elapsed);
  
  // Getting got?
  const double xradius=0.333;
  const double yradius=0.333;
  struct sprite **otherp=g.spritev;
  int i=g.spritec;
  for (;i-->0;otherp++) {
    struct sprite *hero=*otherp;
    if (hero->defunct) continue;
    if (hero->type!=&sprite_type_hero) continue;
    double dx=hero->x-sprite->x;
    if ((dx<-xradius)||(dx>xradius)) continue;
    double dy=hero->y-sprite->y;
    if ((dy<-yradius)||(dy>yradius)) continue;
    if (sprite_hero_carry(hero,SPRITE->treasure)>0) {
      sprite->defunct=1;
      return;
    }
  }
}

const struct sprite_type sprite_type_treasure={
  .name="treasure",
  .objlen=sizeof(struct sprite_treasure),
  .init=_treasure_init,
  .update=_treasure_update,
};
