/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"

/*
 * extern variables
 */

struct char_data *mem_list = 0;

void            stop_follower(struct char_data *ch);
void            do_mobTell(struct char_data *ch, char *mob,
                           char *sentence);

#define MAX_GNAME           7
/*
 * The max group name index
 */

#define RACE_WAR_MIN_LEVEL 10
/* this is the level a user can turn race
 * war ON
 */
#define ARENA_ZONE 124
#define SEVERED_HEAD    29




void do_gain(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_gain");
}

void do_guard(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_guard");

    if (!IS_NPC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Sorry. you can't just put your brain on "
                     "autopilot!\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
            act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("You relax.\n\r", ch);
            REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
        } else {
            SET_BIT(ch->specials.act, ACT_GUARDIAN);
            act("$n alertly watches you.", FALSE, ch, 0, ch->master, TO_VICT);
            act("$n alertly watches $N.", FALSE, ch, 0, ch->master,
                TO_NOTVICT);
            send_to_char("You snap to attention\n\r", ch);
        }
    } else if (!strcasecmp(argument, "on")) {
        if (!IS_SET(ch->specials.act, ACT_GUARDIAN)) {
            SET_BIT(ch->specials.act, ACT_GUARDIAN);
            act("$n alertly watches you.", FALSE, ch, 0, ch->master,
                TO_VICT);
            act("$n alertly watches $N.", FALSE, ch, 0, ch->master,
                TO_NOTVICT);
            send_to_char("You snap to attention\n\r", ch);
        }
    } else if (!strcasecmp(argument, "off")) {
        if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
            act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("You relax.\n\r", ch);
            REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
        }
    }
}

void do_junk(struct char_data *ch, char *argument, int cmd)
{
    char           *arg,
                    buf[100],
                    newarg[100];
    struct obj_data *tmp_object;
    int             num,
                    p,
                    count,
                    value = 0;

    dlog("in do_junk");
    /*
     * get object name & verify
     */
    argument = get_argument(argument, &arg);
    if (arg) {
        if (getall(arg, newarg) != '\0') {
            num = -1;
            strcpy(arg, newarg);
        } else if ((p = getabunch(arg, newarg)) != '\0') {
            num = p;
            strcpy(arg, newarg);
        } else {
            num = 1;
        }
    } else {
        send_to_char("Junk what?\n\r", ch);
        return;
    }
    count = 0;
    while (num != 0) {
        tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
        if (tmp_object) {
            if (IS_OBJ_STAT(tmp_object, extra_flags, ITEM_NODROP)) {
                send_to_char("You can't let go of it, it must be CURSED!\n\r",
                             ch);
                return;
            }
            value += (MIN(1000, MAX(tmp_object->cost / 4, 1)));
            obj_from_char(tmp_object);
            extract_obj(tmp_object);
            if (num > 0) {
                num--;
            }
            count++;
        } else {
            num = 0;
        }
    }
    if (count > 1) {
        sprintf(buf, "You junk %s (%d).", arg, count);
        act(buf, 1, ch, 0, 0, TO_CHAR);
        sprintf(buf, "$n junks %s.", arg);
        act(buf, 1, ch, 0, 0, TO_ROOM);
    } else if (count == 1) {
        sprintf(buf, "You junk %s ", arg);
        act(buf, 1, ch, 0, 0, TO_CHAR);
        sprintf(buf, "$n junks %s.", arg);
        act(buf, 1, ch, 0, 0, TO_ROOM);
    } else {
        send_to_char("You don't have anything like that\n\r", ch);
    }

    value /= 2;
    if (value) {
        act("You are awarded for outstanding performance.",
            FALSE, ch, 0, 0, TO_CHAR);

        if (GetMaxLevel(ch) < 3) {
            gain_exp(ch, MIN(100, value));
        } else {
            GET_GOLD(ch) += value;
        }
    }
    return;
}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("You have to write quit - no less, to quit!\n\r", ch);
    return;
}

void do_set_prompt(struct char_data *ch, char *argument, int cmd)
{
    static struct def_prompt {
        int             n;
        char           *pr;
    } prompts[] = {
        {
        1, "Havok> "}, {
        2, "H:%h V:%v> "}, {
        3, "H:%h M:%m V:%v> "}, {
        4, "H:%h/%H V:%v/%V> "}, {
        5, "H:%h/%H M:%m/%M V:%v/%V> "}, {
        6, "H:%h V:%v C:%C> "}, {
        7, "H:%h M:%m V:%v C:%C> "}, {
        8, "H:%h V:%v C:%C %S> "}, {
        9, "H:%h M:%m V:%v C:%C %S> "}, {
        10, "H:%h M:%m V:%v %c/%C %S> "}, {
        11, "%h/%H %m/%M %v/%V %g %c/%C> "}, {
        12, "H:%h/%H M:%m/%M V:%v/%V %g %c/%C> "}, {
        40, "H:%h R:%R> "}, {
        41, "H:%h R:%R i%iI+> "}, {
        42, "$c000BR:$c000W%R $c000BZ:$c000W%z $c000BFlags:$c000W%iF"
            " $c000BSect:$c000W%is $c000BI:$c000W%iI$c000B>"}, {
    0, NULL}};
    int             i,
                    n;

    dlog("in do_set_prompt");

#if 0
    if (IS_NPC(ch) || !ch->desc) {
        return;
    }
#endif

    argument = skip_spaces(argument);
    if (argument) {
        if ((n = atoi(argument)) != 0) {
            if (n > 39 && !IS_IMMORTAL(ch)) {
                send_to_char("Eh?\r\n", ch);
                return;
            }
            for (i = 0; prompts[i].pr; i++) {
                if (prompts[i].n == n) {
                    if (ch->specials.prompt) {
                        free(ch->specials.prompt);
                    }
                    ch->specials.prompt = strdup(prompts[i].pr);
                    if (cmd != 0) {
                        oldSendOutput(ch, "Your new prompt is : <%s>\n\r",
                                      ch->specials.prompt);
                    }
                    return;
                }
            }
            send_to_char("Invalid prompt number\n\r", ch);
        } else {
            if (ch->specials.prompt) {
                free(ch->specials.prompt);
            }
            ch->specials.prompt = strdup(argument);
            if (cmd != 0) {
                oldSendOutput(ch, "Your new prompt is : <%s>\n\r",
                              ch->specials.prompt);
            }
        }
    } else {
        oldSendOutput(ch, "Your current prompt is : %s\n\r", ch->specials.prompt);
    }
}

void do_set_bprompt(struct char_data *ch, char *argument, int cmd)
{
    static struct def_prompt {
        int             n;
        char           *pr;
    } prompts[] = {
        {
        1, "Havok> "}, {
        2, "H:%h V:%v> "}, {
        3, "H:%h M:%m V:%v> "}, {
        4, "H:%h/%H V:%v/%V> "}, {
        5, "H:%h/%H M:%m/%M V:%v/%V> "}, {
        6, "H:%h V:%v C:%C> "}, {
        7, "H:%h M:%m V:%v C:%C> "}, {
        8, "H:%h V:%v C:%C %S> "}, {
        9, "H:%h M:%m V:%v C:%C %S> "}, {
        10, "H:%h M:%m V:%v %c/%C %S> "}, {
        11, "%h/%H %m/%M %v/%V %g %c/%C> "}, {
        12, "H:%h/%H M:%m/%M V:%v/%V %g %c/%C> "}, {
        40, "H:%h R:%R> "}, {
        41, "H:%h R:%R i%iI+> "}, {
    0, NULL}};
    int             i,
                    n;

    dlog("in do_set_bprompt");

#if 0
    if (IS_NPC(ch) || !ch->desc) return;
#endif

    argument = skip_spaces(argument);
    if (argument) {
        if ((n = atoi(argument)) != 0) {
            if (n > 39 && !IS_IMMORTAL(ch)) {
                send_to_char("Eh?\r\n", ch);
                return;
            }
            for (i = 0; prompts[i].pr; i++) {
                if (prompts[i].n == n) {
                    if (ch->specials.bprompt) {
                        free(ch->specials.bprompt);
                    }
                    ch->specials.bprompt = strdup(prompts[i].pr);
                    if (cmd != 0) {
                        oldSendOutput(ch, "Your new battle prompt is : <%s>\n\r",
                                      ch->specials.bprompt);
                    }
                    return;
                }
            }
            send_to_char("Invalid prompt number\n\r", ch);
        } else {
            if (ch->specials.bprompt) {
                free(ch->specials.bprompt);
            }
            ch->specials.bprompt = strdup(argument);
            if (cmd != 0) {
                oldSendOutput(ch, "Your new battle prompt is : <%s>\n\r",
                              ch->specials.bprompt);
            }

        }
    } else {
        oldSendOutput(ch, "Your current battle prompt is : %s\n\r",
                      ch->specials.bprompt);
    }
}


/*
 * New title system.. Can have name as second word in title
 */

void do_title(struct char_data *ch, char *argument, int cmd)
{
    char            buf2[512];
    char           *arg1,
                   *arg2;

    char           *temp;
    dlog("in do_title");

    if (IS_NPC(ch) || !ch->desc) {
        return;
    }

    argument = skip_spaces(argument);
    if (argument) {
        if (strlen(argument) > 150) {
            send_to_char("Line too long, truncated\n", ch);
            argument[151] = '\0';
        }

        temp = strdup( argument );
        if( !temp ) {
            Log( "Out of memory in do_title" );
            return;
        }

        argument = get_argument(argument, &arg1);
        argument = get_argument(argument, &arg2);

        if (arg2 && strstr(CAP(arg2), GET_NAME(ch))) {
            sprintf(buf2, "%s", temp);
        } else {
            sprintf(buf2, "%s %s", GET_NAME(ch), temp);
        }
        free( temp );

        oldSendOutput(ch, "Your title has been set to : <%s>\n\r", buf2);
        if (ch->player.title) {
            free(ch->player.title);
        }
        ch->player.title = strdup(buf2);
    }
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_quit");
    if (IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM)) {
        return;
    }
    if (!argument || !*argument || strcmp(argument, "now")) {
        do_mobTell(ch, "A Tiny Voice",
                   "Psst. You should really rent at an Inn Keeper.");
        do_mobTell(ch, "A Tiny Voice",
                   "But if you wanna loose your stuff.. Type 'quit now'");
        return;
    }
    if (GET_POS(ch) == POSITION_FIGHTING) {
        send_to_char("No way! You are fighting.\n\r", ch);
        return;
    }
    if (GET_POS(ch) < POSITION_STUNNED) {
        send_to_char("You die before your time!\n\r", ch);
        Log("%s dies via quit.", GET_NAME(ch));
        die(ch, '\0');
        return;
    }
#if 0
    act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
#endif
    do_mobTell(ch, "A Tiny Voice", "Goodbye Friend.. Come Back Soon!");
    act("$n has left the game.", TRUE, ch, 0, 0, TO_ROOM);
    zero_rent(ch);

    /*
     * Char is saved in extract char
     */
    extract_char(ch);
