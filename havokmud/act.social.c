/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

/*
 * extern variables
 */

char           *fread_action(FILE * fl)
{
    char            buf[MAX_STRING_LENGTH],
                   *rslt;

    for (;;) {
        fgets(buf, MAX_STRING_LENGTH, fl);
        if (feof(fl)) {
            Log("Fread_action - unexpected EOF.");
            exit(0);
        }

        if (*buf == '#') {
            return (0);
        } else {
            *(buf + strlen(buf) - 1) = '\0';
            CREATE(rslt, char, strlen(buf) + 1);
            strcpy(rslt, buf);
            return (rslt);
        }
    }
    return (0);
}


int find_action(int cmd)
{
    int             bot,
                    top,
                    mid;

    bot = 0;
    top = socialMessageCount - 1;

    if (top < 0) {
        return (-1);
    }
    for (;;) {
        mid = (bot + top) / 2;

        if (socialMessages[mid].act_nr == cmd) {
            return (mid);
        }
        if (bot >= top) {
            return (-1);
        }
        if (socialMessages[mid].act_nr > cmd) {
            top = --mid;
        } else {
            bot = ++mid;
        }
    }
}

void do_action(struct char_data *ch, char *argument, int cmd)
{
    int             act_nr;
    char           *buf;
    struct social_messg *action;
    struct char_data *vict;
    struct obj_data *objx = 0;

    dlog("in do_action");

    if ((act_nr = find_action(cmd)) < 0) {
        send_to_char("That action is not supported.\n\r", ch);
        return;
    }

    action = &socialMessages[act_nr];

    if (action->msg[MSG_SOCIAL_ARG_SELF]) {
        argument = get_argument(argument, &buf);
    } else {
        buf = NULL;
    }

    if (!buf) {
        ch_printf(ch, "%s\n\r", action->msg[MSG_SOCIAL_NO_ARG_SELF]);
        act(action->msg[MSG_SOCIAL_NO_ARG_ROOM], action->hide, ch, 0, 0,
            TO_ROOM);
        return;
    }

    if (!(vict = get_char_room_vis(ch, buf))) {
        if ((objx = get_obj_in_list_vis(ch, buf, ch->carrying))) {
            act(action->msg[MSG_SOCIAL_ARG_OBJ_SELF], action->hide, ch, objx,
                objx, TO_CHAR);
            act(action->msg[MSG_SOCIAL_ARG_OBJ_ROOM], action->hide, ch, objx,
                objx, TO_ROOM);
            return;
        }

        ch_printf(ch, "%s\n\r", action->msg[MSG_SOCIAL_ARG_NO_VICTIM]);
    } else if (vict == ch) {
        ch_printf(ch, "%s\n\r", action->msg[MSG_SOCIAL_ARG_AUTO_SELF]);
        act(action->msg[MSG_SOCIAL_ARG_AUTO_ROOM], action->hide, ch, 0, 0,
            TO_ROOM);
    } else {
        if (GET_POS(vict) < action->min_victim_position) {
            act("$N is not in a proper position for that.", FALSE, ch, 0,
                vict, TO_CHAR);
        } else {
            act(action->msg[MSG_SOCIAL_ARG_SELF], 0, ch, 0, vict, TO_CHAR);
            act(action->msg[MSG_SOCIAL_ARG_ROOM], action->hide, ch, 0, vict,
                TO_NOTVICT);
            act(action->msg[MSG_SOCIAL_ARG_VICTIM], action->hide, ch, 0, vict,
                TO_VICT);
        }
    }
}

void do_insult(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    struct char_data *victim;

    dlog("in do_insult");

    argument = get_argument(argument, &arg);

    if (arg) {
        send_to_char("Sure you don't want to insult everybody.\n\r", ch);
        return;
    }

    if (!(victim = get_char_room_vis(ch, arg))) {
        send_to_char("Can't hear you!\n\r", ch);
        return;
    } 

    if (victim != ch) {
        send_to_char("You feel insulted.\n\r", ch);
        return;
    }

    ch_printf(ch, "You insult %s.\n\r", GET_NAME(victim));

    switch (number(0,2)) {
    case 0:
        if (GET_SEX(ch) == SEX_MALE) {
            if (GET_SEX(victim) == SEX_MALE) {
                act("$n accuses you of fighting like a woman!",
                    FALSE, ch, 0, victim, TO_VICT);
            } else {
                act("$n says that women can't fight.",
                    FALSE, ch, 0, victim, TO_VICT);
            }
        } else {
            if (GET_SEX(victim) == SEX_MALE) {
                act("$n accuses you of having the smallest...."
                    " (brain?)", FALSE, ch, 0, victim,
                    TO_VICT);
            } else {
                act("$n tells you that you'd loose a beauty "
                    "contest against a troll.", FALSE, ch, 0,
                    victim, TO_VICT);
            }
        }
        break;
    case 1:
        act("$n calls you a punk!",
            FALSE, ch, 0, victim, TO_VICT);
        break;
    default:
        act("$n tells you to get lost!", FALSE, ch, 0,
            victim, TO_VICT);
        break;
    }

    act("$n insults $N.", TRUE, ch, 0, victim, TO_NOTVICT);
}

