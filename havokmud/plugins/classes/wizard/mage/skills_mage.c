/*
 * HavokMUD - all mage spells and skills
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


void spell_armor(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_ARMOR)) {
        af.type = SPELL_ARMOR;
        af.duration = 24;
        af.modifier = -20;
        af.location = APPLY_AC;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        send_to_char("You feel someone protecting you.\n\r", victim);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void cast_armor(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_ARMOR)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        if (ch != tar_ch) {
            act("$N is protected.", FALSE, ch, 0, tar_ch, TO_CHAR);
        }
        spell_armor(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_ARMOR)) {
            return;
        }
        spell_armor(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:    
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (affected_by_spell(tar_ch, SPELL_ARMOR)) {
            return;
        }
        spell_armor(level, ch, ch, 0);
        break;
    default:
        Log("Error in cast_armor()");
        break;
    }
}

void spell_acid_blast(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 6);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_ACID_BLAST);
}

void cast_acid_blast(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_acid_blast(level, ch, victim, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_acid_blast(level, ch, ch, 0);
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:    
        if (victim) {
            spell_acid_blast(level, ch, victim, 0);
        } else {
            spell_acid_blast(level, ch, ch, 0);
        }
        break;
    default:
        Log("Error in cast_acid_blast()");
        break;
    }
}

void spell_blindness(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (saves_spell(victim, SAVING_SPELL)) {
        act("$n seems to avoid the affects of your spell!",
            TRUE, victim, 0, 0, TO_CHAR);
        return;
    }
    if (affected_by_spell(victim, SPELL_BLINDNESS)) {
        act("$n is already blind!", TRUE, victim, 0, 0, TO_CHAR);
        return;
    }
    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);

    af.type = SPELL_BLINDNESS;
    af.location = APPLY_HITROLL;
    af.modifier = -12;
    af.duration = level / 2;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +40;
    affect_to_char(victim, &af);

    if ((!victim->specials.fighting) && (victim != ch)) {
        set_fighting(victim, ch);
    }
}

void cast_blindness(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_blindness(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (IS_AFFECTED(ch, AFF_BLIND)) {
            return;
        }
        spell_blindness(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (IS_AFFECTED(tar_ch, AFF_BLIND)) {
            return;
        }
        spell_blindness(level, ch, tar_ch, 0);
        break;
    default:
        Log("Error in cast_blindness()");
        break;
    }
}

void spell_burning_hands(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(3, 4) + level / 2 + 1;

    send_to_char("Searing flame fans out in front of you!\n\r", ch);
    act("$n sends a fan of flame shooting from the fingertips!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
		rdam = dam;
        temp = tmp_victim->next_in_room;
        if (ch->in_room == tmp_victim->in_room && ch != tmp_victim) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are seared by the burning flame!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam = 0;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    BurnWings(tmp_victim);
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_BURNING_HANDS);
            } else {
                act("You are able to avoid the flames!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        }
    }
}

void cast_burning_hands(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_burning_hands(level, ch, 0, 0);
        break;
    default:
        Log("Error in cast_burning_hands()");
        break;
    }
}

void spell_cacaodemon(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim && obj);

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }

    act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0,
        TO_ROOM);
    act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0,
        TO_CHAR);
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) > 40 && IS_EVIL(ch)) {
        act("$p smokes briefly", TRUE, ch, obj, 0, TO_ROOM);
        act("$p smokes briefly", TRUE, ch, obj, 0, TO_CHAR);
        obj->obj_flags.cost /= 2;
        if (obj->obj_flags.cost < 100) {
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_ROOM);
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_CHAR);
            obj_from_char(obj);
            extract_obj(obj);
        }
    } else {
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_ROOM);
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_CHAR);
        obj_from_char(obj);
        extract_obj(obj);
        GET_ALIGNMENT(ch) -= 5;
    }
    char_to_room(victim, ch->in_room);

    act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0,
        victim, TO_NOTVICT);

    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd!!'",
            TRUE, ch, 0, victim, TO_ROOM);
        act("$N refuses to hang out with crowd of your size!!", TRUE, ch,
            0, victim, TO_CHAR);
    } else {
        /*
         * charm them for a while
         */
        if (victim->master) {
            stop_follower(victim);
        }

        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;
        af.duration = follow_time(ch);
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;

        affect_to_char(victim, &af);
    }

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }
    if (IS_SET(victim->specials.act, ACT_META_AGG)) {
        REMOVE_BIT(victim->specials.act, ACT_META_AGG);
    }
    if (!IS_SET(victim->specials.act, ACT_SENTINEL)) {
        SET_BIT(victim->specials.act, ACT_SENTINEL);
    }
}

#define DEMON_TYPE_I     20
#define DEMON_TYPE_II    21
#define DEMON_TYPE_III   22
#define DEMON_TYPE_IV    23
#define DEMON_TYPE_V     24
#define DEMON_TYPE_VI    25

#define TYPE_VI_ITEM     27002
#define TYPE_V_ITEM      5107
#define TYPE_IV_ITEM     5113
#define TYPE_III_ITEM    1101
#define TYPE_II_ITEM     21014
#define TYPE_I_ITEM      5105

void cast_cacaodemon(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    char           *buffer;
    int             mob,
                    obj;
    struct obj_data *sac;
    struct char_data *el;
    int             held = FALSE,
                    wielded = FALSE;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char( "Huh?\n\r", ch );
        return;
    }

    if (NoSummon(ch)) {
        return;
    }

    if (!strcasecmp(buffer, "one")) {
        mob = DEMON_TYPE_I;
        obj = TYPE_I_ITEM;
    } else if (!strcasecmp(buffer, "two")) {
        mob = DEMON_TYPE_II;
        obj = TYPE_II_ITEM;
    } else if (!strcasecmp(buffer, "three")) {
        mob = DEMON_TYPE_III;
        obj = TYPE_III_ITEM;
    } else if (!strcasecmp(buffer, "four")) {
        mob = DEMON_TYPE_IV;
        obj = TYPE_IV_ITEM;
    } else if (!strcasecmp(buffer, "five")) {
        mob = DEMON_TYPE_V;
        obj = TYPE_V_ITEM;
    } else if (!strcasecmp(buffer, "six")) {
        mob = DEMON_TYPE_VI;
        obj = TYPE_VI_ITEM;
    } else {
        send_to_char("It seems that all demons of that type are currently in "
                     "the service of others.\n\r", ch);
        return;
    }

    if (!ch->equipment[WIELD] && !ch->equipment[HOLD]) {
        send_to_char("You must wield or hold an item to offer the demon for "
                     "its services.\n\r", ch);
        return;
    }

    if (ch->equipment[WIELD] &&
        obj_index[ch->equipment[WIELD]->item_number].virtual == obj) {
        wielded = TRUE;
    }

    if (ch->equipment[HOLD] &&
        obj_index[ch->equipment[HOLD]->item_number].virtual == obj) {
        held = TRUE;
    }

    if (!wielded && !held) {
        send_to_char("You do the spell perfectly, but no demon comes.\n\r", ch);
        send_to_char("You realize that the demon was dissatisfied with your "
                     "offering and\n\r", ch);
        send_to_char("wants you to offer it something else.\n\r", ch);
        return;
    }

    sac = unequip_char(ch, (held ? HOLD : WIELD));
    if ((sac) && (GET_LEVEL(ch, CLERIC_LEVEL_IND) > 40) && IS_EVIL(ch)) {
        if (sac->obj_flags.cost >= 200) {
            equip_char(ch, sac, (held ? HOLD : WIELD));
        } else {
            obj_to_char(sac, ch);
        }
    } else {
        obj_to_char(sac, ch);
    }

    if (sac) {
        if (ObjVnum(sac) != obj) {
            send_to_char("Your offering must be an item that the demon "
                         "values.\n\r", ch);
            return;
        }

        el = read_mobile(mob, VIRTUAL);
        if (!el) {
            send_to_char("You sense that all demons of that kind are in "
                         "others' services...\n\r", ch);
            return;
        }
    } else {
        send_to_char("You must be holding or wielding the item you are "
                     "offering to the demon.\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:    
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_cacaodemon(level, ch, el, sac);
        break;
    default:
        Log("Error in cast_cacaodemon()");
        break;
    }
}


void spell_calm(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    /*
     * removes aggressive bit from monsters
     */
    if (IS_NPC(victim)) {
        if (IS_SET(victim->specials.act, ACT_AGGRESSIVE) ||
            IS_SET(victim->specials.act, ACT_META_AGG)) {
            if (HitOrMiss(ch, victim, CalcThaco(ch))) {
                if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
                    REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
                }
                if (IS_SET(victim->specials.act, ACT_META_AGG)) {
                    REMOVE_BIT(victim->specials.act, ACT_META_AGG);
                }
                send_to_char("You sense peace.\n\r", ch);
            }
        } else {
            send_to_char("You feel calm\n\r", victim);
        }
    } else {
        send_to_char("You feel calm.\n\r", victim);
    }
}


void cast_calm(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_calm(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_calm(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_calm(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            spell_calm(level, ch, tar_ch, 0);
        }
        break;
    default:
        Log("Serious screw-up in continual light!");
        break;
    }
}

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

void spell_charm_person(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r",
                         ch);
            return;
        }

        if (!IsPerson(victim)) {
            send_to_char("Umm,  that's not a person....\n\r", ch);
            return;
        }

        if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
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

