
/*
 * All Cleric skills and spells
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

extern struct time_info_data time_info;


void spell_aid(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    /*
     * combo bless, cure light woundsish
     */
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_AID)) {
        send_to_char("Already in effect\n\r", ch);
        return;
    }

    GET_HIT(victim) += number(1, 8);

    update_pos(victim);

    act("$n looks aided", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel better!\n\r", victim);

    af.type = SPELL_AID;
    af.duration = 10;
    af.modifier = 1;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
}

void cast_aid(int level, struct char_data *ch, char *arg,
              int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_aid(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in scare.");
        break;
    }
}
#if 0
void spell_animate_dead(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *corpse)
{
    struct char_data *mob;
    struct obj_data *obj_object,
                   *next_obj;
    char            buf[MAX_STRING_LENGTH];
    /*
     * virtual # for zombie
     */
    int             r_num = 100;

    /*
     * some sort of check for corpse hood
     */
    if (GET_ITEM_TYPE(corpse) != ITEM_CONTAINER ||
        !corpse->obj_flags.value[3]) {
        send_to_char("The magic fails abruptly!\n\r", ch);
        return;
    }

    mob = read_mobile(r_num, VIRTUAL);
    char_to_room(mob, ch->in_room);

    act("With mystic power, $n animates a corpse.", TRUE, ch, 0, 0, TO_ROOM);
    act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);
    act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_CHAR);
    /*
     * zombie should be charmed and follower ch
     */

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, mob, TO_CHAR);
        act("You take one look at the size of $n's posse and just say no!",
            TRUE, ch, 0, mob, TO_ROOM);
    } else {
        SET_BIT(mob->specials.affected_by, AFF_CHARM);
        add_follower(mob, ch);
    }

    GET_EXP(mob) = 0;
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;

    /*
     * take all from corpse, and give to zombie
     */

    for (obj_object = corpse->contains; obj_object; obj_object = next_obj) {
        next_obj = obj_object->next_content;
        obj_from_obj(obj_object);
        obj_to_char(obj_object, mob);
    }
    /*
     * set up descriptions and such
     */
    sprintf(buf, "%s is here, slowly animating\n\r", corpse->short_description);
    mob->player.long_descr = (char *) strdup(buf);
    /*
     * set up hitpoints
     */
    mob->points.max_hit = dice(MAX(level / 2, 5), 8);
    mob->points.hit = mob->points.max_hit / 2;
    mob->player.sex = 0;
    GET_RACE(mob) = RACE_UNDEAD_ZOMBIE;
    mob->player.class = ch->player.class;
    /*
     * get rid of corpse
     */
    extract_obj(corpse);
}

void cast_animate_dead(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{

    struct obj_data *i;

    if (NoSummon(ch)) {
        return;
    }
    switch (type) {

    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            if (IS_CORPSE(tar_obj)) {
                spell_animate_dead(level, ch, 0, tar_obj);
            } else {
                send_to_char("That's not a corpse!\n\r", ch);
                return;
            }
        } else {
            send_to_char("That isn't a corpse!\n\r", ch);
            return;
        }
        break;
    case SPELL_TYPE_POTION:
        send_to_char("Your body revolts against the magic liquid.\n\r", ch);
        ch->points.hit = 0;
        break;
    case SPELL_TYPE_STAFF:
        for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
            if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
                spell_animate_dead(level, ch, 0, i);
            }
        }
        break;
    default:
        Log("Serious screw-up in animate_dead!");
        break;
    }
}
#endif
#if 0
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
#endif
#define ASTRAL_ENTRANCE 2701
void spell_astral_walk(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp,
                   *tmp2;
    struct room_data *rp;

    if (IS_SET(SystemFlags, SYS_NOASTRAL)) {
        send_to_char("The astral planes are shifting, you cannot!\n", ch);
        return;
    }

    rp = real_roomp(ch->in_room);

    for (tmp = rp->people; tmp; tmp = tmp2) {
        tmp2 = tmp->next_in_room;
        if (in_group(ch, tmp) && !tmp->specials.fighting &&
            (IS_PC(tmp) || IS_SET(tmp->specials.act, ACT_POLYSELF)) &&
            IS_AFFECTED(tmp, AFF_GROUP)) {
            act("$n wavers, fades and dissappears", FALSE, tmp, 0, 0, TO_ROOM);
            char_from_room(tmp);
            char_to_room(tmp, ASTRAL_ENTRANCE);
            do_look(tmp, NULL, 0);
        }
    }
}

void cast_astral_walk(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
        if (!tar_ch) {
            tar_ch = ch;
        } else {
            spell_astral_walk(level, ch, tar_ch, 0);
        }
        break;
    default:
        Log("Serious screw-up in astral walk!");
        break;
    }
}

void spell_blade_barrier(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        if (affected_by_spell(ch, SPELL_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("Your blades rip away the last vistiges of the fireshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_FIRESHIELD);
        }

        if (IS_AFFECTED(ch, AFF_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("Your blades rip away the last vistiges of the fireshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_FIRESHIELD);
        }

        if (affected_by_spell(ch, SPELL_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, ch, 0, 0,
                TO_ROOM);
            act("Your blades rip away the last vistiges of the chillshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_CHILLSHIELD);
        }
        if (IS_AFFECTED(ch, AFF_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, ch, 0, 0,
                TO_ROOM);
            act("Your blades rip away the last vistiges of the chillshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_CHILLSHIELD);
        }

        act("$c000B$n is surrounded by a barrier of whirling blades.",
            TRUE, ch, 0, 0, TO_ROOM);
        act("$c000BYou summon a barrier of whirling blades around you.",
            TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_BLADE_BARRIER;

        if( IS_IMMORTAL(ch) ) {
            af.duration = level;
        } else {
            af.duration = (ch->specials.remortclass == CLERIC_LEVEL_IND + 1 ? 
                           4 : 3 );
        }
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_BLADE_BARRIER;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void cast_blade_barrier(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_blade_barrier(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_blade_barrier");
        break;
    }
}

void spell_bless(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && (victim || obj));

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (GET_POS(victim) != POSITION_FIGHTING &&
        !affected_by_spell(victim, SPELL_BLESS)) {

        send_to_char("You feel righteous.\n\r", victim);
        af.type = SPELL_BLESS;
        af.duration = 6;
        af.modifier = 1;
        af.location = APPLY_HITROLL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.location = APPLY_SAVING_SPELL;
        af.modifier = -1;
        affect_to_char(victim, &af);
    }
}

void cast_bless(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{

    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_obj) {
            /* It's an object */
            return;
        } 

        /* Then it is a PC | NPC */
        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            GET_POS(tar_ch) == POSITION_FIGHTING) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_bless(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_BLESS) ||
            GET_POS(ch) == POSITION_FIGHTING)
            return;
        spell_bless(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            /* It's an object */
            return;
        } 
        
        /* Then it is a PC | NPC */
        if (!tar_ch)
            tar_ch = ch;

        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING))
            return;
        spell_bless(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            /* It's an object */
            return;
        }
        
        /* Then it is a PC | NPC */
        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING))
            return;
        spell_bless(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in bless!");
        break;
    }
}
#if 0
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
        act("$n is allready blind!", TRUE, victim, 0, 0, TO_CHAR);
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
#endif
#if 0
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
#endif
void spell_call_lightning(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    extern struct weather_data weather_info;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level + 2, 8);

    if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        if (saves_spell(victim, SAVING_SPELL)) {
            dam >>= 1;
        }
        MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING);
    } else {
        send_to_char("The proper atmospheric conditions are not at hand.\n\r",
                     ch);
        return;
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
#if 0
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
#endif
void spell_cause_critical(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(3, 8) + 3;

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);
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

void spell_cause_light(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(1, 8);

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
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

void spell_cause_serious(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + 2;

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);
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

void cast_command(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    char           *p;
    char            buf[128];
    /*
     * have to parse the argument to get the command 
     */

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        break;
    default:
        Log("serious screw-up in command.");
        return;
        break;
    }

    arg = skip_spaces(arg);
    if (arg) {
        p = fname(arg);

        if (((GetMaxLevel(tar_ch) < 6 && GET_INT(tar_ch) < 13) ||
             !saves_spell(tar_ch, SAVING_PARA)) && strcmp(p, "quit")) {
            sprintf(buf, "$n has commanded you to '%s'.", p);
            act(buf, FALSE, ch, 0, tar_ch, TO_VICT);
            send_to_char("Ok.\n\r", ch);
            command_interpreter(tar_ch, p);
            return;
        }

        sprintf(buf, "$n just tried to command you to '%s'.", p);
        act(buf, FALSE, ch, 0, tar_ch, TO_VICT);

        if (!IS_PC(tar_ch)) {
            hit(tar_ch, ch, TYPE_UNDEFINED);
        }
    }
}
#if 0
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
#endif
#if 0
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
#endif
#if 0
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
#endif

void cast_control_weather(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    char           *arg1;

    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        arg = get_argument(arg, &arg1);

        if (arg1 && strcmp("better", arg1) && strcmp("worse", arg1)) {
            send_to_char("Do you want it to get better or worse?\n\r", ch);
            return;
        }

        if (!OUTSIDE(ch)) {
            send_to_char("You need to be outside.\n\r", ch);
        }

        if (!strcmp("better", arg1)) {
            if (weather_info.sky == SKY_CLOUDLESS) {
                return;
            }
            if (weather_info.sky == SKY_CLOUDY) {
                send_to_outdoor("The clouds disappear.\n\r");
                weather_info.sky = SKY_CLOUDLESS;
            }
            if (weather_info.sky == SKY_RAINING) {
                if (time_info.month > 3 && time_info.month < 14) {
                    send_to_outdoor("The rain has stopped.\n\r");
                } else {
                    send_to_outdoor("The snow has stopped. \n\r");
                }
                weather_info.sky = SKY_CLOUDY;
            }
            if (weather_info.sky == SKY_LIGHTNING) {
                if (time_info.month > 3 && time_info.month < 14) {
                    send_to_outdoor("The lightning has gone, but it is still "
                                    "raining.\n\r");
                } else {
                    send_to_outdoor("The blizzard is over, but it is still "
                                    "snowing.\n\r");
                }
                weather_info.sky = SKY_RAINING;
            }
            return;
        }
        
        if (weather_info.sky == SKY_CLOUDLESS) {
            send_to_outdoor("The sky is getting cloudy.\n\r");
            weather_info.sky = SKY_CLOUDY;
            return;
        }

        if (weather_info.sky == SKY_CLOUDY) {
            if (time_info.month > 3 && time_info.month < 14) {
                send_to_outdoor("It starts to rain.\n\r");
            } else {
                send_to_outdoor("It starts to snow. \n\r");
            }
            weather_info.sky = SKY_RAINING;
        }

        if (weather_info.sky == SKY_RAINING) {
            if (time_info.month > 3 && time_info.month < 14) {
                send_to_outdoor("You are caught in lightning storm.\n\r");
            } else {
                send_to_outdoor("You are caught in a blizzard. \n\r");
            }
            weather_info.sky = SKY_LIGHTNING;
        }
        break;

    default:
        Log("Serious screw-up in control weather!");
        break;
    }
}

