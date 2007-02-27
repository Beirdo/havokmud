/*
 * HavokMUD - druid skills and spells
 */
#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

#define LITTLE_ROCK 50
void spell_animate_rock(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *mob;
    struct affected_type af;
    int             mobn = LITTLE_ROCK;

    if (ITEM_TYPE(obj) != ITEM_TYPE_ROCK) {
        send_to_char("Thats not the right kind of rock\n\r", ch);
        return;
    }

    if (GET_OBJ_WEIGHT(obj) > 20) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 40) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 80) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 160) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 320) {
        mobn++;
    }
    mob = read_mobile(mobn, VIRTUAL);
    if (mob) {
        char_to_room(mob, ch->in_room);
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);

        af.type = SPELL_ANIMATE_ROCK;
        af.duration = 24;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(mob, &af);

        af.type = SPELL_CHARM_PERSON;

        if (IS_PC(ch) || ch->master) {
            af.duration = 24;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }

        if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
            REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
        }
        if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
            SET_BIT(mob->specials.act, ACT_SENTINEL);
        }

        extract_obj(obj);
    } else {
        send_to_char("Sorry, the spell isn't working today\n\r", ch);
        return;
    }
}

void cast_animate_rock(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_animate_rock(level, ch, 0, tar_obj);
        break;
    default:
        Log("serious screw-up in animate_rock.");
        break;
    }
}

void spell_barkskin(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (!affected_by_spell(victim, SPELL_BARKSKIN)) {
        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = -10;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = -1;
        af.location = APPLY_SAVE_ALL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = 1;
        af.location = APPLY_SAVING_SPELL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        send_to_char("Your skin takes on a rough, bark-like texture.\n\r",
                     victim);
        act("$n's skin takes on a rough, bark-like texture", FALSE, ch, 0,
            0, TO_ROOM);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void cast_barkskin(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_barkskin(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_barkskin(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in barkskin.");
        break;
    }
}

#if 0
void spell_chain_lightn(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             lev = level,
                    dam;
    struct char_data *t,
                   *next;
    int             count = 0;

    damage(ch, victim, dice(lev, 6), SPELL_LIGHTNING_BOLT);
    lev--;
    count++;

    for (t = real_roomp(ch->in_room)->people; t; t = next) {
        next = t->next_in_room;
        if (!in_group(ch, t) && t != victim && !IS_IMMORTAL(t)) {
            if (count > 8) {
                break;
            }
            dam = dice(lev, 6);
            if (saves_spell(t, SAVING_SPELL)) {
                dam >>= 1;
            }
            damage(ch, t, dam, SPELL_LIGHTNING_BOLT);
            lev--;
            count++;
        }
    }
}


void cast_chain_lightn(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_POTION:
        spell_chain_lightn(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_chain_lightn(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in chain lightn.");
        break;
    }
}
#endif

#define MAX_DRUID_POLY 16
#define LAST_DRUID_MOB 16
struct PolyType DruidList[MAX_DRUID_POLY] = {
    {"bear", 10, 9024},
    {"spider", 10, 20010},
    {"lamia", 10, 3648},
    {"lizard", 10, 6822},
    {"bear", 12, 9056},
    {"gator", 12, 9054},
    {"basilisk", 13, 7043},
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

void cast_change_form(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    char           *buffer;
    int             mobn = 0,
                    X = LAST_DRUID_MOB,
                    found = FALSE;
    struct char_data *mob;
    struct affected_type af;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char( "Change form to what?\n\r", ch );
        return;
    }

    if (IS_NPC(ch)) {
        send_to_char("You don't really want to do that.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_CHANGE_FORM)) {
        send_to_char("You can only change form once every 12 hours\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
        while (!found) {
            if (DruidList[X].level > level) {
                X--;
            } else {
                if (!strcasecmp(DruidList[X].name, buffer)) {
                    mobn = DruidList[X].number;
                    found = TRUE;
                } else {
                    X--;
                }
                if (X < 0) {
                    break;
                }
            }
        }

        if (!found) {
            send_to_char("Couldn't find any of those\n\r", ch);
            return;
        } else {
            mob = read_mobile(mobn, VIRTUAL);
            if (mob) {
                af.type = SPELL_CHANGE_FORM;
                af.duration = 12;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = 0;

                affect_to_char(ch, &af);

                spell_poly_self(level, ch, mob, 0);
            } else {
                send_to_char("You couldn't summon an image of that "
                             "creature\n\r", ch);
            }
        }
        break;

    default:
        Log("Problem in change form");
        break;
    }
}

#define TREE 6110
void spell_changestaff(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct obj_data *s;
    struct char_data *t;

    if (!ch->equipment[HOLD]) {
        send_to_char("You must be holding a staff!\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    s = unequip_char(ch, HOLD);
    if (ITEM_TYPE(s) != ITEM_TYPE_STAFF) {
        act("$p is not sufficient to complete this spell",
            FALSE, ch, s, 0, TO_CHAR);
        extract_obj(s);
        return;
    }

    if (!s->value[2]) {
        act("$p is not sufficiently powerful to complete this spell",
            FALSE, ch, s, 0, TO_CHAR);
        extract_obj(s);
        return;
    }

    act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_ROOM);
    act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_CHAR);

    t = read_mobile(TREE, VIRTUAL);
    char_to_room(t, ch->in_room);
    GET_EXP(t) = 0;

    act("$n springs up in front of you!", FALSE, t, 0, 0, TO_ROOM);

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
    } else {
        af.type = SPELL_CHARM_PERSON;

        if (IS_PC(ch) || ch->master) {
            af.duration = follow_time(ch);
            af.duration += s->value[2];
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(t, &af);
        } else {
            SET_BIT(t->specials.affected_by, AFF_CHARM);
        }
        add_follower(t, ch);

        extract_obj(s);
    }
}

void cast_changestaff(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }

    if (!ch->equipment[HOLD]) {
        send_to_char(" You must be holding a staff!\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_changestaff(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in changestaff.");
        break;
    }
}
#if 0
void spell_charm_monster(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (IsVeggie(victim)) {
        send_to_char("You can't charm a plant-creature!\n\r", ch);
        return;
    }

    if (IsPerson(victim)) {
        send_to_char("You can't charm people! Try charm person for that!\n\r",
                     ch);
        return;
    }

    if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
        FailCharm(victim, ch);
        return;
    }

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, 0, victim, TO_ROOM);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r",
                         ch);
            return;
        }

        if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
            FailCharm(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_CHARM)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }

            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }
        } else if (!IsSusc(victim, IMM_CHARM) &&
                   saves_spell(victim, SAVING_PARA)) {
            FailCharm(victim, ch);
            return;
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;

        if (GET_CHR(ch)) {
            af.duration = follow_time(ch);
        } else {
            af.duration = 24 * 18;
        }
        if (IS_GOOD(victim) && IS_GOOD(ch)) {
            af.duration *= 2;
        }
        if (IS_EVIL(victim) && IS_EVIL(ch)) {
            af.duration += af.duration >> 1;
        }
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);

        if (!IS_PC(ch)) {
            REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
            SET_BIT(victim->specials.act, ACT_SENTINEL);
        }
    }
}