#if 0
    close_socket(ch->desc);
#endif
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
    struct obj_cost cost;
    struct char_data *tmp;
    struct obj_data *tl;
    struct obj_data *teq[MAX_WEAR],
                   *o;
    int             i;

    dlog("in do_save");

    if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF))) {
        return;
    }
    if (cmd != 0) {
        send_to_char("Saving...\n\r", ch);
    }
    if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
        /*
         * swap stuff, and equipment
         */
        if (!ch->desc) {
            tmp = ch->orig;
        } else {
            tmp = ch->desc->original;
        }
        if (!tmp) {
            return;
        }
        tl = tmp->carrying;

        /*
         * there is a bug with this: When you save, the alignment thing is
         * checked, to see if you are supposed to be wearing what you are.
         * if your stuff gets kicked off your body, it will end up in room
         * #3, on the floor, and in the inventory of the polymorphed
         * monster.  This is a "bad" thing.  So, to fix it, each item in
         * the inventory is checked.  if it is in a room, it is moved from
         * the room, back to the correct inventory slot.
         */
        tmp->carrying = ch->carrying;
        for (i = 0; i < MAX_WEAR; i++) {
            /*
             * move all the mobs eq to the ch
             */
            teq[i] = tmp->equipment[i];
            tmp->equipment[i] = ch->equipment[i];
        }
        GET_EXP(tmp) = GET_EXP(ch);
        GET_GOLD(tmp) = GET_GOLD(ch);
        GET_ALIGNMENT(tmp) = GET_ALIGNMENT(ch);
        recep_offer(tmp, NULL, &cost, FALSE);
        save_obj(tmp, &cost, 0);

        /*
         * we can't save tmp because they don't have a desc.
         */
        save_char(ch, AUTO_RENT);

        write_char_extra(ch);
        tmp->carrying = tl;
        for (i = 0; i < MAX_WEAR; i++) {
            tmp->equipment[i] = teq[i];
            if (ch->equipment[i] && ch->equipment[i]->in_room != -1) {
                o = ch->equipment[i];
                ch->equipment[i] = 0;
                obj_from_room(o);
#if 0
                obj_to_char(o, ch);
#endif
                equip_char(ch, o, i);
                /*
                 * equip the correct slot
                 */
            }
        }
        return;
    } else {
        recep_offer(ch, NULL, &cost, FALSE);
        save_obj(ch, &cost, 0);
        save_char(ch, AUTO_RENT);
    }
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Sorry, but you cannot do that here!\n\r", ch);
}

void do_recallhome(struct char_data *victim, char *argument, int cmd)
{
    int             location;

    if (number(1, 50) <= GetMaxLevel(victim)) {
        send_to_char("You think of home but nothing happens.\n\r", victim);
        WAIT_STATE(victim, PULSE_VIOLENCE * 1);
        return;
    }
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
        send_to_char("HAH, not in a fight!\n\r", victim);
        return;
    }
    if (!IsOnPmp(victim->in_room) && GetMaxLevel(victim) > 10) {
        send_to_char("You can't recall!, you're on a different plane!\n\r",
                     victim);
        return;
    }

    /*
     * a location has been found.
     */

    act("$n thinks of home and then disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You close your eyes and think of home!  Suddenly, you find "
                 "yourself in a familiar location.\n\r", victim);
    do_look(victim, NULL, 15);
    GET_MOVE(victim) = 0;
    send_to_char("\n\rYou feel rather tired now!\n\r", victim);
}

void do_practice(struct char_data *ch, char *arg, int cmd)
{
    send_to_char("You can only practice at your guildmaster, use \"skills "
                 "class\" to list\n\r"
                 "your skills\n\r", ch );
}

void do_idea(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_idea");

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r",
                     ch);
        return;
    }

    db_report_entry( REPORT_IDEA, ch, argument );

    send_to_char("Ok. Thanks.\n\r", ch);
}

void do_typo(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_typo");

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("I beg your pardon?\n\r", ch);
        return;
    }

    db_report_entry( REPORT_TYPO, ch, argument );

    send_to_char("Ok. thanks.\n\r", ch);
}