void cast_charm_person(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
        break;
    case SPELL_TYPE_SPELL:
        spell_charm_person(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            return;
        }
        spell_charm_person(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_charm_person(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in charm person!");
        break;
    }
}

void spell_colour_spray(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = 4 * level;

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}

void cast_colour_spray(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
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
    case SPELL_TYPE_STAFF:
        if (victim) {
            spell_colour_spray(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in colour spray!");
        break;
    }
}

void spell_comp_languages(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[128];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_COMP_LANGUAGES)) {
        if (ch != victim) {
            act("$n gently touches $N's ears.", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You touch $N's ears gently.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n gently touches your ears, wow you have missed so much!",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n touches $s own ears.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You touch your ears, wow you missed so much!", FALSE, ch,
                0, victim, TO_CHAR);
        }
        af.type = SPELL_COMP_LANGUAGES;
        /*
         * one tic only!
         */
        af.duration = (!IS_IMMORTAL(ch) ? level / 2 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N can already understand languages.");
        } else {
            sprintf(buf, "You can already understand languages.");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_comp_languages(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_comp_languages(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_comp_languages(level, ch, tar_ch, 0);
        break;

    default:
        Log("Serious screw-up in comprehend languages!");
        break;
    }
}

void spell_cone_of_cold(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 5);

    send_to_char("A cone of freezing air fans out before you\n\r", ch);
    act("$n sends a cone of ice shooting from the fingertips!",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are chilled to the bone!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_CONE_OF_COLD);
            } else {
                act("You are able to avoid the cone!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
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

void spell_conjure_elemental(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    /*
     *   victim, in this case, is the elemental
     *   object could be the sacrificial object
     */

    assert(ch && victim && obj);

    /*
     * objects:
     *     fire  : red stone
     *     water : pale blue stone
     *     earth : grey stone
     *     air   : clear stone
     */

    act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
    act("You gesture, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
    act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
    act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
    obj_from_char(obj);
    extract_obj(obj);
    char_to_room(victim, ch->in_room);
    act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

    /*
     * charm them for a while
     */

    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd, but thanks for the"
            " lift'", TRUE, ch, 0, victim, TO_ROOM);
        act("$N refuses to hang out with crowd of your size, but thanks you "
            "for the free ride.", TRUE, ch, 0, victim, TO_CHAR);
    } else {
        if (victim->master) {
            stop_follower(victim);
        }

        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;
        af.duration = 20;
        af.modifier = follow_time(ch);
        af.location = 0;
        af.bitvector = AFF_CHARM;

        affect_to_char(victim, &af);
    }
}

#define FIRE_ELEMENTAL  10
#define WATER_ELEMENTAL 11
#define EARTH_ELEMENTAL 12
#define AIR_ELEMENTAL   13

#define RED_STONE       5233
#define PALE_BLUE_STONE 5230
#define GREY_STONE      5239
#define CLEAR_STONE     5243

void cast_conjure_elemental(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    char           *buffer;
    int             mob,
                    obj;
    struct obj_data *sac;
    struct char_data *el;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char( "What are you trying to conjure?\n\r", ch );
        return;
    }

    if (NoSummon(ch)) {
        return;
    }

    if (!strcasecmp(buffer, "fire")) {
        mob = FIRE_ELEMENTAL;
        obj = RED_STONE;
    } else if (!strcasecmp(buffer, "water")) {
        mob = WATER_ELEMENTAL;
        obj = PALE_BLUE_STONE;
    } else if (!strcasecmp(buffer, "air")) {
        mob = AIR_ELEMENTAL;
        obj = CLEAR_STONE;
    } else if (!strcasecmp(buffer, "earth")) {
        mob = EARTH_ELEMENTAL;
        obj = GREY_STONE;
    } else {
        send_to_char("There are no elementals of that type available\n\r", ch);
        return;
    }
    if (!ch->equipment[HOLD]) {
        send_to_char(" You must be holding the correct stone\n\r", ch);
        return;
    }

    sac = unequip_char(ch, HOLD);
    if (sac) {
        obj_to_char(sac, ch);
        if (ObjVnum(sac) != obj) {
            send_to_char("You must have the correct item to sacrifice.\n\r",
                         ch);
            return;
        }

        el = read_mobile(mob, VIRTUAL);
        if (!el) {
            send_to_char("There are no elementals of that type available\n\r",
                         ch);
            return;
        }
    } else {
        send_to_char("You must be holding the correct item to sacrifice.\n\r",
                     ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_conjure_elemental(level, ch, el, sac);
        break;
    default:
        Log("serious screw-up in conjure_elemental.");
        break;
    }
}


void spell_cont_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = read_object(20, VIRTUAL);
    if (tmp_obj) {
        obj_to_char(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the ball of light\n\r", ch);
        return;
    }

    act("$n twiddles $s thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_ROOM);
    act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_CHAR);
}

void cast_cont_light(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_cont_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_cont_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_cont_light(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in continual light!");
        break;
    }
}


void spell_light(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{

    /*
     * creates a ball of light in the hands.
     */
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = read_object(20, VIRTUAL); /* this is all you have to do */
    if (tmp_obj) {
        tmp_obj->obj_flags.value[2] = 24 + level;
        obj_to_char(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the ball of light\n\r", ch);
        return;
    }

    act("$n twiddles $s thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_ROOM);
    act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_CHAR);
}

void cast_light(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_light(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in light!");
        break;
    }
}

void spell_curse(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (obj) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
        SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

        /*
         * LOWER ATTACK DICE BY -1
         */
        if (IS_WEAPON(obj)) {
            obj->obj_flags.value[2]--;
        }
        act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
        if (saves_spell(victim, SAVING_SPELL) ||
            affected_by_spell(victim, SPELL_CURSE)) {
            return;
        }
        af.type = SPELL_CURSE;
        af.duration = 24 * 7;
        af.modifier = -1;
        af.location = APPLY_HITROLL;
        af.bitvector = AFF_CURSE;
        affect_to_char(victim, &af);

        af.location = APPLY_SAVING_PARA;
        af.modifier = 1;
        affect_to_char(victim, &af);

        act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
        act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);
        if (IS_NPC(victim) && !victim->specials.fighting) {
            set_fighting(victim, ch);
        }
        if (IS_PC(ch) && IS_PC(victim)) {
            GET_ALIGNMENT(ch) -= 2;
        }
    }
}

void cast_curse(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    char            buf[255];

    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_obj) {
            /* It is an object */
            spell_curse(level, ch, 0, tar_obj);
        } else {
            /* Then it is a PC | NPC */
            spell_curse(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_POTION:
        spell_curse(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            /* It is an object */
            spell_curse(level, ch, 0, tar_obj);
        } else {
            /* Then it is a PC | NPC */
            if (!tar_ch) {
                tar_ch = ch;
            }
            spell_curse(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            /* It is an object */
            spell_curse(level, ch, 0, tar_obj);
        } else {
            /* Then it is a PC | NPC */
            if (!tar_ch)
                tar_ch = ch;
            spell_curse(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_curse(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        sprintf(buf, "Serious screw up in curse! Char = %s.", ch->player.name);
        Log(buf);
        break;
    }
}

void spell_disintegrate(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             i,
                    damage;
    struct obj_data *x;
    char            buf[MAX_STRING_LENGTH + 30];

    if (!ch) {
        Log("!ch in spell_disintegrate");
        return;
    }

    if (!victim) {
        Log("!victim in spell_disintegrate");
        return;
    }

    damage = dice(level, 10);
    if (!saves_spell(victim, SAVING_SPELL)) {
        i = 0;
        if (!(IS_PC(ch) && IS_PC(victim))) {
            /*
             * get rid of player-player scrappage
             */
            if (!IS_SET(real_roomp(victim->in_room)->room_flags, ARENA_ROOM)) {
                /*
                 * frag their EQ
                 */
                while (i < MAX_WEAR) {
                    if (victim->equipment[i]) {
                        obj = victim->equipment[i];
                        if (!ItemSave(obj, SPELL_DISINTEGRATE)) {
                            act("$p turns red hot, $N screams, then it "
                                "disappears in a puff of smoke!", TRUE, ch,
                                obj, victim, TO_CHAR);
                            if (obj->equipped_by || obj->carried_by) {
                                act("$p, held by $N, disappears in a puff of "
                                    "smoke!", TRUE, ch, obj, victim, TO_ROOM);
                            }

                            if (obj->carried_by) {
                                obj_from_char(obj);
                            } else if (obj->equipped_by) {
                                obj = unequip_char(obj->equipped_by,
                                                   obj->eq_pos);
                            } else if (obj->in_obj) {
                                obj_from_obj(obj);
                                obj_to_room(obj, ch->in_room);
                            } else if (obj->contains) {
                                while (obj->contains) {
                                    x = obj->contains;
                                    obj_from_obj(x);
                                    obj_to_room(x, ch->in_room);
                                }
                            }
                            if (obj) {
                                extract_obj(obj);
                            }
                        } else if (obj) {
                            act("$c0010$p resists the disintegration ray "
                                "completely!", TRUE, ch, obj, victim, TO_VICT);
                            act("$c0010$p carried by $N, resists $n's "
                                "disintegration ray!", TRUE, ch, obj, victim,
                                TO_ROOM);
                        }
                    }
                    i++;
                }
            }
        } else {
            /*
             * player hitting player with disint outside arena
             */
            sprintf(buf, "%s just hit %s with a disintegrate outside the "
                         "arena!", GET_NAME(ch), GET_NAME(victim));
            Log(buf);
        }
    } else {
        /*
         * we saved ! 1/2 dam and no EQ frag
         */
        damage >>= 1;
    }

    MissileDamage(ch, victim, damage, SPELL_DISINTEGRATE);
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) -= 2;
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

void spell_faerie_fire(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
    act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

    af.type = SPELL_FAERIE_FIRE;
    af.duration = level;
    af.modifier = 20;
    af.location = APPLY_ARMOR;
    af.bitvector = 0;

    affect_to_char(victim, &af);
}

void cast_faerie_fire(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_faerie_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_faerie_fire(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_faerie_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_faerie_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_faerie_fire(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel good!");
        break;
    }
}

void spell_faerie_fog(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim;
    int             revealed = 0;

    assert(ch);

    act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
        TRUE, ch, 0, 0, TO_ROOM);
    act("You snap your fingers, and a cloud of purple smoke billows forth",
        TRUE, ch, 0, 0, TO_CHAR);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = tmp_victim->next_in_room) {
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                break;
            }
            if (!in_group(ch, tmp_victim)) {
                if (IS_AFFECTED(tmp_victim, AFF_HIDE)) {
                    if (ImpSaveSpell(tmp_victim, SAVING_SPELL, -10)) {
                        REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        act("$n is briefly revealed, but disappears again.",
                            TRUE, tmp_victim, 0, 0, TO_ROOM);
                        act("You are briefly revealed, but disappear again.",
                            TRUE, tmp_victim, 0, 0, TO_CHAR);
                        SET_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        revealed = 1;
                    } else {
                        REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        act("$n is revealed!", TRUE, tmp_victim, 0, 0,
                            TO_ROOM);
                        act("You are revealed!", TRUE, tmp_victim, 0, 0,
                            TO_CHAR);
                        revealed = 1;
                    }
                }
                if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE)) {
                    REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
                    if (!revealed) {
                        act("$n is revealed!", TRUE, tmp_victim, 0, 0, TO_ROOM);
                        act("You are revealed!", TRUE, tmp_victim, 0, 0,
                            TO_CHAR);
                    }
                }
            }
        }
    }
}

void cast_faerie_fog(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_faerie_fog(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in faerie fog!");
        break;
    }
}

void spell_feeblemind(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_MINDBLANK) &&
        !saves_spell(victim, SAVING_SPELL)) {

        if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
            send_to_char("They are already dumb enough as it is!\n\r", ch);
            return;
        }
        if (IS_IMMORTAL(victim)) {
            send_to_char("Dur, that was stupid!\n\r", ch);
            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = -5;
            af.location = APPLY_INT;
            af.bitvector = 0;
            affect_to_char(ch, &af);

            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = 70;
            af.location = APPLY_SPELLFAIL;
            af.bitvector = 0;
            affect_to_char(ch, &af);
        } else {
            send_to_char("You feel really really dumb\n\r", victim);
            act("$n suddenly looks rather dumb.", FALSE, victim, 0, 0,
                TO_NOTVICT);
            
            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = -5;
            af.location = APPLY_INT;
            af.bitvector = 0;
            affect_to_char(victim, &af);

            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = 70;
            af.location = APPLY_SPELLFAIL;
            af.bitvector = 0;
            affect_to_char(victim, &af);
#if 0
            /*
             * That is stupid... people were complaining... -MW
             */
            if (!victim->skills) {
                return;
            }
            t = number(1,100);
            while (1) {
                for (i=0;i<MAX_SKILLS;i++) {
                    if (victim->skills[i].learned)  {
                        t--;
                    }
                    if (t==0) {
                        victim->skills[i].learned = 0;
                        victim->skills[i].flags = 0;
                        break;
                    }
                }
                return;
            }
#endif
        }
    } else {
	    if (!victim->specials.fighting) {
            /*
             * they saved
             */
            set_fighting(victim, ch);
        }
        act("$n seems to resist your spell.", FALSE, victim, 0, 0, TO_CHAR);
    }
}

void cast_feeblemind(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
        spell_feeblemind(level, ch, ch, 0);
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_feeblemind(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in feeblemind.");
        break;
    }
}



#define KITTEN  3090
#define PUPPY   3091
#define BEAGLE  3092
#define ROTT    3093
#define WOLF    3094

