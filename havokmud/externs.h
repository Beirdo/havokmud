#ifndef _externs_h
#define _externs_h

#include "structs.h"

extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern long     SystemFlags;
extern int      ArenaNoGroup,
                ArenaNoAssist,
                ArenaNoDispel,
                ArenaNoMagic,
                ArenaNoWSpells,
                ArenaNoSlay,
                ArenaNoFlee,
                ArenaNoHaste,
                ArenaNoPets,
                ArenaNoTravel,
                ArenaNoBash;
extern const struct race_type race_list[];
extern struct descriptor_data *descriptor_list;
extern int      top_of_world;
extern struct index_data *obj_index;

#ifdef HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif

extern const struct skillset weaponskills[];
extern char    *spells[];
extern char    *RaceName[];
extern char    *AttackType[];
extern char    *item_types[];
extern char    *extra_bits[];
extern char    *apply_types[];
extern char    *affected_bits[];
extern char    *affected_bits2[];
extern char    *immunity_names[];
extern char    *wear_bits[];
extern struct char_data *auctioneer;

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
extern char    *dirs[];
extern struct dex_app_type dex_app[];
extern char    *spells[];

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
