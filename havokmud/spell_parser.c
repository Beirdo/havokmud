/*
 * DaleMUD,
 */

#include <stdio.h>
#include <assert.h>

#include "protos.h"

#define MANA_MU 1
#define MANA_CL 1

/*
 * 100 is the MAX_MANA for a character 
 */
#define USE_MANA(ch, sn) \
  MAX((int)spell_info[sn].min_usesmana,100 / \
  MAX(2,(2+GET_LEVEL(ch, BestMagicClass(ch))-SPELL_LEVEL(ch,sn))))

/*
 * Global data 
 */

extern struct room_data *world;
extern struct char_data *character_list;
extern char    *spell_wear_off_msg[];
extern char    *spell_wear_off_soon_msg[];
extern char    *spell_wear_off_room_msg[];
extern char    *spell_wear_off_soon_room_msg[];
extern struct obj_data *object_list;
extern struct index_data *obj_index;
extern struct char_data *mem_list;
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
extern struct time_info_data time_info;
extern funcp    bweapons[];
extern struct chr_app_type chr_apply[];
extern struct int_app_type int_sf_modifier[];

/*
 * internal procedures 
 */
void            SpellWearOffSoon(int s, struct char_data *ch);
void            check_drowning(struct char_data *ch);
int             check_falling(struct char_data *ch);
void            check_decharm(struct char_data *ch);

/*
 * Extern procedures 
 */
void            update_char_objects(struct char_data *ch);
void            do_save(struct char_data *ch, char *arg, int cmd);

void            NailThisSucker(struct char_data *ch);
void            do_look(struct char_data *ch, char *arg, int cmd);
void            DamageAllStuff(struct char_data *ch, int dam_type);

