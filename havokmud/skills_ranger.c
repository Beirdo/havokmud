#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"


/* animal friendship */
/* animal growth */
/* animal summon one */

void do_climb(struct char_data *ch, char *arg, int cmd)
{
    extern char    *dirs[];
    int             dir;
    struct room_direction_data *exitp;
    int             was_in,
                    roll;
    extern char    *dirs[];

    char            buf[256],
                   *direction;

    if (GET_MOVE(ch) < 10) {
        send_to_char("You're too tired to do that\n\r", ch);
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }

    /*
     * make sure that the argument is a direction, or a keyword. 
     */

    arg = get_argument(arg, &direction);
    if (!direction || (dir = search_block(direction, dirs, FALSE)) < 0) {
        send_to_char("You can't climb that way.\n\r", ch);
        return;
    }

    exitp = EXIT(ch, dir);
    if (!exitp) {
        send_to_char("You can't climb that way.\n\r", ch);
        return;
    }

    if (!IS_SET(exitp->exit_info, EX_CLIMB)) {
        send_to_char("You can't climb that way.\n\r", ch);
        return;
    }

    if (dir == UP && real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
        !IS_AFFECTED(ch, AFF_FLYING)) {
        send_to_char("You have no way of getting there!\n\r", ch);
        return;
    }

    if (IS_SET(exitp->exit_info, EX_ISDOOR) &&
        IS_SET(exitp->exit_info, EX_CLOSED)) {
        send_to_char("You can't climb that way.\n\r", ch);
        return;
    }

    sprintf(buf, "$n attempts to climb %swards", dirs[dir]);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "You attempt to climb %swards\n\r", dirs[dir]);
    send_to_char(buf, ch);

    GET_MOVE(ch) -= 10;

    /*
     * now we've checked for failures, time to check for success; 
     */
    if (ch->skills) {
        if (ch->skills[SKILL_CLIMB].learned) {
            roll = number(1, 100);
            if (roll > ch->skills[SKILL_CLIMB].learned) {
                slip_in_climb(ch, dir, exitp->to_room);
                LearnFromMistake(ch, SKILL_CLIMB, 0, 95);
            } else {
                was_in = ch->in_room;

                char_from_room(ch);
                char_to_room(ch, exitp->to_room);
                do_look(ch, NULL, 0);
                DisplayMove(ch, dir, was_in, 1);
                if (!check_falling(ch) && 
                    IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                    !IS_IMMORTAL(ch)) {
                    NailThisSucker(ch);
                    return;
                }

                WAIT_STATE(ch, PULSE_VIOLENCE * 3);
                GET_MOVE(ch) -= 10;
            }
        } else {
            send_to_char("You just don't know the nuances of climbing.\n\r",
                         ch);
            slip_in_climb(ch, dir, exitp->to_room);
        }
    } else {
        send_to_char("You're just a goofy mob.\n\r", ch);
    }
}

void slip_in_climb(struct char_data *ch, int dir, int room)
{
    int             i;

    i = number(1, 6);

    if (dir != DOWN) {
        act("$n falls down and goes splut.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You fall.\n\r", ch);
    } else {
        act("$n loses $s grip and falls further down.", FALSE, ch, 0, 0,
            TO_ROOM);
        send_to_char("You slip and start to fall.\n\r", ch);
        i += number(1, 6);
        char_from_room(ch);
        char_to_room(ch, room);
        do_look(ch, NULL, 0);
    }

    GET_POS(ch) = POSITION_SITTING;
    if (i > GET_HIT(ch)) {
        GET_HIT(ch) = 1;
    } else {
        GET_HIT(ch) -= i;
    }
}
/* detect poison */

void spell_giant_growth(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * +3 to hit +3 dam
     */

    if (affected_by_spell(victim, SPELL_GIANT_GROWTH)) {
        send_to_char("Already in effect\n\r", ch);
        return;
    }

    GET_HIT(victim) += number(1, 8);

    update_pos(victim);

    act("$n grows in size.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel larger!\n\r", victim);

    af.type = SPELL_GIANT_GROWTH;
    af.duration = 10;
    af.modifier = 3;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    af.location = APPLY_DAMROLL;
    af.modifier = 3;
    affect_to_char(victim, &af);
}

void cast_giant_growth(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_giant_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in giant growth.");
        break;
    }
}
