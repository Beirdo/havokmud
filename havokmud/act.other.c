/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"

/*
 * extern variables
 */
extern int      ArenaNoGroup,
                ArenaNoAssist,
                ArenaNoDispel,
                ArenaNoMagic,
                ArenaNoWSpells,
                ArenaNoSlay,
                ArenaNoFlee,
                ArenaNoHaste,
                ArenaNoPets,
                ArenaNoTravel,
                ArenaNoBash;
extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern struct dex_skill_type dex_app_skill[];
extern struct spell_info_type spell_info[];
extern int       spell_index[MAX_SPL_LIST];
extern struct char_data *character_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern char    *spells[];
extern int      MaxArenaLevel,
                MinArenaLevel;
struct char_data *mem_list = 0;
extern int      top_of_p_table;
extern struct player_index_element *player_table;
/*
 * head for the list of memorizers
 */
extern struct skillset warriorskills[];
extern struct skillset thiefskills[];
extern struct skillset barbskills[];
extern struct skillset necroskills[];
extern struct skillset monkskills[];
extern struct skillset warmonkskills[];
extern struct skillset mageskills[];
extern struct skillset sorcskills[];
extern struct skillset clericskills[];
extern struct skillset druidskills[];
extern struct skillset paladinskills[];
extern struct skillset rangerskills[];
extern struct skillset psiskills[];
extern struct skillset mainwarriorskills[];
extern struct skillset mainthiefskills[];
extern struct skillset mainbarbskills[];
extern struct skillset mainnecroskills[];
extern struct skillset mainmonkskills[];
extern struct skillset mainmageskills[];
extern struct skillset mainsorcskills[];
extern struct skillset mainclericskills[];
extern struct skillset maindruidskills[];
extern struct skillset mainpaladinskills[];
extern struct skillset mainrangerskills[];
extern struct skillset mainpsiskills[];
extern struct skillset warninjaskills[];
extern struct skillset thfninjaskills[];
extern struct skillset allninjaskills[];
extern struct skillset loreskills[];
extern struct skillset styleskillset[];
extern const struct clan clan_list[MAX_CLAN];

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
    for (; isspace(*argument); argument++) {
        /*
         * Empty got loop
         */
    }
    if (!*argument) {
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
    } else {
        if (!str_cmp(argument, "on")) {
            if (!IS_SET(ch->specials.act, ACT_GUARDIAN)) {
                SET_BIT(ch->specials.act, ACT_GUARDIAN);
                act("$n alertly watches you.", FALSE, ch, 0, ch->master,
                    TO_VICT);
                act("$n alertly watches $N.", FALSE, ch, 0, ch->master,
                    TO_NOTVICT);
                send_to_char("You snap to attention\n\r", ch);
            }
        } else if (!str_cmp(argument, "off")) {
            if (IS_SET(ch->specials.act, ACT_GUARDIAN)) {
                act("$n relaxes.", FALSE, ch, 0, 0, TO_ROOM);
                send_to_char("You relax.\n\r", ch);
                REMOVE_BIT(ch->specials.act, ACT_GUARDIAN);
            }
        }
    }
}

void do_junk(struct char_data *ch, char *argument, int cmd)
{
    char            arg[100],
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
    only_argument(argument, arg);
    if (*arg) {
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
            if (IS_OBJ_STAT(tmp_object, ITEM_NODROP)) {
                send_to_char("You can't let go of it, it must be CURSED!\n\r",
                             ch);
                return;
            }
            value += (MIN(1000, MAX(tmp_object->obj_flags.cost / 4, 1)));
            obj_from_char(tmp_object);
#if 0
            obj_index[tmp_object->item_number].number--;
#endif
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
    0, NULL}};
    char            buf[512];
    int             i,
                    n;

    dlog("in do_set_prompt");

#if 0
    if (IS_NPC(ch) || !ch->desc) {
        return;
    }
#endif

    for (; isspace(*argument); argument++) {
        /*
         * Empty loop
         */
    }
    if (*argument) {
        if ((n = atoi(argument)) != 0) {
            if (n > 39 && !IS_IMMORTAL(ch)) {
                send_to_char("Eh?\r\n", ch);
                return;
            }
            for (i = 0; prompts[i].pr; i++) {
                if (prompts[i].n == n) {
#if 0
                    sprintf(buf, "Your prompt now is : <%s>\n\r",
                    argument); send_to_char(buf, ch);
#endif
                    if (ch->specials.prompt) {
                        free(ch->specials.prompt);
                    }
                    ch->specials.prompt = strdup(prompts[i].pr);
                    if (cmd != 0) {
                        sprintf(buf, "Your new prompt is : <%s>\n\r",
                                ch->specials.prompt);
                        send_to_char(buf, ch);
                    }
                    return;
                }
            }
            send_to_char("Invalid prompt number\n\r", ch);
        } else {
#if 0
            sprintf(buf, "Your prompt now is : <%s>\n\r", argument);
            send_to_char(buf, ch);
#endif
            if (ch->specials.prompt) {
                free(ch->specials.prompt);
            }
            ch->specials.prompt = strdup(argument);
            if (cmd != 0) {
                sprintf(buf, "Your new prompt is : <%s>\n\r",
                        ch->specials.prompt);
                send_to_char(buf, ch);
            }
        }
    } else {
        sprintf(buf, "Your current prompt is : %s\n\r", ch->specials.prompt);
        send_to_char(buf, ch);
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
    char            buf[512];
    int             i,
                    n;

    dlog("in do_set_bprompt");

#if 0
    if (IS_NPC(ch) || !ch->desc) return;
#endif

    for (; isspace(*argument); argument++) {
        /*
         * Empty loop
         */
    }
    if (*argument) {
        if ((n = atoi(argument)) != 0) {
            if (n > 39 && !IS_IMMORTAL(ch)) {
                send_to_char("Eh?\r\n", ch);
                return;
            }
            for (i = 0; prompts[i].pr; i++) {
                if (prompts[i].n == n) {
#if 0
                    sprintf(buf, "Your prompt now is : <%s>\n\r",
                    argument); send_to_char(buf, ch);
#endif
                    if (ch->specials.bprompt) {
                        free(ch->specials.bprompt);
                    }
                    ch->specials.bprompt = strdup(prompts[i].pr);
                    if (cmd != 0) {
                        sprintf(buf, "Your new battle prompt is : <%s>\n\r",
                                ch->specials.bprompt);
                        send_to_char(buf, ch);
                    }
                    return;
                }
            }
            send_to_char("Invalid prompt number\n\r", ch);
        } else {
#if 0
            sprintf(buf, "Your prompt now is : <%s>\n\r", argument);
            send_to_char(buf, ch);
#endif
            if (ch->specials.bprompt) {
                free(ch->specials.bprompt);
            }
            ch->specials.bprompt = strdup(argument);
            if (cmd != 0) {
                sprintf(buf, "Your new battle prompt is : <%s>\n\r",
                        ch->specials.bprompt);
                send_to_char(buf, ch);
            }

        }
    } else {
        sprintf(buf, "Your current battle prompt is : %s\n\r",
                ch->specials.bprompt);
        send_to_char(buf, ch);
    }
}


/*
 * New title system.. Can have name as second word in title
 */