void cast_charm_monster(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
        break;
    case SPELL_TYPE_SPELL:
        spell_charm_monster(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            return;
        }
        spell_charm_monster(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_charm_monster(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in charm monster!");
        break;
    }
}
#endif
void spell_charm_veggie(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (!IsVeggie(victim)) {
        send_to_char("This can only be used on plants!\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) > GetMaxLevel(ch) + 10) {
        FailCharm(victim, ch);
        return;
    }

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r",
                         ch);
            return;
        }

        if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
            FailCharm(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_CHARM)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }

            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }
        } else {
            if (!IsSusc(victim, IMM_CHARM)) {
                if (saves_spell(victim, SAVING_PARA)) {
                    FailCharm(victim, ch);
                    return;
                }
            }
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);
        af.type = SPELL_CHARM_PERSON;

        if (GET_INT(victim)) {
            af.duration = 24 * GET_CHR(ch) / GET_INT(victim);
        } else {
            af.duration = 24 * 18;
        }
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    }
}

void cast_charm_veggie(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_charm_veggie(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            return;
        }
        spell_charm_veggie(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_charm_veggie(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in charm veggie!");
        break;
    }
}

#define CREEPING_DEATH 39
void spell_creeping_death(int level, struct char_data *ch,
                          struct char_data *victim, int dir)
{
    struct affected_type af;
    struct char_data *cd;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS) ||
        IS_SET(real_roomp(ch->in_room)->sector_type, SECT_INSIDE)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }
    /*
     * obj is really the direction that the death wishes to travel in
     */

    cd = read_mobile(CREEPING_DEATH, VIRTUAL);
    if (!cd) {
        send_to_char("None available\n\r", ch);
        return;
    }

    char_to_room(cd, ch->in_room);
    cd->points.max_hit += (number(1, 4) * 100) + 600;
    cd->points.hit = cd->points.max_hit;

    act("$n makes a horrid coughing sound.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You feel an incredibly nasty feeling inside.\n\r", ch);
    act("A huge gout of poisonous insects spews forth from $n's mouth!",
        FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("A huge gout of insects spews out of your mouth!\n\r", ch);
    act("The insects coalesce into a solid mass - the creeping death.",
        FALSE, ch, 0, 0, TO_ROOM);
    cd->generic = dir;
    /*
     * move the creeping death in the proper direction
     */
    do_move(cd, "\0", dir);
    act("$n slumps to the ground, exhausted.", FALSE, ch, 0, 0, TO_ROOM);
    act("You are overcome by a wave of exhaustion.", FALSE, ch, 0, 0, TO_CHAR);

    if (!IS_IMMORTAL(ch)) {
        GET_POS(ch) = POSITION_STUNNED;
        af.type = SPELL_CREEPING_DEATH;
        af.duration = 2;
        af.modifier = 10500;
        af.location = APPLY_SPELLFAIL;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }
}