void do_pose(struct char_data *ch, char *argument, int cmd)
{
    int             i;
    int             lev,
                    class;

    dlog("in do_pose");

#if 0
    lev = (IS_IMMORTAL(ch) ? MAX_MORT : GetMaxLevel(ch));
#endif
    lev = GetMaxLevel(ch);

    if ((lev < poseMessages[0].level) || !IS_PC(ch)) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    do {
        class = number(0, classCount - 1);
    } while ((lev = GET_LEVEL(ch, class)) < poseMessages[0].level);

    for (i = 0; i < poseMessageCount && poseMessages[i].level < lev; i++) {
        /*
         * Empty loop
         */
    }

    if( i == poseMessageCount ) {
        i--;
    }

    i = number(0, i);

    if (!poseMessages[i].poser_msg[class] ||
        !poseMessages[i].room_msg[class] ) {
        send_to_char("Sorry.. no pose messages for you yet\n", ch);
        return;
    }

    act(poseMessages[i].poser_msg[class], 0, ch, 0, 0, TO_CHAR);
    act(poseMessages[i].room_msg[class], 0, ch, 0, 0, TO_ROOM);
}


#define CMD_OOC     497
#define CMD_GOSSIP  302
#define CMD_SHOUT   18

/*
 * Not quite sure what i'm doing here yet but lets hope that this will be
 * OOC socials
 */
void do_OOCaction(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    int             act_nr;
    char           *name;
    char           *buf,
                    buf2[MAX_INPUT_LENGTH];
    struct social_messg *action;
    struct char_data *vict;
    struct obj_data *objx = 0;

    char            command[100];

    dlog("in do_action");

    if (cmd == CMD_OOC) {
        sprintf(command, "$c000B[$c000WOOC$c000B]$c000w");
    } else if (cmd == CMD_SHOUT) {
        sprintf(command, "$c000R[$c000WSHOUT$c000R]$c000w");
    } else if (cmd == CMD_GOSSIP) {
        sprintf(command, "$c000Y[$c000WGOSSIP$c000Y]$c000w");
    } else {
        sprintf(command, "$c000p[$c000WWORLD$c000p]$c000w");
    }

    argument = get_argument(argument, &buf);

    if (!buf || (act_nr = find_action(FindCommandNumber(buf))) < 0) {
        send_to_char("That action is not supported.\n\r", ch);
        return;
    }

    action = &socialMessages[act_nr];

    if (action->msg[MSG_SOCIAL_ARG_SELF]) {
        argument = get_argument(argument, &buf);
        name = skip_spaces(argument);
    } else {
        buf = NULL;
        name = NULL;
    }

    if (!name) {
        if (action->msg[MSG_SOCIAL_NO_ARG_ROOM]) {
            sprintf(buf2, "%s %s", command, 
                    action->msg[MSG_SOCIAL_NO_ARG_ROOM]);
            act(buf2, action->hide, ch, 0, 0, TO_CHAR);

            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                      !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                    !check_soundproof(i->character)) {

                    act(buf2, action->hide, ch, 0, i->character, TO_VICT);
                }
            }
        } else {
            /*
             * need an argument for this one
             */
            act(action->msg[MSG_SOCIAL_NO_ARG_SELF], action->hide, ch, 0, 0, 
                TO_CHAR);
        }
        return;
    }

    if (!(vict = get_char_vis_world(ch, name, NULL))) {
        if ((objx = get_obj_in_list_vis(ch, name, ch->carrying))) {
            sprintf(buf2, "%s %s\n\r", command, 
                    action->msg[MSG_SOCIAL_ARG_ROOM]);
            act(buf2, action->hide, ch, objx, objx, TO_CHAR);

            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                      !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                    !check_soundproof(i->character)) {

                    act2(buf2, action->hide, ch, objx, objx, i->character,
                         TO_CHAR);
                }
            }
            return;
        }

        ch_printf(ch, "%s\n\r", action->msg[MSG_SOCIAL_ARG_NO_VICTIM]);
        return;
    } 
    
    if (vict == ch) {
        sprintf(buf2, "%s %s", command, action->msg[MSG_SOCIAL_ARG_AUTO_ROOM]);
        act(buf2, action->hide, ch, 0, 0, TO_CHAR);

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                (IS_NPC(i->character) ||
                 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                  !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                  !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                !check_soundproof(i->character)) {

                act(buf2, action->hide, ch, 0, i->character, TO_VICT);
            }
        }
        return;
    } 

    if (GET_POS(vict) < action->min_victim_position) {
        act("$N is not in a proper position for that.", FALSE, ch, 0,
            vict, TO_CHAR);
    } else {
        sprintf(buf2, "%s %s", command, action->msg[MSG_SOCIAL_ARG_ROOM]);
        act(buf2, action->hide, ch, 0, vict, TO_CHAR);

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                (IS_NPC(i->character) ||
                 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                  !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                  !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                !check_soundproof(i->character)) {

                act2(buf2, action->hide, ch, 0, vict, i->character, TO_VICT);
            }
        }
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
