/* map.h
 */
 
#ifndef MAP_H
#define MAP_H

struct map {
  int rid;
  int lat,lng; // -128..127
  int imageid;
  const void *rov;
  const void *v; // Keeping separate in case we want mutable tiles. But I don't think we will.
  const void *cmd;
  int cmdc;
};

/* Caller must zero first, and set (rid) whenever, we won't touch it.
 */
int map_decode(struct map *map,const void *src,int srcc);

/* Rewrites (g.plane*) based on (g.map*).
 */
int maps_build_plane();

/* Get map from the global store.
 * Position is slightly more efficient but both are very efficient.
 */
struct map *map_by_id(int rid);
struct map *map_by_position(int lng,int lat);

#endif
