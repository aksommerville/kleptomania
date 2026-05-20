#include "kleptomania.h"

/* Render game, main entry point.
 */
 
void render_game() {
  
  /* Background from the map.
   * Our framebuffer is exactly the size of a map, and there's no scrolling.
   * Easy!
   */
  //TODO There should be scrolling, for neighbor transitions.
  {
    const uint8_t *src=g.map->v;
    int yi=NS_sys_maph;
    int dsty=NS_sys_tilesize>>1;
    int dstx0=NS_sys_tilesize>>1;
    graf_fill_rect(&g.graf,0,0,FBW,FBH,g.map->bgcolor);
    graf_set_image(&g.graf,g.map->imageid);
    for (;yi-->0;dsty+=NS_sys_tilesize) {
      int xi=NS_sys_mapw;
      int dstx=dstx0;
      for (;xi-->0;dstx+=NS_sys_tilesize,src++) {
        graf_tile(&g.graf,dstx,dsty,*src,0);
      }
    }
  }
  
  //TODO sprites
  //TODO overlay
}
