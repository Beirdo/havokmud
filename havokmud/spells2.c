#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"

/*
 * Global data 
 */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern int      rev_dir[];
extern char    *dirs[];
extern int      movement_loss[];
extern struct weather_data weather_info;
extern struct time_info_data time_info;
extern struct index_data *obj_index;

/*
 * Extern procedures 
 */

void            update_pos(struct char_data *victim);
void            clone_char(struct char_data *ch);
bool            saves_spell(struct char_data *ch, sh_int spell);
void            add_follower(struct char_data *ch,
                             struct char_data *victim);
void            ChangeWeather(int change);
void            raw_unlock_door(struct char_data *ch,
                                struct room_direction_data *exitp,
                                int door);
int             NoSummon(struct char_data *ch);

void cast_unholyword(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_holyword(-level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in unholy word.");
        break;
    }
}

void cast_numb_dead(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_numb_dead(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_numb_dead");
        break;
    }
}

void cast_binding(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
        spell_binding(level, ch, 0, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj)
            return;
        spell_binding(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_binding");
        break;
    }
}

void cast_decay(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_decay(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_decay");
        break;
    }
}

void cast_shadow_step(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_shadow_step(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in cast_shadow_step");
        break;
    }
}

void cast_cavorting_bones(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_cavorting_bones(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in cast_cavorting_bones");
        break;
    }
}

void cast_mist_of_death(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_mist_of_death(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_mist_of_death");
        break;
    }
}

void cast_nullify(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_nullify(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_nullify");
        break;
    }
}

void cast_dark_empathy(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dark_empathy(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_dark_empathy");
        break;
    }
}

void cast_eye_of_the_dead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_eye_of_the_dead(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_eye_of_the_dead");
        break;
    }
}

void cast_soul_steal(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_soul_steal(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_soul_steal");
        break;
    }
}

void cast_life_leech(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_leech(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_life_leech");
        break;
    }
}

void cast_dark_pact(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dark_pact(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_dark_pact");
        break;
    }
}

void cast_darktravel(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_darktravel(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_darktravel");
        break;
    }
}

void cast_vampiric_embrace(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_vampiric_embrace(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_vampiric_embrace");
        break;
    }
}

void cast_bind_affinity(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_bind_affinity(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_bind_affinity");
        break;
    }
}

void cast_scourge_warlock(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_scourge_warlock(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_scourge_warlock");
        break;
    }
}

void cast_finger_of_death(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_finger_of_death(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_finger_of_death");
        break;
    }
}

void cast_flesh_golem(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_flesh_golem(level, ch, 0, tar_obj);
        break;
    default:
        Log("serious screw-up in cast_flesh_golem");
        break;
    }
}

void cast_chillshield(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_chillshield(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_chillshield");
        break;
    }
}

void cast_cold_light(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_cold_light(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cold light.");
        break;
    }
}

void cast_disease(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_disease(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in disease.");
        break;
    }
}

void cast_invis_to_undead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_invis_to_undead(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in invis_to_undead.");
        break;
    }
}

void cast_life_tap(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_tap(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in _life_tap.");
        break;
    }
}

void cast_suit_of_bone(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_suit_of_bone(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in suit_of_bone.");
        break;
    }
}

void cast_spectral_shield(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_spectral_shield(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in spectral_shield.");
        break;
    }
}

void cast_clinging_darkness(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_clinging_darkness(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in clinging_darkness.");
        break;
    }
}

void cast_dominate_undead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dominate_undead(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in dominate_undead.");
        break;
    }
}

void cast_unsummon(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_unsummon(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in unsummon.");
        break;
    }
}

void cast_siphon_strength(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_siphon_strength(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in siphon_strength.");
        break;
    }
}

void cast_gather_shadows(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_gather_shadows(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in gather_shadows.");
        break;
    }
}

void cast_mend_bones(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_mend_bones(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in mend_bones.");
        break;
    }
}

void cast_trace_corpse(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_trace_corpse(level, ch, 0, arg);
        break;
    default:
        Log("serious screw-up in trace_corpse.");
        break;
    }
}

void cast_endure_cold(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_endure_cold(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in endure_cold.");
        break;
    }
}

void cast_life_draw(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_draw(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in life_draw.");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
