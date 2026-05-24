/* shared_symbols.h
 * This file is consumed by eggdev and editor, in addition to compiling in with the game.
 */

#ifndef SHARED_SYMBOLS_H
#define SHARED_SYMBOLS_H

#define EGGDEV_importUtil "res,font,text,graf,stdlib" /* Comma-delimited list of Egg 'util' units to include in the build. */
#define EGGDEV_ignoreData "" /* Comma-delimited glob patterns for editor and builder to ignore under src/data/ */

#define NS_sys_tilesize 16
#define NS_sys_mapw 20
#define NS_sys_maph 11
#define NS_sys_bgcolor 0x85d0f2

#define CMD_map_villagers 0x01 /* --- */
#define CMD_map_image     0x20 /* u16:imageid */
#define CMD_map_villspawn 0x21 /* u16:position ; Marks available starting position for a villager. */
#define CMD_map_position  0x40 /* s8:long, s8:lat, u16:unused */
#define CMD_map_bgcolor   0x41 /* u24:rgb u8:unused */
#define CMD_map_sprite    0x61 /* u16:position, u16:spriteid, u32:arg */

#define CMD_sprite_image     0x20 /* u16:imageid */
#define CMD_sprite_tile      0x21 /* u8:tileid, u8:xform */
#define CMD_sprite_type      0x22 /* u16:sprtype */
#define CMD_sprite_layer     0x23 /* u16:layer ; default 100 */
#define CMD_sprite_treasure  0x24 /* u16:treasure ; for villager */
#define CMD_sprite_solid     0x40 /* s8:l s8:r s8:t s8:b ; in pixels */

#define NS_tilesheet_physics 1
#define NS_tilesheet_family 0
#define NS_tilesheet_neighbors 0
#define NS_tilesheet_weight 0

#define NS_physics_vacant 0
#define NS_physics_solid 1
#define NS_physics_oneway 2
#define NS_physics_hazard 3

#define NS_treasure_gem 1
#define NS_treasure_book 2
#define NS_treasure_crown 3
#define NS_treasure_avocado 4
#define NS_treasure_violin 5
#define NS_treasure_sock 6
#define NS_treasure_key 7
#define NS_treasure_watermelon 8

// Editor uses the comment after a 'sprtype' symbol as a prompt in the new-sprite modal.
// Should match everything after 'spriteid' in the CMD_map_sprite args.
#define NS_sprtype_dummy 0 /* (u32)0 */
#define NS_sprtype_hero 1 /* (u32)0 */
#define NS_sprtype_brick 2 /* (u32)0 */
#define NS_sprtype_dust 3 /* (u32)0 */
#define NS_sprtype_treasure 4 /* (u32)0 */
#define NS_sprtype_soulballs 5 /* (u32)0 */
#define NS_sprtype_villager 6 /* (u32)0 */
#define NS_sprtype_lock 7 /* (u32)0 */
#define NS_sprtype_platform 8 /* (u8:dir)0x00 (u24)0 */
#define NS_sprtype_drawbridge 9 /* (u32)0 */
#define FOR_EACH_SPRTYPE \
  _(dummy) \
  _(hero) \
  _(brick) \
  _(dust) \
  _(treasure) \
  _(soulballs) \
  _(villager) \
  _(lock) \
  _(platform) \
  _(drawbridge)

#endif
