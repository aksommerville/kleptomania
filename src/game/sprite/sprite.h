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
  double hbl,hbr,hbt,hbb; // Hitbox relative to (x,y). (l,t) are normally negative. Maintain horizontal symmetry if you flop!
  int seated;
  double gravity;
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
  void (*landed)(struct sprite *sprite,double velocity); // Optional callback if you're requesting gravity.
  void (*falling)(struct sprite *sprite); // ''
};

const struct sprite_type *sprite_type_by_id(int sprtype); // => null if unknown
const struct sprite_type *sprite_type_for_res(const void *src,int srcc); // => never null

#define _(tag) extern const struct sprite_type sprite_type_##tag;
FOR_EACH_SPRTYPE
#undef _

/* Specific types.
 ************************************************************************/
 
struct sprite *get_hero();
int sprite_hero_carry(struct sprite *sprite,int treasure); // >0 if accepted.
int sprite_hero_carrying(const struct sprite *sprite);
 
/* Physics.
 *************************************************************************/

/* If (sprite) is solid, correct collisions immediately.
 * No other sprite will move.
 * Sprite will only move in the direction of this vector, never backward.
 * Returns nonzero if we move at all, or zero if fully blocked.
 * Result undefined if both deltas are zero.
 */
int sprite_move(struct sprite *sprite,double dx,double dy);

/* We offer complimentary gravity service to all paying customers!
 * It works for non-solid sprites too, but you must define the hitbox, and we'll temporarily enable solid.
 * (otherwise, gravity without solid doesn't make sense, it would fall forever).
 * We own (gravity,seated). You can read but please don't write them. Initialize both to zero.
 */
void sprite_update_gravity(struct sprite *sprite,double elapsed);

/* Spam this while jumping, or gravity suspended for whatever reason.
 * This may trigger (falling).
 */
void sprite_force_null_gravity(struct sprite *sprite);

#endif
