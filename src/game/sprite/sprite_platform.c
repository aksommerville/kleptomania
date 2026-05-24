#include "game/kleptomania.h"

struct sprite_platform {
  struct sprite hdr;
  double dx,dy; // m/s, speed baked in.
  uint8_t dir;
  double animclock;
  int animframe;
};

#define SPRITE ((struct sprite_platform*)sprite)

static int _platform_init(struct sprite *sprite) {
  const double speed=3.0;
  switch (SPRITE->dir=sprite->arg>>24) {
    case 0: break; // Neutral is fine but maybe a little weird.
    case 0x40: SPRITE->dx=0.0; SPRITE->dy=-speed; break;
    case 0x10: SPRITE->dx=-speed; SPRITE->dy=0.0; break;
    case 0x08: SPRITE->dx=speed; SPRITE->dy=0.0; break;
    case 0x02: SPRITE->dx=0.0; SPRITE->dy=speed; break;
    // Diagonals are not currently possible. Would need different handling at update.
    default: SPRITE->dir=0; break;
  }
  return 0;
}

/* Zero if some pumpkin fails to move at all.
 * (yeffective) is my *prior* (y), if moving down. The position at which we search for pumpkins.
 */
static int platform_move_pumpkins(struct sprite *sprite,double dx,double dy,double yeffective) {
  double l=sprite->x+sprite->hbl;
  double r=sprite->x+sprite->hbr;
  double t=yeffective+sprite->hbt;
  int fail=0;
  struct sprite **otherp=g.spritev;
  int i=g.spritec;
  for (;i-->0;otherp++) {
    struct sprite *other=*otherp;
    double ol=other->x+other->hbl; if (ol>=r) continue;
    double or=other->x+other->hbr; if (or<=l) continue;
    double ob=other->y+other->hbb;
    double yoff=ob-t;
    if ((yoff<-0.010)||(yoff>0.010)) continue; // Allow some real variance in verticals, to accomodate roundoff and what have you.
    if (!sprite_move(other,dx,dy)) {
      fail=1;
    }
  }
  if (fail) return 0;
  return 1;
}

static void _platform_update(struct sprite *sprite,double elapsed) {
  
  // Animate. 9 frame: 5 frames ping-ponging, and the far frame plays twice (near frame just once).
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.049;
    if (++(SPRITE->animframe)>=9) SPRITE->animframe=0;
  }
  
  // Motion.
  if (SPRITE->dir) {
  
    // If we move horizontally, all pumpkins get that same move eagerly and their outcome doesn't affect us.
    if (SPRITE->dir&0x18) {
      platform_move_pumpkins(sprite,SPRITE->dx*elapsed,0.0,sprite->y);
      if (!sprite_move(sprite,SPRITE->dx*elapsed,0.0)) {
        SPRITE->dx*=-1.0;
      }
      
    // We're vertical, so it's a little weird. We'll behave differently, up vs down.
    } else if (SPRITE->dy<0.0) {
      if (platform_move_pumpkins(sprite,0.0,SPRITE->dy*elapsed,sprite->y)) {
        if (sprite_move(sprite,0.0,SPRITE->dy*elapsed)) {
        } else {
          SPRITE->dy*=-1.0;
        }
      } else {
        SPRITE->dy*=-1.0;
      }
    } else if (SPRITE->dy>0.0) {
      double y0=sprite->y;
      if (!sprite_move(sprite,0.0,SPRITE->dy*elapsed)) {
        SPRITE->dy*=-1.0;
      } else {
        platform_move_pumpkins(sprite,0.0,SPRITE->dy*elapsed,y0);
      }
    }
  }
}

static void _platform_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
  uint8_t tileid=sprite->tileid; // Left side.
  uint8_t xform=0;
  switch (SPRITE->animframe) {
    case 0: break;
    case 1: tileid+=2; break;
    case 2: tileid+=4; break;
    case 3: tileid+=6; break;
    case 4: tileid+=8; break;
    case 5: tileid+=8; xform=EGG_XFORM_XREV; break;
    case 6: tileid+=6; xform=EGG_XFORM_XREV; break;
    case 7: tileid+=4; xform=EGG_XFORM_XREV; break;
    case 8: tileid+=2; xform=EGG_XFORM_XREV; break;
  }
  const int ht=NS_sys_tilesize>>1;
  if (xform) {
    graf_tile(&g.graf,x-ht,y,tileid+1,xform);
    graf_tile(&g.graf,x+ht,y,tileid,xform);
  } else {
    graf_tile(&g.graf,x-ht,y,tileid,0);
    graf_tile(&g.graf,x+ht,y,tileid+1,0);
  }
}

const struct sprite_type sprite_type_platform={
  .name="platform",
  .objlen=sizeof(struct sprite_platform),
  .init=_platform_init,
  .update=_platform_update,
  .render=_platform_render,
};