void cast_creeping_death(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    char           *p;
    int             i;

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        /*
         * get the argument, parse it into a direction 
         */
        arg = skip_spaces(arg);
        if (!arg) {
            send_to_char("you must supply a direction!\n\r", ch);
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
            send_to_char("you must supply a direction!\n\r", ch);
            return;
        }
        spell_creeping_death(level, ch, 0, i);
        break;

    default:
        Log("serious screw-up in creeping_death.");
        break;
    }
}

#define FIRE_ELEMENTAL 40
void spell_elemental_summoning(int level, struct char_data *ch,
                               struct char_data *victim, int spell)
{
    int             vnum;
    struct char_data *mob;
    struct affected_type af;

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }

    if (affected_by_spell(ch, spell)) {
        send_to_char("You can only do this once per 24 hours\n\r", ch);
        return;
    }

    vnum = spell - SPELL_FIRE_SERVANT;
    vnum += FIRE_ELEMENTAL;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    mob = read_mobile(vnum, VIRTUAL);

    if (!mob) {
        send_to_char("None available\n\r", ch);
        return;
    }

    act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
    act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

    char_to_room(mob, ch->in_room);
    act("$n appears through a momentary rift in the ether!",
        FALSE, mob, 0, 0, TO_ROOM);
    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd!!'",
            TRUE, ch, 0, mob, TO_ROOM);
        act("$N refuses to hang out with crowd of your size!!", TRUE, ch,
            0, mob, TO_CHAR);
    } else {
        /*
         * charm them for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);

        af.type = SPELL_CHARM_PERSON;

        if (IS_PC(ch) || ch->master) {
            af.duration = 24;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }
    }

    af.type = spell;
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    /*
     * adjust the bits...
     * get rid of aggressive, add sentinel
     */

    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
    }
}

void cast_fire_servant(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_FIRE_SERVANT);
        break;
    default:
        Log("serious screw-up in fire servant.");
        break;
    }
}

void cast_earth_servant(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_EARTH_SERVANT);
        break;
    default:
        Log("serious screw-up in earth servant.");
        break;
    }
}

void cast_water_servant(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_WATER_SERVANT);
        break;
    default:
        Log("serious screw-up in water servant.");
        break;
    }
}

void cast_wind_servant(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_WIND_SERVANT);
        break;
    default:
        Log("serious screw-up in wind servant.");
        break;
    }
}

void spell_entangle(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
        send_to_char("You must cast this spell in the forest!\n\r", ch);
        return;
    }

    if (IsImmune(victim, IMM_HOLD)) {
        FailSnare(victim, ch);
        return;
    }

    if (IsResist(victim, IMM_HOLD) && saves_spell(victim, SAVING_PARA)) {
        FailSnare(victim, ch);
        return;
    }

    /*
     * if victim fails save, paralyzed for a very short time
     */
    if (saves_spell(victim, SAVING_PARA)) {
        if (IsSusc(victim, IMM_HOLD)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailSnare(victim, ch);
                return;
            }
        } else {
            FailSnare(victim, ch);
            return;
        }
    } else {
        act("Roots and vines entwine around you!", FALSE, victim, 0, 0,
            TO_CHAR);
        act("Roots and vines surround $n!", FALSE, victim, 0, 0, TO_ROOM);

        af.type = SPELL_ENTANGLE;
        af.duration = 1;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_PARALYSIS;
        affect_to_char(victim, &af);
    }
}

void cast_entangle(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_entangle(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch) && !IS_IMMORTAL(tar_ch)) {
                spell_entangle(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in entangle.");
        break;
    }
}