void spell_create_food(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);

    tmp_obj->name = (char *) strdup("mushroom");
    tmp_obj->short_description = (char *) strdup("A Magic Mushroom");
    tmp_obj->description = (char *)strdup("A really delicious looking magic"
                                          " mushroom lies here.");

    tmp_obj->obj_flags.type_flag = ITEM_FOOD;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->obj_flags.value[0] = 5 + level;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;
    tmp_obj->obj_flags.cost_per_day = 1;

    tmp_obj->next = object_list;
    object_list = tmp_obj;

    obj_to_room(tmp_obj, ch->in_room);

    tmp_obj->item_number = -1;

    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void cast_create_food(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        act("$n magically creates a mushroom.", FALSE, ch, 0, 0, TO_ROOM);
        spell_create_food(level, ch, 0, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (tar_ch) {
            return;
        }
        spell_create_food(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in create food!");
        break;
    }
}
#if 0
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
#endif

void spell_create_water(int level, struct char_data *ch,
			struct char_data *victim, struct obj_data *obj)
{
    int             water;

    extern struct weather_data weather_info;
    void            name_to_drinkcon(struct obj_data *obj, int type);
    void            name_from_drinkcon(struct obj_data *obj);

    assert(ch && obj);

    if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
	if (obj->obj_flags.value[2] != LIQ_WATER &&
	    obj->obj_flags.value[1] != 0) {

	    name_from_drinkcon(obj);
	    obj->obj_flags.value[2] = LIQ_SLIME;
	    name_to_drinkcon(obj, LIQ_SLIME);
	} else {
	    water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

	    /*
	     * Calculate water it can contain, or water created
	     */
	    water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1],
			water);

	    if (water > 0) {
		obj->obj_flags.value[2] = LIQ_WATER;
		obj->obj_flags.value[1] += water;

		weight_change_object(obj, water);

		name_from_drinkcon(obj);
		name_to_drinkcon(obj, LIQ_WATER);
		act("$p is partially filled.", FALSE, ch, obj, 0, TO_CHAR);
	    }
	}
    }
}

void cast_create_water(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
            send_to_char("It is unable to hold water.\n\r", ch);
            return;
        }
        spell_create_water(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in create water!");
        break;
    }
}

void spell_cure_blind(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    if (!victim || level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (IS_AFFECTED(victim, AFF_BLIND)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_BLIND);
        send_to_char("Your vision returns!\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BLINDNESS)) {
        affect_from_char(victim, SPELL_BLINDNESS);
        send_to_char("Your vision returns!\n\r", victim);
    }

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) += 1;
    }
}

void cast_cure_blind(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_blind(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_blind(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_blind(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure blind!");
        break;
    }
}
 
void spell_cure_critic(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    healpoints = dice(3, 8) + 3;

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);
    
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=2;
    }
}

void cast_cure_critic(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_critic(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_critic(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure critic!");
        break;
    }
}

void spell_cure_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    healpoints = dice(1, 8);

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=1;
    }
}

void cast_cure_light(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_light(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure light!");
        break;
    }
}

void spell_cure_serious(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + 2;

    if ((dam + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += dam;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=1;
    }

}

void cast_cure_serious(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_serious(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_serious(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure serious!");
        break;
    }
}
#if 0
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
#endif

void spell_detect_evil(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_EVIL)) {
        return;
    }
    af.type = SPELL_DETECT_EVIL;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;

    affect_to_char(victim, &af);

    act("$n's eyes briefly glow red.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_evil(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_DETECT_EVIL)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_DETECT_EVIL)) {
            return;
        }
        spell_detect_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_DETECT_EVIL)) {
                spell_detect_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect evil!");
        break;
    }
}

void spell_detect_good(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_GOOD)) {
        return;
    }
    af.type = SPELL_DETECT_GOOD;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_DETECT_GOOD;

    affect_to_char(victim, &af);

    act("$n's eyes briefly glow white.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_good(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_DETECT_GOOD)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_DETECT_GOOD)) {
            return;
        }
        spell_detect_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED2(tar_ch, AFF2_DETECT_GOOD)) {
                spell_detect_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect good!");
        break;
    }
}
#if 0
void spell_detect_invisibility(int level, struct char_data *ch,
                               struct char_data *victim,
                               struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE)) {
        return;
    }
    af.type = SPELL_DETECT_INVISIBLE;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVISIBLE;

    affect_to_char(victim, &af);
    act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void cast_detect_invisibility(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_invisibility(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) {
            return;
        }
        spell_detect_invisibility(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (in_group(ch, tar_ch) && 
                !IS_AFFECTED(tar_ch, AFF_DETECT_INVISIBLE)) {
                spell_detect_invisibility(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect invisibility!");
        break;
    }
}
#endif
#if 0
void spell_detect_magic(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_MAGIC)) {
        return;
    }
    af.type = SPELL_DETECT_MAGIC;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;

    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
}


void cast_detect_magic(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_DETECT_MAGIC)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_magic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_DETECT_MAGIC)) {
            return;
        }
        spell_detect_magic(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, SPELL_DETECT_MAGIC)) {
                spell_detect_magic(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect magic!");
        break;
    }
}
#endif

