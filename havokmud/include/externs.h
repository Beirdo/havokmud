
/**
 * @file
 * @brief OLD!  List of all external variables.
 */

#ifndef _externs_h
#define _externs_h

#include "structs.h"

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

extern char    *AttackType[];

extern struct str_app_type str_app[];

extern struct class_def *classes;
int             classCount;

extern struct skillset weaponskills[];
extern int             weaponskillCount;
extern struct skillset loreskills[];
extern int             loreskillCount;
extern struct skillset warmonkskills[];
extern int             warmonkskillCount;
extern struct skillset archerskills[];
extern int             archerskillCount;
extern struct skillset allninjaskills[];
extern int             allninjaskillCount;
extern struct skillset warninjaskills[];
extern int             warninjaskillCount;
extern struct skillset thfninjaskills[];
extern int             thfninjaskillCount;
extern struct skillset styleskillset[];
extern int             styleskillCount;


extern int      top_of_zone_table;
extern int      top_of_mobt;
extern int      top_of_objt;

extern char     wizlist[];
extern char     iwizlist[];
extern char    *where[];
extern char    *color_liquid[];
extern char    *fullness[];
extern char    *system_flag_types[];
extern time_t   Uptime;
extern long     room_count;
extern long     mob_count;
extern int      spell_status[];
extern struct radix_list radix_head[];
extern const char *weekdays[];
extern const char *month_name[];
extern const char *fight_styles[];
extern struct clan    *clan_list;
extern int             clanCount;

extern struct dex_skill_type dex_app_skill[];
extern int      MaxArenaLevel,
                MinArenaLevel;
extern int spell_info_count;

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
extern char    *ItemDamType[];
extern int      ItemSaveThrows[22][5];
extern struct dex_app_type dex_app[];
extern char    *room_bits[];

extern struct player_index_element *player_table;
extern int      top_of_p_table;
extern long     SystemFlags;
extern long     TempDis;
extern const struct pc_race_choice race_choice[];
extern int       race_choice_count;

extern struct wis_app_type wis_app[];
extern struct con_app_type con_app[];

extern int      gSeason;

extern const struct map_coord map_coords[];
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern int      top_of_world;
extern struct int_app_type int_app[];

extern struct zone_data *zone_table;

extern char    *spells[];
/*
 * for objects
 */
extern char    *item_types[];
extern char    *wear_bits[];
extern char    *extra_bits[];
extern char    *drinks[];
/*
 * for rooms
 */
extern char    *exit_bits[];
/*
 * for chars
 */
extern char    *equipment_types[];
extern char    *affected_bits[];
extern char    *affected_bits2[];
extern char    *immunity_names[];
extern char    *special_user_flags[];
extern char    *apply_types[];
extern char    *pc_class_types[];
extern char    *npc_class_types[];
extern char    *action_bits[];
extern char    *procedure_bits[];
extern char    *player_bits[];
extern char    *position_types[];
extern char    *connected_types[];

extern struct char_data *mem_list;
extern funcp    bweapons[];
extern struct chr_app_type chr_apply[];
extern struct int_app_type int_sf_modifier[];
extern struct syllable syls[];

extern struct skill_def *skills;
extern int               skillCount;

extern struct dir_data *direction;
extern int      directionCount;

extern int      drink_aff[][3];

extern int      map[7][7];


extern char    *att_kick_hit_room[];
extern char    *att_kick_hit_victim[];
extern char    *att_kick_hit_ch[];
extern char    *att_kick_miss_room[];
extern char    *att_kick_miss_victim[];
extern char    *att_kick_miss_ch[];
extern char    *att_kick_kill_room[];
extern char    *att_kick_kill_victim[];
extern char    *att_kick_kill_ch[];

extern struct breather breath_monsters[];


extern char    *drinknames[];

extern int      qp_patience;

extern int      auct_loop;
extern int      minbid;
extern int      intbid;
extern struct char_data *auctioneer;
extern struct char_data *bidder;

extern int      gLightLevel;
extern unsigned char moontype;

extern int      mob_tick_count;
extern long     total_obc;
extern struct reset_q_type reset_q;

extern struct lang_def    *languages;
extern int                 languageCount;

extern int      Silence;

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
