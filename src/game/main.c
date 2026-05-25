#include "kleptomania.h"

struct g g={0};

void egg_client_quit(int status) {
}

void egg_client_notify(int k,int v) {
}

/* Init.
 */

int egg_client_init() {

  int fbw=0,fbh=0;
  egg_texture_get_size(&fbw,&fbh,1);
  if ((fbw!=FBW)||(fbh!=FBH)) {
    fprintf(stderr,"Framebuffer size mismatch! metadata=%dx%d header=%dx%d\n",fbw,fbh,FBW,FBH);
    return -1;
  }
  
  // Was thinking I wouldn't need rand, but actually we do, for cosmetic things only.
  srand_auto();
  
  if (camera_init_scratches()<0) return -1;
  if (res_init()<0) return -1;
  
  hello_begin();

  return 0;
}

/* Update.
 */

void egg_client_update(double elapsed) {

  g.pvinput=g.input;
  g.input=egg_input_get_one(0);
  
  int modal0=g.modal;
  switch (g.modal) {
    case MODAL_HELLO: hello_update(elapsed); break;
    case MODAL_VAMPIRE: vampire_update(elapsed); break;
    case MODAL_GAMEOVER: gameover_update(elapsed); break;
    default: game_update(elapsed); break;
  }
  if (modal0&&!g.modal) { // A modal just dismissed. Update the game, to avoid a possible flicker frame.
    game_update(elapsed);
  }
}

/* Render.
 */

void egg_client_render() {
  graf_reset(&g.graf);
  switch (g.modal) {
    case MODAL_HELLO: hello_render(); break;
    case MODAL_VAMPIRE: vampire_render(); break;
    case MODAL_GAMEOVER: gameover_render(); break;
    default: render_game();
  }
  graf_flush(&g.graf);
}

/* Audio.
 */
 
void kl_sound(int rid) {
  //TODO Blackout?
  egg_play_sound(rid,1.0,0.0);
}
