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
extern struct room_data *room_db[WORLD_SIZE];
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

extern long     TempDis;
extern struct weather_data weather_info;
extern const char *languagelist[];
extern char    *system_flag_types[];
extern struct zone_data *zone_table;
extern int      top_of_zone_table;

extern struct index_data *mob_index;
extern int      top_of_p_table;
extern int      top_of_mobt;
extern int      top_of_objt;
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern struct player_index_element *player_table;
extern char    *room_bits[];
extern struct str_app_type str_app[];
extern char    *motd;
extern char    *wmotd;
extern const struct class_def classes[MAX_CLASS];


#ifdef SITELOCK
extern int      numberhosts;
extern char     hostlist[MAX_BAN_HOSTS][256];
#endif


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
