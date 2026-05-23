#include "game/kleptomania.h"

#define BALLC 7 /* Only Dot can die, and a witch's soul has 7 circles. */

struct sprite_soulballs {
  struct sprite hdr;
  double r;
  double animclock;
  int frame;
};

#define SPRITE ((struct sprite_soulballs*)sprite)

static int _soulballs_init(struct sprite *sprite) {
  return 0;
}

static void _soulballs_update(struct sprite *sprite,double elapsed) {
  SPRITE->r+=8.0*elapsed;
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.100;
    if (++(SPRITE->frame)>=8) SPRITE->frame=0;
  }
}

static void _soulballs_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,RID_image_sprites);
  uint8_t tileid=0x40;
  switch (SPRITE->frame) {
    case 0: break;
    case 1: tileid+=1; break;
    case 2: tileid+=2; break;
    case 3: tileid+=3; break;
    case 4: tileid+=4; break;
    case 5: tileid+=3; break;
    case 6: tileid+=2; break;
    case 7: tileid+=1; break;
  }
  double t=0.0;
  double dt=(M_PI*2.0)/BALLC;
  int i=BALLC;
  for (;i-->0;t+=dt) {
    int bx=x+lround(sin(t)*SPRITE->r*NS_sys_tilesize);
    int by=y-lround(cos(t)*SPRITE->r*NS_sys_tilesize);
    graf_tile(&g.graf,bx,by,tileid,0);
  }
}

const struct sprite_type sprite_type_soulballs={
  .name="soulballs",
  .objlen=sizeof(struct sprite_soulballs),
  .init=_soulballs_init,
  .update=_soulballs_update,
  .render=_soulballs_render,
};
