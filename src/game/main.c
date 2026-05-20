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
  
  if (camera_init_scratches()<0) return -1;
  if (res_init()<0) return -1;
  
  if (game_reset()<0) return -1;

  return 0;
}

/* Update.
 */

void egg_client_update(double elapsed) {

  g.pvinput=g.input;
  g.input=egg_input_get_one(0);
  
  //TODO modals, global input triggers?
  
  sprites_update(elapsed);
  if (g.txclock>0.0) g.txclock-=elapsed;
  check_transitions();
}

/* Render.
 */

void egg_client_render() {
  graf_reset(&g.graf);
  //TODO modals
  render_game();
  graf_flush(&g.graf);
}
