/* sprite_brick.c
 * Basically dummy, but with gravity enabled.
 */

#include "game/kleptomania.h"

static void _brick_update(struct sprite *sprite,double elapsed) {
  sprite_update_gravity(sprite,elapsed);
}

const struct sprite_type sprite_type_brick={
  .name="brick",
  .objlen=sizeof(struct sprite),
  .update=_brick_update,
};