void spell_dispel_evil(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    
    assert(ch && victim);
    dam = dice(5,10);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (IsExtraPlanar(victim)) {
        if (IS_EVIL(ch)) {
            victim = ch;
        } else if (IS_GOOD(victim)) {
            act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        }

        if (!saves_spell(victim, SAVING_SPELL)) {
            act("$n forces $N from this plane.", TRUE, ch, 0, victim,
                TO_ROOM);
            act("You force $N from this plane.", TRUE, ch, 0, victim,
                TO_CHAR);
            act("$n forces you from this plane.", TRUE, ch, 0, victim,
                TO_VICT);
            gain_exp(ch, GET_EXP(victim) / 2);
            extract_char(victim);
        } else {
            act("$N resists the attack", TRUE, ch, 0, victim, TO_CHAR);
            act("you resist $n's attack.", TRUE, ch, 0, victim, TO_VICT);
            damage(ch, victim, dam, SPELL_EARTHQUAKE);
        }
    } else {
        act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
        act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
    }
}

void cast_dispel_evil(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_dispel_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_dispel_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel evil!");
        break;
    }
}

void cast_dispel_good(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_dispel_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_dispel_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel good!");
        break;
    }
}

void spell_dispel_good(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int     dam;
    
    dam = dice(5,10);
    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (IsExtraPlanar(victim)) {
        if (IS_GOOD(ch)) {
            victim = ch;
        } else if (IS_EVIL(victim)) {
            act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        }

        if (!saves_spell(victim, SAVING_SPELL)) {
            act("$n forces $N from this plane.", TRUE, ch, 0, victim,
                TO_NOTVICT);
            act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
            act("$n forces you from this plane.", TRUE, ch, 0, victim, TO_VICT);
            gain_exp(ch, MIN(GET_EXP(victim) / 2, 50000));
            extract_char(victim);
        } else {
            act("$N resists the attack.", TRUE, ch, 0, victim, TO_CHAR);
            act("You resist $n's attack.", TRUE, ch, 0, victim, TO_VICT);
            damage(ch, victim, dam, SPELL_EARTHQUAKE);
        }
    } else {
        act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
        act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
    }
}
#if 0
void spell_dispel_magic(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             yes = 0;
    int             i;
    int             check_falling(struct char_data *ch);

    assert(ch && (victim || obj));
    if (A_NODISPEL(ch)) {
        send_to_char("The arena rules do not allow the use of dispel "
                     "magic!\n\r", ch);
        return;
    }

    if (obj) {
        if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE))
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);

        if (level >= 45) {
            /*
             * if level 45> then they can do this
             */
            if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
                REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
            }
            /*
             * strip off everything
             */
            for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                obj->affected[i].location = 0;
            }
        }

        if (level >= IMMORTAL) {
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_HUM);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
        }
        return;
    }

    /*
     * gets rid of infravision, invisibility, detect, etc
     */
    if (GetMaxLevel(victim) <= level) {
        /* Changed so it's actually using scroll level (GH) */
        yes = TRUE;
    } else {
        yes = FALSE;
    }

    if (affected_by_spell(victim, SPELL_INVISIBLE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INVISIBLE);
        send_to_char("You feel exposed.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_INVISIBLE);
        send_to_char("You feel less perceptive.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_EVIL) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_EVIL);
        send_to_char("You feel less morally alert.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_GOOD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_GOOD);
        send_to_char("You can't sense the goodness around you anymore.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_MAGIC) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_MAGIC);
        send_to_char("You stop noticing the magic in your life.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SENSE_LIFE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SENSE_LIFE);
        send_to_char("You feel less in touch with living things.\n\r", victim);
    }
    
    if (affected_by_spell(victim, SPELL_ENTANGLE)) {
        affect_from_char(victim, SPELL_ENTANGLE);
        send_to_char("The magical vines holding you wither away.\n\r", victim);
    }
    
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
            if (affected_by_spell(victim, SPELL_SANCTUARY)) {
                affect_from_char(victim, SPELL_SANCTUARY);
            }
            send_to_char("You don't feel so invulnerable anymore.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The white glow around $n's body fades.", FALSE,
                    victim, 0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROTECT_FROM_EVIL);
        send_to_char("You feel less morally protected.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROTECT_FROM_GOOD);
        send_to_char("Some of your evilness dissipates into the ether.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_INFRAVISION) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INFRAVISION);
        send_to_char("Your sight grows dimmer.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SLEEP) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SLEEP);
        send_to_char("You don't feel so tired.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_CHARM_PERSON) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CHARM_PERSON);
        send_to_char("You feel less enthused about your master.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WEAKNESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WEAKNESS);
        send_to_char("You don't feel so weak.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_STRENGTH);
        send_to_char("You don't feel so strong.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_ARMOR) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ARMOR);
        send_to_char("You don't feel so well protected.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_POISON) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_POISON);
        send_to_char("You don't feel so sensitive to fumes.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BLESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_BLESS);
        send_to_char("You don't feel so blessed.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FLY) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FLY);
        send_to_char("You don't feel lighter than air anymore.\n\r", victim);
        check_falling(victim);
    }

    if (affected_by_spell(victim, SPELL_WATER_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WATER_BREATH);
        send_to_char("You don't feel so fishy anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FIRE_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FIRE_BREATH);
        send_to_char("You don't feel so fiery anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_LIGHTNING_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_LIGHTNING_BREATH);
        send_to_char("You don't feel so electric anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GAS_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GAS_BREATH);
        send_to_char("You don't have gas anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FROST_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FROST_BREATH);
        send_to_char("You don't feel so frosty anymore.\n\r", victim);
    }

    if (IS_AFFECTED(victim, AFF_FIRESHIELD)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_FIRESHIELD);
            if (affected_by_spell(victim, SPELL_FIRESHIELD)) {
                affect_from_char(victim, SPELL_FIRESHIELD);
            }
            send_to_char("Your aura of burning flames suddenly winks out of"
                         " existence.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The fiery aura around $n's body fades.", FALSE,
                    victim, 0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (IS_AFFECTED(victim, AFF_CHILLSHIELD)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_CHILLSHIELD);
            if (affected_by_spell(victim, SPELL_CHILLSHIELD)) {
                affect_from_char(victim, SPELL_CHILLSHIELD);
            }
            send_to_char("Your aura of chill flames suddenly winks out of "
                         "existence.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The cold aura around $n's body fades.", FALSE, victim,
                    0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (IS_AFFECTED(victim, AFF_BLADE_BARRIER)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_BLADE_BARRIER);
            if (affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
                affect_from_char(victim, SPELL_BLADE_BARRIER);
            }
            send_to_char("Your blade barrier suddenly winks out of "
                         "existence.\n\r", victim);
            act("$n's blade barrier disappears.", FALSE, victim, 0, 0, TO_ROOM);
        }
        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_FAERIE_FIRE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FAERIE_FIRE);
        send_to_char("You don't feel so pink anymore.\n\r", victim);
        if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
            act("The pink glow around $n's body fades.", TRUE, ch, 0,
                0, TO_ROOM);
        }
    }

    if (affected_by_spell(victim, SPELL_MINOR_TRACK) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_MINOR_TRACK);
        send_to_char("You lose the trail.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_MAJOR_TRACK) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_MAJOR_TRACK);
        send_to_char("You lose the trail.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WEB)) {
        affect_from_char(victim, SPELL_WEB);
        send_to_char("You don't feel so sticky anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SILENCE)) {
        affect_from_char(victim, SPELL_SILENCE);
        send_to_char("You don't feel so quiet anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_TREE_TRAVEL)) {
        affect_from_char(victim, SPELL_TREE_TRAVEL);
        send_to_char("You don't feel so in touch with trees anymore.\n\r",
                     victim);
    }

    if (IS_SET(victim->specials.affected_by2, AFF2_HEAT_STUFF)) {
#if 0
        if (affected_by_spell(victim, SPELL_HEAT_STUFF)) {
#endif
        affect_from_char(victim, SPELL_HEAT_STUFF);
        send_to_char("You don't feel so hot anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HASTE)) {
        affect_from_char(victim, SPELL_HASTE);
        send_to_char("You don't feel so fast anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SLOW)) {
        affect_from_char(victim, SPELL_SLOW);
        send_to_char("You don't feel so slow anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BARKSKIN)) {
        affect_from_char(victim, SPELL_BARKSKIN);
        send_to_char("You don't feel so barky anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_AID) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_AID);
        send_to_char("You feel less aided.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SHIELD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SHIELD);
        send_to_char("You feel less shielded.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_TRUE_SIGHT) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        REMOVE_BIT(victim->specials.affected_by, AFF_TRUE_SIGHT);
        affect_from_char(victim, SPELL_TRUE_SIGHT);
        send_to_char("You feel less keen.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS)) {
        affect_from_char(victim, SPELL_INVIS_TO_ANIMALS);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_UNDEAD)) {
        affect_from_char(victim, SPELL_INVIS_TO_UNDEAD);
    }

    if (affected_by_spell(victim, SPELL_DRAGON_RIDE)) {
        affect_from_char(victim, SPELL_DRAGON_RIDE);
    }

    if (IS_AFFECTED(victim, AFF_DARKNESS)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_DARKNESS);
            if (affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                affect_from_char(victim, SPELL_GLOBE_DARKNESS);
            }
            send_to_char("The dark globe surrounding you vanishes.\n\r",
                         victim);
            act("The globe of darkness surrounding $n vanishes.", FALSE,
                victim, 0, 0, TO_ROOM);
        }
        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_GLOBE_MINOR_INV) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GLOBE_MINOR_INV);
        send_to_char("You see your globe of protection vanish.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GLOBE_MAJOR_INV) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GLOBE_MAJOR_INV);
        send_to_char("You see your globe of protection vanish.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_ENERGY_DRAIN) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_ENERGY_DRAIN);
        send_to_char("You feel fearful of vampiric creatures.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_DRAGON_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_DRAGON_BREATH);
        send_to_char("You feel the urge to avoid dragons now.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WIZARDEYE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WIZARDEYE);
        send_to_char("Your wizardeye disappears with a *pop*\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_FIRE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_FIRE);
        send_to_char("You feel the urge to avoid fire dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_FROST) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_FROST);
        send_to_char("You feel the urge to avoid frost dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_ELEC) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_ELEC);
        send_to_char("You feel the urge to avoid electric dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_ACID) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_ACID);
        send_to_char("You feel the urge to avoid acid dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_GAS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_GAS);
        send_to_char("You feel the urge to avoid gas dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_GIANT_GROWTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GIANT_GROWTH);
        send_to_char("You feel less powerful.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FIND_TRAPS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FIND_TRAPS);
        send_to_char("Your sense of traps just left you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_STONE_SKIN) &&
        !saves_spell(victim, SAVING_SPELL)) {
        affect_from_char(victim, SPELL_STONE_SKIN);
        send_to_char("You skin softens considerably.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HOLY_ARMOR) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_HOLY_ARMOR);
        send_to_char("Your Deity's protection has left you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HOLY_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_HOLY_STRENGTH);
        send_to_char("Your divine strength drains away.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_ENLIGHTENMENT) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ENLIGHTENMENT);
        send_to_char("You don't feel so terribly smart anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_CIRCLE_PROTECTION) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CIRCLE_PROTECTION);
        send_to_char("Your circle of protection fails!\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_AURA_POWER) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_AURA_POWER);
        send_to_char("You lose the strength granted by your Deity.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_UNDEAD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INVIS_TO_UNDEAD);
        send_to_char("The eyes of the dead will see you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SUIT_OF_BONE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SUIT_OF_BONE);
        send_to_char("Your suit of bones crumbles to dust.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SPECTRAL_SHIELD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SPECTRAL_SHIELD);
        send_to_char("The battlescarred armor surrounding you disappears.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_CLINGING_DARKNESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CLINGING_DARKNESS);
        send_to_char("You feel the cloak of darkness lift from your eyes.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_SIPHON_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SIPHON_STRENGTH);
        send_to_char("Your strength turn back to normal.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GATHER_SHADOWS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GATHER_SHADOWS);
        send_to_char("The shadows around you dissipate into thin air.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_ENDURE_COLD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ENDURE_COLD);
        send_to_char("Your skin loses it's white hue.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_EYE_OF_THE_DEAD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_EYE_OF_THE_DEAD);
        send_to_char("Your eye of the dead goes blind and strays beyond your"
                     " grasp.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_VAMPIRIC_EMBRACE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_VAMPIRIC_EMBRACE);
        send_to_char("The aura of negative energy leaves your hands.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_IRON_SKINS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_IRON_SKINS);
        send_to_char("Your iron skins shatter leaving you unprotected.\n\r",
                     victim);
    }

    if (level >= IMPLEMENTOR) {
        if (affected_by_spell(victim, SPELL_STONE_SKIN)) {
            affect_from_char(victim, SPELL_STONE_SKIN);
            send_to_char("You skin softens considerably.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
            affect_from_char(victim, SPELL_ANTI_MAGIC_SHELL);
            send_to_char("Your anti-magic shell fizzles out.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_BLINDNESS)) {
            affect_from_char(victim, SPELL_BLINDNESS);
            send_to_char("Your vision returns.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_PARALYSIS)) {
            affect_from_char(victim, SPELL_PARALYSIS);
            send_to_char("You feel freedom of movement.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_DARK_PACT)) {
            affect_from_char(victim, SPELL_DARK_PACT);
            send_to_char("Your pact with your Lord is cruelly ripped "
                         "apart.\n\r", victim);
        }
        if (affected_by_spell(victim, COND_WINGS_FLY)) {
            affect_from_char(victim, COND_WINGS_FLY);
        }
        if (IS_AFFECTED2(victim, AFF2_WINGSBURNED)) {
            REMOVE_BIT(victim->specials.affected_by2, AFF2_WINGSBURNED);
        }
        if (IS_AFFECTED2(victim, AFF2_WINGSTIRED)) {
            REMOVE_BIT(victim->specials.affected_by2, AFF2_WINGSTIRED);
        }
        if (IS_AFFECTED(victim, COND_WINGS_BURNED)) {
            affect_from_char(victim, COND_WINGS_BURNED);
        }
        if (IS_AFFECTED(victim, COND_WINGS_TIRED)) {
            affect_from_char(victim, COND_WINGS_TIRED);
        }
        if (affected_by_spell(victim, SPELL_POISON)) {
            affect_from_char(victim, SPELL_POISON);
        }
        if (affected_by_spell(victim, SPELL_DECAY)) {
            affect_from_char(victim, SPELL_DECAY);
        }
        if (affected_by_spell(victim, SPELL_DISEASE)) {
            affect_from_char(victim, SPELL_DISEASE);
        }
        if (affected_by_spell(victim, SPELL_FLESH_GOLEM)) {
            affect_from_char(victim, SPELL_FLESH_GOLEM);
        }
        if (affected_by_spell(victim, SPELL_MANA_SHIELD)) {
            affect_from_char(victim, SPELL_MANA_SHIELD);
        }
        if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
            affect_from_char(victim, SPELL_FEEBLEMIND);
        }
        if (affected_by_spell(victim, SKILL_WALL_OF_THOUGHT)) {
            affect_from_char(victim, SKILL_WALL_OF_THOUGHT);
            send_to_char("Your wall of thought suddenly fails!\n\r", victim);
        }
    }
}

void cast_dispel_magic(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        spell_dispel_magic(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_dispel_magic(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            spell_dispel_magic(level, ch, 0, tar_obj);
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_dispel_magic(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_dispel_magic(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel magic");
        break;
    }
}
#endif
#define DUST_DEVIL 60
void spell_dust_devil(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             vnum;
    struct char_data *mob;
    struct affected_type af;

    if (affected_by_spell(ch, SPELL_DUST_DEVIL)) {
        send_to_char("You can only do this once per 24 hours\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    vnum = DUST_DEVIL;

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

    af.type = SPELL_DUST_DEVIL;
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

void cast_dust_devil(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_dust_devil(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in dust_devil.");
        break;
    }
}

void spell_earthquake(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(1, 4) + level + 1;

    send_to_char("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim) &&
                !IS_AFFECTED(tmp_victim, AFF_FLYING)) {

                if (GetMaxLevel(tmp_victim) > 4) {
                    act("You fall and hurt yourself!!\n\r",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
                } else {
                    act("You are sucked into a huge hole in the ground!",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    act("$N is sucked into a huge hole in the ground!",
                        FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                    MissileDamage(ch, tmp_victim, GET_MAX_HIT(tmp_victim) * 12,
                                  SPELL_EARTHQUAKE);
                }
            } else {
                act("You almost fall and hurt yourself!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        } else {
            if (real_roomp(ch->in_room)->zone ==
                real_roomp(tmp_victim->in_room)->zone)
                send_to_char("The earth trembles...\n\r", tmp_victim);
        }
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
#if 0
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
#endif

void spell_energy_restore(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    char            buf[100];

    if (IS_IMMORTAL(victim)) {
        send_to_char("You tried to cast energy restore on an immortal... "
                     "oops...", ch);
        send_to_char("Some puny mortal tried to cast energy restore on you... "
                     "funny...", victim);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You casting energy restore on a mob, silly :P", ch);
        return;
    }

    if (victim->old_exp) {
        act("$n does a complex religious ritual!", FALSE, ch, 0, 0, TO_ROOM);
        sprintf(buf, "You call the gods to restore %s's lost energy!\n",
                GET_NAME(victim));
        send_to_char(buf, ch);
        send_to_char("You feel your life energy flowing back in your veins!\n",
                     victim);
        GET_EXP(victim) = victim->old_exp;
        victim->old_exp = 0;
        act("You are overcome by a wave of exhaustion.", FALSE, ch, 0, 0,
            TO_CHAR);
        act("$n slumps to the ground, exhausted.", FALSE, ch, 0, 0, TO_ROOM);
        if (!IS_IMMORTAL(ch)) {
            WAIT_STATE(ch, PULSE_VIOLENCE * 12);
            GET_POS(ch) = POSITION_STUNNED;
        }
    } else {
        send_to_char("This person can't be restored!", ch);
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
#if 0
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
#endif
void spell_find_traps(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * raise their detect traps skill
     */
    if (affected_by_spell(ch, SPELL_FIND_TRAPS)) {
        send_to_char("You area already searching for traps.\n\r", ch);
        return;
    }

    af.type = SPELL_FIND_TRAPS;
    af.duration = level;
    af.modifier = 50 + level;
    af.location = APPLY_FIND_TRAPS;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void cast_find_traps(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_find_traps(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
        spell_find_traps(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in find_traps.");
        break;
    }
}

void spell_flamestrike(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(6, 8);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    } else if (!saves_spell(victim, SAVING_SPELL - 4)) {
        BurnWings(victim);
        /*
         * Fail two saves, burn the wings
         */
    }
    heat_blind(victim);
    MissileDamage(ch, victim, dam, SPELL_FLAMESTRIKE);
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
#if 0
void spell_fly(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim && IS_SET(victim->specials.act, PLR_NOFLY)) {
        REMOVE_BIT(victim->specials.act, PLR_NOFLY);
    }

    if (victim && affected_by_spell(victim, SPELL_FLY)) {
        send_to_char("The spell seems to be wasted.\n\r", ch);
        return;
    }

    act("You feel lighter than air!", TRUE, ch, 0, victim, TO_VICT);
    if (victim != ch) {
        act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
    } else {
        send_to_char("Your feet rise up off the ground.\n\r", ch);
    }
    act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type = SPELL_FLY;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
}

void cast_flying(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_fly(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_fly(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_fly(level, ch, tar_ch, 0);
        break;

    default:
        Log("Serious screw-up in fly");
        break;
    }
}
#endif
#define GOLEM 38
void spell_golem(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    int             count = 0;
    int             armor;
    struct char_data *gol;
    struct obj_data *helm = 0,
                   *jacket = 0,
                   *leggings = 0,
                   *sleeves = 0,
                   *gloves = 0,
                   *boots = 0,
                   *o;
    struct room_data *rp;

    /*
     * you need: helm, jacket, leggings, sleeves, gloves, boots
     */

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return;
    }
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_ARMOR) {
            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HEAD) && !helm) {
                count++;
                helm = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) && !boots) {
                count++;
                boots = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_BODY) && !jacket) {
                count++;
                jacket = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) && !leggings) {
                count++;
                leggings = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS) && !sleeves) {
                count++;
                sleeves = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) && !gloves) {
                count++;
                gloves = o;
                continue;
            }
        }
    }

    if (count < 6) {
        send_to_char("You don't have all the correct pieces!\n\r", ch);
        return;
    }

    if (count > 6) {
        send_to_char("Smells like an error to me!\n\r", ch);
        return;
    }

    if (!boots || !sleeves || !gloves || !helm || !jacket || !leggings) {
        /*
         * shouldn't get this far
         */
        send_to_char("You don't have all the correct pieces!\n\r", ch);
        return;
    }

    gol = read_mobile(GOLEM, VIRTUAL);
    char_to_room(gol, ch->in_room);

    /*
     * add up the armor values in the pieces
     */
    armor = boots->obj_flags.value[0];
    armor += helm->obj_flags.value[0];
    armor += gloves->obj_flags.value[0];
    armor += (leggings->obj_flags.value[0] * 2);
    armor += (sleeves->obj_flags.value[0] * 2);
    armor += (jacket->obj_flags.value[0] * 3);

    GET_AC(gol) -= armor;

    gol->points.max_hit = dice((armor / 6), 10) + GetMaxLevel(ch);
    GET_HIT(gol) = GET_MAX_HIT(gol);
    GET_LEVEL(gol, WARRIOR_LEVEL_IND) = (armor / 6);
    SET_BIT(gol->specials.affected_by, AFF_CHARM);
    GET_EXP(gol) = 0;
    IS_CARRYING_W(gol) = 0;
    IS_CARRYING_N(gol) = 0;
    gol->player.class = CLASS_WARRIOR;

    if (GET_LEVEL(gol, WARRIOR_LEVEL_IND) > 10) {
        gol->mult_att += 0.5;
    }
    /*
     * add all the effects from all the items to the golem
     */
    AddAffects(gol, boots);
    AddAffects(gol, gloves);
    AddAffects(gol, jacket);
    AddAffects(gol, sleeves);
    AddAffects(gol, leggings);
    AddAffects(gol, helm);

    act("$n waves $s hand over a pile of armor on the floor", FALSE, ch, 0,
        0, TO_ROOM);
    act("You wave your hands over the pile of armor", FALSE, ch, 0, 0, TO_CHAR);
    act("The armor flys together to form a humanoid figure!", FALSE, ch, 0,
        0, TO_ROOM);
    act("$N is quickly assembled from the pieces", FALSE, ch, 0, gol, TO_CHAR);

    add_follower(gol, ch);
    extract_obj(helm);
    extract_obj(boots);
    extract_obj(gloves);
    extract_obj(leggings);
    extract_obj(sleeves);
    extract_obj(jacket);
}

void cast_golem(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_golem(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in golem.");
        break;
    }
}

void spell_group_heal(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tch;

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("You cannot cast this spell when you're not in a "
                     "group!\n\r", ch);
        return;
    }

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (in_group(tch, ch)) {
            spell_heal(level, ch, tch, 0);
        }
    }
}

void spell_harm(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = GET_HIT(victim) - dice(1, 4);

    if (dam < 0) {
        /*
         * Kill the suffering bastard
         */
        dam = 100;
    } else {
        if (GET_RACE(ch) == RACE_GOD) {
            dam = 0;
        }
        if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
            dam = 0;
        }
    }
    dam = MIN(dam, 100);

    damage(ch, victim, dam, SPELL_HARM);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) -= 4;
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

void spell_heal(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(victim);

    spell_cure_blind(level, ch, victim, obj);

    GET_HIT(victim) += 100;

    if (GET_HIT(victim) >= hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim) - dice(1, 4);
    }
    send_to_char("A warm feeling fills your body.\n\r", victim);

    if (IS_AFFECTED2(victim, AFF2_WINGSBURNED)) {
        affect_from_char(victim, COND_WINGS_BURNED);
        send_to_char("Your wings knit together and begin to sprout new "
                     "feathers!\n\r", victim);
    }

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) += 5;
    }
}

void cast_heal(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_ch == ch) {
            act("$n heals $mself.", FALSE, ch, 0, 0, TO_ROOM);
            act("You heal yourself.", FALSE, ch, 0, 0, TO_CHAR);
        } else {
            act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
            act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
        }
        spell_heal(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_heal(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_heal(level, ch, tar_ch, 0);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (in_group(ch, tar_ch)) {
                spell_heal(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in heal!");
        break;
    }
}

void spell_heroes_feast(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tch;

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
        if ((in_group(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
            send_to_char("You partake of a magnificent feast!\n\r", tch);
            gain_condition(tch, FULL, 24);
            gain_condition(tch, THIRST, 24);
            if (GET_HIT(tch) < GET_MAX_HIT(tch)) {
                GET_HIT(tch) += 1;
            }
            GET_MOVE(tch) = GET_MAX_MOVE(tch);
        }
    }
}

void cast_heroes_feast(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
        spell_heroes_feast(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in heroes feast");
        break;
    }
}
#if 0
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
#endif
#if 0
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
        sprintf(buf, "%s is of excellent moral character.\n\r", name);
    } else if (ap > 100) {
        sprintf(buf, "%s is often kind and thoughtful.\n\r", name);
    } else if (ap > 25) {
        sprintf(buf, "%s isn't a bad sort...\n\r", name);
    } else if (ap > -25) {
        sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r",
                name);
    } else if (ap > -100) {
        sprintf(buf, "%s isn't the worst you've come across\n\r", name);
    } else if (ap > -350) {
        sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r", name);
    } else if (ap > -700) {
        sprintf(buf, "%s probably just had a bad childhood\n\r", name);
    } else {
        sprintf(buf, "I'd rather just not say anything at all about %s\n\r",
                name);
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
#endif

void spell_locate_object(int level, struct char_data *ch,
                         struct char_data *victim, char *arg)
{
    struct obj_data *i;
    char            name[256];
    char            buf[MAX_STRING_LENGTH],
                    buf2[256];
    int             j,
                    found = 0;
    struct char_data *target;

    assert(ch);
    sprintf(name, "%s", arg);

    /*
     * when starting out, no object has been found yet
     */
    found = 0;

    if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
        send_to_char("Some powerful magic interference provide you from "
                     "finding this object\n", ch);
        return;
    }

    j = level >> 2;
    if (j < 2) {
        j = 2;
    }
    buf[0] = '\0';

    for (i = object_list; i && (j > 0); i = i->next) {
        if (isname(name, i->name) &&
            !IS_SET(i->obj_flags.extra_flags, ITEM_QUEST)) {
            /*
             * we found at least one item
             */
            if (i->carried_by) {
                target = i->carried_by;
                if (((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                      real_roomp(ch->in_room)->zone ==
                       real_roomp(target->in_room)->zone) ||
                      (!IS_SET(SystemFlags, SYS_ZONELOCATE))) &&
                    !IS_IMMORTAL(target) &&
                    strlen(PERS_LOC(i->carried_by, ch)) > 0) {

                    found = 1;
                    sprintf(buf2, "%s carried by %s.\n\r",
                            i->short_description, PERS(i->carried_by, ch));
                    strcat(buf, buf2);
                    j--;
                }
            } else if (i->equipped_by) {
                target = i->equipped_by;
                if (((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                      real_roomp(ch->in_room)->zone ==
                      real_roomp(target->in_room)->zone) ||
                     (!IS_SET(SystemFlags, SYS_ZONELOCATE))) &&
                    !IS_IMMORTAL(target) &&
                    strlen(PERS_LOC(i->equipped_by, ch)) > 0) {

                    found = 1;
                    sprintf(buf2, "%s equipped by %s.\n\r",
                            i->short_description, PERS(i->equipped_by, ch));
                    strcat(buf, buf2);
                    j--;
                }
            } else if (i->in_obj) {
                if ((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                     real_roomp(ch->in_room)->zone ==
                     real_roomp(i->in_obj->in_room)->zone) ||
                    (!IS_SET(SystemFlags, SYS_ZONELOCATE))) {

                    found = 1;
                    sprintf(buf2, "%s in %s.\n\r",
                            i->short_description, i->in_obj->short_description);
                    strcat(buf, buf2);
                    j--;
                }
            } else if ((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                        real_roomp(ch->in_room)->zone ==
                        real_roomp(i->in_room)->zone) ||
                       (!IS_SET(SystemFlags, SYS_ZONELOCATE))) {
                found = 1;
                sprintf(buf2, "%s in %s.\n\r",
                        i->short_description,
                        (i->in_room == NOWHERE ? "use but uncertain." :
                         real_roomp(i->in_room)->name));
                strcat(buf, buf2);
                j--;
            }
        }
    }
    page_string(ch->desc, buf, 0);

    if (j == 0) {
        send_to_char("You are very confused.\n\r", ch);
    }
    if (found == 0) {
        send_to_char("Nothing at all by that name.\n\r", ch);
    }
}
void cast_locate_object(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_locate_object(level, ch, NULL, arg);
        break;
    default:
        Log("Serious screw-up in locate object!");
        break;
    }
}
#if 0
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
#endif
void spell_poison(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);

    if (victim) {
        if (IS_SET(victim->M_immune, IMM_POISON)) {
            send_to_char("Forces unknown help you resist the poison!\n\r",
                         victim);
            return;
        }

        if (IS_NPC(ch)) {
            if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
                if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
                    af.type = SPELL_POISON;
                    af.duration = level * 2;
                    af.modifier = -2;
                    af.location = APPLY_STR;
                    af.bitvector = AFF_POISON;
                    affect_join(victim, &af, FALSE, FALSE);
                    send_to_char("You feel very sick.\n\r", victim);
                    if (!victim->specials.fighting) {
                        AddHated(victim, ch);
                        set_fighting(victim, ch);
                    }
                } else {
                    FailPoison(victim, ch);
                    return;
                }
            } else {
                if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
                    act("Deadly poison fills your veins.", TRUE, ch, 0, 0,
                        TO_CHAR);
                    if (!damage(victim, victim, MAX(10, GET_HIT(victim) * 2),
                                SPELL_POISON)) {
                        AddHated(victim, ch);
                    }
                } else {
                    FailPoison(victim, ch);
                    return;
                }
            }
        } else if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
            af.type = SPELL_POISON;
            af.duration = level * 2;
            af.modifier = -2;
            af.location = APPLY_STR;
            af.bitvector = AFF_POISON;
            affect_join(victim, &af, FALSE, FALSE);

            act("$N looks very sick as you poison $M.", TRUE, ch, 0,
                victim, TO_CHAR);
            act("$N looks very sick as $n poisons $M.", TRUE, ch, 0,
                victim, TO_NOTVICT);
            send_to_char("You feel very sick suddenly.\n\r", victim);
            if (!victim->specials.fighting && !IS_PC(victim)) {
                AddHated(victim, ch);
                set_fighting(victim, ch);
            }
        } else {
            FailPoison(victim, ch);
        }
    } else if (obj->obj_flags.type_flag == ITEM_DRINKCON ||
               obj->obj_flags.type_flag == ITEM_FOOD) {
        /*
         * Object poison
         */
        obj->obj_flags.value[3] = 1;
    }
}

void cast_poison(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
        spell_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_poison(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in poison!");
        break;
    }
}

void spell_prot_dragon_breath_acid(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a acid breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a acid breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a acid breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a acid breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a acid breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_ACID;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a acid protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a acid protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_dragon_breath_acid(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_acid(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_acid.");
        break;
    }
}

void spell_prot_dragon_breath_elec(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a electric breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a electric breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a electric breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_ELEC;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim){
        sprintf(buf, "$N is already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_dragon_breath_elec(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_elec(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_elec.");
        break;
    }
}

void spell_prot_dragon_breath_fire(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_FIRE)) {
        if (ch != victim) {
            act("$n summons a fire breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a fire breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a fire breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a fire breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a fire breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_FIRE;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a fire protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a fire protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_dragon_breath_fire(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_fire(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_fire.");
        break;
    }
}

void spell_prot_cold(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_COLD)) {
        if (ch != victim) {
            act("$n summons a cold protective globe around $N.", FALSE, ch,
                0, victim, TO_NOTVICT);
            act("You summon a cold protective globe about $N.", FALSE, ch,
                0, victim, TO_CHAR);
            act("$n summons a cold protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a cold protective globe about $mself.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a cold protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_COLD;
        af.modifier = IMM_COLD;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a cold protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a cold protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_cold(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_cold!");
        break;
    }
}

void spell_prot_energy(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_ENERGY)) {
        if (ch != victim) {
            act("$n summons a energy protective globe around $N.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a energy protective globe about $N.", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n summons a energy protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a energy protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a energy protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_ENERGY;
        af.modifier = IMM_ENERGY;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a energy protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a energy protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_energy(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_energy!");
        break;
    }
}

void spell_prot_energy_drain(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROT_ENERGY_DRAIN)) {
        send_to_char("You call forth a protective shield against energy "
                     "drain.\n\r", ch);
        af.type = SPELL_PROT_ENERGY_DRAIN;
        af.duration = !IS_IMMORTAL(ch) ? 3 : level;
        af.modifier = IMM_DRAIN;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("You are already protected from energy drain.\n\r", ch);
    }
}

void cast_prot_energy_drain(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_energy_drain(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-energy drain.");
        break;
    }
}

void spell_prot_elec(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_ELEC)) {
        if (ch != victim) {
            act("$n summons a electric protective globe around $N.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a electric protective globe about $N.", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n summons a electric protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a electric protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_ELEC;
        af.modifier = IMM_ELEC;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_elec(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_elec!");
        break;
    }
}

void spell_protection_from_evil(int level, struct char_data *ch,
                                struct char_data *victim,
                                struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
        af.type = SPELL_PROTECT_FROM_EVIL;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_PROTECT_FROM_EVIL;
        affect_to_char(victim, &af);
        send_to_char("You have a righteous feeling!\n\r", victim);
    }
}

void cast_protection_from_evil(int level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
    }
}

void spell_protection_from_evil_group(int level, struct char_data *ch,
                                      struct char_data *victim,
                                      struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *dude;

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (IS_FOLLOWING(ch, dude) &&
            !affected_by_spell(dude, SPELL_PROTECT_FROM_EVIL)) {

            af.type = SPELL_PROTECT_FROM_EVIL;
            af.duration = 24;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PROTECT_FROM_EVIL;
            affect_to_char(dude, &af);
            send_to_char("You have a righteous feeling!\n\r", dude);
        }
    }
}

void cast_protection_from_evil_group(int level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil_group(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil_group(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
    }
}

void spell_prot_fire(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_FIRE)) {
        if (ch != victim) {
            act("$n summons a fire protective globe around $N.", FALSE, ch,
                0, victim, TO_NOTVICT);
            act("You summon a fire protective globe about $N.", FALSE, ch,
                0, victim, TO_CHAR);
            act("$n summons a fire protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a fire protective globe about $mself.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a fire protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_FIRE;
        af.modifier = IMM_FIRE;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N is already surrounded by a fire protective "
                         "globe.");
        } else {
            sprintf(buf, "You are already surrounded by a fire protective "
                         "globe.");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_fire(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_fire!");
        break;
    }
}

void spell_protection_from_good(int level, struct char_data *ch,
                                struct char_data *victim,
                                struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD)) {
        af.type = SPELL_PROTECT_FROM_GOOD;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_SPELL2;
        af.bitvector = AFF2_PROTECT_FROM_GOOD;
        affect_to_char(victim, &af);
        send_to_char("You have a sinister feeling!\n\r", victim);
    }
}

void cast_protection_from_good(int level, struct char_data *ch, char
                               *arg, int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from good!");
        break;
    }
}

void spell_protection_from_good_group(int level, struct char_data *ch,
                                      struct char_data *victim,
                                      struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *dude;

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (IS_FOLLOWING(ch, dude) &&
            !affected_by_spell(dude, SPELL_PROTECT_FROM_GOOD)) {

            af.type = SPELL_PROTECT_FROM_GOOD;
            af.duration = 24;
            af.modifier = 0;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_PROTECT_FROM_GOOD;
            affect_to_char(dude, &af);
            send_to_char("You have a sinister feeling!\n\r", dude);
        }
    }
}

void cast_protection_from_good_group(int level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_good_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_good_group(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_good_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_good_group(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from good group!");
        break;
    }
}

void spell_prot_dragon_breath_frost(int level, struct char_data *ch,
                                    struct char_data *victim,
                                    struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_FROST)) {
        if (ch != victim) {
            act("$n summons a frost breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a frost breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a frost breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a frost breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a frost breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_FROST;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a frost protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a frost protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_dragon_breath_frost(int level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_frost(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_frost.");
        break;
    }
}

void spell_prot_dragon_breath_gas(int level, struct char_data *ch,
                                  struct char_data *victim,
                                  struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a gas breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a gas breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a gas breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a gas breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a gas breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_GAS;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a gas protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a gas protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void cast_prot_dragon_breath_gas(int level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_gas(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_gas.");
        break;
    }
}
#if 0
void spell_refresh(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct affected_type *k;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4) + level;
    dam = MAX(dam, 20);

    if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
        GET_MOVE(victim) = move_limit(victim);
    } else {
        GET_MOVE(victim) += dam;
    }
    if (affected_by_spell(victim, COND_WINGS_TIRED) ||
        affected_by_spell(victim, COND_WINGS_FLY)) {
        for (k = victim->affected; k; k = k->next) {
            if (k->type == COND_WINGS_TIRED) {
                k->duration = k->duration - dice(1, 10);
                if (k->duration < 0) {
                    affect_from_char(victim, COND_WINGS_TIRED);
                }
            } else if (k->type == COND_WINGS_FLY) {
                k->duration = k->duration + dice(1, 10);
                k->duration = MIN(k->duration, GET_CON(ch));
            }
        }
    }
    send_to_char("You feel less tired\n\r", victim);
}

void cast_refresh(int level, struct char_data *ch, char *arg, int type,
                  struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_refresh(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_refresh(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_refresh(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_refresh(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in refresh!");
        break;
    }
}
#endif

void spell_remove_curse(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             i,
                    found = FALSE;

    assert(ch && (victim || obj));

    if (obj) {
        if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
            act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
            act("$p, held by $n, briefly glows blue.", TRUE, ch, obj, 0,
                TO_ROOM);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        }
    } else {
        /*
         * Then it is a PC | NPC
         */
        if (affected_by_spell(victim, SPELL_CURSE)) {
            act("$n briefly glows red, then blue.", FALSE, victim, 0, 0,
                TO_ROOM);
            act("You feel better.", FALSE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_CURSE);
        } else {
            /*
             * uncurse items equiped by a person
             */
            i = 0;
            do {
                if (victim->equipment[i] &&
                    IS_SET(victim->equipment[i]->obj_flags.extra_flags,
                           ITEM_NODROP)) {
                    spell_remove_curse(level, victim, NULL,
                                       victim->equipment[i]);
                    found = TRUE;
                }
                i++;
            } while (i < MAX_WEAR && !found);
        }

        if (IS_PC(ch) && IS_PC(victim)) {
            GET_ALIGNMENT(ch) += 2;
        }
    }
}

void cast_remove_curse(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_curse(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_curse(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            spell_remove_curse(level, ch, 0, tar_obj);
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_remove_curse(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_curse(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove curse!");
        break;
    }
}

void spell_remove_paralysis(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if (IS_AFFECTED(victim, AFF_PARALYSIS)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_PARALYSIS);
    }
    if (affected_by_spell(victim, SPELL_PARALYSIS)) {
        affect_from_char(victim, SPELL_PARALYSIS);
        act("A warm feeling runs through your body.", FALSE, victim, 0, 0,
            TO_CHAR);
        act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
    }
}

void cast_remove_paralysis(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_paralysis(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_paralysis(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_remove_paralysis(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_paralysis(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove paralysis!");
        break;
    }
}

void spell_remove_poison(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if (affected_by_spell(victim, SPELL_POISON)) {
            affect_from_char(victim, SPELL_POISON);
            act("A warm feeling runs through your body.", FALSE, victim, 0,
                0, TO_CHAR);
            act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
        }
    } else {
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            obj->obj_flags.value[3] = 0;
            act("The $p steams briefly.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}

void cast_remove_poison(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_poison(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove poison!");
        break;
    }
}

void spell_resurrection(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_file_u st;
    struct affected_type af;
    struct obj_data *obj_object,
                   *next_obj;
    FILE           *fl;

    if (!obj) {
        return;
    }
    if (IS_CORPSE(obj)) {
        if (obj->char_vnum) {
            /*
             * corpse is a npc
             */

            if (GET_GOLD(ch) < 25000) {
                send_to_char("The gods are not happy with your sacrifice.\n\r",
                             ch);
                return;
            } else {
                GET_GOLD(ch) -= 25000;
            }

            victim = read_mobile(obj->char_vnum, VIRTUAL);
            char_to_room(victim, ch->in_room);
            GET_GOLD(victim) = 0;
            GET_EXP(victim) = 0;
            GET_HIT(victim) = 1;
            GET_POS(victim) = POSITION_STUNNED;

            act("With mystic power, $n resurrects a corpse.", TRUE, ch,
                0, 0, TO_ROOM);
            act("$N slowly rises from the ground.", FALSE, ch, 0, victim,
                TO_ROOM);

            /*
             * should be charmed and follower ch
             */

            if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM) ||
                IS_IMMORTAL(victim)) {
                act("$N says 'Thank you'", FALSE, ch, 0, victim, TO_CHAR);
                act("$N says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
            } else if (too_many_followers(ch)) {
                act("$N takes one look at the size of your posse and just says"
                    " no!", TRUE, ch, 0, victim, TO_CHAR);
                act("$N takes one look at the size of $n's posse and just says"
                    " no!", TRUE, ch, 0, victim, TO_ROOM);
            } else {
                af.type = SPELL_CHARM_PERSON;
                af.duration = follow_time(ch);
                af.modifier = 0;
                af.location = 0;
                af.bitvector = AFF_CHARM;

                affect_to_char(victim, &af);
                add_follower(victim, ch);
            }

            IS_CARRYING_W(victim) = 0;
            IS_CARRYING_N(victim) = 0;

            /*
             * take all from corpse, and give to person
             */

            for (obj_object = obj->contains; obj_object;
                 obj_object = next_obj) {
                next_obj = obj_object->next_content;
                obj_from_obj(obj_object);
                obj_to_char(obj_object, victim);
            }

            /*
             * get rid of corpse
             */
            extract_obj(obj);
        } else {
            /*
             * corpse is a pc
             */
            if (GET_GOLD(ch) < 75000) {
                send_to_char("The gods are not happy with your sacrifice.\n\r",
                             ch);
                return;
            } else {
                GET_GOLD(ch) -= 75000;
            }

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
#if 0
            if (st.talks[2] && st.abilities.con > 3) {
#else
            if (!get_char(st.name) && st.abilities.con > 3) {
#endif
                st.points.exp *= 2;
                st.points.leadership_exp *= 2;
                st.m_deaths--;
                st.talks[2] = TRUE;
                st.abilities.con -= 1;
                act("A clear bell rings throughout the heavens",
                    TRUE, ch, 0, 0, TO_CHAR);
                act("A ghostly spirit smiles, and says 'Thank you'",
                    TRUE, ch, 0, 0, TO_CHAR);
                act("A clear bell rings throughout the heavens",
                    TRUE, ch, 0, 0, TO_ROOM);
                act("A ghostly spirit smiles, and says 'Thank you'",
                    TRUE, ch, 0, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                GET_MANA(ch) = 1;
                GET_MOVE(ch) = 1;
                GET_HIT(ch) = 1;
                GET_POS(ch) = POSITION_STUNNED;
                act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
                send_to_char("You collapse from the effort\n\r", ch);
                rewind(fl);
                fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
                fwrite(&st, sizeof(struct char_file_u), 1, fl);
                ObjFromCorpse(obj);
            } else {
                send_to_char("The body does not have the strength to be "
                             "recreated.\n\r", ch);
            }
            fclose(fl);
        }
    }
}

void cast_resurrection(int level, struct char_data *ch, char *arg,
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
        spell_resurrection(level, ch, 0, tar_obj);
        break;
    case SPELL_TYPE_STAFF:
        if (!tar_obj) {
            return;
        }
        spell_resurrection(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious problem in 'resurrection'");
        break;
    }
}

void spell_sanctuary(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_SANCTUARY) &&
        !IS_AFFECTED(victim, AFF_SANCTUARY)) {
        act("$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM);
        act("You start glowing.", TRUE, victim, 0, 0, TO_CHAR);

        af.type = SPELL_SANCTUARY;

        if (IS_IMMORTAL(ch)) {
            af.duration = level;
        } else {
            af.duration = (ch->specials.remortclass == CLERIC_LEVEL_IND + 1 ? 
                           4 : 3); 
        }
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(victim, &af);
    }
}

void cast_sanctuary(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_sanctuary(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
        spell_sanctuary(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_sanctuary(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) { 
            if (tar_ch != ch) {
                spell_sanctuary(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in sanctuary!");
        break;
    }
}
#if 0
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
#endif
void spell_sense_life(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_SENSE_LIFE) &&
        !IS_AFFECTED(victim, AFF_SENSE_LIFE)) {
        if (ch != victim) {
            act("$n waves $s hands around $N's eyes.", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You help $N see hidden life forms.", FALSE, ch, 0, victim,
                TO_CHAR);
            act("$n waves $s hands around your eyes, you feel your senses "
                "improve.", FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n eyes flash briefly.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You feel your senses improve.", FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_SENSE_LIFE;
        af.duration = 5 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SENSE_LIFE;
        affect_to_char(victim, &af);
    } else {
        act("$N can already sense hidden life forms.", FALSE, ch, 0,
            victim, TO_CHAR);
    }
}

void cast_sense_life(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_sense_life(level, ch, ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_sense_life(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_sense_life(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in sense life!");
        break;
    }
}
#if 0
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
#endif
#if 0
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
#endif
#if 0
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
#endif

void spell_true_seeing(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
        if (ch != victim) {
            send_to_char("Your eyes glow silver for a moment.\n\r", victim);
            act("$n's eyes take on a silvery hue.", FALSE, victim, 0, 0,
                TO_ROOM);
        } else {
            send_to_char("Your eyes glow silver.\n\r", ch);
            act("$n's eyes glow silver.", FALSE, ch, 0, 0, TO_ROOM);
        }

        af.type = SPELL_TRUE_SIGHT;
        af.duration = 2 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_TRUE_SIGHT;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing seems to happen.\n\r", ch);
    }
}

void cast_true_seeing(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj || IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
                spell_true_seeing(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in true_seeing!");
        break;
    }
}

void spell_turn(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             diff,
                    i;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (IsUndead(victim)) {
        diff = level - GetTotLevel(victim);
        if (diff <= 0) {
            act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
            return;
        } else {
            for (i = 1; i <= diff; i++) {
                if (!saves_spell(victim, SAVING_SPELL)) {
                    act("$n forces $N from this room.", TRUE, ch, 0,
                        victim, TO_NOTVICT);
                    act("You force $N from this room.", TRUE, ch, 0,
                        victim, TO_CHAR);
                    act("$n forces you from this room.", TRUE, ch, 0,
                        victim, TO_VICT);
                    do_flee(victim, "", 0);
                    break;
                }
            }
            if (i < diff) {
                act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
                act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
                act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
            }
        }
    } else {
        act("$n just tried to turn you, what a moron!", TRUE, ch, 0,
            victim, TO_VICT);
        act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
    }
}

void cast_turn(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_turn(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in turn");
        break;
    }
}

void spell_word_of_recall(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             location;

    assert(victim);

    if (IS_NPC(victim)) {
        return;
    }
#if 0
    loc_nr = GET_HOME(ch);
#endif

    if (victim->player.hometown) {
        location = victim->player.hometown;
    } else {
        location = 3001;
    }

    if (!real_roomp(location)) {
        send_to_char("You are completely lost.\n\r", victim);
        location = 0;
        return;
    }

    if (victim->specials.fighting) {
        send_to_char("HAH, not in a fight!\n\r", ch);
        return;
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("You can't recall!, you're on a different plane!\n\r",
                     victim);
        return;
    }

    /*
     * a location has been found.
     */
    send_to_char("You think of better times and close your eyes.\n\r", victim);

    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    do_look(victim, NULL, 15);

}

void cast_word_of_recall(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    struct char_data *tmp_ch;

    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_word_of_recall(level, ch, ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_word_of_recall(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_word_of_recall(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_word_of_recall(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people; tar_ch;
             tar_ch = tmp_ch) {
            tmp_ch = tar_ch->next_in_room;
            if (tar_ch != ch) {
                spell_word_of_recall(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in word of recall!");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
