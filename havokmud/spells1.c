#include "config.h"
#include "platform.h"
#include <stdio.h>

#include "protos.h"

/*
 * Global data 
 */

extern struct room_data *world;
extern struct char_data *character_list;

/*
 * Extern functions 
 */


void cast_call_lightning(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    extern struct weather_data weather_info;

    switch (type) {
    case SPELL_TYPE_SPELL:
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING) {
            spell_call_lightning(level, ch, victim, 0);
        } else {
            send_to_char("You fail to call upon the lightning from the "
                         "sky!\n\r", ch);
        }
        break;
    case SPELL_TYPE_POTION:
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING) {
            spell_call_lightning(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_SCROLL:
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING) {
            if (victim) {
                spell_call_lightning(level, ch, victim, 0);
            } else if (!tar_obj) {
                spell_call_lightning(level, ch, ch, 0);
            }
        }
        break;
    case SPELL_TYPE_STAFF:
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING) {
            for (victim = real_roomp(ch->in_room)->people;
                 victim; victim = victim->next_in_room) {
                if (!in_group(victim, ch)) {
                    spell_call_lightning(level, ch, victim, 0);
                }
            }
        }
        break;
    default:
        Log("Serious screw-up in call lightning!");
        break;
    }
}

void cast_chill_touch(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
        spell_chill_touch(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in chill touch!");
        break;
    }
}



void cast_earthquake(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_earthquake(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in earthquake!");
        break;
    }
}


void cast_energy_restore(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_energy_restore(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_energy_drain(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_energy_drain(level, ch, ch, 0);
        }
        break;
    default:
        Log("Serious screw-up in energy restore!");
        break;
    }
}


void cast_harm(int level, struct char_data *ch, char *arg, int type,
               struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_harm(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_harm(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (victim = real_roomp(ch->in_room)->people;
             victim; victim = victim->next_in_room) {
            if (!in_group(ch, victim)) {
                spell_harm(level, ch, victim, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in harm!");
        break;
    }
}





void cast_flamestrike(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_flamestrike(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_flamestrike(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_flamestrike(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_flamestrike(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in flamestrike!");
        break;
    }
}


void cast_cause_light(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
        spell_cause_light(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cause_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (victim = real_roomp(ch->in_room)->people;
             victim; victim = victim->next_in_room) {
            if (!in_group(ch, victim)) {
                spell_cause_light(level, ch, victim, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cause light wounds!");
        break;
    }
}

void cast_cause_serious(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_cause_serious(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cause_serious(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!victim) {
            victim = ch;
        }
        spell_cause_serious(level, ch, victim, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (victim = real_roomp(ch->in_room)->people;
             victim; victim = victim->next_in_room) {
            if (!in_group(ch, victim)) {
                spell_cause_serious(level, ch, victim, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cause serious wounds!");
        break;
    }
}

void cast_cause_critic(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_cause_critical(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!victim) {
            victim = ch;
        }
        spell_cause_critical(level, ch, victim, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!victim) {
            victim = ch;
        }
        spell_cause_critical(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        if (!victim) {
            victim = ch;
        }
        spell_cause_critical(level, ch, victim, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (victim = real_roomp(ch->in_room)->people;
             victim; victim = victim->next_in_room) {
            if (!in_group(ch, victim)) {
                spell_cause_critical(level, ch, victim, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cause critical!");
        break;
    }
}

void cast_geyser(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_geyser(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in geyser!");
        break;
    }
}

void cast_green_slime(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_green_slime(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_green_slime(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_green_slime(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_green_slime(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in green Slime!");
        break;
    }
}





/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