#if 0
void spell_fireshield(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_FIRESHIELD)) {
        if (affected_by_spell(victim, SPELL_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, victim,
                0, 0, TO_ROOM);
            act("The heat of your spell melts the icey aura surrounding you.",
                TRUE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_CHILLSHIELD);
        }
        if (IS_AFFECTED(victim, AFF_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, victim,
                0, 0, TO_ROOM);
            act("The heat of your spell melts the icey aura surrounding you.",
                TRUE, victim, 0, 0, TO_CHAR);
            REMOVE_BIT(victim->specials.affected_by, AFF_CHILLSHIELD);
        }
        if (affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
            act("The whirling blades around $n burn to cinders.", TRUE,
                victim, 0, 0, TO_ROOM);
            act("The heat of your spell burns your blade barrier to cinders.",
                TRUE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_BLADE_BARRIER);
        }
        if (IS_AFFECTED(victim, AFF_BLADE_BARRIER)) {
            act("The whirling blades around $n burn to cinders.", TRUE,
                victim, 0, 0, TO_ROOM);
            act("The heat of your spell burns your blade barrier to cinders.",
                TRUE, victim, 0, 0, TO_CHAR);
            REMOVE_BIT(victim->specials.affected_by, AFF_BLADE_BARRIER);
        }

        act("$c000R$n is surrounded by a glowing red aura.", TRUE, victim,
            0, 0, TO_ROOM);
        act("$c000RYou start glowing red.", TRUE, victim, 0, 0, TO_CHAR);

        af.type = SPELL_FIRESHIELD;
        af.duration = (!IS_IMMORTAL(ch) ? 3 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_FIRESHIELD;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", victim);
    }
}

