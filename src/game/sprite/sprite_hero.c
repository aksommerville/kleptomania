#include "game/kleptomania.h"

#define DUCK_TIME_MIN 0.250
#define WALLJUMP_INITIAL 18.0 /* m/s ; Beyond this, it's just a jump. */
#define JUMP_INITIAL     22.0 /* m/s */
#define JUMP_DECEL       60.0 /* m/s**2 */
#define WALK_SPEED        7.0 /* m/s */
#define WALLSLIDE_DIMINUATION 0.250
#define COYOTE_TIME    0.100 /* Allow jumping so far into a fall. */
#define PREBOUNCE_TIME 0.100 /* Allow jumping if triggered so close to landing. */
#define FOOTFALL_TIME  0.120
#define WALLSLIDE_TIME 0.110
#define DASH_TIME      0.150
#define SHADOW_LIMIT 6
#define SHADOW_TIME 0.250
#define CARRY_BLACKOUT_TIME 0.500 /* Can't pick something up so close to having dropped it. */

struct sprite_hero {
  struct sprite hdr;
  
  // State constituents.
  int indx; // -1,0,1, straight off (g.input).
  double duck_clock; // Counts down to enforce minimum duck time.
  double walk_animclock;
  int walk_animframe;
  double jump_velocity; // m/s, decreases during the jump.
  double fall_clock;
  double prebounce; // Sample of (fall_clock) when a jump was rejected due to unseated.
  double footfall_clock;
  double wallslide_clock;
  double dash_clock;
  int dash_charged;
  int dashdx,dashdy;
  double carry_blackout;
  
  // High-level state. What is actually happening.
  int ducking;
  int dashing;
  int jumping;
  int walking;
  int wallsliding;
  int carrying; // 0 or NS_treasure_*
  
  double shadow_clock;
  struct shadow {
    double x,y;
    double ttl;
    int ducking;
    uint8_t xform;
  } shadowv[SHADOW_LIMIT];
  int shadowc;
};

#define SPRITE ((struct sprite_hero*)sprite)

/* Cleanup.
 */
 
static void _hero_del(struct sprite *sprite) {
}

/* Init.
 */
 
static int _hero_init(struct sprite *sprite) {
  SPRITE->prebounce=-1.0;
  SPRITE->dash_charged=1;
  return 0;
}

/* Charge dash if not charged.
 */
 
static void hero_charge_dash(struct sprite *sprite) {
  if (SPRITE->dash_charged) return;
  SPRITE->dash_charged=1;
  kl_sound(RID_sound_charge_dash);
}

/* Gravity events.
 */
 
static void hero_make_landing_dust(struct sprite *sprite) {
  double x=sprite->x;
  double y=sprite->y+sprite->hbb;
  struct sprite *dust=sprite_spawn(&sprite_type_dust,x,y,0,0);
}
 
static void _hero_landed(struct sprite *sprite,double velocity) {
  /* The lowest velocity I can get by jumping is just a hair over 6.
   * Initial collision, when you haven't actually fallen, is a bit under 1.
   * From 1 meter, 9-ish. From 2 meters, 13-ish, 3 and above, the full 15.
   */
  //fprintf(stderr,"%s vel=%.03f dur=%.03f\n",__func__,velocity,SPRITE->fall_clock);
  double prebounce=SPRITE->fall_clock-SPRITE->prebounce;
  if ((g.input&EGG_BTN_SOUTH)&&(prebounce>=0.0)&&(prebounce<PREBOUNCE_TIME)) {
    // Prebounce jump. Triggered slightly before landing.
    // We don't effect the jump itself, we just lie about the input state so our next update will see it as freshly pressed.
    g.input&=~EGG_BTN_SOUTH;
  } else if (velocity<1.000) {
    // Too small to warrant any feedback at all. We might have turned gravity back on when already seated.
  } else if (velocity<10.000) {
    // Covers up to 1-meter drops.
    kl_sound(RID_sound_land_minor);
  } else if (velocity<15.000) {
    // 2-meter drops.
    kl_sound(RID_sound_land_middle);
    hero_make_landing_dust(sprite);
  } else {
    // 3-meter or higher drops.
    kl_sound(RID_sound_land_major);
    hero_make_landing_dust(sprite);
  }
  if (SPRITE->fall_clock>=0.750) { // 750 is just enough to create it with a simple jump. At 800, you'd have to also drop one meter.
    g.screenshake=0.100;
  }
  SPRITE->fall_clock=0.0;
  SPRITE->prebounce=-1.0;
  hero_charge_dash(sprite);
}

