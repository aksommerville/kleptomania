#include "game/kleptomania.h"

struct sprite_villager {
  struct sprite hdr;
  int treasure;
  uint8_t tileid0;
  int satisfied;
  double animclock;
  int animframe;
  int complaining;
  double complainclock;
};

#define SPRITE ((struct sprite_villager*)sprite)

/* Set (complainclock) for some random interval.
 */
 
static void villager_delay_complaint(struct sprite *sprite) {
  SPRITE->complaining=0;
  SPRITE->complainclock=2.000+4.000*((rand()&0xffff)/65535.0);
}

/* Init.
 */
 
static int _villager_init(struct sprite *sprite) {
  SPRITE->tileid0=sprite->tileid;
  
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,sprite->res,sprite->resc)>=0) {
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_sprite_treasure: SPRITE->treasure=(cmd.arg[0]<<8)|cmd.arg[1]; break;
      }
    }
  }
  
  villager_delay_complaint(sprite);
  
  return 0;
}

/* If all the villager and treasure sprites are defunct, trigger new ones.
 */
 
static void villager_maybe_spawn_new() {
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->defunct) continue;
    if (sprite->type==&sprite_type_villager) return;
    if (sprite->type==&sprite_type_treasure) return;
  }
  spawn_villagers();
}

/* Update.
 */
 
static void _villager_update(struct sprite *sprite,double elapsed) {

  // If satisfied, walk off to the left.
  if (SPRITE->satisfied) {
    sprite->xform=EGG_XFORM_XREV;
    sprite->x-=5.0*elapsed;
    if (sprite->x<-1.0) {
      sprite->defunct=1;
      villager_maybe_spawn_new();
    }
    if ((SPRITE->animclock-=elapsed)<=0.0) {
      SPRITE->animclock+=0.200;
      if (++(SPRITE->animframe)>=2) SPRITE->animframe=0;
    }
    return;
  }
  
  // Randomly complain.
  if ((SPRITE->complainclock-=elapsed)<=0.0) {
    if (SPRITE->complaining) {
      villager_delay_complaint(sprite);
    } else {
      SPRITE->complaining=1;
      SPRITE->complainclock=2.000;
    }
  }

  // Not satisfied yet. Consult the hero.
  struct sprite *hero=get_hero();
  if (!hero) return;
  double dx=hero->x-sprite->x;
  if (dx>0.0) sprite->xform=0;
  else sprite->xform=EGG_XFORM_XREV;
  
  if (!SPRITE->satisfied&&(SPRITE->treasure>0)) {
    double dy=hero->y-sprite->y;
    if ((dx>=-1.0)&&(dx<=1.0)&&(dy>=-1.0)&&(dy<=1.0)) {
      if (sprite_hero_carrying(hero)==SPRITE->treasure) {
        kl_sound(RID_sound_deliver);
        sprite_hero_carry(hero,0);
        SPRITE->satisfied=1;
        SPRITE->complaining=0;
        if ((SPRITE->treasure>=0)&&(SPRITE->treasure<TREASURE_LIMIT)) {
          g.treasurev[SPRITE->treasure]=1;
        }
      }
    }
  }
}

/* Render.
 */
 
static void _villager_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
  uint8_t tileid=SPRITE->tileid0;
  if (SPRITE->satisfied) tileid+=SPRITE->animframe?1:2;
  graf_tile(&g.graf,x,y,tileid,sprite->xform);
  
  // Word bubble?
  if (SPRITE->complaining) {
    int ht=NS_sys_tilesize>>1;
    graf_tile(&g.graf,x-ht,y-NS_sys_tilesize,0xb9,0);
    graf_tile(&g.graf,x+ht,y-NS_sys_tilesize,0xba,0);
    graf_tile(&g.graf,x-ht,y-NS_sys_tilesize*2,0xa9,0);
    graf_tile(&g.graf,x+ht,y-NS_sys_tilesize*2,0xaa,0);
    uint8_t tileid=tileid_for_treasure(SPRITE->treasure);
    if (tileid) {
      graf_tile(&g.graf,x,y-NS_sys_tilesize-ht-1,tileid,0);
    }
  }
  
  // Treasure going with me? Don't draw the sock; it's on her foot.
  if (SPRITE->satisfied&&(SPRITE->treasure!=NS_treasure_sock)) {
    int tx=x;
    int ty=y;
    if (SPRITE->treasure==NS_treasure_crown) {
      ty-=NS_sys_tilesize-1;
    } else {
      tx-=12;
      ty-=4;
    }
    graf_tile(&g.graf,tx,ty,tileid_for_treasure(SPRITE->treasure),EGG_XFORM_XREV);
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_villager={
  .name="villager",
  .objlen=sizeof(struct sprite_villager),
  .init=_villager_init,
  .update=_villager_update,
  .render=_villager_render,
};