void do_bug(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_bug");

    if (IS_NPC(ch)) {
        send_to_char("You are a monster! Bug off!\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    db_report_entry( REPORT_BUG, ch, argument );

    Log("BUG Report by %s [%ld]: %s", GET_NAME(ch), ch->in_room, argument);

    send_to_char("Ok.\n\r", ch);
}

void do_brief(struct char_data *ch, char *argument, int cmd)
{

    dlog("in do_brief");

    /*
     * for some reason mobs with a func keep setting themselves to brief
     * mode -Lennya
     */
    if (IS_NPC(ch)) {
        return;
    }

    if (IS_SET(ch->specials.act, PLR_BRIEF)) {
        send_to_char("Brief mode off.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_BRIEF);
    } else {
        send_to_char("Brief mode on.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_BRIEF);
    }
}

void do_compact(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_compact");

    if (IS_NPC(ch)) {
        return;
    }
    if (IS_SET(ch->specials.act, PLR_COMPACT)) {
        send_to_char("You are now in the uncompacted mode.\n\r", ch);
        REMOVE_BIT(ch->specials.act, PLR_COMPACT);
    } else {
        send_to_char("You are now in compact mode.\n\r", ch);
        SET_BIT(ch->specials.act, PLR_COMPACT);
    }
}

char           *Condition(struct char_data *ch)
{
    float           a,
                    b,
                    t;
    int             c;

    a = (float) GET_HIT(ch);
    b = (float) GET_MAX_HIT(ch);

    t = a / b;
    c = (int) 100.0 * t;

    return(how_good(c));
}

char           *Tiredness(struct char_data *ch)
{
    float           a,
                    b,
                    t;
    int             c;

    a = (float) GET_MOVE(ch);
    b = (float) GET_MAX_MOVE(ch);

    t = a / b;
    c = (int) 100.0 *t;

    return (how_good(c));
}

void do_group(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                    buf[256];
    struct char_data *victim,
                   *k;
    struct follow_type *f;
    bool            found;
    int             gnum;

    static char    *rand_groupname[] = {
        "The Seekers",          /* 0 */
        "The Subclan of Harpers",
        "The Farwalkers",
        "The God Squad",
        "The Vampire Slayers",
        "The Nobody Crew",      /* 5 */
        "The Dragon Hunters",   /* 6 */
        "The Lightning Catchers"
    };

    dlog("in do_group");

    if (A_NOGROUP(ch)) {
        send_to_char("The arena rules do not allow groups!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);
    if (!name) {
        if (!IS_AFFECTED(ch, AFF_GROUP)) {
            send_to_char("But you are a member of no group?!\n\r", ch);
        } else {
            if (ch->specials.group_name) {
                sprintf(buf, "$c0015Your group \"%s\" consists of:",
                        ch->specials.group_name);
            } else if (ch->master && ch->master->specials.group_name) {
                sprintf(buf, "$c0015Your group \"%s\" consists of:",
                        ch->master->specials.group_name);
            } else {
                sprintf(buf, "$c0015Your group consists of:");
            }
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
            if (ch->master) {
                k = ch->master;
            } else {
                k = ch;
            }
            if (IS_AFFECTED(k, AFF_GROUP) && GET_MAX_HIT(k) > 0
                && GET_MAX_MANA(k) > 0 && GET_MAX_MOVE(k) > 0) {
                sprintf(buf, "$c0014    %-15s $c0011(Head of group) $c0006"
                             "HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
                        fname(k->player.name),
                        ((float) GET_HIT(k) / (int) GET_MAX_HIT(k)) *
                        100.0 + 0.5,
                        ((float) GET_MANA(k) / (int) GET_MAX_MANA(k)) *
                        100.0 + 0.5,
                        ((float) GET_MOVE(k) / (int) GET_MAX_MOVE(k)) *
                        100.0 + 0.5);
                act(buf, FALSE, ch, 0, k, TO_CHAR);

            }

            for (f = k->followers; f; f = f->next) {
                if (IS_AFFECTED(f->follower, AFF_GROUP) &&
                    GET_MAX_HIT(f->follower) > 0 &&
                    GET_MAX_MANA(f->follower) > 0 &&
                    GET_MAX_MOVE(f->follower) > 0) {
                    sprintf(buf, "$c0014    %-15s             $c0011%s $c0006"
                            "HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%",
                            fname(f->follower->player.name),
                            IS_AFFECTED2(f->follower, AFF2_CON_ORDER) ?
                            "(O)" : "   ",
                            ((float) GET_HIT(f->follower) /
                             (int) GET_MAX_HIT(f->follower)) * 100.0 + 0.5,
                            ((float) GET_MANA(f->follower) /
                             (int) GET_MAX_MANA(f->follower)) * 100.0 + 0.5,
                            ((float) GET_MOVE(f->follower) /
                             (int) GET_MAX_MOVE(f->follower)) * 100.0 + 0.5);
                    act(buf, FALSE, ch, 0, f->follower, TO_CHAR);
                }
            }

        }
        return;
    }

    /*
     * ----- Start of the group all functions -----
     */
    if (strcasecmp(name, "self") == 0) {
        name = GET_NAME(ch);
    }

    if (!(strcmp(name, "all"))) {
        if (ch->master) {
            act("You can not enroll group members without being head of a "
                "group.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }

        if (!IS_AFFECTED(ch, AFF_GROUP)) {
            /*
             * If the leader is not grouped, group him...
             */
            SET_BIT(ch->specials.affected_by, AFF_GROUP);
            REMOVE_BIT(ch->specials.affected_by2, AFF2_CON_ORDER);
            act("$n is now a member of $s own group.", FALSE, ch, 0, ch,
                TO_ROOM);
            act("You are now a member of your group!", FALSE, ch, 0, ch,
                TO_CHAR);
        }

        for (found = FALSE, f = ch->followers; f; f = f->next) {
            victim = f->follower;
            if (!IS_NPC(victim)) {
                if (GetMaxLevel(victim) < MAX_IMMORT && IS_IMMORTAL(victim) &&
                    !IS_IMMORTAL(ch)) {
                    /*
                     * Do not let mortals group imms less than 60
                     */
                    act("You really don't want $N in your group.", FALSE, ch,
                        0, victim, TO_CHAR);
                    return;
                }
                
                if (GetMaxLevel(ch) < MAX_IMMORT && IS_IMMORTAL(ch) && 
                    !IS_IMMORTAL(victim)) {
                    /*
                     * Do not let imms group mortals except 60's
                     */
                    act("Now now.  That would be CHEATING!", FALSE, ch, 0, 0,
                        TO_CHAR);
                    return;
                }
            }

            if (!IS_AFFECTED(victim, AFF_GROUP)) {
                /*
                 * If the follower isn't grouped, group him
                 */
                act("$n is now a member of $N's group.", FALSE, victim, 0,
                    ch, TO_ROOM);
                act("You are now a member of $N's group.", FALSE, victim,
                    0, ch, TO_CHAR);
                SET_BIT(victim->specials.affected_by, AFF_GROUP);
                REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);
                found = TRUE;
            }
        }

        if (found) {
            /*
             * set group name if not one
             */
            if (!ch->master && !ch->specials.group_name && ch->followers) {
                if (GET_CLAN(ch) > 1) {
                    ch->specials.group_name =
                        strdup(clan_list[GET_CLAN(ch)].name);
                    sprintf(buf, "You form <%s> adventuring group!",
                            clan_list[GET_CLAN(ch)].name);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                } else {
                    gnum = number(0, MAX_GNAME);
                    ch->specials.group_name = strdup(rand_groupname[gnum]);
                    sprintf(buf, "You form <%s> adventuring group!",
                            rand_groupname[gnum]);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                }
            }
        }
    } else if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("No one here by that name.\n\r", ch);
    } else {
        if (ch->master) {
            act("You can not enroll group members without being head of a "
                "group.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }

        found = FALSE;

        if (victim == ch) {
            found = TRUE;
        } else {
            for (f = ch->followers; f; f = f->next) {
                if (f->follower == victim) {
                    found = TRUE;
                    break;
                }
            }
        }

        if (found) {
            if (IS_AFFECTED(victim, AFF_GROUP)) {
                if (victim != ch) {
                    act("$n has been kicked out of $N's group!", FALSE,
                        victim, 0, ch, TO_ROOM);
                    act("You are no longer a member of $N's group!", FALSE,
                        victim, 0, ch, TO_CHAR);
                } else {
                    act("$n has been kicked out of $s own group!", FALSE,
                        victim, 0, ch, TO_ROOM);
                    act("You are no longer a member of your group!", FALSE,
                        victim, 0, ch, TO_CHAR);
                }
                REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
                REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);
            } else {
                if (!IS_NPC(victim)) {
                    if (GetMaxLevel(victim) < MAX_IMMORT && 
                        IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
                        act("You really don't want $N in your group.", FALSE,
                            ch, 0, victim, TO_CHAR);
                        return;
                    }
                    if (GetMaxLevel(ch) < MAX_IMMORT && IS_IMMORTAL(ch) && 
                        !IS_IMMORTAL(victim)) {
                        act("Now now.  That would be CHEATING!", FALSE, ch, 0,
                            0, TO_CHAR);
                        return;
                    }

                }

                if (victim == ch) {
                    act("$n is now a member of $s own group!", FALSE,
                        victim, 0, ch, TO_ROOM);
                    act("You are now a member of your own group!", FALSE,
                        victim, 0, ch, TO_CHAR);
                } else {
                    act("$n is now a member of $N's group.", FALSE, victim,
                        0, ch, TO_ROOM);
                    act("You are now a member of $N's group.", FALSE,
                        victim, 0, ch, TO_CHAR);
                }
                SET_BIT(victim->specials.affected_by, AFF_GROUP);
                REMOVE_BIT(victim->specials.affected_by2, AFF2_CON_ORDER);

                /*
                 * set group name if not one
                 */
                if (!ch->master && !ch->specials.group_name && ch->followers) {
                    gnum = number(0, MAX_GNAME);
                    ch->specials.group_name = strdup(rand_groupname[gnum]);
                    sprintf(buf, "You form <%s> adventuring group!",
                            rand_groupname[gnum]);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                }

            }
        } else {
            act("$N must follow you, to enter the group", FALSE, ch, 0, victim,
                TO_CHAR);
        }
    }
}

void do_group_name(struct char_data *ch, char *arg, int cmd)
{
    int             count;
    struct follow_type *f;

    dlog("in do_group_name");

    /*
     * check to see if this person is the master
     */
    if (ch->master || !IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("You aren't the master of a group.\n\r", ch);
        return;
    }
    /*
     * check to see at least 2 pcs in group
     */
    for (count = 0, f = ch->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP) && IS_PC(f->follower)) {
            count++;
        }
    }

    if (count < 1) {
        send_to_char("You can't have a group with just one player!\n\r", ch);
        return;
    }

    /*
     * free the old ch->specials.group_name
     */
    if (ch->specials.group_name) {
        free(ch->specials.group_name);
        ch->specials.group_name = NULL;
    }

    /*
     * set ch->specials.group_name to the argument
     */
    arg = skip_spaces(arg);
    if( arg ) {
        oldSendOutput(ch, "Setting your group name to: %s\n\r", arg);
        ch->specials.group_name = strdup(arg);
    } else {
        send_to_char("Clearing your group name\n\r", ch);
    }
}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;
    struct obj_data *temp;
    int             i,
                    index;
    bool            equipped;
    Keywords_t     *key;

    equipped = FALSE;

    dlog("in do_quaff");

    argument = get_argument(argument, &buf);
    if( !buf ) {
        send_to_char( "Quaff whar?\n\r", ch );
        return;
    }

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
        temp = ch->equipment[HOLD];
        equipped = TRUE;
        key = StringToKeywords( buf, NULL );
        if (!temp || !KeywordsMatch(key, &temp->keywords)) {
            FreeKeywords(key, TRUE);
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        FreeKeywords(key, TRUE);
    }

    if (!IS_IMMORTAL(ch) && GET_COND(ch, FULL) > 23) {
        act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (temp->type_flag != ITEM_TYPE_POTION) {
        act("You can only quaff potions.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        if (number(1, 40) > ch->abilities.dex) {
            act("$n blindly fumbles $p to the ground!  It shatters!", TRUE,
                ch, temp, 0, TO_ROOM);
            act("You blindly fumble $p to the ground!  It shatters!", TRUE,
                ch, temp, 0, TO_CHAR);
            extract_obj(temp);
            return;
        }
    }

    if (ch->specials.fighting &&
        number(1, 20) > ch->abilities.dex - ( equipped ? 0 : 4 ) ) {
        act("$n is jolted and drops $p!  It shatters!",
            TRUE, ch, temp, 0, TO_ROOM);
        act("You arm is jolted and $p flies from your hand, *SMASH*", TRUE, ch,
            temp, 0, TO_CHAR);
        if (equipped) {
            temp = unequip_char(ch, HOLD);
        }
        extract_obj(temp);
        return;
    }

    act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
    act("You quaff $p which dissolves.", FALSE, ch, temp, 0, TO_CHAR);

    for (i = 1; i < 4; i++) {
        if (temp->value[i] >= 1) {
            index = spell_index[temp->value[i]];
            if( index != -1 && spell_info[index].spell_pointer ) {
                ((*spell_info[index].spell_pointer)
                 ((byte) temp->value[0], ch, "", SPELL_TYPE_POTION,
                  ch, temp));
            }
        }
    }

    if (equipped) {
        temp = unequip_char(ch, HOLD);
    }
    extract_obj(temp);

    if (!IS_IMMORTAL(ch)) {
        GET_COND(ch, FULL) += 1;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_recite(struct char_data *ch, char *argument, int cmd)
{
    char           *buf,
                   *buf2,
                   *tempname;
    struct obj_data *scroll,
                   *obj;
    struct char_data *victim;
    int             i,
                    index,
                    spl;
    bool            equipped;
    bool            target_ok;
    int             target = 0;
    Keywords_t     *key;

    target_ok = FALSE;
    equipped = FALSE;
    obj = 0;
    victim = 0;

    dlog("in do_recite");

    if (!ch->skills) {
        return;
    }
    if (A_NOMAGIC(ch)) {
        send_to_char("The arena rules do not allow you to use spells!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &buf);
    argument = get_argument(argument, &buf2);

    if( !buf ) {
        act("What are you trying to recite", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))) {
        scroll = ch->equipment[HOLD];
        equipped = TRUE;
        key = StringToKeywords(buf, NULL);
        if (!scroll || !KeywordsMatch(key, &scroll->keywords)) {
            FreeKeywords(key, TRUE);
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
        FreeKeywords(key, TRUE);
    }

    if (scroll->type_flag != ITEM_TYPE_SCROLL) {
        act("Recite is normally used for scrolls.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    spl = scroll->value[1];
    index = spell_index[spl];

    if (!spl || index == -1) {
        act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
        act("You recite $p which bursts into flame. Nothing happens.",
            FALSE, ch, scroll, 0, TO_CHAR);
        return;
    }

    if (buf2) {
        if (strcasecmp(buf2, "self") == 0) {
            tempname = strdup(GET_NAME(ch));
            buf2 = tempname;
        } else {
            tempname = NULL;
        }

        /*
         * Multiple spells can still crash if they don't have the same
         * spellinfo[i].target This will have to do for now -Lennya
         */
        if (IS_SET(spell_info[index].targets, TAR_CHAR_ROOM)) {
            if ((victim = get_char_room_vis(ch, buf2)) ||
                strcasecmp(GET_NAME(ch), buf2) == 0) {
                if (strcasecmp(GET_NAME(ch), buf2) == 0) {
                    victim = ch;
                }
                if (victim == ch || victim == ch->specials.fighting ||
                    victim->attackers < 6 ||
                    victim->specials.fighting == ch) {
                    target_ok = TRUE;
                    target = TAR_CHAR_ROOM;
                } else {
                    send_to_char("Too much fighting, you can't get a clear "
                                 "shot.\n\r", ch);
                    target_ok = FALSE;
                }
            } else {
                target_ok = FALSE;
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_CHAR_WORLD)) {
            if ((victim = get_char_vis(ch, buf2))) {
                target_ok = TRUE;
                target = TAR_CHAR_WORLD;
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_OBJ_INV)) {
            if ((obj = get_obj_in_list_vis(ch, buf2, ch->carrying))) {
                target = TAR_OBJ_INV;
                target_ok = TRUE;
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_OBJ_ROOM)) {
            if ((obj = get_obj_in_list_vis(ch, buf2,
                                     real_roomp(ch->in_room)->contents))) {
                target_ok = TRUE;
                target = TAR_OBJ_ROOM;
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_OBJ_WORLD)) {
            target_ok = TRUE;
            target = TAR_OBJ_WORLD;
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_OBJ_EQUIP)) {
            for (i = 0; i < MAX_WEAR && !target_ok; i++) {
                if (ch->equipment[i] && 
                    strcasecmp(buf2, ch->equipment[i]->name) == 0) {
                    obj = ch->equipment[i];
                    target_ok = TRUE;
                    target = TAR_OBJ_EQUIP;
                }
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_SELF_ONLY)) {
            if (strcasecmp(GET_NAME(ch), buf2) == 0) {
                victim = ch;
                target_ok = TRUE;
                target = TAR_SELF_ONLY;
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_NAME)) {
            obj = (void *) buf2;
            target_ok = TRUE;
            target = TAR_NAME;
        }

        if( tempname ) {
            free( tempname );
        }

        if (victim && IS_NPC(victim) &&
            IS_SET(victim->specials.act, ACT_IMMORTAL)) {
            send_to_char("You can't recite magic on that!\n\r", ch);
            return;
        }

        if (!target_ok) {
            send_to_char("No such thing around to recite the scroll on.\n\r",
                         ch);
            return;
        }
    } else {
        if (IS_SET(spell_info[index].targets, TAR_SELF_NONO)) {
            send_to_char("You cannot recite this scroll upon yourself.\n\r",
                         ch);
            return;
        } 
        
        victim = ch;
        target = TAR_CHAR_ROOM;
    }

    if (!HasClass(ch, CLASS_MAGIC_USER) &&
        !HasClass(ch, CLASS_CLERIC) &&
        !HasClass(ch, CLASS_DRUID) &&
        !HasClass(ch, CLASS_NECROMANCER) &&
        !HasClass(ch, CLASS_SORCERER)) {
        if (number(1, 95) > ch->skills[SKILL_READ_MAGIC].learned ||
            ch->skills[SKILL_READ_MAGIC].learned == 0) {
            WAIT_STATE(ch, PULSE_VIOLENCE * 3);
            send_to_char("After several seconds of study, your head hurts "
                         "trying to understand.\n\r", ch);
            return;
        }
    }

    act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
    act("You recite $p which bursts into flame.", FALSE, ch, scroll, 0,
        TO_CHAR);

    for (i = 1; i < 4; i++) {
        if (scroll->value[0] > 0) {
            /*
             * spells for casting
             */
            if (scroll->value[i] >= 1) {
                index = spell_index[scroll->value[i]];
                if (index == -1 || 
                    !IS_SET(spell_info[index].targets, target)) {
                    /*
                     * this should keep 2nd and 3rd spells
                     * from crashing with the wrong target
                     */
                    continue;
                }

                if (IS_SET(spell_info[index].targets, TAR_VIOLENT) &&
                    check_peaceful(ch, "Impolite magic is banned here.")) {
                    continue;
                }

                if (check_nomagic(ch,
                                  "The magic is blocked by unknown forces.\n\r",
                                  "The magic dissolves powerlessly.\n\r")) {
                    continue;
                }

                if( spell_info[index].spell_pointer ) {
                    ((*spell_info[index].spell_pointer)
                     ((byte) scroll->value[0], ch, "", 
                      SPELL_TYPE_SCROLL, victim, obj));
                }
            }
        } else {
            /*
             * this is a learning scroll
             */
            if (scroll->value[0] < -30) {
                /*
                 * max learning is 30%
                 */
                scroll->value[0] = -30;
            }
            if (scroll->value[i] > 0) {
                /*
                 * positive learning
                 */
                if (ch->skills &&
                    ch->skills[scroll->value[i]].learned < 45) {
                    ch->skills[scroll->value[i]].learned +=
                            (-scroll->value[0]);
                }
            } else {
                /*
                 * negative learning (cursed)
                 */
                if (scroll->value[i] < 0) {
                    /*
                     * 0 = blank
                     */
                    if (ch->skills) {
                        if (ch->skills[-scroll->value[i]].learned > 0) {
                            ch->skills[-scroll->value[i]].learned +=
                                scroll->value[0];
                        }
                        ch->skills[-scroll->value[i]].learned = 
                            MAX(0, ch->skills[scroll->value[i]].learned);
                    }
                }
            }
        }
    }

    if (equipped) {
        scroll = unequip_char(ch, HOLD);
    }

    extract_obj(scroll);
}

void do_swim(struct char_data *ch, char *arg, int cmd)
{

    struct affected_type af;
    byte            percent;

    send_to_char("Ok, you'll try to swim for a while.\n\r", ch);

    if (IS_AFFECTED(ch, AFF_WATERBREATH)) {
        /*
         * kinda pointless if they don't need to...
         */
        return;
    }

    if (affected_by_spell(ch, SKILL_SWIM)) {
        send_to_char("You're too exhausted to swim right now\n", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);

    if (!ch->skills) {
        return;
    }
    if (percent > ch->skills[SKILL_SWIM].learned) {
        send_to_char("You're too afraid to enter the water\n\r", ch);
        LearnFromMistake(ch, SKILL_SWIM, 0, 95);
        return;
    }

    af.type = SPELL_WATER_BREATH;
    af.duration = (ch->skills[SKILL_SWIM].learned / 10) + 1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_WATERBREATH;
    affect_to_char(ch, &af);

    af.type = SKILL_SWIM;
    af.duration = 13;
    af.modifier = -10;
    af.location = APPLY_MOVE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void do_use(struct char_data *ch, char *argument, int cmd)
{
    char           *buf,
                   *buf2,
                   *buf3,
                   *tempname;
    struct char_data *tmp_char;
    struct obj_data *tmp_object,
                   *stick;

    int             bits,
                    index;
    struct spell_info_type *spellp;
    Keywords_t     *key;

    dlog("in do_use");

    if (A_NOMAGIC(ch)) {
        send_to_char("The arena rules do not allow the use of magic!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &buf);
    argument = get_argument(argument, &buf2);
    argument = get_argument(argument, &buf3);

    if( !buf ) {
        send_to_char("Use what?\n\r", ch);
        return;
    }

    key = StringToKeywords(buf, NULL);
    if (!ch->equipment[HOLD] ||
        !KeywordsMatch(key, &ch->equipment[HOLD]->keywords)) {
        act("You do not hold that item in your hand.", FALSE, ch, 0, 0,
            TO_CHAR);
        return;
    }

    if (RIDDEN(ch)) {
        return;
    }

    stick = ch->equipment[HOLD];

    if (stick->type_flag == ITEM_TYPE_STAFF) {
        act("$n taps $p three times on the ground.", TRUE, ch, stick, 0,
            TO_ROOM);
        act("You tap $p three times on the ground.", FALSE, ch, stick, 0,
            TO_CHAR);
        if (stick->value[2] > 0) {
            /*
             * Are there any charges left?
             */
            stick->value[2]--;
            if (check_nomagic(ch, "The magic is blocked by unknown forces.",
                              "The magic is blocked by unknown forces.")) {
                return;
            }

            index = spell_index[stick->value[3]];
            if( index != -1 && spell_info[index].spell_pointer) {
                ((*spell_info[index].spell_pointer)
                 ((byte) stick->value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
                WAIT_STATE(ch, PULSE_VIOLENCE);
            }
        } else {
            send_to_char("The staff seems powerless.\n\r", ch);
        }
    } else if (buf2 && stick->type_flag == ITEM_TYPE_WAND) {
        if (!strcmp(buf2, "self")) {
            tempname = strdup(GET_NAME(ch));
            buf2 = tempname;
        } else {
            tempname = NULL;
        }

        bits = generic_find(buf2, FIND_CHAR_ROOM | FIND_OBJ_INV |
                                  FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch,
                            &tmp_char, &tmp_object);
        index = spell_index[stick->value[3]];

        if( index != -1 ) {
            if (bits) {
                spellp = &spell_info[index];

                if (bits == FIND_CHAR_ROOM) {
                    if (ch == tmp_char) {
                        act("$n points $p at $mself.", TRUE, ch, stick,
                            tmp_char, TO_ROOM);
                        act("You point $p at yourself.", FALSE, ch, stick,
                            tmp_char, TO_CHAR);
                    } else {
                        act("$n points $p at $N.", TRUE, ch, stick, tmp_char,
                            TO_NOTVICT);
                        act("$n points $p at you.", TRUE, ch, stick, tmp_char,
                            TO_VICT);
                        act("You point $p at $N.", FALSE, ch, stick, tmp_char,
                            TO_CHAR);
                    }
                } else {
                    act("$n points $p at $P.", TRUE, ch, stick, tmp_object,
                        TO_ROOM);
                    act("You point $p at $P.", FALSE, ch, stick, tmp_object,
                        TO_CHAR);
                }

                if (IS_SET(spellp->targets, TAR_VIOLENT) &&
                    check_peaceful(ch, "Impolite magic is banned here.")) {
                    return;
                }

                if (stick->value[2] > 0) {
                    /*
                     * Are there any charges left?
                     */
                    stick->value[2]--;

                    if (check_nomagic(ch,
                                   "The magic is blocked by unknown forces.",
                                   "The magic is blocked by unknown forces.")) {
                        return;
                    }

                    if( spellp->spell_pointer ) {
                        (*spellp->spell_pointer) 
                            ((byte) stick->value[0], ch, "", 
                              SPELL_TYPE_WAND, tmp_char, tmp_object);
                        WAIT_STATE(ch, PULSE_VIOLENCE);
                    }
                } else {
                    send_to_char("The wand seems powerless.\n\r", ch);
                }
            } else if (buf2 && buf3 && find_door(ch, buf2, buf3) >= 0) {
                /*
                 * For when the arg is a door
                 */
                spellp = &spell_info[index];

                if (stick->value[3] != SPELL_KNOCK) {
                    send_to_char("That spell is useless on doors.\n\r", ch);
                    return;
                }

                if (stick->value[2] > 0) {
                    /*
                     * Are there any charges left?
                     */
                    stick->value[2]--;

                    if (check_nomagic(ch,
                                   "The magic is blocked by unknown forces.",
                                   "The magic is blocked by unknown forces.")) {
                        return;
                    }

                    argument = strcat(strcat(buf2, " "), buf3);

                    if( spellp->spell_pointer ) {
                        (*spellp->spell_pointer)
                            ((byte) stick->value[0], ch, argument,
                             SPELL_TYPE_WAND, tmp_char, tmp_object);
                        WAIT_STATE(ch, PULSE_VIOLENCE);
                    }
                } else {
                    send_to_char("The wand seems powerless.\n\r", ch);
                }
            } else {
                send_to_char("What should the wand be pointed at?\n\r", ch);
            }
        }

        if( tempname ) {
            free( tempname );
        }
    } else {
        send_to_char("Use is normally only for wands and staves.\n\r", ch);
    }
}

void do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;

    dlog("in do_plr_noshout");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_DEAF)) {
            send_to_char("You can now hear shouts again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_DEAF);
        } else {
            send_to_char("From now on, you won't hear shouts.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_DEAF);
        }
    } else {
        send_to_char("Only the gods can shut up someone else. \n\r", ch);
    }
}

void do_plr_nogossip(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;

    dlog("in do_plr_no_yell_gossip");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOGOSSIP)) {
            send_to_char("You can now hear yells again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOGOSSIP);
        } else {
            send_to_char("From now on, you won't hear yell.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOGOSSIP);
        }
    } else {
        send_to_char("Only the gods can no yell someone else. \n\r", ch);
    }
}

void do_plr_noauction(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;

    dlog("in do_plr_noauction");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &buf);

    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOAUCTION)) {
            send_to_char("You can now hear auctions again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOAUCTION);
        } else {
            send_to_char("From now on, you won't hear auctions.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOAUCTION);
        }
    } else {
        send_to_char("Only the gods can no auction someone else. \n\r", ch);
    }
}

void do_plr_notell(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;

    dlog("in do_plr_notell");

    if (IS_NPC(ch)) {
        return;
    }
    
    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOTELL)) {
            send_to_char("You can now hear tells again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOTELL);
        } else {
            send_to_char("From now on, you won't hear tells.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOTELL);
        }
    } else {
        send_to_char("Only the gods can notell up someone else. \n\r", ch);
    }
}

void do_alias(struct char_data *ch, char *arg, int cmd)
{
    char            buf[512],
                    buf2[512];
    char           *p,
                   *p2;
    int             i,
                    num;

    dlog("in do_alias");

    if (cmd == 260) {
        arg = skip_spaces(arg);

        if (!arg) {
            /*
             * print list of current aliases
             */
            if (ch->specials.A_list) {
                for (i = 0; i < 10; i++) {
                    if (ch->specials.A_list->com[i]) {
                        oldSendOutput(ch, "[%d] %s\n\r", i,
                                      ch->specials.A_list->com[i]);
                    }
                }
            } else {
                send_to_char("You have no aliases defined!\n\r", ch);
                return;
            }
        } else {
            /*
             * assign a particular alias
             */
            if (!ch->specials.A_list) {
                ch->specials.A_list = (Alias *) malloc(sizeof(Alias));
                for (i = 0; i < 10; i++) {
                    ch->specials.A_list->com[i] = (char *) 0;
                }
            }
            strcpy(buf, arg);
            p = strtok(buf, " ");
            num = atoi(p);
            if (num < 0 || num > 9) {
                send_to_char("numbers between 0 and 9, please \n\r", ch);
                return;
            }
            if (GET_ALIAS(ch, num)) {
                free(GET_ALIAS(ch, num));
                GET_ALIAS(ch, num) = 0;
            }
            /*
             * testing
             */
            p = strtok(0, " ");
            /*
             * get the command string
             */
            if (!p) {
                send_to_char("Need to supply a command to alias, buddy\n\r",
                             ch);
                return;
            }
            p2 = strtok(p, " ");
            /*
             * check the command, make sure its not an alias
             */
            if (!p2) {
                send_to_char("Hmmmmm\n\r", ch);
                return;
            }
            if (*p2 >= '0' && *p2 <= '9') {
                send_to_char("Can't use an alias inside an alias\n\r", ch);
                return;
            }
            if (strncmp(p2, "alias", strlen(p2)) == 0) {
                send_to_char("Can't use the word 'alias' in an alias\n\r", ch);
                return;
            }
            /*
             * verified.. now the copy.
             */
            if (strlen(p) <= 80) {
                /*
                 * have to rebuild, because buf is full of nulls
                 */
                strcpy(buf2, arg);
                p = strchr(buf2, ' ');
                p++;
                ch->specials.A_list->com[num] =
                    (char *) malloc(strlen(p) + 1);
                strcpy(ch->specials.A_list->com[num], p);
            } else {
                send_to_char("alias must be less than 80 chars, lost\n\r", ch);
                return;
            }
        }
    } else {
        /*
         * execute this alias
         * 260 = alias
         */
        num = cmd - 260;
        if (num == 10) {
            num = 0;
        }
        if (ch->specials.A_list) {
            if (GET_ALIAS(ch, num)) {
                strcpy(buf, GET_ALIAS(ch, num));
                if (arg) {
                    sprintf(buf2, "%s %s", buf, arg);
                    command_interpreter(ch, buf2);
                } else {
                    command_interpreter(ch, buf);
                }
            }
        }
    }
}

void Dismount(struct char_data *ch, struct char_data *h, int pos)
{
    MOUNTED(ch) = 0;
    RIDDEN(h) = 0;
    GET_POS(ch) = pos;

    check_falling(ch);
}

void do_mount(struct char_data *ch, char *arg, int cmd)
{
    char           *name;
    int             check;
    struct char_data *horse;

    dlog("in do_mount");

    if (cmd == 276 || cmd == 278) {
        arg = get_argument(arg, &name);

        if (!name || !(horse = get_char_room_vis(ch, name))) {
            send_to_char("Mount what?\n\r", ch);
            return;
        }

        if (!IsHumanoid(ch)) {
            send_to_char("You can't ride things!\n\r", ch);
            return;
        }

        if (IsRideable(horse)) {
            if (GET_POS(horse) < POSITION_STANDING) {
                send_to_char("Your mount must be standing\n\r", ch);
                return;
            }

            if (RIDDEN(horse)) {
                send_to_char("Already ridden\n\r", ch);
                return;
            } else if (MOUNTED(ch)) {
                send_to_char("Already riding\n\r", ch);
                return;
            }

            check = MountEgoCheck(ch, horse);
            if (check > 5) {
                act("$N snarls and attacks!", FALSE, ch, 0, horse, TO_CHAR);
                act("as $n tries to mount $N, $N attacks $n!", FALSE, ch,
                    0, horse, TO_NOTVICT);
                WAIT_STATE(ch, PULSE_VIOLENCE);
                hit(horse, ch, TYPE_UNDEFINED);
                return;
            } else if (check > -1) {
                act("$N moves out of the way, you fall on your butt",
                    FALSE, ch, 0, horse, TO_CHAR);
                act("as $n tries to mount $N, $N moves out of the way",
                    FALSE, ch, 0, horse, TO_NOTVICT);
                WAIT_STATE(ch, PULSE_VIOLENCE);
                GET_POS(ch) = POSITION_SITTING;
                return;
            }

            if (RideCheck(ch, 50)) {
                act("You hop on $N's back", FALSE, ch, 0, horse, TO_CHAR);
                act("$n hops on $N's back", FALSE, ch, 0, horse, TO_NOTVICT);
                act("$n hops on your back!", FALSE, ch, 0, horse, TO_VICT);
                MOUNTED(ch) = horse;
                RIDDEN(horse) = ch;
                GET_POS(ch) = POSITION_MOUNTED;
                REMOVE_BIT(ch->specials.affected_by, AFF_SNEAK);
            } else {
                act("You try to ride $N, but falls on $s butt", FALSE, ch,
                    0, horse, TO_CHAR);
                act("$n tries to ride $N, but falls on $s butt", FALSE, ch,
                    0, horse, TO_NOTVICT);
                act("$n tries to ride you, but falls on $s butt", FALSE,
                    ch, 0, horse, TO_VICT);
                GET_POS(ch) = POSITION_SITTING;
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            }
        } else {
            send_to_char("You can't ride that!\n\r", ch);
        }
    } else if (cmd == 277) {
        horse = MOUNTED(ch);

        act("You dismount from $N", FALSE, ch, 0, horse, TO_CHAR);
        act("$n dismounts from $N", FALSE, ch, 0, horse, TO_NOTVICT);
        act("$n dismounts from you", FALSE, ch, 0, horse, TO_VICT);
        Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
}

void do_memorize(struct char_data *ch, char *argument, int cmd)
{

    int             spl,
                    diff;
    float           duration;
    struct affected_type af;
    char            buf[MAX_STRING_LENGTH * 2],
                    buffer[MAX_STRING_LENGTH * 2],
                    temp[20],
                   *spellnm;
    int             i, 
                    index;


    dlog("in do_memorize");

    if (IS_NPC(ch) && (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
        return;
    }
    if (!IsHumanoid(ch)) {
        send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch) && !HasClass(ch, CLASS_SORCERER)) {
        send_to_char("What, do you think you're a sorcerer?\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SKILL_MEMORIZE)) {
        send_to_char("You can only learn one spell at a time.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);

    /*
     * If there is no chars in argument
     */
    if (!argument) {
        send_to_char("Memorize 'spell name'\n\rCurrent spells in memory:\n\r",
                     ch);

        oldSendOutput(ch, "You can memorize one spell %d times, with a total of "
                      "%d spells memorized.\n\r",
                      MaxCanMemorize(ch, 0), TotalMaxCanMem(ch));
        oldSendOutput(ch, "You currently have %d spells memorized.\n\r",
                      TotalMemorized(ch));
        send_to_char("Your spellbook holds these spells:\n\r", ch);

        buffer[0] = '\0';
        for (i = 0; i < spell_info_count; i++) {
            index = spell_index[i + 1];
            if( index == -1 ) {
                continue;
            }

            if (spell_info[index].spell_pointer &&
                (spell_info[index].min_level_sorcerer <=
                 GET_LEVEL(ch, SORCERER_LEVEL_IND)) &&
                IS_SET(ch->skills[i + 1].flags, SKILL_KNOWN) &&
                IS_SET(ch->skills[i + 1].flags, SKILL_KNOWN_SORCERER) &&
                ch->skills[i + 1].nummem > 0) {
                sprintf(buf, "[%d] %s %s",
                        spell_info[index].min_level_sorcerer, spells[i],
                        how_good(ch->skills[i + 1].learned));
                if (IsSpecialized(ch->skills[i + 1].special)) {
                    strcat(buf, " (special)");
                }
                if (MEMORIZED(ch, i + 1)) {
                    sprintf(temp, " x%d", ch->skills[i + 1].nummem);
                    strcat(buf, temp);
                }

                strcat(buf, " \n\r");
                if (strlen(buf) + strlen(buffer) >
                     (MAX_STRING_LENGTH * 2) - 2) {
                    break;
                }
                strcat(buffer, buf);
                strcat(buffer, "\r");
            }
        }
        page_string(ch->desc, buffer, 0);
        return;
    }

    if (GET_POS(ch) > POSITION_SITTING) {
        send_to_char("You cannot do this standing up.\n\r", ch);

        if (affected_by_spell(ch, SKILL_MEMORIZE)) {
            SpellWearOff(SKILL_MEMORIZE, ch);
            affect_from_char(ch, SKILL_MEMORIZE);
            stop_memorizing(ch);
        }
        return;
    }

    argument = get_argument_delim(argument, &spellnm, '\'');
    if (!spellnm || spellnm[-1] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols "
                     ": '\n\r", ch);
        return;
    }

    spl = old_search_block(spellnm, 0, strlen(spellnm), spells, 0);
    index = spell_index[spl];

    if (!spl || index == -1) {
        send_to_char("You flip through your spell book but do not find that "
                     "spell.\n\r", ch);
        return;
    }

    if (!ch->skills) {
        return;
    }

    if ((spl > 0) && (spl < MAX_SKILLS) && spell_info[index].spell_pointer) {
        if (!IS_IMMORTAL(ch) &&
            spell_info[index].min_level_sorcerer >
            GET_LEVEL(ch, SORCERER_LEVEL_IND)) {
            send_to_char("Sorry, you do not have the skills for that "
                         "spell.\n\r", ch);
            return;
        }

        /*
         * Non-Sorcerer spell, cleric/druid or something else
         */
        if (spell_info[index].min_level_sorcerer == 0) {
            send_to_char("Sorry, you do not have the skills for that "
                         "spell.\n\r", ch);
            return;
        }

        /*
         * made it, lets memorize the spell!
         */
        if (ch->skills[spl].nummem < 0) {
            /*
             * should not happen
             */
            ch->skills[spl].nummem = 0;
        }

        /*
         * players skill in the spell is the base for duration to memorize
         */
        diff = ch->skills[spl].learned;
        if (diff <= 10) {
            duration = 14;
        } else if (diff <= 20) {
            duration = 12;
        } else if (diff <= 30) {
            duration = 10;
        } else if (diff <= 45) {
            duration = 8;
        } else if (diff <= 65) {
            duration = 4;
        } else if (diff <= 85) {
            duration = 2;
        } else if (diff <= 95) {
            duration = 1;
        } else {
            duration = 0;
        }

        /*
         * adjust directly for the level of the spell
         */
        diff = spell_info[index].min_level_magic;
        if (diff <= 5) {
            duration += 2;
        } else if (diff <= 10) {
            duration += 4;
        } else if (diff <= 25) {
            duration += 6;
        } else if (diff <= 45) {
            duration += 8;
        } else if (diff <= 47) {
            duration += 10;
        } else {
            duration += 12;
        }

        /*
         * adjust for intelligence
         */
        if (GET_INT(ch) >= 18) {
            duration -= 2;
        } else if (GET_INT(ch) >= 16) {
            duration -= 1;
        } else if (GET_INT(ch) >= 13) {
            duration -= 0;
        } else if (GET_INT(ch) >= 9) {
            duration += 1;
        } else if (GET_INT(ch) <= 8) {
            duration += 2;
        }

        /*
         * adjust for levels beyond min required for spell
         */
        diff = GET_LEVEL(ch, SORCERER_LEVEL_IND) -
               spell_info[index].min_level_sorcerer;
        if (diff >= 17) {
            duration -= 2;
        } else if (diff >= 7) {
            duration -= 1;
        }
#if 0
        else if ( diff >= 5 ) {
            duration -=0;
        }
#endif
        else if (diff <= 2) {
            duration += 2;
        }

        if (duration < 0) {
            duration = 0;
        } else if (duration > 24) {
            duration = 24;
        }

        af.type = SKILL_MEMORIZE;
        af.duration = duration;
        af.modifier = spl;
        /*
         * index of spell being memorized.
         */
        af.location = APPLY_NONE;
        af.bitvector = 0;

        ch->next_memorize = mem_list;
        mem_list = ch;
        affect_to_char(ch, &af);

        send_to_char("You flip open your spell book then begin to read and "
                     "meditate.\n\r", ch);
        act("$n opens a spell book then begins to read and meditate.",
            FALSE, ch, 0, 0, TO_ROOM);
    }

    return;
}

int TotalMaxCanMem(struct char_data *ch)
{
    extern struct int_app_type int_app[26];
    int             i;

    if (OnlyClass(ch, CLASS_SORCERER)) {
        i = GET_LEVEL(ch, SORCERER_LEVEL_IND);
    } else {
        /*
         * Multis get less spells
         */
        i = (int) (GET_LEVEL(ch, SORCERER_LEVEL_IND) / HowManyClasses(ch) *
                   0.5);
    }

    i += int_app[(int)GET_INT(ch)].learn / 2;
    return (i);
}

int TotalMemorized(struct char_data *ch)
{
    int             i,
                    ii = 0;

    for (i = 0; i < MAX_SKILLS; i++) {
        if (ch->skills[i].nummem &&
            IS_SET(ch->skills[i].flags, SKILL_KNOWN_SORCERER)) {
            ii += ch->skills[i].nummem;
        }
    }

    return (ii);
}

void check_memorize(struct char_data *ch, struct affected_type *af)
{
    int             max,
                    index;

    if (af->type == SKILL_MEMORIZE) {
        if (ch->skills[af->modifier].nummem >=
            MaxCanMemorize(ch, af->modifier)) {
            send_to_char("You cannot memorize this spell anymore than you "
                         "already have.\n\r", ch);
            return;
        }

        if (TotalMemorized(ch) >= TotalMaxCanMem(ch)) {
            send_to_char("You cannot memorize any more spells, your mind "
                         "cannot contain it!\n\r", ch);
            return;
        }

        /*
         * check for spellfail here ....
         */

        max = ch->specials.spellfail;

        /* 0 - 240 */
        max += GET_COND(ch, DRUNK) * 10;

        /*
         * check EQ and alter spellfail accordingly
         */
        switch (BestMagicClass(ch)) {
        case MAGE_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_MAGE)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case SORCERER_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_SORCERER)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case CLERIC_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_CLERIC)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case DRUID_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_DRUID)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case PALADIN_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_PALADIN)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case PSI_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_PSI)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        case RANGER_LEVEL_IND:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_RANGER)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        default:
            if (HasAntiBitsEquipment(ch, ITEM_ANTI_MAGE)) {
                max += 10;      /* 20% harder to cast spells */
            }
            break;
        }

        index = spell_index[af->modifier];
        if( index != -1 ) {
            if (ch->attackers > 0) {
                max += spell_info[index].spellfail;
            } else if (ch->specials.fighting) {
                max += spell_info[index].spellfail / 2;
            }
        }

        if (number(1, max) > ch->skills[af->modifier].learned &&
            !IsSpecialized(ch->skills[af->modifier].special)) {
            send_to_char("This spell eludes you, your efforts to memorize "
                         "fail.\n\r", ch);
            LearnFromMistake(ch, af->modifier, 0, 95);
        } else {
            ch->skills[af->modifier].nummem += 1;
        }
    }
}

void do_set_afk(struct char_data *ch, char *argument, int cmd)
{

    dlog("in do_set_afk");

    if (!ch) {
        return;
    }
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        return;
    }
    if (IS_AFFECTED2(ch, AFF2_AFK)) {
        act("$c0006$n has returned to $s keyboard", TRUE, ch, 0, 0, TO_ROOM);
        act("$c0006You return to the keyboard.", TRUE, ch, 0, 0, TO_CHAR);
        REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
        if (ch->pc) {
            REMOVE_BIT(ch->pc->comm, COMM_AFK);
        }
    } else {
        act("$c0006$n quietly goes Away From Keyboard.", TRUE, ch, 0, 0,
            TO_ROOM);
        act("$c0006You quietly go AFK.", TRUE, ch, 0, 0, TO_CHAR);
        SET_BIT(ch->specials.affected_by2, AFF2_AFK);
        if (ch->pc) {
            SET_BIT(ch->pc->comm, COMM_AFK);
        }
    }
}

void do_set_quest(struct char_data *ch, char *argument, int cmd)
{
    int             qcheck = 0;
    struct descriptor_data *i;
    struct char_data *tmp;

    dlog("in do_set_quest");

    if (!ch) {
        return;
    }
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        return;
    }
    if (IS_AFFECTED2(ch, AFF2_QUEST)) {
        qcheck = 0;
        if (IS_IMMORTAL(ch)) {
            /*
             * make him stop
             */
            REMOVE_BIT(ch->specials.affected_by2, AFF2_QUEST);

            /*
             * see if another visible imm sports a qflag
             */
            for (i = descriptor_list; i; i = i->next) {
                if (i->character && !i->connected &&
                    IS_IMMORTAL(i->character) &&
                    IS_AFFECTED2(i->character, AFF2_QUEST) &&
                    i->character->invis_level < IMMORTAL) {
                    /*
                     * there is, just stop questing
                     */
                    qcheck = 1;
                }
            }

            if (qcheck) {
                /*
                 * just stop
                 */
                act("$n stops questing.", TRUE, ch, 0, 0, TO_ROOM);
                act("You stop questing.", TRUE, ch, 0, 0, TO_CHAR);
            } else {
                act("You end the quest.", TRUE, ch, 0, 0, TO_CHAR);

                /*
                 * make all morts stop questing too
                 */
                for (i = descriptor_list; i; i = i->next) {
                    if (i->character && !i->connected &&
                        !IS_IMMORTAL(i->character) &&
                        IS_AFFECTED2(i->character, AFF2_QUEST)) {
                        /*
                         * there is, just stop questing
                         */
                        tmp = i->character;
                        act("$N has ended the quest.", TRUE, tmp, 0, ch,
                            TO_CHAR);
                        REMOVE_BIT(tmp->specials.affected_by2, AFF2_QUEST);
                    }
                }
            }
        } else {
            /*
             * not an imm
             */
            act("$n has stopped questing.", TRUE, ch, 0, 0, TO_ROOM);
            act("You stop questing.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by2, AFF2_QUEST);
        }
    } else {
        qcheck = 0;
        /*
         * imms can always go into questy mode
         */
        if (IS_IMMORTAL(ch)) {
            act("$n starts a quest!", TRUE, ch, 0, 0, TO_ROOM);
            qlog(ch, "started a quest!");
            act("The need to run a quest courses through your veins!",
                TRUE, ch, 0, 0, TO_CHAR);
            SET_BIT(ch->specials.affected_by2, AFF2_QUEST);
        } else {
            /*
             * see if there's a connected, questy imm
             */
            for (i = descriptor_list; i; i = i->next) {
                if (i->character && CAN_SEE(ch, i->character) &&
                    !i->connected && IS_IMMORTAL(i->character) &&
                    IS_AFFECTED2(i->character, AFF2_QUEST)) {
                    qcheck = 1;
                }
            }

            if (qcheck) {
                /*
                 * there is
                 */
                act("$n joins the quest!", TRUE, ch, 0, 0, TO_ROOM);
                qlog(ch, "joined the quest.");
                act("Okay, you're now part of the quest!", TRUE, ch, 0, 0,
                    TO_CHAR);
                SET_BIT(ch->specials.affected_by2, AFF2_QUEST);
            } else {
                /*
                 * there isn't
                 */
                act("You cannot embark on a quest without Divine Guidance.",
                    TRUE, ch, 0, 0, TO_CHAR);
            }
        }
    }
}

void do_flag_status(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Flag Status: \n\r", ch);
}


void do_auto(struct char_data *ch, char *argument, int cmd)
{
    switch (cmd) {
    case 576:
        if (IS_SET(ch->specials.act, PLR_AUTOASSIST)) {
            command_interpreter(ch, "set autoassist disable");
        } else {
            command_interpreter(ch, "set autoassist enable");
        }
        break;

    case 577:
        if (IS_SET(ch->specials.act, PLR_AUTOLOOT)) {
            command_interpreter(ch, "set autoloot disable");
        } else {
            command_interpreter(ch, "set autoloot enable");
        }
        break;

    case 578:
        if (IS_SET(ch->specials.act, PLR_AUTOGOLD)) {
            command_interpreter(ch, "set autogold disable");
        } else {
            command_interpreter(ch, "set autogold enable");
        }
        break;

    case 579:
        if (IS_SET(ch->specials.act, PLR_AUTOSPLIT)) {
            command_interpreter(ch, "set autosplit disable");
        } else {
            command_interpreter(ch, "set autosplit enable");
        }
        break;

    case 580:
        if (IS_SET(ch->specials.act, PLR_AUTOSAC)) {
            command_interpreter(ch, "set autosac disable");
        } else {
            command_interpreter(ch, "set autosac enable");
        }
        break;

    case 581:
        if (IS_SET(ch->specials.act, PLR_AUTOEXIT)) {
            command_interpreter(ch, "set autoexits disable");
        } else {
            command_interpreter(ch, "set autoexits enable");
        }
        break;

    default:
        command_interpreter(ch, "set");

        break;
    }
}

void do_set_flags(struct char_data *ch, char *argument, int cmd)
{
    char           *type,
                   *field,
                   *newfield,
                   *temp;

    dlog("in do_set_flags");

    if (!ch) {
        return;
    }

    argument = get_argument(argument, &type);
    field = skip_spaces(argument);

    if (!type) {
        send_to_char("Set, but set what?!?!? type help set for further "
                     "details on\n\r"
                     "email, answer, autoexits, groupname, clan, pause, "
                     "autoloot, autogold, autoassist, autosplit, ansi, "
                     "advice, sound, cloak oldcolors\n\r", ch);
        return;
    }

    if (!strcmp("war", type) && (!field)) {
        send_to_char("Use 'set war enable', REMEMBER ONCE THIS IS SET YOU "
                     "CANNOT REMOVE IT!\n\r"
                     "Be sure to READ the help on RACE WAR.\n\r", ch);
        return;
    }

    if (!strcmp(type, "cloak")) {
        if (!IS_SET(ch->player.user_flags, CLOAKED)) {
            if (!ch->equipment[12]) {
                send_to_char("You don't have a cloak to do that with.", ch);
                return;
            }

            SET_BIT(ch->player.user_flags, CLOAKED);
            act("You pull $p down over your body, cloaking your equipment "
                "from view.", FALSE, ch, ch->equipment[12], 0, TO_CHAR);
            act("$n pulls $p down over $s body, cloaking $s equipment from "
                "view.", FALSE, ch, ch->equipment[12], 0, TO_NOTVICT);
        } else {
            REMOVE_BIT(ch->player.user_flags, CLOAKED);
            if (!ch->equipment[12]) {
                send_to_char("You don't even have a cloak on.", ch);
                return;
            }
            act("You pull $p back away from your body.", FALSE, ch,
                ch->equipment[12], 0, TO_CHAR);
            act("$n pulls back $p away from $s body.", FALSE, ch,
                ch->equipment[12], 0, TO_NOTVICT);
        }
        return;
    }

    if (!field) {
        send_to_char("Set it to what? (Enable,Disable/Off)\n\r", ch);
        return;
    }

    /*
     * get rid of annoying carriage returns
     */
    while (field[strlen(field) - 1] == '\r' ||
           field[strlen(field) - 1] == '\n' ||
           field[strlen(field) - 1] == ' ') {
        field[strlen(field) - 1] = '\0';
    }

    newfield = strdup(field);
    if( !newfield ) {
        Log( "Out of memory in do_set" );
        return;
    }
    temp = newfield;

    temp = get_argument(temp, &field);
    if(!field) {
        Log("Strangeness in do_set");
        free( newfield );
        return;
    }

    if (!strcmp(type, "sound")) {
        /*
         * turn ansi stuff ON/OFF
         */
        if (is_abbrev(field, "enable")) {
            send_to_char("Sound and Music enabled.\n\r", ch);
            SET_BIT(ch->player.user_flags, ZONE_SOUNDS);
        } else {
            act("Sound and Music disabled.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, ZONE_SOUNDS)) {
                REMOVE_BIT(ch->player.user_flags, ZONE_SOUNDS);
            }
        }
    } else if (!strcmp(type, "private")) {
        /*
         * turn ansi stuff ON/OFF
         */
        if (is_abbrev(field, "enable")) {
            send_to_char("Private flag enabled.\n\r", ch);
            SET_BIT(ch->player.user_flags, CHAR_PRIVATE);
        } else {
            act("Private flag disabled.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, CHAR_PRIVATE)) {
                REMOVE_BIT(ch->player.user_flags, CHAR_PRIVATE);
            }
        }
    } else if (!strcmp(type, "ansi") || !strcmp(type, "color")) {
        /*
         * turn ansi stuff ON/OFF
         */
        if (is_abbrev(field, "enable")) {
            send_to_char("Setting ansi colors enabled.\n\r", ch);
            SET_BIT(ch->player.user_flags, USE_ANSI);
        } else {
            act("Setting ansi colors off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, USE_ANSI)) {
                REMOVE_BIT(ch->player.user_flags, USE_ANSI);
            }
        }
    } else if (!strcmp(type, "oldcolors") || !strcmp(type, "oldschool")) {
        /*
         * turn ansi stuff ON/OFF
         */
        if (is_abbrev(field, "enable")) {
            send_to_char("Setting old style colors.\n\r", ch);
            SET_BIT(ch->player.user_flags, OLD_COLORS);
        } else {
            act("Removing old school colors.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
                REMOVE_BIT(ch->player.user_flags, OLD_COLORS);
            }
        }
    } else if (!strcmp(type, "pause")) {
        /*
         * turn page mode ON/OFF
         */
        if (strstr(field, "enable")) {
            send_to_char("Setting page pause mode enabled.\n\r", ch);
            SET_BIT(ch->player.user_flags, USE_PAGING);
        } else {
            act("Turning page pause off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, USE_PAGING)) {
                REMOVE_BIT(ch->player.user_flags, USE_PAGING);
            }
        }
    } else if (!strcmp(type, "groupname")) {
        do_group_name(ch, newfield, 0);
    } else if (!strcmp(type, "autoexits")) {
        if (strstr(field, "enable")) {
            act("Setting autodisplay exits on.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->player.user_flags, SHOW_EXITS)) {
                SET_BIT(ch->player.user_flags, SHOW_EXITS);
            }
        } else {
            act("Setting autodisplay exits off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, SHOW_EXITS)) {
                REMOVE_BIT(ch->player.user_flags, SHOW_EXITS);
            }
        }
    } else if (!strcmp(type, "autosplit")) {
        if (strstr(field, "enable")) {
            act("Setting autosplit on.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->specials.act, PLR_AUTOSPLIT)) {
                SET_BIT(ch->specials.act, PLR_AUTOSPLIT);
            }
        } else {
            act("Setting autosplit off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->specials.act, PLR_AUTOSPLIT)) {
                REMOVE_BIT(ch->specials.act, PLR_AUTOSPLIT);
            }
        }
    } else if (!strcmp(type, "autogold")) {
        if (strstr(field, "enable")) {
            act("Setting autogold on.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->specials.act, PLR_AUTOGOLD)) {
                SET_BIT(ch->specials.act, PLR_AUTOGOLD);
            }
        } else {
            act("Setting autogold off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->specials.act, PLR_AUTOGOLD)) {
                REMOVE_BIT(ch->specials.act, PLR_AUTOGOLD);
            }
        }
    } else if (!strcmp(type, "autoloot")) {
        if (strstr(field, "enable")) {
            act("Setting autoloot on.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->specials.act, PLR_AUTOLOOT)) {
                SET_BIT(ch->specials.act, PLR_AUTOLOOT);
            }
        } else {
            act("Setting autoloot off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->specials.act, PLR_AUTOLOOT)) {
                REMOVE_BIT(ch->specials.act, PLR_AUTOLOOT);
            }
        }
    } else if (!strcmp(type, "autoassist")) {
        if (strstr(field, "enable")) {
            act("Setting autoassist on.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->specials.act, PLR_AUTOASSIST)) {
                SET_BIT(ch->specials.act, PLR_AUTOASSIST);
            }
        } else {
            act("Setting autoassist off.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->specials.act, PLR_AUTOASSIST)) {
                REMOVE_BIT(ch->specials.act, PLR_AUTOASSIST);
            }
        }
    } else if (!strcmp(type, "advice")) {
        if (strstr(field, "enable")) {
            act("Turning on Newbie Help.", FALSE, ch, 0, 0, TO_CHAR);
            if (!IS_SET(ch->player.user_flags, NEW_USER)) {
                SET_BIT(ch->player.user_flags, NEW_USER);
            }
        } else {
            act("Turning newbie help OFF.", FALSE, ch, 0, 0, TO_CHAR);
            if (IS_SET(ch->player.user_flags, NEW_USER)) {
                REMOVE_BIT(ch->player.user_flags, NEW_USER);
            }
        }
    } else if (!strcmp(type, "email")) {
        if (strstr(field, "disable")) {
            if (ch->specials.email) {
                free(ch->specials.email);
            }
            ch->specials.email = strdup("None");
            if (cmd) {
                write_char_extra(ch);
                send_to_char("Email address disabled.\n\r", ch);
            }
            return;
        }

        /*
         * set email to field
         */
        if (ch->specials.email) {
            free(ch->specials.email);
        }

        while( strchr( field, '\n' ) ) {
            *(strchr( field, '\n' )) = '\0';
        }

        while( strchr( field, '\r' ) ) {
            *(strchr( field, '\r' )) = '\0';
        }

        ch->specials.email = strdup(field);
        if (cmd) {
            write_char_extra(ch);
            send_to_char("Email address set.\n\r", ch);
        }
    } else {
        send_to_char("Unknown type to set.\n\r", ch);
        return;
    }
}

void do_finger(struct char_data *ch, char *argument, int cmd)
{
    char           *name;
    struct char_data *temp = 0;
    struct char_data *finger = 0;
    struct char_data *i;
    struct char_file_u tmp_store;
    int             akills = 0,
                    adeaths = 0;

    dlog("in do_finger");

    argument = get_argument(argument, &name);

    if (!name) {
        send_to_char("Whois who?!?!\n\r", ch);
        return;
    }

    CREATE(finger, struct char_data, 1);
    clear_char(finger);

    if (load_char(name, &tmp_store) > -1) {
        /*
         * load actual char
         */
        temp = finger;
        store_to_char(&tmp_store, finger);
        load_char_extra(finger);
        /*
         * Load Clan and email fields
         */

        if (IS_NPC(finger)) {
            send_to_char("No person by that name.\n\r", ch);
            return;
        }
        /*
         * Display Character information
         */
        send_to_char("        $c0008-=* $c000BAdventurer information $c0008"
                     "*=-\n\r", ch);
        oldSendOutput(ch, "$c000BName                  : $c0007%s\n\r",
                      finger->player.title);
        i = get_char(name);

        /*
         * Last time sited??
         */
        if ((IS_IMMORTAL(finger) && !IS_IMMORTAL(ch)) ||
            GetMaxLevel(finger) > GetMaxLevel(ch)) {
            oldSendOutput(ch, "$c000BLast time sited       : $c0007Unknown\n\r");
        } else if (i && i->desc) {
            /*
             * if there is a name, and a file descriptor
             */
            oldSendOutput(ch, "$c000BLast time sited       : $c0007Currently "
                          "Playing\n\r");
        } else {
            /* NOTE: asctime includes a \n\r at the end of the string */
            oldSendOutput(ch, "$c000BLast time sited       : $c0007%s",
                          asctime(localtime(&tmp_store.last_logon)));
        }

        /*
         * Display char email addy
         */
        if (finger->specials.email == NULL) {
            oldSendOutput(ch, "$c000BKnown message drop    : $c0007None\n\r");
        } else {
            oldSendOutput(ch, "$c000BKnown message drop    : $c0007%s\n\r",
                          finger->specials.email);
        }

        /*
         * Display clan info
         */
        oldSendOutput(ch, "$c000BClan info             : $c0007%s\n\r",
                      clan_list[GET_CLAN(finger)].name);

        if (IS_IMMORTAL(ch)) {
            if (finger->specials.hostip == NULL) {
                oldSendOutput(ch, "$c000BHostIP                : $c0007None\n\r");
            } else {
                oldSendOutput(ch, "$c000BHostIP                : $c0007%s\n\r",
                              finger->specials.hostip);
            }
        }

        if (finger->specials.rumor == NULL) {
            oldSendOutput(ch, "$c000BRumored info          : $c0007None\n\r");
        } else {
            oldSendOutput(ch, "$c000BRumored info           : $c0007%s\n\r",
                          finger->specials.rumor);
        }

        oldSendOutput(ch, "$c000BArena stats           : $c0007%d kills$c000B/"
                      "$c0007%d deaths\n\r",
                  finger->specials.a_kills, finger->specials.a_deaths);

        /*
         * let's give em a ratio to keep working on
         */
        akills = finger->specials.a_kills;
        adeaths = finger->specials.a_deaths;
        oldSendOutput(ch, "$c000BArena ratio           : $c0007%3.0f%%\n\r",
                  ((akills + adeaths) == 0) ? 0 :
                  (((float) akills /
                    ((int) (akills + adeaths))) * 100.0 + 0.5));
    } else {
        /*
         * Else there is no character in char DB
         */
        send_to_char("Character not found!!\n\r", ch);
    }

    /*
     * Ack.. dont' forget to free the char data of finger.
     */
    free(finger);
}

char           *trim(char *str)
{
    char           *ibuf,
                   *obuf;

    if (str) {
        for (ibuf = obuf = str; *ibuf;) {
            while (*ibuf && (isspace((int)*ibuf))) {
                ibuf++;
            }
            if (*ibuf && (obuf != str)) {
                *(obuf++) = ' ';
            }
            while (*ibuf && (!isspace((int)*ibuf))) {
                *(obuf++) = *(ibuf++);
            }
        }
        *obuf = '\0';
    }
    return (str);
}

void do_plr_noooc(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;

    dlog("in do_plr_noooc");

    if (IS_NPC(ch)) {
        return;
    }
    
    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOOOC)) {
            send_to_char("You can now hear the OOC channel again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOOOC);
        } else {
            send_to_char("From now on, you won't hear the OOC channel.\n\r",
                         ch);
            SET_BIT(ch->specials.act, PLR_NOOOC);
        }
    } else {
        send_to_char("Only the gods can shut up someone else. \n\r", ch);
    }
}

void do_arena(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_STRING_LENGTH];

    if (real_roomp(ch->in_room)->zone == ARENA_ZONE) {
        send_to_char("You're already in the Arena.\n\r", ch);
        return;
    }

    if (!MaxArenaLevel) {
        send_to_char("The Arena is closed.\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) < MinArenaLevel) {
        send_to_char("You're not strong enough to enter this arena.\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) > MaxArenaLevel) {
        send_to_char("You're too strong to enter this arena.\n\r", ch);
        return;
    }

    sprintf(buf, "%s just entered the ARENA!\n\r", GET_NAME(ch));
    send_to_all(buf);
    stop_follower(ch);
    spell_dispel_magic(60, ch, ch, 0);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    sprintf(buf, "%s disappears suddenly!\n\r", GET_NAME(ch));
    send_to_room_except(buf, ch->in_room, ch);
    char_from_room(ch);

    char_to_room(ch, ARENA_ENTRANCE);
    sprintf(buf, "%s appears in the room!\n\r", GET_NAME(ch));
    send_to_room_except(buf, ch->in_room, ch);
    do_look(ch, NULL, 0);
    return;
}

void do_promote(struct char_data *ch, char *arg, int cmd)
{
    int             count = 0;
    int             x = 0;
    struct char_data *c[25];
    char            buf[100];
    struct char_data *ch2;
    char           *name;
    struct follow_type *k;

    if (!arg) {
        send_to_char("Proper usage is:\n\rpromote <name>\n\r", ch);
        return;
    }

    arg = get_argument(arg, &name);
    if( !name ) {
        send_to_char("Proper usage is:\n\rpromote <name>\n\r", ch);
        return;
    }
    ch2 = get_char(name);


    if (!strcmp(name, GET_NAME(ch))) {
        send_to_char("Your already the leader of the group.\n\r", ch);
        return;
    }

    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("But you aren't even IN a group.\n\r", ch);
        return;
    } else if (!ch2) {
        send_to_char("You don't see that person.\n\r", ch);
        return;
    } else if (ch->master && ch->master->player.name == ch2->player.name) {
        send_to_char("You already follow that person.\n\r", ch);
        return;
    } else if (ch->master) {
        send_to_char
            ("Maybe you should ask your group leader to do that.\n\r", ch);
        return;
    } else if (IS_NPC(ch2)) {
        send_to_char("You might want to entrust that position to an actual "
                     "character.\n\r", ch);
        return;
    } else if (!IS_AFFECTED(ch2, AFF_GROUP) || ch2->master != ch) {
        send_to_char("They aren't even in your group!\n\r", ch);
        return;
    }

    act("$c0012You promote $N to leader of the group.\r", FALSE, ch, 0,
        ch2, TO_CHAR);
    act("$c0012You have been promoted by $n to lead the group\r", FALSE,
        ch, 0, ch2, TO_VICT);

    /*
     * Person to be promoted follows himself
     */
#if 0
    stop_follower(ch2);
#endif
    k = ch->followers;

    /*
     * There is a faster method in doing this.. This is just a temp. fix.
     * Greg Hovey
     */

    /*
     * takes all chars in group and adds it to an character array..
     */
    for (k = ch->followers; k; k = k->next) {
        if (k) {
            c[count] = k->follower;
            count++;
        }
    }

    sprintf(buf, "$c0012%s has been promoted to leader of the group.\r",
            GET_NAME(ch2));
    for (x = 0; x < count; x++) {
        if (ch2 != c[x]) {
            /*
             * display to each char wasn't going on..
             */
            act(buf, FALSE, c[x], 0, name, TO_CHAR);
        }

        stop_follower_quiet(c[x]);

        /*
         * add follower stuff..
         */
        c[x]->master = ch2;
        CREATE(k, struct follow_type, 1);

        k->follower = c[x];
        k->next = ch2->followers;
        ch2->followers = k;
    }

    ch->master = ch2;
    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = ch2->followers;
    ch2->followers = k;

    /* the new leader shouldn't be following anyone at all, needs to be a 
     * leader
     */
    stop_follower_quiet(ch2);
    SET_BIT(ch2->specials.affected_by, AFF_GROUP);
    REMOVE_BIT(ch2->specials.affected_by2, AFF2_CON_ORDER);
}

void do_behead(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *j = NULL;
    struct obj_data *head = NULL;
    char           *arg1,
                   *itemname,
                    temp[MAX_STRING_LENGTH],
                    buf[MAX_STRING_LENGTH];
    int             r_num = 0;

    if (IS_NPC(ch) || !ch->skills) {
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("Not from this mount you cannot!\n\r", ch);
        return;
    }

    if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        argument = get_argument(argument, &itemname);
    }

    if (!itemname) {
        send_to_char("Behead what?\n\r", ch);
        return;
    }

    /*
     * Find Corpse
     */
    if (!(j = get_obj_in_list_vis(ch, itemname,
                                  real_roomp(ch->in_room)->contents))) {
        send_to_char("Where did that bloody corpse go?\n\r", ch);
        return;
    } 
    
    /*
     * affect[0] == race of corpse, affect[1] == level of corpse
     */
    if (j->affected[0].modifier == 0 || j->affected[1].modifier == 0) {
        send_to_char("Sorry, the corpse is too mangled up to behead.\n\r", ch);
        return;
    }

    /*
     * item not a corpse if v3 = 0
     */
    if (!IS_CORPSE(j)) {
        send_to_char("Sorry, this is not a carcass.\n\r", ch);
        return;
    }

    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon, to make it a "
                     "success.\n\r", ch);
        return;
    }

    /*
     * need to do weapon check..
     */
    if (!(Getw_type(ch->equipment[WIELD]) == TYPE_SLASH ||
          Getw_type(ch->equipment[WIELD]) == TYPE_CLEAVE)) {
        send_to_char("Your weapon isn't really good for that type of "
                     "thing.\n\r", ch);
        return;
    }

    /* Take the "corpse of the blah" and remove the corpse part */
    argument = strdup( j->short_description );
    arg1 = argument;
    strsep( &arg1, " " );
    strsep( &arg1, " " );
    sprintf(buf, "%s", arg1);
    free( argument );

    /*
     * load up the head object
     */
    head = read_object(SEVERED_HEAD, VIRTUAL);
    if (!head) {
        Log("ERROR IN Behead.. make head object");
        return;
    }

    /*
     * to room perhaps?
     */
    obj_to_room(head, ch->in_room);

    /*
     * CHange name of head
     */
    FreeKeywords(&head->keywords, FALSE);
    sprintf(temp, "head %s", buf);
    StringToKeywords(temp, &head->keywords);

    if (head->short_description) {
        free(head->short_description);
    }
    sprintf(temp, "The head %s", buf);
    head->short_description = strdup(temp);

    if (head->description) {
        free(head->description);
    }
    sprintf(temp, "The head %s lies here.", buf);
    head->description = strdup(temp);

    /*
     * make corpse unusable for another behead
     */
    j->affected[1].modifier = 0;

    if (j->description) {
        arg1 = j->description;
        strsep( &arg1, " " );
        sprintf(buf, "The beheaded %s", arg1);
        free(j->description);
    }
    j->description = strdup(buf);

    oldSendOutput(ch, "You behead %s.\n\r", j->short_description);

    sprintf(buf, "%s beheads %s.", GET_NAME(ch), j->short_description);
    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE * 1);
}