static void _hero_falling(struct sprite *sprite) {
  SPRITE->fall_clock=0.0;
  SPRITE->prebounce=-1.0;
}

/* Update duck state.
 */
 
static void hero_update_duck(struct sprite *sprite,double elapsed) {
  if (SPRITE->ducking) {
    if ((SPRITE->duck_clock-=elapsed)>0.0) {
      // Brief interval where you can't release the duck.
    } else if (!(g.input&EGG_BTN_DOWN)||!sprite->seated) {
      SPRITE->ducking=0;
      if (sprite->seated) kl_sound(RID_sound_unduck); // No unduck sound if downjumping.
    }
  } else {
    //TODO Any other state conditions restricting duck?
    if (sprite->seated&&(g.input&EGG_BTN_DOWN)) {
      SPRITE->ducking=1;
      SPRITE->duck_clock=DUCK_TIME_MIN;
      kl_sound(RID_sound_duck);
    }
  }
}

/* Add a shadow to the list, if there's room.
 */
 
static void hero_add_shadow(struct sprite *sprite) {
  
  // First reap anything defunct.
  int i=SPRITE->shadowc;
  struct shadow *shadow=SPRITE->shadowv+i-1;
  for (;i-->0;shadow--) {
    if (shadow->ttl<=0.0) {
      SPRITE->shadowc--;
      memmove(shadow,shadow+1,sizeof(struct shadow)*(SPRITE->shadowc-i));
    }
  }
  
  if (SPRITE->shadowc>=SHADOW_LIMIT) return;
  shadow=SPRITE->shadowv+SPRITE->shadowc++;
  shadow->x=sprite->x;
  shadow->y=sprite->y;
  shadow->ttl=SHADOW_TIME;
  shadow->ducking=SPRITE->ducking;
  shadow->xform=sprite->xform;
}

/* Drop whatever we're carrying.
 */
 
static void hero_drop(struct sprite *sprite) {
  if (!SPRITE->carrying) return;
  
  // Ensure we have some freedom in the facing direction, don't drop it deep into a wall.
  double x0=sprite->x;
  int moved=(
    sprite_move(sprite,(sprite->xform&EGG_XFORM_XREV)?-0.5:0.5,0.0)&&
    sprite_move(sprite,(sprite->xform&EGG_XFORM_XREV)?-0.75:0.75,0.0)
  );
  sprite->x=x0;
  if (!moved) return;
  
  kl_sound(RID_sound_drop);
  int treasure=SPRITE->carrying;
  SPRITE->carrying=0;
  
  int rid=spriteid_for_treasure(treasure);
  if (rid) {
    double x=sprite->x;
    double y=sprite->y-(2.0/NS_sys_tilesize);
    if (sprite->xform&EGG_XFORM_XREV) x-=1.000;
    else x+=1.000;
    struct sprite *treasure=sprite_spawn(0,x,y,rid,0);
    if (treasure) {
      treasure->xform=sprite->xform;
    }
  }
  
  SPRITE->carry_blackout=CARRY_BLACKOUT_TIME;
}

/* Update dash.
 */
 