void cast_fireshield(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_fireshield(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
        spell_fireshield(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_fireshield(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_fireshield(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in fireshield!");
        break;
    }
}
#endif

void spell_firestorm(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + level + 1;

    send_to_char("Searing flame surround you!\n\r", ch);
    act("$n sends a firestorm whirling across the room!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        rdam = dam;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }

            if (!in_group(ch, tmp_victim)) {
                act("You are seared by the burning flame!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    BurnWings(tmp_victim);
                }
                heat_blind(tmp_victim);
                MissileDamage(ch, tmp_victim, rdam, SPELL_BURNING_HANDS);
            } else {
                act("You are able to avoid the flames!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        }
    }
}

void cast_firestorm(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_firestorm(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in firestorm.");
        break;
    }
}

void spell_flame_blade(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (ch->equipment[WIELD]) {
        send_to_char("You can't be wielding a weapon\n\r", ch);
        return;
    }

    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);
    StringToKeywords( "blade flame", &tmp_obj->keywords );
    tmp_obj->short_description = strdup("a flame blade");
    tmp_obj->description = strdup("A flame blade burns brightly here.");
    tmp_obj->type_flag = ITEM_TYPE_WEAPON;
    tmp_obj->wear_flags = ITEM_TAKE | ITEM_WIELD;
    tmp_obj->value[0] = 0;
    tmp_obj->value[1] = 1;
    tmp_obj->value[2] = 4;
    tmp_obj->value[3] = 3;
    tmp_obj->weight = 1;
    tmp_obj->cost = 10;
    tmp_obj->cost_per_day = 1;
    SET_BIT(tmp_obj->extra_flags, ITEM_MAGIC);
    tmp_obj->affected[0].location = APPLY_DAMROLL;
    tmp_obj->affected[0].modifier = 4 + GET_LEVEL(ch, DRUID_LEVEL_IND) / 8;
    tmp_obj->next = object_list;
    object_list = tmp_obj;
    equip_char(ch, tmp_obj, WIELD);
    tmp_obj->item_number = -1;

    act("$p appears in your hand.", TRUE, ch, tmp_obj, 0, TO_CHAR);
    act("$p appears in $n's hand.", TRUE, ch, tmp_obj, 0, TO_ROOM);
}

void cast_flame_blade(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_flame_blade(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in flame blade.");
        break;
    }
}

void spell_heat_stuff(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             j = 0;

    assert(victim);

    if (IS_SET(victim->specials.affected_by2, AFF2_HEAT_STUFF)) {
        send_to_char("Already affected\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(victim) && IS_PC(victim)) {
        act("Your magic rebounds from the divine aura surrounding $N!",
            FALSE, ch, 0, victim, TO_CHAR);
        act("$n sends a heat spell towards $N, but it rebounds off $S divine "
            "aura!", FALSE, ch, 0, victim, TO_NOTVICT);
        send_to_char("The heat spell aimed at you is rebounded by your divine"
                     " aura.\n\r", victim);
        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = -2;
        af.location = APPLY_DEX;
        af.bitvector = 0;

        affect_to_char(ch, &af);

        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_HEAT_STUFF;

        affect_to_char(ch, &af);

        if (IS_PC(ch)) {
            /*
             * all metal flagged equips zap off!
             */
            for (j = 0; j < MAX_WEAR; j++) {
                if (ch->equipment[j]) {
                    obj = ch->equipment[j];
                    if (IS_OBJ_STAT(obj, extra_flags, ITEM_METAL)) {
                        act("$p glows brightly from the heat, and you quickly"
                            " let go of it!", FALSE, ch, obj, 0, TO_CHAR);
                        act("$p turns so hot that $n is forced to let go of "
                            "it!", FALSE, ch, obj, 0, TO_ROOM);
                        if ((obj = unequip_char(ch, j)) != NULL) {
                            obj_to_room(obj, ch->in_room);
                        }
                    }
                }
            }
        }
        return;
    }

    if (HitOrMiss(ch, victim, CalcThaco(ch))) {
        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = -2;
        af.location = APPLY_DEX;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_HEAT_STUFF;

        affect_to_char(victim, &af);
        send_to_char("Your armor starts to sizzle and smoke.\n\r", victim);
        act("$N's armor starts to sizzle.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N's armor starts to sizzle.", FALSE, ch, 0, victim, TO_NOTVICT);

        if (IS_PC(victim)) {
            /*
             * all metal flagged equips zap off!
             */
            for (j = 0; j < MAX_WEAR; j++) {
                if (victim->equipment[j]) {
                    obj = victim->equipment[j];
                    if (IS_OBJ_STAT(obj, extra_flags, ITEM_METAL)) {
                        act("$p glows brightly from the heat, and you quickly"
                            " let go of it!", FALSE, victim, obj, 0, TO_CHAR);
                        act("$p turns so hot that $n is forced to let go of "
                            "it!", FALSE, victim, obj, 0, TO_ROOM);
                        if ((obj = unequip_char(victim, j)) != NULL) {
                            obj_to_room(obj, victim->in_room);
                        }
                    }
                }
            }
        } else {
            if (!victim->specials.fighting) {
                set_fighting(victim, ch);
            }
        }
    }
}

void cast_heat_stuff(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_heat_stuff(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in heat_stuff.");
        break;
    }
}

void spell_insect_growth(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (GET_RACE(victim) != RACE_INSECT) {
        send_to_char("Thats not an insect.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_INSECT_GROWTH)) {
        act("$N is already affected by that spell", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
        send_to_char("You can't make it more powerful than you!\n\r", ch);
        return;
    }

    if (IS_PC(victim)) {
        send_to_char("It would be in bad taste to cast that on a player\n\r",
                     ch);
        return;
    }

    act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
    act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = GET_MAX_HIT(victim);
    af.location = APPLY_HIT;
    af.bitvector = AFF_GROWTH;
    affect_to_char(victim, &af);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = 5;
    af.location = APPLY_HITNDAM;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = 3;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    /*
     * GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
     */
}

void cast_insect_growth(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_insect_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in insect_growth.");
        break;
    }
}



void spell_iron_skins(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    /*
     * put up some iron skinned barriers
     * that will absorb (duration) hps.
     */
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_IRON_SKINS)) {
        act("$n invokes the earthen forces, and surrounds $mself with iron "
            "skins.", TRUE, ch, 0, 0, TO_ROOM);
        act("You invoke the earthen forces, and surround yourself with iron "
            "skins.", TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_IRON_SKINS;
        af.duration = (level * 3) / 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void cast_iron_skins(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_iron_skins(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_iron_skins");
        break;
    }
}

void spell_plant_gate(int level, struct char_data *ch,
                      struct char_data *victim, char *arg)
{
    struct room_data *rp;
    struct obj_data *o,
                   *i,
                   *obj = NULL;
    struct char_data *tch,
                   *tch2;
    int             has_companions = 0;
    Keywords_t     *key;

    /*
     * find the tree in the room
     */

    rp = real_roomp(ch->in_room);
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_TYPE_TREE) {
            break;
        }
    }

    if (!o) {
        send_to_char("You need to have a tree nearby.\n\r", ch);
        return;
    }

    key = StringToKeywords( arg, NULL );

    /*
     * find the target tree
     */
    for (i = object_list; i; i = i->next) {
        if (ITEM_TYPE(i) == ITEM_TYPE_TREE && 
            KeywordsMatch(key, &i->keywords)) {
            /*
             * we found a druid tree with the right name
             */
            obj = i;
            break;
        }
    }
    FreeKeywords(key, TRUE);

    if (!obj) {
        send_to_char("You can not sense a tree by that name.\n\r", ch);
        return;
    }

    if (obj->in_room < 0) {
        send_to_char("That tree is nowhere to be found!\n\r", ch);
        return;
    }

    if (!real_roomp(obj->in_room)) {
        send_to_char("That tree is nowhere to be found!\n\r", ch);
        return;
    }

    act("$n places $s hand on $p and its surface wavers as a large gate opens "
        "within it!", FALSE, ch, o, 0, TO_ROOM);
    act("You place a hand on $p and its surface wavers as a large gate opens "
        "within it!", FALSE, ch, o, 0, TO_CHAR);

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch2) {
        tch2 = tch->next_in_room;
        if (in_group(tch, ch) && GET_POS(tch) > POSITION_SLEEPING &&
            tch != ch) {
            act("$n holds open the magical gateway in $p and ushers you "
                "through it.", FALSE, ch, o, tch, TO_VICT);
            act("$N steps inside the magical gate in $p and vanishes!",
                FALSE, ch, o, tch, TO_NOTVICT);
            act("You hold open the plant gate as $N steps inside and vanishes!",
                FALSE, ch, o, tch, TO_CHAR);
            char_from_room(tch);
            char_to_room(tch, obj->in_room);
            if (!has_companions) {
                act("$p rustles and a large gate opens within it!", FALSE,
                    tch, obj, 0, TO_ROOM);
            }
            act("$n suddenly appears as $e steps from the gate in $p!",
                FALSE, tch, obj, 0, TO_ROOM);
            act("You are instantly transported to $p!", FALSE, tch, obj, 0,
                TO_CHAR);
            do_look(tch, NULL, 0);
            has_companions++;
        }
    }

    if (has_companions) {
        send_to_char("After your companions are safely through the gate, "
                     "you step within and join forms!\n\r", ch);
    } else {
        send_to_char("You step within the gate and your forms merge!\n\r", ch);
    }

    act("$n steps within the magical gate in $p and vanishes just before it"
        " closes!", FALSE, ch, o, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, obj->in_room);

    if (!has_companions) {
        act("$p rustles and a large gate opens within it!",
            FALSE, ch, obj, 0, TO_ROOM);
    }

    act("You are instantly transported to $p!", FALSE, ch, obj, 0, TO_CHAR);
    do_look(ch, NULL, 0);
    act("$n steps through the magical gate in $p and the gate closes "
        "behind $m!", FALSE, ch, obj, 0, TO_ROOM);
}

void cast_plant_gate(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!arg || !*arg) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_plant_gate(level, ch, 0, arg);
        break;
    default:
        Log("Serious screw-up in plant gate");
        break;
    }
}

void spell_reincarnate(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct char_data *newch;
    struct char_file_u st;
    struct descriptor_data *d;
    FILE           *fl;
    int             origage;
    int             origrace;
    int             newage;
    int             newrace;
    struct time_info_data my_age;

    if (!obj) {
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }
    if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
        send_to_char("You must cast this spell in the forest!\n\r", ch);
        return;
    }
    if (IS_CORPSE(obj)) {
        if (obj->char_vnum) {
            send_to_char("This spell only works on players\n\r", ch);
            return;
        }
        if (obj->char_f_pos) {
            fl = fopen(PLAYER_FILE, "r+");
            if (!fl) {
                perror("player file");
                assert(0);
            }
            rewind(fl);
            fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
            fread(&st, sizeof(struct char_file_u), 1, fl);
            /*
             * this is a serious kludge, and must be changed before multiple
             * languages can be implemented
             */
            if (st.talks[2] && st.abilities.con > 3) {
                st.points.exp *= 2;
                st.points.leadership_exp *= 2;
                st.talks[2] = TRUE;
                st.m_deaths--;
                st.abilities.con -= 1;

                act("The forest comes alive with the sounds of birds and "
                    "animals", TRUE, ch, 0, 0, TO_CHAR);
                act("The forest comes alive with the sounds of birds and "
                    "animals", TRUE, ch, 0, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_CHAR);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                GET_MANA(ch) = 1;
                GET_MOVE(ch) = 1;
                GET_HIT(ch) = 1;
                GET_POS(ch) = POSITION_SITTING;
                act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
                send_to_char("You collapse from the effort\n\r", ch);

                rewind(fl);
                fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
                fwrite(&st, sizeof(struct char_file_u), 1, fl);
                ObjFromCorpse(obj);

                CREATE(newch, struct char_data, 1);
                clear_char(newch);

                origage = GET_AGE(newch);
                origrace = GET_RACE(newch);

                st.race = GetNewRace(&st);

                newrace = GET_RACE(newch);

                store_to_char(&st, newch);

                reset_char(newch);

                newch->next = character_list;
                character_list = newch;

                char_to_room(newch, ch->in_room);
                newch->invis_level = 51;

                set_title(newch);
                GET_HIT(newch) = 1;
                GET_MANA(newch) = 1;
                GET_MOVE(newch) = 1;
                GET_POS(newch) = POSITION_SITTING;

                newage = race_list[newrace].start;
                newrace = GET_RACE(newch);
                age2(newch, &my_age);

                newch->player.time.birth -=
                    my_age.year * SECS_PER_MUD_YEAR * (-1);
                newch->player.time.birth -=
                    SECS_PER_MUD_YEAR * race_list[newrace].start;

                save_char(newch, AUTO_RENT);

                /*
                 * if they are in the descriptor list, suck them into the
                 * game
                 */

                for (d = descriptor_list; d; d = d->next) {
                    if (d->character && (strcmp(GET_NAME(d->character),
                                                GET_NAME(newch)) == 0)) {
                        if (STATE(d) != CON_PLYNG) {
                            free_char(d->character);
                            d->character = newch;
                            STATE(d) = CON_PLYNG;
                            newch->desc = d;
                            send_to_char("You awake to find yourself "
                                         "changed\n\r", newch);
                            break;
                        }
                    }
                }
            } else {
                send_to_char("The spirit does not have the strength to be "
                             "reincarnated\n\r", ch);
            }
            fclose(fl);
        }
    }
}

void cast_reincarnate(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{

    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        if (!tar_obj) {
            return;
        }
        spell_reincarnate(level, ch, 0, tar_obj);
        break;
    case SPELL_TYPE_STAFF:
        if (!tar_obj) {
            return;
        }
        spell_reincarnate(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious problem in 'reincarnate'");
        break;
    }
}

/**
 * @todo change to use the weapon type, not keywords to find clubs
 */
void spell_shillelagh(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             i;
    int             count = 0;
    Keywords_t     *key;

    if(!ch || !obj || MAX_OBJ_AFFECT < 2) {
        return;
    }

    if ((ITEM_TYPE(obj) == ITEM_TYPE_WEAPON) &&
        !IS_OBJ_STAT(obj, extra_flags, ITEM_MAGIC)) {

        key = StringToKeywords( "club", NULL );
        if (!KeywordsMatch(key, &obj->keywords)) {
            send_to_char("That isn't a club!\n\r", ch);
            FreeKeywords(key, TRUE);
            return;
        }
        FreeKeywords(key, TRUE);

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location == APPLY_NONE) {
                count++;
            }
            if (obj->affected[i].location == APPLY_HITNDAM ||
                obj->affected[i].location == APPLY_HITROLL ||
                obj->affected[i].location == APPLY_DAMROLL)
                return;
        }

        if (count < 2) {
            return;
        }
        /*
         * find the slots
         */
        i = getFreeAffSlot(obj);
        SET_BIT(obj->extra_flags, ITEM_MAGIC);
        obj->affected[i].location = APPLY_HITNDAM;
        obj->affected[i].modifier = 1;
        obj->value[1] = 2;
        obj->value[2] = 4;
        act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
        SET_BIT(obj->anti_flags, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
    }
}

void cast_shillelagh(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_shillelagh(level, ch, ch, tar_obj);
        break;
    default:
        Log("serious screw-up in shillelagh.");
        break;
    }
}



