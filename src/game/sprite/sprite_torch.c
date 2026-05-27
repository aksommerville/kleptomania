#include "game/kleptomania.h"

#define FRAME_PERIOD 0.150

struct sprite_torch {
  struct sprite hdr;
  double animclock;
  int animframe;
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_torch*)sprite)

static int _torch_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  SPRITE->animframe=rand()&3;
  SPRITE->animclock=((rand()&0xffff)*FRAME_PERIOD)/65535.0;
  return 0;
}

static void _torch_update(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=FRAME_PERIOD;
    if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
  }
  switch (SPRITE->animframe) {
    case 0: sprite->tileid=SPRITE->tileid0+0; sprite->xform=0; break;
    case 1: sprite->tileid=SPRITE->tileid0+1; sprite->xform=0; break;
    case 2: sprite->tileid=SPRITE->tileid0+0; sprite->xform=EGG_XFORM_XREV; break;
    case 3: sprite->tileid=SPRITE->tileid0+1; sprite->xform=EGG_XFORM_XREV; break;
  }
}

const struct sprite_type sprite_type_torch={
  .name="torch",
  .objlen=sizeof(struct sprite_torch),
  .init=_torch_init,
  .update=_torch_update,
};
