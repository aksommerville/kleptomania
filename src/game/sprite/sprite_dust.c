#include "game/kleptomania.h"

#define TTL 0.400
#define FRAMETIME (TTL/7.0)

struct sprite_dust {
  struct sprite hdr;
  double ttl;
  uint8_t tileid0;
};

#define SPRITE ((struct sprite_dust*)sprite)

static int _dust_init(struct sprite *sprite) {
  sprite->imageid=RID_image_sprites;
  sprite->tileid=SPRITE->tileid0=0x81;
  SPRITE->ttl=TTL;
  sprite->layer=101;
  return 0;
}

static void _dust_update(struct sprite *sprite,double elapsed) {
  SPRITE->ttl-=elapsed;
  if (SPRITE->ttl<=0.0) {
    sprite->defunct=1;
    return;
  }
       if (SPRITE->ttl<=FRAMETIME*1.0) sprite->tileid=SPRITE->tileid0+6;
  else if (SPRITE->ttl<=FRAMETIME*2.0) sprite->tileid=SPRITE->tileid0+5;
  else if (SPRITE->ttl<=FRAMETIME*3.0) sprite->tileid=SPRITE->tileid0+4;
  else if (SPRITE->ttl<=FRAMETIME*4.0) sprite->tileid=SPRITE->tileid0+3;
  else if (SPRITE->ttl<=FRAMETIME*5.0) sprite->tileid=SPRITE->tileid0+2;
  else if (SPRITE->ttl<=FRAMETIME*6.0) sprite->tileid=SPRITE->tileid0+1;
  else sprite->tileid=SPRITE->tileid0+0;
}

const struct sprite_type sprite_type_dust={
  .name="dust",
  .objlen=sizeof(struct sprite_dust),
  .init=_dust_init,
  .update=_dust_update,
};
