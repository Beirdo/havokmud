
/*
 * HavokMUD - cleric skills and spells
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

void spell_aid(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
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
            return;
        } 

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
            return;
        } 
        
        if (!tar_ch)
            tar_ch = ch;

        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING))
            return;
        spell_bless(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        
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
        obj->cost /= 2;
        if (obj->cost < 100) {
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_ROOM);
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_CHAR);
            objectTakeFromChar(obj);
            objectExtract(obj);
        }
    } else {
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_ROOM);
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_CHAR);
        objectTakeFromChar(obj);
        objectExtract(obj);
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

    if (ch->equipment[WIELD] && ch->equipment[WIELD]->item_number == obj) {
        wielded = TRUE;
    }

    if (ch->equipment[HOLD] && ch->equipment[HOLD]->item_number == obj) {
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
        if (sac->cost >= 200) {
            equip_char(ch, sac, (held ? HOLD : WIELD));
        } else {
            objectGiveToChar(sac, ch);
        }
    } else {
        objectGiveToChar(sac, ch);
    }

    if (sac) {
        if (sac->item_number != obj) {
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

void cast_command(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    char           *p;
    char            buf[128];

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

void spell_create_food(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    CREATE(tmp_obj, struct obj_data, 1);
    objectClear(tmp_obj);

    StringToKeywords("mushroom", &tmp_obj->keywords);
    tmp_obj->short_description = (char *) strdup("A Magic Mushroom");
    tmp_obj->description = (char *)strdup("A really delicious looking magic"
                                          " mushroom lies here.");

    tmp_obj->type_flag = ITEM_TYPE_FOOD;
    tmp_obj->wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->value[0] = 5 + level;
    tmp_obj->weight = 1;
    tmp_obj->cost = 10;
    tmp_obj->cost_per_day = 1;

    /**
     * @todo why are we using object_list?
     */
    tmp_obj->next = object_list;
    object_list = tmp_obj;

    objectPutInRoom(tmp_obj, ch->in_room);

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

    tmp_obj = objectRead(20); /* this is all you have to do */
    if (tmp_obj) {
        tmp_obj->value[2] = 24 + level;
        objectGiveToChar(tmp_obj, ch);
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

    if (GET_ITEM_TYPE(obj) == ITEM_TYPE_DRINKCON) {
	if (obj->value[2] != LIQ_WATER &&
	    obj->value[1] != 0) {

	    name_from_drinkcon(obj);
	    obj->value[2] = LIQ_SLIME;
	    name_to_drinkcon(obj, LIQ_SLIME);
	} else {
	    water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);
	    water = MIN(obj->value[0] - obj->value[1], water);

	    if (water > 0) {
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;

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
        if (tar_obj->type_flag != ITEM_TYPE_DRINKCON) {
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
        SET_BIT(obj->anti_flags, ITEM_ANTI_GOOD);
        SET_BIT(obj->extra_flags, ITEM_NODROP);

        /*
         * LOWER ATTACK DICE BY -1
         */
        if (IS_WEAPON(obj)) {
            obj->value[2]--;
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
            spell_curse(level, ch, 0, tar_obj);
        } else {
            spell_curse(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_POTION:
        spell_curse(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            spell_curse(level, ch, 0, tar_obj);
        } else {
            if (!tar_ch) {
                tar_ch = ch;
            }
            spell_curse(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            spell_curse(level, ch, 0, tar_obj);
        } else {
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
        if (ITEM_TYPE(o) == ITEM_TYPE_ARMOR) {
            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_HEAD) && !helm) {
                count++;
                helm = o;
                continue;
            }

            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_FEET) && !boots) {
                count++;
                boots = o;
                continue;
            }

            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_BODY) && !jacket) {
                count++;
                jacket = o;
                continue;
            }

            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_LEGS) && !leggings) {
                count++;
                leggings = o;
                continue;
            }

            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_ARMS) && !sleeves) {
                count++;
                sleeves = o;
                continue;
            }

            if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_HANDS) && !gloves) {
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
    armor = boots->value[0];
    armor += helm->value[0];
    armor += gloves->value[0];
    armor += (leggings->value[0] * 2);
    armor += (sleeves->value[0] * 2);
    armor += (jacket->value[0] * 3);

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
    objectExtract(helm);
    objectExtract(boots);
    objectExtract(gloves);
    objectExtract(leggings);
    objectExtract(sleeves);
    objectExtract(jacket);
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

void cast_group_heal(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_group_heal(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_group_heal");
        break;
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
        sprinttype(ITEM_TYPE(obj), item_types, buf2);
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
                    (obj->item_number >= 0) ? obj->item_number : 0, color1);

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

        if (obj->bitvector) {
            sprintf(buf2, "%sItem will give you following abilities:%s  ",
                    color1, color2);
            send_to_char(buf2, ch);

            sprintbit((unsigned) obj->bitvector, affected_bits, buf);
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
        }

        sprintf(buf, "%sItem is:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->extra_flags, extra_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        sprintf(buf, "%sWeight: %s%d%s, Value: %s%d%s, Rent cost: %s%d%s ",
                color1, color2, obj->weight, color1, color2,
                obj->cost, color1, color2,
                obj->cost_per_day, color1);
        send_to_char(buf, ch);

        if (IS_RARE(obj)) {
            sprintf(buf, "%s[%sRARE%s]", color1, color2, color1);
            send_to_char(buf, ch);
        }
        send_to_char("\n\r", ch);

        sprintf(buf, "%sCan be worn on:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->wear_flags, wear_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        switch (ITEM_TYPE(obj)) {

        case ITEM_TYPE_SCROLL:
        case ITEM_TYPE_POTION:
            sprintf(buf, "%sLevel %s%d%s spells of:\n\r", color1, color2,
                    obj->value[0], color1);
            send_to_char(buf, ch);

            for( i = 1; i < 4; i++ ) {
                if (obj->value[i] >= 1) {
                    sprinttype(obj->value[i] - 1, spells, buf);
                    sprintf(buf2, "%s%s", color2, buf);
                    strcat(buf2, "\n\r");
                    send_to_char(buf2, ch);
                }
            }
            break;

        case ITEM_TYPE_WAND:
        case ITEM_TYPE_STAFF:
            sprintf(buf, "%sCosts %s%d%s mana to use, with %s%d%s charges "
                         "left.\n\r",
                    color1, color2, obj->value[1], color1,
                    color2, obj->value[2], color1);
            send_to_char(buf, ch);

            sprintf(buf, "%sLevel %s%d%s spell of:\n\r", color1, color2,
                    obj->value[0], color1);
            send_to_char(buf, ch);
            if (obj->value[3] >= 1) {
                sprinttype(obj->value[3] - 1, spells, buf);
                sprintf(buf2, "%s%s", color2, buf);
                strcat(buf2, "\n\r");
                send_to_char(buf2, ch);
            }
            break;

        case ITEM_TYPE_WEAPON:
            sprintf(buf, "%sDamage Dice is '%s%dD%d%s' [%s%s%s] [%s%s%s]\n\r",
                    color1, color2, obj->value[1],
                    obj->value[2], color1, color2,
                    AttackType[obj->value[3]], color1, color2,
                    weaponskills[obj->weapontype].name, color1);
            send_to_char(buf, ch);
            break;

        case ITEM_TYPE_ARMOR:
            sprintf(buf, "%sAC-apply is: %s%d%s,   Size of armor is: %s%s\n\r",
                    color1, color2, obj->value[0], color1,
                    color2, ArmorSize(obj->value[2]));
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
#endif

void spell_locate_object(int level, struct char_data *ch,
                         struct char_data *victim, char *arg)
{
    struct obj_data *i;
    char            buf[MAX_STRING_LENGTH],
                    buf2[256];
    int             j,
                    found = 0;
    struct char_data *target;
    Keywords_t     *key;

    if( !ch ) {
        return;
    }

    /*
     * when starting out, no object has been found yet
     */
    found = 0;

    if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
        send_to_char("Some powerful magic interference provide you from "
                     "finding this object\n", ch);
        return;
    }

    key = StringToKeywords( arg, NULL );

    j = level >> 2;
    if (j < 2) {
        j = 2;
    }
    buf[0] = '\0';

    /**
     * @todo let's see if we can't somehow use btrees for this
     */
    for (i = object_list; i && (j > 0); i = i->next) {
        if ( !IS_OBJ_STAT(i, extra_flags, ITEM_QUEST) && 
             KeywordsMatch(key, &i->keywords) ) {
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
    FreeKeywords(key, TRUE);
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

#if 0
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
#endif
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

void spell_remove_curse(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             i,
                    found = FALSE;

    assert(ch && (victim || obj));

    if (obj) {
        if (IS_OBJ_STAT(obj, extra_flags, ITEM_NODROP)) {
            act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
            act("$p, held by $n, briefly glows blue.", TRUE, ch, obj, 0,
                TO_ROOM);
            REMOVE_BIT(obj->extra_flags, ITEM_NODROP);
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
                    IS_OBJ_STAT(victim->equipment[i], extra_flags,
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
                objectTakeFromObject(obj_object);
                objectGiveToChar(obj_object, victim);
            }

            /*
             * get rid of corpse
             */
            objectExtract(obj);
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

void do_scribe(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH];
    char            arg[MAX_INPUT_LENGTH],
                   *spellnm;
    struct obj_data *obj;
    int             sn = -1,
                    x,
                    index,
                    formula = 0;

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!MainClass(ch, CLERIC_LEVEL_IND) && !IS_IMMORTAL(ch)) {
        send_to_char("Alas, you can only dream of scribing scrolls.\n\r", ch);
        return;
    }

    if (!ch->skills) {
        send_to_char("You don't seem to have any skills.\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_SCRIBE].learned)) {
        send_to_char("You cannot seem to comprehend the intricacies of "
                     "scribing.\n\r", ch);
        return;
    }

    if (!argument) {
        send_to_char("Which spell would you like to scribe?\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 50 && !IS_IMMORTAL(ch)) {
        send_to_char("You don't have enough mana to scribe that spell.\n\r",
                     ch);
        return;
    }

    if (!(obj = objectRead(EMPTY_SCROLL))) {
        Log("no default scroll could be found for scribe");
        send_to_char("woops, something's wrong.\n\r", ch);
        return;
    }

    argument = get_argument_delim(argument, &spellnm, '\'');

    /*
     * Check for beginning quote 
     */
    if (!spellnm || spellnm[-1] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    sn = old_search_block(spellnm, 0, strlen(spellnm), spells, 0);
    sn = sn - 1;

    /* 
     * find spell number..
     */
    for (x = 0; x < 250; x++) {
        if (is_abbrev(arg, spells[x])) {
            sn = x;
            break;
        }
    }

    if (sn == -1) {
        /* 
         * no spell found?
         */
        send_to_char("Scribe what??.\n\r", ch);
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

    if (!(spell_info[index].min_level_cleric)) {
        /*
         * is it a mage spell?
         */
        send_to_char("Alas, you cannot scribe that spell, it's not in your "
                     "sphere.\n\r", ch);
        return;
    }

    if (spell_info[index].brewable == 0 && !IS_IMMORTAL(ch)) {
        send_to_char("You can't scribe this spell.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < spell_info[index].min_usesmana * 2 && 
        !IS_IMMORTAL(ch)) {
        send_to_char("You don't have enough mana to scribe that spell.\n\r",
                     ch);
        return;
    }

    act("$n holds up a scroll and binds it with enchantments.", TRUE, ch,
        obj, NULL, TO_ROOM);
    act("You bind an empty scroll with enchantments.", TRUE, ch, obj, NULL,
        TO_CHAR);

    formula = ((ch->skills[SKILL_SCRIBE].learned) + GET_INT(ch) / 3) + 
              (GET_WIS(ch) / 3);

    if (formula > 98) {
        formula = 98;
    }
    if ((number(1, 101) >= formula || ch->skills[SKILL_SCRIBE].learned < 10) &&
        !IS_IMMORTAL(ch)) {

        WAIT_STATE(ch, PULSE_VIOLENCE * 5);
        act("$p goes up in flames!", TRUE, ch, obj, NULL, TO_CHAR);
        act("$p goes up in flames!", TRUE, ch, obj, NULL, TO_ROOM);
        GET_HIT(ch) -= 10;
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        act("$n yelps in pain.", TRUE, ch, 0, NULL, TO_ROOM);
        act("Ouch!", TRUE, ch, 0, NULL, TO_CHAR);
        LearnFromMistake(ch, SKILL_SCRIBE, 0, 90);

        objectExtract(obj);
    } else {
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        sprintf(buf, "You have imbued a spell to %s.\n\r",
                obj->short_description);
        send_to_char(buf, ch);
        send_to_char("The scribing process was a success!\n\r", ch);

        if (obj->short_description) {
            free(obj->short_description);
        }
        sprintf(buf, "a scroll of %s", spells[sn]);
        obj->short_description = (char *) strdup(buf);

        FreeKeywords( &obj->keywords, FALSE );
        sprintf(buf, "scroll %s", spells[sn]);
        StringToKeywords( buf, &obj->keywords );

        if (obj->description) {
            free(obj->description);
        }
        obj->description = strdup("A scroll, bound with enchantments, lies on "
                                  "the ground.");

        if (IS_IMMORTAL(ch)) {
            /* 
             * set spell level.
             */
            obj->value[0] = MAX_MORT;
            
            /* 
             * set ego to level.
             */
            obj->level = MAX_MORT;
        } else {
            /* 
             * set spell level.
             */
            obj->value[0] = GetMaxLevel(ch);

            /*
             * set ego to level.
             */
            obj->level = GetMaxLevel(ch);
        }
        
        /* 
         * set spell in slot.
         */
        obj->value[1] = sn + 1;
        obj->timer = 42;

        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        objectGiveToChar(obj, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

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
void spell_succor(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *o;

    o = objectRead(3052);
    objectGiveToChar(o, ch);

    o->cost = 0;
    o->cost_per_day = -1;

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