void            cast_cold_light(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_disease(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);

void            cast_invis_to_undead(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);

void            cast_suit_of_bone(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_spectral_shield(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);

void            cast_clinging_darkness(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);

void            cast_dominate_undead(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);

void            cast_unsummon(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_siphon_strength(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);

void            cast_gather_shadows(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);

void            cast_mend_bones(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_trace_corpse(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_endure_cold(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);

void            cast_life_draw(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);

/*
 * Extern procedures 
 */

/*
 * psi stuff 
 */
void            mind_use_burn(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *victim,
                              struct obj_data *tar_obj);
void            mind_use_teleport(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *victim,
                                  struct obj_data *tar_obj);
void            mind_use_probability_travel(byte level,
                                            struct char_data *ch,
                                            char *arg, int type,
                                            struct char_data *victim,
                                            struct obj_data *tar_obj);
void            mind_use_danger_sense(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *victim,
                                      struct obj_data *tar_obj);
void            mind_use_clairvoyance(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *victim,
                                      struct obj_data *tar_obj);
void            mind_use_disintegrate(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *victim,
                                      struct obj_data *tar_obj);
void            mind_use_telekinesis(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *victim,
                                     struct obj_data *tar_obj);
void            mind_use_levitation(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *victim,
                                    struct obj_data *tar_obj);
void            mind_use_cell_adjustment(byte level, struct char_data *ch,
                                         char *arg, int type,
                                         struct char_data *victim,
                                         struct obj_data *tar_obj);
void            mind_use_chameleon(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *victim,
                                   struct obj_data *tar_obj);
void            mind_use_psi_strength(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *victim,
                                      struct obj_data *tar_obj);
void            mind_use_mind_over_body(byte level, struct char_data *ch,
                                        char *arg, int type,
                                        struct char_data *victim,
                                        struct obj_data *tar_obj);
void            mind_use_domination(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *victim,
                                    struct obj_data *tar_obj);
void            mind_use_mind_wipe(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *victim,
                                   struct obj_data *tar_obj);
void            mind_use_psychic_crush(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *victim,
                                       struct obj_data *tar_obj);
void            mind_use_tower_iron_will(byte level, struct char_data *ch,
                                         char *arg, int type,
                                         struct char_data *victim,
                                         struct obj_data *tar_obj);
void            mind_use_mindblank(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *victim,
                                   struct obj_data *tar_obj);
void            mind_use_psychic_impersonation(byte level,
                                               struct char_data *ch,
                                               char *arg, int type,
                                               struct char_data *victim,
                                               struct obj_data *tar_obj);
void            mind_use_ultra_blast(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *victim,
                                     struct obj_data *tar_obj);
void            mind_use_intensify(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *victim,
                                   struct obj_data *tar_obj);
void            mind_use_mind_tap(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *victim,
                                  struct obj_data *tar_obj);
void            mind_use_kinolock(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            mind_use_sense_object(byte level, struct char_data *ch,
                                      char *arg, int si,
                                      struct char_data *tar_ch,
                                      struct obj_data *tar_obj);
/*
 * end psi mind stuff 
 */

void            cast_animate_dead(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_conjure_elemental(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);
void            cast_acid_blast(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_armor(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_teleport(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_bless(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_blindness(byte level, struct char_data *ch, char *arg,
                               int si, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_burning_hands(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_incendiary_cloud(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *victim,
                                      struct obj_data *tar_obj);
void            cast_prismatic_spray(byte level, struct char_data *ch,
                                     struct char_data *victim,
                                     struct obj_data *obj);
void            cast_call_lightning(byte level, struct char_data *ch,
                                    char *arg, int si,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_charm_person(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_charm_monster(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_cacaodemon(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_chill_touch(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_shocking_grasp(byte level, struct char_data *ch,
                                    char *arg, int si,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_clone(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_colour_spray(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_control_weather(byte level, struct char_data *ch,
                                     char *arg, int si,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_create_food(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_create_water(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_cure_blind(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_cure_critic(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_cause_critic(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_cure_light(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_cause_light(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_curse(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_cont_light(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_calm(byte level, struct char_data *ch, char *arg,
                          int si, struct char_data *tar_ch,
                          struct obj_data *tar_obj);
void            cast_detect_evil(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_detect_good(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_detect_invisibility(byte level, struct char_data *ch,
                                         char *arg, int si,
                                         struct char_data *tar_ch,
                                         struct obj_data *tar_obj);
void            cast_detect_magic(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_detect_poison(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_dispel_evil(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_dispel_good(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_dispel_magic(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_earthquake(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_enchant_weapon(byte level, struct char_data *ch,
                                    char *arg, int si,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_enchant_armor(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_energy_drain(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_fear(byte level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj);
void            cast_fireball(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_flamestrike(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_flying(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);
void            cast_flying(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);

void            cast_harm(byte level, struct char_data *ch, char *arg,
                          int si, struct char_data *tar_ch,
                          struct obj_data *tar_obj);
void            cast_heal(byte level, struct char_data *ch, char *arg,
                          int si, struct char_data *tar_ch,
                          struct obj_data *tar_obj);
void            cast_infravision(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_invisibility(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_cone_of_cold(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_ice_storm(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_knock(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_know_alignment(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_true_seeing(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_minor_creation(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_faerie_fire(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_faerie_fog(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_heroes_feast(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_fly_group(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_web(byte level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj);
void            cast_minor_track(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_major_track(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_mana(byte level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj);

void            cast_lightning_bolt(byte level, struct char_data *ch,
                                    char *arg, int si,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_light(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_locate_object(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_magic_missile(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);

void            cast_mon_sum1(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum2(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum3(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum4(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum5(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum6(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_mon_sum7(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_meteor_swarm(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_poly_self(byte level, struct char_data *ch, char *arg,
                               int si, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_change_form(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);

void            cast_poison(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);

void            cast_protection_from_evil(byte level, struct char_data *ch,
                                          char *arg, int si,
                                          struct char_data *tar_ch,
                                          struct obj_data *tar_obj);
void            cast_protection_from_evil_group(byte level,
                                                struct char_data *ch,
                                                char *arg, int type,
                                                struct char_data *tar_ch,
                                                struct obj_data *tar_obj);
void            cast_protection_from_good(byte level, struct char_data *ch, char
                                          *arg, int si,
                                          struct char_data *tar_ch,
                                          struct obj_data *tar_obj);
void            cast_protection_from_good_group(byte level,
                                                struct char_data *ch,
                                                char *arg, int type,
                                                struct char_data *tar_ch,
                                                struct obj_data *tar_obj);
void            cast_giant_growth(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_remove_curse(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_sanctuary(byte level, struct char_data *ch, char *arg,
                               int si, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_sleep(byte level, struct char_data *ch, char *arg,
                           int si, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_strength(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_stone_skin(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_summon(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);
void            cast_ventriloquate(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_word_of_recall(byte level, struct char_data *ch,
                                    char *arg, int si,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);

void            cast_water_breath(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_remove_poison(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_remove_paralysis(byte level, struct char_data *ch,
                                      char *arg, int si,
                                      struct char_data *tar_ch,
                                      struct obj_data *tar_obj);
void            cast_weakness(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_sense_life(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_identify(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_paralyze(byte level, struct char_data *ch, char *arg,
                              int si, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_dragon_breath(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *potion);

void            cast_fireshield(byte level, struct char_data *ch,
                                char *arg, int si,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_cure_serious(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_cause_serious(byte level, struct char_data *ch,
                                   char *arg, int si,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_refresh(byte level, struct char_data *ch, char *arg,
                             int si, struct char_data *tar_ch,
                             struct obj_data *tar_obj);
void            cast_second_wind(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_shield(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);

void            cast_turn(byte level, struct char_data *ch, char *arg,
                          int si, struct char_data *tar_ch,
                          struct obj_data *tar_obj);

void            cast_succor(byte level, struct char_data *ch, char *arg,
                            int si, struct char_data *tar_ch,
                            struct obj_data *tar_obj);
void            cast_astral_walk(byte level, struct char_data *ch,
                                 char *arg, int si,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_resurrection(byte level, struct char_data *ch,
                                  char *arg, int si,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_tree_travel(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_haste(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_slow(byte level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj);
void            cast_speak_with_plants(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);

void            cast_transport_via_plant(byte level, struct char_data *ch,
                                         char *arg, int type,
                                         struct char_data *tar_ch,
                                         struct obj_data *tar_obj);

void            cast_plant_gate(byte level, struct char_data *ch, 
                                char *arg, int type, struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_changestaff(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);

void            cast_holyword(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_unholyword(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_aid(byte level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj);

void            cast_golem(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);

void            cast_familiar(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_pword_kill(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_pword_blind(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);

void            cast_command(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);

void            cast_scare(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);

void            cast_chain_lightn(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_reincarnate(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_feeblemind(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_shillelagh(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_goodberry(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_flame_blade(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_animal_growth(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_insect_growth(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_creeping_death(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_commune(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);

void            cast_animal_summon_1(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
#if 0
void            cast_animal_summon_2(byte level, struct char_data *ch, 
                                     char *arg, int type, 
                                     struct char_data *tar_ch, 
                                     struct obj_data *tar_obj);
void            cast_animal_summon_3(byte level, struct char_data *ch, 
                                     char *arg, int type, 
                                     struct char_data *tar_ch, 
                                     struct obj_data *tar_obj);
#endif
void            cast_fire_servant(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_earth_servant(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_water_servant(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_wind_servant(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_charm_veggie(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_veggie_growth(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_tree(byte level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj);

void            cast_animate_rock(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_travelling(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_animal_friendship(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);
void            cast_invis_to_animals(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *tar_ch,
                                      struct obj_data *tar_obj);
void            cast_slow_poison(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_entangle(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_snare(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);

void            cast_gust_of_wind(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_barkskin(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);

void            cast_sunray(byte level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj);
void            cast_heat_stuff(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_warp_weapon(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_find_traps(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_firestorm(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_dust_devil(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_know_monster(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);

void            cast_silence(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);

void            cast_sending(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);
void            cast_messenger(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);

void            cast_teleport_wo_error(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);
void            cast_portal(byte level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj);
void            cast_dragon_ride(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_mount(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);

/*
 * new ones msw 
 */
void            cast_globe_darkness(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_globe_minor_inv(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_globe_major_inv(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_prot_energy_drain(byte level, struct char_data *ch,
                                       char *arg, int type,
                                       struct char_data *tar_ch,
                                       struct obj_data *tar_obj);
void            cast_prot_dragon_breath(byte level, struct char_data *ch,
                                        char *arg, int type,
                                        struct char_data *tar_ch,
                                        struct obj_data *tar_obj);
void            cast_prot_dragon_breath_fire(byte level,
                                             struct char_data *ch,
                                             char *arg, int type,
                                             struct char_data *tar_ch,
                                             struct obj_data *tar_obj);
void            cast_prot_dragon_breath_frost(byte level,
                                              struct char_data *ch,
                                              char *arg, int type,
                                              struct char_data *tar_ch,
                                              struct obj_data *tar_obj);
void            cast_prot_dragon_breath_elec(byte level,
                                             struct char_data *ch,
                                             char *arg, int type,
                                             struct char_data *tar_ch,
                                             struct obj_data *tar_obj);
void            cast_prot_dragon_breath_acid(byte level,
                                             struct char_data *ch,
                                             char *arg, int type,
                                             struct char_data *tar_ch,
                                             struct obj_data *tar_obj);
void            cast_prot_dragon_breath_gas(byte level,
                                            struct char_data *ch,
                                            char *arg, int type,
                                            struct char_data *tar_ch,
                                            struct obj_data *tar_obj);

void            cast_anti_magic_shell(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *tar_ch,
                                      struct obj_data *tar_obj);
void            cast_comp_languages(byte level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *tar_ch,
                                    struct obj_data *tar_obj);
void            cast_prot_fire(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_prot_cold(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_prot_energy(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_prot_elec(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);

void            cast_disintegrate(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *victim,
                                  struct obj_data *tar_obj);
void            cast_wizard_eye(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *victim,
                                struct obj_data *tar_obj);

void            cast_energy_restore(byte level, struct char_data *ch, 
                                    char *arg, int type, 
                                    struct char_data *victim, 
                                    struct obj_data *tar_obj);

/*
 * Necro spells 
 */
void            cast_life_tap(byte level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj);
void            cast_disease(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);
void            cast_life_draw(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_numb_dead(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_binding(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);
void            cast_decay(byte level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj);
void            cast_shadow_step(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_cavorting_bones(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_mist_of_death(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_nullify(byte level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj);
void            cast_dark_empathy(byte level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj);
void            cast_eye_of_the_dead(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_soul_steal(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_life_leech(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_dark_pact(byte level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj);
void            cast_darktravel(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);
void            cast_vampiric_embrace(byte level, struct char_data *ch,
                                      char *arg, int type,
                                      struct char_data *tar_ch,
                                      struct obj_data *tar_obj);
void            cast_bind_affinity(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);
void            cast_scourge_warlock(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_finger_of_death(byte level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj);
void            cast_flesh_golem(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_chillshield(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);

void            cast_blade_barrier(byte level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj);

/* 
 * main class spells
 */
void            cast_mana_shield(byte level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj);
void            cast_iron_skins(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

void            cast_group_heal(byte level, struct char_data *ch,
                                char *arg, int type,
                                struct char_data *tar_ch,
                                struct obj_data *tar_obj);

struct spell_info_type spell_info[MAX_SPL_LIST];

char           *spells[] = {
    "armor",                    /* 1 */
    "teleport",
    "bless",
    "blindness",
    "burning hands",
    "call lightning",
    "charm person",
    "chill touch",
    "clone",
    "colour spray",
    "control weather",          /* 11 */
    "create food",
    "create water",
    "cure blind",
    "cure critic",
    "cure light",
    "curse",
    "detect evil",
    "detect invisibility",
    "detect magic",
    "detect poison",            /* 21 */
    "dispel evil",
    "earthquake",
    "enchant weapon",
    "energy drain",
    "fireball",
    "harm",
    "heal",
    "invisibility",
    "lightning bolt",
    "locate object",            /* 31 */
    "magic missile",
    "poison",
    "protection from evil",
    "remove curse",
    "sanctuary",
    "shocking grasp",
    "sleep",
    "strength",
    "summon",
    "ventriloquate",            /* 41 */
    "word of recall",
    "remove poison",
    "sense life",               /* 44 */

    /*
     * RESERVED SKILLS 
     */
    "sneak",                    /* 45 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "kick",                     /* 50 */
    "bash",
    "rescue",
    /*
     * NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) 
     */

    "identify",                 /* 53 */
    "infravision",
    "cause light",
    "cause critical",
    "flamestrike",
    "dispel good",
    "weakness",
    "dispel magic",
    "knock",
    "know alignment",
    "animate dead",
    "paralyze",
    "remove paralysis",
    "fear",
    "acid blast",               /* 67 */
    "water breath",
    "fly",
    "cone of cold",             /* 70 */
    "meteor swarm",
    "ice storm",
    "shield",
    "monsum one",
    "monsum two",
    "monsum three",
    "monsum four",
    "monsum five",
    "monsum six",
    "monsum seven",             /* 80 */
    "fireshield",
    "charm monster",
    "cure serious",
    "cause serious",
    "refresh",
    "second wind",
    "turn",
    "succor",
    "create light",
    "continual light",          /* 90 */
    "calm",
    "stone skin",
    "conjure elemental",
    "true sight",
    "minor creation",
    "faerie fire",
    "faerie fog",
    "cacaodemon",
    "polymorph self",
    "mana",                     /* 100 */
    "astral walk",
    "resurrection",
    "heroes feast",             /* 103 */
    "group fly",
    "breath",
    "web",
    "minor track",
    "major track",
    "golem",
    "find familiar",            /* 110 */
    "changestaff",
    "holy word",
    "unholy word",
    "power word kill",
    "power word blind",
    "chain lightning",
    "scare",
    "aid",
    "command",
    "change form",              /* 120 */
    "feeblemind",
    "shillelagh",
    "goodberry",
    "flame blade",
    "animal growth",
    "insect growth",
    "creeping death",
    "commune",
    "animal summon one",
    "animal summon two",        /* 130 */
    "animal summon three",
    "fire servant",
    "earth servant",
    "water servant",
    "wind servant",
    "reincarnate",
    "charm vegetable",
    "vegetable growth",
    "tree",
    "animate rock",             /* 140 */
    "tree travel",
    "travelling",
    "animal friendship",
    "invis to animals",
    "slow poison",
    "entangle",
    "snare",
    "gust of wind",
    "barkskin",
    "sunray",                   /* 150 */
    "warp weapon",
    "heat stuff",
    "find traps",
    "firestorm",
    "haste",
    "slowness",
    "dust devil",
    "know monster",
    "transport via plant",
    "speak with plants",        /* 160 */
    "silence",
    "sending",
    "teleport without error",
    "portal",
    "dragon ride",
    "mount",
    "energy restore",
    "detect good",              /* 168 */
    "protection from good",
    "first aid",                /* 170 */
    "sign language",
    "riding",
    "switch opponents",
    "dodge",
    "remove trap",
    "retreat",
    "quivering palm",
    "safe fall",
    "feign death",
    "hunt",                     /* 180 */
    "find trap",
    "spring leap",
    "disarm",
    "read magic",
    "evaluate",
    "spy",
    "doorbash",
    "swim",
    "necromancy",
    "vegetable lore",           /* 190 */
    "demonology",
    "animal lore",
    "reptile lore",
    "people lore",
    "giant lore",
    "other lore",               /* 196 */
    "disguise",
    "climb",
    "disengage",
    "***",                      /* 200 */
    "flying with your wings",
    "your wings are tired",
    "Your wings are burned",
    "protection from good group",
    "giant growth",
    "***",
    "berserk",
    "tan",
    "avoid back attack",
    "find food",                /* 210 */
    "find water",               /* 211 */
    "pray",                     /* spell Prayer, 212 */
    "memorizing",
    "bellow",
    "darkness",
    "minor invulnerability",
    "major invulnerability",
    "protection from drain",
    "protection from breath",
    "anti magic shell",         /* anti magic shell */
    "doorway",
    "psi portal",
    "psi summon",
    "psi invisibility",
    "canibalize",
    "flame shroud",
    "aura sight",
    "great sight",
    "psionic blast",
    "hypnosis",                 /* 230 */
    "meditate",
    "scry",
    "adrenalize",
    "brew",
    "ration",
    "warcry",
    "blessing",
    "lay on hands",
    "heroic rescue",
    "dual wield",               /* 240 */
    "psi shield",
    "protection from evil group",
    "prismatic spray",
    "incendiary cloud",
    "disintegrate",
    "language common",
    "language elvish",
    "language halfling",
    "language dwarvish",
    "language orcish",          /* 250 */
    "language giantish",
    "language ogre",
    "language gnomish",
    "esp",
    "comprehend languages",
    "protection from fire",
    "protection from cold",
    "protection from energy",
    "protection from electricity",
    "enchant armor",            /* 260 */
    "messenger",
    "protection fire breath",
    "protection frost breath",
    "protection electric breath",
    "protection acid breath",
    "protection gas breath",
    "wizardeye",
    "mind burn",
    "clairvoyance",
    "psionic danger sense",     /* 270 */
    "psionic disintegrate",
    "telekinesis",
    "levitation",
    "cell adjustment",
    "chameleon",
    "psionic strength",
    "mind over body",
    "probability travel",
    "psionic teleport",
    "domination",               /* 280 */
    "mind wipe",
    "psychic crush",
    "tower of iron will",
    "mindblank",
    "psychic impersonation",
    "ultra blast",
    "intensify",
    "spot",
    "holy armor",               /* 289 */
    "holy strength",
    "enlightenment",
    "circle of protection",
    "wrath of god",
    "pacifism",
    "aura of power",            /* 295 */
    "legsweep",
    "charge",
    "standard",
    "berserked",
    "aggressive",
    "defensive",
    "evasive",
    "mend",                     /* 303 */
    "call steed",
    /*
     * necro spells 
     */
    "cold light",
    "disease",
    "invis to undead",
    "life tap",
    "suit of bone",
    "spectral shield",          /* 310 */
    "clinging darkness",
    "dominate undead",
    "unsummon",
    "siphon strength",
    "gather shadows",
    "mend bones",
    "trace corpse",
    "endure cold",
    "life draw",
    "numb the dead",            /* 320 */
    "binding",
    "decay",
    "shadow step",
    "cavorting bones",
    "mist of death",
    "nullify",
    "dark empathy",
    "eye of the dead",
    "soul steal",
    "life leech",               /* 330 */
    "dark pact",
    "darktravel",
    "vampiric embrace",
    "bind affinity",
    "scourge of the warlock",
    "finger of death",
    "flesh golem",
    "chillshield",              /* 338 */

    /*
     * psi stuff 
     */
    "wall of thought",
    "mind tap",

    "blade barrier",            /* 341 - blade barrier */
    "mana shield",
    "iron skins",
    "sense object",
    "kinolock",
    "flowerfist",
    "flurry of blows",
    "scribe",                   /* 348 */
    "group heal",

    "plant gate",               /* 350 */
    "\n"
};

const byte      saving_throws[MAX_CLASS][5][ABS_MAX_LVL] = {
    {
     /*
      * mage 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * cleric 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37, 36,
      35, 35, 34, 34, 33, 33, 32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 27,
      26, 25, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 36, 35,
      34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 26,
      25, 24, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * warrior 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 28,
      28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21,
      21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33,
      33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25,
      24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * thief 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23,
      22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33,
      32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 23,
      22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {47, 46, 46, 45, 45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38,
      37, 37, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 29, 28, 28,
      27, 27, 26, 26, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * druid 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37, 36,
      35, 35, 34, 34, 33, 33, 32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 27,
      26, 25, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 36, 35,
      34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 26,
      25, 24, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * monk 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 28,
      28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21,
      21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33,
      33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25,
      24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * barbarian 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27,
      27, 26, 26, 25, 25, 24, 24, 23, 23, 23, 22, 22, 21, 21, 20, 20, 19,
      19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31,
      30, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 23, 22, 22,
      21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 36, 36, 35, 35, 34, 33, 33, 32, 32,
      31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 22, 22,
      21, 21, 20, 20, 19, 18, 18, 17, 17, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * sorcerer 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * paladin 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {30, 30, 29, 29, 28, 28, 28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24,
      24, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 19, 19, 19, 18, 18, 17,
      17, 17, 16, 16, 16, 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28,
      27, 27, 26, 26, 25, 25, 24, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28, 27, 27, 26, 26,
      26, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21, 21, 20, 20, 19, 19, 18,
      18, 18, 17, 17, 16, 16, 15, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 21,
      21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * ranger 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28,
      27, 27, 26, 26, 25, 25, 24, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23,
      22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 21,
      21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 36, 36, 35, 35, 34, 33, 33, 32, 32,
      31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 22, 22,
      21, 21, 20, 20, 19, 18, 18, 17, 17, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * psi 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 27,
      27, 27, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 21, 21,
      21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 31, 30, 30,
      29, 29, 28, 28, 27, 27, 26, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22,
      21, 21, 20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {27, 27, 26, 26, 26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 23, 23, 23,
      23, 22, 22, 22, 22, 21, 21, 21, 21, 20, 20, 20, 19, 19, 19, 19, 18,
      18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 35, 34, 34, 33, 33, 32, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 28, 27, 27, 26, 26, 25, 25, 24,
      24, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30,
      30, 29, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 24, 23,
      23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 16, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * necromancer 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     }
};

void spello(int nr, byte beat, byte pos,
            byte mlev, byte clev, byte dlev,
            byte slev, byte plev, byte rlev, byte ilev,
            ubyte mana, sh_int tar, void *func, sh_int sf,
            byte brewable, byte blev, byte nlev)
{
    spell_info[nr].spell_pointer = func;
    spell_info[nr].beats = beat;
    spell_info[nr].minimum_position = pos;
    spell_info[nr].min_level_cleric = clev;
    spell_info[nr].min_level_magic = mlev;
    spell_info[nr].min_level_druid = dlev;
    spell_info[nr].min_level_sorcerer = slev;
    spell_info[nr].min_level_paladin = plev;
    spell_info[nr].min_level_ranger = rlev;
    spell_info[nr].min_level_psi = ilev;
    spell_info[nr].min_usesmana = mana;
    spell_info[nr].targets = tar;
    spell_info[nr].spellfail = sf;
    spell_info[nr].brewable = brewable;
    spell_info[nr].min_level_bard = blev;
    spell_info[nr].min_level_necromancer = nlev;
}

int SPELL_LEVEL(struct char_data *ch, int sn)
{
    int             min;

    min = ABS_MAX_LVL;

    if (HasClass(ch, CLASS_MAGIC_USER)) {
        min = MIN(min, spell_info[sn].min_level_magic);
    }
    if (HasClass(ch, CLASS_SORCERER)) {
        min = MIN(min, spell_info[sn].min_level_sorcerer);
    }
    if (HasClass(ch, CLASS_NECROMANCER)) {
        min = MIN(min, spell_info[sn].min_level_necromancer);
    }
    if (HasClass(ch, CLASS_CLERIC)) {
        min = MIN(min, spell_info[sn].min_level_cleric);
    }
    if (HasClass(ch, CLASS_PALADIN)) {
        min = MIN(min, spell_info[sn].min_level_paladin);
    }
    if (HasClass(ch, CLASS_RANGER)) {
        min = MIN(min, spell_info[sn].min_level_ranger);
    }
    if (HasClass(ch, CLASS_PSI)) {
        min = MIN(min, spell_info[sn].min_level_psi);
    }
    if (HasClass(ch, CLASS_DRUID)) {
        min = MIN(min, spell_info[sn].min_level_druid);
    }
    return (min);

#if 0
    if ((HasClass(ch, CLASS_MAGIC_USER)) && (HasClass(ch, CLASS_CLERIC))) {
        return (MIN(spell_info[sn].min_level_magic,
                    spell_info[sn].min_level_cleric));
    } else if (HasClass(ch, CLASS_MAGIC_USER)) {
        return (spell_info[sn].min_level_magic);
    } else {
        return (spell_info[sn].min_level_cleric);
    }
#endif
}

#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

/*
 * This function returns the damage that a player will get when in a
 * certain sector type 
 */
int RoomElementalDamage(int flags, struct char_data *ch)
{
    int             damage = number(25, 50);
    int             type = 0;
    int             x;

    if (IS_SET(ch->specials.act, PLR_NOOUTDOOR)) {
        return 0;
    }

    if (IS_SET(flags, FIRE_ROOM)) {
        if (IS_IMMUNE(ch, IMM_FIRE)) {
            return 0;
        }
        if (IsResist(ch, IMM_FIRE)) {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_FIRE)) {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin.. OUCH\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin... OUCH!!\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, ICE_ROOM)) {
        if (IS_IMMUNE(ch, IMM_COLD)) {
            return 0;
        }
        if (IsResist(ch, IMM_COLD)) {   
            send_to_char("$c000CThe cold and frost seem to be taking "
                         "toll...\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_COLD)) {
            send_to_char("$c000CThe cold and frost seem to be taking toll... "
                         "OUCH\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000CThe cold and frost seem to be taking toll... "
                         "OUCH!!\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, EARTH_ROOM)) {
        if (IS_IMMUNE(ch, IMM_BLUNT)) {
            return 0;
        }
        if (IsResist(ch, IMM_BLUNT)) {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_BLUNT)) {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, ELECTRIC_ROOM)) {
        if (IS_IMMUNE(ch, IMM_ELEC)) {
            return 0;
        }
        if (IsResist(ch, IMM_ELEC)) {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_ELEC)) {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, WIND_ROOM)) {
        if (IS_IMMUNE(ch, IMM_PIERCE)) {
            return 0;
        }
        if (IsResist(ch, IMM_PIERCE)) {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_PIERCE)) {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage;
        }
    }

    return 0;
}


int GetMoveRegen(struct char_data *i)
{
    int             damagex = 0;

    /*
     * Movement
     */
    if (ValidRoom(i) && IS_SET(real_roomp(i->in_room)->room_flags, MOVE_ROOM)) {
        send_to_char("Your feel your stamina decrease.\n\r", i);
        damagex = number(15, 30);
    }
    return GET_MOVE(i) + move_gain(i) - damagex;

}

int GetHitRegen(struct char_data *i)
{
    char            buf[256];
    int             damagex = 0,
                    trollregen = 0,
                    darkpact = 0;
#if 0
    if(GET_RACE(i) == RACE_TROLL && GET_HIT(i)!=hit_limit(i)) {
        trollregen=hit_gain(i)*0.5; 
        send_to_char("Your wounds seem to close up and heal over some.\n\r",i);
        sprintf(buf,"%s's wounds seem to close up.\n\r",GET_NAME(i)); 
        send_to_room_except(buf,i->in_room,i);
    } 
#endif

    /*
     * darkpact regen penalty 
     */
    if (affected_by_spell(i, SPELL_DARK_PACT)) {
        send_to_char("$c0008Your pact with the Dark Lord grants your mental "
                     "power a boost, but drains some of your life.\n\r", i);
        darkpact = 15;
    }

    /*
     * Damage sector regen!!!!
     */
    damagex = RoomElementalDamage(real_roomp(i->in_room)->room_flags, i);
    if (damagex != 0) {
        sprintf(buf, "%s screams in pain!\n\r", GET_NAME(i));
        send_to_room_except(buf, i->in_room, i);
    }

    /*
     * Lets regen the character's Hitpoints
     */
    return hit_gain(i) + GET_HIT(i) - damagex - darkpact;       
#if 0
    +trollregen;
#endif
}

int ValidRoom(struct char_data *ch)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0        
        log("/* no room? BLAH!!! least it never crashed */");
#endif        
        return (FALSE);
    }
    return (TRUE);
}

int GetManaRegen(struct char_data *i)
{
    int             damagex = 0,
                    darkpact = 0;

    if (ValidRoom(i) && IS_SET(real_roomp(i->in_room)->room_flags, MANA_ROOM)) {
        send_to_char("You feel your aura being drained by some unknown "
                     "force!\n\r", i);
        damagex = number(15, 30);
    }

    /*
     * darkpact regen bonus 
     */
    if (affected_by_spell(i, SPELL_DARK_PACT)) {
        darkpact = 30;
    }

    /*
     * Mana
     */
    return GET_MANA(i) + mana_gain(i) - damagex + darkpact;
}

void affect_update(int pulse)
{
    register struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    register struct obj_data *j;
    struct obj_data *next_thing;
    struct char_data *next_char;
    struct room_data *rp;
    int             dead = FALSE,
                    room,
                    k;
    int             regenroom = 0,
                    damagex;
    char            buf[256];

    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        /*
         *  check the effects on the char
         */
        dead = FALSE;
        for (af = i->affected; af && !dead; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1 && af->type != SKILL_MEMORIZE) {
                af->duration--;
                if (af->duration == 1) {
                    SpellWearOffSoon(af->type, i);
                }
            } else if (af->type != SKILL_MEMORIZE) {
                /* dur < 1 */
                /*
                 * It must be a spell 
                 *
                 * /----- was FIRST_BREATH_WEAPON 
                 */
                if (af->type > 0 && af->type < MAX_EXIST_SPELL && 
                    af->type != SKILL_MEMORIZE) {
                    if (!af->next || af->next->type != af->type ||
                        af->next->duration > 0) {
                        SpellWearOff(af->type, i);
                        /*
                         * moved to it's own pulse update bcw
                         * check_memorize(i,af); 
                         */
#if 0
                        if (!affected_by_spell(i, SPELL_CHARM_PERSON))
                            /*
                             * added to fix bug 
                             */
                            /*
                             * ro does not remove it here! 
                             */
                            affect_remove(i, af);       /* msw 12/16/94 */
#endif
                    }
                } else if (af->type >= FIRST_BREATH_WEAPON && 
                           af->type <= LAST_BREATH_WEAPON) {
                    (bweapons[af->type - FIRST_BREATH_WEAPON])
                        (-af->modifier / 2, i, "", SPELL_TYPE_SPELL, i, 0);

                    if (!i->affected) {
                        /*
                         * oops, you're dead :) 
                         */
                        dead = TRUE;
                        break;
                    }

                    /*
                     * ro does not do it here either
                     *
                     *
                     * affect_remove(i, af); 
                     */
                }
                /*
                 * ro does it here! 
                 */
                affect_remove(i, af);
            }
        }

        if (!dead) {
            if (GET_POS(i) >= POSITION_STUNNED && (i->desc || !IS_PC(i))) {
                /*
                 * note - because of poison, this one has to be in the
                 * opposite order of the others.  The logic:
                 * 
                 * hit_gain() modifies the characters hps if they are
                 * poisoned. but if they were in the opposite order, the
                 * total would be: hps before poison + gain.  But of
                 * course, the hps after poison are lower, but No one
                 * cares! and that is why the gain is added to the hits,
                 * not vice versa 
                 */

                /*
                 * Regen mana/hitpoint/move
                 */
                regenroom = 0;

                if (ValidRoom(i) && 
                    IS_SET(real_roomp(i->in_room)->room_flags, REGEN_ROOM)) {
                    regenroom = 10;
                    if (GET_POS(i) > POSITION_SITTING) {
                        /* 
                         * Standing, fighting etc 
                         */
                        regenroom = 15;
                    } else if (GET_POS(i) > POSITION_SLEEPING) {
                        /* 
                         * Resting and sitting 
                         */
                        regenroom = 20;
                    } else if (GET_POS(i) > POSITION_STUNNED) {
                        /* 
                         * sleeping 
                         */
                        regenroom = 25;
                    } else {
                        regenroom = 20;
                    }
                    /*
                     * make it so imms can forego seeing this: 
                     */
                    if (!IS_SET(i->specials.act, PLR_NOOUTDOOR)) {
                        if (GET_HIT(i) != GET_MAX_HIT(i)) {
                            send_to_char("Your wounds seem to heal "
                                         "exceptionally quick.\n\r", i);
                        } else if (GET_MANA(i) != GET_MAX_MANA(i)) {
                            send_to_char("You feel your mystical abilities "
                                         "increase.\n\r", i);
                        } else if (GET_MOVE(i) != GET_MAX_MOVE(i)) {
                            send_to_char("Your stamina seems to increase "
                                         "rather quick.\n\r", i);
                        }
                    }
                }

                /*
                 * Lets regen the character's Hitpoints
                 */
                GET_HIT(i) = MIN(GetHitRegen(i) + regenroom, hit_limit(i));
                GET_MANA(i) = MIN(GetManaRegen(i) + regenroom, mana_limit(i));
                GET_MOVE(i) = MIN(GetMoveRegen(i) + regenroom, move_limit(i));
#if 0
                if (i->pc) {
                    GET_DIMD(i) += 2;
                }
#endif

                update_pos(i);
                if (GET_POS(i) == POSITION_DEAD) {
                    die(i, '\0');
                } else if (GET_POS(i) == POSITION_STUNNED) {
                    update_pos(i);
                }
            } else if (GET_POS(i) == POSITION_INCAP) {
                /*
                 * do nothing 
                 */ 
                damage(i, i, 0, TYPE_SUFFERING);
            } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
                damage(i, i, 1, TYPE_SUFFERING);
            }

            if (IS_PC(i)) {
                update_char_objects(i);
                if (GetMaxLevel(i) < DEMIGOD && i->in_room != 3 && 
                    i->in_room != 2) {
                    check_idling(i);
                }

                rp = real_roomp(i->in_room);
                if (rp) {
                    if (rp->sector_type == SECT_WATER_SWIM ||
                        rp->sector_type == SECT_WATER_NOSWIM) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -2);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -1);
                        }
                    } else if (rp->sector_type == SECT_DESERT) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -3);
                            gain_condition(i, THIRST, -3);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -3);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -2);
                        }
                    } else if (rp->sector_type == SECT_MOUNTAIN ||
                               rp->sector_type == SECT_HILLS) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                        } else {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                        }
                    } else {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -2);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                            gain_condition(i, THIRST, -3);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -1);
                            gain_condition(i, THIRST, -1);
                        }
                    }
                }

                if (i->specials.tick == time_info.hours) { 
                    /* 
                     * works for 24, change for anything else 
                     */
                    if (!IS_IMMORTAL(i) && i->in_room != 3) {
                        /* 
                         * the special case for room 3 is a hack to keep link
                         * dead people who have no stuff from being saved
                         * without stuff... 
                         */
                        do_save(i, "", 0);
                    }
                }
            }
            check_nature(i);    
            /* 
             * check falling, check drowning, etc 
             */
        }
    }

    /*
     *  update the objects
     */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;   
        /* 
         * Next in object list 
         */

        /*
         * If this is a corpse 
         */
        if (GET_ITEM_TYPE(j) == ITEM_CONTAINER && j->obj_flags.value[3]) {
            /*
             * timer count down 
             */
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by) {
                    act("$p biodegrades in your hands. Everything in it falls "
                        "to the floor", FALSE, j->carried_by, j, 0, TO_CHAR);
                } else if (j->in_room != NOWHERE && 
                           real_roomp(j->in_room)->people) {
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                ObjFromCorpse(j);
            }
        } else if (obj_index[j->item_number].virtual == EMPTY_SCROLL) {
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by)  {
                    act("$p crumbles to dust.", FALSE, j->carried_by, j, 0,
                        TO_CHAR);
                } else if (j->in_room != NOWHERE && 
                           real_roomp(j->in_room)->people) {
                    act("$p crumbles to dust.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p crumbles to dust.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                extract_obj(j);
            }
        } else if (obj_index[j->item_number].virtual == EMPTY_POTION) {
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by) {
                    act("$p dissolves into nothingness.", FALSE,
                        j->carried_by, j, 0, TO_CHAR);
                } else if ((j->in_room != NOWHERE)
                           && (real_roomp(j->in_room)->people)) {
                    act("$p dissolves into nothingness.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p dissolves into nothingness.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                extract_obj(j);
            }
        } else {
            /*
             *  Sound objects
             */
            if (ITEM_TYPE(j) == ITEM_AUDIO) {
                if ((j->obj_flags.value[0] &&
                     pulse % j->obj_flags.value[0] == 0) || !number(0, 5)) {
                    if (j->carried_by) {
                        room = j->carried_by->in_room;
                    } else if (j->equipped_by) {
                        room = j->equipped_by->in_room;
                    } else if (j->in_room != NOWHERE) {
                        room = j->in_room;
                    } else {
                        room = RecGetObjRoom(j);
                    }
                    /*
                     *  broadcast to room
                     */

                    if (j->action_description) {
                        MakeNoise(room, j->action_description,
                                  j->action_description);
                    }
                }
            } else {
                if (obj_index[j->item_number].func && j->item_number >= 0) {
                    (*obj_index[j->item_number].func) (0, 0, 0, j, PULSE_TICK);
                }
            }
        }
    }
}

void update_mem(int pulse)
{
    register struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    struct char_data *next_char;

    /*
     * start looking here 
     */
    for (i = mem_list; i; i = next_char) {
        next_char = i->next_memorize;
        for (af = i->affected; af; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1 && af->type == SKILL_MEMORIZE) {
                af->duration--;
            } else if (af->type == SKILL_MEMORIZE) {
                SpellWearOff(af->type, i);
                check_memorize(i, af);
                stop_memorizing(i);
                affect_remove(i, af);
            }
        }
    }
}

void stop_memorizing(struct char_data *ch)
{
    struct char_data *tmp,
                    tch;

    if (mem_list == ch && !ch->next_memorize) {
        mem_list = 0;
    } else if (mem_list == ch) {
        mem_list = ch->next_memorize;
    } else {
        for (tmp = mem_list; tmp && (tmp->next_memorize != ch);
             tmp = tmp->next_memorize) {
            /*
             * Empty loop
             */
        }
        if (!tmp) {
            log("Char memorize not found Error (spell_parser.c, "
                "stop_memorizing)");
            SpellWearOff(SKILL_MEMORIZE, ch);
            affect_from_char(ch, SKILL_MEMORIZE);
            ch->next_memorize = 0;
            return;
        }
        tmp->next_memorize = ch->next_memorize;
    }

    ch->next_memorize = 0;
}

#if 0
        /*
         * testing affect_update() 
         */
void affect_update(int pulse)
{
    struct descriptor_data *d;
    static struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    register struct obj_data *j;
    struct obj_data *next_thing;
    struct char_data *next_char;
    struct room_data *rp;
    int             dead = FALSE,
                    room,
                    y,
                    x,
                    bottom = 1,
                    top = 1,
                    time_until_backup = 1;
    long            time_until_reboot;
    char            buf[400];
    struct char_data *vict,
                   *next_v;
    struct obj_data *obj,
                   *next_o;

    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        /*
         *  check the effects on the char
         */
        dead = FALSE;
        for (af = i->affected; af && !dead; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1)
                af->duration--;
            else {
                /*
                 * It must be a spell 
                 */
                if ((af->type > 0) && (af->type <= 120)) {      /* urg.. a 
                                                                 * constant 
                                                                 */
                    if (!af->next || (af->next->type != af->type) ||
                        (af->next->duration > 0)) {
                        if (*spell_wear_off_msg[af->type]) {
                            send_to_char(spell_wear_off_msg[af->type], i);
                            send_to_char("\n\r", i);
                        }

                    }
                } else if (af->type >= FIRST_BREATH_WEAPON &&
                           af->type <= LAST_BREATH_WEAPON) {
                    extern funcp    bweapons[];
                    bweapons[af->type -
                             FIRST_BREATH_WEAPON] (-af->modifier / 2, i,
                                                   "", SPELL_TYPE_SPELL, i,
                                                   0);
                    if (!i->affected) {
                        /*
                         * oops, you're dead :) 
                         */
                        dead = TRUE;
                        break;
                    }
                }

                affect_remove(i, af);
            }
        }
        if (!dead) {

            if (GET_POS(i) >= POSITION_STUNNED) {
                GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
                GET_MANA(i) =
                    MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
                GET_MOVE(i) =
                    MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
                if (GET_POS(i) == POSITION_STUNNED)
                    update_pos(i);
            } else if (GET_POS(i) == POSITION_INCAP) {
                /*
                 * do nothing 
                 */ damage(i, i, 0, TYPE_SUFFERING);
            } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
                damage(i, i, 1, TYPE_SUFFERING);
            }
            if (IS_PC(i)) {
                update_char_objects(i);
                if (GetMaxLevel(i) < DEMIGOD)
                    check_idling(i);
                rp = real_roomp(i->in_room);
                if (rp) {
                    if (rp->sector_type == SECT_WATER_SWIM ||
                        rp->sector_type == SECT_WATER_NOSWIM) {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -1);
                    } else if (rp->sector_type == SECT_DESERT) {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -2);
                        gain_condition(i, THIRST, -2);
                    } else if (rp->sector_type == SECT_MOUNTAIN ||
                               rp->sector_type == SECT_HILLS) {
                        gain_condition(i, FULL, -2);
                        gain_condition(i, DRUNK, -2);
                    } else {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -1);
                        gain_condition(i, THIRST, -1);
                    }
                }

            }
        }
    }

    /*
     *  update the objects
     */

    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;   /* Next in object list */

        /*
         * If this is a corpse 
         */
        if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) &&
            (j->obj_flags.value[3])) {
            /*
             * timer count down 
             */
            if (j->obj_flags.timer > 0)
                j->obj_flags.timer--;

            if (!j->obj_flags.timer) {
                if (j->carried_by)
                    act("$p biodegrades in your hands.",
                        FALSE, j->carried_by, j, 0, TO_CHAR);
                else if ((j->in_room != NOWHERE) &&
                         (real_roomp(j->in_room)->people)) {
                    act("$p dissolves into a fertile soil.",
                        TRUE, real_roomp(j->in_room)->people, j, 0,
                        TO_ROOM);
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                ObjFromCorpse(j);
            }
        } else {

            /*
             *  Sound objects
             */
            if (ITEM_TYPE(j) == ITEM_AUDIO) {
                if (((j->obj_flags.value[0]) &&
                     (pulse % j->obj_flags.value[0]) == 0) ||
                    (!number(0, 5))) {
                    if (j->carried_by) {
                        room = j->carried_by->in_room;
                    } else if (j->equipped_by) {
                        room = j->equipped_by->in_room;
                    } else if (j->in_room != NOWHERE) {
                        room = j->in_room;
                    } else {
                        room = RecGetObjRoom(j);
                    }
                    /*
                     *  broadcast to room
                     */

                    if (j->action_description) {
                        MakeNoise(room, j->action_description,
                                  j->action_description);
                    }
                }
            }
        }
    }
}
#endif


/*
 * Check if making CH follow VICTIM will create an illegal 
 * Follow "Loop/circle" 
 * Problem: Appears to give us an infinite loop every once in a while 
 * Attempting to put in some guide to stop it...  Temp fix (GH)
 */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
    struct char_data *k;
    int             counter = 0;

    for (k = victim; k; k = k->master) {
        counter++;
        if (k == ch) {
            return (TRUE);
        }
        if (counter > 20) {
            log("Possible infinite Loop in circle follower?");
            return (TRUE);
        }
    }

    return (FALSE);
}

/*
 * Called when stop following persons, or stopping charm 
 * This will NOT do if a character quits/dies!! 
 */
void stop_follower(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (!ch->master) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

        if (affected_by_spell(ch, SPELL_CHARM_PERSON)) {
            affect_from_char(ch, SPELL_CHARM_PERSON);
        }
    } else {
        act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
            act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
        }
    }

    if (ch->master->followers->follower == ch) {
        /* 
         * Head of follower-list? 
         */
        k = ch->master->followers;
        ch->master->followers = k->next;
        if (k)
            free(k);
    } else {
        /* 
         * locate follower who is not head of list 
         */
        for (k = ch->master->followers; k->next && k->next->follower != ch;
             k = k->next) {
            /*
             * Empty loop
             */
        }

        if (k->next) {
            j = k->next;
            k->next = j->next;
            if (j) {
                free(j);
            }
        } else {
            assert(FALSE);
        }
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

void stop_follower_quiet(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (!ch->master) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

        if (affected_by_spell(ch, SPELL_CHARM_PERSON)) {
            affect_from_char(ch, SPELL_CHARM_PERSON);
        }
    } else {
#if 0
        act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
#endif
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
#if 0
            act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
#endif
        }
    }

    if (ch->master->followers->follower == ch) {
        /* 
         * Head of follower-list? 
         */
        k = ch->master->followers;
        ch->master->followers = k->next;
        if (k) {
            free(k);
        }
    } else {
        /* 
         * locate follower who is not head of list 
         */
        for (k = ch->master->followers; k->next && k->next->follower != ch;
             k = k->next) {
            /*
             * Empty loop
             */
        }
        if (k->next) {
            j = k->next;
            k->next = j->next;
            if (j) {
                free(j);
            }
        } else {
            assert(FALSE);
        }
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

/*
 * Called when a character that follows/is followed dies 
 */
void die_follower(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (ch->master) {
        stop_follower(ch);
    }
    for (k = ch->followers; k; k = j) {
        j = k->next;
        stop_follower(k->follower);
    }
}

/*
 * Do NOT call this before having checked if a circle of followers 
 * will arise. CH will follow leader 
 */
void add_follower(struct char_data *ch, struct char_data *leader)
{
    struct follow_type *k;
    char            buf[200];

    /*
     * instead of crashing the mud we try this 
     */
    if (ch->master) {
        act("$n cannot follow you for some reason.", TRUE, ch, 0, leader,
            TO_VICT);
        act("You cannot follow $N for some reason.", TRUE, ch, 0, leader,
            TO_CHAR);
        sprintf(buf, "%s cannot follow %s for some reason", GET_NAME(ch),
                GET_NAME(leader));
        log(buf);
        return;
    }

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
        act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
        act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
    }
}

struct syllable {
    char            org[10];
    char            new[10];
};

struct syllable syls[] = {
    {" ", " "}, {"ar", "abra"}, {"au", "kada"}, {"bless", "fido"},
    {"blind", "nose"}, {"bur", "mosa"}, {"cu", "judi"}, {"ca", "jedi"},
    {"de", "oculo"}, {"en", "unso"}, {"light", "dies"}, {"lo", "hi"},
    {"mor", "zak"}, {"move", "sido"}, {"ness", "lacri"}, {"ning", "illa"},
    {"per", "duda"}, {"ra", "gru"}, {"re", "candus"}, {"son", "sabru"},
    {"se", "or"}, {"tect", "infra"}, {"tri", "cula"}, {"ven", "nofo"},
    {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"}, {"e", "z"}, {"f", "y"},
    {"g", "o"}, {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
    {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"},
    {"s", "g"}, {"t", "h"}, {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
    {"y", "l"}, {"z", "k"}, {"", ""}
};

say_spell(struct char_data *ch, int si)
{
    char            buf[MAX_STRING_LENGTH],
                    splwd[MAX_BUF_LENGTH];
    char            buf2[MAX_STRING_LENGTH];

    int             j,
                    offs;
    struct char_data *temp_char;
    extern struct syllable syls[];

    strcpy(buf, "");
    strcpy(splwd, spells[si - 1]);

    offs = 0;

    while (*(splwd + offs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, splwd + offs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                if (strlen(syls[j].org)) {
                    offs += strlen(syls[j].org);
                } else {
                    ++offs;
                }
            }
        }
    }

    sprintf(buf2, "$n utters the words, '%s'", buf);
    sprintf(buf, "$n utters the words, '%s'", spells[si - 1]);

    for (temp_char = real_roomp(ch->in_room)->people;
         temp_char; temp_char = temp_char->next_in_room) {
        if (temp_char != ch) {
            if (GET_RACE(ch) == GET_RACE(temp_char)) {
                act(buf, FALSE, ch, 0, temp_char, TO_VICT);
            } else {
                act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
            }
        }
    }
}

weave_song(struct char_data *ch, int si)
{
    char            buf[MAX_STRING_LENGTH],
                    splwd[MAX_BUF_LENGTH];
    char            buf2[MAX_STRING_LENGTH];

    int             j,
                    offs;
    struct char_data *temp_char;
    extern struct syllable syls[];

    strcpy(buf, "");
    strcpy(splwd, spells[si - 1]);

    offs = 0;

    while (*(splwd + offs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, splwd + offs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                if (strlen(syls[j].org)) {
                    offs += strlen(syls[j].org);
                } else {
                    ++offs;
                }
            }
        }
    }

    sprintf(buf2, "$n utters the words, '%s'", buf);
    sprintf(buf, "$n magically weaves the song of %s.", spells[si - 1]);

    for (temp_char = real_roomp(ch->in_room)->people; temp_char;
         temp_char = temp_char->next_in_room) {
        if (temp_char != ch) {
            if (GET_RACE(ch) == GET_RACE(temp_char)) {
                act(buf, FALSE, ch, 0, temp_char, TO_VICT);
            } else {
                act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
            }
        }
    }
}

bool saves_spell(struct char_data *ch, sh_int save_type)
{
    int             save;

    /*
     * Negative apply_saving_throw makes saving throw better! 
     */

    save = ch->specials.apply_saving_throw[save_type];

    if (!IS_NPC(ch)) {
        save += saving_throws[BestMagicClass(ch)][save_type]
                             [GET_LEVEL(ch, BestMagicClass(ch))];
        if (GetMaxLevel(ch) > MAX_MORT) {
            return (TRUE);
        }
    }

    if (GET_RACE(ch) == RACE_GOD) {
        /* 
         * gods always save 
         */
        return (1);
    }

    return (MAX(5, save) < number(1, 50));
#if 0
    return(MAX(1,save) < number(1,20));
    char buf[200]; 
    int saveroll = number(1,50); 
    bool didsave = MAX(5,save) < saveroll; 
    sprintf(buf,"NaturalSave: %d SaveBonus: %d NumberRolled: %d "
                "DidSave?: %d\n\r", 
            saving_throws[BestMagicClass(ch)][save_type]
            [GET_LEVEL(ch,BestMagicClass(ch))], 
            ch->specials.apply_saving_throw[save_type], saveroll, didsave);
    log(buf); return(didsave);
#endif
}

bool ImpSaveSpell(struct char_data * ch, sh_int save_type, int mod)
{
    int             save;

    /*
     * Positive mod is better for save 
     * Negative apply_saving_throw makes saving throw better! 
     */

    save = ch->specials.apply_saving_throw[save_type] - mod;

    if (!IS_NPC(ch)) {
        save += saving_throws[BestMagicClass(ch)][save_type]
                             [GET_LEVEL(ch, BestMagicClass(ch))];
        if (GetMaxLevel(ch) >= LOW_IMMORTAL) {
            return (TRUE);
        }
    }

    return (MAX(1, save) < number(1, 20));
}

char           *skip_spaces(char *string)
{
    for (; *string && isspace(*string); string++) {
        /* 
         * Empty loop 
         */
    }

    return (string);
}

void do_id(struct char_data *ch, char *argument, int cmd)
{
    char            buf[256];

    sprintf(buf, "'id' %s", argument);
    do_cast(ch, buf, 0);
}

/*
 * Assumes that *argument does start with first letter of chopped string 
 */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
    char            buf[254];
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    char            name[MAX_INPUT_LENGTH];
    char            ori_argument[256];
    int             qend,
                    spl,
                    i,
                    exp;
    bool            target_ok;
    int             max,
                    cost;

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!IsHumanoid(ch)) {
        send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch)) {
        if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
            send_to_char("Think you had better stick to fighting...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
            send_to_char("Think you should stick to stealing...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
            send_to_char("Think you should stick to meditating...\n\r", ch);
            return;
        }
    }

    if (A_NOMAGIC(ch)) {
        send_to_char("The arena rules do not allow the use of magic!\n\r", ch);
        return;
    }

    if (cmd != 370 && apply_soundproof(ch)) {
        send_to_char("Too silent here to make a noise.\n\r", ch);
        return;
    }

    if (cmd == 370 && !HasClass(ch, CLASS_PSI)) {
        /* 
         * take away mind spells for non psi 
         */
        send_to_char("You, think, think harder.. and nearly bust a vein.\n\r",
                     ch);
        return;
    }

    if (!IS_IMMORTAL(ch) && HasClass(ch, CLASS_NECROMANCER) && 
        GET_ALIGNMENT(ch) >= -350) {
        /*
         * necro too GOOD to cast 
         */
        send_to_char("Alas, you have strayed too far from the Dark Lord's "
                     "guidance.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    for (i = 0; argument[i] && (i < 255); i++) {
        ori_argument[i] = argument[i];
    }
    ori_argument[i] = '\0';

    /*
     * If there is no chars in argument 
     */
    if (!(*argument)) {
        if (cmd != 600) {
            send_to_char("Cast which what where?\n\r", ch);
        } else {
            send_to_char("Sing which what where?\n\r", ch);
        }
        return;
    }

    if (*argument != '\'') {
        if (cmd != 600) {
            send_to_char("Magic must always be enclosed by the holy magic "
                         "symbols : '\n\r", ch);
        } else {
            send_to_char("Songs must always be enclosed by the vibrant symbols"
                         " : '\n\r", ch);
        }
        return;
    }

    /*
     * Locate the last quote && lowercase the magic words (if any) 
     */
    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++) {
        *(argument + qend) = LOWER(*(argument + qend));
    }

    if (*(argument + qend) != '\'') {
        if (cmd != 600) {
            send_to_char("Magic must always be enclosed by the holy magic "
                         "symbols : '\n\r", ch);
        } else {
            send_to_char("Songs must always be enclosed by the vibrant symbols"
                         " : '\n\r", ch);
        }
        return;
    }

    spl = old_search_block(argument, 1, qend - 1, spells, 0);

    if (!spl) {
        send_to_char("Nothing seems to happen! Wow! \n\r", ch);
        return;
    }

    /*
     * mobs do not get skills so we just check it for PC's 
     */
    if (!ch->skills && (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))) {
        send_to_char("You do not have skills!\n\r", ch);
        return;
    }

    if (cmd != 370 && OnlyClass(ch, CLASS_PSI)) {
        send_to_char("Use your mind!\n\r", ch);
        return;
    }

    if ((cmd == 84 || cmd == 370) && OnlyClass(ch, CLASS_SORCERER)) {
        send_to_char("You must use recall.\n\r", ch);
        return;
    }

    if ((cmd == 84 || cmd == 370) && HasClass(ch, CLASS_SORCERER) &&
        !IS_IMMORTAL(ch) && 
        IS_SET(ch->skills[spl].flags, SKILL_KNOWN_SORCERER)) {
        send_to_char("You must use recall for this spell.\n\r", ch);
        return;
    }

    if (ch->skills[spl].learned == 0) {
        send_to_char("You have no knowledge of this spell.\n\r", ch);
        return;
    }

    if (spl > 0 && spl < MAX_SKILLS && spell_info[spl].spell_pointer) {
        if (GET_POS(ch) < spell_info[spl].minimum_position) {
            switch (GET_POS(ch)) {
            case POSITION_SLEEPING:
                send_to_char("You dream about great magical powers.\n\r", ch);
                break;
            case POSITION_RESTING:
                send_to_char("You can't concentrate enough while resting.\n\r",
                             ch);
                break;
            case POSITION_SITTING:
                send_to_char("You can't do this sitting!\n\r", ch);
                break;
            case POSITION_FIGHTING:
                send_to_char("Impossible! You can't concentrate enough!.\n\r",
                             ch);
                break;
            default:
                send_to_char("It seems like you're in pretty bad shape!\n\r",
                             ch);
                break;
            }
        } else {
            if (!IS_IMMORTAL(ch) && 
                spell_info[spl].min_level_magic > 
                    GET_LEVEL(ch, MAGE_LEVEL_IND) && 
                spell_info[spl].min_level_sorcerer >
                    GET_LEVEL(ch, SORCERER_LEVEL_IND) && 
                spell_info[spl].min_level_cleric >
                    GET_LEVEL(ch, CLERIC_LEVEL_IND) && 
                spell_info[spl].min_level_paladin >
                    GET_LEVEL(ch, PALADIN_LEVEL_IND) && 
                spell_info[spl].min_level_ranger >
                    GET_LEVEL(ch, RANGER_LEVEL_IND) && 
                spell_info[spl].min_level_psi >
                    GET_LEVEL(ch, PSI_LEVEL_IND) && 
                spell_info[spl].min_level_druid >
                    GET_LEVEL(ch, DRUID_LEVEL_IND) && 
                spell_info[spl].min_level_necromancer >
                    GET_LEVEL(ch, NECROMANCER_LEVEL_IND)) {

                send_to_char("Sorry, you can't do that.\n\r", ch);
                return;
            }
            argument += qend + 1;       
            /* 
             * Point to the last ' 
             */
            for (; *argument == ' '; argument++) {
                /* 
                 * Empty loop 
                 */
            }

            /*
             **************** Locate targets **************** */
            target_ok = FALSE;
            tar_char = 0;
            tar_obj = 0;

            if (cmd != 600 && IS_SET(spell_info[spl].targets, TAR_VIOLENT) && 
                check_peaceful(ch, 
                               "This seems to be an dead magic zone!\n\r")) {
                return;
            }

            if (cmd == 600 && IS_SET(spell_info[spl].targets, TAR_VIOLENT) && 
                check_peaceful(ch, "Ah, no battle songs in here, matey!\n\r")) {
                return;
            }

            if (IS_IMMORTAL(ch) && IS_PC(ch) && GetMaxLevel(ch) < 59) {
                sprintf(buf, "%s cast %s", GET_NAME(ch), ori_argument);
                log(buf);
            }

            if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {
                argument = one_argument(argument, name);

                if (str_cmp(name, "self") == 0) {
                    sprintf(name, "%s", GET_NAME(ch));
                }

                if (*name) {
                    /*
                     * room char spells 
                     */
                    if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM)) {
                        if ((tar_char = get_char_room_vis(ch, name)) || 
                            !str_cmp(GET_NAME(ch), name)) {
                            if (!str_cmp(GET_NAME(ch), name)) {
                                tar_char = ch;
                            }

                            if (tar_char == ch || 
                                tar_char == ch->specials.fighting || 
                                tar_char->attackers < 6 || 
                                tar_char->specials.fighting == ch) {
                                target_ok = TRUE;
                            } else if (cmd != 600) {
                                send_to_char("Too much fighting, you can't get"
                                             " a clear shot.\n\r", ch);
                                target_ok = FALSE;
                            }
                        } else {
                            target_ok = FALSE;
                        }
                    }

                    /*
                     * world char spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD) && 
                        (tar_char = get_char_vis(ch, name))) {
                        target_ok = TRUE;
                    }

                    /*
                     * inv obj spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_OBJ_INV) &&
                        (tar_obj = 
                             get_obj_in_list_vis(ch, name, ch->carrying))) {
                        target_ok = TRUE;
                    }

                    /*
                     * room obj spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM) &&
                        (tar_obj = get_obj_in_list_vis(ch, name,
                                        real_roomp(ch->in_room)->contents))) {
                        target_ok = TRUE;
                    }

                    /*
                     * world obj spells, locate object & transport via
                     * plant 
                     */
                    if (!target_ok && IS_SET(spell_info[spl].targets,
                                             TAR_OBJ_WORLD)) {
                        target_ok = TRUE;
                        sprintf(argument, "%s", name);
                    }

                    /*
                     * eq obj spells 
                     */
                    if (!target_ok && IS_SET(spell_info[spl].targets,
                                             TAR_OBJ_EQUIP)) {
                        for (i = 0; i < MAX_WEAR && !target_ok; i++) {
                            if (ch->equipment[i] && 
                                !str_cmp(name, ch->equipment[i]->name)) {
                                tar_obj = ch->equipment[i];
                                target_ok = TRUE;
                            }
                        }
                    }

                    /*
                     * self only spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_SELF_ONLY) &&
                        !str_cmp(GET_NAME(ch), name)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    /*
                     * group spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_GROUP) &&
                        (tar_char = get_char_vis(ch, name)) && 
                        IS_AFFECTED(tar_char, AFF_GROUP) && 
                        in_group(ch, tar_char)) {

                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    /*
                     * name spells (?) 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_NAME)) {
                        tar_obj = (void *) name;
                        target_ok = TRUE;
                    }

                    if (tar_char && IS_NPC(tar_char) &&
                        IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
                        send_to_char("You can't cast magic on that!", ch);
                        return;
                    }
                } else {
                    /* 
                     * No argument 
                     */
                    if (IS_SET(spell_info[spl].targets, TAR_GROUP) &&
                        IS_AFFECTED(ch, AFF_GROUP)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF) &&
                        ch->specials.fighting) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT) &&
                        ch->specials.fighting) {
                        tar_char = ch->specials.fighting;
                        target_ok = TRUE;
                    }

                    if (!target_ok && 
                        IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }
                }
            } else {
                /* 
                 * No target, is a good target 
                 */
                target_ok = TRUE;
            }

            if (!target_ok) {
                if (*name) {
                    if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD)) {
                        send_to_char("Nobody playing by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, 
                                      TAR_CHAR_ROOM)) {
                        send_to_char("Nobody here by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV)) {
                        send_to_char("You are not carrying anything like "
                                     "that.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM)) {
                        send_to_char("Nothing here by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, 
                                      TAR_OBJ_WORLD)) {
                        send_to_char("Nothing at all by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, 
                                      TAR_OBJ_EQUIP)) {
                        send_to_char("You are not wearing anything like "
                                     "that.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, 
                                      TAR_OBJ_WORLD)) { 
                        send_to_char("Nothing at all by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[spl].targets, TAR_GROUP)) {
                        send_to_char("You can only cast this spell when "
                                     "grouped.\n\r", ch);
                    }
                } else {
                    /* 
                     * No argument 
                     */
                    if (IS_SET(spell_info[spl].targets, TAR_GROUP)) {
                        send_to_char("You can only cast this spell when "
                                     "grouped.\n\r", ch);
                    } else if (spell_info[spl].targets < TAR_OBJ_INV) {
                        if (cmd != 600) {
                            send_to_char("Who should the spell be cast "
                                         "upon?\n\r", ch);
                        } else {
                            send_to_char("Who should the song be aimed "
                                         "at?\n\r", ch);
                        }
                    } else {
                        if (cmd != 600) {
                            send_to_char("What should the spell be cast "
                                         "upon?\n\r", ch);
                        } else {
                            send_to_char("What should the song be aimed "
                                         "at?\n\r", ch);
                        }
                    }
                }
                return;
            } 
            
            /* 
             * TARGET IS OK 
             */
            if (tar_char == ch && 
                IS_SET(spell_info[spl].targets, TAR_SELF_NONO)) {
                if (cmd != 600) {
                    send_to_char("You can not cast this spell upon "
                                 "yourself.\n\r", ch);
                } else {
                    send_to_char("You can not aim this song at yourself.\n\r",
                                 ch);
                }
                return;
            } else if (tar_char != ch && 
                       IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
                if (cmd != 600) {
                    send_to_char("You can only cast this spell upon "
                                 "yourself.\n\r", ch);
                } else {
                    send_to_char("You can only aim this song at yourself.\n\r",
                                 ch);
                }
                return;
            } else if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == tar_char) {
                send_to_char("You are afraid that it could harm your "
                             "master.\n\r", ch);
                return;
            } else if (!IS_SET(spell_info[spl].targets, TAR_IGNORE) &&
                       A_NOASSIST(ch, tar_char)) {
                act("$N is engaged with someone else, no can do.",
                    FALSE, ch, 0, tar_char, TO_CHAR);
                return;
            }

            if (cmd == 283 && !MEMORIZED(ch, spl)) {
                /* 
                 * recall 
                 */
                send_to_char("You don't have that spell memorized!\n\r", ch);
                return;
            } 
            
            if (GetMaxLevel(ch) < LOW_IMMORTAL &&
                (GET_MANA(ch) < USE_MANA(ch, spl) || GET_MANA(ch) <= 0)) {
                send_to_char("You can't summon enough energy!\n\r", ch);
                return;
            }

            if (spl != SPELL_VENTRILOQUATE && cmd != 370) {
                /* 
                 * mind 
                 */
                say_spell(ch, spl);
            }

            WAIT_STATE(ch, spell_info[spl].beats);

            if (!spell_info[spl].spell_pointer && spl > 0) {
                send_to_char("Sorry, this magic has not yet been implemented "
                             ":(\n\r", ch);
            } else {
                max = ch->specials.spellfail;
                /* 
                 * 0 - 240 
                 */
                max += GET_COND(ch, DRUNK) * 10;

                switch (BestMagicClass(ch)) {
                case MAGE_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case SORCERER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10; 
                    }
                    break;
                case CLERIC_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_CLERIC)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case DRUID_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_DRUID)) {
                        /* 
                         * 20% harder to cast spells 
                         */ 
                        max += 10;
                    }
                    break;
                case PALADIN_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_PALADIN)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case PSI_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_PSI)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case RANGER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_RANGER)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case NECROMANCER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_NECROMANCER)) {
                        max += 10;
                    }
                    break;

                default:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                }

                max += int_sf_modifier[GET_INT(ch)].learn;

                if (ch->attackers > 0) {
                    max += spell_info[spl].spellfail;
                } else if (ch->specials.fighting) {
                    max += spell_info[spl].spellfail / 2;
                }
                if (number(1, max) > ch->skills[spl].learned && 
                    !IsSpecialized(ch->skills[spl].special) && cmd != 283) {
                    send_to_char("You lost your concentration!\n\r", ch);
                    cost = (int) USE_MANA(ch, (int) spl);
                    GET_MANA(ch) -= (cost >> 1);
                    LearnFromMistake(ch, spl, 0, 95);
                    return;
                }

                if (!IS_IMMORTAL(ch)) {
                    if (tar_char == ch &&
                        affected_by_spell(tar_char, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against your own anti-magic "
                            "shell!", FALSE, ch, 0, 0, TO_CHAR);
                        act("$n wastes a spell on $s own anti-magic shell!", 
                            FALSE, ch, 0, 0, TO_ROOM);
                        return;
                    }

                    if (tar_char && 
                        affected_by_spell(tar_char, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against $N's anti-magic shell!",
                            FALSE, ch, 0, tar_char, TO_CHAR);
                        act("$n wastes a spell on $N's anti-magic shell!",
                            FALSE, ch, 0, tar_char, TO_NOTVICT);
                        act("$n casts a spell and growls as it fizzles against"
                            " your anti-magic shell!", FALSE, ch, 0, tar_char,
                            TO_VICT);
                        return;
                    }
                    
                    if (GET_POS(tar_char) == POSITION_DEAD) {
                        send_to_char("The magic fizzles against the dead "
                                     "body.\n", ch);
                        return;
                    }

                    if (affected_by_spell(ch, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against your anti-magic shell!",
                            FALSE, ch, 0, 0, TO_CHAR);
                        act("$n tries to cast a spell within a anti-magic "
                            "shell, muhahaha!", FALSE, ch, 0, 0, TO_ROOM);
                        return;
                    }

                    if (check_nomagic(ch, "Your skill appears useless in this"
                                          " magic dead zone.\n\r",
                                      "$n's spell dissolves like so much wet "
                                      "toilet paper.\n\r")) {
                        return;
                    }
                }

                send_to_char("Ok.\n\r", ch);
                (*spell_info[spl].spell_pointer)
                    (GET_LEVEL(ch, BestMagicClass(ch)), ch, argument, 
                     SPELL_TYPE_SPELL, tar_char, tar_obj);

                cost = USE_MANA(ch, spl);
                exp = NewExpCap(ch, cost * 50);

                if (cmd == 283) {
                    /* 
                     * recall 
                     */
                    FORGET(ch, spl);
                }

                GET_MANA(ch) -= cost;
                sprintf(buf, "$c000BYou receive $c000W%d $c000Bexperience from"
                             " your expert casting abilities.$c000w\n\r", exp);
                send_to_char(buf, ch);
                gain_exp(ch, exp);
            }
        }
        return;
    }

    switch (number(1, 5)) {
    case 1:
        send_to_char("Bylle Grylle Grop Gryf???\n\r", ch);
        break;
    case 2:
        send_to_char("Olle Bolle Snop Snyf?\n\r", ch);
        break;
    case 3:
        send_to_char("Olle Grylle Bolle Bylle?!?\n\r", ch);
        break;
    case 4:
        send_to_char("Gryffe Olle Gnyffe Snop???\n\r", ch);
        break;
    default:
        send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r", ch);
        break;
    }
}

void assign_spell_pointers(void)
{
    int             i;

    /*
     * make sure defaults are 0! msw 
     */
    for (i = 0; i < MAX_SPL_LIST; i++) {
        spell_info[i].spell_pointer = 0;
        spell_info[i].min_level_cleric = 0;
        spell_info[i].min_level_magic = 0;
        spell_info[i].min_level_druid = 0;
        spell_info[i].min_level_sorcerer = 0;
        spell_info[i].min_level_paladin = 0;
        spell_info[i].min_level_ranger = 0;
        spell_info[i].min_level_psi = 0;
        spell_info[i].min_level_bard = 0;
        spell_info[i].brewable = 0;
        spell_info[i].min_level_necromancer = 0;
    }

    spello(1, 12, POSITION_STANDING, 5, 2, LOW_IMMORTAL,
           5, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_armor, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(2, 12, POSITION_STANDING, 17, LOW_IMMORTAL, LOW_IMMORTAL,
           17, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           33, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_teleport,
           60, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(3, 12, POSITION_STANDING, LOW_IMMORTAL, 1, LOW_IMMORTAL,
           17, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_bless, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(4, 24, POSITION_FIGHTING, 12, 14, LOW_IMMORTAL,
           12, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_blindness, 60, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(5, 24, POSITION_FIGHTING, 6, LOW_IMMORTAL, LOW_IMMORTAL,
           6, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE | TAR_VIOLENT, cast_burning_hands, 5, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(6, 36, POSITION_FIGHTING, LOW_IMMORTAL, 45, 18,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_call_lightning, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(7, 12, POSITION_STANDING, 4, LOW_IMMORTAL, IMMORTAL,
           4, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10,
           TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_charm_person, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(8, 12, POSITION_FIGHTING, 4, LOW_IMMORTAL, LOW_IMMORTAL,
           4, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_chill_touch, 10, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(9, 12, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           40, TAR_CHAR_ROOM, cast_clone, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(10, 24, POSITION_FIGHTING, 14, LOW_IMMORTAL, LOW_IMMORTAL,
           14, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_colour_spray, 40, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(11, 36, POSITION_STANDING, LOW_IMMORTAL, 26, 15,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           25, TAR_IGNORE, cast_control_weather, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(12, 12, POSITION_STANDING, LOW_IMMORTAL, 5, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_IGNORE, cast_create_food, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(13, 12, POSITION_STANDING, LOW_IMMORTAL, 2, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_create_water, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(14, 12, POSITION_STANDING, LOW_IMMORTAL, 6, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_cure_blind, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(15, 24, POSITION_FIGHTING, LOW_IMMORTAL, 10, 13,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           11, TAR_CHAR_ROOM, cast_cure_critic, 30, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(16, 12, POSITION_FIGHTING, LOW_IMMORTAL, 1, 2,
           LOW_IMMORTAL, 9, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_cure_light, 10, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(17, 24, POSITION_STANDING, 13, 12, LOW_IMMORTAL,
           13, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20,
           TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP |
           TAR_FIGHT_VICT | TAR_VIOLENT, cast_curse, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(18, 12, POSITION_STANDING, LOW_IMMORTAL, 1, 6,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_detect_evil, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(19, 12, POSITION_STANDING, 2, 5, 7,
           2, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_detect_invisibility, 0, 1, 6, 8);

    spello(20, 12, POSITION_STANDING, 1, 3, 5,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_detect_magic, 0, 1, 5, 3);

    spello(21, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 3, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,
           cast_detect_poison, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(22, 24, POSITION_FIGHTING, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_evil, 20, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(23, 24, POSITION_FIGHTING, LOW_IMMORTAL, 15, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE | TAR_VIOLENT, cast_earthquake, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(24, 48, POSITION_STANDING, 14, LOW_IMMORTAL, LOW_IMMORTAL,
           14, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           100, TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_weapon, 0, 0,
           LOW_IMMORTAL, 21);

    spello(25, 36, POSITION_FIGHTING, 22, LOW_IMMORTAL, LOW_IMMORTAL,
           22, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_energy_drain, 60, 1, LOW_IMMORTAL, 15);

    spello(26, 36, POSITION_FIGHTING, 25, LOW_IMMORTAL, LOW_IMMORTAL,
           25, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_IGNORE | TAR_VIOLENT, cast_fireball, 35, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(27, 36, POSITION_FIGHTING, LOW_IMMORTAL, 25, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_harm,
           110, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(28, 12, POSITION_FIGHTING, LOW_IMMORTAL, 25, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM, cast_heal, 90, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(29, 12, POSITION_STANDING, 4, LOW_IMMORTAL, LOW_IMMORTAL,
           4, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,
           cast_invisibility, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(30, 24, POSITION_FIGHTING, 10, LOW_IMMORTAL, LOW_IMMORTAL,
           10, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_lightning_bolt, 25, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(31, 12, POSITION_STANDING, LOW_IMMORTAL, 11, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_OBJ_WORLD, cast_locate_object, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(32, 12, POSITION_FIGHTING, 1, LOW_IMMORTAL, LOW_IMMORTAL,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_magic_missile, 10, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(33, 24, POSITION_FIGHTING, LOW_IMMORTAL, 13, 8,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10,
           TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP |
           TAR_FIGHT_VICT | TAR_VIOLENT, cast_poison, 60, 0, LOW_IMMORTAL,
           8);

    spello(34, 12, POSITION_STANDING, LOW_IMMORTAL, 7, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, 25, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_protection_from_evil, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(35, 12, POSITION_STANDING, LOW_IMMORTAL, 8, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,
           cast_remove_curse, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(36, 36, POSITION_STANDING, LOW_IMMORTAL, 26, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM, cast_sanctuary, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(37, 12, POSITION_FIGHTING, 2, LOW_IMMORTAL, LOW_IMMORTAL,
           2, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_shocking_grasp, 10, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(38, 24, POSITION_FIGHTING, 3, LOW_IMMORTAL, LOW_IMMORTAL,
           3, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, cast_sleep,
           20, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(39, 12, POSITION_STANDING, 6, LOW_IMMORTAL, LOW_IMMORTAL,
           6, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM, cast_strength, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(40, 36, POSITION_STANDING, 27, 19, LOW_IMMORTAL,
           27, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_WORLD | TAR_VIOLENT, cast_summon, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(41, 12, POSITION_STANDING, 1, LOW_IMMORTAL, LOW_IMMORTAL,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO,
           cast_ventriloquate, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(42, 12, POSITION_STANDING, LOW_IMMORTAL, 15, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_word_of_recall, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(43, 12, POSITION_STANDING, LOW_IMMORTAL, 17, 8,
           LOW_IMMORTAL, 40, 20, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,
           cast_remove_poison, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(44, 12, POSITION_STANDING, LOW_IMMORTAL, 4, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_sense_life, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* sneak */
    spello(45, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, 4, BIG_GUY + 1, 200, TAR_IGNORE,
           0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL); 

    /* hide */
    spello(46, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, 2, BIG_GUY + 1, 200, TAR_IGNORE,
           0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(47, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(48, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(49, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(50, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* bash */
    spello(51, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, 1, 1, BIG_GUY + 1, 200, TAR_IGNORE, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(52, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, 3, BIG_GUY + 1,
           200, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(53, 24, POSITION_STANDING, 10, 16, IMMORTAL,
           10, IMMORTAL, IMMORTAL, IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV, cast_identify,
           0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(54, 12, POSITION_STANDING, 8, LOW_IMMORTAL, 5,
           8, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           7, TAR_CHAR_ROOM, cast_infravision, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(55, 12, POSITION_FIGHTING, LOW_IMMORTAL, 1, 2,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           8, TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_cause_light, 10, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(56, 24, POSITION_FIGHTING, LOW_IMMORTAL, 10, 13,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           11, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_cause_critic, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(57, 36, POSITION_FIGHTING, LOW_IMMORTAL, 15, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_flamestrike, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(58, 36, POSITION_FIGHTING, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_dispel_good, 20, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(59, 12, POSITION_FIGHTING, 6, LOW_IMMORTAL, LOW_IMMORTAL,
           6, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_weakness,
           30, 0, LOW_IMMORTAL, 9);

    spello(60, 12, POSITION_FIGHTING, 9, 10, 9,
           9, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_ROOM | TAR_OBJ_INV |
           TAR_VIOLENT, cast_dispel_magic, 50, 1, LOW_IMMORTAL, 14);

    spello(61, 12, POSITION_STANDING, 3, LOW_IMMORTAL, LOW_IMMORTAL,
           3, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_knock, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(62, 12, POSITION_STANDING, 7, 4, 2,
           7, 5, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_know_alignment, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(63, 12, POSITION_STANDING, 16, 8, LOW_IMMORTAL,
           16, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_OBJ_ROOM, cast_animate_dead, 0, 0, LOW_IMMORTAL, 5);

    spello(64, 36, POSITION_FIGHTING, 20, LOW_IMMORTAL, LOW_IMMORTAL,
           20, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_paralyze,
           90, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(65, 12, POSITION_FIGHTING, LOW_IMMORTAL, 6, 9,
           LOW_IMMORTAL, 15, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT, cast_remove_paralysis, 30,
           1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(66, 12, POSITION_FIGHTING, 8, LOW_IMMORTAL, LOW_IMMORTAL,
           8, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_fear, 30,
           0, LOW_IMMORTAL, 4);

    spello(67, 24, POSITION_FIGHTING, 6, LOW_IMMORTAL, LOW_IMMORTAL,
           6, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_acid_blast, 20, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(68, 12, POSITION_STANDING, 9, LOW_IMMORTAL, 6,
           9, LOW_IMMORTAL, 17, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM, cast_water_breath, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(69, 12, POSITION_STANDING, 11, 22, 14,
           11, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM, cast_flying, 0, 1, LOW_IMMORTAL, 20);

    spello(70, 24, POSITION_FIGHTING, 17, LOW_IMMORTAL, LOW_IMMORTAL,
           17, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE | TAR_VIOLENT, cast_cone_of_cold, 40, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(71, 24, POSITION_FIGHTING, 36, LOW_IMMORTAL, LOW_IMMORTAL,
           36, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_meteor_swarm, 80, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(72, 12, POSITION_FIGHTING, 11, LOW_IMMORTAL, LOW_IMMORTAL,
           11, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE | TAR_VIOLENT, cast_ice_storm, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(73, 24, POSITION_FIGHTING, 1, LOW_IMMORTAL, LOW_IMMORTAL,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           1, TAR_CHAR_ROOM, cast_shield, 15, 1, 5, LOW_IMMORTAL);

    spello(74, 24, POSITION_STANDING, 4, LOW_IMMORTAL, LOW_IMMORTAL,
           4, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_mon_sum1, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(75, 24, POSITION_STANDING, 7, LOW_IMMORTAL, LOW_IMMORTAL,
           7, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           12, TAR_IGNORE, cast_mon_sum2, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(76, 24, POSITION_STANDING, 9, LOW_IMMORTAL, LOW_IMMORTAL,
           9, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE, cast_mon_sum3, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(77, 24, POSITION_STANDING, 12, LOW_IMMORTAL, LOW_IMMORTAL,
           12, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           17, TAR_IGNORE, cast_mon_sum4, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(78, 24, POSITION_STANDING, 15, LOW_IMMORTAL, LOW_IMMORTAL,
           15, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_IGNORE, cast_mon_sum5, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(79, 24, POSITION_STANDING, 18, LOW_IMMORTAL, LOW_IMMORTAL,
           18, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           22, TAR_IGNORE, cast_mon_sum6, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(80, 24, POSITION_STANDING, 22, LOW_IMMORTAL, LOW_IMMORTAL,
           22, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           25, TAR_IGNORE, cast_mon_sum7, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(81, 24, POSITION_STANDING, 40, LOW_IMMORTAL, 48,
           40, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_SELF_ONLY | TAR_CHAR_ROOM, cast_fireshield, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(82, 12, POSITION_STANDING, 10, LOW_IMMORTAL, 12,
           10, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_monster, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(83, 12, POSITION_FIGHTING, LOW_IMMORTAL, 7, 8,
           45, 45, LOW_IMMORTAL, LOW_IMMORTAL,
           9, TAR_CHAR_ROOM, cast_cure_serious, 20, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(84, 12, POSITION_FIGHTING, LOW_IMMORTAL, 7, 8,
           45, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           9, TAR_CHAR_ROOM | TAR_VIOLENT, cast_cause_serious, 20, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(85, 12, POSITION_STANDING, 6, 3, 4,
           6, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_refresh, 0, 1, LOW_IMMORTAL, 5);

    spello(86, 12, POSITION_FIGHTING, 20, 9, 14,
           20, 35, 30, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_second_wind, 25, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(87, 12, POSITION_STANDING, LOW_IMMORTAL, 1, 12,
           LOW_IMMORTAL, 10, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_turn, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(88, 24, POSITION_STANDING, 24, 23, LOW_IMMORTAL,
           24, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE, cast_succor, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(89, 12, POSITION_STANDING, 1, 2, 3,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_IGNORE, cast_light, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(90, 24, POSITION_STANDING, 10, 26, 16,
           10, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_cont_light, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(91, 24, POSITION_STANDING, 4, 2, LOW_IMMORTAL,
           4, 18, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM, cast_calm, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(92, 24, POSITION_STANDING, 26, LOW_IMMORTAL, LOW_IMMORTAL,
           26, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_SELF_ONLY, cast_stone_skin, 0, 0, LOW_IMMORTAL, 32);

    spello(93, 24, POSITION_STANDING, 16, 13, 11,
           16, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_conjure_elemental, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(94, 24, POSITION_STANDING, LOW_IMMORTAL, 19, 24,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM, cast_true_seeing, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(95, 24, POSITION_STANDING, 8, LOW_IMMORTAL, LOW_IMMORTAL,
           8, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_minor_creation, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(96, 12, POSITION_STANDING, 5, 4, 1,
           5, LOW_IMMORTAL, 7, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT,
           cast_faerie_fire, 0, 0, 9, LOW_IMMORTAL);

    spello(97, 24, POSITION_STANDING, 16, 11, 10,
           16, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_IGNORE, cast_faerie_fog, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(98, 24, POSITION_STANDING, 30, 29, LOW_IMMORTAL,
           30, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_IGNORE, cast_cacaodemon, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(99, 12, POSITION_STANDING, 9, LOW_IMMORTAL, LOW_IMMORTAL,
           9, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_poly_self, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(100, 12, POSITION_FIGHTING, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE, cast_mana, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(101, 12, POSITION_STANDING, LOW_IMMORTAL, 30, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_CHAR_WORLD, cast_astral_walk, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(102, 36, POSITION_STANDING, LOW_IMMORTAL, 36, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           33, TAR_OBJ_ROOM, cast_resurrection, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(103, 12, POSITION_STANDING, LOW_IMMORTAL, 24, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_IGNORE, cast_heroes_feast, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(104, 12, POSITION_STANDING, 24, LOW_IMMORTAL, 22,
           24, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_fly_group, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(105, 250, POSITION_FIGHTING, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           200, TAR_IGNORE | TAR_VIOLENT, cast_dragon_breath, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(106, 12, POSITION_FIGHTING, 11, LOW_IMMORTAL, LOW_IMMORTAL,
           11, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           3, TAR_CHAR_ROOM, cast_web, 40, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(107, 12, POSITION_STANDING, 12, LOW_IMMORTAL, 7,
           12, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM, cast_minor_track, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(108, 12, POSITION_STANDING, 20, LOW_IMMORTAL, 17,
           20, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM, cast_major_track, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(109, 24, POSITION_STANDING, IMMORTAL, 15, IMMORTAL,
           IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_golem, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(110, 24, POSITION_STANDING, 2, IMMORTAL, IMMORTAL,
           2, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_familiar, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(111, 24, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 30,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_changestaff, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(112, 24, POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE | TAR_VIOLENT, cast_holyword, 80, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(113, 24, POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE | TAR_VIOLENT, cast_unholyword, 80, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(114, 24, POSITION_FIGHTING, 23, IMMORTAL, IMMORTAL,
           23, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_pword_kill, 40, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(115, 24, POSITION_FIGHTING, 16, IMMORTAL, IMMORTAL,
           16, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_pword_blind, 20, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(116, 24, POSITION_FIGHTING, 25, IMMORTAL, 25,
           25, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_chain_lightn, 70, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(117, 24, POSITION_FIGHTING, 4, IMMORTAL, IMMORTAL,
           4, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           3, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_scare, 20,
           0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(118, 24, POSITION_FIGHTING, LOW_IMMORTAL, 4, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_aid, 10, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(119, 24, POSITION_FIGHTING, LOW_IMMORTAL, 1, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           3, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_command,
           30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(120, 12, POSITION_STANDING, IMMORTAL, IMMORTAL, 12,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_IGNORE, cast_change_form, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(121, 24, POSITION_FIGHTING, 34, IMMORTAL, IMMORTAL,
           34, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_feeblemind, 40, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(122, 12, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 3,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_OBJ_INV, cast_shillelagh, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(123, 12, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 4,
           LOW_IMMORTAL, LOW_IMMORTAL, 10, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_goodberry, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(124, 12, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 7,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_flame_blade, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(125, 24, POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 35,
           LOW_IMMORTAL, LOW_IMMORTAL, 35, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_SELF_NONO, cast_animal_growth, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(126, 24, POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 33,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_SELF_NONO, cast_insect_growth, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(127, 36, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 45,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_IGNORE, cast_creeping_death, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(128, 24, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 20,
           LOW_IMMORTAL, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_commune, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(129, 24, POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 15,
           LOW_IMMORTAL, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           15, TAR_IGNORE, cast_animal_summon_1, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(130, 24, POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 20,
           LOW_IMMORTAL, LOW_IMMORTAL, 25, LOW_IMMORTAL,
           20, TAR_IGNORE, cast_animal_summon_2, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(131, 24, POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 25,
           LOW_IMMORTAL, LOW_IMMORTAL, 30, LOW_IMMORTAL,
           25, TAR_IGNORE, cast_animal_summon_3, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(132, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, 35,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_fire_servant, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(133, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, 36,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_earth_servant, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(134, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, 37,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_water_servant, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(135, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, 38,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_IGNORE, cast_wind_servant, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(136, 36, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 39,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_OBJ_ROOM, cast_reincarnate, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(137, 12, POSITION_STANDING, IMMORTAL, LOW_IMMORTAL, 17,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM | TAR_VIOLENT, cast_charm_veggie, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(138, 24, POSITION_FIGHTING, LOW_IMMORTAL, IMMORTAL, 20,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_SELF_NONO, cast_veggie_growth, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(139, 24, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 15,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_tree, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(140, 24, POSITION_STANDING, LOW_IMMORTAL, IMMORTAL, 31,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_OBJ_INV | TAR_OBJ_ROOM, cast_animate_rock, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(141, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 8,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           2, TAR_SELF_ONLY, cast_tree_travel, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(142, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 10,
           LOW_IMMORTAL, LOW_IMMORTAL, 15, LOW_IMMORTAL,
           2, TAR_SELF_ONLY, cast_travelling, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(143, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 5,
           LOW_IMMORTAL, LOW_IMMORTAL, 3, LOW_IMMORTAL,
           2, TAR_CHAR_ROOM | TAR_SELF_NONO, cast_animal_friendship, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(144, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 11,
           LOW_IMMORTAL, LOW_IMMORTAL, 8, LOW_IMMORTAL,
           10, TAR_SELF_ONLY, cast_invis_to_animals, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(145, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 6,
           LOW_IMMORTAL, 10, 10, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM, cast_slow_poison, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(146, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 16,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_entangle,
           0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(147, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 8,
           LOW_IMMORTAL, LOW_IMMORTAL, 5, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_snare, 0,
           0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(148, 12, POSITION_FIGHTING, 10, LOW_IMMORTAL, IMMORTAL,
           10, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           2, TAR_IGNORE, cast_gust_of_wind, 30, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(149, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 3,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_barkskin, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(150, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 27,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_sunray,
           30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(151, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 19,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_OBJ_ROOM |
           TAR_OBJ_INV, cast_warp_weapon, 60, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(152, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 23,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_heat_stuff, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(153, 12, POSITION_STANDING, LOW_IMMORTAL, 16, 15,
           LOW_IMMORTAL, LOW_IMMORTAL, 13, LOW_IMMORTAL,
           10, TAR_SELF_ONLY, cast_find_traps, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(154, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL, 22,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_IGNORE, cast_firestorm, 30, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(155, 12, POSITION_STANDING, 23, IMMORTAL, IMMORTAL,
           23, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_VIOLENT, cast_haste, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(156, 12, POSITION_STANDING, 19, IMMORTAL, IMMORTAL,
           19, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_CHAR_ROOM | TAR_VIOLENT, cast_slow, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(157, 24, POSITION_STANDING, LOW_IMMORTAL, 3, 1,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_IGNORE, cast_dust_devil, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(158, 12, POSITION_FIGHTING, 9, IMMORTAL, IMMORTAL,
           9, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_SELF_NONO | TAR_CHAR_ROOM | TAR_FIGHT_VICT,
           cast_know_monster, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(159, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 10,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           20, TAR_OBJ_WORLD, cast_transport_via_plant, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(160, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 7,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_OBJ_ROOM, cast_speak_with_plants, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(161, 12, POSITION_FIGHTING, 21, 23, 25,
           21, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           30, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, cast_silence,
           80, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(162, 12, POSITION_STANDING, 1, LOW_IMMORTAL, 1,
           1, LOW_IMMORTAL, 1, LOW_IMMORTAL,
           5, 0, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(163, 12, POSITION_STANDING, 30, BIG_GUY, BIG_GUY,
           30, BIG_GUY, BIG_GUY, BIG_GUY,
           20, TAR_CHAR_WORLD, cast_teleport_wo_error, 0, 1, LOW_IMMORTAL,
           35);

    spello(164, 12, POSITION_STANDING, 43, BIG_GUY, BIG_GUY,
           43, BIG_GUY, BIG_GUY, BIG_GUY,
           50, TAR_CHAR_WORLD, cast_portal, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(165, 12, POSITION_STANDING, BIG_GUY, BIG_GUY, BIG_GUY,
           BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
           20, TAR_IGNORE, cast_dragon_ride, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(166, 12, POSITION_STANDING, 47, BIG_GUY, 15,
           47, BIG_GUY, BIG_GUY, BIG_GUY,
           20, TAR_IGNORE, cast_mount, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(167, 1, POSITION_STANDING, LOW_IMMORTAL, 48, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM, cast_energy_restore, 20, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(168, 12, POSITION_STANDING, LOW_IMMORTAL, 1, 6,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_detect_good, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(169, 12, POSITION_STANDING, LOW_IMMORTAL, 7, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, TAR_CHAR_ROOM, cast_protection_from_good, 0, 1, LOW_IMMORTAL,
           1);

    /* first aid */
    spello(170, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, 1, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* switch opp */
    spello(173, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, 1, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* dodge */
    spello(174, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, 1, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* hunt */
    spello(180, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* disarm */
    spello(183, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, 1, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* spy */
    spello(186, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(187, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, 1, BIG_GUY + 1, 200,
           TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* climb */
    spello(198, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(203, 12, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, TAR_IGNORE, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(204, 12, POSITION_STANDING, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           45, TAR_IGNORE, cast_protection_from_good_group, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(205, 12, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           IMMORTAL, IMMORTAL, 20, IMMORTAL,
           50, TAR_CHAR_ROOM, cast_giant_growth, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* berserk */
    spello(207, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 200,
           0, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* tan */
    spello(208, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* avoid ba */
    spello(209, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 200,
           0, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* find food */
    spello(210, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* find water */
    spello(211, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* bellow */
    spello(214, 0, POSITION_STANDING, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1,
           BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, BIG_GUY + 1, 200,
           0, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(215, 12, POSITION_STANDING, 5, 3, 3,
           5, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM, cast_globe_darkness, 0, 1, LOW_IMMORTAL, 5);

    spello(216, 12, POSITION_STANDING, 20, IMMORTAL, IMMORTAL,
           20, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           25, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_globe_minor_inv, 0, 1,
           LOW_IMMORTAL, 26);

    spello(217, 24, POSITION_STANDING, 27, IMMORTAL, IMMORTAL,
           27, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_globe_major_inv, 0, 0,
           LOW_IMMORTAL, 30);

    spello(218, 12, POSITION_STANDING, IMMORTAL, 48, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_energy_drain, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(219, 12, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           200, TAR_CHAR_ROOM, cast_prot_dragon_breath, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(220, 24, POSITION_STANDING, 48, BIG_GUY, BIG_GUY,
           48, BIG_GUY, BIG_GUY, BIG_GUY,
           100, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_anti_magic_shell, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* doorway */
    spello(221, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL, 
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 3, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* portal */
    spello(222, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 32, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* summon */
    spello(223, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 10, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* invis */
    spello(224, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 1, 20, 0, 0, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* canibalize */
    spello(225, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 2, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* flame shroud */
    spello(226, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 15, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* aura site */
    spello(227, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 5, 20, 0, 0, 0, 0, 
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* great site */
    spello(228, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL, 
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 11, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* blast */
    spello(229, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 1, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* hypnosis */
    spello(230, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 2, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* meditate */
    spello(231, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 1, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* scry */
    spello(232, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 7, 20, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* adrenalize */
    spello(233, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 20, 20, 0, 0, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* brew potion */
    spello(234, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, 10, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, 10, 200, 0, 0, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* ration */
    spello(235, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* paladin warcry */
    spello(236, 0, POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 5, LOW_IMMORTAL, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* paladin blessing */
    spello(237, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 1, LOW_IMMORTAL, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* paladin lay on hands */
    spello(238, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 1, LOW_IMMORTAL, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* paladin heroic rescue */
    spello(239, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 1, LOW_IMMORTAL, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* dual wield */
    spello(240, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* psi shield */
    spello(241, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 1, 20,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(242, 12, POSITION_STANDING, LOW_IMMORTAL, 20, LOW_IMMORTAL,
           LOW_IMMORTAL, 35, 45, LOW_IMMORTAL,
           45, TAR_IGNORE, cast_protection_from_evil_group, 0, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(243, 36, POSITION_FIGHTING, 17, LOW_IMMORTAL, LOW_IMMORTAL,
           17, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_IGNORE | TAR_VIOLENT, cast_prismatic_spray, 50, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(244, 36, POSITION_FIGHTING, 45, LOW_IMMORTAL, LOW_IMMORTAL,
           45, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           60, TAR_IGNORE | TAR_VIOLENT, cast_incendiary_cloud, 80, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(245, 12, POSITION_FIGHTING, 48, LOW_IMMORTAL, LOW_IMMORTAL,
           48, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           45, TAR_CHAR_ROOM | TAR_VIOLENT, cast_disintegrate, 60, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* psi esp */
    spello(254, 24, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 1, 10, 
           TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(255, 12, POSITION_STANDING, 1, 3, LOW_IMMORTAL,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM, cast_comp_languages, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(256, 12, POSITION_STANDING, BIG_GUY, 21, 22,
           BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_fire, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(257, 12, POSITION_STANDING, BIG_GUY, 20, 21,
           BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_cold, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(258, 12, POSITION_STANDING, BIG_GUY, 19, BIG_GUY,
           BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_energy, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(259, 12, POSITION_STANDING, BIG_GUY, 18, 19,
           BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_elec, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(260, 48, POSITION_STANDING, 16, LOW_IMMORTAL, LOW_IMMORTAL,
           16, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           100, TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_enchant_armor, 0, 1,
           LOW_IMMORTAL, 17);

    spello(261, 12, POSITION_STANDING, 1, 1, LOW_IMMORTAL,
           1, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           5, 0, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(262, 24, POSITION_STANDING, IMMORTAL, 40, 41,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_dragon_breath_fire,
           0, 1, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(263, 24, POSITION_STANDING, IMMORTAL, 38, 39,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY,
           cast_prot_dragon_breath_frost, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(264, 24, POSITION_STANDING, IMMORTAL, 39, 40,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_dragon_breath_elec,
           0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(265, 24, POSITION_STANDING, IMMORTAL, 40, 41,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_dragon_breath_acid,
           0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(266, 24, POSITION_STANDING, IMMORTAL, 37, IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_prot_dragon_breath_gas,
           0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(267, 24, POSITION_STANDING, 35, LOW_IMMORTAL, LOW_IMMORTAL,
           35, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_wizard_eye, 20, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(268, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 5, 15,
           TAR_IGNORE | TAR_VIOLENT, mind_use_burn, 5, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(269, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 8, 45,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_clairvoyance, 40, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(270, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 23, 50,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_danger_sense, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(271, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 38, 50,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT,
           mind_use_disintegrate, 10, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(272, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 35, 15,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT,
           mind_use_telekinesis, 50, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(273, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 10, 20,
           TAR_CHAR_ROOM, mind_use_levitation, 40, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(274, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 14, 100,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_cell_adjustment, 80, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(275, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 8, 40,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_chameleon, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(276, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 6, 15,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_psi_strength, 20, 1,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(277, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 14, 60,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_mind_over_body, 20, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(278, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 21, 40,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_probability_travel, 0,
           0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(279, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 12, 15,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, mind_use_teleport,
           30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(280, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 51, 30,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT,
           mind_use_domination, 50, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(281, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 21, 20,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT,
           mind_use_mind_wipe, 70, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(282, 34, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 37, 45,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT,
           mind_use_psychic_crush, 40, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(283, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 34, 50,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_tower_iron_will, 30, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(284, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 17, 50,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_mindblank, 20, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(285, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 3, 50,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, mind_use_psychic_impersonation,
           0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(286, 35, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 20, 30,
           TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_IGNORE,
           mind_use_ultra_blast, 50, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(287, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 100, TAR_SELF_ONLY | TAR_CHAR_ROOM,
           mind_use_intensify, 30, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* spot */
    spello(288, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, 1, 1, LOW_IMMORTAL, 100, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(289, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 5, LOW_IMMORTAL, LOW_IMMORTAL,
           25, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_holy_armor, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(290, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 3, LOW_IMMORTAL, LOW_IMMORTAL,
           25, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_holy_strength, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(291, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 25, LOW_IMMORTAL, LOW_IMMORTAL,
           10, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_enlightenment, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(292, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 1, LOW_IMMORTAL, LOW_IMMORTAL,
           200, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_circle_protection, 0,
           0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(293, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 51, LOW_IMMORTAL, LOW_IMMORTAL,
           200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_wrath_god, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(294, 0, POSITION_FIGHTING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 7, LOW_IMMORTAL, LOW_IMMORTAL,
           15, TAR_CHAR_ROOM, cast_pacifism, 30, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(295, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 15, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_CHAR_ROOM | TAR_SELF_ONLY, cast_aura_power, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    /* paladin charge */
    spello(297, 0, POSITION_STANDING, IMMORTAL, IMMORTAL, IMMORTAL,
           LOW_IMMORTAL, 4, LOW_IMMORTAL, LOW_IMMORTAL, 200, 0, 0, 0, 0,
           LOW_IMMORTAL, LOW_IMMORTAL);

    spello(305, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 35, 20,
           TAR_SELF_ONLY, cast_wall_of_thought, 0, 1, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(306, 36, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 20, 35,
           TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, mind_use_mind_tap,
           110, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    spello(305, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_IGNORE, cast_cold_light, 0, 0,
           LOW_IMMORTAL, 1);

    spello(306, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_disease, 10, 0, LOW_IMMORTAL, 1);

    spello(307, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_IGNORE, cast_invis_to_undead, 0, 1,
           LOW_IMMORTAL, 2);

    spello(308, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_life_tap, 10, 0, LOW_IMMORTAL, 2);

    spello(309, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_IGNORE, cast_suit_of_bone, 0, 1,
           LOW_IMMORTAL, 3);

    spello(310, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 10, TAR_IGNORE, cast_spectral_shield, 20, 1,
           LOW_IMMORTAL, 4);

    spello(311, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_clinging_darkness, 30, 0, LOW_IMMORTAL, 5);

    spello(312, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
           cast_dominate_undead, 0, 0, LOW_IMMORTAL, 6);

    spello(313, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_unsummon, 40, 0, LOW_IMMORTAL, 7);

    spello(314, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_siphon_strength, 20, 0, LOW_IMMORTAL, 8);

    spello(315, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 5, TAR_IGNORE, cast_gather_shadows, 0, 0,
           LOW_IMMORTAL, 8);

    spello(316, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 11, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
           cast_mend_bones, 50, 0, LOW_IMMORTAL, 10);

    spello(317, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_OBJ_WORLD, cast_trace_corpse, 0, 0,
           LOW_IMMORTAL, 11);

    spello(318, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 25, TAR_IGNORE, cast_endure_cold, 0, 0,
           LOW_IMMORTAL, 12);

    spello(319, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 12, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_life_draw, 20, 0, LOW_IMMORTAL, 15);

    spello(320, 36, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 30, TAR_CHAR_ROOM | TAR_FIGHT_VICT,
           cast_numb_dead, 0, 0, LOW_IMMORTAL, 15);

    spello(321, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 15, TAR_IGNORE, cast_binding, 0, 0, LOW_IMMORTAL,
           16);

    spello(322, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_decay, 10, 0, LOW_IMMORTAL, 18);

    spello(323, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_IGNORE, cast_shadow_step, 0, 0,
           LOW_IMMORTAL, 20);

    spello(324, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 25, TAR_OBJ_ROOM, cast_cavorting_bones, 0, 0,
           LOW_IMMORTAL, 22);

    spello(325, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 35, TAR_IGNORE | TAR_VIOLENT, cast_mist_of_death,
           80, 0, LOW_IMMORTAL, 28);

    spello(326, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM, cast_nullify, 0, 0,
           LOW_IMMORTAL, 23);

    spello(327, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 30, TAR_CHAR_ROOM, cast_dark_empathy, 70, 0,
           LOW_IMMORTAL, 24);

    spello(328, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 25, TAR_IGNORE, cast_eye_of_the_dead, 0, 0,
           LOW_IMMORTAL, 26);

    spello(329, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_soul_steal, 30, 0, LOW_IMMORTAL, 27);

    spello(330, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_life_leech, 30, 0, LOW_IMMORTAL, 31);

    spello(331, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 33, TAR_IGNORE, cast_dark_pact, 0, 0,
           LOW_IMMORTAL, 36);

    spello(332, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 30, TAR_CHAR_WORLD, cast_darktravel, 0, 0,
           LOW_IMMORTAL, 39);

    spello(333, 36, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 50, TAR_IGNORE, cast_vampiric_embrace, 0, 0,
           LOW_IMMORTAL, 41);

    spello(334, 36, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 50, TAR_IGNORE, cast_bind_affinity, 0, 0,
           LOW_IMMORTAL, 48);

    spello(335, 12, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_scourge_warlock, 50, 0, LOW_IMMORTAL, 29);

    spello(336, 24, POSITION_FIGHTING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,
           cast_finger_of_death, 60, 0, LOW_IMMORTAL, 25);

    spello(337, 36, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 33, TAR_OBJ_ROOM, cast_flesh_golem, 0, 0,
           LOW_IMMORTAL, 37);

    spello(338, 24, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, 40, TAR_IGNORE, cast_chillshield, 0, 0,
           LOW_IMMORTAL, 45);

    spello(341, 24, POSITION_STANDING, LOW_IMMORTAL, 45, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           40, TAR_IGNORE, cast_blade_barrier, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(342, 36, POSITION_STANDING, 14, LOW_IMMORTAL, LOW_IMMORTAL,
           14, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           50, TAR_IGNORE, cast_mana_shield, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    spello(343, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 20,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           65, TAR_IGNORE, cast_iron_skins, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* sense item */
    spello(344, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 25, 25,
           TAR_OBJ_WORLD, mind_use_sense_object, 0, 0, LOW_IMMORTAL,
           LOW_IMMORTAL);

    /* kinolock */
    spello(345, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 9, 15,
           TAR_IGNORE, mind_use_kinolock, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

#if 0
    /* flowerfist */
    spello(346,24,POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 36, LOW_IMMORTAL, 25,
           TAR_IGNORE | TAR_ROOM, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* flurry of blows */
    spello(347,24,POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, 18, LOW_IMMORTAL, 40, 
           TAR_IGNORE | TAR_GROUP, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL); 

    /* scribe */
    spello(348,36,POSITION_STANDING, LOW_IMMORTAL, 1, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 80,
           TAR_CHAR_ROOM | TAR_GROUP, 0, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL); 
#endif

    /* group heal */
    spello(349, 12, POSITION_FIGHTING, LOW_IMMORTAL, 29, LOW_IMMORTAL,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 75,
           TAR_IGNORE, cast_group_heal, 60, 0, LOW_IMMORTAL, LOW_IMMORTAL);

    /* plant gate */
    spello(350, 12, POSITION_STANDING, LOW_IMMORTAL, LOW_IMMORTAL, 40,
           LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, LOW_IMMORTAL, 60,
           TAR_OBJ_WORLD, cast_plant_gate, 0, 0, LOW_IMMORTAL, LOW_IMMORTAL); 
}

void SpellWearOffSoon(int s, struct char_data *ch)
{
    if (s > MAX_SKILLS + 10) {
        return;
    }
    if (spell_wear_off_soon_msg[s] && *spell_wear_off_soon_msg[s]) {
        send_to_char(spell_wear_off_soon_msg[s], ch);
        send_to_char("\n\r", ch);
    }

    if (spell_wear_off_soon_room_msg[s] && *spell_wear_off_soon_room_msg[s]) {
        act(spell_wear_off_soon_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
    }
}

void SpellWearOff(int s, struct char_data *ch)
{
    struct affected_type af;

    if (s > MAX_SKILLS + 10) {
        return;
    }
    if (spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
        send_to_char(spell_wear_off_msg[s], ch);
        send_to_char("\n\r", ch);
    }

    if (spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s]) {
        act(spell_wear_off_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
    }

    if (s == SPELL_CHARM_PERSON || s == SPELL_CHARM_MONSTER) {
        check_decharm(ch);
    }

    if (s == SPELL_FLY) {
        check_falling(ch);
    }
    if (s == SPELL_WATER_BREATH) {
        check_drowning(ch);
    }

    if (s == COND_WINGS_FLY) {
        af.type = COND_WINGS_TIRED;
        af.location = APPLY_BV2;
        af.duration = 25 - GET_CON(ch);
        af.modifier = 0;
        af.bitvector = AFF2_WINGSTIRED;
        affect_to_char(ch, &af);
        check_falling(ch);
    }
}

void check_decharm(struct char_data *ch)
{
    struct char_data *m;

    if (!ch->master) {
        return;
    }
    m = ch->master;
    stop_follower(ch);
    REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
    AddFeared(ch, m);
    do_flee(ch, "", 0);
}

int check_falling(struct char_data *ch)
{
    struct room_data *rp,
                   *targ;
    int             done,
                    count,
                    saved;
    char            buf[256];

    if (IS_IMMORTAL(ch)) {
        /* 
         * so if this guy is using redit the mud does not crash when he 
         * falls... 
         */
        return (FALSE);         
    }

    if (GET_POS(ch) <= POSITION_DEAD) {
        /* 
         * ch will be purged, check corpse instead, bug fix msw 
         */
        return (FALSE);
    }

    if (IS_AFFECTED(ch, AFF_FLYING)) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);
    if (!rp) {
        return (FALSE);
    }
    if (rp->sector_type != SECT_AIR) {
        return (FALSE);
    }
    if (ch->skills && number(1, 101) < ch->skills[SKILL_SAFE_FALL].learned) {
        act("You manage to slow your fall, enough to stay alive..",
            TRUE, ch, 0, 0, TO_CHAR);
        saved = TRUE;
    } else {
        act("The world spins, and you sky-dive out of control",
            TRUE, ch, 0, 0, TO_CHAR);
        saved = FALSE;
    }

    done = FALSE;
    count = 0;

    while (!done && count < 100) {
        /*
         * check for an exit down. if there is one, go through it. 
         */
        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        } else {
            /*
             * pretend that this is the smash room. 
             */
            if (IS_SET(rp->room_flags, ARENA_ROOM)) {
                /* 
                 * Volcano part of the Arena
                 */
                send_to_char("You are enveloped in a sea of burning flames and"
                             " molten rock.\n\r", ch);
                send_to_char("Soon, you are nothing but ashes.\n\r", ch);
                sprintf(buf, "%s disappears beneath a sea of flame with a loud"
                             " hiss and lot of smoke.\n\r", GET_NAME(ch));
                send_to_room_except(buf, ch->in_room, ch);

                sprintf(buf, "%s killed by burning in ARENA!\n\r",
                        GET_NAME(ch));
                send_to_all(buf);
                raw_kill_arena(ch);
                return (TRUE);
            }

            if (count > 1) {
                send_to_char("You are smashed into tiny pieces.\n\r", ch);
                act("$n smashes against the ground at high speed",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("You are drenched with blood and gore",
                    FALSE, ch, 0, 0, TO_ROOM);

                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                if (!IS_IMMORTAL(ch)) {
                    GET_HIT(ch) = 0;
                    sprintf(buf, "%s has fallen to death", GET_NAME(ch));
                    log(buf);

                    if (!ch->desc) {
                        GET_GOLD(ch) = 0;
                    }
                    die(ch, '\0');      /* change to the smashed type */
                }
                return (TRUE);
            }
            
            send_to_char("You land with a resounding THUMP!\n\r", ch);
            GET_HIT(ch) = 0;
            GET_POS(ch) = POSITION_STUNNED;
            act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

            /*
             * should damage all their stuff 
             */
            DamageAllStuff(ch, BLOW_DAMAGE);

            return (TRUE);
        }

        act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You plunge from the sky\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, rp->dir_option[DOWN]->to_room);
        act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
        count++;

        do_look(ch, "", 0);

        if (IS_SET(targ->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
            NailThisSucker(ch);
            return (TRUE);
        }

        if (targ->sector_type != SECT_AIR) {
            /*
             * do damage, or kill 
             */
            if (count == 1) {
                send_to_char("You land with a resounding THUMP!\n\r", ch);
                GET_HIT(ch) = 0;
                GET_POS(ch) = POSITION_STUNNED;
                act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0,
                    TO_ROOM);
                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                return (TRUE);
            }
            
            if (!saved) {
                send_to_char("You are smashed into tiny pieces.\n\r", ch);
                if (targ->sector_type >= SECT_WATER_SWIM) {
                    act("$n is smashed to a pulp by $s impact with the water",
                        FALSE, ch, 0, 0, TO_ROOM);
                } else {
                    act("$n is smashed to a bloody pulp by $s impact with the"
                        " ground", FALSE, ch, 0, 0, TO_ROOM);
                }
                act("You are drenched with blood and gore", FALSE, ch, 0,
                    0, TO_ROOM);

                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                if (!IS_IMMORTAL(ch)) {
                    GET_HIT(ch) = 0;
                    sprintf(buf, "%s has fallen to death", GET_NAME(ch));
                    log(buf);
                    
                    if (!ch->desc) {
                        GET_GOLD(ch) = 0;
                    }    
                    die(ch, '\0');
                }
                return (TRUE);
            }
            
            send_to_char("You land with a resounding THUMP!\n\r", ch);
            GET_HIT(ch) = 0;
            GET_POS(ch) = POSITION_STUNNED;
            act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

            /*
             * should damage all their stuff 
             */
            DamageAllStuff(ch, BLOW_DAMAGE);
            return (TRUE);
        }
        
        /*
         * time to try the next room 
         */
        rp = targ;
        targ = 0;
    }

    if (count >= 100) {
        log("Someone messed up an air room.");
        char_from_room(ch);
        char_to_room(ch, 2);
        do_look(ch, "", 0);
        return (FALSE);
    }
}

void check_drowning(struct char_data *ch)
{
    struct room_data *rp;
    char            buf[256];

    if (IS_AFFECTED(ch, AFF_WATERBREATH)) {
        return;
    }
    rp = real_roomp(ch->in_room);

    if (!rp) {
        return;
    }
    if (rp->sector_type == SECT_UNDERWATER) {
        send_to_char("PANIC!  You're drowning!!!!!!", ch);
        GET_HIT(ch) -= number(1, 30);
        GET_MOVE(ch) -= number(10, 50);
        update_pos(ch);

        if (GET_HIT(ch) < -10) {
            sprintf(buf, "%s killed by drowning", GET_NAME(ch));
            log(buf);

            if (!ch->desc) {
                GET_GOLD(ch) = 0;
            }
            die(ch, '\0');
        }
    }
}

void check_falling_obj(struct obj_data *obj, int room)
{
    struct room_data *rp,
                   *targ;
    int             done,
                    count;

    if (obj->in_room != room) {
        log("unusual object information in check_falling_obj");
        return;
    }

    rp = real_roomp(room);
    if (!rp || rp->sector_type != SECT_AIR) {
        return;
    }
    done = FALSE;
    count = 0;

    while (!done && count < 100) {
        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        } else if (count > 1) {
            if (rp->people) {
                /*
                 * pretend that this is the smash room. 
                 */
                act("$p smashes against the ground at high speed",
                    FALSE, rp->people, obj, 0, TO_ROOM);
                act("$p smashes against the ground at high speed",
                    FALSE, rp->people, obj, 0, TO_CHAR);
            }
            return;
        } else if (rp->people) {
            act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0, 
                TO_ROOM);
            act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0,
                TO_CHAR);
            return;
        }

        if (rp->people) {
            /* 
             * have to reference a person 
             */
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
        }

        obj_from_room(obj);
        obj_to_room(obj, rp->dir_option[DOWN]->to_room);

        if (targ->people) {
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
        }

        count++;

        if (targ->sector_type != SECT_AIR) {
            if (count == 1) {
                if (targ->people) {
                    act("$p lands with a loud THUMP!", FALSE, targ->people,
                        obj, 0, TO_ROOM);
                    act("$p lands with a loud THUMP!", FALSE, targ->people,
                        obj, 0, TO_CHAR);
                }
                return;
            } 

            if (targ->people) {
                if (targ->sector_type >= SECT_WATER_SWIM) {
                    act("$p smashes against the water at high speed",
                        FALSE, targ->people, obj, 0, TO_ROOM);
                    act("$p smashes against the water at high speed",
                        FALSE, targ->people, obj, 0, TO_CHAR);
                } else {
                    act("$p smashes against the ground at high speed",
                        FALSE, targ->people, obj, 0, TO_ROOM);
                    act("$p smashes against the ground at high speed",
                        FALSE, targ->people, obj, 0, TO_CHAR);
                }
            }
            return;
        } else {
            /*
             * time to try the next room 
             */
            rp = targ;
            targ = 0;
        }
    }

    if (count >= 100) {
        log("Someone screwed up an air room.");
        obj_from_room(obj);
        obj_to_room(obj, 4);
        return;
    }
}

int check_nature(struct char_data *i)
{

    if (check_falling(i)) {
        return (TRUE);
    }
    check_drowning(i);

}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