void spell_speak_with_plants(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    char            buffer[128];
    char           *temp;

    assert(ch && obj);

    if (ITEM_TYPE(obj) != ITEM_TYPE_TREE) {
        send_to_char("Sorry, you can't talk to that sort of thing\n\r", ch);
        return;
    }

    temp = KeywordsToString( &obj->keywords, " " );
    sprintf(buffer, "%s says 'Hi $n, how ya doin?'", temp);
    free( temp );

    act(buffer, FALSE, ch, obj, 0, TO_CHAR);
    act("$p rustles slightly.", FALSE, ch, obj, 0, TO_ROOM);
}

void cast_speak_with_plants(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    if (!tar_obj) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_speak_with_plants(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in speak_with_plants");
        break;
    }
}

void spell_sunray(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct char_data *t,
                   *n;
    int             dam = 0,
                    j = 0;

    /*
     * blind all in room
     */
    for (t = real_roomp(ch->in_room)->people; t; t = n) {
        n = t->next_in_room;
        if (!in_group(ch, t) && !IS_IMMORTAL(t)) {
            spell_blindness(level, ch, t, obj);
            /*
             * if not arena, scrap any ANTI-SUN equipment worn by
             * ungroupies
             */
            if (!IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
                for (j = 0; j <= (MAX_WEAR - 1); j++) {
                    if (t->equipment[j] && t->equipment[j]->item_number >= 0 &&
                        IS_OBJ_STAT(t->equipment[j], anti_flags, 
                                    ITEM_ANTI_SUN)) {
                        obj = t->equipment[j];
                        act("$n's sunray strikes your $p, causing it to fall "
                            "apart!", FALSE, ch, 0, 0, TO_VICT);
                        act("$n's sunray strikes $N's $p, causing it to fall "
                            "apart!", FALSE, ch, obj, t, TO_NOTVICT);
                        act("Your sunray strikes $N's $p, causing it to fall "
                            "apart!", FALSE, ch, obj, t, TO_CHAR);
                        MakeScrap(t, 0, obj);
                    }
                }
            }

            /*
             * hit undead target
             */
            if (IsUndead(t) || GET_RACE(t) == RACE_VEGMAN) {
                dam = dice(6, 8);
                if (saves_spell(t, SAVING_SPELL) &&
                    GET_RACE(t) != RACE_VEGMAN) {
                    dam = (t == victim ? dam >> 1 : 0);
                }
                damage(ch, t, dam, SPELL_SUNRAY);
            }
        }
    }
}

