#include "game/kleptomania.h"

/* Delete.
 */

void sprite_del(struct sprite *sprite) {
  if (!sprite) return;
  if (sprite->type->del) sprite->type->del(sprite);
  free(sprite);
}

/* New.
 */
 
struct sprite *sprite_new(
  const struct sprite_type *type,
  double x,double y,
  int rid,uint32_t arg
) {
  
  /* Acquire resource if we can, and type if we need to.
   */
  const void *res=0;
  int resc=0;
  if (rid) {
    if ((resc=res_get(&res,EGG_TID_sprite,rid))<1) {
      fprintf(stderr,"sprite:%d not found\n",rid);
      return 0;
    }
    const struct sprite_type *type_per_res=sprite_type_for_res(res,resc);
    if (!type_per_res) return 0; // Guaranteed non-null but just to be sure.
    if (!type) type=type_per_res;
    else if (type!=type_per_res) {
      fprintf(stderr,"sprite:%d must be type '%s' but '%s' was provided programmatically.\n",rid,type_per_res->name,type->name);
      return 0;
    }
  }
  if (!type) {
    fprintf(stderr,"%s: Must provide (rid) or (type)\n",__func__);
    return 0;
  }
  
  /* Allocate and assign defaults.
   */
  struct sprite *sprite=calloc(1,type->objlen);
  if (!sprite) return 0;
  sprite->type=type;
  sprite->x=x;
  sprite->y=y;
  sprite->arg=arg;
  sprite->res=res;
  sprite->resc=resc;
  sprite->layer=100;
  
  /* Assign per generic commands.
   */
  if (sprite->resc) {
    struct cmdlist_reader reader;
    if (sprite_reader_init(&reader,res,resc)<0) {
      free(sprite);
      return 0;
    }
    struct cmdlist_entry cmd;
    while (cmdlist_reader_next(&cmd,&reader)>0) {
      switch (cmd.opcode) {
        case CMD_sprite_solid: sprite->solid=1; break;
        case CMD_sprite_image: sprite->imageid=(cmd.arg[0]<<8)|cmd.arg[1]; break;
        case CMD_sprite_tile: sprite->tileid=cmd.arg[0]; sprite->xform=cmd.arg[1]; break;
        case CMD_sprite_layer: sprite->layer=(cmd.arg[0]<<8)|cmd.arg[1]; break;
      }
    }
  }
  
  /* Call initializer if present.
   */
  if (type->init&&(type->init(sprite)<0)) {
    sprite_del(sprite);
    return 0;
  }
  
  return sprite;
}

/* Spawn sprite.
 */

struct sprite *sprite_spawn(
  const struct sprite_type *type,
  double x,double y,
  int rid,uint32_t arg
) {
  struct sprite *sprite=sprite_new(type,x,y,rid,arg);
  if (!sprite) return 0;
  
  if (g.spritec>=g.spritea) {
    int na=g.spritea+64;
    if (na>INT_MAX/sizeof(void*)) { sprite_del(sprite); return 0; }
    void *nv=realloc(g.spritev,sizeof(void*)*na);
    if (!nv) { sprite_del(sprite); return 0; }
    g.spritev=nv;
    g.spritea=na;
  }
  
  /* Find an insertion point.
   * Best effort only, since the list isn't fully sorted at all times.
   */
  int insp=g.spritec;
  struct sprite **otherp=g.spritev;
  int i=0;
  for (;i<g.spritec;i++,otherp++) {
    struct sprite *other=*otherp;
    if (other->layer>sprite->layer) { // If same layer, put us after. Stop only at greater.
      insp=i;
      break;
    }
  }
  
  memmove(g.spritev+insp+1,g.spritev+insp,sizeof(void*)*(g.spritec-insp));
  g.spritec++;
  g.spritev[insp]=sprite;
  
  return sprite;
}

/* Update sprites.
 */

void sprites_update(double elapsed) {
  struct sprite **spritep=g.spritev;
  int i=g.spritec;
  for (;i-->0;spritep++) {
    struct sprite *sprite=*spritep;
    if (sprite->defunct) continue;
    if (sprite->type->update) sprite->type->update(sprite,elapsed);
  }
  for (i=g.spritec,spritep=g.spritev+g.spritec-1;i-->0;spritep--) {
    struct sprite *sprite=*spritep;
    if (!sprite->defunct) continue;
    g.spritec--;
    memmove(spritep,spritep+1,sizeof(void*)*(g.spritec-i));
    sprite_del(sprite);
  }
}

/* Registry of types.
 */

const struct sprite_type *sprite_type_by_id(int sprtype) {
  switch (sprtype) {
    #define _(tag) case NS_sprtype_##tag: return &sprite_type_##tag;
    FOR_EACH_SPRTYPE
    #undef _
  }
  return 0;
}

const struct sprite_type *sprite_type_for_res(const void *src,int srcc) {
  struct cmdlist_reader reader;
  if (sprite_reader_init(&reader,src,srcc)<0) return &sprite_type_dummy;
  struct cmdlist_entry cmd;
  while (cmdlist_reader_next(&cmd,&reader)>0) {
    if (cmd.opcode==CMD_sprite_type) {
      const struct sprite_type *type=sprite_type_by_id((cmd.arg[0]<<8)|cmd.arg[1]);
      if (type) return type;
      fprintf(stderr,"sprtype %d not found. Will fall back to dummy if there isn't another 'type' command\n",(cmd.arg[0]<<8)|cmd.arg[1]);
    }
  }
  return &sprite_type_dummy;
}
