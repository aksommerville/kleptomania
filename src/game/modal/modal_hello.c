#include "game/kleptomania.h"

#define VTX_LIMIT 64
#define BLINK_PERIOD 0.800
#define BLINK_DUTY   0.600

static struct {
  struct egg_render_tile vtxv[VTX_LIMIT];
  int vtxc;
  int vtxc_always; // Those from here to (vtxc) blink.
  double blinkclock;
} hello={0};

/* Prepare (vtxv) with our text.
 */
 
static void hello_prepare_vertices() {

  /* If we have high scores, print them a little left of center and low of center.
   */
  if (g.hiscore_100[0]) {
    int x=40;
    int y=107;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'1',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'0',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'0',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'%',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,':',0}; x+=16;
    const char *src=g.hiscore_100;
    int i=9;
    if (src[0]=='0') { src++; i--; } // High scores will usually not have a tens digit.
    for (;i-->0;src++,x+=8) {
      hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,*src,0};
    }
  }
  if (g.hiscore_any[0]) {
    int x=40;
    int y=115;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'A',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'n',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'y',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,'%',0}; x+=8;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,':',0}; x+=16;
    const char *src=g.hiscore_any;
    int i=9;
    if (src[0]=='0') { src++; i--; } // High scores will usually not have a tens digit.
    for (;i-->0;src++,x+=8) {
      hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,*src,0};
    }
  }
  hello.vtxc_always=hello.vtxc;
  
  /* Then "Press Start Key", blinking.
   */
  int x=45;
  int y=80;
  const char *msg="Press Start Key";
  for (;*msg;msg++,x+=8) {
    if ((unsigned char)(*msg)<=0x20) continue;
    hello.vtxv[hello.vtxc++]=(struct egg_render_tile){x,y,*msg,0};
  }
}

/* Begin.
 */
 
void hello_begin() {
  memset(&hello,0,sizeof(hello));
  g.modal=MODAL_HELLO;
  egg_play_song(1,RID_song_kleptomania,1,0.333,0.0);
  hello_prepare_vertices();
}

/* Update.
 */
 
void hello_update(double elapsed) {

  if ((hello.blinkclock-=elapsed)<=0.0) {
    hello.blinkclock+=BLINK_PERIOD;
  }

  if (
    ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH))||
    ((g.input&EGG_BTN_AUX1)&&!(g.pvinput&EGG_BTN_AUX1)) /* It's normally SOUTH, but we do say "Start Key", which is more like AUX1. */
  ) {
    kl_sound(RID_sound_uiactivate);
    g.modal=0;
    g.pvinput=g.input; // Neuter this keystroke.
    if (game_reset()<0) egg_terminate(1);
  }
}

/* Render.
 */
 
void hello_render() {
  graf_set_image(&g.graf,RID_image_dialogue);
  graf_decal(&g.graf,0,0,0,128,FBW,FBH);
  graf_set_image(&g.graf,RID_image_fonttiles);
  if (hello.blinkclock<BLINK_DUTY) {
    graf_tile_batch(&g.graf,hello.vtxv,hello.vtxc);
  } else {
    graf_tile_batch(&g.graf,hello.vtxv,hello.vtxc_always);
    graf_set_alpha(&g.graf,0x80);
    graf_tile_batch(&g.graf,hello.vtxv+hello.vtxc_always,hello.vtxc-hello.vtxc_always);
    graf_set_alpha(&g.graf,0xff);
  }
}
