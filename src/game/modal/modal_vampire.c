#include "game/kleptomania.h"

static const struct slide {
  int lframe; // Dot: (0,1,2)=(silent,normal,angry)
  int rframe; // Vampire: (0,1,2)=(silent,sock,normal)
  const char *text;
} slidev[]={
  {1,0,"Vampire! Give back that sock!"},
  {0,1,"Never!"},
  {2,0,"You're lucky this is a non-violent game, mister."},
  {0,2,"Tell you what. I'll trade it for a watermelon."},
};
static const int slidec=sizeof(slidev)/sizeof(slidev[0]);

static struct {
  int slidep;
  int textp;
  double textclock;
} vampire={0};

/* Begin.
 */
 
void vampire_begin() {
  memset(&vampire,0,sizeof(vampire));
  g.modal=MODAL_VAMPIRE;
}

/* Dismiss.
 */
 
static void vampire_dismiss() {
  g.modal=0;
  g.pvinput=g.input;
}

/* Activate.
 */
 
static void vampire_activate() {
  kl_sound(RID_sound_uiactivate);
  if ((vampire.slidep<0)||(vampire.slidep>=slidec)) { vampire_dismiss(); return; }
  const struct slide *slide=slidev+vampire.slidep;
  if (slide->text[vampire.textp]) { // Still paying out text. Advance to the end.
    while (slide->text[vampire.textp]) vampire.textp++;
  } else { // Advance to next slide or dismiss.
    vampire.textp=0;
    vampire.textclock=0.0;
    if (++(vampire.slidep)>=slidec) {
      vampire_dismiss();
    }
  }
}

/* Update.
 */
 
void vampire_update(double elapsed) {
  if ((g.input&EGG_BTN_SOUTH)&&!(g.pvinput&EGG_BTN_SOUTH)) {
    vampire_activate();
  } else {
    if ((vampire.textclock-=elapsed)<=0.0) {
      vampire.textclock+=0.100;
      if ((vampire.slidep>=0)&&(vampire.slidep<slidec)) {
        const struct slide *slide=slidev+vampire.slidep;
        if (slide->text[vampire.textp]) {
          kl_sound(RID_sound_typewriter);
          vampire.textp++;
          while (slide->text[vampire.textp]&&((unsigned char)slide->text[vampire.textp]<=0x20)) vampire.textp++; // Skip spaces.
        } else {
          vampire.textclock=999.999;
        }
      } else {
        vampire.textclock=999.999;
      }
    }
  }
}

/* Render.
 */
 
void vampire_render() {
  graf_fill_rect(&g.graf,0,0,FBW,FBH,0x000000ff);
  if ((vampire.slidep<0)||(vampire.slidep>=slidec)) return;
  const struct slide *slide=slidev+vampire.slidep;
  
  const int portraitw=64;
  const int portraith=64;
  int lx=FBW/3-(portraitw>>1);
  int rx=(FBW*2)/3-(portraitw>>1);
  int py=FBH/3-(portraith>>1);
  graf_set_image(&g.graf,RID_image_dialogue);
  graf_decal(&g.graf,lx,py,slide->lframe*portraitw,0,portraitw,portraith);
  graf_decal(&g.graf,rx,py,slide->rframe*portraitw,portraith,portraitw,portraith);
  
  graf_set_image(&g.graf,RID_image_fonttiles);
  int x0=20;
  int xz=FBW-x0;
  int y=(FBH*2)/3;
  int x=x0;
  const char *src=slide->text;
  int srcc=vampire.textp;
  int srcp=0;
  while (srcp<srcc) {
    if ((unsigned char)src[srcp]<=0x20) { srcp++; continue; }
    const char *word=src+srcp;
    int wordc=1;
    // Read to the end of the word, whether it's all slated to print or not. Important, so we break lines in the right place while typing.
    while ((unsigned char)word[wordc]>0x20) wordc++;
    // Need a line break first?
    if ((x>x0)&&(x+wordc*8>xz)) {
      y+=8;
      x=x0;
    }
    // Now reduce (wordc) if it crosses (srcc).
    if (srcp+wordc>srcc) wordc=srcc-srcp;
    // And print it.
    int i=wordc;
    for (;i-->0;word++,x+=8) {
      graf_tile(&g.graf,x,y,*word,0);
    }
    // Plus a space.
    x+=8;
    srcp+=wordc;
  }
}
