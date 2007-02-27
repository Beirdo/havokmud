/*
 * HavokMUD - generic rogue spells and skills
 * REQUIRED BY - thief, monk
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

extern struct dex_skill_type dex_app_skill[];

void do_disguise(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    struct char_data *k;

    if (!ch->skills) {
        return;
    }
    if (!ch->skills[SKILL_DISGUISE].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    send_to_char("You attempt to disguise yourself\n\r", ch);

    if (affected_by_spell(ch, SKILL_DISGUISE)) {
        send_to_char("You can only do this once per day\n\r", ch);
        return;
    }

    if (number(1, 101) < ch->skills[SKILL_DISGUISE].learned) {
        for (k = character_list; k; k = k->next) {
            if (k->specials.hunting == ch) {
                k->specials.hunting = 0;
            }
            if (number(1, 101) < ch->skills[SKILL_DISGUISE].learned) {
                if (Hates(k, ch)) {
                    ZeroHatred(k, ch);
                }
                if (Fears(k, ch)) {
                    ZeroFeared(k, ch);
                }
            }
        }
    } else {
        LearnFromMistake(ch, SKILL_DISGUISE, 0, 95);
    }
    af.type = SKILL_DISGUISE;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_hide");

    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }
    if (!ch->skills[SKILL_HIDE].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    send_to_char("You attempt to hide in the shadows.\n\r", ch);
    
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    percent = number(1, 101);
    /*
     * 101% is a complete failure
     */
    if (!ch->skills) {
        return;
    }
    if (percent > ch->skills[SKILL_HIDE].learned +
        dex_app_skill[(int)GET_DEX(ch)].hide) {
        LearnFromMistake(ch, SKILL_HIDE, 1, 90);
        if (cmd) {
            WAIT_STATE(ch, PULSE_VIOLENCE * 1);
        }
        return;
    }
    SET_BIT(ch->specials.affected_by, AFF_HIDE);
    if (cmd) {
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }
}

void do_pick(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;
    int             door;
    char           *type,
                   *dir;
    struct room_direction_data *exitp;
    struct obj_data *obj;
    struct char_data *victim;

    dlog("in do_pick");

    argument = get_argument(argument, &type);
    argument = get_argument(argument, &dir);

    percent = number(1, 101);   /* 101% is a complete failure */

    if (!ch->skills) {
        send_to_char("You failed to pick the lock.\n\r", ch);
        return;
    }
    if (!ch->skills[SKILL_PICK_LOCK].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
        send_to_char("You failed to pick the lock.\n\r", ch);
        act("$n mutters as $s lockpicks jam in the lock.", TRUE, ch, 0, 0,
            TO_ROOM);
        LearnFromMistake(ch, SKILL_PICK_LOCK, 0, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE * 4);
        return;
    }

    if (!type) {
        send_to_char("Pick what?\n\r", ch);
        return;
    } 
    
    if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, 
                     &obj)) {
        /*
         * this is an object 
         */
        if (obj->type_flag != ITEM_TYPE_CONTAINER) {
            send_to_char("That's not a container.\n\r", ch);
        } else if (!IS_SET(obj->value[1], CONT_CLOSED)) {
            send_to_char("Silly - it ain't even closed!\n\r", ch);
        } else if (obj->value[2] < 0) { 
            send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
        } else if (!IS_SET(obj->value[1], CONT_LOCKED)) {
            send_to_char("Oho! This thing is NOT locked!\n\r", ch);
        } else if (IS_SET(obj->value[1], CONT_PICKPROOF)) {
            send_to_char("It resists your attempts at picking it.\n\r", ch);
        } else {
            REMOVE_BIT(obj->value[1], CONT_LOCKED);
            send_to_char("*Click*\n\r", ch);
            act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
    } else if ((door = find_door(ch, type, dir)) >= 0) {
        exitp = EXIT(ch, door);
        if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
            send_to_char("That's absurd.\n\r", ch);
        } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
            send_to_char("You realize that the door is already open.\n\r", ch);
        } else if (exitp->key < 0) {
            send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
        } else if (!IS_SET(exitp->exit_info, EX_LOCKED)) {
            send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
        } else if (IS_SET(exitp->exit_info, EX_PICKPROOF)) {
            send_to_char("You seem to be unable to pick this lock.\n\r", ch);
        } else {
            if (exitp->keyword) {
                act("$n skillfully picks the lock of the $F.", 0, ch, 0,
                    exitp->keyword, TO_ROOM);
            } else {
                act("$n picks the lock.", TRUE, ch, 0, 0, TO_ROOM);
            }
            send_to_char("The lock quickly yields to your skills.\n\r", ch);
            raw_unlock_door(ch, exitp, door);

        }
    }
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_sneak");

    if (IS_AFFECTED2(ch, AFF2_SKILL_SNEAK)) {
        if (IS_AFFECTED(ch, AFF_HIDE)) {
            REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
        }
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        send_to_char("You are no longer sneaky.\n\r", ch);
        return;
    }
    if (!ch->skills[SKILL_SNEAK].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    if (!IS_AFFECTED(ch, AFF_SILENCE)) {
        if (HasExtraBits(ch, ITEM_HUM)) {
            send_to_char("Gonna be hard to sneak around with that thing "
                         "humming\n\r", ch);
            return;
        }
    }
    send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
    percent = number(1, 101);
    /*
     * 101% is a complete failure
     */
    if (!ch->skills) {
        return;
    }
    if (IS_SET(ch->specials.affected_by2, AFF2_SKILL_SNEAK)) {
        send_to_char("You stop being sneaky!", ch);
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
    } else {
        send_to_char("You start jumping from shadow to shadow.", ch);
        SET_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

