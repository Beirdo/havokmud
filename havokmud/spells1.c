#include "config.h"
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

void cast_burning_hands(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_burning_hands(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in burning hands!");
        break;
    }
}

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

void cast_shocking_grasp(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_shocking_grasp(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in shocking grasp!");
        break;
    }
}

void cast_colour_spray(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_colour_spray(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_colour_spray(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_colour_spray(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_colour_spray(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in colour spray!");
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

void cast_energy_drain(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_energy_drain(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_energy_drain(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_energy_drain(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_energy_drain(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_energy_drain(level, ch, victim, 0);
        }
        break;
    case SPELL_TYPE_STAFF:
        for (victim = real_roomp(ch->in_room)->people;
             victim; victim = victim->next_in_room) {
            if (!in_group(ch, victim) && victim != ch) {
                spell_energy_drain(level, ch, victim, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in energy drain!");
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

void cast_fireball(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_fireball(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in fireball");
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

void cast_lightning_bolt(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_lightning_bolt(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_lightning_bolt(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_lightning_bolt(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_lightning_bolt(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in lightning bolt!");
        break;

    }
}

void cast_acid_blast(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_acid_blast(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_acid_blast(level, ch, victim, 0);
        } else {
            spell_acid_blast(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_acid_blast(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in acid blast!");
        break;
    }
}

void cast_cone_of_cold(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_cone_of_cold(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in cone of cold!");
        break;
    }
}

void cast_ice_storm(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_ice_storm(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in ice storm");
        break;
    }
}

void cast_meteor_swarm(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_meteor_swarm(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_meteor_swarm(level, ch, victim, 0);
        } else {
            spell_meteor_swarm(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_meteor_swarm(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in meteor swarm!");
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

void cast_magic_missile(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_magic_missile(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_magic_missile(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_magic_missile(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_magic_missile(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in magic missile!");
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

void cast_incendiary_cloud(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_incendiary_cloud(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in incendiary cloud!");
        break;
    }
}

void cast_prismatic_spray(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *victim,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_prismatic_spray(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in prismatic spray!");
        break;
    }
}

void cast_wizard_eye(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
        spell_wizard_eye(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in cast_wizard_eye!");
        break;
    }
}

void cast_disintegrate(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_disintegrate(level, ch, victim, tar_obj);
        break;
    default:
        Log("Serious screw-up in cast_disintegrate");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
