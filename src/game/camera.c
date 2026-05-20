#include "kleptomania.h"

/* Initialize scratch textures.
 */
 
int camera_init_scratches() {
  if ((g.bgbits=egg_texture_new())<1) return -1;
  if ((g.txbits=egg_texture_new())<1) return -1;
  if (egg_texture_load_raw(g.bgbits,FBW,FBH,FBW<<2,0,0)<0) return -1;
  if (egg_texture_load_raw(g.txbits,FBW,FBH,FBW<<2,0,0)<0) return -1;
  // Be sure of it:
  int w=0,h=0;
  egg_texture_get_size(&w,&h,g.bgbits);
  if ((w!=FBW)||(h!=FBH)) return -1;
  w=h=0;
  egg_texture_get_size(&w,&h,g.txbits);
  if ((w!=FBW)||(h!=FBH)) return -1;
  return 0;
}

/* Draw bgbits.
 */
 
static void camera_draw_bgbits() {
  graf_set_output(&g.graf,g.bgbits);
  const uint8_t *src=g.map->v;
  int yi=NS_sys_maph;
  int dsty=NS_sys_tilesize>>1;
  int dstx0=NS_sys_tilesize>>1;
  graf_fill_rect(&g.graf,0,0,FBW,FBH,g.map->bgcolor);
  graf_set_image(&g.graf,g.map->imageid);
  for (;yi-->0;dsty+=NS_sys_tilesize) {
    int xi=NS_sys_mapw;
    int dstx=dstx0;
    for (;xi-->0;dstx+=NS_sys_tilesize,src++) {
      graf_tile(&g.graf,dstx,dsty,*src,0);
    }
  }
  graf_set_output(&g.graf,1);
}

/* Render the game into the current graf context, at the specified origin.
 */
 
static void render_game_inner(int dstx,int dsty,int include_hero) {

  graf_set_input(&g.graf,g.bgbits);
  graf_decal(&g.graf,dstx,dsty,0,0,FBW,FBH);
  
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->defunct) continue;
    if ((sprite->type==&sprite_type_hero)&&!include_hero) continue;
    int x=(int)(sprite->x*NS_sys_tilesize)+dstx;
    int y=(int)(sprite->y*NS_sys_tilesize)+dsty;
    if (sprite->type->render) {
      sprite->type->render(sprite,x,y);
    } else {
      graf_set_image(&g.graf,sprite->imageid);
      graf_tile(&g.graf,x,y,sprite->tileid,sprite->xform);
    }
  }
}

/* Draw transition bits.
 */
 
void camera_draw_txbits(int include_hero) {
  graf_set_output(&g.graf,g.txbits);
  render_game_inner(0,0,include_hero);
  graf_set_output(&g.graf,1);
}

/* TRANSITION_FADE.
 */
 
static void render_game_FADE(double t) {
  double blackness; // 0..1
  if (t<0.5) {
    graf_set_input(&g.graf,g.txbits);
    graf_decal(&g.graf,0,0,0,0,FBW,FBH);
    blackness=t*2.0;
  } else {
    render_game_inner(0,0,1);
    blackness=(1.0-t)*2.0;
  }
  int alpha=(int)(blackness*255.0);
  if (alpha<=0) return;
  if (alpha>0xff) alpha=0xff;
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x00000000|alpha);
}

/* TRANSITION_PAN_*.
 * (dx,dy) is a normal vector in the direction of the camera's movement.
 */
 
static void render_game_PAN(double t,int dx,int dy) {
  int ox=(int)(-dx*FBW*t);
  int oy=(int)(-dy*FBH*t);
  int nx=ox+FBW*dx;
  int ny=oy+FBH*dy;
  graf_set_input(&g.graf,g.txbits);
  graf_decal(&g.graf,ox,oy,0,0,FBW,FBH);
  render_game_inner(nx,ny,1);
}

/* Render game, main entry point.
 */
 
void render_game() {

  /* If the background is dirty, redraw it.
   */
  if (g.bgbits_dirty) {
    g.bgbits_dirty=0;
    camera_draw_bgbits();
  }
  
  /* One pass of a cocktail sort on sprites.
   */
  if (g.spritec>1) {
    int first,last,d,i;
    if (g.sortd==1) {
      first=0;
      last=g.spritec-1;
      d=1;
    } else {
      first=g.spritec-1;
      last=0;
      d=-1;
    }
    for (i=first;i!=last;i+=d) {
      struct sprite *a=g.spritev[i];
      struct sprite *b=g.spritev[i+d];
      int cmp;
           if (a->layer<b->layer) cmp=-1;
      else if (a->layer>b->layer) cmp=1;
      else cmp=0;
      if (cmp==d) {
        g.spritev[i]=b;
        g.spritev[i+d]=a;
      }
    }
    if (g.sortd==1) g.sortd=-1;
    else g.sortd=1;
  }
  
  /* Transition in progress?
   */
  if (g.txclock>0.0) {
    double t=1.0-g.txclock/TRANSITION_TIME;
    if (t<0.0) t=0.0; else if (t>1.0) t=1.0;
    switch (g.transition) {
      case TRANSITION_FADE: render_game_FADE(t); break;
      case TRANSITION_PAN_LEFT: render_game_PAN(t,-1,0); break;
      case TRANSITION_PAN_RIGHT: render_game_PAN(t,1,0); break;
      case TRANSITION_PAN_UP: render_game_PAN(t,0,-1); break;
      case TRANSITION_PAN_DOWN: render_game_PAN(t,0,1); break;
      default: render_game_inner(0,0,1); break;
    }
  } else {
    render_game_inner(0,0,1);
  }
  
  //TODO overlay, anything independent of transition.
}
