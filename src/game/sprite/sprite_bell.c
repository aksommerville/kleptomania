#include "game/kleptomania.h"

struct sprite_bell {
  struct sprite hdr;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_bell*)sprite)

static int _bell_init(struct sprite *sprite) {
  return 0;
}

static void _bell_update(struct sprite *sprite,double elapsed) {
  if (g.rung_bell) {
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.250;
      if (++(SPRITE->animframe)>=4) SPRITE->animframe=0;
      switch (SPRITE->animframe) {
        case 1: kl_sound(RID_sound_bell1); break;
        case 3: kl_sound(RID_sound_bell2); break;
      }
    }
  } else {
    struct sprite *hero=get_hero();
    if (hero) {
      double dx=hero->x-(sprite->x+0.5);
      double dy=hero->y-sprite->y;
      if ((dx>-0.5)&&(dx<0.5)&&(dy>-0.5)&&(dy<0.5)) {
        g.rung_bell=1;
      }
    }
  }
}

static void _bell_render(struct sprite *sprite,int x,int y) {
  // Our position and reference tile are the lower-left corner.
  graf_set_image(&g.graf,sprite->imageid);
  uint8_t tileid=sprite->tileid;
  uint8_t xform=0;
  switch (SPRITE->animframe) {
    case 1: tileid+=2; break;
    case 3: tileid+=2; xform=EGG_XFORM_XREV; break;
  }
  const int ts=NS_sys_tilesize;
  if (xform) {
    graf_tile(&g.graf,x   ,y   ,tileid+0x01,xform);
    graf_tile(&g.graf,x+ts,y   ,tileid+0x00,xform);
    graf_tile(&g.graf,x   ,y-ts,tileid-0x0f,xform);
    graf_tile(&g.graf,x+ts,y-ts,tileid-0x10,xform);
  } else {
    graf_tile(&g.graf,x   ,y   ,tileid+0x00,xform);
    graf_tile(&g.graf,x+ts,y   ,tileid+0x01,xform);
    graf_tile(&g.graf,x   ,y-ts,tileid-0x10,xform);
    graf_tile(&g.graf,x+ts,y-ts,tileid-0x0f,xform);
  }
}

const struct sprite_type sprite_type_bell={
  .name="bell",
  .objlen=sizeof(struct sprite_bell),
  .init=_bell_init,
  .update=_bell_update,
  .render=_bell_render,
};
