/* sprite.h
 */
 
#ifndef SPRITE_H
#define SPRITE_H

struct sprite;
struct sprite_type;

/* Generic sprite instance and global registry.
 ************************************************************************/
 
struct sprite {
  const struct sprite_type *type; // REQUIRED
  double x,y; // meters, within map (ie within screen)
  int defunct;
  int layer; // Default 100.
  int imageid;
  uint8_t tileid,xform;
  uint32_t arg;
  const void *res; // The entire sprite resource we loaded from. OPTIONAL.
  int resc;
  int solid;
};

/* Should be internal use only.
 * These don't interact with the global list.
 */
void sprite_del(struct sprite *sprite);
struct sprite *sprite_new(
  const struct sprite_type *type,
  double x,double y,
  int rid,uint32_t arg
);

/* The normal way of creating a new sprite.
 * And the normal way to delete one is to set (defunct).
 * (type,rid) you only need to supply one.
 */
struct sprite *sprite_spawn(
  const struct sprite_type *type,
  double x,double y,
  int rid,uint32_t arg
);

/* Call update on those that need it, then drop defunct sprites.
 * There's no equivalent global "sprites_render"; camera takes care of it.
 */
void sprites_update(double elapsed);

/* Generic sprite type.
 ************************************************************************/
 
struct sprite_type {
  const char *name;
  int objlen;
  void (*del)(struct sprite *sprite);
  int (*init)(struct sprite *sprite);
  void (*update)(struct sprite *sprite,double elapsed);
  void (*render)(struct sprite *sprite,int dstx,int dsty); // Single tile if not implemented.
};

const struct sprite_type *sprite_type_by_id(int sprtype); // => null if unknown
const struct sprite_type *sprite_type_for_res(const void *src,int srcc); // => never null

#define _(tag) extern const struct sprite_type sprite_type_##tag;
FOR_EACH_SPRTYPE
#undef _

/* Specific types.
 ************************************************************************/
 
struct sprite *get_hero();
 
/* Physics.
 *************************************************************************/
 
//TODO

#endif
