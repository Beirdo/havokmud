/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"

/*
 * extern variables
 */

extern struct descriptor_data *descriptor_list;

struct social_messg {
    int             act_nr;
    int             hide;
    int             min_victim_position;
    /*
     * Position of victim
     */
    /*
     * No argument was supplied
     */
    char           *char_no_arg;
    char           *others_no_arg;

    /*
     * An argument was there, and a victim was found
     */
    char           *char_found;
    /*
     * if NULL, read no further, ignore args
     */
    char           *others_found;
    char           *vict_found;
    /*
     * An argument was there, but no victim was found
     */
    char           *not_found;
    /*
     * The victim turned out to be the character
     */
    char           *char_auto;
    char           *others_auto;
    /*
     * For objects
     */
    char           *obj_you;
    char           *obj_other;

}              *soc_mess_list = 0;

struct pose_type {
    int             level;
    /*
     * minimum level for poser
     */
    char           *poser_msg[4];
    /*
     * message to poser
     */
    char           *room_msg[4];
    /*
     * message to room
     */
} pose_messages[MAX_MESSAGES];

static int      list_top = -1;

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

void boot_social_messages(void)
{
    FILE           *fl;
    int             tmp,
                    hide,
                    min_pos;

    if (!(fl = fopen(SOCMESS_FILE, "r"))) {
        perror("boot_social_messages");
        assert(0);
    }

    for (;;) {
        fscanf(fl, " %d ", &tmp);
        if (tmp < 0) {
            break;
        }
        fscanf(fl, " %d ", &hide);
        fscanf(fl, " %d \n", &min_pos);

        /*
         * alloc a new cell
         */
        if (!soc_mess_list) {
            CREATE(soc_mess_list, struct social_messg, 1);
            list_top = 0;
        } else if (!(soc_mess_list = (struct social_messg *)
                     realloc(soc_mess_list,
                             sizeof(struct social_messg) * (++list_top + 1)))) {
            perror("boot_social_messages. realloc");
            exit(0);
        }

        /*
         * read the stuff
         */
        soc_mess_list[list_top].act_nr = tmp;
        soc_mess_list[list_top].hide = hide;
        soc_mess_list[list_top].min_victim_position = min_pos;
        soc_mess_list[list_top].char_no_arg = fread_action(fl);
        soc_mess_list[list_top].others_no_arg = fread_action(fl);
        soc_mess_list[list_top].char_found = fread_action(fl);
#if 0
        if(!soc_mess_list[list_top].char_found) {
            soc_mess_list[list_top].obj_you = fread_action(fl);
            soc_mess_list[list_top].obj_other = fread_action(fl);
        }
#endif

        /*
         * if no char_found, the rest is to be ignored
         */
        if (!soc_mess_list[list_top].char_found) {
            continue;
        }
        soc_mess_list[list_top].others_found = fread_action(fl);
        soc_mess_list[list_top].vict_found = fread_action(fl);
        soc_mess_list[list_top].not_found = fread_action(fl);
        soc_mess_list[list_top].char_auto = fread_action(fl);
        soc_mess_list[list_top].others_auto = fread_action(fl);
        soc_mess_list[list_top].obj_you = fread_action(fl);
        soc_mess_list[list_top].obj_other = fread_action(fl);

    }
    fclose(fl);
}

int find_action(int cmd)
{
    int             bot,
                    top,
                    mid;

    bot = 0;
    top = list_top;

    if (top < 0) {
        return (-1);
    }
    for (;;) {
        mid = (bot + top) / 2;

        if (soc_mess_list[mid].act_nr == cmd) {
            return (mid);
        }
        if (bot >= top) {
            return (-1);
        }
        if (soc_mess_list[mid].act_nr > cmd) {
            top = --mid;
        } else {
            bot = ++mid;
        }
    }
}