void spell_familiar(int level, struct char_data *ch,
                    struct char_data **victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *f;

    if (affected_by_spell(ch, SPELL_FAMILIAR)) {
        send_to_char("You can't have more than 1 familiar per day\n\r", ch);
        return;
    }

    /*
     * depending on the level, one of the pet shop kids.
     */

    if (level < 2) {
        f = read_mobile(KITTEN, VIRTUAL);
    } else if (level < 4) {
        f = read_mobile(PUPPY, VIRTUAL);
    } else if (level < 6) {
        f = read_mobile(BEAGLE, VIRTUAL);
    } else if (level < 8) {
        f = read_mobile(ROTT, VIRTUAL);
    } else {
        f = read_mobile(WOLF, VIRTUAL);
    }
    char_to_room(f, ch->in_room);

    af.type = SPELL_FAMILIAR;
    af.duration = 24;
    af.modifier = -1;
    af.location = APPLY_ARMOR;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    act("$n appears in a flash of light!\n\r", FALSE, f, 0, 0, TO_ROOM);

    SET_BIT(f->specials.affected_by, AFF_CHARM);
    GET_EXP(f) = 0;
    add_follower(f, ch);
    IS_CARRYING_W(f) = 0;
    IS_CARRYING_N(f) = 0;

    *victim = f;
}

void cast_familiar(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    char            buf[128];

    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_familiar(level, ch, &tar_ch, 0);

        if (tar_ch) {
            sprintf(buf, "%s %s", GET_NAME(tar_ch), fname(arg));

            if (GET_NAME(tar_ch)) {
                free(GET_NAME(tar_ch));
            }
            GET_NAME(tar_ch) = (char *) malloc(strlen(buf) + 1);
            strcpy(GET_NAME(tar_ch), buf);
        }
        break;
    default:
        Log("serious screw-up in familiar.");
        break;
    }
}

void spell_fireball(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 8);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    /*
                     * Fail the second save, then their wings are toast
                     * for two weeks
                     */
                    BurnWings(tmp_victim);
                }
                heat_blind(tmp_victim);
                MissileDamage(ch, tmp_victim, rdam, SPELL_FIREBALL);
            } else {
                act("You dodge the mass of flame!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        } else if (tmp_victim->in_room != NOWHERE &&
                   real_roomp(ch->in_room)->zone ==
                   real_roomp(tmp_victim->in_room)->zone) {
            send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
        }
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

void spell_fly_group(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *tch;

    assert(ch);

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    if (ch && IS_SET(ch->specials.act, PLR_NOFLY)) {
        REMOVE_BIT(ch->specials.act, PLR_NOFLY);
    }

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
        if (in_group(ch, tch)) {
            act("You feel lighter than air!", TRUE, ch, 0, tch, TO_VICT);
            if (tch != ch) {
                act("$N's feet rise off the ground.", TRUE, ch, 0, tch,
                    TO_CHAR);
            } else {
                send_to_char("Your feet rise up off the ground.", ch);
            }
            act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_NOTVICT);

            af.type = SPELL_FLY;
            af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_FLYING;
            affect_to_char(tch, &af);
        }
    }
}

void cast_fly_group(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_fly_group(level, ch, 0, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_fly(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in fly");
        break;
    }
}


void spell_gust_of_wind(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    send_to_char("You wave your hands, and a gust of wind boils forth!\n\r",
                 ch);
    act("$n sends a gust of wind towards you!", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }

            if (!in_group(ch, tmp_victim)) {
                if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                    act("Despite your sudden gusts, $N manages to keep $S "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_CHAR);
                    act("Despite $n's sudden gusts, you manage to keep your "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_VICT);
                    act("Despite $n's sudden gusts, $N manages to keep $S "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                    return;
                }
                act("Your gust of wind makes $N stumble and $E falls on $S "
                    "bum.", FALSE, ch, 0, tmp_victim, TO_CHAR);
                act("$n's gust of wind makes you stumble and you lose your "
                    "footing.", FALSE, ch, 0, tmp_victim, TO_VICT);
                act("$n's gust of wind makes $N stumble and $E falls on $S "
                    "bum.", FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                GET_POS(tmp_victim) = POSITION_SITTING;
            } else {
                act("You are able to avoid the swirling gust.", FALSE, ch,
                    0, tmp_victim, TO_VICT);
            }
        }
    }
}

void cast_gust_of_wind(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_gust_of_wind(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in snare.");
        break;
    }
}

void spell_haste(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_HASTE)) {
        act("$N is already hasty!", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (affected_by_spell(victim, SPELL_SLOW)) {
        act("$N doesn't seem so slow anymore.", FALSE, ch, 0, victim,
            TO_CHAR);
        affect_from_char(victim, SPELL_SLOW);
        return;
    }

    if (IS_IMMUNE(victim, IMM_HOLD)) {
        act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
        act("$n just tried to haste you, but you ignored it.", FALSE, ch,
            0, victim, TO_VICT);
        if (!in_group(ch, victim) && !IS_PC(ch)) {
            hit(victim, ch, TYPE_UNDEFINED);
        }
        return;
    }

    af.type = SPELL_HASTE;
    af.duration = (!IS_IMMORTAL(ch) ? level : 99);
    af.modifier = 1;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_HASTE;
    affect_to_char(victim, &af);

    send_to_char("You feel fast!\n\r", victim);
    if (!IS_NPC(victim)) {
        victim->player.time.birth -= SECS_PER_MUD_YEAR;
    } else if (victim->desc && victim->desc->original) {
        victim->desc->original->player.time.birth -= SECS_PER_MUD_YEAR;
    }

    if (!in_group(ch, victim) && !IS_PC(ch)) {
        hit(victim, ch, TYPE_UNDEFINED);
    }
}

void cast_haste(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_haste(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
        if (tar_obj && tar_obj->obj_flags.type_flag != ITEM_POTION) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_haste(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_haste(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_haste(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in haste");
        break;
    }
}

void spell_ice_storm(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4);

    send_to_char("You conjure a storm of ice\n\r", ch);
    act("$n conjures an ice storm!", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are blasted by the storm",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_ICE_STORM);
            } else {
                act("You are able to dodge the storm!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
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


void spell_identify(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    char            buf[256],
                    buf2[256];
    int             i;
    bool            found,
                    auct = FALSE;

    char            color1[10],
                    color2[10];
    struct time_info_data ma;

    assert(ch && (obj || victim));

    if (obj) {
        if (auctioneer && auctioneer->specials.auction &&
            obj == auctioneer->specials.auction) {
            auct = TRUE;
        }

        if (auct) {
            sprintf(color1, "$c000c");
            sprintf(color2, "$c000w");
            send_to_char("$c000cThe item currently on auction has the "
                         "following stats:\n\r", ch);
        } else if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
            sprintf(color1, "$c000p");
            sprintf(color2, "$c000C");
            sprintf(buf, "%sYou feel informed:\n\r", color1);
            send_to_char(buf, ch);
        } else {
            sprintf(color1, "$c000B");
            sprintf(color2, "$c000w");
            sprintf(buf, "%sYou feel informed:\n\r", color1);
            send_to_char(buf, ch);
        }

        sprintf(buf, "%sObject '%s%s%s', Item type: %s", color1, color2,
                obj->name, color1, color2);
        sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
        strcat(buf, buf2);
        if (IS_WEAPON(obj)) {
            if (IS_IMMORTAL(ch)) {
                sprintf(buf2, "%s, Weapon Speed: %s%s%s (%s%.2f%s)",
                        color1, color2, SpeedDesc(obj->speed), color1,
                        color2, (float) obj->speed / 100, color1);
            } else {
                sprintf(buf2, "%s, Weapon Speed: %s%s", color1, color2,
                        SpeedDesc(obj->speed));
            }
            strcat(buf, buf2);
        }
        strcat(buf, "\n\r");
        send_to_char(buf, ch);

        /*
         * alittle more info for immortals -bcw
         */
        if (IS_IMMORTAL(ch)) {
            sprintf(buf, "%sR-number: [%s%d%s], V-number: [%s%ld%s]",
                    color1, color2, obj->item_number, color1, color2,
                    (obj->item_number >= 0) ?
                     obj_index[obj->item_number].virtual : 0, color1);

            if (obj->max == 0) {
                sprintf(buf2, "%s", "unlimited");
            } else {
                sprintf(buf2, "%d", obj->max);
            }

            sprintf(buf, "%s %sLoadrate: [%s%s%s], Tweak Rate: [%s%d%s], "
                         "ObjValue[%s%d%s]\n\r",
                    buf, color1, color2, buf2, color1, color2, obj->tweak,
                    color1, color2, eval(obj), color1);
            send_to_char(buf, ch);

            if (obj->level == 0) {
                sprintf(buf2, "%sEgo: %sNone%s, ", color1, color2, color1);
            } else {
                sprintf(buf2, "%sEgo: %sLevel %d%s, ", color1, color2,
                        obj->level, color1);
            }
            send_to_char(buf2, ch);

            sprintf(buf2, "%sLast modified by %s%s%s on %s%s", color1,
                    color2, obj->modBy ? obj->modBy : "unmodified", color1,
                    color2, asctime(localtime(&obj->modified)));
            send_to_char(buf2, ch);
        }

        if (obj->obj_flags.bitvector) {
            sprintf(buf2, "%sItem will give you following abilities:%s  ",
                    color1, color2);
            send_to_char(buf2, ch);

            sprintbit((unsigned) obj->obj_flags.bitvector, affected_bits, buf);
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
        }

        sprintf(buf, "%sItem is:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->obj_flags.extra_flags, extra_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        sprintf(buf, "%sWeight: %s%d%s, Value: %s%d%s, Rent cost: %s%d%s ",
                color1, color2, obj->obj_flags.weight, color1, color2,
                obj->obj_flags.cost, color1, color2,
                obj->obj_flags.cost_per_day, color1);
        send_to_char(buf, ch);

        if (IS_RARE(obj)) {
            sprintf(buf, "%s[%sRARE%s]", color1, color2, color1);
            send_to_char(buf, ch);
        }
        send_to_char("\n\r", ch);

        sprintf(buf, "%sCan be worn on:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->obj_flags.wear_flags, wear_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        switch (GET_ITEM_TYPE(obj)) {

        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "%sLevel %s%d%s spells of:\n\r", color1, color2,
                    obj->obj_flags.value[0], color1);
            send_to_char(buf, ch);

            for( i = 1; i < 4; i++ ) {
                if (obj->obj_flags.value[i] >= 1) {
                    sprinttype(obj->obj_flags.value[i] - 1, spells, buf);
                    sprintf(buf2, "%s%s", color2, buf);
                    strcat(buf2, "\n\r");
                    send_to_char(buf2, ch);
                }
            }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "%sCosts %s%d%s mana to use, with %s%d%s charges "
                         "left.\n\r",
                    color1, color2, obj->obj_flags.value[1], color1,
                    color2, obj->obj_flags.value[2], color1);
            send_to_char(buf, ch);

            sprintf(buf, "%sLevel %s%d%s spell of:\n\r", color1, color2,
                    obj->obj_flags.value[0], color1);
            send_to_char(buf, ch);
            if (obj->obj_flags.value[3] >= 1) {
                sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
                sprintf(buf2, "%s%s", color2, buf);
                strcat(buf2, "\n\r");
                send_to_char(buf2, ch);
            }
            break;

        case ITEM_WEAPON:
            sprintf(buf, "%sDamage Dice is '%s%dD%d%s' [%s%s%s] [%s%s%s]\n\r",
                    color1, color2, obj->obj_flags.value[1],
                    obj->obj_flags.value[2], color1, color2,
                    AttackType[obj->obj_flags.value[3]], color1, color2,
                    weaponskills[obj->weapontype].name, color1);
            send_to_char(buf, ch);
            break;

        case ITEM_ARMOR:
            sprintf(buf, "%sAC-apply is: %s%d%s,   Size of armor is: %s%s\n\r",
                    color1, color2, obj->obj_flags.value[0], color1,
                    color2, ArmorSize(obj->obj_flags.value[2]));
            send_to_char(buf, ch);
            break;
        }

        found = FALSE;

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location != APPLY_NONE &&
                obj->affected[i].modifier != 0) {
                if (!found) {
                    sprintf(buf, "%sCan affect you as:\n\r", color1);
                    send_to_char(buf, ch);
                    found = TRUE;
                }

                sprinttype(obj->affected[i].location, apply_types, buf2);
                sprintf(buf, "%s    Affects: %s%s%s by %s", color1, color2,
                        buf2, color1, color2);
                send_to_char(buf, ch);

                switch (obj->affected[i].location) {
                case APPLY_M_IMMUNE:
                case APPLY_IMMUNE:
                case APPLY_SUSC:
                    sprintbit(obj->affected[i].modifier, immunity_names, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_ATTACKS:
                    sprintf(buf2, "%.2f\n\r", obj->affected[i].modifier / 10.0);
                    break;

                case APPLY_WEAPON_SPELL:
                case APPLY_EAT_SPELL:
                    sprintf(buf2, "%s\n\r",
                            spells[obj->affected[i].modifier - 1]);
                    break;

                case APPLY_SPELL:
                    sprintbit(obj->affected[i].modifier, affected_bits, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_SPELL2:
                    sprintbit(obj->affected[i].modifier, affected_bits2, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_RACE_SLAYER:
                    sprintf(buf2, "%s\n\r",
                            RaceName[obj->affected[i].modifier]);
                    break;

                case APPLY_ALIGN_SLAYER:
                    if (obj->affected[i].modifier > 1) {
                        sprintf(buf2, "SLAY GOOD\n\r");
                    } else if (obj->affected[i].modifier == 1) {
                        sprintf(buf2, "SLAY NEUTRAL\n\r");
                    } else {
                        sprintf(buf2, "SLAY EVIL\n\r");
                    }
                    break;

                default:
                    sprintf(buf2, "%ld\n\r", obj->affected[i].modifier);
                    break;
                }
                send_to_char(buf2, ch);
            }
        }
    } else if (!IS_NPC(victim)) {
        if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
            sprintf(color1, "$c000p");
            sprintf(color2, "$c000C");
        } else {
            sprintf(color1, "$c000B");
            sprintf(color2, "$c000w");
        }
        sprintf(buf, "%sYou feel informed:\n\r", color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sName: %s%s is ", color1, color2, GET_NAME(victim));
        send_to_char(buf, ch);

        age2(victim, &ma);
        sprintf(buf, "%s%d%s years,  %s%d%s months, %s%d%s days, %s%d%s hours"
                     " old.\n\r", color2, ma.year, color1, color2, ma.month,
                     color1, color2, ma.day, color1, color2, ma.hours, color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sHeight:  %s%d%s cm, Weight:  %s%d%s pounds.\n\r",
                color1, color2, GET_HEIGHT(victim), color1, color2,
                GET_WEIGHT(victim), color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sArmor Class %s%d\n\r", color1, color2,
                victim->points.armor);
        send_to_char(buf, ch);

        if (GET_LEVEL(ch, BestMagicClass(ch)) > 30) {
            sprintf(buf, "%sStr %s%d%s/%s%d%s, Int %s%d%s, Wis %s%d%s, Dex "
                         "%s%d%s, Con %s%d$%s, Ch %s%d\n\r",
                    color1, color2, GET_STR(victim), color1, color2,
                    GET_ADD(victim), color1, color2, GET_INT(victim),
                    color1, color2, GET_WIS(victim), color1, color2,
                    GET_DEX(victim), color1, color2, GET_CON(victim), color1,
                    color2, GET_CHR(victim));
            send_to_char(buf, ch);
        }
    } else {
        send_to_char("You learn nothing new.\n\r", ch);
    }
    if (!IS_IMMORTAL(ch)) {
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
}

void cast_identify(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_identify(level, ch, tar_ch, tar_obj);
        if (!IS_IMMORTAL(ch)) {
            send_to_char("A wave of nausea overcomes you.  You collapse!\n\r",
                         ch);
            WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        }
        break;
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
        spell_identify(level, ch, tar_ch, tar_obj);
        break;
    default:
        Log("Serious screw-up in identify!");
        break;
    }
}

void spell_incendiary_cloud(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 10);

    send_to_char("Billowing clouds of incendiary gases pour from your "
                 "fingertips.\n\r", ch);
    act("Billowing clouds of incendiary gases pour from $n's fingertips!",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                    if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                        /*
                         * Fail two saves, wings burn
                         */
                        BurnWings(tmp_victim);
                    }
                    heat_blind(tmp_victim);
                    MissileDamage(ch, tmp_victim, rdam, SPELL_INCENDIARY_CLOUD);
                } else {
                    rdam >>= 1;
                    /*
                     * half dam
                     */
                    heat_blind(tmp_victim);
                    MissileDamage(ch, tmp_victim, rdam, SPELL_INCENDIARY_CLOUD);
                }
            } else {
                act("You avoid the smouldering gases!\n\r", FALSE, ch, 0,
                    tmp_victim, TO_VICT);
            }
        }
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

void spell_infravision(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
        if (ch != victim) {
            send_to_char("Your eyes glow red.\n\r", victim);
            act("$n's eyes glow red.", FALSE, victim, 0, 0, TO_ROOM);
        } else {
            send_to_char("Your eyes glow red.\n\r", ch);
            act("$n's eyes glow red.", FALSE, ch, 0, 0, TO_ROOM);
        }

        af.type = SPELL_INFRAVISION;
        af.duration = 4 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_INFRAVISION;
        affect_to_char(victim, &af);

    }
}

