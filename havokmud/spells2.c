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

extern int      ArenaNoPets;

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

/*
 * I think this is the highest level you get a new poly, but I ain't sure..
 */

#define MAX_DRUID_POLY 16       /* max number of Druid polies */
#define FG_TEMPLATE 6

struct PolyType DruidList[MAX_DRUID_POLY] = {
    {"bear", 10, 9024},
    {"spider", 10, 20010},
    {"lamia", 10, 3648},
    {"lizard", 10, 6822},
    {"bear", 12, 9056},
    {"gator", 12, 9054},
    {"basilisk", 13, 7043},
#if 0
    {"snog", 14, 27008},
#endif
    {"snake", 15, 6517},
    {"spider", 15, 6113},
    {"lizard", 16, 6505},
    {"allosaurus", 18, 21801},
    {"tiger", 28, 9027},
    {"mulichort", 30, 15830},
    {"tiger", 35, 9055},
    {"lion", 35, 13718},
    {"salamander", 35, 25506}
};

#define LAST_DRUID_MOB 16       /* last level you get a new poly type */




void spell_flesh_golem(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct room_data *rp;
    struct char_data *mob;
    int             lev = 1,
                    mlev,
                    mhps,
                    mtohit;

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (!ch) {
        Log("screw up in flesh golem, no caster found");
        return;
    }

    if (!obj) {
        send_to_char("What would you use for a component?\n\r", ch);
        return;
    }

    if (!IS_CORPSE(obj)) {
        send_to_char("That's not a corpse.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FLESH_GOLEM)) {
        send_to_char("You do not yet have the mental reserves to create "
                     "another flesh golem.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There's no room to create a flesh golem.\n\r", ch);
        return;
    }

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to animate "
                     "corpses!\n\r", ch);
        return;
    }

    act("$n stares at $p while phrasing some arcane incantations.", TRUE,
        ch, obj, 0, TO_ROOM);
    act("You stare at $p while mumbling some arcane incantations.", TRUE,
        ch, obj, 0, TO_CHAR);
    lev = GetMaxLevel(ch);

    mob = read_mobile(FG_TEMPLATE, VIRTUAL);
    if (!mob) {
        Log("No template found for flesh golem spell (no mobile with vnum 6)");
        send_to_char("Screw up in this spell, no flesh golem template "
                     "found\n\r", ch);
        return;
    }

    if (lev > 45) {
        mlev = number(30, 40);
        mhps = number(140, 200);
        mtohit = 8;
    } else if (lev > 37) {
        mlev = number(25, 35);
        mhps = number(90, 120);
        mtohit = 6;
    } else if (lev > 29) {
        mlev = number(20, 29);
        mhps = number(30, 60);
        mtohit = 4;
    } else if (lev > 21) {
        mlev = number(14, 20);
        mhps = number(10, 40);
        mtohit = 2;
    } else {
        mlev = number(4, 6);
        mhps = number(0, 20);
        mtohit = 0;
    }

    mob->player.level[2] = mlev;
    mob->points.max_hit = mob->points.max_hit + mhps;
    mob->points.hit = mob->points.max_hit;
    mob->points.hitroll = mob->points.hitroll + mtohit;
    char_to_room(mob, ch->in_room);
    extract_obj(obj);

    act("The corpse starts stirring, and rises as $n.", FALSE, mob, obj, 0,
        TO_ROOM);

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, mob, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, 0, mob, TO_NOTVICT);
        act("You take one look at the size of $n's posse and just say no!",
            TRUE, ch, 0, mob, TO_VICT);
    } else {
        /*
         * charm it for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);
        af.type = SPELL_CHARM_PERSON;
        if (IS_PC(ch) || ch->master) {
            af.duration = GET_CHR(ch);
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }
    }
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
    }

    af.type = SPELL_FLESH_GOLEM;
    af.duration = 12;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);
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



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */


