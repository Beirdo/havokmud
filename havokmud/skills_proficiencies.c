/*
 * HavokMUD - proficiencies
 * REQUIRED BY - all classes
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


/*
 * track/hunt start
 */
struct hunting_data {
    char    	*name;
    struct char_data **victim;
};

int 	is_target_room_p(int room, void *tgt_room);
int	named_object_on_ground(int room, void *c_data);

int named_mobile_in_room(int room, struct hunting_data *c_data)
{
    struct char_data *scan;

    for (scan = real_roomp(room)->people; scan; scan = scan->next_in_room) {
        if (isname(c_data->name, scan->player.name)) {
            *(c_data->victim) = scan;
            return 1;
        }
    }
    return 0;
}

void do_track(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                    buf[256],
                    found = FALSE;
    int             dist,
                    code;
    struct hunting_data huntd;
    struct char_data *scan;
    extern struct char_data *character_list;

#ifndef USE_TRACK
    send_to_char("Sorry, tracking is disabled. Try again after reboot.\n\r",
                 ch);
    return;
#endif

    argument = get_argument(argument, &name);
    if( !name ) {
        send_to_char("You are unable to find traces of one.\n\r", ch);
        return;
    }


    found = FALSE;
    for (scan = character_list; scan; scan = scan->next) {
        if (isname(name, scan->player.name)) {
            found = TRUE;
        }
    }

    if (!found) {
        send_to_char("You are unable to find traces of one.\n\r", ch);
        return;
    }

    if (!ch->skills) {
        dist = 10;
    } else {
        dist = ch->skills[SKILL_HUNT].learned;
    }

    if (IS_SET(ch->player.class, CLASS_RANGER)) {
        dist *= 3;
    }

    switch (GET_RACE(ch)) {
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_FOREST_GNOME:
    case RACE_AVARIEL:
        dist *= 2;
        break;
    case RACE_DEVIL:
    case RACE_DEMON:
        dist = MAX_ROOMS;
        break;
    default:
        break;
    }

    if (GetMaxLevel(ch) >= IMMORTAL) {
        dist = MAX_ROOMS;
    }
    if (affected_by_spell(ch, SPELL_MINOR_TRACK)) {
        dist = GetMaxLevel(ch) * 50;
    } else if (affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
        dist = GetMaxLevel(ch) * 100;
    }

    if (dist == 0) {
        return;
    }
    ch->hunt_dist = dist;

    ch->specials.hunting = 0;
    huntd.name = name;
    huntd.victim = &ch->specials.hunting;

    if (GetMaxLevel(ch) < MIN_GLOB_TRACK_LEV ||
        affected_by_spell(ch, SPELL_MINOR_TRACK) ||
        affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
        code = find_path(ch->in_room, named_mobile_in_room, &huntd, -dist, 1);
    } else {
        code = find_path(ch->in_room, named_mobile_in_room, &huntd, -dist, 0);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 1);

    if (code == -1) {
        send_to_char("You are unable to find traces of one.\n\r", ch);
    } else if (IS_LIGHT(ch->in_room) || IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        SET_BIT(ch->specials.act, PLR_HUNTING);
        sprintf(buf, "You see traces of your quarry to the %s\n\r", dirs[code]);
        send_to_char(buf, ch);
    } else {
        ch->specials.hunting = 0;
        send_to_char("It's too dark in here to track...\n\r", ch);
    }
}

int track(struct char_data *ch, struct char_data *vict)
{
    char            buf[256];
    int             code;

    if ((!ch) || (!vict)) {
        return (-1);
    }
    if (GetMaxLevel(ch) < MIN_GLOB_TRACK_LEV ||
        affected_by_spell(ch, SPELL_MINOR_TRACK) ||
        affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
        code = choose_exit_in_zone(ch->in_room, vict->in_room, ch->hunt_dist);
    } else {
        code = choose_exit_global(ch->in_room, vict->in_room, ch->hunt_dist);
    }

    if ((!ch) || (!vict)) {
        return (-1);
    }
    if (ch->in_room == vict->in_room) {
        send_to_char("##You have found your target!\n\r", ch);
        /* 
         * false to continue the hunt 
         */
        return (FALSE);
    }
    if (code == -1) {
        send_to_char("##You have lost the trail.\n\r", ch);
        return (FALSE);
    } else {
        sprintf(buf, "##You see a faint trail to the %s\n\r", dirs[code]);
        send_to_char(buf, ch);
        return (TRUE);
    }
}

int dir_track(struct char_data *ch, struct char_data *vict)
{
    char            buf[256];
    int             code;

    if ((!ch) || (!vict)) {
        return (-1);
    }
    if (GetMaxLevel(ch) >= MIN_GLOB_TRACK_LEV ||
        affected_by_spell(ch, SPELL_MINOR_TRACK) ||
        affected_by_spell(ch, SPELL_MAJOR_TRACK)) {
        code = choose_exit_global(ch->in_room, vict->in_room, ch->hunt_dist);
    } else {
        code = choose_exit_in_zone(ch->in_room, vict->in_room, ch->hunt_dist);
    }

    if ((!ch) || (!vict)) {
        return (-1);
    }
    if (code == -1) {
        if (ch->in_room == vict->in_room) {
            send_to_char("##You have found your target!\n\r", ch);
        } else {
            send_to_char("##You have lost the trail.\n\r", ch);
        }
        return (-1);
    } else {
        sprintf(buf, "##You see a faint trail to the %s\n\r", dirs[code]);
        send_to_char(buf, ch);
        return (code);
    }
}

/* 
 * Perform breadth first search on rooms from start (in_room)
 * until end (tgt_room) is reached. Then return the correct   
 * direction to take from start to reach end.               
 */
/*
 * thoth@manatee.cis.ufl.edu if dvar<0 then search THROUGH closed but not
 * locked doors, for mobiles that know how to open doors. 
 */

#define IS_DIR    (real_roomp(q_head->room_nr)->dir_option[i])
#define GO_OK  (!IS_SET(IS_DIR->exit_info,EX_CLOSED) \
                 && (IS_DIR->to_room != NOWHERE))
#define GO_OK_SMARTER  (!IS_SET(IS_DIR->exit_info,EX_LOCKED) \
                 && (IS_DIR->to_room != NOWHERE))

void donothing(void)
{
    return;
}

int find_path(int in_room, int (*predicate) (), void *c_data,
              int depth, int in_zone)
{
    struct room_q  *tmp_q,
                   *q_head,
                   *q_tail;
    struct hash_header x_room;
    int             i,
                    tmp_room,
                    count = 0,
                    thru_doors;
    struct room_data *herep,
                   *therep;
    struct room_data *startp;
    struct room_direction_data *exitp;
    struct room_data *rp;

    /*
     * If start = destination we are done 
     */
    if ((predicate)(in_room, c_data)) {
        return -1;
    }

    /*
     * so you cannot track mobs in no_summon rooms 
     */
    if (in_room) {
        rp = real_roomp(in_room);
        if (rp && IS_SET(rp->room_flags, NO_SUM)) {
            return (-1);
        }
    }

    if (depth < 0) {
        thru_doors = TRUE;
        depth = -depth;
    } else {
        thru_doors = FALSE;
    }

    startp = real_roomp(in_room);

    init_hash_table(&x_room, sizeof(int), 2048);
    hash_enter(&x_room, in_room, (void *) -1);

    /*
     * initialize queue 
     */
    q_head = (struct room_q *) malloc(sizeof(struct room_q));
    q_tail = q_head;
    q_tail->room_nr = in_room;
    q_tail->next_q = 0;

    while (q_head) {
        herep = real_roomp(q_head->room_nr);
        /*
         * for each room test all directions 
         */
        if (herep->zone == startp->zone || !in_zone) {
            /*
             * only look in this zone.. saves cpu time.  makes world safer 
             * for players 
             */
            for (i = 0; i <= 5; i++) {
                exitp = herep->dir_option[i];
                if (exit_ok(exitp, &therep) && 
                    (thru_doors ? GO_OK_SMARTER : GO_OK)) {
                    /*
                     * next room 
                     */
                    tmp_room = herep->dir_option[i]->to_room;
                    if (!((predicate)(tmp_room, c_data))) {
                        /*
                         * shall we add room to queue ? 
                         */
                        /*
                         * count determines total breadth and depth 
                         */
                        if (!hash_find(&x_room, tmp_room) && count < depth &&
                            !IS_SET(RM_FLAGS(tmp_room), DEATH)) {
                            count++;
                            /*
                             * mark room as visted and put on queue 
                             */
                            tmp_q = (struct room_q *)
                                        malloc(sizeof(struct room_q));
                            tmp_q->room_nr = tmp_room;
                            tmp_q->next_q = 0;
                            q_tail->next_q = tmp_q;
                            q_tail = tmp_q;

                            /*
                             * ancestor for first layer is the direction 
                             */
                            hash_enter(&x_room, tmp_room,
                              ((int)hash_find(&x_room, q_head->room_nr) == -1) ?
                               (void *) (i + 1) : 
                               hash_find(&x_room, q_head->room_nr));
                        }
                    } else {
                        /*
                         * have reached our goal so free queue 
                         */
                        tmp_room = q_head->room_nr;
                        for (; q_head; q_head = tmp_q) {
                            tmp_q = q_head->next_q;
                            if (q_head) {
                                free(q_head);
                            }
                        }
                        /*
                         * return direction if first layer 
                         */
                        if ((int) hash_find(&x_room, tmp_room) == -1) {
                            if (x_room.buckets) {
                                /* 
                                 * junk left over from a previous track 
                                 */
                                destroy_hash_table(&x_room, donothing);
                            }
                            return (i);
                        } else {
                            /* 
                             * else return the ancestor 
                             */
                            i = (int) hash_find(&x_room, tmp_room);
                            if (x_room.buckets) {
                                /* 
                                 * junk left over from a previous track 
                                 */
                                destroy_hash_table(&x_room, donothing);
                            }
                            return (-1 + i);
                        }
                    }
                }
            }
        }

        /*
         * free queue head and point to next entry 
         */
        tmp_q = q_head->next_q;
        if (q_head) {
            free(q_head);
        }
        q_head = tmp_q;
    }

    /*
     * couldn't find path 
     */
    if (x_room.buckets) {
        /* 
         * junk left over from a previous track 
         */
        destroy_hash_table(&x_room, donothing);
    }
    return (-1);

}

int choose_exit_global(int in_room, int tgt_room, int depth)
{
    return find_path(in_room, is_target_room_p, (void *) tgt_room, depth, 0);
}

int choose_exit_in_zone(int in_room, int tgt_room, int depth)
{
    return find_path(in_room, is_target_room_p, (void *) tgt_room, depth, 1);
}

void go_direction(struct char_data *ch, int dir)
{
    if (ch->specials.fighting) {
        return;
    }
    if (!IS_SET(EXIT(ch, dir)->exit_info, EX_CLOSED)) {
        do_move(ch, "", dir + 1);
    } else if (IsHumanoid(ch) && !IS_SET(EXIT(ch, dir)->exit_info, EX_LOCKED) &&
               !IS_SET(EXIT(ch, dir)->exit_info, EX_SECRET)) {
        open_door(ch, dir);
    }
}
/* track/hunt end */

void do_spot(struct char_data *ch, char *argument, int cmd)
{
    static char    *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "\n"
    };
    char           *dir_desc[] = {
        "to the north",
        "to the east",
        "to the south",
        "to the west",
        "upwards",
        "downwards"
    };
    char           *rng_desc[] = {
        "right here",
        "immediately",
        "nearby",
        "a ways",
        "a ways",
        "far",
        "far",
        "very far",
        "very far"
    };
    char            buf[MAX_STRING_LENGTH],
                    buf2[MAX_STRING_LENGTH];
    char           *arg1;
    int             sd,
                    smin,
                    smax,
                    swt,
                    i,
                    max_range = 6,
                    range,
                    rm,
                    nfnd;
    struct char_data *spud;

    dlog("in do_spot");

    /*
     * Check mortals spot skill, and give THEM a max scan of 2 rooms.
     */

    if (!ch->skills) {
        send_to_char("You do not have skills!\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch)) {
        if (!ch->skills[SKILL_SPOT].learned) {
            send_to_char("You have not been trained to spot.\n\r", ch);
            return;
        }

        if (dice(1, 101) > ch->skills[SKILL_SPOT].learned) {
            send_to_char("Absolutely no-one anywhere.\n\r", ch);
            WAIT_STATE(ch, 2);
            return;
        }
        /*
         * failed
         */
        max_range = 2;          /* morts can only spot two rooms away */
    }
    /*
     * was mortal
     */
    argument = get_argument(argument, &arg1);

    if( !arg1 || (sd = search_block(arg1, keywords, FALSE)) == -1) {
        smin = 0;
        smax = 5;
        swt = 3;
        sprintf(buf, "$n peers intently all around.");
        sprintf(buf2, "You peer intently all around, and see :\n\r");
    } else {
        smin = sd;
        smax = sd;
        swt = 1;
        sprintf(buf, "$n peers intently %s.", dir_desc[sd]);
        sprintf(buf2, "You peer intently %s, and see :\n\r", dir_desc[sd]);
    }

    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf2, ch);
    nfnd = 0;
    /*
     * Check in room first
     */
    for (spud = real_roomp(ch->in_room)->people; spud; 
         spud = spud->next_in_room) {
        if (CAN_SEE(ch, spud) && !IS_AFFECTED(spud, AFF_HIDE) && spud != ch) {
            if (IS_NPC(spud)) {
                sprintf(buf, "%30s : right here\n\r", spud->player.short_descr);
            } else {
                sprintf(buf, "%30s : right here\n\r", GET_NAME(spud));
            }
            send_to_char(buf, ch);
            nfnd++;
        }
    }

    for (i = smin; i <= smax; i++) {
        rm = ch->in_room;
        range = 0;
        while (range < max_range) {
            range++;
            if (clearpath(ch, rm, i)) {
                rm = real_roomp(rm)->dir_option[i]->to_room;
                for (spud = real_roomp(rm)->people; spud;
                     spud = spud->next_in_room) {
                    if (CAN_SEE(ch, spud) && !IS_AFFECTED(spud, AFF_HIDE)) {
                        if (IS_NPC(spud)) {
                            sprintf(buf, "%30s : %s %s\n\r",
                                    spud->player.short_descr,
                                    rng_desc[range], dir_desc[i]);
                        } else {
                            sprintf(buf, "%30s : %s %s\n\r",
                                    GET_NAME(spud), rng_desc[range],
                                    dir_desc[i]);
                        }
                        send_to_char(buf, ch);
                        nfnd++;
                    }
                }
            } else {
                range = max_range + 1;
            }
        }
    }

    if (nfnd == 0) {
        send_to_char("Absolutely no-one anywhere.\n\r", ch);
    }

    WAIT_STATE(ch, swt);
}

int SpyCheck(struct char_data *ch)
{
    if (!ch->skills) {
        return (FALSE);
    }
    if (number(1, 101) > ch->skills[SKILL_SPY].learned) {
        return (FALSE);
    }
    return (TRUE);
}

void do_spy(struct char_data *ch, char *arg, int cmd)
{
    struct affected_type af;
    byte            percent;

    if (!ch->skills[SKILL_SPY].learned){
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    send_to_char("Ok, you'll try to act like a tracker so you can scout "
                 "ahead.\n\r", ch);

    if (IS_AFFECTED(ch, AFF_SCRYING)) {
        /*
         * kinda pointless if they don't need to...
         */
        return;
    }

    if (affected_by_spell(ch, SKILL_SPY)) {
        send_to_char("You're already acting like a hunter.\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);

    if (!ch->skills) {
        return;
    }
    if (percent > ch->skills[SKILL_SPY].learned) {
        send_to_char("You fail to enhance your vision.", ch);
        LearnFromMistake(ch, SKILL_SPY, 0, 95);
        

        af.type = SKILL_SPY;
        af.duration = (ch->skills[SKILL_SPY].learned / 10) + 1;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
        return;
    }

    af.type = SKILL_SPY;
    af.duration = (ch->skills[SKILL_SPY].learned / 10) + 1;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SCRYING;
    affect_to_char(ch, &af);
}
/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

