#include "game/kleptomania.h"

static struct {
  int TODO;
} gameover={0};

/* Begin.
 */
 
void gameover_begin() {
  memset(&gameover,0,sizeof(gameover));
  g.modal=MODAL_GAMEOVER;
  egg_play_song(1,0,1,0.0,0.0);
}

/* Update.
 */
 
void gameover_update(double elapsed) {
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    kl_sound(RID_sound_uiactivate);
    g.modal=0;
    g.pvinput=g.input;
    hello_begin();
  }
}

/* Render.
 */
 
void gameover_render() {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  const char msg[]="You win!";
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