void cast_infravision(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_infravision(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
            return;
        }
        spell_infravision(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
            return;
        }
        spell_infravision(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
            return;
        }
        spell_infravision(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_INFRAVISION)) {
                spell_infravision(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in infravision!");
        break;
    }
}

void spell_invisibility(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert((ch && obj) || victim);

    if (obj) {
        if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
            act("$p turns invisible.", FALSE, ch, obj, 0, TO_CHAR);
            act("$p turns invisible.", TRUE, ch, obj, 0, TO_ROOM);
            SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        }
    } else {
        /*
         * Then it is a PC | NPC
         */
        if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
            act("$n slowly fades out of existence.", TRUE, victim, 0, 0,
                TO_ROOM);
            send_to_char("You vanish.\n\r", victim);

            af.type = SPELL_INVISIBLE;
            af.duration = 24;
            af.modifier = -40;
            af.location = APPLY_AC;
            af.bitvector = AFF_INVISIBLE;
            affect_to_char(victim, &af);
        }
    }
}

void cast_invisibility(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_obj) {
            if (IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
                send_to_char("Nothing new seems to happen.\n\r", ch);
            } else {
                spell_invisibility(level, ch, 0, tar_obj);
            }
        } else {
            /* tar_ch */
            if (IS_AFFECTED(tar_ch, AFF_INVISIBLE)) {
                send_to_char("Nothing new seems to happen.\n\r", ch);
            } else {
                spell_invisibility(level, ch, tar_ch, 0);
            }
        }
        break;
    case SPELL_TYPE_POTION:
        if (!IS_AFFECTED(ch, AFF_INVISIBLE)) {
            spell_invisibility(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE))) {
                spell_invisibility(level, ch, 0, tar_obj);
            }
        } else {
            /* tar_ch */
            if (!tar_ch) {
                tar_ch = ch;
            }
            if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE))) {
                spell_invisibility(level, ch, tar_ch, 0);
            }
        }
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            if (!(IS_SET(tar_obj->obj_flags.extra_flags, ITEM_INVISIBLE)))
                spell_invisibility(level, ch, 0, tar_obj);
        } else {
            /* tar_ch */
            if (!(IS_AFFECTED(tar_ch, AFF_INVISIBLE)))
                spell_invisibility(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_INVISIBLE)) {
                spell_invisibility(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in invisibility!");
        break;
    }
}

void cast_knock(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    int             door;
    char           *dir;
    char           *otype;
    struct obj_data *obj;
    struct char_data *victim;
    struct room_direction_data *exitdata;

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        arg = get_argument(arg, &otype);
        arg = get_argument(arg, &dir);

        if (!otype) {
            send_to_char("Knock on what?\n\r", ch);
            return;
        }

        if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, 
                         &obj)) {
            send_to_char("sorry.. this spell can only be cast on doors\n", ch);
            return;
        } 
        
        if ((door = find_door(ch, otype, dir)) >= 0) {
            if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
                send_to_char("That's absurd.\n\r", ch);
            } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
                send_to_char("You realize that the door is already open.\n\r",
                             ch);
            } else if (EXIT(ch, door)->key < 0) {
                send_to_char("You can't seem to spot any lock to pick.\n\r",
                             ch);
            } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
                send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
            } else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) {
                send_to_char("You seem to be unable to knock this...\n\r", ch);
            } else {
                if (EXIT(ch, door)->keyword &&
                    strcmp("secret", fname(EXIT(ch, door)->keyword))) {
                    act("$n magically opens the lock of the $F.", 0,
                        ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
                } else {
                    act("$n magically opens the lock.", TRUE, ch, 0, 0,
                        TO_ROOM);
                }
                send_to_char("The lock quickly yields to your skills.\n\r", ch);
                raw_unlock_door(ch, EXIT(ch, door), door);
            }
        }
        break;
    case SPELL_TYPE_STAFF:
        for (door = 0; door <= 5; door++) {
            exitdata = EXIT(ch, door);
            if (exitdata && !IS_SET(exitdata->exit_info, EX_PICKPROOF) &&
                IS_SET(exitdata->exit_info, EX_CLOSED) &&
                IS_SET(exitdata->exit_info, EX_LOCKED) &&
                IS_SET(exitdata->exit_info, EX_ISDOOR) &&
                !(exitdata->key < 0)) {
                act("$N magically opens a lock.", TRUE, ch, 0, 0, TO_ROOM);
                send_to_char("You magically open a lock.\n", ch);
                raw_unlock_door(ch, exitdata, door);
            }
        }
        break;
    default:
        Log("serious error in Knock.");
        break;
    }
}

void spell_know_alignment(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             ap;
    char            buf[200],
                    name[100];

    assert(victim && ch);

    if (IS_NPC(victim)) {
        strcpy(name, victim->player.short_descr);
    } else {
        strcpy(name, GET_NAME(victim));
    }
    ap = GET_ALIGNMENT(victim);

    if (ap > 700) {
        sprintf(buf, "%s has an aura as white as the driven snow.\n\r", name);
    } else if (ap > 350) {
        sprintf(buf, "%s is of excellent moral character, whose aura glows"
                     " off-white.\n\r", name);
    } else if (ap > 100) {
        sprintf(buf, "%s is often kind and thoughtful, their aura is"
                     " predominantly white.\n\r", name);
    } else if (ap > 25) {
        sprintf(buf, "%s's aura is a light shade of grey, they do more"
                     " good than harm.\n\r", name);
    } else if (ap > -25) {
        sprintf(buf, "%s doesn't seem to have a firm moral commitment, their"
                     " aura is a dull grey.\n\r",
                name);
    } else if (ap > -100) {
        sprintf(buf, "%s's aura is grey, with a tinge of red around the"
                     " edges.\n\r", name);
    } else if (ap > -350) {
        sprintf(buf, "%s is very close to being considered evil, with a"
                     " predominantly red aura.\n\r", name);
    } else if (ap > -700) {
        sprintf(buf, "%s's reddish aura shows that they are on the path"
                     " of darkness.\n\r", name);
    } else {
        sprintf(buf, "%s has a red, pulsing aura, showing no compassion or"
                     " mercy.\n\r", name);
    }
    send_to_char(buf, ch);
}