void cast_sunray(int level, struct char_data *ch, char *arg,
                 int type, struct char_data *tar_ch,
                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_sunray(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in sunray.");
        break;
    }
}

void spell_transport_via_plant(int level, struct char_data *ch,
                               struct char_data *victim, char *arg)
{
    struct room_data *rp;
    struct obj_data *o;
    struct obj_data *i;
    struct obj_data *obj = NULL;
    int             found = 0;
    Keywords_t     *key;

    /*
     * find the tree in the room
     */
    rp = real_roomp(ch->in_room);
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_TYPE_TREE) {
            break;
        }
    }

    if (!o) {
        send_to_char("You need to have a tree nearby\n\r", ch);
        return;
    }

    key = StringToKeywords( name, NULL );

    /*
     * find the target tree
     */
    for (i = object_list; i; i = i->next) {
        if ( ITEM_TYPE(i) == ITEM_TYPE_TREE && 
             KeywordsMatch(key, &i->keywords) ) {
            /*
             * we found a druid tree with the right name
             */
            obj = i;
            found = 1;
            break;
        }
    }

    FreeKeywords(key, TRUE);

    if (!found) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }

    if (obj->in_room < 0) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }

    if (!real_roomp(obj->in_room)) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }

    act("$n touches $p, and slowly vanishes within!", FALSE, ch, o, 0, TO_ROOM);
    act("You touch $p, and join your forms.", FALSE, ch, o, 0, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, obj->in_room);
    act("$p rustles slightly, and $n magically steps from within!", FALSE,
        ch, obj, 0, TO_ROOM);
    act("You are instantly transported to $p!", FALSE, ch, obj, 0, TO_CHAR);
    do_look(ch, NULL, 0);
}

