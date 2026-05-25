#include "game/kleptomania.h"

static struct {
  int texid;
  int texw,texh;
} gameover={0};

/* Generate a line of the final report.
 * Must be set up in (g.graf).
 */
 
static void gameover_report_text(int row,const char *src,int srcc) {
  if (!src) return;
  if (srcc<0) { srcc=0; while (src[srcc]) srcc++; }
  int x=(gameover.texw>>1)-(srcc*4)+4;
  int y=row*8+4;
  for (;srcc-->0;src++,x+=8) {
    if ((*src<=0x20)||(*src>=0x7f)) continue;
    graf_tile(&g.graf,x,y,*src,0);
  }
}

static void gameover_report_kv(int row,const char *k,const char *v,int vc) {
  int kc=0; if (k) while (k[kc]) kc++;
  if (!v) vc=0; else if (vc<0) { vc=0; while (v[vc]) vc++; }
  int y=row*8+4;
  int colonx=FBW>>1;
  graf_tile(&g.graf,colonx,y,':',0);
  int x,i;
  for (x=colonx-8,i=kc;i-->0;x-=8) {
    if ((k[i]<=0x20)||(k[i]>=0x7f)) continue;
    graf_tile(&g.graf,x,y,k[i],0);
  }
  for (x=colonx+16,i=0;i<vc;i++,x+=8) {
    if ((v[i]<=0x20)||(v[i]>=0x7f)) continue;
    graf_tile(&g.graf,x,y,v[i],0);
  }
}

static void gameover_report_integer(int row,const char *k,int v) {
  char msg[16];
  int msgc=0;
  #define APPEND(digit) { \
    msg[msgc++]='0'+(digit); \
  }
  if (v<0) v=0;
  if (v>=1000000) v=999999;
  if (v>=100000) APPEND(v/100000)
  if (v>= 10000) APPEND((v/10000)%10)
  if (v>=  1000) APPEND((v/ 1000)%10)
  if (v>=   100) APPEND((v/  100)%10)
  if (v>=    10) APPEND((v/   10)%10)
  APPEND(v%10)
  #undef APPEND
  gameover_report_kv(row,k,msg,msgc);
}

static void gameover_report_time(int row,const char *k,double sf) {
  char msg[16];
  int msgc=0;
  int ms=(int)(sf*1000.0);
  if (ms<0) ms=0;
  int sec=ms/1000; ms%=1000;
  int min=sec/60; sec%=60;
  if (min>99) {
    min=sec=99;
    ms=999;
  }
  #define APPEND(ch) { \
    msg[msgc++]=(ch); \
  }
  if (min>=10) APPEND('0'+min/10)
  APPEND('0'+min%10)
  APPEND(':')
  APPEND('0'+sec/10)
  APPEND('0'+sec%10)
  APPEND('.')
  APPEND('0'+ms/100)
  APPEND('0'+(ms/10)%10)
  APPEND('0'+ms%10)
  #undef APPEND
  gameover_report_kv(row,k,msg,msgc);
}

/* Generate the final report.
 */
 
static void gameover_compose_report() {

  gameover.texw=FBW;
  gameover.texh=8*7;

  if (!gameover.texid) gameover.texid=egg_texture_new();
  egg_texture_load_raw(gameover.texid,gameover.texw,gameover.texh,gameover.texw<<2,0,0);
  egg_texture_clear(gameover.texid);
  graf_flush(&g.graf);
  graf_set_output(&g.graf,gameover.texid);
  graf_set_image(&g.graf,RID_image_fonttiles);
  
  gameover_report_text(0,"All the villagers got their",-1);
  gameover_report_text(1,"things back, and the morning sun",-1);
  gameover_report_text(2,"has vanquished the horrible night.",-1);
  gameover_report_time(4,"Time",g.playtime);
  gameover_report_integer(5,"Death",g.deathc);
  gameover_report_kv(6,"Bell",g.rung_bell?"Rung":"---",-1);
  
  graf_set_output(&g.graf,1);
}

/* Begin.
 */
 
void gameover_begin() {
  memset(&gameover,0,sizeof(gameover));
  g.modal=MODAL_GAMEOVER;
  egg_play_song(1,0,1,0.0,0.0);
  gameover_compose_report();
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
  graf_set_input(&g.graf,gameover.texid);
  graf_decal(&g.graf,(FBW>>1)-(gameover.texw>>1),(FBH>>1)-(gameover.texh>>1),0,0,gameover.texw,gameover.texh);
}