void do_title(struct char_data *ch, char *argument, int cmd)
{
    char            buf[512];
    char            buf2[512];

    char            temp[512];
    dlog("in do_title");

    if (IS_NPC(ch) || !ch->desc) {
        return;
    }
    for (; isspace(*argument); argument++) {
        /*
         * Empty  for loop
         */
    }
    if (*argument) {
        if (strlen(argument) > 150) {
            send_to_char("Line too long, truncated\n", ch);
            *(argument + 151) = '\0';
#if 0
            sprintf(buf2,"%s %s", GET_NAME(ch), argument);
#endif
        }

        sprintf(temp, "%s", argument);
        argument = one_argument(argument, buf);
        argument = one_argument(argument, buf2);
#if 0
        send_to_char(CAP(buf2),ch);
        send_to_char(GET_NAME(ch),ch);
#endif
        if (strstr(CAP(buf2), GET_NAME(ch))) {
            sprintf(buf2, "%s", temp);
        } else {
            sprintf(buf2, "%s %s", GET_NAME(ch), temp);
        }
        sprintf(buf, "Your title has been set to : <%s>\n\r", buf2);
        send_to_char(buf, ch);
        if (ch->player.title) {
            free(ch->player.title);
        }
        ch->player.title = strdup(buf2);
    }
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
    char            buf[256];

    dlog("in do_quit");
    if (IS_NPC(ch) || !ch->desc || IS_AFFECTED(ch, AFF_CHARM)) {
        return;
    }
    if (!*argument || strcmp(argument, "now")) {
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
        sprintf(buf, "%s dies via quit.", GET_NAME(ch));
        Log(buf);
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
    do_look(victim, "", 15);
    GET_MOVE(victim) = 0;
    send_to_char("\n\rYou feel rather tired now!\n\r", victim);
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_sneak");

    if (IS_AFFECTED2(ch, AFF2_SKILL_SNEAK)) {
#if 0
        affect_from_char(ch, SKILL_SNEAK);
#endif
        if (IS_AFFECTED(ch, AFF_HIDE)) {
            REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
        }
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        send_to_char("You are no longer sneaky.\n\r", ch);
        return;
    }
    if (!HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_RANGER)) {
        send_to_char("You're not trained to walk silently!\n\r", ch);
        return;
    }
    if (HasClass(ch, CLASS_RANGER) && !OUTSIDE(ch)) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("You must do this outdoors!\n\r", ch);
            return;
        }
    }
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    if (!IS_AFFECTED(ch, AFF_SILENCE)) {
        /*
         * removed this to balance with to many sneak items.. (GH)
         */
#if 0
        if (EqWBits(ch, ITEM_ANTI_THIEF)) {
            send_to_char("Gonna be hard to "
                    "sneak around in that!\n\r", ch); return;
        }
#endif
        if (HasWBits(ch, ITEM_HUM)) {
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
#if 0
    if (percent > ch->skills[SKILL_SNEAK].learned +
            dex_app_skill[GET_DEX(ch)].sneak) {
        LearnFromMistake(ch, SKILL_SNEAK, 1, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE); return;
    }
#endif
    if (IS_SET(ch->specials.affected_by2, AFF2_SKILL_SNEAK)) {
        send_to_char("You stop being sneaky!", ch);
#if 0
        affect_from_char(ch, SKILL_SNEAK);
#endif
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
    } else {
        send_to_char("You start jumping from shadow to shadow.", ch);
        SET_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }

#if 0
    af.type = SKILL_SNEAK;
    af.duration = GET_LEVEL(ch, BestThiefClass(ch));
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SNEAK;
    affect_to_char(ch, &af);
    WAIT_STATE(ch, PULSE_VIOLENCE);
#endif
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_hide");

    if (cmd == 334 && !HasClass(ch, CLASS_BARBARIAN)) {
        send_to_char("Hey, you can't do that!\n\r", ch);
        return;
    }
    if (cmd == 153 && !HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_RANGER)) {
        send_to_char("Hey, you can't do that!\n\r", ch);
        return;
    }
    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }
    if (!HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_BARBARIAN |
                      CLASS_RANGER)) {
        send_to_char("You're not trained to hide!\n\r", ch);
        return;
    }
    if (!HasClass(ch, CLASS_BARBARIAN | CLASS_RANGER)) {
        send_to_char("You attempt to hide in the shadows.\n\r", ch);
    } else {
        send_to_char("You attempt to camouflage yourself.\n\r", ch);
    }
    if (HasClass(ch, CLASS_BARBARIAN | CLASS_RANGER) && !OUTSIDE(ch)) {
        send_to_char("You must do this outdoors.\n\r", ch);
        return;
    }
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

void do_steal(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char            victim_name[240];
    char            obj_name[240];
    char            buf[240];
    int             percent;
    int             gold,
                    eq_pos;
    bool            ohoh = FALSE;

    dlog("in do_steal");

    if (!ch->skills) {
        return;
    }
    if (check_peaceful(ch, "What if he caught you?\n\r")) {
        return;
    }
    argument = one_argument(argument, obj_name);
    only_argument(argument, victim_name);

    if (!HasClass(ch, CLASS_THIEF)) {
        send_to_char("You're no thief!\n\r", ch);
        return;
    }
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    if (!(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Steal what from who?\n\r", ch);
        return;
    } else if (victim == ch) {
        send_to_char("Come on now, that's rather stupid!\n\r", ch);
        return;
    }
    if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
        send_to_char("Steal from a God?!?  Oh the thought!\n\r", ch);
        sprintf(buf, "NOTE: %s tried to steal from GOD %s", GET_NAME(ch),
                GET_NAME(victim));
        Log(buf);
        return;
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    if ((GetMaxLevel(ch) < 2) && (!IS_NPC(victim))) {
        send_to_char("Due to misuse of steal, you can't steal from other "
                     "players\n\r", ch);
        send_to_char("unless you are at least 2nd level. \n\r", ch);
        return;
    }
    if ((!victim->desc) && (!IS_NPC(victim))) {
        return;
    }

    /*
     * 101% is a complete failure
     */
    percent = number(1, 101) - dex_app_skill[(int)GET_DEX(ch)].p_pocket;

    if (GET_POS(victim) < POSITION_SLEEPING || GetMaxLevel(ch) >= IMPLEMENTOR) {
        /*
         * ALWAYS SUCCESS
         */
        percent = -1;
    }

    percent += GET_AVE_LEVEL(victim);

    if (GetMaxLevel(victim) > MAX_MORT && GetMaxLevel(ch) < IMPLEMENTOR) {
        percent = 101;
        /*
         * Failure
         */
    }
    if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold")) {
        if (!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) {
            for (eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++) {
                if (victim->equipment[eq_pos] &&
                    isname(obj_name, victim->equipment[eq_pos]->name) &&
                    CAN_SEE_OBJ(ch, victim->equipment[eq_pos])) {
                    obj = victim->equipment[eq_pos];
                    break;
                }
            }

            if (!obj) {
                act("$E has not got that item.", FALSE, ch, 0, victim,
                    TO_CHAR);
                return;
            } else {
                /*
                 * It is equipment
                 */
                if ((GET_POS(victim) > POSITION_STUNNED)) {
                    send_to_char("Steal the equipment now? Impossible!\n\r",
                                 ch);
                    return;
                } else {
                    act("You unequip $p and steal it.", FALSE, ch, obj, 0,
                        TO_CHAR);
                    act("$n steals $p from $N.", FALSE, ch, obj, victim,
                        TO_NOTVICT);
                    obj_to_char(unequip_char(victim, eq_pos), ch);
#if NODUPLICATES
                    do_save(ch, "", 0);
                    do_save(victim, "", 0);
#endif
                    if (IS_PC(ch) && IS_PC(victim)) {
                        GET_ALIGNMENT(ch) -= 20;
                    }
                }
            }
        } else {
            /*
             * obj found in inventory
             */
            if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
                send_to_char("You can't steal it, it must be CURSED!\n\r", ch);
                return;
            }

            percent += GET_OBJ_WEIGHT(obj);
            /*
             * Make heavy harder
             */
            if (AWAKE(victim) && percent > ch->skills[SKILL_STEAL].learned) {
                ohoh = TRUE;
                act("Yikes, you fumbled!", FALSE, ch, 0, 0, TO_CHAR);
                LearnFromMistake(ch, SKILL_STEAL, 0, 90);
                SET_BIT(ch->player.user_flags, STOLE_1);
                act("$n tried to steal something from you!", FALSE, ch, 0,
                    victim, TO_VICT);
                act("$n tries to steal something from $N.", TRUE, ch, 0,
                    victim, TO_NOTVICT);
            } else {
                /*
                 * Steal the item
                 */
                if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
                    if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) <
                        CAN_CARRY_W(ch)) {
                        obj_from_char(obj);
                        obj_to_char(obj, ch);
                        send_to_char("Got it!\n\r", ch);
#if NODUPLICATES
                        do_save(ch, "", 0);
                        do_save(victim, "", 0);
#endif
                        if (IS_PC(ch) && IS_PC(victim)) {
                            GET_ALIGNMENT(ch) -= 20;
                        }
                    } else {
                        send_to_char("You cannot carry that much.\n\r", ch);
                    }
                } else
                    send_to_char("You cannot carry that much.\n\r", ch);
            }
        }
    } else {
        /*
         * Steal some coins
         */
        if (AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
            ohoh = TRUE;
            act("Oops..", FALSE, ch, 0, 0, TO_CHAR);
            if (ch->skills[SKILL_STEAL].learned < 90) {
                act("Even though you were caught, you realize your mistake "
                    "and promise to remember.", FALSE, ch, 0, 0, TO_CHAR);
                ch->skills[SKILL_STEAL].learned++;
                if (ch->skills[SKILL_STEAL].learned >= 90) {
                    send_to_char("You are now learned in this skill!\n\r", ch);
                }
            }
            act("You discover that $n has $s hands in your wallet.", FALSE,
                ch, 0, victim, TO_VICT);
            act("$n tries to steal gold from $N.", TRUE, ch, 0, victim,
                TO_NOTVICT);
        } else {
            /*
             * Steal some gold coins
             */
            gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
            gold = MIN(number(1000, 2000), gold);
            if (gold > 0) {
                GET_GOLD(ch) += gold;
                GET_GOLD(victim) -= gold;
                sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
                send_to_char(buf, ch);
                if (IS_PC(ch) && IS_PC(victim)) {
                    GET_ALIGNMENT(ch) -= 20;
                }
            } else {
                send_to_char("You couldn't get any gold...\n\r", ch);
            }
        }
    }

    if (ohoh && IS_NPC(victim) && AWAKE(victim)) {
        if (IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
            sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
            do_shout(victim, buf, 0);
            do_say(victim, "Don't you ever do that again!", 0);
        } else {
            if (CAN_SEE(victim, ch)) {
                hit(victim, ch, TYPE_UNDEFINED);
            } else if (number(0, 1)) {
                hit(victim, ch, TYPE_UNDEFINED);
            }
        }
    }
}