void cast_know_alignment(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_know_alignment(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_know_alignment(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_know_alignment(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in know alignment!");
        break;
    }
}

void spell_know_monster(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    char            buf[256],
                    buf2[256];
    int             exp,
                    lev,
                    hits;

    extern char    *pc_class_types[];
    extern char    *immunity_names[];
    extern char    *RaceName[];
    extern const char *RaceDesc[];
    int             att;
    int             no,
                    s;

    /*
     * depending on level, give info.. sometimes inaccurate
     */

    if (!IS_PC(victim)) {
        sprintf(buf, "$c000W$N$c000p belongs to the $c000W%s$c000p race.",
                RaceName[GET_RACE(victim)]);
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        if (level > 5) {
            exp = GetApprox(GET_EXP(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p is worth approximately $c000W%d$c000p "
                         "experience", exp);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 10) {
            lev = GetApprox(GetMaxLevel(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p fights like a $c000W%d$c000p level "
                         "warrior, you think", lev);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 15) {
            if (IS_SET(victim->hatefield, HATE_RACE)) {
                sprintf(buf, "$c000W$N$c000p seems to hate the $c000W%s$c000p "
                             "race", RaceName[victim->hates.race]);
                act(buf, FALSE, ch, 0, victim, TO_CHAR);
            }
            if (IS_SET(victim->hatefield, HATE_CLASS)) {
                sprintbit((unsigned) victim->hates.class, pc_class_types, buf2);
                sprintf(buf, "$c000W$N$c000p seems to hate the $c000W%s$c000p "
                             "class(es)", buf2);
                act(buf, FALSE, ch, 0, victim, TO_CHAR);
            }
        }

        if (level > 20) {
            hits = GetApprox(GET_MAX_HIT(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p probably has about $c000W%d$c000p hit "
                         "points", hits);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 25 && victim->susc) {
            sprintbit(victim->susc, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is susceptible to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 30 && victim->immune) {
            sprintbit(victim->immune, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is resistant to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 35 && victim->M_immune) {
            sprintbit(victim->M_immune, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is immune to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 40) {
            att = GetApprox((int) victim->mult_att, 30 + level);
            sprintf(buf, "$c000W$N$c000p gets approx $c000W%d.0$c000p "
                    "attack(s) per round", att);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 45) {
            no = GetApprox(victim->specials.damnodice, 30 + level);
            s = GetApprox(victim->specials.damsizedice, 30 + level);

            sprintf(buf, "$c000pEach does about $c000W%dd%d$c000p points of "
                         "damage", no, s);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        ch_printf(ch, "$c000pDescription:$c000W \n\r%s",
                  RaceDesc[victim->race]);
    } else {
        send_to_char("Thats not a REAL monster\n\r", ch);
        return;
    }
}

void cast_know_monster(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            return;
        }
        spell_know_monster(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in know_monster.");
        break;
    }
}

void spell_lightning_bolt(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 6);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
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

void spell_magic_missile(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice((int) (level / 2) + 1, 4) + (level / 2) + 1;

    if (affected_by_spell(victim, SPELL_SHIELD)) {
        dam = 0;
    }
    MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE);
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

void spell_major_track(int level, struct char_data *ch,
                       struct char_data *targ, struct obj_data *obj)
{
    struct affected_type af;

    if (ch != targ) {
        send_to_char("You feel your awareness grow!\n\r", targ);
    } else {
        send_to_char("You feel your awareness grow!\n\r", ch);
    }

    act("$N's eyes take on an emerald hue for just a moment.", 0, ch, 0,
        targ, TO_ROOM);

    af.type = SPELL_MAJOR_TRACK;
    af.duration = level * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(targ, &af);
}

void cast_major_track(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_major_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_major_track(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_major_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_major_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) { 
            if (tar_ch != ch) {
                spell_major_track(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious problem in major track");
        break;
    }
}

void spell_meteor_swarm(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 12);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_METEOR_SWARM);
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

void spell_minor_create(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    assert(ch && obj);

    act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
    act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
    act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
    act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);
    obj_to_room(obj, ch->in_room);
}

#define LONG_SWORD   3022
#define SHIELD       3042
#define RAFT         3060
#define BAG          3032
#define WATER_BARREL 6013
#define BREAD        3010

void cast_minor_creation(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    char           *buffer;
    int             obj;
    struct obj_data *o;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char( "Whatcha trying to create, numbskull?\n\r", ch );
        return;
    }

    if (!strcasecmp(buffer, "sword")) {
        obj = LONG_SWORD;
    } else if (!strcasecmp(buffer, "shield")) {
        obj = SHIELD;
    } else if (!strcasecmp(buffer, "raft")) {
        obj = RAFT;
    } else if (!strcasecmp(buffer, "bag")) {
        obj = BAG;
    } else if (!strcasecmp(buffer, "barrel")) {
        obj = WATER_BARREL;
    } else if (!strcasecmp(buffer, "bread")) {
        obj = BREAD;
    } else {
        send_to_char("There is nothing of that available\n\r", ch);
        return;
    }

    o = read_object(obj, VIRTUAL);
    if (!o) {
        send_to_char("There is nothing of that available\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_minor_create(level, ch, 0, o);
        break;
    default:
        Log("serious screw-up in minor_create.");
        break;
    }
}

void spell_minor_track(int level, struct char_data *ch,
                       struct char_data *targ, struct obj_data *obj)
{
    struct affected_type af;


    if (ch != targ) {
        send_to_char("You feel your awareness grow!\n\r", targ);
    } else {
        send_to_char("You feel your awareness grow!\n\r", ch);
    }

    act("$N's eyes take on an emerald hue for just a moment.", 0, ch, 0,
        targ, TO_ROOM);

    af.type = SPELL_MINOR_TRACK;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(targ, &af);
}

void cast_minor_track(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_minor_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_minor_track(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_minor_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_minor_track(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_minor_track(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious problem in minor track");
        break;
    }
}

void spell_Create_Monster(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *mob;
    int             rnum;
    struct room_data *rp;

    /*
     * load in a monster of the correct type, determined by level of the spell
     * really simple to start out with
     */

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There isn't enough room in here to summon that.\n\r", ch);
        return;
    }
    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }
    if (level <= 5) {
        mob = read_mobile(16034, VIRTUAL);
#if 0
        rnum = number(1, 10) + 200;
        /*
         * removed this plot of mobs
         */
        mob = read_mobile(rnum, VIRTUAL);
#endif
    } else if (level <= 7) {
        mob = read_mobile(9191, VIRTUAL);
#if 0
        rnum = number(1, 10) + 210;
        /*
         * removed this plot of mobs
         */
        mob = read_mobile(rnum, VIRTUAL);
#endif
    } else if (level <= 9) {
        rnum = number(1, 10) + 220;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 11) {
        rnum = number(1, 10) + 230;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 13) {
        rnum = number(1, 10) + 240;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 15) {
        rnum = 250 + number(1, 10);
        mob = read_mobile(rnum, VIRTUAL);
    } else {
        rnum = 260 + number(1, 10);
        mob = read_mobile(rnum, VIRTUAL);
    }

    if (!mob) {
        send_to_char("The summons failed\n\r", ch);
        return;
    }

    /*
     * lets make monsum stronger.. they are pussies... (GH)
     */
    GET_DAMROLL(mob) += (int) level / 2;
    GET_HITROLL(mob) += (int) level / 2;
    mob->points.max_hit += level * 5;
#if 0
    GET_MAX_HIT(mob) += level*2;
#endif
    char_to_room(mob, ch->in_room);
    act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
    act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

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

        if (GET_CHR(ch)) {
            if (IS_PC(ch) || ch->master) {
                af.duration = follow_time(ch);
                af.modifier = 0;
                af.location = 0;
                af.bitvector = AFF_CHARM;
                affect_to_char(mob, &af);
            } else {
                SET_BIT(mob->specials.affected_by, AFF_CHARM);
            }
        } else {
            af.duration = 1;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        }
    }

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

void cast_mon_sum1(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(5, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum2(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(7, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum3(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(9, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum4(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(11, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum5(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(13, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum6(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(15, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}

void cast_mon_sum7(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_Create_Monster(17, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in monster_summoning_1");
        break;
    }
}


#define MOUNT_ONE 65
#define MOUNT_GOOD 69
#define MOUNT_EVIL 70
#define MOUNT_NEUT 71

void spell_mount(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct char_data *m;
    int             mnr;

    /*
     * create a ridable mount, and automatically mount said creature
     */

    mnr = MOUNT_ONE;
    if (level < 30) {
        if (level < 12) {
            mnr++;
        }
        if (level < 18) {
            mnr++;
        }
        if (level < 24) {
            mnr++;
        }
    } else {
        if (IS_EVIL(ch)) {
            mnr = MOUNT_EVIL;
        } else if (IS_GOOD(ch)) {
            mnr = MOUNT_GOOD;
        } else {
            mnr = MOUNT_NEUT;
        }
    }

    m = read_mobile(mnr, VIRTUAL);
    if (m) {
        char_to_room(m, ch->in_room);
        act("In a flash of light, $N appears", FALSE, ch, 0, m, TO_CHAR);
        act("In a flash of light, $N appears, and $n hops on $s back",
            FALSE, ch, 0, m, TO_ROOM);
        send_to_char("You hop on your mount's back\n\r", ch);
        MOUNTED(ch) = m;
        RIDDEN(m) = ch;
        GET_POS(ch) = POSITION_MOUNTED;
    } else {
        send_to_char("horses aren't in database\n\r", ch);
        return;
    }
}

void cast_mount(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_mount(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in mount.");
        break;
    }
}

void spell_paralyze(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
        if (IsImmune(victim, IMM_HOLD)) {
            FailPara(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_HOLD)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailPara(victim, ch);
                return;
            }
            if (saves_spell(victim, SAVING_PARA)) {
                FailPara(victim, ch);
                return;
            }
        }

        if (!IsSusc(victim, IMM_HOLD) && saves_spell(victim, SAVING_PARA)) {
            FailPara(victim, ch);
            return;
        } else if (IsSusc(victim, IMM_HOLD) &&
                   saves_spell(victim, SAVING_PARA) &&
                   saves_spell(victim, SAVING_PARA)) {
            FailPara(victim, ch);
            return;
        }

        af.type = SPELL_PARALYSIS;
        af.duration = 4 + level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_PARALYSIS;
        affect_join(victim, &af, FALSE, FALSE);

        act("Your limbs freeze in place", FALSE, victim, 0, 0, TO_CHAR);
        act("$n is paralyzed!", TRUE, victim, 0, 0, TO_ROOM);
        GET_POS(victim) = POSITION_STUNNED;
    } else {
        send_to_char("Someone tries to paralyze you AGAIN!\n\r", victim);
    }
}

void cast_paralyze(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_paralyze(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_paralyze(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_paralyze(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_paralyze(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_paralyze(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in paralyze");
        break;
    }
}

void spell_poly_self(int level, struct char_data *ch,
                     struct char_data *mob, struct obj_data *obj)
{
    char           *buf;

    /*
     * Checking for No_Poly system flag) - Manwe Windmaster 97/03/07
     */

    if (IS_SET(SystemFlags, SYS_NO_POLY)) {
        send_to_char("The gods prevent you from changing your shape", ch);
        extract_char(mob);
        return;
    }
    /*
     *  Check to make sure that there is no snooping going on.
     */
    if (!ch->desc || ch->desc->snoop.snooping) {
        send_to_char("Godly interference prevents the spell from working.", ch);
        extract_char(mob);
        return;
    }
    if (ch->desc->snoop.snoop_by) {
        /*
         * force the snooper to stop
         */
        do_snoop(ch->desc->snoop.snoop_by, GET_NAME(ch->desc->snoop.snoop_by),
                 0);
    }
    /*
     * Put mobile in correct room
     */
    char_to_room(mob, ch->in_room);
    SwitchStuff(ch, mob);
    /*
     *  move char to storage
     */
    act("$n's flesh melts and flows into the shape of $N.", TRUE, ch, 0, mob,
        TO_ROOM);
    act("Your flesh melts and flows into the shape of $N.", TRUE, ch, 0, mob,
        TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, 3);
    /*
     * stop following whoever you are following..
     */
    if (ch->master) {
        stop_follower(ch);
    }
    /*
     *  switch caster into mobile
     */
    ch->desc->character = mob;
    ch->desc->original = ch;
    mob->desc = ch->desc;
    ch->desc = 0;
    SET_BIT(mob->specials.act, ACT_POLYSELF);
    SET_BIT(mob->specials.act, ACT_NICE_THIEF);
    SET_BIT(mob->specials.act, ACT_SENTINEL);
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    REMOVE_BIT(mob->specials.act, ACT_META_AGG);
    REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
    /*
     * Why set current mana at max 85?
     */
    GET_MANA(mob) = MIN((GET_MANA(mob) - 15), 85);
    WAIT_STATE(mob, PULSE_VIOLENCE * 2);
    /*
     * do some fiddling with the strings
     */
    buf = (char *) malloc(strlen(GET_NAME(mob)) + strlen(GET_NAME(ch)) + 2);
    sprintf(buf, "%s %s", GET_NAME(ch), GET_NAME(mob));

    if (GET_NAME(mob)) {
        free(GET_NAME(mob));
    }

    GET_NAME(mob) = buf;
    buf = (char *) malloc(strlen(mob->player.short_descr) +
                          strlen(GET_NAME(ch)) + 2);
    sprintf(buf, "%s %s", GET_NAME(ch), mob->player.short_descr);

    if (mob->player.short_descr) {
        free(mob->player.short_descr);
    }
    mob->player.short_descr = buf;

#if 0
    buf = (char *)malloc(strlen(mob->player.short_descr)+12);
    sprintf(buf, "%s is here\n\r", mob->player.short_descr);
#endif

    if (mob->player.long_descr) {
        free(mob->player.long_descr);
    }

#if 0
    mob->player.long_descr = buf;
#endif

    /*
     * prettied up the way polies look in the room
     */
    mob->player.long_descr = NULL;
}


#define MAX_MAGE_POLY 48        /* total number of polies choices */

struct PolyType PolyList[MAX_MAGE_POLY] = {
    {"goblin", 4, 5100},
    {"parrot", 4, 9001},
    {"bat", 4, 7001},
    {"orc", 5, 4005},
    {"trog", 5, 9210},
    {"gnoll", 6, 9211},
    {"parrot", 6, 9010},
    {"lizard", 6, 224},
    {"ogre", 8, 4113},
    {"parrot", 8, 9011},
    {"wolf", 8, 3094},
    {"spider", 9, 227},
    {"beast", 9, 242},
    {"minotaur", 9, 247},
    {"snake", 10, 249},
    {"bull", 10, 1008},
    {"warg", 10, 6100},
    {"rat", 11, 7002},
    {"sapling", 12, 1421},
    {"ogre-maji", 12, 257},
    {"black", 12, 230},
    {"giant", 13, 261},
    {"troll", 14, 4101},
    {"crocodile", 14, 5310},
    {"mindflayer", 14, 7202},
    {"bear", 16, 9024},
    {"blue", 16, 233},
    {"enfan", 18, 21001},
    {"lamia", 18, 5201},
    {"drider", 18, 5011},
    {"wererat", 19, 7203},
    {"wyvern", 20, 3752},
    {"mindflayer", 20, 7201},
    {"spider", 20, 20010},
#if 0
    {"snog", 22, 27008}, 
#endif
    {"roc", 22, 3724},
    {"mud", 23, 7000},
    {"enfan", 23, 21004},
    {"giant", 24, 9406},
    {"white", 26, 243},
    {"master", 28, 7200},
    {"red", 30, 7040},
    {"roo", 35, 27411},
    {"brontosaurus", 35, 21802},
    {"mulichort", 40, 15830},
    {"beholder", 45, 5200}      /* number 47 (48) */
};


#define LAST_POLY_MOB 46

void cast_poly_self(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    char           *buffer;
    int             mobn = 0,
                    X = LAST_POLY_MOB,
                    found = FALSE;
    struct char_data *mob;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char("Just what are you trying to do?\n\r", ch);
        return;
    }

    if (IS_NPC(ch)) {
        send_to_char("You don't really want to do that.\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
        while (!found) {
            if (PolyList[X].level > level) {
                X--;
            } else {
                if (!strcasecmp(PolyList[X].name, buffer)) {
                    mobn = PolyList[X].number;
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
        }
        
        mob = read_mobile(mobn, VIRTUAL);
        if (mob) {
            spell_poly_self(level, ch, mob, 0);
        } else {
            send_to_char("You couldn't summon an image of that creature\n\r",
                         ch);
        }
        break;

    default:
        Log("Problem in poly_self");
        break;
    }
}


#define PORTAL 31

void spell_portal(int level, struct char_data *ch,
                  struct char_data *tmp_ch, struct obj_data *obj)
{
    struct obj_data *tmp_obj;
    struct extra_descr_data *ed;
    struct room_data *rp,
                   *nrp;
    char            buf[512];
    char            str[180];

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    /*
     * check target room for legality.
     */
    rp = real_roomp(ch->in_room);
    tmp_obj = read_object(PORTAL, VIRTUAL);
    if (!rp || !tmp_obj) {
        send_to_char("The magic fails\n\r", ch);
        return;
    }

    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
        send_to_char("Eldritch wizardry obstructs thee.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There is no room in here to portal!\n\r", ch);
        return;
    }

    if (!(nrp = real_roomp(tmp_ch->in_room))) {
        sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
        Log(str);
        send_to_char("The magic cannot locate the target.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(tmp_ch->in_room)->room_flags, NO_SUM)) {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (!IsOnPmp(tmp_ch->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
        send_to_char("The planes are fuzzy, you cannot portal!\n", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_LINKDEAD(tmp_ch)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_IMMORTAL(tmp_ch)) {
        send_to_char("You can't portal to someone of that magnitude!\n\r", ch);
        return;
    }

    sprintf(buf, "Through the mists of the portal, you can faintly see %s",
            nrp->name);

    CREATE(ed, struct extra_descr_data, 1);
    ed->next = tmp_obj->ex_description;
    tmp_obj->ex_description = ed;
    CREATE(ed->keyword, char, strlen(tmp_obj->name) + 1);
    strcpy(ed->keyword, tmp_obj->name);
    ed->description = strdup(buf);

    tmp_obj->obj_flags.value[0] = level / 5;
    tmp_obj->obj_flags.value[1] = tmp_ch->in_room;

    obj_to_room(tmp_obj, ch->in_room);

    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void cast_portal(int level, struct char_data *ch, char *arg,
                 int type, struct char_data *tar_ch,
                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_portal(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in portal");
        break;
    }
}

void spell_pword_blind(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    if (GET_MAX_HIT(victim) <= 100 || GetMaxLevel(ch) > 53) {
        SET_BIT(victim->specials.affected_by, AFF_BLIND);
    } else {
        send_to_char("They are too powerful to blind this way\n\r", ch);
    }
}

void cast_pword_blind(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_pword_blind(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in pword_blind.");
        break;
    }
}

void spell_pword_kill(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             max = 80;

    max += level;
    max += level / 2;

    if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > 53) {
        damage(ch, victim, GET_MAX_HIT(victim) * 12, SPELL_PWORD_KILL);
    } else {
        send_to_char("They are too powerful to destroy this way\n\r", ch);
    }
}

void cast_pword_kill(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_pword_kill(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in pword_kill.");
        break;
    }
}

void spell_prismatic_spray(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct char_data *tmp_victim;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    /*
     * spell effects:
     * red 1 40 dam
     * orange 2 80
     * yellow 3 100
     * green 4 poison
     * blue 5 lightning
     * indigo 6 feeble
     * violet 7 petrify
     */

    send_to_char
        ("You send a rainbow of prismatic spray from you hands.\n\r", ch);
    act("$n sends a rainbow of prismatic spray from $s hands!", FALSE, ch,
        0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people;
         tmp_victim; tmp_victim = tmp_victim->next_in_room) {
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                switch (number(1, 7)) {
                case 1:
                    dam = 40;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by a red shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with a red shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with a red shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                case 2:
                    dam = 80;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                case 3:
                    dam = 100;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by a yellow shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with a yellow shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with a yellow shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                    /*
                     * these have internal saves already
                     */
                case 4:
                    act("$N is hit by a green shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a green shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a yellow shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_poison(level, ch, "", SPELL_TYPE_SPELL,
                                tmp_victim, NULL);
                    break;
                case 5:
                    act("$N is hit by a blue shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a blue shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a blue shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_lightning_bolt(level, ch, "", SPELL_TYPE_SPELL,
                                  tmp_victim, NULL);
                    break;
                case 6:
                    act("$N is hit by an indigo shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with an indigo shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with an indigo shaft of light!",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    cast_feeblemind(level, ch, "", SPELL_TYPE_SPELL,
                                    tmp_victim, NULL);
                    break;
                case 7:
                    act("$N is hit by a violet shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a violet shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a violet shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_paralyze(level, ch, "", SPELL_TYPE_SPELL,
                                  tmp_victim, NULL);
                    break;
                default:
                    /*
                     * should never get here
                     */
                    dam = 100;
                    MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY);
                    break;
                }
                if (!tmp_victim->specials.fighting &&
                    ch->specials.fighting != tmp_victim) {
                    set_fighting(ch, tmp_victim);
                }
            } else {
                act("You avoid the colorful spray!", FALSE, ch, 0,
                    tmp_victim, TO_VICT);
            }
            act("$N avoids the colorful spray!", FALSE, ch, 0, tmp_victim,
                TO_NOTVICT);
            act("$N avoids your colorful spray!", FALSE, ch, 0, tmp_victim,
                TO_CHAR);
        }
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

void spell_scare(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    if (GetMaxLevel(victim) <= 5) {
        do_flee(victim, "\0", 0);
    }
}

void cast_scare(int level, struct char_data *ch, char *arg,
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
        spell_scare(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in scare.");
        break;
    }
}

void spell_second_wind(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct affected_type *k;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 8) + level;

    if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
        GET_MOVE(victim) = move_limit(victim);
    } else {
        GET_MOVE(victim) += dam;
    }
    if (affected_by_spell(victim, COND_WINGS_TIRED)) {
        affect_from_char(victim, COND_WINGS_TIRED);
    }
    if (affected_by_spell(victim, COND_WINGS_FLY)) {
        for (k = victim->affected; k; k = k->next) {
            if (k->type == COND_WINGS_FLY) {
                k->duration = GET_CON(victim);
            }
        }
    }
    send_to_char("You feel less tired\n\r", victim);
}

void cast_second_wind(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_second_wind(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_second_wind(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_second_wind(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in second_wind!");
        break;
    }
}

void do_sending(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *target;
    int             skill_check = 0;
    char           *target_name,
                    buf[1024],
                   *message;

    if (!ch->skills) {
        return;
    }
    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SPELL_SENDING].learned &&
        !ch->skills[SPELL_MESSENGER].learned) {
        send_to_char("You are unable to use this skill.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) < 5) && !IS_IMMORTAL(ch)) {
        send_to_char("You don't have the power to do that.\n\r", ch);
        return;
    }

    if (ch->skills[SPELL_SENDING].learned >
        ch->skills[SPELL_MESSENGER].learned) {
        skill_check = ch->skills[SPELL_SENDING].learned;
    } else {
        skill_check = ch->skills[SPELL_MESSENGER].learned;
    }

    if (skill_check < number(1, 101)) {
        send_to_char("You fumble and screw up the spell.\n\r", ch);
        if (!IS_IMMORTAL(ch)) {
            GET_MANA(ch) -= 3;
        }
        if (ch->skills[SPELL_SENDING].learned >
            ch->skills[SPELL_MESSENGER].learned) {
            LearnFromMistake(ch, SPELL_SENDING, 0, 95);
        } else {
            LearnFromMistake(ch, SPELL_MESSENGER, 0, 95);
        }
        return;
    }

    if (!IS_IMMORTAL(ch)) {
        GET_MANA(ch) -= 5;
    }

    argument = get_argument(argument, &target_name);
    message = skip_spaces(argument);

    if (!target_name || !(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if (IS_NPC(target) && !IS_SET(target->specials.act, ACT_POLYSELF)) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if (check_soundproof(target)) {
        send_to_char("In a silenced room, try again later.\n\r", ch);
        return;
    }

    if (IS_SET(target->specials.act, PLR_NOTELL)) {
        send_to_char("They are ignoring messages at this time.\n\r", ch);
        return;
    }

    if( !message ) {
        send_to_char("You seem to want to send a message, but what?\n\r", ch);
        return;
    }

    sprintf(buf, "$n sends you a mystic message:$c0013 %s", message);
    act(buf, TRUE, ch, 0, target, TO_VICT);
    sprintf(buf, "You send $N%s the message:$c0013 %s",
            (IS_AFFECTED2(target, AFF2_AFK) ? " (Who is AFK)" : ""), message);
    act(buf, TRUE, ch, 0, target, TO_CHAR);
}

void spell_shield(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!affected_by_spell(victim, SPELL_SHIELD)) {
        act("$N is surrounded by a strong force shield.",
            TRUE, ch, 0, victim, TO_NOTVICT);
        if (ch != victim) {
            act("$N is surrounded by a strong force shield.",
                TRUE, ch, 0, victim, TO_CHAR);
            act("You are surrounded by a strong force shield.",
                TRUE, ch, 0, victim, TO_VICT);
        } else {
            act("You are surrounded by a strong force shield.",
                TRUE, ch, 0, 0, TO_CHAR);
        }

        af.type = SPELL_SHIELD;
        af.duration = 8 + level;
        af.modifier = -10;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}

void cast_shield(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_shield(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_shield(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_shield(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_shield(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in shield!");
        break;
    }
}

void spell_shocking_grasp(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = number(1, 8) + level;

    if ((GET_HIT(victim) < -4) && IsHumanoid(victim) && !IsUndead(victim)) {
        act("$n utters the words 'clear', and touches $N's chest",
            FALSE, ch, 0, victim, TO_ROOM);
        GET_HIT(victim) += dam;
        update_pos(victim);
        return;
    }

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
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

void spell_silence(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    extern struct index_data *mob_index;

    assert(ch && victim);

    if (!saves_spell(victim, SAVING_SPELL)) {
        act("$n ceases to make noise!", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You can't hear anything!.\n\r", victim);

        af.type = SPELL_SILENCE;
        af.duration = level;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SILENCE;
        affect_to_char(victim, &af);
    } else {
        send_to_char("You feel quiet for a moment, but the effect fades\n\r",
                     victim);
        if (!IS_PC(victim) && !victim->specials.fighting) {
            set_fighting(victim, ch);
            if (mob_index[victim->nr].func) {
                (*mob_index[victim->nr].func) (victim, 0, "");
            }
        }
    }
}

void cast_silence(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
        spell_silence(level, ch, ch, 0);
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_silence(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch) && !IS_IMMORTAL(tar_ch)) {
                spell_silence(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in silence.");
        break;
    }
}

void spell_sleep(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    assert(victim);

    if (IsImmune(victim, IMM_SLEEP)) {
        FailSleep(victim, ch);
        return;
    }
    if (IsResist(victim, IMM_SLEEP)) {
        if (saves_spell(victim, SAVING_SPELL)) {
            FailSleep(victim, ch);
            return;
        }
        if (saves_spell(victim, SAVING_SPELL)) {
            FailSleep(victim, ch);
            return;
        }
    } else if (!IsSusc(victim, IMM_SLEEP) &&
               saves_spell(victim, SAVING_SPELL)) {
        FailSleep(victim, ch);
        return;
    }

    af.type = SPELL_SLEEP;
    af.duration = 4 + level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim) > POSITION_SLEEPING) {
        act("You feel very sleepy ..... zzzzzz", FALSE, victim, 0, 0, TO_CHAR);
        act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
        GET_POS(victim) = POSITION_SLEEPING;
    }

    /*
     * make fighting between ch and victim stop, so ws:sleep items have a use
     */
    if (victim->specials.fighting) {
        stop_fighting(victim);
    }
    if (ch->specials.fighting == victim) {
        stop_fighting(ch);
    }
}

void cast_sleep(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_sleep(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_sleep(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_sleep(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_sleep(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_sleep(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in sleep!");
        break;
    }
}

void spell_slow(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_SLOW)) {
        act("$N is already slowed.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (affected_by_spell(victim, SPELL_HASTE)) {
        affect_from_char(victim, SPELL_HASTE);
        act("$N doesnt seem so quick anymore.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_IMMUNE(victim, IMM_HOLD)) {
        act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
        act("$n just tried to slow you, but you ignored it.", FALSE, ch, 0,
            victim, TO_VICT);
        if (!in_group(ch, victim) && !IS_PC(ch)) {
            hit(victim, ch, TYPE_UNDEFINED);
        }
        return;
    }

    af.type = SPELL_SLOW;
    af.duration = 10;
    af.modifier = 1;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_SLOW;
    affect_to_char(victim, &af);

    send_to_char("You feel very slow!\n\r", victim);

    if (!IS_PC(victim) && !IS_SET(victim->specials.act, ACT_POLYSELF)) {
        hit(victim, ch, TYPE_UNDEFINED);
    }
}

void cast_slow(int level, struct char_data *ch, char *arg,
               int type, struct char_data *tar_ch,
               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_slow(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_slow(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_slow(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_slow(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in slow");
        break;
    }
}

void spell_strength(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_STRENGTH)) {
        act("You feel stronger.", FALSE, victim, 0, 0, TO_CHAR);
        act("$n seems stronger!", FALSE, victim, 0, 0, TO_ROOM);
        af.type = SPELL_STRENGTH;
        af.duration = 2 * level;
        if (IS_NPC(victim)) {
            if (level >= CREATOR) {
                af.modifier = 25 - GET_STR(victim);
            } else {
                af.modifier = number(1, 6);
            }
        } else if (HasClass(ch, CLASS_WARRIOR) || 
                   HasClass(ch, CLASS_RANGER) ||
                   HasClass(ch, CLASS_BARBARIAN) || 
                   HasClass(ch, CLASS_PALADIN)) {
            af.modifier = number(1, 8);
        } else if (HasClass(ch, CLASS_CLERIC) || 
                   HasClass(ch, CLASS_THIEF) ||
                   HasClass(ch, CLASS_PSI)) {
            af.modifier = number(1, 6);
        } else {
                af.modifier = number(1, 4);
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void cast_strength(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        spell_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_strength(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_strength(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in strength!");
        break;
    }
}

void spell_succor(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *o;

    o = read_object(3052, VIRTUAL);
    obj_to_char(o, ch);

    o->obj_flags.cost = 0;
    o->obj_flags.cost_per_day = -1;

    act("$n waves $s hand, and creates $p.", TRUE, ch, o, 0, TO_ROOM);
    act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);
}

void cast_succor(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_succor(level, ch, 0, 0);
    }
}

void spell_summon(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp;
    struct room_data *rp;
    int             count;

    assert(ch && victim);

    if (victim->in_room <= NOWHERE) {
        send_to_char("Couldn't find any of those.\n\r", ch);
        return;
    }

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
        send_to_char("Eldritch wizardry obstructs thee.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There is no room in here to summon!\n\r", ch);
        return;
    }

    if (rp->sector_type == SECT_AIR || rp->sector_type == SECT_UNDERWATER) {
        send_to_char("Strange powers block your summons\n", ch);
        return;
    }

    if (check_peaceful(ch, "Ancient powers obstruct thy magik.\n"))
        return;

    if (check_peaceful(victim, "")) {
        send_to_char("You cannot get past the magical defenses.\n\r", ch);
        return;
    }

    if (IS_PC(victim) && IS_LINKDEAD(victim)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(victim)) {
        send_to_char("A large hand suddenly appears before you and thumps "
                     "your head!\n\r", ch);
        return;
    }

    if (victim->specials.fighting) {
        send_to_char("You can't get a clear fix on them\n", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOSUMMON)) {
        send_to_char("A mystical fog blocks your attemps!\n", ch);
        return;
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n", ch);
        return;
    }

    if (CanFightEachOther(ch, victim) && saves_spell(victim, SAVING_SPELL)) {
        act("You failed to summon $N!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n tried to summon you!", FALSE, ch, 0, victim, TO_VICT);
        return;
    }

    if (!IS_PC(victim)) {
        count = 0;
        for (tmp = real_roomp(victim->in_room)->people;
             tmp; tmp = tmp->next_in_room) {
            count++;
        }

        if (count == 0) {
            send_to_char("You failed.\n\r", ch);
            return;
        } else {
            count = number(0, count);

            for (tmp = real_roomp(victim->in_room)->people;
                 count && tmp; tmp = tmp->next_in_room, count--) {
                /*
                 * Empty loop
                 */
            }

            if (tmp && GET_MAX_HIT(tmp) < GET_HIT(ch) &&
                !saves_spell(tmp, SAVING_SPELL)) {
                RawSummon(tmp, ch);
            } else {
                send_to_char("You failed\n\r", ch);
                return;
            }
        }
    } else {
        RawSummon(victim, ch);
    }
}

void cast_summon(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {

    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_summon(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in summon!");
        break;
    }
}

void spell_teleport(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             to_room,
                    try = 0;
    extern int      top_of_world;
    struct room_data *room;

    assert(ch && victim);

    if (victim != ch) {
        if (saves_spell(victim, SAVING_SPELL)) {
            send_to_char("Your spell has no effect.\n\r", ch);
            if (IS_NPC(victim)) {
                if (!victim->specials.fighting) {
                    set_fighting(victim, ch);
                }
            } else {
                send_to_char("You feel strange, but the effect fades.\n\r",
                             victim);
            }
            return;
        } else {
            /*
             * the character (target) is now the victim
             */
            ch = victim;
        }
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    do {
        to_room = number(0, top_of_world);
        room = real_roomp(to_room);
        if (room && ((IS_SET(room->room_flags, PRIVATE)) ||
                     (IS_SET(room->room_flags, TUNNEL)) ||
                     (IS_SET(room->room_flags, NO_SUM)) ||
                     (IS_SET(room->room_flags, NO_MAGIC)) ||
                     !IsOnPmp(to_room))) {
            room = 0;
            try++;
        }
    } while (!room && try < 10);

    if (try >= 10) {
        send_to_char("The magic fails.\n\r", ch);
        return;
    }

    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades in to existence.", FALSE, ch, 0, 0, TO_ROOM);

    do_look(ch, NULL, 0);

    if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
        !IS_IMMORTAL(ch)) {
        NailThisSucker(ch);
        return;
    }
    check_falling(ch);
}

void cast_teleport(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_teleport(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_teleport(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) {
                spell_teleport(level, ch, tar_ch, 0);
            }
        break;

    default:
        Log("Serious screw-up in teleport!");
        break;
    }
}

void cast_ventriloquate(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    struct char_data *tmp_ch;
    char            buf1[MAX_STRING_LENGTH];
    char            buf2[MAX_STRING_LENGTH];
    char            buf3[MAX_STRING_LENGTH];

    if (type != SPELL_TYPE_SPELL) {
        Log("Attempt to ventriloquate by non-cast-spell.");
        return;
    }

    arg = skip_spaces(arg);
    if( !arg ) {
        return;
    }

    if (tar_obj) {
        sprintf(buf1, "The %s says '%s'\n\r", fname(tar_obj->name), arg);
        sprintf(buf2, "Someone makes it sound like the %s says '%s'.\n\r",
                fname(tar_obj->name), arg);
    } else {
        sprintf(buf1, "%s says '%s'\n\r", GET_NAME(tar_ch), arg);
        sprintf(buf2, "Someone makes it sound like %s says '%s'\n\r",
                GET_NAME(tar_ch), arg);
    }

    sprintf(buf3, "Someone says, '%s'\n\r", arg);

    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) { 
        if ((tmp_ch != ch) && (tmp_ch != tar_ch)) {
            if (saves_spell(tmp_ch, SAVING_SPELL)) {
                send_to_char(buf2, tmp_ch);
            } else {
                send_to_char(buf1, tmp_ch);
            }
        } else if (tmp_ch == tar_ch) {
            send_to_char(buf3, tar_ch);
        }
    }
}

void spell_water_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
    if (victim != ch) {
        act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
    }
    act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type = SPELL_WATER_BREATH;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_WATERBREATH;
    affect_to_char(victim, &af);
}

void cast_water_breath(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_water_breath(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_water_breath(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_water_breath(level, ch, tar_ch, 0);
        break;

    default:
        Log("Serious screw-up in water breath");
        break;
    }
}

void spell_web(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    bool            fail,
                    pissed,
                    big,
                    no_effect;

    assert(ch && victim);

    big = pissed = fail = no_effect = FALSE;

    switch (GET_RACE(victim)) {
    case RACE_ARACHNID:
    case RACE_SLIME:
    case RACE_GHOST:
        act("$N laughs at the webs!", FALSE, ch, 0, victim, TO_CHAR);
        act("Hah, $n casted web on you, what a flake.", FALSE, ch, 0,
            victim, TO_VICT);
        act("$N laughs at $n's webs!", FALSE, ch, 0, victim, TO_NOTVICT);
        return;
        break;
    }

    if (!saves_spell(victim, SAVING_PARA)) {
        fail = TRUE;
    }
    if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
        big = TRUE;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        if (IsSmall(victim) && !fail && !number(0, 3)) {
            pissed = TRUE;
            /*
             * 25%
             */
        } else if (big) {
            if (fail) {
                if (number(0, 4) < 2) {
                    pissed = TRUE;
                    /*
                     * 40%
                     */
                }
            } else {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            }
        } else {
            if (fail) {
                if (!number(0, 4)) {
                    pissed = TRUE;
                    /*
                     * 20%
                     */
                }
            } else {
                if (!number(0, 2)) {
                    pissed = TRUE;
                    /*
                     * 33%
                     */
                }
            }
        }
    } else {
        /*
         * assume if not indoors, outdoors and web is less affective at
         * blocking the victim from the caster.
         */
        if (IsSmall(victim) && !fail && !number(0, 2)) {
            pissed = TRUE;
            /*
             * 33%
             */
        } else if (big) {
            if (fail) {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            } else {
                pissed = TRUE;
                /*
                 * 100%
                 */
            }
        } else {
            if (fail) {
                if (number(0, 4) < 2) {
                    pissed = TRUE;
                    /*
                     * 40%
                     */
                }
            } else {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            }
        }
    }

    if (fail) {
        af.type = SPELL_WEB;
        af.duration = level;
        af.modifier = -50;
        af.location = APPLY_MOVE;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        if (!pissed) {
            act("You are stuck in a sticky webbing!", FALSE, ch, 0, victim,
                TO_VICT);
            act("$N is stuck in a sticky webbing!", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You wrap $N in a sticky webbing!", FALSE, ch, 0, victim,
                TO_CHAR);
        } else {
            act("You are wrapped in webs, but they don't stop you!", FALSE,
                ch, 0, victim, TO_VICT);
            act("$N attacks, paying little heed to the webs that slow it.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You only manage to piss off $N with your webs, ack!",
                FALSE, ch, 0, victim, TO_CHAR);
        }
    } else {
        if (pissed) {
            act("You are almost caught in a sticky webbing, GRRRR!",
                FALSE, ch, 0, victim, TO_VICT);
            act("$N growls and dodges $n's sticky webbing!",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You miss $N with your sticky webbing!  Uh oh, it's mad.",
                FALSE, ch, 0, victim, TO_CHAR);
        } else {
            act("You watch with amusement as $n casts web about the room.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$n misses $N with the webs!", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You miss with your webs, but $N doesn't seem to notice.",
                FALSE, ch, 0, victim, TO_CHAR);
        }
    }
    if (pissed && IS_NPC(victim) && !victim->specials.fighting) {
        set_fighting(victim, ch);
    }
}

void cast_web(int level, struct char_data *ch, char *arg, int type,
              struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_web(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_web(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_web(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(ch, tar_ch)) {
                spell_web(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in web!");
        break;
    }
}

void spell_wizard_eye(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_WIZARDEYE)) {
        act("$n summons a disembodied eye to scout ahead", TRUE, victim, 0,
            0, TO_ROOM);
        act("You summon a disembodied eye to scout ahead.", TRUE, victim,
            0, 0, TO_CHAR);

        af.type = SPELL_WIZARDEYE;
        af.duration = (!IS_IMMORTAL(ch) ? 3 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SCRYING;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N already uses a wizard eye.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You already have a wizard eye.", FALSE, ch, 0, victim, TO_CHAR);
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


/* Main skills */

void spell_anti_magic_shell(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
        send_to_char("You erect a anti-magic globe about yourself.\n\r", ch);
        spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);

        af.type = SPELL_ANTI_MAGIC_SHELL;
        /*
         * one tic only!
         */
        af.duration = (!IS_IMMORTAL(ch) ? 1 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        send_to_char("You are already protected by a anti-magic shell.\n\r",
                     ch);
    }
}

void cast_anti_magic_shell(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_anti_magic_shell(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_anti_magic_shell(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_anti_magic_shell(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_anti_magic_shell!");
        break;
    }
}

void do_brew(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH],
                   *spellnm;
    struct obj_data *obj;
    int             sn = -1,
                    index,
                    formula = 0;

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!MainClass(ch, MAGE_LEVEL_IND) && !IS_IMMORTAL(ch)) {
        send_to_char("Alas, you can only dream of brewing your own "
                     "potions.\n\r", ch);
        return;
    }

    if (!ch->skills) {
        send_to_char("You don't seem to have any skills.\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_BREW].learned)) {
        send_to_char("You haven't been properly trained in the art of "
                     "brewing.\n\r", ch);
        return;
    }

    if (!argument) {
        send_to_char("What would you like to brew?\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 50 && !IS_IMMORTAL(ch)) {
        send_to_char("You don't have enough mana to brew that spell.\n\r", ch);
        return;
    }

    if (!(obj = read_object(EMPTY_POTION, VIRTUAL))) {
        Log("no default potion could be found for brew");
        send_to_char("woops, something's wrong.\n\r", ch);
        return;
    }

    argument = get_argument_delim(argument, &spellnm, '\'');

    /*
     * Check for beginning qoute 
     */
    if (!spellnm || spellnm[-1] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    sn = old_search_block(spellnm, 0, strlen(spellnm), spells, 0);
    sn = sn - 1;

    if (sn == -1) {
        /* 
         * no spell found?
         */
        send_to_char("Brew what??.\n\r", ch);
        return;
    }

    index = spell_index[sn + 1];
    if (!ch->skills[sn + 1].learned || index == -1) {
        /*
         * do you know that spell?
         */
        send_to_char("You don't know of this spell.\n\r", ch);
        return;
    }

    if (!(spell_info[index].min_level_magic)) {
        /*
         * is it a mage spell?
         */
        send_to_char("You cannot concoct that spell!\n\r", ch);
        return;
    }

    if (spell_info[index].brewable == 0 && !IS_IMMORTAL(ch)) {
        send_to_char("You can't brew that spell.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < spell_info[index].min_usesmana * 2 && 
        !IS_IMMORTAL(ch)) {
        send_to_char("You don't have enough mana to brew that spell.\n\r", ch);
        return;
    }

    act("$n begins preparing a potion.", TRUE, ch, obj, NULL, TO_ROOM);
    act("You start preparing a potion.", TRUE, ch, obj, NULL, TO_CHAR);

    formula = ((ch->skills[SKILL_BREW].learned) + GET_INT(ch) / 3) +
              (GET_WIS(ch) / 3);

    if (formula > 98) {
        formula = 98;
    }
    if ((number(1, 101) >= formula || ch->skills[SKILL_BREW].learned < 10) &&
        !IS_IMMORTAL(ch)) {
        WAIT_STATE(ch, PULSE_VIOLENCE * 5);
        act("$p explodes violently!", TRUE, ch, obj, NULL, TO_CHAR);
        act("$p explodes violently!", TRUE, ch, obj, NULL, TO_ROOM);
        GET_HIT(ch) -= spell_info[index].min_level_magic;
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        act("$n screams in pain as $p exploded on $m.", TRUE, ch, obj,
            NULL, TO_ROOM);
        act("You scream in pain as $p explodes.", TRUE, ch, obj, NULL, TO_CHAR);
        LearnFromMistake(ch, SKILL_BREW, 0, 90);
        extract_obj(obj);
    } else {
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        sprintf(buf, "You have imbued a new spell to %s.\n\r",
                obj->short_description);
        send_to_char(buf, ch);
        send_to_char("The brew was a success!\n\r", ch);

        if (obj->short_description) {
            free(obj->short_description);
        }
        sprintf(buf, "a potion of %s", spells[sn]);
        obj->short_description = (char *) strdup(buf);

        if (obj->name) {
            free(obj->name);
        }
        sprintf(buf, "potion %s", spells[sn]);
        obj->name = (char *) strdup(buf);

        if (obj->description) {
            free(obj->description);
        }
        sprintf(buf, "%s", "A weird coloured potion is on the ground.");
        obj->description = (char *) strdup(buf);

        if (IS_IMMORTAL(ch)) {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = MAX_MORT;
        } else {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = GetMaxLevel(ch);
        }

        /*
         * set spell in slot.
         */
        obj->obj_flags.value[1] = sn + 1;
        obj->obj_flags.timer = 60;

        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        obj_to_char(obj, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

void spell_mana_shield(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_MANA_SHIELD)) {
        act("$c000y$n summons forth a $c000Ygolden $c000yglobe to float next "
            "to $s head.", TRUE, ch, 0, 0, TO_ROOM);
        act("$c000yYou summon forth a $c000Ygolden $c000yglobe to float next "
            "to your head.", TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_MANA_SHIELD;
        af.duration = (!IS_IMMORTAL(ch) ? 3 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void cast_mana_shield(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_mana_shield(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_mana_shield");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
