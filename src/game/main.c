#include "kleptomania.h"

struct g g={0};

void egg_client_quit(int status) {
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
  
  if (res_init()<0) return -1;

  //TODO Need a structured "load_scene" kind of function.
  if (!(g.map=map_by_id(RID_map_start))) return -1;

  return 0;
}

void egg_client_notify(int k,int v) {
}

/* Update.
 */

void egg_client_update(double elapsed) {
  //TODO
}

/* Render.
 */

void egg_client_render() {
  graf_reset(&g.graf);
  //TODO modals
  render_game();
  graf_flush(&g.graf);
}
