#include "kleptomania.h"

/* Receive map, pre-link.
 */
 
static int res_add_map(const struct rom_entry *res) {
  if (g.mapc>=g.mapa) {
    int na=g.mapa+32;
    if (na>INT_MAX/sizeof(struct map)) return -1;
    void *nv=realloc(g.mapv,sizeof(struct map)*na);
    if (!nv) return -1;
    g.mapv=nv;
    g.mapa=na;
  }
  struct map *map=g.mapv+g.mapc++;
  memset(map,0,sizeof(struct map));
  map->rid=res->rid;
  if (map_decode(map,res->v,res->c)<0) return -1;
  return 0;
}

/* Receive tilesheet, pre-link.
 */
 
static int res_add_tilesheet(const struct rom_entry *res) {
  fprintf(stderr,"%s %d c=%d\n",__func__,res->rid,res->c);//TODO
  return 0;
}

/* Init.
 */
 
int res_init() {

  /* Acquire the ROM and tell text unit about it.
   */
  g.romc=egg_rom_get(0,0);
  if (!(g.rom=malloc(g.romc))) return -1;
  egg_rom_get(g.rom,g.romc);
  text_set_rom(g.rom,g.romc);
  
  /* Examine each resource and do any pre-link per-resource processing.
   */
  struct rom_reader reader;
  if (rom_reader_init(&reader,g.rom,g.romc)<0) return -1;
  struct rom_entry res;
  while (rom_reader_next(&res,&reader)>0) {
    int include=0;
    switch (res.tid) {
      // Res types with special handling:
      case EGG_TID_map: if (res_add_map(&res)<0) return -1; break;
      case EGG_TID_tilesheet: if (res_add_tilesheet(&res)<0) return -1; break;
      // Ones we index verbatim:
      case EGG_TID_sprite:
        include=1;
        break;
      // Ones we know we can ignore:
      case EGG_TID_code:
      case EGG_TID_metadata:
      case EGG_TID_image:
      case EGG_TID_song:
      case EGG_TID_sound:
      case EGG_TID_strings:
        break;
      // Anything else is an error.
      default: {
          fprintf(stderr,"Unexpected resource type %d (rid %d, size %d)\n",res.tid,res.rid,res.c);
          return -1;
        }
    }
    if (include) {
      if (g.resc>=g.resa) {
        int na=g.resa+64;
        if (na>INT_MAX/sizeof(struct rom_entry)) return -1;
        void *nv=realloc(g.resv,sizeof(struct rom_entry)*na);
        if (!nv) return -1;
        g.resv=nv;
        g.resa=na;
      }
      // Resources are stored sorted, they can't be otherwise. Convenient!
      g.resv[g.resc++]=res;
    }
  }
  
  /* Linkage. Any further processing which requires the entire set loaded.
   */
  if (maps_build_plane()<0) return -1;
  
  return 0;
}

/* Get from globals.
 */
 
int res_search(int tid,int rid) {
  int lo=0,hi=g.resc;
  while (lo<hi) {
    int ck=(lo+hi)>>1;
    const struct rom_entry *res=g.resv+ck;
         if (tid<res->tid) hi=ck;
    else if (tid>res->tid) lo=ck+1;
    else if (rid<res->rid) hi=ck;
    else if (rid>res->rid) lo=ck+1;
    else return ck;
  }
  return -lo-1;
}

int res_get(void *dstpp,int tid,int rid) {
  int p=res_search(tid,rid);
  if (p<0) return 0;
  const struct rom_entry *res=g.resv+p;
  if (dstpp) *(const void**)dstpp=res->v;
  return res->c;
}
