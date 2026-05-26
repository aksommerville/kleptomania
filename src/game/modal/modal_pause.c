#include "game/kleptomania.h"

static struct pause {
  int x,y,w,h;
  int selp;
} pause={0};

/* Begin.
 */
 
void pause_begin() {
  memset(&pause,0,sizeof(pause));
  g.modal=MODAL_PAUSE;
  
  pause.w=100;
  pause.h=32;
  pause.x=(FBW>>1)-(pause.w>>1);
  pause.y=(FBH>>1)-(pause.h>>1);
}

/* Dismiss.
 */
 
static void pause_dismiss() {
  g.modal=0;
  g.pvinput=g.input;
}

/* Activate.
 */
 
static void pause_activate() {
  kl_sound(RID_sound_uiactivate);
  switch (pause.selp) {
    case 0: pause_dismiss(); break; // Resume
    case 1: pause_dismiss(); hello_begin(); break; // Main Menu
  }
}

/* Move cursor.
 */
 
static void pause_move(int d) {
  pause.selp^=1;
  kl_sound(RID_sound_typewriter);
}

/* Update.
 */
 
void pause_update(double elapsed) {
  if ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1)) { pause_dismiss(); return; }
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) { pause_activate(); return; }
  if ((g.input&EGG_BTN_UP)&&!(g.pvinput&EGG_BTN_UP)) pause_move(-1);
  else if ((g.input&EGG_BTN_DOWN)&&!(g.pvinput&EGG_BTN_DOWN)) pause_move(1);
}

/* Render.
 * Game renders first. Do not blackout the framebuffer.
 */
 
static void pause_string(int row,const char *src,int color) {
  graf_set_tint(&g.graf,color);
  int srcc=0; while (src[srcc]) srcc++;
  int y=pause.y+4+row*8+4;
  int x=pause.x+(pause.w>>1)-(srcc*4)+4;
  for (;srcc-->0;src++,x+=8) {
    if ((unsigned char)(*src)<=0x20) continue;
    graf_tile(&g.graf,x,y,*src,0);
  }
  graf_set_tint(&g.graf,0);
}
 
void pause_render() {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000a0);
  graf_fill_rect(&g.graf,pause.x+1,pause.y+1,pause.w,pause.h,0x000000c0);
  graf_fill_rect(&g.graf,pause.x,pause.y,pause.w,pause.h,0x000000ff);
  graf_set_image(&g.graf,RID_image_fonttiles);
  pause_string(0,"~ Paused ~",0x808080ff);
  pause_string(1,"Resume",0xffffffff);
  pause_string(2,"Main Menu",0xffffffff);
  graf_set_image(&g.graf,RID_image_sprites);
  graf_tile(&g.graf,pause.x,pause.y+8+(pause.selp+1)*8,0x45,0);
}
