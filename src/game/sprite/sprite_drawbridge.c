#include "game/kleptomania.h"

#define CLOSED_H 5 /* Our reference position is the bottom of the closed posture. */
#define OPEN_DX -2 /* When open, reference position is the right edge. */
#define OPEN_DY  1
#define OPEN_W 7

struct sprite_drawbridge {
  struct sprite hdr;
};

#define SPRITE ((struct sprite_drawbridge*)sprite)

static void drawbridge_open(struct sprite *sprite) {
  g.drawbridged=1;
  sprite->x+=OPEN_DX;
  sprite->y+=OPEN_DY;
  sprite->solid=1;
  sprite->hbl=0.5-OPEN_W;
  sprite->hbr=0.5;
  sprite->hbt=-0.5;
  sprite->hbb=0.0;
}

static int _drawbridge_init(struct sprite *sprite) {
  if (g.drawbridged) drawbridge_open(sprite);
  return 0;
}

static void _drawbridge_update(struct sprite *sprite,double elapsed) {
  if (g.drawbridged) return;
  struct sprite *hero=get_hero();
  if (hero) {
    double dx=hero->x-sprite->x;
    if ((dx<-0.5)||(dx>0.5)) return;
    // Don't worry about the vertical.
    drawbridge_open(sprite);
    kl_sound(RID_sound_deliver);
  }
}

static void _drawbridge_render(struct sprite *sprite,int x,int y) {
  if (g.drawbridged) {
    uint8_t tileid=sprite->tileid+2;
    int i=OPEN_W;
    for (;i-->0;x-=NS_sys_tilesize) {
      graf_tile(&g.graf,x,y,tileid,0);
      if (i==1) tileid=sprite->tileid;
      else tileid=sprite->tileid+1;
    }
  } else {
    uint8_t tileid=sprite->tileid;
    int i=CLOSED_H;
    for (;i-->0;y-=NS_sys_tilesize) {
      graf_tile(&g.graf,x,y,tileid,EGG_XFORM_SWAP|EGG_XFORM_XREV);
      if (i==1) tileid=sprite->tileid+2;
      else tileid=sprite->tileid+1;
    }
  }
}

const struct sprite_type sprite_type_drawbridge={
  .name="drawbridge",
  .objlen=sizeof(struct sprite_drawbridge),
  .init=_drawbridge_init,
  .update=_drawbridge_update,
  .render=_drawbridge_render,
};