static void hero_update_dash(struct sprite *sprite,double elapsed) {

  /* Tick shadows and remove from the end only.
   */
  if (SPRITE->shadowc>0) {
    struct shadow *shadow=SPRITE->shadowv;
    int i=SPRITE->shadowc;
    for (;i-->0;shadow++) {
      shadow->ttl-=elapsed;
    }
    while (SPRITE->shadowc&&(SPRITE->shadowv[SPRITE->shadowc-1].ttl<=0.0)) SPRITE->shadowc--;
  }
  
  /* Dash in progress? Pay it out.
   */
  if (SPRITE->dashing) {
    if ((SPRITE->shadow_clock-=elapsed)<=0.0) {
      SPRITE->shadow_clock+=0.050;
      hero_add_shadow(sprite);
    }
    if ((SPRITE->dash_clock-=elapsed)<=0.0) {
      SPRITE->dashing=0;
      if (sprite->seated) {
        // Gravity is suspended during most dashes. Try nudging down a little, to see whether we need to recharge.
        if (sprite_move(sprite,0.0,0.010)) {
          // Not really seated.
        } else {
          hero_charge_dash(sprite);
        }
      }
    } else {
      double speed=20.0;
      if (SPRITE->dashdx&&!sprite_move(sprite,speed*SPRITE->dashdx*elapsed,0.0)) {
        hero_charge_dash(sprite);
      }
      if (SPRITE->dashdy&&!sprite_move(sprite,0.0,speed*SPRITE->dashdy*elapsed)) {
        hero_charge_dash(sprite);
      }
    }
  }
  
  /* Start a new dash? You can do this even when a dash is currently paying out.
   * The new one cancels the old one.
   */
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
    if (SPRITE->carrying) {
      hero_drop(sprite);
    } else if (!SPRITE->dash_charged) {
      kl_sound(RID_sound_dash_reject);
    } else {
      kl_sound(RID_sound_dash);
      SPRITE->dashing=1;
      SPRITE->dash_charged=0;
      SPRITE->dash_clock=DASH_TIME;
      SPRITE->shadow_clock=0.0;
      SPRITE->dashdx=SPRITE->dashdy=0;
      switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
        case EGG_BTN_LEFT: SPRITE->dashdx=-1; break;
        case EGG_BTN_RIGHT: SPRITE->dashdx=1; break;
      }
      switch (g.input&(EGG_BTN_UP|EGG_BTN_DOWN)) {
        case EGG_BTN_UP: SPRITE->dashdy=-1; break;
        case EGG_BTN_DOWN: SPRITE->dashdy=1; break;
      }
      if (!SPRITE->dashdx&&!SPRITE->dashdy) {
        // Dash without dpad: Go horizontal in the direction we're facing.
        if (sprite->xform&EGG_XFORM_XREV) {
          SPRITE->dashdx=-1;
        } else {
          SPRITE->dashdx=1;
        }
      }
      if (!SPRITE->dashdx&&(SPRITE->dashdy>0)&&sprite->seated) {
        // Likewise, if we're only holding down, and already at the ground, call it a forward diagonal.
        if (sprite->xform&EGG_XFORM_XREV) {
          SPRITE->dashdx=-1;
        } else {
          SPRITE->dashdx=1;
        }
      }
    }
  }
}

/* Jump while ducking. Begin down-jump if we can, otherwise do nothing.
 */
 
static void hero_down_jump_maybe(struct sprite *sprite) {
  if (!SPRITE->ducking||!sprite->seated) return; // Should already have been checked but hey.
  
  /* This cheesy cheat should be pretty resilient:
   * Force my (y) just a hair lower, then test whether we can move down from there.
   * If so, leave me in that cheated position. If not, put me back where we were.
   */
  double y0=sprite->y;
  sprite->y+=0.010; // <1px
  int can_drop=sprite_move(sprite,0.0,0.100);
  sprite->y=y0;
  if (!can_drop) return; // Nope, it's solid or something.
  sprite->y+=0.010;
  kl_sound(RID_sound_downjump);
}

/* Update jump or gravity.
 */
 
