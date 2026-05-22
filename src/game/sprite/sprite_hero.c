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
  
  // High-level state. What is actually happening.
  int ducking;
  int dashing;
  int jumping;
  int walking;
  int wallsliding;
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
  return 0;
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

/* Update dash.
 */
 
static void hero_update_dash(struct sprite *sprite,double elapsed) {
  
  //TODO Dash in progress?
  
  if ((g.input&EGG_BTN_WEST)&&!(g.pvinput&EGG_BTN_WEST)) {
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
      sprite_move(sprite,0.0,-SPRITE->jump_velocity*elapsed);
      return;
    }
  }
  
  /* No jump in progress, so gravity is in play.
   * During a wallslide, capture the pre and post positions, then cheat it backward.
   * It's ok to force (y) in this case, since gravity will have confirmed a larger move was legal.
   */
  if (SPRITE->wallsliding) {
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
      //TODO Maybe schedule to pay out some additional velocity.
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
   * TODO Maybe similar treatment when dashing.
   */
  if (SPRITE->ducking) {
    //TODO As with the plain indx release, I think we might want some payout.
    SPRITE->walking=0;
    SPRITE->footfall_clock=0.0;
    return;
  }
  
  /* Move.
   * TODO Should velocity ramp up?
   */
  double speed=WALK_SPEED;
  int moved=sprite_move(sprite,speed*SPRITE->indx*elapsed,0.0);
  
  /* If the move failed and we're falling, do the wall slide.
   */
  if (!moved&&!sprite->seated&&!SPRITE->jumping) {
    //TODO No wallslide while carrying (and therefore no walljump either).
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
  //TODO hazards
}

/* Update.
 */
 
static void _hero_update(struct sprite *sprite,double elapsed) {
  hero_update_duck(sprite,elapsed);
  hero_update_dash(sprite,elapsed);
  hero_update_jump(sprite,elapsed);
  hero_update_walk(sprite,elapsed);
  hero_check_hazards(sprite);
}

/* Render.
 */
 
static void _hero_render(struct sprite *sprite,int x,int y) {
  graf_set_image(&g.graf,sprite->imageid);
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