void do_practice(struct char_data *ch, char *arg, int cmd)
{
    char            buf[254],
                    buffer[MAX_STRING_LENGTH],
                    temp[20];
    int             i = 0;

    dlog("in do_practice");

    buffer[0] = '\0';

    if ((cmd != 164) && (cmd != 170)) {
        return;
    }
    if (!ch->skills) {
        return;
    }
    for (; isspace(*arg); arg++) {
        /*
         * Empty loop
         */
    }
    if (!arg) {
        send_to_char("You need to supply a class for that.", ch);
        return;
    }
    switch (*arg) {
    case 'w':
    case 'W':
    case 'f':
    case 'F':{
            if (!HasClass(ch, CLASS_WARRIOR)) {
                send_to_char("I bet you think you're a warrior.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (warriorskills[i].level != -1) {
                if (IS_SET(ch->skills[warriorskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            warriorskills[i].level, warriorskills[i].name,
                            how_good(ch->skills[warriorskills[i].skillnum].
                                     learned));
                    if (IsSpecialized(
                            ch->skills[warriorskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == WARRIOR_LEVEL_IND + 1) {
                while (mainwarriorskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainwarriorskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainwarriorskills[i].level,
                                mainwarriorskills[i].name,
                                how_good(ch->
                                         skills[mainwarriorskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[mainwarriorskills[i].skillnum].
                            special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (warninjaskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[warninjaskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            warninjaskills[i].level,
                            warninjaskills[i].name,
                            how_good(ch->
                                     skills[warninjaskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[warninjaskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (warmonkskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[warmonkskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            warmonkskills[i].level, warmonkskills[i].name,
                            how_good(ch->skills[warmonkskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[warmonkskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET(ch->skills[styleskillset[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 't':
    case 'T':{
            if (!HasClass(ch, CLASS_THIEF)) {
                send_to_char("I bet you think you're a thief.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (thiefskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[thiefskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            thiefskills[i].level, thiefskills[i].name,
                            how_good(ch->skills[thiefskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[thiefskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == THIEF_LEVEL_IND + 1) {
                while (mainthiefskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainthiefskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainthiefskills[i].level,
                                mainthiefskills[i].name,
                                how_good(ch->
                                         skills[mainthiefskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[mainthiefskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (thfninjaskills[i].level != -1) {
                if (IS_SET(ch->skills[thfninjaskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            thfninjaskills[i].level,
                            thfninjaskills[i].name,
                            how_good(ch->
                                     skills[thfninjaskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[thfninjaskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET(ch->skills[styleskillset[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'M':
    case 'm':{
            if (!HasClass(ch, CLASS_MAGIC_USER)) {
                send_to_char("I bet you think you're a mage.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (mageskills[i].level != -1) {
                if (IS_SET(ch->skills[mageskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", mageskills[i].level,
                            mageskills[i].name,
                            how_good(ch->skills[mageskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[mageskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == MAGE_LEVEL_IND + 1) {
                while (mainmageskills[i].level != -1) {
                    if (IS_SET
                        (ch->skills[mainmageskills[i].skillnum].flags,
                         SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainmageskills[i].level,
                                mainmageskills[i].name,
                                how_good(ch->
                                         skills[mainmageskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainmageskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[loreskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'S':
    case 's':{
            if (!HasClass(ch, CLASS_SORCERER)) {
                send_to_char("I bet you think you're a sorcerer.\n\r", ch);
                return;
            }
            sprintf(buf, "You can memorize one spell %d times, with a total "
                         "of %d spells memorized.\n\r",
                    MaxCanMemorize(ch, 0), TotalMaxCanMem(ch));
            send_to_char(buf, ch);
            sprintf(buf, "You currently have %d spells memorized.\n\r",
                    TotalMemorized(ch));
            send_to_char(buf, ch);
            send_to_char("Your spellbook holds these spells:\n\r", ch);
            while (sorcskills[i].level != -1) {
                if (IS_SET(ch->skills[sorcskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", sorcskills[i].level,
                            sorcskills[i].name,
                            how_good(ch->skills[sorcskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[sorcskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    if (MEMORIZED(ch, sorcskills[i].skillnum)) {
                        sprintf(temp, " [x%d]",
                                ch->skills[sorcskills[i].skillnum].nummem);
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
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == SORCERER_LEVEL_IND + 1) {
                while (mainsorcskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainsorcskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainsorcskills[i].level,
                                mainsorcskills[i].name,
                                how_good(ch->
                                         skills[mainsorcskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainsorcskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'C':
    case 'c':
        {
            if (!HasClass(ch, CLASS_CLERIC)) {
                send_to_char("I bet you think you're a cleric.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (clericskills[i].level != -1) {
                if (IS_SET(ch->skills[clericskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            clericskills[i].level, clericskills[i].name,
                            how_good(ch->skills[clericskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[clericskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == CLERIC_LEVEL_IND + 1) {
                while (mainclericskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainclericskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainclericskills[i].level,
                                mainclericskills[i].name,
                                how_good(ch->
                                         skills[mainclericskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainclericskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'D':
    case 'd':
        {
            if (!HasClass(ch, CLASS_DRUID)) {
                send_to_char("I bet you think you're a druid.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (druidskills[i].level != -1) {
                if (IS_SET(ch->skills[druidskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            druidskills[i].level, druidskills[i].name,
                            how_good(ch->skills[druidskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[druidskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == DRUID_LEVEL_IND + 1) {
                while (maindruidskills[i].level != -1) {
                    if (IS_SET(ch->skills[maindruidskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                maindruidskills[i].level,
                                maindruidskills[i].name,
                                how_good(ch->
                                         skills[maindruidskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[maindruidskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;
    case 'K':
    case 'k':{
            if (!HasClass(ch, CLASS_MONK)) {
                send_to_char("I bet you think you're a monk.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (monkskills[i].level != -1) {
                if (IS_SET(ch->skills[monkskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", monkskills[i].level,
                            monkskills[i].name,
                            how_good(ch->skills[monkskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[monkskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == MONK_LEVEL_IND + 1) {
                while (mainmonkskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainmonkskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainmonkskills[i].level,
                                mainmonkskills[i].name,
                                how_good(ch->
                                         skills[mainmonkskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainmonkskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET(ch->skills[styleskillset[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;
    case 'b':
    case 'B':
        {
            if (!HasClass(ch, CLASS_BARBARIAN)) {
                send_to_char("I bet you think you're a Barbarian.\n\r",
                             ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (barbskills[i].level != -1) {
                if (IS_SET(ch->skills[barbskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", barbskills[i].level,
                            barbskills[i].name,
                            how_good(ch->skills[barbskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[barbskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
                while (mainbarbskills[i].level != -1) {
                    if (IS_SET(ch->skills[mainbarbskills[i].skillnum].flags,
                               SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainbarbskills[i].level,
                                mainbarbskills[i].name,
                                how_good(ch->
                                         skills[mainbarbskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainbarbskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET(ch->skills[loreskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET(ch->skills[styleskillset[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'n':
    case 'N':
        {
            if (!HasClass(ch, CLASS_NECROMANCER)) {
                send_to_char("I bet you think you're a necromancer.\n\r",
                             ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (necroskills[i].level != -1) {
                if (IS_SET(ch->skills[necroskills[i].skillnum].flags,
                           SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            necroskills[i].level, necroskills[i].name,
                            how_good(ch->skills[necroskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[necroskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == NECROMANCER_LEVEL_IND + 1) {
                while (mainnecroskills[i].level != -1) {
                    if (IS_SET
                        (ch->skills[mainnecroskills[i].skillnum].flags,
                         SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainnecroskills[i].level,
                                mainnecroskills[i].name,
                                how_good(ch->
                                         skills[mainnecroskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainnecroskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[loreskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'R':
    case 'r':{
            if (!HasClass(ch, CLASS_RANGER)) {
                send_to_char("I bet you think you're a ranger.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (rangerskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[rangerskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            rangerskills[i].level, rangerskills[i].name,
                            how_good(ch->skills[rangerskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[rangerskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == RANGER_LEVEL_IND + 1) {
                while (mainrangerskills[i].level != -1) {
                    if (IS_SET
                        (ch->skills[mainrangerskills[i].skillnum].flags,
                         SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainrangerskills[i].level,
                                mainrangerskills[i].name,
                                how_good(ch->
                                         skills[mainrangerskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainrangerskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[loreskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET
                    (ch->skills[styleskillset[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'i':
    case 'I':{
            if (!HasClass(ch, CLASS_PSI)) {
                send_to_char("I bet you think you're a psionist.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (psiskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[psiskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", psiskills[i].level,
                            psiskills[i].name,
                            how_good(ch->skills[psiskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[psiskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
                while (mainpsiskills[i].level != -1) {
                    if (IS_SET
                        (ch->skills[mainpsiskills[i].skillnum].flags,
                         SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainpsiskills[i].level,
                                mainpsiskills[i].name,
                                how_good(ch->
                                         skills[mainpsiskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[mainpsiskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[loreskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;

    case 'P':
    case 'p':{
            if (!HasClass(ch, CLASS_PALADIN)) {
                send_to_char("I bet you think you're a paladin.\n\r", ch);
                return;
            }
            sprintf(buffer, "You have knowledge of these skills:\n\r\n\r");
            while (paladinskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[paladinskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            paladinskills[i].level, paladinskills[i].name,
                            how_good(ch->skills[paladinskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[paladinskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
                while (mainpaladinskills[i].level != -1) {
                    if (IS_SET
                        (ch->skills[mainpaladinskills[i].skillnum].flags,
                         SKILL_KNOWN)) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                mainpaladinskills[i].level,
                                mainpaladinskills[i].name,
                                how_good(ch->
                                         skills[mainpaladinskills[i].
                                                skillnum].learned));
                        if (IsSpecialized
                            (ch->skills[mainpaladinskills[i].skillnum].
                             special)) {
                            strcat(buf, " (special)");
                        }
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2) {
                            break;
                        }
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
            }
            i = 0;
            while (loreskills[i].level != -1) {
                if (IS_SET
                    (ch->skills[loreskills[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                            loreskills[i].name,
                            how_good(ch->skills[loreskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[loreskills[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            i = 0;
            while (styleskillset[i].level != -1) {
                if (IS_SET
                    (ch->skills[styleskillset[i].skillnum].flags,
                     SKILL_KNOWN)) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            styleskillset[i].level, styleskillset[i].name,
                            how_good(ch->skills[styleskillset[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[styleskillset[i].skillnum].special)) {
                        strcat(buf, " (special)");
                    }
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2) {
                        break;
                    }
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                }
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return;
        }
        break;
    default:
        send_to_char("Which class?\n\r", ch);
    }

    send_to_char
        ("Go to your guildmaster to see the spells you don't have.\n\r",
         ch);

}

void do_idea(struct char_data *ch, char *argument, int cmd)
{
    FILE           *fl;
    char            str[MAX_INPUT_LENGTH + 20];

    dlog("in do_idea");

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
        return;
    }

    /*
     * skip whites
     */
    for (; isspace(*argument); argument++) {
        /*
         * Empty loop
         */
    }

    if (!*argument) {
        send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r",
                     ch);
        return;
    }
    if (!(fl = fopen(IDEA_FILE, "a"))) {
        perror("do_idea");
        send_to_char("Could not open the idea-file.\n\r", ch);
        return;
    }

    sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);

    fputs(str, fl);
    fclose(fl);
    send_to_char("Ok. Thanks.\n\r", ch);
}

void do_typo(struct char_data *ch, char *argument, int cmd)
{
    FILE           *fl;
    char            str[MAX_INPUT_LENGTH + 20];

    dlog("in do_typo");

    if (IS_NPC(ch)) {
        send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
        return;
    }

    /*
     * skip whites
     */
    for (; isspace(*argument); argument++) {
        /*
         * Empty loop
         */
    }

    if (!*argument) {
        send_to_char("I beg your pardon?\n\r", ch);
        return;
    }
    if (!(fl = fopen(TYPO_FILE, "a"))) {
        perror("do_typo");
        send_to_char("Could not open the typo-file.\n\r", ch);
        return;
    }

    sprintf(str, "**%s[%ld]: %s\n", GET_NAME(ch), ch->in_room, argument);
    fputs(str, fl);
    fclose(fl);
    send_to_char("Ok. thanks.\n\r", ch);
}

void do_bug(struct char_data *ch, char *argument, int cmd)
{
    FILE           *fl;
    char            str[MAX_INPUT_LENGTH + 20];

    dlog("in do_bug");

    if (IS_NPC(ch)) {
        send_to_char("You are a monster! Bug off!\n\r", ch);
        return;
    }

    /*
     * skip whites
     */
    for (; isspace(*argument); argument++) {
        /*
         * Empty loop
         */
    }

    if (!*argument) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    if (!(fl = fopen(BUG_FILE, "a"))) {
        perror("do_bug");
        send_to_char("Could not open the bug-file.\n\r", ch);
        return;
    }

    sprintf(str, "**%s[%ld]: %s\n", GET_NAME(ch), ch->in_room, argument);
    fputs(str, fl);
    fclose(fl);
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
    static char     buf[100];
    static char    *p;

    a = (float) GET_HIT(ch);
    b = (float) GET_MAX_HIT(ch);

    t = a / b;
    c = (int) 100.0 *t;

    strcpy(buf, how_good(c));
    p = buf;
    return (p);

}

char           *Tiredness(struct char_data *ch)
{
    float           a,
                    b,
                    t;
    int             c;
    static char     buf[100];
    static char    *p;

    a = (float) GET_MOVE(ch);
    b = (float) GET_MAX_MOVE(ch);

    t = a / b;
    c = (int) 100.0 *t;

    strcpy(buf, how_good(c));
    p = buf;
    return (p);

}

void do_group(struct char_data *ch, char *argument, int cmd)
{
    char            name[256],
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

    only_argument(argument, name);

    if (!*name) {
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
    if (str_cmp(name, "self") == 0) {
        sprintf(name, "%s", GET_NAME(ch));
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
            if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
                /*
                 * Do not let mortals group imms
                 */
                act("You really don't want $N in your group.", FALSE, ch,
                    0, victim, TO_CHAR);
                return;
            }
            /*
             * victim stronger??
             */

            if ((GetMaxLevel(victim) - GetMaxLevel(ch)) > 8) {
                act("$N looks to be too strong to join you.",
                        FALSE, ch, 0, victim, TO_CHAR);
                return;
            }

            /*
             * your stronger??
             */

            if ((GetMaxLevel(ch) - GetMaxLevel(victim)) > 8) {
                act("$N looks to be too puny and week to join you.",
                        FALSE, ch, 0, victim, TO_CHAR);
                return;
            }

            if (IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
                /*
                 * Do not let imms group mortals
                 */
                act("Now now.  That would be CHEATING!", FALSE, ch, 0, 0,
                    TO_CHAR);
                return;
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
                if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch)) {
                    act("You really don't want $N in your group.", FALSE,
                        ch, 0, victim, TO_CHAR);
                    return;
                }
                if (IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) {
                    act("Now now.  That would be CHEATING!", FALSE, ch, 0,
                        0, TO_CHAR);
                    return;
                }

                /*
                 * victim stronger??
                 */
                if ((GetMaxLevel(victim) - GetMaxLevel(ch)) > 8) {
                    act("$N looks to be too strong to join you.", FALSE,
                        ch, 0, victim, TO_CHAR);
                    return;
                }

                /*
                 * your stronger??
                 */
                if ((GetMaxLevel(ch) - GetMaxLevel(victim)) > 8) {
                    act("$N looks to be too puny and week to join you.",
                        FALSE, ch, 0, victim, TO_CHAR);
                    return;
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
        ch->specials.group_name = 0;
    }
    /*
     * set ch->specials.group_name to the argument
     */
    for (; isspace(*arg); arg++) {
        /*
         * Empty loop
         */
    }

    send_to_char("Setting your group name to: ", ch);
    send_to_char(arg, ch);
    send_to_char("\n\r", ch);
    ch->specials.group_name = strdup(arg);

}

void do_quaff(struct char_data *ch, char *argument, int cmd)
{
    char            buf[100];
    struct obj_data *temp;
    int             i,
                    index;
    bool            equipped;

    equipped = FALSE;

    dlog("in do_quaff");

    only_argument(argument, buf);

    if (!(temp = get_obj_in_list_vis(ch, buf, ch->carrying))) {
        temp = ch->equipment[HOLD];
        equipped = TRUE;
        if ((temp == 0) || !isname(buf, temp->name)) {
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }

    if (!IS_IMMORTAL(ch) && GET_COND(ch, FULL) > 23) {
        act("Your stomach can't contain anymore!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (temp->obj_flags.type_flag != ITEM_POTION) {
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

    /*
     * my stuff
     */
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
        if (temp->obj_flags.value[i] >= 1) {
            index = spell_index[temp->obj_flags.value[i]];
            if( index != -1 && spell_info[index].spell_pointer ) {
                ((*spell_info[index].spell_pointer)
                 ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION,
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
    char            buf[100],
                    buf2[100],
                    buf3[100];
    struct obj_data *scroll,
                   *obj;
    struct char_data *victim;
    int             i,
                    index,
                    spl;
    bool            equipped;
    bool            target_ok;
    int             target = 0;

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

    three_arg(argument, buf, buf2, buf3);

    argument = one_argument(argument, buf);

    if (!(scroll = get_obj_in_list_vis(ch, buf, ch->carrying))) {
        scroll = ch->equipment[HOLD];
        equipped = TRUE;
        if ((scroll == 0) || !isname(buf, scroll->name)) {
            act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
            return;
        }
    }

    if (scroll->obj_flags.type_flag != ITEM_SCROLL) {
        act("Recite is normally used for scrolls.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    spl = scroll->obj_flags.value[1];
    index = spell_index[spl];

    if (!spl || index == -1) {
        act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
        act("You recite $p which bursts into flame. Nothing happens.",
            FALSE, ch, scroll, 0, TO_CHAR);
        return;
    }

    if (*buf2) {
        if (str_cmp(buf2, "self") == 0) {
            sprintf(buf2, "%s", GET_NAME(ch));
        }

        /*
         * Multiple spells can still crash if they don't have the same
         * spellinfo[i].target This will have to do for now -Lennya
         */
        if (IS_SET(spell_info[index].targets, TAR_CHAR_ROOM)) {
            if ((victim = get_char_room_vis(ch, buf2)) ||
                str_cmp(GET_NAME(ch), buf2) == 0) {
                if (str_cmp(GET_NAME(ch), buf2) == 0) {
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
            sprintf(argument, "%s", buf2);
            target = TAR_OBJ_WORLD;
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_OBJ_EQUIP)) {
            for (i = 0; i < MAX_WEAR && !target_ok; i++) {
                if (ch->equipment[i]
                    && str_cmp(buf2, ch->equipment[i]->name) == 0) {
                    obj = ch->equipment[i];
                    target_ok = TRUE;
                    target = TAR_OBJ_EQUIP;
                }
            }
        }

        if (!target_ok && IS_SET(spell_info[index].targets, TAR_SELF_ONLY)) {
            if (str_cmp(GET_NAME(ch), buf2) == 0) {
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

        if (victim) {
            if (IS_NPC(victim)) {
                if (IS_SET(victim->specials.act, ACT_IMMORTAL)) {
                    send_to_char("You can't recite magic on that!\n\r", ch);
                    return;
                }
            }
        }

        if (target_ok == 0) {
            send_to_char("No such thing around to recite the scroll on.\n\r",
                         ch);
            return;
        }
    } else {
        if (IS_SET(spell_info[index].targets, TAR_SELF_NONO)) {
            send_to_char("You cannot recite this scroll upon yourself.\n\r",
                         ch);
            return;
        } else {
            victim = ch;
            target = TAR_CHAR_ROOM;
        }
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
        if (scroll->obj_flags.value[0] > 0) {
            /*
             * spells for casting
             */
            if (scroll->obj_flags.value[i] >= 1) {
                index = spell_index[scroll->obj_flags.value[i]];
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
                     ((byte) scroll->obj_flags.value[0], ch, "", 
                      SPELL_TYPE_SCROLL, victim, obj));
                }
            }
        } else {
            /*
             * this is a learning scroll
             */
            if (scroll->obj_flags.value[0] < -30) {
                /*
                 * max learning is 30%
                 */
                scroll->obj_flags.value[0] = -30;
            }
            if (scroll->obj_flags.value[i] > 0) {
                /*
                 * positive learning
                 */
                if (ch->skills) {
                    if (ch->skills[scroll->obj_flags.value[i]].learned < 45) {
                        ch->skills[scroll->obj_flags.value[i]].learned +=
                            (-scroll->obj_flags.value[0]);
                    }
                }
            } else {
                /*
                 * negative learning (cursed)
                 */
                if (scroll->obj_flags.value[i] < 0) {
                    /*
                     * 0 = blank
                     */
                    if (ch->skills) {
                        if (ch->skills[-scroll->obj_flags.value[i]].learned >
                            0) {
                            ch->skills[-scroll->obj_flags.value[i]].learned +=
                                scroll->obj_flags.value[0];
                        }
                        ch->skills[-scroll->obj_flags.value[i]].learned =
                            MAX(0,
                                ch->skills[scroll->obj_flags.value[i]].learned);
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

void do_use(struct char_data *ch, char *argument, int cmd)
{
    char            buf[100],
                    buf2[100],
                    buf3[100];
    struct char_data *tmp_char;
    struct obj_data *tmp_object,
                   *stick;

    int             bits,
                    index;
    struct spell_info_type *spellp;

    dlog("in do_use");

    if (A_NOMAGIC(ch)) {
        send_to_char("The arena rules do not allow the use of magic!\n\r",
                     ch);
        return;
    }

    three_arg(argument, buf, buf2, buf3);

    if (ch->equipment[HOLD] == 0 || !isname(buf, ch->equipment[HOLD]->name)) {
        act("You do not hold that item in your hand.", FALSE, ch, 0, 0,
            TO_CHAR);
        return;
    }

#if 0
    if (!IS_PC(ch) && ch->master) {
        act("$n looks confused, and shrugs helplessly", FALSE, ch, 0, 0,
            TO_ROOM);
        return;
    }
#endif

    if (RIDDEN(ch)) {
        return;
    }

    stick = ch->equipment[HOLD];

    if (stick->obj_flags.type_flag == ITEM_STAFF) {
        act("$n taps $p three times on the ground.", TRUE, ch, stick, 0,
            TO_ROOM);
        act("You tap $p three times on the ground.", FALSE, ch, stick, 0,
            TO_CHAR);
        if (stick->obj_flags.value[2] > 0) {
            /*
             * Are there any charges left?
             */
            stick->obj_flags.value[2]--;
            if (check_nomagic(ch, "The magic is blocked by unknown forces.",
                              "The magic is blocked by unknown forces.")) {
                return;
            }

            index = spell_index[stick->obj_flags.value[3]];
            if( index != -1 && spell_info[index].spell_pointer) {
                ((*spell_info[index].spell_pointer)
                 ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF,
                  0, 0));
                WAIT_STATE(ch, PULSE_VIOLENCE);
            }
        } else {
            send_to_char("The staff seems powerless.\n\r", ch);
        }
    } else if (stick->obj_flags.type_flag == ITEM_WAND) {
        if (str_cmp(buf2, "self") == 0) {
            sprintf(buf2, "%s", GET_NAME(ch));
        }

        bits = generic_find(buf2, FIND_CHAR_ROOM | FIND_OBJ_INV |
                                  FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch,
                            &tmp_char, &tmp_object);
        index = spell_index[stick->obj_flags.value[3]];

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

                if (stick->obj_flags.value[2] > 0) {
                    /*
                     * Are there any charges left?
                     */
                    stick->obj_flags.value[2]--;

                    if (check_nomagic(ch,
                                   "The magic is blocked by unknown forces.",
                                   "The magic is blocked by unknown forces.")) {
                        return;
                    }

                    if( spellp->spell_pointer ) {
                        ((*spellp->spell_pointer)
                         ((byte) stick->obj_flags.value[0], ch, "", 
                          SPELL_TYPE_WAND, tmp_char, tmp_object));
                        WAIT_STATE(ch, PULSE_VIOLENCE);
                    }
                } else {
                    send_to_char("The wand seems powerless.\n\r", ch);
                }
            } else if ((find_door(ch, buf2, buf3)) >= 0) {
                /*
                 * For when the arg is a door
                 */
                spellp = &spell_info[index];

                if (stick->obj_flags.value[3] != SPELL_KNOCK) {
                    send_to_char("That spell is useless on doors.\n\r", ch);
                    /*
                     * Spell is not knock, error and return
                     */
                    return;
                }

                if (stick->obj_flags.value[2] > 0) {
                    /*
                     * Are there any charges left?
                     */
                    stick->obj_flags.value[2]--;

                    if (check_nomagic(ch,
                                   "The magic is blocked by unknown forces.",
                                   "The magic is blocked by unknown forces.")) {
                        return;
                    }

                    argument = strcat(strcat(buf2, " "), buf3);

                    if( spellp->spell_pointer ) {
                        ((*spellp->spell_pointer)
                          ((byte) stick->obj_flags.value[0], ch, argument,
                           SPELL_TYPE_WAND, tmp_char, tmp_object));
                        WAIT_STATE(ch, PULSE_VIOLENCE);
                    }
                } else {
                    send_to_char("The wand seems powerless.\n\r", ch);
                }
            } else {
                send_to_char("What should the wand be pointed at?\n\r", ch);
            }
        }
    } else {
        send_to_char("Use is normally only for wands and staves.\n\r", ch);
    }
}

void do_plr_noshout(struct char_data *ch, char *argument, int cmd)
{
    char            buf[128];

    dlog("in do_plr_noshout");

    if (IS_NPC(ch)) {
        return;
    }
    only_argument(argument, buf);

    if (!*buf) {
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
    char            buf[128];

    dlog("in do_plr_no_yell_gossip");

    if (IS_NPC(ch)) {
        return;
    }
    only_argument(argument, buf);

    if (!*buf) {
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
    char            buf[128];

    dlog("in do_plr_noauction");

    if (IS_NPC(ch)) {
        return;
    }
    only_argument(argument, buf);

    if (!*buf) {
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
    char            buf[128];

    dlog("in do_plr_notell");

    if (IS_NPC(ch)) {
        return;
    }
    only_argument(argument, buf);

    if (!*buf) {
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
        for (; isspace(*arg); arg++) {
            /*
             * Empty loop
             */
        }

        if (!*arg) {
            /*
             * print list of current aliases
             */
            if (ch->specials.A_list) {
                for (i = 0; i < 10; i++) {
                    if (ch->specials.A_list->com[i]) {
                        sprintf(buf, "[%d] %s\n\r", i,
                                ch->specials.A_list->com[i]);
                        send_to_char(buf, ch);
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
                if (*arg) {
                    sprintf(buf2, "%s%s", buf, arg);
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
    char            name[112];
    int             check;
    struct char_data *horse;

    dlog("in do_mount");

    if (cmd == 276 || cmd == 278) {
        only_argument(arg, name);

        if (!(horse = get_char_room_vis(ch, name))) {
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
                    qend,
                    diff;
    float           duration;
    struct affected_type af;
    char            buf[MAX_STRING_LENGTH * 2],
                    buffer[MAX_STRING_LENGTH * 2],
                    temp[20];
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

    if (!IS_IMMORTAL(ch)) {
        if (BestMagicClass(ch) == WARRIOR_LEVEL_IND ||
            BestMagicClass(ch) == BARBARIAN_LEVEL_IND) {
            send_to_char("Think you had better stick to fighting...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
            send_to_char("Think you should stick to stealing...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
            send_to_char("Think you should stick to meditating...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == MAGE_LEVEL_IND ||
                   BestMagicClass(ch) == DRUID_LEVEL_IND ||
                   BestMagicClass(ch) == CLERIC_LEVEL_IND) {
            send_to_char("Think you better stick to your way of magic....\n\r",
                         ch);
            return;
        }
    }

    if (affected_by_spell(ch, SKILL_MEMORIZE)) {
        send_to_char("You can only learn one spell at a time.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);

    /*
     * If there is no chars in argument
     */
    if (!(*argument)) {
        send_to_char("Memorize 'spell name'\n\rCurrent spells in memory:\n\r",
                     ch);

        sprintf(buf, "You can memorize one spell %d times, with a total of "
                     "%d spells memorized.\n\r",
                MaxCanMemorize(ch, 0), TotalMaxCanMem(ch));
        send_to_char(buf, ch);
        sprintf(buf, "You currently have %d spells memorized.\n\r",
                TotalMemorized(ch));
        send_to_char(buf, ch);
        send_to_char("Your spellbook holds these spells:\n\r", ch);
        for (i = 0; *spells[i] != '\n'; i++) {
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
        buffer[0] = '\0';
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

    if (*argument != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols "
                     ": '\n\r", ch);
        return;
    }

    for (qend = 1; argument[qend] && argument[qend] != '\''; qend++) {
        argument[qend] = LOWER(argument[qend]);
    }

    if (argument[qend] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols "
                     ": '\n\r", ch);
        return;
    }

    spl = old_search_block(argument, 1, qend - 1, spells, 0);
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
            if (EqWBits(ch, ITEM_ANTI_MAGE))
                max += 10;      /* 20% harder to cast spells */
            break;
        case SORCERER_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_MAGE))
                max += 10;      /* 20% harder to cast spells */
            break;
        case CLERIC_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_CLERIC))
                max += 10;      /* 20% harder to cast spells */
            break;
        case DRUID_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_DRUID))
                max += 10;      /* 20% harder to cast spells */
            break;
        case PALADIN_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_PALADIN))
                max += 10;      /* 20% harder to cast spells */
            break;
        case PSI_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_PSI))
                max += 10;      /* 20% harder to cast spells */
            break;
        case RANGER_LEVEL_IND:
            if (EqWBits(ch, ITEM_ANTI_RANGER))
                max += 10;      /* 20% harder to cast spells */
            break;
        default:
            if (EqWBits(ch, ITEM_ANTI_MAGE))
                max += 10;      /* 20% harder to cast spells */
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
    char            buf[254];

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
                    i->character->invis_level < 51) {
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
            sprintf(buf, "%s started a quest.\n\r", GET_NAME(ch));

            /*
             * quest log this
             */
            qlog(buf);
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
                sprintf(buf, "%s just joined the quest.\n\r", GET_NAME(ch));
                qlog(buf);
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
#if 0
    ch_printf("AFK [%s] ANSI [%s]\n\r" ,(IS_AFFECTED2(ch,AFF2_AFK) ?
        "X" : " "),(IS_SET(ch->player.user_flags, USE_ANSI) ? "X":" ") );

    ch_printf("%15s [%s] %15s [%s]\n\r"
        ,"Wimpy",(IS_SET(ch->specials.act, PLR_WIMPY) ? "X" : " "), "Zone "
        "Sounds",(IS_SET(ch->specials.act, ZONE_SOUNDS) ? "X":" ") );

    ch_printf("%15s [%s] %15s [%s]\n\r"
        ,"Cloaked",(IS_SET(ch->specials.act, CLOAKED) ? "X" : " "),
        "Private",(IS_SET(ch->specials.act, CHAR_PRIVATE) ? "X":" ") );

    send_to_char("Auto settings.",ch);

    ch_printf("%15s [%s] %15s [%s]\n\r" ,"Auto "
        "Assist",(IS_SET(ch->specials.act, PLR_AUTOASSIST) ? "X" : " "),
        "Auto Split",(IS_SET(ch->specials.act, PLR_AUTOSPLIT) ? "X":" ") );

    ch_printf("%15s [%s] %15s [%s]\n\r" ,"Auto "
        "Exit",(IS_SET(ch->specials.act, PLR_AUTOEXIT) ? "X" : " "), "Auto "
        "Loot",(IS_SET(ch->specials.act, PLR_AUTOLOOT) ? "X":" ") );

    ch_printf("%15s [%s] %15s [%s]\n\r" ,"Auto "
            "Gold",(IS_SET(ch->specials.act, PLR_AUTOGOLD) ? "X" : " "),
            "Auto Sac",(IS_SET(ch->specials.act, PLR_AUTOSAC) ? "X":" ") );
#endif
}


void do_auto(struct char_data *ch, char *argument, int cmd)
{
    switch (cmd) {
    case 576:
        if (IS_SET(ch->specials.act, PLR_AUTOASSIST)) {
            do_set_flags(ch, "autoassist disable", 0);
        } else {
            do_set_flags(ch, "autoassist enable", 0);
        }
        break;

    case 577:
        if (IS_SET(ch->specials.act, PLR_AUTOLOOT)) {
            do_set_flags(ch, "autoloot disable", 0);
        } else {
            do_set_flags(ch, "autoloot enable", 0);
        }
        break;

    case 578:
        if (IS_SET(ch->specials.act, PLR_AUTOGOLD)) {
            do_set_flags(ch, "autogold disable", 0);
        } else {
            do_set_flags(ch, "autogold enable", 0);
        }
        break;

    case 579:
        if (IS_SET(ch->specials.act, PLR_AUTOSPLIT)) {
            do_set_flags(ch, "autosplit disable", 0);
        } else {
            do_set_flags(ch, "autosplit enable", 0);
        }
        break;

    case 580:
        if (IS_SET(ch->specials.act, PLR_AUTOSAC)) {
            do_set_flags(ch, "autosac disable", 0);
        } else {
            do_set_flags(ch, "autosac enable", 0);
        }
        break;

    case 581:
        if (IS_SET(ch->specials.act, PLR_AUTOEXIT)) {
            do_set_flags(ch, "autoexits disable", 0);
        } else {
            do_set_flags(ch, "autoexits enable", 0);
        }
        break;

    default:
        do_set_flags(ch, "", 0);

        break;
    }
}

void do_set_flags(struct char_data *ch, char *argument, int cmd)
{
    char            type[255],
                    field[255],
                    name[254];

    dlog("in do_set_flags");

    if (!ch) {
        return;
    }
    half_chop(argument, type, field);

    if (!*type) {
        send_to_char("Set, but set what?!?!? type help set for further "
                     "details on\n\r", ch);
        send_to_char("email, answer, autoexits, groupname, clan, pause, "
                     "autoloot, autogold, autoassist, autosplit, ansi, "
                     "advice, sound, cloak oldcolors\n\r", ch);
        return;
    }

    if (!strcmp("war", type) && (!*field)) {
        send_to_char("Use 'set war enable', REMEMBER ONCE THIS IS SET YOU "
                     "CANNOT REMOVE IT!\n\r", ch);
        send_to_char("Be sure to READ the help on RACE WAR.\n\r", ch);
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

    if (!*field) {
        send_to_char("Set it to what? (Enable,Disable/Off)\n\r", ch);
        return;
    }

    /*
     * get rid of annoying carriage returns -Lennya
     */
    while (field[strlen(field) - 1] == '\r' ||
           field[strlen(field) - 1] == '\n' ||
           field[strlen(field) - 1] == ' ') {
        field[strlen(field) - 1] = '\0';
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
        only_argument(field, name);
        do_group_name(ch, name, 0);
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
        if (*field) {
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
            if (cmd) {
                send_to_char("Set email address to what?\n\r", ch);
            }
        }
    } else if (!strcmp(type, "clan")) {
        if (*field) {
            if (ch->specials.clan) {
                free(ch->specials.clan);
            }
            ch->specials.clan = strdup(field);
            if (cmd == 280) {
                write_char_extra(ch);
                send_to_char("Clan Field set.\n\r", ch);
            }
        } else {
            if (ch->specials.clan) {
                free(ch->specials.clan);
            }
            send_to_char("Set Clan field to None.\n\r", ch);
        }
    } else {
        send_to_char("Unknown type to set.\n\r", ch);
        return;
    }
}

void do_finger(struct char_data *ch, char *argument, int cmd)
{
    char            name[128],
                    buf[254];
    struct char_data *temp = 0;
    struct char_data *finger = 0;
    struct char_data *i;
    struct char_file_u tmp_store;
    int             akills = 0,
                    adeaths = 0;

    dlog("in do_finger");

    argument = one_argument(argument, name);
#if 0
    skip_spaces(&argument);
#endif
    if (!*name) {
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
        sprintf(buf, "        $c0008-=* $c000BAdventurer information $c0008"
                     "*=-\n\r");
        send_to_char(buf, ch);
        sprintf(buf, "$c000BName                  : $c0007%s\n\r",
                finger->player.title);
        send_to_char(buf, ch);
        i = get_char(name);
        /*
         * Last time sited??
         */
        if ((IS_IMMORTAL(finger) && !IS_IMMORTAL(ch)) ||
            GetMaxLevel(finger) > GetMaxLevel(ch)) {
            sprintf(buf, "$c000BLast time sited       : $c0007Unknown\n\r");
        } else if (i && i->desc) {
            /*
             * if there is a name, and a file descriptor
             */
            sprintf(buf, "$c000BLast time sited       : $c0007Currently "
                    "Playing\n\r");
        } else {
            /* NOTE: asctime includes a \n\r at the end of the string */
            sprintf(buf, "$c000BLast time sited       : $c0007%s",
                    asctime(localtime(&tmp_store.last_logon)));
        }
        send_to_char(buf, ch);

        /*
         * Display char email addy
         */
        if (finger->specials.email == NULL) {
            sprintf(buf, "$c000BKnown message drop    : $c0007None\n\r");
        } else {
            sprintf(buf, "$c000BKnown message drop    : $c0007%s\n\r",
                    finger->specials.email);
        }
        send_to_char(buf, ch);

        /*
         * Display clan info
         */
        if (finger->specials.clan == NULL) {
            sprintf(buf, "$c000BClan info             : $c0007None\n\r");
        } else {
            sprintf(buf, "$c000BClan info             : $c0007%s\n\r",
                    CAP(finger->specials.clan));
        }
        send_to_char(buf, ch);

        if (IS_IMMORTAL(ch)) {
            if (finger->specials.hostip == NULL) {
                sprintf(buf, "$c000BHostIP                : $c0007None\n\r");
            } else {
                sprintf(buf, "$c000BHostIP                : $c0007%s\n\r",
                        finger->specials.hostip);
            }
            send_to_char(buf, ch);
        }

        if (finger->specials.rumor == NULL) {
            sprintf(buf, "$c000BRumored info          : $c0007None\n\r");
        } else {
            sprintf(buf, "$c000BRumored info           : $c0007%s\n\r",
                    finger->specials.rumor);
        }
        send_to_char(buf, ch);

        ch_printf(ch, "$c000BArena stats           : $c0007%d kills$c000B/"
                      "$c0007%d deaths\n\r",
                  finger->specials.a_kills, finger->specials.a_deaths);

        /*
         * let's give em a ratio to keep working on
         */
        akills = finger->specials.a_kills;
        adeaths = finger->specials.a_deaths;
        ch_printf(ch, "$c000BArena ratio           : $c0007%3.0f%%\n\r",
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
            while (*ibuf && (isspace(*ibuf))) {
                ibuf++;
            }
            if (*ibuf && (obuf != str)) {
                *(obuf++) = ' ';
            }
            while (*ibuf && (!isspace(*ibuf))) {
                *(obuf++) = *(ibuf++);
            }
        }
        *obuf = '\0';
    }
    return (str);
}

void do_plr_noooc(struct char_data *ch, char *argument, int cmd)
{
    char            buf[128];

    dlog("in do_plr_noooc");

    if (IS_NPC(ch)) {
        return;
    }
    only_argument(argument, buf);

    if (!*buf) {
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
    do_look(ch, "", 0);
    return;
}

void do_promote(struct char_data *ch, char *arg, int cmd)
{
    int             count = 0;
    int             x = 0;
    struct char_data *c[25];
    char            buf[100];
    struct char_data *ch2;
    char            name[100];
    struct follow_type *k;

    if (!arg) {
        send_to_char("Proper usage is:\n\rpromote <name>\n\r", ch);
        return;
    }

    only_argument(arg, name);
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

    /*
     * dont know if this is very efficient but anyway..
     */

    /*
     * tell leader to follow themself
     */
    do_follow(ch2, GET_NAME(ch2), 91);

    /*
     * and group all
     */
    do_group(ch2, "all", 202);
}

void do_behead(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *j = NULL;
    struct obj_data *head = NULL;
    char            temp[256],
                    itemname[80],
                    buf[MAX_STRING_LENGTH];
    int             r_num = 0;

    if (IS_NPC(ch)) {
        return;
    }
    if (!ch->skills) {
        return;
    }
    if (MOUNTED(ch)) {
        send_to_char("Not from this mount you cannot!\n\r", ch);
        return;
    }

    if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        argument = one_argument(argument, itemname);
    }
    if (!*itemname) {
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
    } else {
        /*
         * affect[0] == race of corpse, affect[1] == level of corpse
         */
        if (j->affected[0].modifier != 0 && j->affected[1].modifier != 0) {
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
            /*
             * obj_flags.value[value]
             */
            if (!(Getw_type(ch->equipment[WIELD]) == TYPE_SLASH ||
                  Getw_type(ch->equipment[WIELD]) == TYPE_CLEAVE)) {
                send_to_char("Your weapon isn't really good for that type of "
                             "thing.\n\r", ch);
                return;
            }

            argument = one_argument(j->short_description, temp);

            sprintf(buf, "%s", argument);

            /*
             * load up the head object
             */
            if ((r_num = real_object(SEVERED_HEAD)) >= 0) {
                head = read_object(r_num, REAL);
                if (!head) {
                    Log("ERROR IN BEhead.. make head object");
                    return;

                }
                obj_to_room(head, ch->in_room);
                /*
                 * to room perhaps?
                 */
            }
            /*
             * CHange name of head
             */
            if (head->name) {
                free(head->name);
            }
            sprintf(temp, "head%s", buf);
            head->name = strdup(temp);

            if (head->short_description) {
                free(head->short_description);
            }
            sprintf(temp, "The head%s", buf);
            head->short_description = strdup(temp);

            if (head->description) {
                free(head->description);
            }
            sprintf(temp, "The head%s lies here.", buf);
            head->description = strdup(temp);

            /*
             * make corpse unusable for another behead
             */
            j->affected[1].modifier = 0;

            if (j->description) {
                argument = one_argument(j->description, temp);

                sprintf(buf, "The beheaded %s%s", temp, argument);
                free(j->description);
            }
            j->description = strdup(buf);

            sprintf(buf, "You behead %s.\n\r", j->short_description);
            send_to_char(buf, ch);

            sprintf(buf, "%s beheads %s.", GET_NAME(ch),
                    j->short_description);
            act(buf, TRUE, ch, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE * 1);
        } else {
            send_to_char("Sorry, the corpse is too mangled up to behead.\n\r",
                         ch);
        }
    }
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

    ch_printf(ch, "Deadly: %s   %d.\n\rRichest: %s   %d\n\r", deadlyname,
              deadly, richestname, richest);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
