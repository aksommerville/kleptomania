#include "game/kleptomania.h"

/* Move sprite, main entry point.
 */
 
int sprite_move(struct sprite *sprite,double dx,double dy) {

  // Invalid or not solid, get out fast.
  if (!sprite) return 0;
  if (!sprite->solid) {
    sprite->x+=dx;
    sprite->y+=dy;
    return 1;
  }
  
  // One axis should be zero. If they're both nonzero, consider each axis independently.
  uint8_t dir;
       if (dx<-0.001) dir=0x10;
  else if (dx> 0.001) dir=0x08;
  else if (dy<-0.001) dir=0x40;
  else if (dy> 0.001) dir=0x02;
  else return 0;
  if ((dir&0x18)&&((dy<-0.001)||(dy>0.001))) {
    int xok=sprite_move(sprite,dx,0.0);
    int yok=sprite_move(sprite,0.0,dy);
    return xok||yok;
  }
  
  // Arrange to carefully track original and proposed position.
  double ox=sprite->x,oy=sprite->y;
  double nx=ox+dx,ny=sprite->y+dy;
  double nl,nr,nt,nb;
  #define REBOX { \
    nl=nx+sprite->hbl; \
    nr=nx+sprite->hbr; \
    nt=ny+sprite->hbt; \
    nb=ny+sprite->hbb; \
  }
  REBOX
  #define CHECKBOX(_ql,_qr,_qt,_qb) { \
    double ql=(_ql),qr=(_qr),qt=(_qt),qb=(_qb); \
    if ((nl>=qr)||(nr<=ql)||(nt>=qb)||(nb<=qt)) { \
    } else { \
      CHECKBOX_EXTRA \
      switch (dir) { \
        case 0x10: nx=qr-sprite->hbl; if (nx>=ox) return 0; break; \
        case 0x08: nx=ql-sprite->hbr; if (nx<=ox) return 0; break; \
        case 0x40: ny=qb-sprite->hbt; if (ny>=oy) return 0; break; \
        case 0x02: ny=qt-sprite->hbb; if (ny<=oy) return 0; break; \
      } \
      REBOX \
    } \
  }
  #define CHECKBOX_EXTRA
  
  // If we're moving downward, we also need to record the original bottom position, for oneways.
  double ob=oy+sprite->hbb;
  
  /* Check against the map.
   * OOB cells participate; their value is that of their nearest IB neighbor.
   */
  {
    int cola=(int)(nl);
    int colz=(int)(nr);
    int rowa=(int)(nt);
    int rowz=(int)(nb);
    if ((cola<=colz)&&(rowa<=rowz)) {
      const uint8_t *srcrow=g.map->v+rowa*NS_sys_mapw+cola; // NB may be OOB; check coords before reading.
      int row=rowa;
      for (;row<=rowz;row++,srcrow+=NS_sys_mapw) {
        int erow=row;
        if (erow<0) erow=0; else if (erow>=NS_sys_maph) erow=NS_sys_maph-1;
        const uint8_t *srcp=srcrow;
        int col=cola;
        for (;col<=colz;col++,srcp++) {
          int ecol=col;
          if (ecol<0) ecol=0; else if (ecol>=NS_sys_mapw) ecol=NS_sys_mapw-1;
          uint8_t tileid;
          if ((erow==row)&&(ecol==col)) tileid=*srcp;
          else tileid=g.map->v[erow*NS_sys_mapw+ecol];
          uint8_t physics=g.map->physics[tileid];
          if ((physics==NS_physics_oneway)&&(row!=erow)) {
            // Oneways on the edge, everything beyond them is vacant, not more oneways.
            physics=NS_physics_vacant;
          }
          if (physics==NS_physics_safe) continue;
          if (physics==NS_physics_vacant) continue;
          if (physics==NS_physics_hazard) continue; // hazards are not our problem.
          if (physics==NS_physics_oneway) {
            if (dir!=0x02) continue;
            if (ob>(double)row) continue; // Only in play if our foot was previously above or touching it.
          }
          CHECKBOX(col,col+1.0,row,row+1.0)
        }
      }
    }
  }
  
  /* Check against other solid sprites.
   */
  {
    #undef CHECKBOX_EXTRA
    #define CHECKBOX_EXTRA { \
      if (sprite->type->collide) sprite->type->collide(sprite,other); \
      if (other->type->collide) other->type->collide(other,sprite); \
    }
    struct sprite **otherp=g.spritev;
    int i=g.spritec;
    for (;i-->0;otherp++) {
      struct sprite *other=*otherp;
      if (other->defunct) continue;
      if (!other->solid) continue;
      if (other==sprite) continue;
      CHECKBOX(other->x+other->hbl,other->x+other->hbr,other->y+other->hbt,other->y+other->hbb)
    }
  }
  
  #undef REBOX
  #undef CHECKBOX
  #undef CHECKBOX_EXTRA
  sprite->x=nx;
  sprite->y=ny;
  return 1;
}

/* Gravity.
 */
 
#define GRAVITY_ACCEL 40.0 /* m/s**2 */
#define GRAVITY_LIMIT 15.0 /* m/s */
 
void sprite_update_gravity(struct sprite *sprite,double elapsed) {
  if (!sprite) return;
  
  sprite->gravity+=GRAVITY_ACCEL*elapsed;
  if (sprite->gravity>GRAVITY_LIMIT) sprite->gravity=GRAVITY_LIMIT;
  
  int solid0=sprite->solid;
  sprite->solid=1;
  int moved=sprite_move(sprite,0.0,sprite->gravity*elapsed);
  sprite->solid=solid0;
  
  if (moved) {
    if (sprite->seated) {
      sprite->seated=0;
      if (sprite->type->falling) sprite->type->falling(sprite);
    }
  } else {
    double velocity=sprite->gravity;
    sprite->gravity=0.0;
    if (!sprite->seated) {
      sprite->seated=1;
      if (sprite->type->landed) sprite->type->landed(sprite,velocity);
    }
  }
}

void sprite_force_null_gravity(struct sprite *sprite) {
  sprite->gravity=0.0;
  if (sprite->seated) {
    sprite->seated=0;
    if (sprite->type->falling) sprite->type->falling(sprite);
  }
}