void do_top10(struct char_data *ch, char *arg, int cmd)
{
    struct char_data *tmp;
    struct char_file_u player;
    int             i = 0;
    int             deadly = 0,
                    tempd = 0,
                    richest = 0;
    char            deadlyname[16],
                    richestname[16];

    if (!ch) {
        return;
    }
    for (i = 0; i < top_of_p_table + 1; i++) {
        if (load_char((player_table + i)->name, &player) > -1) {
            /*
             * store to a tmp char that we can deal with
             */
            CREATE(tmp, struct char_data, 1);
            clear_char(tmp);
            store_to_char(&player, tmp);
            if (!IS_IMMORTAL(tmp)) {
                if (GET_GOLD(tmp) > richest) {
                    richest = GET_GOLD(tmp);
                    sprintf(richestname, "%s", GET_NAME(tmp));
                }
                if ((tempd = CalcPowerLevel(tmp)) > deadly) {
                    deadly = tempd;
                    sprintf(deadlyname, "%s", GET_NAME(tmp));
                }
            }
            free(tmp);
        } else {
            Log("screw up bigtime in load_char, saint part, in clanlist");
            return;
        }
    }

    oldSendOutput(ch, "Deadly: %s   %d.\n\rRichest: %s   %d\n\r", deadlyname,
              deadly, richestname, richest);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
