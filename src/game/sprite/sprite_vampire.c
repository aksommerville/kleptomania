#include "game/kleptomania.h"

#define STAGE_PLAY 1 /* Talking to my sock. Enter cutscene on contact. */
#define STAGE_WAIT 2 /* Stand still and await my watermelon. */
#define STAGE_DONE 3 /* Gaze upon my watermelon. */

struct sprite_vampire {
  struct sprite hdr;
  int stage;
  double animclock;
  int animframe; // Different range per stage.
  int complaining; // Just like villagers. Only applicable in STAGE_WAIT.
};

#define SPRITE ((struct sprite_vampire*)sprite)

/* Init.
 */
 
static int _vampire_init(struct sprite *sprite) {
  if (g.treasurev[NS_treasure_watermelon]) {
    SPRITE->stage=STAGE_DONE;
    SPRITE->animframe=10;
  } else if (g.talked_to_vampire) {
    SPRITE->stage=STAGE_WAIT;
  } else {
    SPRITE->stage=STAGE_PLAY;
  }
  return 0;
}

/* Update, STAGE_PLAY. It's a static animated sequence, until the hero touches us.
 */
 
static void vampire_update_PLAY(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.300;
    if (++(SPRITE->animframe)>=24) SPRITE->animframe=0;
  }
  struct sprite *hero=get_hero();
  if (!hero) return;
  double dx=hero->x-sprite->x;
  double dy=hero->y-sprite->y;
  if ((dy>-1.0)&&(dy<1.0)&&(dx>-1.0)&&(dx<1.0)) {
    vampire_begin();
    g.talked_to_vampire=1;
    SPRITE->stage=STAGE_WAIT;
    SPRITE->animclock=0.0;
    SPRITE->animframe=0;
    if (dx<0.0) sprite->xform=EGG_XFORM_XREV;
  }
}

/* Update, STAGE_WAIT. We're the same as a villager now.
 */
 
static void vampire_update_WAIT(struct sprite *sprite,double elapsed) {
  SPRITE->complaining=0;
  struct sprite *hero=get_hero();
  if (!hero) return; // Will be waiting a while, i guess...
  double dx=hero->x-sprite->x;
  if (dx<0.0) sprite->xform=EGG_XFORM_XREV;
  else sprite->xform=0;
  const double COMPLAIN_RADIUS=2.000;
  const double DELIVER_RADIUS =1.000;
  const double VERTICAL_RADIUS=1.000;
  if ((dx<-COMPLAIN_RADIUS)||(dx>COMPLAIN_RADIUS)) return;
  double dy=hero->y-sprite->y;
  if ((dy<-VERTICAL_RADIUS)||(dy>VERTICAL_RADIUS)) return;
  if ((dx>-DELIVER_RADIUS)&&(dx<DELIVER_RADIUS)&&(sprite_hero_carrying(hero)==NS_treasure_watermelon)) {
    kl_sound(RID_sound_deliver);
    sprite_hero_carry(hero,0); // Must zero before giving the new one.
    sprite_hero_carry(hero,NS_treasure_sock);
    g.treasurev[NS_treasure_watermelon]=1;
    SPRITE->stage=STAGE_DONE;
    SPRITE->animclock=0.0;
    SPRITE->animframe=10;
    sprite->xform=0;
  } else {
    SPRITE->complaining=1;
  }
}

/* Update, STAGE_DONE. Animate and nothing else.
 */
 
static void vampire_update_DONE(struct sprite *sprite,double elapsed) {
  if ((SPRITE->animclock-=elapsed)<=0.0) {
    SPRITE->animclock+=0.120;
    if (++(SPRITE->animframe)>=50) SPRITE->animframe=0;
  }
}

/* Update.
 */
 
static void _vampire_update(struct sprite *sprite,double elapsed) {
  switch (SPRITE->stage) {
    case STAGE_PLAY: vampire_update_PLAY(sprite,elapsed); break;
    case STAGE_WAIT: vampire_update_WAIT(sprite,elapsed); break;
    case STAGE_DONE: vampire_update_DONE(sprite,elapsed); break;
  }
}

/* Render.
 */
 
static void _vampire_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
  uint8_t tileid=sprite->tileid;
  switch (SPRITE->stage) {
    case STAGE_PLAY: switch (SPRITE->animframe) {
        case  0: tileid+=0; break;
        case  1: tileid+=1; break;
        case  2: tileid+=0; break;
        case  3: tileid+=1; break;
        case  4: tileid+=0; break;
        case  5: tileid+=1; break;
        case  6: tileid+=0; break;
        case  7: tileid+=2; break;
        case  8: tileid+=0; break;
        case  9: tileid+=2; break;
        case 10: tileid+=0; break;
        case 11: tileid+=2; break;
        case 12: tileid+=0; break;
        case 13: tileid+=1; break;
        case 14: tileid+=0; break;
        case 15: tileid+=1; break;
        case 16: tileid+=3; break;
        case 17: tileid+=4; break;
        case 18: tileid+=3; break;
        case 19: tileid+=4; break;
        case 20: tileid+=3; break;
        case 21: tileid+=4; break;
        case 22: tileid+=3; break;
        case 23: tileid+=4; break;
      } break;
    case STAGE_WAIT: {
        tileid+=5;
      } break;
    case STAGE_DONE: switch (SPRITE->animframe) {
        case 0: tileid+=7; break;
        case 1: tileid+=8; break;
        case 2: tileid+=9; break;
        case 3: tileid+=10; break;
        default: tileid+=6; break;
      } break;
  }
  graf_tile(&g.graf,x,y,tileid,sprite->xform);
  
  // Complain just like a villager, as warranted.
  if ((SPRITE->stage==STAGE_WAIT)&&SPRITE->complaining) {
    int ht=NS_sys_tilesize>>1;
    graf_tile(&g.graf,x-ht,y-NS_sys_tilesize,0xb9,0);
    graf_tile(&g.graf,x+ht,y-NS_sys_tilesize,0xba,0);
    graf_tile(&g.graf,x-ht,y-NS_sys_tilesize*2,0xa9,0);
    graf_tile(&g.graf,x+ht,y-NS_sys_tilesize*2,0xaa,0);
    uint8_t tileid=tileid_for_treasure(NS_treasure_watermelon);
    if (tileid) {
      graf_tile(&g.graf,x,y-NS_sys_tilesize-ht-1,tileid,0);
    }
  }
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_vampire={
  .name="vampire",
  .objlen=sizeof(struct sprite_vampire),
  .init=_vampire_init,
  .update=_vampire_update,
  .render=_vampire_render,
};
