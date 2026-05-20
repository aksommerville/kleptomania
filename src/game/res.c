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
 
static uint8_t *physicsv_require(int rid) {
  if ((rid<1)||(rid>99)) return 0;
  int p=rid-1;
  if (p>=g.physicsc) {
    int na=p+1;
    if (na>g.physicsa) {
      na=(na+16)&~15;
      if (na>INT_MAX/256) return 0;
      void *nv=realloc(g.physicsv,na*256);
      if (!nv) return 0;
      g.physicsv=nv;
      g.physicsa=na;
    }
    memset(g.physicsv+g.physicsc*256,0,(p-g.physicsc+1)*256);
    g.physicsc=p+1;
  }
  return g.physicsv+p*256;
}
 
static int res_add_tilesheet(const struct rom_entry *res) {
  uint8_t *dst=physicsv_require(res->rid);
  if (!dst) return -1;
  struct tilesheet_reader reader;
  if (tilesheet_reader_init(&reader,res->v,res->c)<0) return -1;
  struct tilesheet_entry entry;
  while (tilesheet_reader_next(&entry,&reader)>0) {
    if (entry.tableid==NS_tilesheet_physics) {
      memcpy(dst+entry.tileid,entry.v,entry.c);
    }
  }
  return 0;
}

/* With all maps and tilesheets loaded, set (physics) in each map.
 */
 
static const uint8_t physics_default[256]={0};

static int res_acquire_physics() {
  struct map *map=g.mapv;
  int i=g.mapc;
  for (;i-->0;map++) {
    int p=map->imageid-1;
    if ((p<0)||(p>=g.physicsc)) {
      map->physics=physics_default;
    } else {
      map->physics=g.physicsv+p*256;
    }
  }
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
  if (res_acquire_physics()<0) return -1;
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
