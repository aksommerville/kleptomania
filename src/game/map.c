#include "kleptomania.h"

/* Decode one map.
 */
 
int map_decode(struct map *map,const void *src,int srcc) {
  struct map_res mres;
  if (map_res_decode(&mres,src,srcc)<0) return -1;
  if ((mres.w!=NS_sys_mapw)||(mres.h!=NS_sys_maph)) {
    fprintf(stderr,"map:%d: Invalid size %dx%d, must be %dx%d\n",map->rid,mres.w,mres.h,NS_sys_mapw,NS_sys_maph);
    return -1;
  }
  map->rov=mres.v;
  map->v=map->rov;
  map->cmd=mres.cmd;
  map->cmdc=mres.cmdc;
  struct cmdlist_reader reader={.v=map->cmd,.c=map->cmdc};
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    switch (cmd.opcode) {
      case CMD_map_image: map->imageid=(cmd.arg[0]<<8)|cmd.arg[1]; break;
      case CMD_map_position: map->lat=(int8_t)cmd.arg[0]; map->lng=(int8_t)cmd.arg[1]; break;
    }
  }
  return 0;
}

/* Rebuild plane.
 */
 
int maps_build_plane() {
  fprintf(stderr,"%s mapc=%d\n",__func__,g.mapc);
  if (g.mapc<1) {
    fprintf(stderr,"No maps!\n");
    return -1;
  }
  
  // Determine bounds of plane.
  int xlo=g.mapv[0].lng;
  int ylo=g.mapv[0].lat;
  int xhi=xlo,yhi=ylo;
  struct map *map=g.mapv;
  int i=g.mapc;
  for (;i-->0;map++) {
    if (map->lng<xlo) xlo=map->lng;
    else if (map->lng>xhi) xhi=map->lng;
    if (map->lat<ylo) ylo=map->lat;
    else if (map->lat>yhi) yhi=map->lat;
  }
  g.planex=xlo;
  g.planey=ylo;
  g.planew=xhi-xlo+1;
  g.planeh=yhi-ylo+1;
  
  // Allocate (planev).
  void *nv=realloc(g.planev,sizeof(void*)*g.planew*g.planeh);
  if (!nv) { g.planew=g.planeh=0; return -1; }
  g.planev=nv;
  memset(g.planev,0,sizeof(void*)*g.planew*g.planeh);
  
  // Populate (planev).
  for (i=g.mapc,map=g.mapv;i-->0;map++) {
    int xn=map->lng-g.planex;
    int yn=map->lat-g.planey;
    int p=yn*g.planew+xn;
    if (g.planev[p]) {
      fprintf(stderr,"World position %d,%d claimed by both map:%d and map:%d\n",map->lng,map->lat,g.planev[p]->rid,map->rid);
      return -1;
    }
    g.planev[p]=map;
  }
  
  return 0;
}

/* Get map from store.
 */
 
struct map *map_by_id(int rid) {
  int lo=0,hi=g.mapc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    struct map *map=g.mapv+ck;
         if (rid<map->rid) hi=ck;
    else if (rid>map->rid) lo=ck+1;
    else return map;
  }
  return 0;
}

struct map *map_by_position(int lng,int lat) {
  if (lng<g.planex) return 0; lng-=g.planex;
  if (lat<g.planey) return 0; lat-=g.planey;
  if ((lng>=g.planew)||(lat>=g.planeh)) return 0;
  return g.planev[lat*g.planew+lng];
}
