
/*
 ***     DaleMUD
 ***     PSI skills
 */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include "protos.h"

extern char    *dirs[];

void mind_use_burn(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_burn(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in mind_burn");
        break;
    }
}

void mind_use_teleport(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_teleport(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_teleport");
        break;
    }
}

void mind_use_probability_travel(int level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *victim,
                                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_probability_travel(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_probability_travel");
        break;
    }
}

void mind_use_danger_sense(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_danger_sense(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_danger_sense");
        break;
    }
}

void mind_use_clairvoyance(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_clairvoyance(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_clairvoyance");
        break;
    }
}

void mind_use_disintegrate(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_disintegrate(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_disintegrate");
        break;
    }
}

void mind_use_telekinesis(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *victim,
                          struct obj_data *tar_obj)
{
    char           *p;
    int             i = -1;

    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        if (!ch->specials.fighting) {
            /*
             * get the argument, parse it into a direction 
             */
            for (; *arg == ' '; arg++) {
                /* 
                 * Empty loop 
                 */
            }

            if (!*arg) {
                mind_telekinesis(level, ch, victim, i);
                return;
            }

            p = fname(arg);
            for (i = 0; i < 6; i++) {
                if (strncmp(p, dirs[i], strlen(p)) == 0) {
                    i++;
                    break;
                }
            }

            if (i == 6) {
                send_to_char("You must supply a direction!\n\r", ch);
                return;
            }
        }
        mind_telekinesis(level, ch, victim, i);
        break;
    default:
        Log("Serious screw-up in mind_telekinesis");
        break;
    }
}

void mind_use_levitation(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_levitation(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_levitation");
        break;
    }
}

void mind_use_cell_adjustment(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *victim,
                              struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_cell_adjustment(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_cell_adjustment");
        break;
    }
}

void mind_use_chameleon(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_chameleon(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_chameleon");
        break;
    }
}

void mind_use_psi_strength(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psi_strength(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psi_strength");
        break;
    }
}

void mind_use_mind_over_body(int level, struct char_data *ch, char *arg,
                             int type, struct char_data *victim,
                             struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_over_body(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mind_over_body");
        break;
    }
}


void mind_use_mind_wipe(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_wipe(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_wipe");
        break;
    }
}

void mind_use_psychic_crush(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *victim,
                            struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psychic_crush(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psyic_crush");
        break;
    }
}

void mind_use_tower_iron_will(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *victim,
                              struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_tower_iron_will(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_tower_iron_will");
        break;
    }
}

void mind_use_mindblank(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mindblank(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mindblank");
        break;
    }
}

void mind_use_psychic_impersonation(int level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *victim,
                                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psychic_impersonation(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psychic_impersonation");
        break;
    }
}

void mind_use_ultra_blast(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *victim,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_ultra_blast(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_ultra_blast");
        break;
    }
}


void mind_use_mind_tap(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_tap(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mind_tap");
        break;
    }
}

void mind_use_kinolock(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_kinolock(level, ch, arg, 0, tar_ch, tar_obj);
        break;
    default:
        Log("Serious screw-up in mind_kinolock");
        break;
    }
}

void mind_use_sense_object(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_sense_object(level, ch, NULL, arg);
        break;
    default:
        Log("Serious screw-up in mind_sense_object");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
