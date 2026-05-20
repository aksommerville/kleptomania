#include "kleptomania.h"

/* Reset entire state, start a new game.
 */
 
int game_reset() {
  g.map=0; // If the previous session ended at a cardinal neighbor of RID_map_start, don't pan. Always fade.
  g.safex=g.safey=-1; // No default position; RID_map_start must contain an explicit spawn point.
  return game_load_map(RID_map_start);
}

/* Load a map, possibly starting a transition.
 */
 
int game_load_map(int rid) {

  /* Find the new map.
   */
  struct map *map=map_by_id(rid);
  if (!map) {
    fprintf(stderr,"map:%d not found\n",rid);
    egg_terminate(1); // In case our caller isn't situated for termination.
    return -1;
  }
  
  /* Determine transition.
   * Either cardinal neighbor or "other".
   */
  g.txclock=TRANSITION_TIME;
  g.transition=TRANSITION_FADE;
  if (g.map) {
    int dx=map->lng-g.map->lng;
    int dy=map->lat-g.map->lat;
         if ((dx==1)&&(dy==0)) g.transition=TRANSITION_PAN_RIGHT;
    else if ((dx==-1)&&(dy==0)) g.transition=TRANSITION_PAN_LEFT;
    else if ((dx==0)&&(dy==1)) g.transition=TRANSITION_PAN_DOWN;
    else if ((dx==0)&&(dy==-1)) g.transition=TRANSITION_PAN_UP;
  }
  camera_draw_txbits(g.transition==TRANSITION_FADE);
  
  /* Drop sprites.
   * The hero gets treated a little special, depending what kind of transition we're doing.
   */
  struct sprite *hero=0;
  int i=g.spritec;
  while (i-->0) {
    struct sprite *sprite=g.spritev[i];
    if (!hero&&(sprite->type==&sprite_type_hero)) {
      switch (g.transition) {
        // For pans, capture the sprite and shift its position by one framebuffer.
        case TRANSITION_PAN_LEFT: hero=sprite; hero->x+=NS_sys_mapw; break;
        case TRANSITION_PAN_RIGHT: hero=sprite; hero->x-=NS_sys_mapw; break;
        case TRANSITION_PAN_UP: hero=sprite; hero->y+=NS_sys_maph; break;
        case TRANSITION_PAN_DOWN: hero=sprite; hero->y-=NS_sys_maph; break;
        // For FADE or anything else, kill the hero like any other sprite.
        case TRANSITION_FADE: default: {
            sprite_del(sprite);
            g.spritec--;
            memmove(g.spritev+i,g.spritev+i+1,sizeof(void*)*(g.spritec-i));
          } break;
      }
    } else {
      sprite_del(sprite);
      g.spritec--;
      memmove(g.spritev+i,g.spritev+i+1,sizeof(void*)*(g.spritec-i));
    }
  }
  
  /* Commit other globals.
   * bgbits_dirty might not strictly need set, if we're reloading the current map.
   * But let's not stress over it; in most games we don't even cache the background, it's not expensive to render.
   */
  g.map=map;
  g.bgbits_dirty=1;
  //TODO If we make volatile cells, copy (rov) over (v) now.
  
  /* Run map commands, in particular spawn sprites.
   * If there's a hero spawn point and we don't have (hero) yet, make it and record it.
   */
  {
    struct cmdlist_reader reader={.v=map->cmd,.c=map->cmdc};
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_map_sprite: {
            double x=cmd.arg[0]+0.5;
            double y=cmd.arg[1]+0.5;
            int rid=(cmd.arg[2]<<8)|cmd.arg[3];
            uint32_t arg=(cmd.arg[4]<<24)|(cmd.arg[5]<<16)|(cmd.arg[6]<<8)|cmd.arg[7];
            if (!hero||(rid!=RID_sprite_hero)) {
              struct sprite *sprite=sprite_spawn(0,x,y,rid,arg);
              if (!sprite) {
                fprintf(stderr,"Failed to spawn sprite:%d on map:%d\n",rid,g.map->rid);
                return -1;
              }
              if (rid==RID_sprite_hero) hero=sprite;
            }
          } break;
        //TODO song? other things?
      }
    }
  }
  
  /* If we still don't have a hero, (safex,safey) must be valid, spawn there.
   * That will happen when you die.
   * If they're invalid, report an error.
   */
  if (!hero) {
    if ((g.safex<0)||(g.safey<0)||(g.safex>=NS_sys_mapw)||(g.safey>=NS_sys_maph)) {
      fprintf(stderr,"Explicit hero spawn point required on map:%d\n",g.map->rid);
      return -1;
    }
    if (!(hero=sprite_spawn(&sprite_type_hero,g.safex+0.5,g.safey+0.5,RID_sprite_hero,0))) {
      fprintf(stderr,"Failed to respawn hero on map:%d\n",g.map->rid);
      return -1;
    }
  }
  
  /* Update (safex,safey) with the hero's current position.
   * Shouldn't be necessary, but clamp to the map to be on the safe side.
   */
  g.safex=(int)hero->x; if (g.safex<0) g.safex=0; else if (g.safex>=NS_sys_mapw) g.safex=NS_sys_mapw-1;
  g.safey=(int)hero->y; if (g.safey<0) g.safey=0; else if (g.safey>=NS_sys_maph) g.safey=NS_sys_maph-1;
  
  return 0;
}

/* Check for transitions.
 */
 
void check_transitions() {
  struct sprite *hero=get_hero();
  if (!hero) return;
  int dx=0,dy=0;
  if (hero->x<0.0) dx=-1;
  else if (hero->y<0.0) dy=-1;
  else if (hero->x>=NS_sys_mapw) dx=1;
  else if (hero->y>=NS_sys_maph) dy=1;
  if (!dx&&!dy) return;
  struct map *nmap=map_by_position(g.map->lng+dx,g.map->lat+dy);
  if (!nmap) return;
  game_load_map(nmap->rid);
}