void cast_transport_via_plant(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj)
{
    if (!arg || !*arg) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_transport_via_plant(level, ch, 0, arg);
        break;
    default:
        Log("Serious screw-up in transport_via_plant");
        break;
    }
}


#define SAPLING 45
void spell_tree(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    struct char_data *mob;
    int             mobn;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    mobn = SAPLING;
    if (level > 20) {
        mobn++;
    }
    if (level > 30) {
        mobn++;
    }
    if (level > 40) {
        mobn++;
    }
    if (level > 48) {
        mobn++;
    }
    mob = read_mobile(mobn, VIRTUAL);
    if (mob) {
        spell_poly_self(level, ch, mob, 0);
    } else {
        send_to_char("You couldn't summon an image of that creature\n\r",
                     ch);
    }
}

void cast_tree(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    if (IS_NPC(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
        spell_tree(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in tree");
        break;
    }
}

void spell_tree_travel(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_TREE_TRAVEL)) {
        af.type = SPELL_TREE_TRAVEL;

        af.duration = 24;
        af.modifier = -5;
        af.location = APPLY_AC;
        af.bitvector = AFF_TREE_TRAVEL;
        affect_to_char(victim, &af);

        send_to_char("You feel as one with the trees... Groovy!\n\r", victim);
    } else {
        send_to_char("Nothing seems to happen\n\r", ch);
    }
}

void cast_tree_travel(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_tree_travel(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in tree travel");
        break;
    }
}

void spell_veggie_growth(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!IsVeggie(victim)) {
        send_to_char("Thats not a plant-creature!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_VEGGIE_GROWTH)) {
        act("$N is already affected by that spell", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
        send_to_char("You can't make it more powerful than you!\n\r", ch);
        return;
    }

    if (IS_PC(victim)) {
        send_to_char("It would be in bad taste to cast that on a player\n\r",
                     ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
    act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = GET_MAX_HIT(victim);
    af.location = APPLY_HIT;
    af.bitvector = AFF_GROWTH;
    affect_to_char(victim, &af);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = 5;
    af.location = APPLY_HITNDAM;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = 3;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2 * GetMaxLevel(victim);
}

void cast_veggie_growth(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_veggie_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in veggie_growth.");
        break;
    }
}

void spell_warp_weapon(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (!obj) {
        if (!victim->equipment[WIELD]) {
            act("$N doesn't have a weapon wielded!", FALSE, ch, 0,
                victim, TO_CHAR);
            return;
        }
        obj = victim->equipment[WIELD];
    }

    act("$p is warped and twisted by the power of the spell", FALSE,
        ch, obj, 0, TO_CHAR);
    act("$p is warped and twisted by the power of the spell", FALSE,
        ch, obj, 0, TO_ROOM);
    DamageOneItem(victim, BLOW_DAMAGE, obj);

    if (!IS_PC(victim) && !victim->specials.fighting) {
        set_fighting(victim, ch);
    }
}

void cast_warp_weapon(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_warp_weapon(level, ch, tar_ch, tar_obj);
        break;
    default:
        Log("serious screw-up in warp_weapon.");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