void do_action(struct char_data *ch, char *argument, int cmd)
{
    int             act_nr;
    char            buf[MAX_INPUT_LENGTH];
    struct social_messg *action;
    struct char_data *vict;
    struct obj_data *objx = 0;

    dlog("in do_action");

    if ((act_nr = find_action(cmd)) < 0) {
        send_to_char("That action is not supported.\n\r", ch);
        return;
    }

    action = &soc_mess_list[act_nr];

    if (action->char_found) {
        only_argument(argument, buf);
    } else {
        *buf = '\0';
    }

    if (!*buf) {
        send_to_char(action->char_no_arg, ch);
        send_to_char("\n\r", ch);
        act(action->others_no_arg, action->hide, ch, 0, 0, TO_ROOM);
        return;
    }

    if (!(vict = get_char_room_vis(ch, buf))) {
        if ((objx = get_obj_in_list_vis(ch, buf, ch->carrying))) {
            act(action->obj_you, action->hide, ch, objx, objx, TO_CHAR);
            act(action->obj_other, action->hide, ch, objx, objx, TO_ROOM);
            return;
        }
        send_to_char(action->not_found, ch);
        send_to_char("\n\r", ch);
    } else if (vict == ch) {
        send_to_char(action->char_auto, ch);
        send_to_char("\n\r", ch);
        act(action->others_auto, action->hide, ch, 0, 0, TO_ROOM);
    } else {
        if (GET_POS(vict) < action->min_victim_position) {
            act("$N is not in a proper position for that.", FALSE, ch, 0,
                vict, TO_CHAR);
        } else {
            act(action->char_found, 0, ch, 0, vict, TO_CHAR);
            act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
            act(action->vict_found, action->hide, ch, 0, vict, TO_VICT);
        }
    }
}

void do_insult(struct char_data *ch, char *argument, int cmd)
{
    static char     buf[100];
    static char     arg[MAX_STRING_LENGTH];
    struct char_data *victim;

    dlog("in do_insult");

    only_argument(argument, arg);

    if (*arg) {
        if (!(victim = get_char_room_vis(ch, arg))) {
            send_to_char("Can't hear you!\n\r", ch);
        } else {
            if (victim != ch) {
                sprintf(buf, "You insult %s.\n\r", GET_NAME(victim));
                send_to_char(buf, ch);

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
            } else {
                send_to_char("You feel insulted.\n\r", ch);
            }
        }
    } else
        send_to_char("Sure you don't want to insult everybody.\n\r", ch);
}

void boot_pose_messages(void)
{
    FILE           *fl;
    int             counter;
    int             class;

    if (!(fl = fopen(POSEMESS_FILE, "r"))) {
        perror("boot_pose_messages");
        exit(0);
    }

    for (counter = 0;; counter++) {
        fscanf(fl, " %d ", &pose_messages[counter].level);
        if (pose_messages[counter].level < 0) {
            break;
        }
        for (class = 0; class < 4; class++) {
            pose_messages[counter].poser_msg[class] = fread_action(fl);
            pose_messages[counter].room_msg[class] = fread_action(fl);
        }
    }
    fclose(fl);
}

void do_pose(struct char_data *ch, char *argument, int cmd)
{
    int             to_pose;
    int             counter;
    int             lev,
                    class;

    dlog("in do_pose");

    lev = GetMaxLevel(ch);
    if (IS_IMMORTAL(ch)) {
        lev = LOW_IMMORTAL - 1;
    }

    if ((lev < pose_messages[0].level) || !IS_PC(ch)) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (!IS_SET(ch->player.class, CLASS_MAGIC_USER | CLASS_CLERIC |
                                  CLASS_WARRIOR | CLASS_THIEF)) {
        send_to_char("Sorry.. no pose messages for you yet\n", ch);
        return;
    }

    do {
        class = number(0, OLD_MAX_CLASS - 1);
    } while ((lev = GET_LEVEL(ch, class)) < pose_messages[0].level);

    for (counter = 0;
         pose_messages[counter].level < lev &&
         pose_messages[counter].level > 0;
         counter++) {
        /*
         * Empty loop
         */
    }
    counter--;

    to_pose = number(0, counter);

    act(pose_messages[to_pose].poser_msg[class], 0, ch, 0, 0, TO_CHAR);
    act(pose_messages[to_pose].room_msg[class], 0, ch, 0, 0, TO_ROOM);
}


