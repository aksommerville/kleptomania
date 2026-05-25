#include "game/kleptomania.h"

static struct {
  int TODO;
} hello={0};

/* Begin.
 */
 
void hello_begin() {
  memset(&hello,0,sizeof(hello));
  g.modal=MODAL_HELLO;
  egg_play_song(1,RID_song_kleptomania,1,0.333,0.0);
}

/* Update.
 */
 
void hello_update(double elapsed) {
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    kl_sound(RID_sound_uiactivate);
    g.modal=0;
    g.pvinput=g.input; // Neuter this keystroke.
    if (game_reset()<0) egg_terminate(1);
  }
}

/* Render.
 */
 
void hello_render() {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  const char msg[]="Kleptomania";
  int msgc=sizeof(msg)-1;
  int msgw=8*msgc;
  int x=(FBW>>1)-(msgw>>1)+4;
  int y=FBH>>1;
  int i=0;
  graf_set_image(&g.graf,RID_image_fonttiles);
  for (;i<msgc;i++,x+=8) {
    graf_tile(&g.graf,x,y,msg[i],0);
  }
}