static void hero_update_jump(struct sprite *sprite,double elapsed) {

  /* Fall ongoing?
   */
  if (!sprite->seated) {
    if (SPRITE->wallsliding) {
      SPRITE->fall_clock=0.0;
    } else {
      SPRITE->fall_clock+=elapsed;
    }
  }

  /* Jump ongoing?
   */
  if (SPRITE->jumping) {
    SPRITE->jump_velocity-=JUMP_DECEL*elapsed;
    // Terminate due to exhaustion?
    if (SPRITE->jump_velocity<=0.0) {
      SPRITE->jumping=0;
    // Terminate due to released key?
    } else if (!(g.input&EGG_BTN_SOUTH)) {
      SPRITE->jumping=0;
    // Proceed with jump.
    } else {
      sprite_force_null_gravity(sprite);
      if (!sprite_move(sprite,0.0,-SPRITE->jump_velocity*elapsed)) {
        // Hit the ceiling, get your dash back.
        hero_charge_dash(sprite);
      }
      return;
    }
  }
  
  /* No jump in progress, so gravity is in play.
   * During a wallslide, capture the pre and post positions, then cheat it backward.
   * It's ok to force (y) in this case, since gravity will have confirmed a larger move was legal.
   */
  if (SPRITE->dashing&&(SPRITE->dashdy<=0)) {
    // Suspend gravity during dash, unless it's pointing down. Then the loss of gravity is apparent and weird.
  } else if (SPRITE->wallsliding) {
    double ypre=sprite->y;
    sprite_update_gravity(sprite,elapsed);
    double ypost=sprite->y;
    if ((ypost>ypre+0.100)&&!sprite->seated) {
      sprite->y=ypre+(ypost-ypre)*WALLSLIDE_DIMINUATION;
    }
    if ((SPRITE->wallslide_clock-=elapsed)<=0.0) {
      SPRITE->wallslide_clock+=WALLSLIDE_TIME;
      kl_sound(RID_sound_wallslide);
    }
  } else {
    sprite_update_gravity(sprite,elapsed);
  }
  
  /* Requesting to start a jump?
   */
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    if (SPRITE->wallsliding) {
      SPRITE->fall_clock=0.0;
      SPRITE->prebounce=-1.0;
      SPRITE->wallsliding=0;
      SPRITE->jumping=1;
      SPRITE->jump_velocity=WALLJUMP_INITIAL;
      kl_sound(RID_sound_walljump);
      return;
    }
    if (!sprite->seated) {
      if (SPRITE->fall_clock<COYOTE_TIME) {
        // Coyote jump, let it thru.
      } else {
        // Record time in case it prebounces.
        SPRITE->prebounce=SPRITE->fall_clock;
        return;
      }
    }
    if (SPRITE->ducking) {
      hero_down_jump_maybe(sprite);
      return;
    }
    SPRITE->jumping=1;
    SPRITE->jump_velocity=JUMP_INITIAL;
    kl_sound(RID_sound_jump);
  }
}

/* Update walking.
 */
 
static void hero_update_walk(struct sprite *sprite,double elapsed) {

  /* Wall slide is guilty until proven innocent; see way below.
   */
  int pvwallslide=SPRITE->wallsliding;
  SPRITE->wallsliding=0;

  /* Collect current input state.
   */
  int nindx=0;
  switch (g.input&(EGG_BTN_LEFT|EGG_BTN_RIGHT)) {
    case EGG_BTN_LEFT: nindx=-1; break;
    case EGG_BTN_RIGHT: nindx=1; break;
  }
  
  /* Update xform on any input change to nonzero.
   */
  if (nindx!=SPRITE->indx) {
    if (nindx<0) sprite->xform=EGG_XFORM_XREV;
    else if (nindx>0) sprite->xform=0;
    SPRITE->indx=nindx;
  }
  
  /* Did we stop walking, or continuing not to walk?
   */
  if (!SPRITE->indx) {
    if (SPRITE->walking) {
      SPRITE->walking=0;
    }
    SPRITE->footfall_clock=0.0;
    return;
  }
  
  /* Are we just starting to walk?
   * Reject depending on state.
   */
  if (!SPRITE->walking) {
    SPRITE->footfall_clock=0.0;
    if (SPRITE->ducking) return;
    SPRITE->walking=1;
    SPRITE->walk_animclock=0.0;
    SPRITE->walk_animframe=0;
  }
  
  /* If we started ducking, abort the walk.
   */
  if (SPRITE->ducking) {
    SPRITE->walking=0;
    SPRITE->footfall_clock=0.0;
    return;
  }
  
  /* Move.
   */
  double speed=WALK_SPEED;
  int moved=sprite_move(sprite,speed*SPRITE->indx*elapsed,0.0);
  
  /* Dash recharges if movement fails -- even horizontal movement.
   */
  if (!moved&&!SPRITE->dash_charged) {
    hero_charge_dash(sprite);
  }
  
  /* If the move failed and we're falling, do the wall slide.
   */
  if (!moved&&!sprite->seated&&!SPRITE->jumping&&!SPRITE->carrying) {
    SPRITE->wallsliding=1;
    if (!pvwallslide) SPRITE->wallslide_clock=0.200; // Delay the first tick.
  }
  
  /* Animate.
   */
  if ((SPRITE->walk_animclock-=elapsed)<=0.0) {
    SPRITE->walk_animclock+=0.150;
    if (++(SPRITE->walk_animframe)>=4) SPRITE->walk_animframe=0;
  }
  
  /* Sound effects periodically, if we're seated.
   */
  if (!moved||!sprite->seated) {
    SPRITE->footfall_clock=0.0;
  } else if ((SPRITE->footfall_clock-=elapsed)<=0.0) {
    SPRITE->footfall_clock+=FOOTFALL_TIME;
    kl_sound(RID_sound_footfall);
  }
}

