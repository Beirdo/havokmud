/*
 * HavokMUD - warrior skills
 */
#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

void do_mend(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct obj_data *cmp;
    char            buf[254],
                   *arg;
    int             perc = 0;
    extern struct index_data *obj_index;

    dlog("in do_mend");

    if (!ch) {
        return;
    }

    if (!ch->skills[SKILL_MEND].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("Heh, not when yer fighting, silly!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Mend what?\n\r", ch);
        return;
    }

    obj = get_obj_in_list_vis(ch, arg, ch->carrying);
    perc = number(1, 101);
    if (obj) {
        if (ITEM_TYPE(obj) == ITEM_ARMOR) {
            if (obj->obj_flags.value[0] == 0 ||
                obj->obj_flags.value[1] == 0) {
                sprintf(buf, "%s tried to mend an invalid armor value: %s, "
                             "vnum %ld.",
                        GET_NAME(ch), obj->short_description,
                        (obj->item_number >= 0) ?
                         obj_index[obj->item_number].virtual : 0);
                Log(buf);
                return;
            }

            if (obj->obj_flags.value[0] >= obj->obj_flags.value[1]) {
                send_to_char("That item has no need of your attention.\n\r",
                             ch);
                return;
            } 
            
            if (ch->skills[SKILL_MEND].learned < perc) {
                /*
                 * failure permanently lowers full strength by one
                 * point
                 */
                obj->obj_flags.value[1] -= 1;
                sprintf(buf, "%s tries to mend %s, but only makes things "
                             "worse.", GET_NAME(ch), obj->short_description);
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                sprintf(buf, "You try to mend %s, but make matters worse.\n\r",
                        obj->short_description);
                send_to_char(buf, ch);
                LearnFromMistake(ch, SKILL_MEND, 0, 95);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                return;
            } 
            
            obj->obj_flags.value[0] = obj->obj_flags.value[1];
            sprintf(buf, "%s expertly mends %s.", GET_NAME(ch),
                    obj->short_description);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "You expertly mend %s.\n\r", obj->short_description);
            send_to_char(buf, ch);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
            WAIT_STATE(ch, PULSE_VIOLENCE * 1);
            return;
        } 
        
        if (ITEM_TYPE(obj) == ITEM_WEAPON) {
            if (obj->obj_flags.value[2] == 0) {
                sprintf(buf, "%s tried to mend an weapon with invalid "
                             "value: %s, vnum %d.",
                        GET_NAME(ch), obj->short_description,
                        obj->item_number);
                Log(buf);
                return;
            }

            cmp = read_object(obj->item_number, REAL);
            if (cmp->obj_flags.value[2] == 0) {
                sprintf(buf, "%s tried to mend an weapon with invalid "
                             "value: %s, vnum %d.",
                        GET_NAME(ch), obj->short_description,
                        obj->item_number);
                Log(buf);
                extract_obj(cmp);
                return;
            }

            if (cmp->obj_flags.value[2] == obj->obj_flags.value[2]) {
                send_to_char("That item has no need of your attention.\n\r",
                             ch);
                extract_obj(cmp);
            } else {
                if (ch->skills[SKILL_MEND].learned < perc) {
                    /*
                     * failure lowers damage die by one point
                     */
                    obj->obj_flags.value[2] -= 1;
                    sprintf(buf, "%s tries to mend %s, but only makes matters "
                                 "worse.",
                            GET_NAME(ch), obj->short_description);
                    act(buf, FALSE, ch, 0, 0, TO_ROOM);
                    sprintf(buf, "You try to mend %s, but only make things "
                                 "worse.\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                    /*
                     * did this scrap the weapon?
                     */
                    if (obj->obj_flags.value[2] < 1) {
                        sprintf(buf, "%s screwed up so bad that %s is reduced "
                                     "to junk!",
                                GET_NAME(ch), obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);
                        sprintf(buf, "You screwed up so bad that %s is reduced"
                                     " to junk!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);
                        MakeScrap(ch, NULL, obj);
                    }
                    extract_obj(cmp);
                    LearnFromMistake(ch, SKILL_MEND, 0, 95);
                    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                } else {
                    obj->obj_flags.value[2] = cmp->obj_flags.value[2];
                    sprintf(buf, "%s expertly mends %s.", GET_NAME(ch),
                            obj->short_description);
                    act(buf, FALSE, ch, 0, 0, TO_ROOM);
                    sprintf(buf, "You expertly mend %s.\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                    send_to_char("$c000BYou receive $c000W100 $c000B"
                                 "experience for using your abilities."
                                 "$c0007\n\r", ch);
                    gain_exp(ch, 100);
                    extract_obj(cmp);
                    WAIT_STATE(ch, PULSE_VIOLENCE * 1);
                }
            }
        } else {
            send_to_char("You can't mend that!", ch);
        }
    } else {
        sprintf(buf, "Mend what?\n\r");
        send_to_char(buf, ch);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

