
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