/* Check hazards.
 */
 
static void hero_check_hazards(struct sprite *sprite) {
  int x=(int)sprite->x;
  if ((x<0)||(x>=NS_sys_mapw)) return;
  int ay=(int)sprite->y;
  int by=(int)(sprite->y-0.75);
  if ((ay>=0)&&(ay<NS_sys_maph)) {
    uint8_t tileid=g.map->v[ay*NS_sys_mapw+x];
    uint8_t physics=g.map->physics[tileid];
    if (physics==NS_physics_hazard) {
      sprite->defunct=1;
      struct sprite *soulballs=sprite_spawn(&sprite_type_soulballs,sprite->x,sprite->y,0,0);
      kl_sound(RID_sound_die);
      g.deathc++;
      return;
    }
    if (physics==NS_physics_safe) {
      g.safex=x;
      g.safey=ay;
    }
  }
  if ((by!=ay)&&(by>=0)&&(by<NS_sys_maph)) {
    uint8_t tileid=g.map->v[by*NS_sys_mapw+x];
    uint8_t physics=g.map->physics[tileid];
    if (physics==NS_physics_hazard) {
      sprite->defunct=1;
      struct sprite *soulballs=sprite_spawn(&sprite_type_soulballs,sprite->x,sprite->y,0,0);
      kl_sound(RID_sound_die);
      g.deathc++;
      return;
    }
  }
}

/* Kill me if we're more than a screenful away from the screen. That can only mean trouble.
 */
 
static void hero_kill_if_distant(struct sprite *sprite) {
  int rx=(int)sprite->x/NS_sys_mapw; if (sprite->x<0.0) rx--;
  int ry=(int)sprite->y/NS_sys_maph; if (sprite->y<0.0) ry--;
  //fprintf(stderr,"%s %d,%d f=%f,%f\n",__func__,rx,ry,sprite->x,sprite->y);
  if ((rx<-1)||(ry<-1)||(rx>1)||(ry>1)) {
    fprintf(stderr,"Hero too far offscreen. Killing.\n");
    sprite->defunct=1;
    kl_sound(RID_sound_die);
    g.deathc++;
  }
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  if (SPRITE->carry_blackout>0.0) SPRITE->carry_blackout-=elapsed;
  hero_update_duck(sprite,elapsed);
  hero_update_dash(sprite,elapsed);
  hero_update_jump(sprite,elapsed);
  hero_update_walk(sprite,elapsed);
  hero_check_hazards(sprite);
  hero_kill_if_distant(sprite);
}

