#include "game/kleptomania.h"

static struct {
  int TODO;
} vampire={0};

/* Begin.
 */
 
void vampire_begin() {
  memset(&vampire,0,sizeof(vampire));
  g.modal=MODAL_VAMPIRE;
}

/* Update.
 */
 
void vampire_update(double elapsed) {
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    g.modal=0;
    g.pvinput=g.input;
  }
}

/* Render.
 */
 
void vampire_render() {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  
  const char msg[]="TODO: Discuss with vampire";
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