/*
 * Not quite sure what i'm doing here yet but lets hope that this will be
 * OOC socials
 */
void do_OOCaction(struct char_data *ch, char *argument, int cmd)
{
    int             CMD_OOC = 497,
                    CMD_GOSSIP = 302,
                    CMD_SHOUT = 18;
    struct descriptor_data *i;
    int             act_nr;
    char            name[MAX_INPUT_LENGTH + 80];
    char            buf[100],
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
    argument++;
    half_chop(argument, buf, name);

    if ((act_nr = find_action(FindCommandNumber(buf))) < 0) {
        send_to_char("That action is not supported.\n\r", ch);
        return;
    }

    action = &soc_mess_list[act_nr];

    if (action->char_found) {
        half_chop(argument, buf, name);
    } else
        *buf = '\0';

#if 0
    if(!*buf) {
        send_to_char("ooc %<Social name> <Person/object/noarg>.\n\r",ch);
        return;
    }
#endif

    if (!*name) {
        /*
         * No arguments
         */

        if (action->others_no_arg) {
            sprintf(buf2, "%s %s", command, action->others_no_arg);
            act(buf2, action->hide, ch, 0, 0, TO_CHAR);
            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                      !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                    !check_soundproof(i->character)) {
                    sprintf(buf2, "%s %s", command, action->others_no_arg);
                    act(buf2, action->hide, ch, 0, i->character, TO_VICT);
                }
            }
        } else {
            /*
             * need an argument for this one
             */
            sprintf(buf2, "%s", action->char_no_arg);
            act(buf2, action->hide, ch, 0, 0, TO_CHAR);
        }
        return;
    }

    if (!(vict = get_char_vis_world(ch, name, NULL))) {
        if ((objx = get_obj_in_list_vis(ch, name, ch->carrying))) {
            sprintf(buf2, "%s %s\n\r", command, action->obj_other);
            act(buf2, action->hide, ch, objx, objx, TO_CHAR);

            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                      !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                    !check_soundproof(i->character)) {
                    sprintf(buf2, "%s %s\n\r", command, action->obj_other);
                    act2(buf2, action->hide, ch, objx, objx, i->character,
                         TO_CHAR);
                }
            }
            return;
        }
        send_to_char(action->not_found, ch);
        send_to_char("\n\r", ch);
    } else if (vict == ch) {
        sprintf(buf2, "%s %s", command, action->others_auto);
        act(buf2, action->hide, ch, 0, 0, TO_CHAR);

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                (IS_NPC(i->character) ||
                 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                  !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                  !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                !check_soundproof(i->character)) {
                sprintf(buf2, "%s %s", command, action->others_auto);
                act(buf2, action->hide, ch, 0, i->character, TO_VICT);
            }
        }
    } else {
        if (GET_POS(vict) < action->min_victim_position) {
            act("$N is not in a proper position for that.", FALSE, ch, 0,
                vict, TO_CHAR);
        } else {
            sprintf(buf2, "%s %s", command, action->others_found);
            act(buf2, action->hide, ch, 0, vict, TO_CHAR);
#if 0
            act(action->others_found, action->hide, ch, 0, vict, TO_NOTVICT);
#endif
            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOOOC) &&
                      !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
                    !check_soundproof(i->character)) {
#if 0
                    sprintf(buf2, "[OOC] %s \n\r", action->vict_found);
#endif
                    sprintf(buf2, "%s %s", command, action->others_found);
                    act2(buf2, action->hide, ch, 0, vict, i->character,
                         TO_VICT);
                }
            }
        }
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