/* Render.
 */
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
  
  // Shadows, triggered by dashing.
  if (SPRITE->shadowc>0) {
    int expectx=(int)(sprite->x*NS_sys_tilesize);
    int expecty=(int)(sprite->y*NS_sys_tilesize);
    int dx=x-expectx;
    int dy=y-expecty;
    struct shadow *shadow=SPRITE->shadowv;
    int i=SPRITE->shadowc;
    for (;i-->0;shadow++) {
      if (shadow->ttl<=0.0) continue;
      int alpha=(int)((shadow->ttl*96.0)/SHADOW_TIME);
      if (alpha<=0) continue;
      if (alpha>0xff) alpha=0xff;
      graf_set_alpha(&g.graf,alpha);
      int shx=(int)(shadow->x*NS_sys_tilesize)+dx;
      int shy=(int)(shadow->y*NS_sys_tilesize)+dy;
      uint8_t tileid=sprite->tileid;
      if (shadow->ducking) tileid+=0x01;
      graf_tile(&g.graf,shx,shy,tileid,shadow->xform);
      graf_tile(&g.graf,shx,shy-NS_sys_tilesize,tileid-0x10,shadow->xform);
    }
    graf_set_alpha(&g.graf,0xff);
  }
  
  // When dashing, tint.
  if (SPRITE->dashing) {
    double t=SPRITE->dash_clock/DASH_TIME;
    if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
    int alpha=0x40+(int)(0x80*t);
    graf_set_tint(&g.graf,0xffffff00|alpha);
  }
  
  // Main body.
  uint8_t tileid=sprite->tileid; // (sprite->tileid) is constant. We choose the real tile dynamically, right here.
  if (SPRITE->ducking) {
    tileid+=0x01;
  } else if (SPRITE->wallsliding) {
    tileid+=0x04;
  } else if (SPRITE->jumping) {
    tileid+=0x05;
    y+=2; // Tile is offset by 2 pixels.
  } else if (!sprite->seated) {
    tileid+=0x06;
  } else if (SPRITE->walking) {
    switch (SPRITE->walk_animframe) {
      case 1: tileid+=0x02; break;
      case 3: tileid+=0x03; break;
    }
  }
  graf_tile(&g.graf,x,y,tileid,sprite->xform);
  graf_tile(&g.graf,x,y-NS_sys_tilesize,tileid-0x10,sprite->xform);
  
  // If we're carrying something, draw it, then an overlay for my forward arm.
  if (SPRITE->carrying) {
    uint8_t carrytileid=tileid_for_treasure(SPRITE->carrying);
    if (carrytileid) {
      int cx=x;
      if (sprite->xform&EGG_XFORM_XREV) cx-=11;
      else cx+=11;
      int cy=y-2;
      if (SPRITE->jumping) cy-=2;
      else if (SPRITE->ducking) cy+=4;
      graf_tile(&g.graf,cx,cy,carrytileid,sprite->xform);
    }
    int overx=x;
    if (sprite->xform&EGG_XFORM_XREV) overx-=4;
    else overx+=4;
    graf_tile(&g.graf,overx,y,tileid+0x20,sprite->xform);
    graf_tile(&g.graf,overx,y-NS_sys_tilesize,tileid+0x10,sprite->xform);
  }
  
  graf_set_tint(&g.graf,0);
}

/* Type definition.
 */
 
const struct sprite_type sprite_type_hero={
  .name="hero",
  .objlen=sizeof(struct sprite_hero),
  .del=_hero_del,
  .init=_hero_init,
  .update=_hero_update,
  .render=_hero_render,
  .landed=_hero_landed,
  .falling=_hero_falling,
};

/* Get the global hero sprite if there is one.
 */
 
struct sprite *get_hero() {
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->type!=&sprite_type_hero) continue;
    if (sprite->defunct) continue;
    return sprite;
  }
  return 0;
}

/* Start carrying a treasure if we can.
 */
 
int sprite_hero_carry(struct sprite *sprite,int treasure) {
  if (!sprite||(sprite->type!=&sprite_type_hero)) return 0;
  if (!treasure) { // Always able to nix the carry. Whatever happens to it after is the caller's responsibility.
    SPRITE->carrying=0;
    return 1;
  }
  if (SPRITE->carrying) return 0; // One thing at a time.
  if (SPRITE->carry_blackout>0.0) return 0; // Please hold.
  if (SPRITE->dashing) return 0; // Stop dashing first.
  kl_sound(RID_sound_pickup);
  SPRITE->carrying=treasure;
  return 1;
}

int sprite_hero_carrying(const struct sprite *sprite) {
  if (!sprite||(sprite->type!=&sprite_type_hero)) return 0;
  return SPRITE->carrying;
}
