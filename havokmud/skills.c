/*
 ***  DaleMUD
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "protos.h"
#include "externs.h"


struct hunting_data {
    char           *name;
    struct char_data **victim;
};

#define TAN_SHIELD   67
#define TAN_JACKET   68
#define TAN_BOOTS    69
#define TAN_GLOVES   70
#define TAN_LEGGINGS 71
#define TAN_SLEEVES  72
#define TAN_HELMET   73
#define TAN_BAG      14

#define FOUND_FOOD 21
#define FOUND_WATER 13

/*************************************/
/*
 * predicates for find_path function 
 */

int             is_target_room_p(int room, void *tgt_room);
int             named_object_on_ground(int room, void *c_data);

/*
 * predicates for find_path function 
 */
/*************************************/

/*
 **  Disarm:
 */

void do_disarm(struct char_data *ch, char *argument, int cmd)
{
    char           *name;
    int             percent;
    struct char_data *victim;
    struct obj_data *w,
                   *trap;

    if (!ch->skills) {
        return;
    }

    if (check_peaceful(ch, "You feel too peaceful to contemplate "
                           "violence.\n\r")) {
        return;
    }

    if (!IS_PC(ch) && cmd) {
        return;
    }

    /*
     *   get victim
     */
    argument = get_argument(argument, &name);
    if (!name) {
        send_to_char("Disarm who/what?\n\r", ch);
        return;
    }

    if(!(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            if (!ch->skills) {
                send_to_char("You do not have skills!\n\r", ch);
                return;
            }

            if (!ch->skills[SKILL_REMOVE_TRAP].learned) {
                send_to_char("Disarm who?\n\r", ch);
                return;
            } else {
                if (MOUNTED(ch)) {
                    send_to_char("Yeah... right... while mounted\n\r", ch);
                    return;
                }

                if (!(trap = get_obj_in_list_vis(ch, name, 
                                         real_roomp(ch->in_room)->contents)) &&
                    !(trap = get_obj_in_list_vis(ch, name, ch->carrying))) {
                    send_to_char("Disarm what?\n\r", ch);
                    return;
                }

                if (trap) {
                    remove_trap(ch, trap);
                    return;
                }
            }
        }
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    if (victim != ch->specials.fighting) {
        send_to_char("but you aren't fighting them!\n\r", ch);
        return;
    }

    if (ch->attackers > 3) {
        send_to_char("There is no room to disarm!\n\r", ch);
        return;
    }

    if (!HasClass(ch, CLASS_WARRIOR | CLASS_MONK | CLASS_BARBARIAN | 
                      CLASS_RANGER | CLASS_PALADIN | CLASS_THIEF)) {
        send_to_char("You're no warrior!\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);   

    percent -= dex_app[(int)GET_DEX(ch)].reaction * 10;
    percent += dex_app[(int)GET_DEX(victim)].reaction * 10;
    if (!ch->equipment[WIELD] && !HasClass(ch, CLASS_MONK)) {
        percent += 50;
    }

    percent += GetMaxLevel(victim);
    if (HasClass(victim, CLASS_MONK)) {
        percent += GetMaxLevel(victim);
    }
    if (HasClass(ch, CLASS_MONK)) {
        percent -= GetMaxLevel(ch);
    } else {
        percent -= GetMaxLevel(ch) >> 1;
    }

    if (percent > ch->skills[SKILL_DISARM].learned) {
        act("You try to disarm $N, but fail miserably.",
            TRUE, ch, 0, victim, TO_CHAR);
        act("$n does a nifty fighting move, but then falls on $s butt.",
            TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POSITION_SITTING;
        if (IS_NPC(victim) && GET_POS(victim) > POSITION_SLEEPING &&
            !victim->specials.fighting) {
            set_fighting(victim, ch);
        }
        LearnFromMistake(ch, SKILL_DISARM, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
        if (victim->equipment[WIELD]) {
            w = unequip_char(victim, WIELD);
            act("$n makes an impressive fighting move.",
                TRUE, ch, 0, 0, TO_ROOM);
            act("You send $p flying from $N's grasp.", TRUE, ch, w, victim,
                TO_CHAR);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
            act("$p flies from your grasp.", TRUE, ch, w, victim, TO_VICT);
            /*
             * send the object to a nearby room, instead 
             */
            obj_to_room(w, victim->in_room);
        } else {
            act("You try to disarm $N, but $E doesn't have a weapon.",
                TRUE, ch, 0, victim, TO_CHAR);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for"
                         " using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
            act("$n makes an impressive fighting move, but does little more.",
                TRUE, ch, 0, 0, TO_ROOM);
        }
        if (IS_NPC(victim) && GET_POS(victim) > POSITION_SLEEPING &&
            !victim->specials.fighting) {
            set_fighting(victim, ch);
        }
        WAIT_STATE(victim, PULSE_VIOLENCE * 2);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
}

/*
 **   Track:
 */

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
#if 1
    struct hash_header x_room;
#else
    struct nodes    x_room[MAX_ROOMS];
#endif
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
#if 0
    if (top_of_world > MAX_ROOMS) {
        Log("TRACK Is disabled, too many rooms.\n\rContact Loki soon.\n\r");
        return -1;
    }
#endif

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

void slam_into_wall(struct char_data *ch,
                    struct room_direction_data *exitp)
{
    char            doorname[128];
    char            buf[256];

    if (exitp->keyword && *exitp->keyword) {
        if (strcmp(fname(exitp->keyword), "secret") == 0 ||
            IS_SET(exitp->exit_info, EX_SECRET)) {
            strcpy(doorname, "wall");
        } else {
            strcpy(doorname, fname(exitp->keyword));
        }
    } else {
        strcpy(doorname, "barrier");
    }

    sprintf(buf, "You slam against the %s with no effect\n\r", doorname);
    send_to_char(buf, ch);
    send_to_char("OUCH!  That REALLY Hurt!\n\r", ch);
    sprintf(buf, "$n crashes against the %s with no effect\n\r", doorname);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    GET_HIT(ch) -= number(1, 10) * 2;
    if (GET_HIT(ch) < 0) {
        GET_HIT(ch) = 0;
    }
    GET_POS(ch) = POSITION_STUNNED;
}

/*
 * skill to allow fighters to break down doors 
 */
void do_doorbash(struct char_data *ch, char *arg, int cmd)
{
    extern char    *dirs[];
    int             dir;
    int             ok;
    struct room_direction_data *exitp;
    int             was_in,
                    roll;
    char            buf[256],
                   *type,
                   *direction;

    if (!ch->skills) {
        return;
    }
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
    arg = get_argument(arg, &type);
    arg = get_argument(arg, &direction);

    if ((dir = find_door(ch, type, direction)) >= 0) {
        ok = TRUE;
    } else {
        act("$n looks around, bewildered.", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    if (!ok) {
        send_to_char("Hmm, you shouldn't have gotten this far\n\r", ch);
        return;
    }

    exitp = EXIT(ch, dir);
    if (!exitp) {
        send_to_char("you shouldn't have gotten here.\n\r", ch);
        return;
    }

    if (dir == UP) {
#if 1
        /*
         * disabledfor now 
         */
        send_to_char("Are you crazy, you can't door bash UPWARDS!\n\r", ch);
        return;
    }
#else
        if (real_roomp(exitp->to_room)->sector_type == SECT_AIR &&
            !IS_AFFECTED(ch, AFF_FLYING)) {
            send_to_char("You have no way of getting there!\n\r", ch);
            return;
        }
    }
#endif

    sprintf(buf, "$n charges %swards", dirs[dir]);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "You charge %swards\n\r", dirs[dir]);
    send_to_char(buf, ch);

    if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
        was_in = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, exitp->to_room);
        do_look(ch, NULL, 0);

        DisplayMove(ch, dir, was_in, 1);
        if (check_falling(ch)) {
            return;
        }

        if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
            !IS_IMMORTAL(ch)) {
            NailThisSucker(ch);
            return;
        } else {
            WAIT_STATE(ch, PULSE_VIOLENCE * 3);
            GET_MOVE(ch) -= 10;
        }

        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        GET_MOVE(ch) -= 10;
        return;
    }

    GET_MOVE(ch) -= 10;

    if (IS_SET(exitp->exit_info, EX_LOCKED) &&
        IS_SET(exitp->exit_info, EX_PICKPROOF)) {
        slam_into_wall(ch, exitp);
        return;
    }

    /*
     * now we've checked for failures, time to check for success; 
     */
    if (ch->skills) {
        if (ch->skills[SKILL_DOORBASH].learned) {
            roll = number(1, 100);
            if (roll > ch->skills[SKILL_DOORBASH].learned) {
                slam_into_wall(ch, exitp);
                LearnFromMistake(ch, SKILL_DOORBASH, 0, 95);
            } else {
                /*
                 * unlock and open the door 
                 */
                sprintf(buf, "$n slams into the %s, and it bursts open!",
                        fname(exitp->keyword));
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                send_to_char("$c000BYou receive $c000W100 $c000Bexperience for"
                             " using your combat abilities.$c0007\n\r", ch);
                gain_exp(ch, 100);
                sprintf(buf, "You slam into the %s, and it bursts open!\n\r",
                        fname(exitp->keyword));
                send_to_char(buf, ch);

                raw_unlock_door(ch, exitp, dir);
                raw_open_door(ch, dir);
                GET_HIT(ch) -= number(1, 5);
                /*
                 * Now a dex check to keep from flying into the next room 
                 */
                roll = number(1, 20);
                if (roll > GET_DEX(ch)) {
                    was_in = ch->in_room;

                    char_from_room(ch);
                    char_to_room(ch, exitp->to_room);
                    do_look(ch, NULL, 0);

                    DisplayMove(ch, dir, was_in, 1);
                    if (check_falling(ch)) {
                        return;
                    }

                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                        !IS_IMMORTAL(ch)) {
                        NailThisSucker(ch);
                        return;
                    }

                    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
                    GET_MOVE(ch) -= 10;
                    return;
                } else {
                    WAIT_STATE(ch, PULSE_VIOLENCE * 1);
                    GET_MOVE(ch) -= 5;
                    return;
                }
            }
        } else {
            send_to_char("You just don't know the nuances of "
                         "door-bashing.\n\r", ch);
            slam_into_wall(ch, exitp);
        }
    } else {
        send_to_char("You're just a goofy mob.\n\r", ch);
    }
}

/*
 * skill to allow anyone to move through rivers and underwater 
 */

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




void do_feign_death(struct char_data *ch, char *arg, int cmd)
{
    struct room_data *rp;
    struct char_data *t;

    if (!ch->skills) {
        return;
    }
    if (!ch->specials.fighting) {
        send_to_char("But you are not fighting anything...\n\r", ch);
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_MONK) && !HasClass(ch, CLASS_NECROMANCER)) {
        send_to_char("You're not a monk!\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_FEIGN_DEATH].learned)) {
        send_to_char("Derr.. what's that?\n\r", ch);
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return;
    }
    send_to_char("You try to fake your own demise\n\r", ch);

    death_cry(ch);
    act("$n is dead! R.I.P.", FALSE, ch, 0, 0, TO_ROOM);

    if (number(1, 101) < ch->skills[SKILL_FEIGN_DEATH].learned) {
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your combat abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        stop_fighting(ch);

        for (t = rp->people; t; t = t->next_in_room) {
            if (t->specials.fighting == ch) {
                stop_fighting(t);
                ZeroHatred(t, ch);

                if (number(1, 101) < 
                    ch->skills[SKILL_FEIGN_DEATH].learned / 2) {
                    SET_BIT(ch->specials.affected_by, AFF_HIDE);
                }
                GET_POS(ch) = POSITION_SLEEPING;
            }
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    } else {
        GET_POS(ch) = POSITION_SLEEPING;
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        LearnFromMistake(ch, SKILL_FEIGN_DEATH, 0, 95);
    }
}

void do_first_aid(struct char_data *ch, char *arg, int cmd)
{
    struct affected_type af;
    int             exp_level = 0;

    if (!ch->skills) {
        return;
    }
    if (affected_by_spell(ch, SKILL_FIRST_AID)) {
        send_to_char("You can only do this once per day.\n\r", ch);
        return;
    }

    if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        exp_level = GetMaxLevel(ch);
    }
    if (number(1, 101) < ch->skills[SKILL_FIRST_AID].learned) {
        send_to_char("You render first aid unto yourself.\n\r", ch);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using"
                     " your combat abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        GET_HIT(ch) += ((number(3, 15)) + exp_level);
        if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
            GET_HIT(ch) = GET_MAX_HIT(ch);
        }
        af.duration = 24;
    } else {
        af.duration = 6;
        send_to_char("You attempt to render first aid unto yourself, but "
                     "fail.\n\r", ch);
        LearnFromMistake(ch, SKILL_FIRST_AID, 0, 95);
    }

    af.type = SKILL_FIRST_AID;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void do_disguise(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    struct char_data *k;

    if (!ch->skills) {
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

/*
 * Skill for climbing walls and the like -DM 
 */


void do_tan(struct char_data *ch, char *arg, int cmd)
{
    struct obj_data *j = 0;
    struct obj_data *hide;
    char           *itemname,
                   *itemtype,
                    hidetype[80],
                    buf[MAX_STRING_LENGTH];
    int             percent = 0;
    int             acapply = 0;
    int             acbonus = 0;
    int             lev = 0;
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

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_BARBARIAN | CLASS_WARRIOR | CLASS_RANGER)) {
        send_to_char("What do you think you are, A tanner?\n\r", ch);
        return;
    }

    arg = get_argument(arg, &itemname);
    arg = get_argument(arg, &itemtype);

    if (!itemname) {
        send_to_char("Tan what?\n\r", ch);
        return;
    }

    if (!itemtype) {
        send_to_char("I see that, but what do you wanna make?\n\r", ch);
        return;
    }

    if (!(j = get_obj_in_list_vis(ch, itemname, 
                                  real_roomp(ch->in_room)->contents))) {
        send_to_char("Where did that carcass go?\n\r", ch);
        return;
    }

    if ((strcmp(itemtype, "shield")) &&
        (strcmp(itemtype, "jacket")) &&
        (strcmp(itemtype, "boots")) &&
        (strcmp(itemtype, "gloves")) &&
        (strcmp(itemtype, "leggings")) &&
        (strcmp(itemtype, "sleeves")) &&
        (strcmp(itemtype, "helmet")) &&
        (strcmp(itemtype, "bag"))) {
        send_to_char("You can't make that.\n\rTry shield, jacket, boots, "
                     "gloves, leggings, sleeves, helmet, or bag.\n\r", ch);
        return;
    }

    /*
     * affect[0] == race of corpse, affect[1] == level of corpse 
     */
    if (j->affected[0].modifier != 0 && j->affected[1].modifier != 0) {
        /* 
         * 101% is a complete failure 
         */
        percent = number(1, 101);

        if (!ch->skills || !ch->skills[SKILL_TAN].learned ||
            GET_POS(ch) <=  POSITION_SLEEPING) {
            return;
        }

        if (percent > ch->skills[SKILL_TAN].learned) {
            /* 
             * make corpse unusable for another tan 
             */
            j->affected[1].modifier = 0;

            sprintf(buf, "You hack at %s but manage to only destroy the "
                         "hide.\n\r", j->short_description);
            send_to_char(buf, ch);

            sprintf(buf, "%s tries to skins %s for it's hide, but destroys it.",
                    GET_NAME(ch), j->short_description);
            act(buf, TRUE, ch, 0, 0, TO_ROOM);
            LearnFromMistake(ch, SKILL_TAN, 0, 95);
            WAIT_STATE(ch, PULSE_VIOLENCE * 3);
            return;
        }

        /*
         * item not a corpse if v3 = 0 
         */
        if (!j->obj_flags.value[3]) {
            send_to_char("Sorry, this is not a carcass.\n\r", ch);
            return;
        }

        lev = j->affected[1].modifier;

        /*
         * We could use a array using the race as a pointer 
         * but this way makes it more visable and easier to
         * handle however it is ugly. 
         */
        switch (j->affected[0].modifier) {
        case RACE_HUMAN:
            sprintf(hidetype, "human leather");
            lev = (int) lev / 2;
            break;
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_SEA_ELF:
        case RACE_MOON_ELF:
            sprintf(hidetype, "elf hide");
            lev = (int) lev / 2;
            break;
        case RACE_AVARIEL:
            sprintf(hidetype, "feathery elf hide");
            lev = (int) lev / 2;
            break;
        case RACE_DARK_DWARF:
        case RACE_DWARF:
            sprintf(hidetype, "dwarf hide");
            lev = (int) lev / 2;
            break;
        case RACE_HALFLING:
            sprintf(hidetype, "halfing hide");
            lev = (int) lev / 2;
            break;
        case RACE_DEEP_GNOME:
        case RACE_FOREST_GNOME:
        case RACE_ROCK_GNOME:
            sprintf(hidetype, "gnome hide");
            lev = (int) lev / 2;
            break;
        case RACE_REPTILE:
            sprintf(hidetype, "reptile hide");
            break;
        case RACE_DRAGON:
            sprintf(hidetype, "dragon hide");
            break;
        case RACE_UNDEAD:
        case RACE_UNDEAD_VAMPIRE:
        case RACE_UNDEAD_LICH:
        case RACE_UNDEAD_WIGHT:
        case RACE_UNDEAD_GHAST:
        case RACE_UNDEAD_SPECTRE:
        case RACE_UNDEAD_ZOMBIE:
        case RACE_UNDEAD_SKELETON:
        case RACE_UNDEAD_GHOUL:
            sprintf(hidetype, "rotting hide");
            lev = (int) lev / 2;
            break;
        case RACE_ORC:
            sprintf(hidetype, "orc hide");
            lev = (int) lev / 2;
            break;
        case RACE_INSECT:
            sprintf(hidetype, "insectiod hide");
            break;
        case RACE_ARACHNID:
            sprintf(hidetype, "hairy leather");
            lev = (int) lev / 2;
            break;
        case RACE_DINOSAUR:
            sprintf(hidetype, "thick leather");
            break;
        case RACE_FISH:
            sprintf(hidetype, "fishy hide");
            break;
        case RACE_BIRD:
            sprintf(hidetype, "feathery hide");
            lev = (int) lev / 2;
            break;
        case RACE_GIANT:
            sprintf(hidetype, "giantish hide");
            break;
        case RACE_SLIME:
            sprintf(hidetype, "leather");
            lev = (int) lev / 2;
            break;
        case RACE_DEMON:
            sprintf(hidetype, "demon hide");
            break;
        case RACE_SNAKE:
            sprintf(hidetype, "snake hide");
            break;
        case RACE_TREE:
            sprintf(hidetype, "bark hide");
            break;
        case RACE_VEGGIE:
        case RACE_VEGMAN:
            sprintf(hidetype, "green hide");
            break;
        case RACE_DEVIL:
            sprintf(hidetype, "devil hide");
            break;
        case RACE_GHOST:
            sprintf(hidetype, "ghostly hide");
            break;
        case RACE_GOBLIN:
            sprintf(hidetype, "goblin hide");
            lev = (int) lev / 2;
            break;
        case RACE_TROLL:
            sprintf(hidetype, "troll leather");
            break;
        case RACE_MFLAYER:
            sprintf(hidetype, "mindflayer hide");
            break;
        case RACE_ENFAN:
            sprintf(hidetype, "enfan hide");
            lev = (int) lev / 2;
            break;
        case RACE_DROW:
            sprintf(hidetype, "drow hide");
            lev = (int) lev / 2;
            break;
        case RACE_TYTAN:
            sprintf(hidetype, "tytan hide");
            break;
        case RACE_SMURF:
            sprintf(hidetype, "blue leather");
            break;
        case RACE_ROO:
            sprintf(hidetype, "roo hide");
            break;
        case RACE_ASTRAL:
            sprintf(hidetype, "strange hide");
            break;
        case RACE_GIANT_HILL:
            sprintf(hidetype, "hill giant hide");
            break;
        case RACE_GIANT_FROST:
            sprintf(hidetype, "frost giant hide");
            break;
        case RACE_GIANT_FIRE:
            sprintf(hidetype, "fire giant hide");
            break;
        case RACE_GIANT_CLOUD:
            sprintf(hidetype, "cloud giant hide");
            break;
        case RACE_GIANT_STORM:
            sprintf(hidetype, "storm giant hide");
            break;
        case RACE_GIANT_STONE:
            sprintf(hidetype, "stone giant hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_RED:
            sprintf(hidetype, "red dragon hide");
            acapply += 2;
            acbonus += 3;
            break;
        case RACE_DRAGON_BLACK:
            sprintf(hidetype, "black dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_GREEN:
            sprintf(hidetype, "green dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_WHITE:
            sprintf(hidetype, "white dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_BLUE:
            sprintf(hidetype, "blue dragon hide");
            acapply++;
            acbonus++;
            break;
        case RACE_DRAGON_SILVER:
            sprintf(hidetype, "silver dragon hide");
            acapply += 2;
            acbonus += 2;
            break;
        case RACE_DRAGON_GOLD:
            sprintf(hidetype, "gold dragon hide");
            acapply += 2;
            acbonus += 3;
            break;
        case RACE_DRAGON_BRONZE:
            sprintf(hidetype, "bronze dragon hide");
            acapply++;
            acbonus += 2;
            break;
        case RACE_DRAGON_COPPER:
            sprintf(hidetype, "copper dragon hide");
            acapply++;
            acbonus += 2;
            break;
        case RACE_DRAGON_BRASS:
            sprintf(hidetype, "brass dragon hide");
            acapply++;
            acbonus += 2;
            break;
    case RACE_DRAGON_AMETHYST:
            sprintf(hidetype, "amethyst dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_CRYSTAL:
            sprintf(hidetype, "crystal dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_EMERALD:
            sprintf(hidetype, "emerald dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_SAPPHIRE:
            sprintf(hidetype, "sapphire dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_TOPAZ:
            sprintf(hidetype, "topaz dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_BROWN:
            sprintf(hidetype, "brown dragon hide");
            acapply ++;
            acbonus ++;
            break;
    case RACE_DRAGON_CLOUD:
            sprintf(hidetype, "cloud dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_DEEP:
            sprintf(hidetype, "deep dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_MERCURY:
            sprintf(hidetype, "mercury dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_MIST:
            sprintf(hidetype, "mist dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_SHADOW:
            sprintf(hidetype, "shadow dragon hide");
            acapply += 7;
            acbonus += 7;
            break;
    case RACE_DRAGON_STEEL:
            sprintf(hidetype, "steel dragon hide");
            acapply += 6;
            acbonus += 6;
            break;
    case RACE_DRAGON_YELLOW:
            sprintf(hidetype, "yellow dragon hide");
            acapply += 4;
            acbonus += 4;
            break;
    case RACE_DRAGON_TURTLE:
            sprintf(hidetype, "turtle dragon hide");
            acapply += 8;
            acbonus += 8;
            break;
        case RACE_GOLEM:
        case RACE_SKEXIE:
        case RACE_TROGMAN:
        case RACE_LIZARDMAN:
        case RACE_PATRYN:
        case RACE_LABRAT:
        case RACE_SARTAN:
        case RACE_PRIMATE:
        case RACE_PREDATOR:
        case RACE_PARASITE:
        case RACE_HALFBREED:
        case RACE_SPECIAL:
        case RACE_LYCANTH:
        case RACE_ELEMENT:
        case RACE_HERBIV:
        case RACE_PLANAR:
        case RACE_HORSE:
        case RACE_DRAAGDIM:
        case RACE_GOD:
        default:
            sprintf(hidetype, "leather");
            break;

        }

        /*
         * figure out what type of armor it is and make it. 
         */

        acbonus += (int) lev / 10;      /* 1-6 */
        acapply += (int) lev / 10;      /* 1-6 */

        /*
         * class bonus 
         */

        if (HasClass(ch, CLASS_RANGER)) {
            acbonus += 1;
        }

        /*
         * racial bonus 
         */

        if (acbonus < 0) {
            acbonus = 0;
        }
        if (acapply < 0) {
            acapply = 0;
        }
        if (!strcmp(itemtype, "shield")) {
            if ((r_num = real_object(TAN_SHIELD)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply++;
            acbonus++;
            strcat(hidetype, " shield");
        } else if (!strcmp(itemtype, "jacket")) {
            if ((r_num = real_object(TAN_JACKET)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply += 5;
            acbonus += 2;
            strcat(hidetype, " jacket");
        } else if (!strcmp(itemtype, "boots")) {
            if ((r_num = real_object(TAN_BOOTS)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply--;
            if (acapply < 0) {
                acapply = 0;
            }
            acbonus--;
            if (acbonus < 0) {
                acbonus = 0;
            }
            strcat(hidetype, " pair of boots");
        } else if (!strcmp(itemtype, "gloves")) {
            if ((r_num = real_object(TAN_GLOVES)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }

            acapply--;
            if (acapply < 0) {
                acapply = 0;
            }
            acbonus--;
            if (acbonus < 0) {
                acbonus = 0;
            }
            strcat(hidetype, " pair of gloves");
        } else if (!strcmp(itemtype, "leggings")) {
            if ((r_num = real_object(TAN_LEGGINGS)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply++;
            acbonus++;
            strcat(hidetype, " set of leggings");
        } else if (!strcmp(itemtype, "sleeves")) {
            if ((r_num = real_object(TAN_SLEEVES)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply++;
            acbonus++;
            strcat(hidetype, " set of sleeves");
        } else if (!strcmp(itemtype, "helmet")) {
            if ((r_num = real_object(TAN_HELMET)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            acapply--;
            if (acapply < 0) {
                acapply = 0;
            }
            acbonus--;
            if (acbonus < 0) {
                acbonus = 0;
            }
            strcat(hidetype, " helmet");
        } else if (!strcmp(itemtype, "bag")) {
            if ((r_num = real_object(TAN_BAG)) >= 0) {
                hide = read_object(r_num, REAL);
                obj_to_char(hide, ch);
            }
            strcat(hidetype, " bag");
        } else {
            send_to_char("Illegal type of equipment!\n\r", ch);
            return;
        }

        sprintf(buf, "%s name %s", itemtype, hidetype);
        do_ooedit(ch, buf, 0);

        sprintf(buf, "%s ldesc A %s lies here", itemtype, hidetype);
        do_ooedit(ch, buf, 0);

        sprintf(buf, "%s sdesc a %s", itemtype, hidetype);
        do_ooedit(ch, buf, 0);

        /*
         * we do not mess with vX if the thing is a bag 
         */
        if (strcmp(itemtype, "bag")) {
            sprintf(buf, "%s v0 %d", itemtype, acapply);
            do_ooedit(ch, buf, 0);
            /*
             * I think v1 is how many times it can be hit, so lev
             * of corpse /10 times 
             */
            sprintf(buf, "%s v1 %d", itemtype, (int) lev / 10);
            do_ooedit(ch, buf, 0);
            /*
             * add in AC bonus here 
             */
            sprintf(buf, "%s aff1 %d 17", itemtype, 0 - acbonus);
            do_ooedit(ch, buf, 0);
        }
        /*
         * was not a bag ^ 
         */
        j->affected[1].modifier = 0;    
        /* 
         * make corpse unusable
         * for another tan 
         */

        sprintf(buf, "You hack at the %s and finally make the %s.\n\r",
                j->short_description, itemtype);
        send_to_char(buf, ch);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);

        sprintf(buf, "%s skins %s for it's hide.", GET_NAME(ch),
                j->short_description);
        act(buf, TRUE, ch, 0, 0, TO_ROOM);
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
        return;
    } else {
        send_to_char("Sorry, nothing left of the carcass to make an item "
                     "with.\n\r", ch);
        return;
    }
}


void do_find_food(struct char_data *ch, char *arg, int cmd)
{
    int             r_num,
                    percent = 0;
    struct obj_data *obj;

    if (!ch->skills) {
        return;
    }
    if (!ch->skills[SKILL_FIND_FOOD].learned > 0) {
        send_to_char("You search blindly for anything, but fail.\n\r.", ch);
        return;
    }

    if (!OUTSIDE(ch)) {
        send_to_char("You need to be outside.\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);

    if (ch->skills && ch->skills[SKILL_FIND_FOOD].learned &&
        GET_POS(ch) > POSITION_SITTING) {

        if (percent > ch->skills[SKILL_FIND_FOOD].learned) {
            act("You search around for some edibles but failed to find "
                "anything.", TRUE, ch, 0, 0, TO_CHAR);
            act("$n searches and searches for something to eat but comes up "
                "empty.", TRUE, ch, 0, 0, TO_ROOM);
            LearnFromMistake(ch, SKILL_FIND_FOOD, 0, 90);
        } else {
            act("You search around for some edibles and managed to find some "
                "roots and berries.", TRUE, ch, 0, 0, TO_CHAR);
            act("$n searches the area for something to eat and manages to find "
                "something.", TRUE, ch, 0, 0, TO_ROOM);
            if ((r_num = real_object(FOUND_FOOD)) >= 0) {
                obj = read_object(r_num, REAL);
                obj_to_char(obj, ch);
                send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                             "using your abilities.$c0007\n\r", ch);
                gain_exp(ch, 100);
            }
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
        act("You search around for some edibles but failed to find anything.",
            TRUE, ch, 0, 0, TO_CHAR);
        act("$n searches and searches for something to eat but comes up empty.",
            TRUE, ch, 0, 0, TO_ROOM);
    }
}


void do_find_water(struct char_data *ch, char *arg, int cmd)
{
    int             r_num,
                    percent = 0;
    struct obj_data *obj;

    if (!ch->skills) {
        return;
    }
    if (!ch->skills[SKILL_FIND_WATER].learned > 0) {
        send_to_char("You search blindly for anything, but fail.\n\r.", ch);
        return;
    }

    if (!OUTSIDE(ch)) {
        send_to_char("You need to be outside.\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);

    if (ch->skills && ch->skills[SKILL_FIND_WATER].learned &&
        GET_POS(ch) > POSITION_SITTING) {
        if (percent > ch->skills[SKILL_FIND_WATER].learned) {
            act("You search around for stream or puddle of water but failed "
                "to find anything.", TRUE, ch, 0, 0, TO_CHAR);
            act("$n searches and searches for something to drink but comes up "
                "empty.", TRUE, ch, 0, 0, TO_ROOM);
            LearnFromMistake(ch, SKILL_FIND_WATER, 0, 90);
        } else {
            act("You search around and find enough water to fill a water cup.",
                TRUE, ch, 0, 0, TO_CHAR);
            act("$n searches the area for something to drink and manages to "
                "find a small amount of water.", TRUE, ch, 0, 0, TO_ROOM);
            if ((r_num = real_object(FOUND_WATER)) >= 0) {
                obj = read_object(r_num, REAL);
                obj_to_char(obj, ch);
                send_to_char("$c000BYou receive $c000W100 $c000Bexperience "
                             "for using your abilities.$c0007\n\r", ch);
                gain_exp(ch, 100);
            }
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
        act("You search around for stream or puddle of water but failed to "
            "find anything.", TRUE, ch, 0, 0, TO_CHAR);
        act("$n searches and searches for something to drink but comes up "
            "empty.", TRUE, ch, 0, 0, TO_ROOM);
    }
}

void do_find(struct char_data *ch, char *arg, int cmd)
{
    char           *findwhat;

    if (!ch->skills) {
        return;
    }
    arg = get_argument(arg, &findwhat);

    if( !findwhat ) {
        send_to_char("Find what?\n\r", ch);
        return;
    }

    if (!strcmp(findwhat, "water")) {
        do_find_water(ch, arg, cmd);
    } else if (!strcmp(findwhat, "food")) {
        do_find_food(ch, arg, cmd);
    } else if (!strcmp(findwhat, "traps")) {
        do_find_traps(ch, arg, cmd);
    } else {
        send_to_char("Find what?!?!?\n\r", ch);
    }
}

void do_bellow(struct char_data *ch, char *arg, int cmd)
{
    struct char_data *vict,
                   *tmp;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_BARBARIAN) && !HasClass(ch, CLASS_WARRIOR)) {
        send_to_char("What do you think you are, a warrior!\n\r", ch);
        return;
    }

    if (check_peaceful(ch, "You feel too peaceful to contemplate "
                           "violence.\n\r")) { 
        return;
    }

    if (check_soundproof(ch)) {
        send_to_char("You cannot seem to break the barrier of silence "
                     "here.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 15) {
        send_to_char("You just cannot get enough energy together for a "
                     "bellow.\n\r", ch);
        return;
    }

    if (ch->skills && ch->skills[SKILL_BELLOW].learned &&
        number(1, 101) < ch->skills[SKILL_BELLOW].learned) {

        GET_MANA(ch) -= 15;
        send_to_char("You let out a bellow that rattles your bones!\n\r", ch);
        act("$n lets out a bellow that rattles your bones.", FALSE, ch, 0,
            0, TO_ROOM);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your combat abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);

        for (vict = character_list; vict; vict = tmp) {
            tmp = vict->next;
            if (ch->in_room == vict->in_room && ch != vict &&
                !in_group(ch, vict) && !IS_IMMORTAL(vict)) {
                if (GetMaxLevel(vict) - 3 <= GetMaxLevel(ch)) {
                    if (!saves_spell(vict, SAVING_PARA)) {
                        if ((GetMaxLevel(ch) + number(1, 40)) > 70) {
                            act("You stunned $N!", TRUE, ch, 0, vict, TO_CHAR);
                            act("$n stuns $N with a loud bellow!", FALSE, ch, 
                                0, vict, TO_ROOM);
                            GET_POS(vict) = POSITION_STUNNED;
                            AddFeared(vict, ch);
                        } else {
                            act("You scared $N to death with your bellow!",
                                TRUE, ch, 0, vict, TO_CHAR);
                            act("$n scared $N with a loud bellow!",
                                FALSE, ch, 0, vict, TO_ROOM);
                            do_flee(vict, "", 0);
                            AddFeared(vict, ch);
                        }
                    } else {
                        AddHated(vict, ch);
                        set_fighting(vict, ch);
                    }
                } else {
                    /*
                     * nothing happens 
                     */
                    AddHated(vict, ch);
                    set_fighting(vict, ch);
                }

                if (ch->specials.remortclass == (WARRIOR_LEVEL_IND + 1) && 
                    GET_POS(vict) > POSITION_DEAD) {
                    act("$c000rYou send $N reeling with a mighty bellow.$c000w",
                        FALSE, ch, 0, vict, TO_CHAR);
                    act("$c000r$N is sent reeling by $n's mighty bellow.$c000w",
                        FALSE, ch, 0, vict, TO_NOTVICT);
                    act("$c000r$n sends you reeling with a mighty bellow."
                        "$c000w", FALSE, ch, 0, vict, TO_VICT);
                    GET_POS(vict) = POSITION_SITTING;
                    WAIT_STATE(vict, PULSE_VIOLENCE * 2);
                    GET_POS(vict) = POSITION_SITTING;
                    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                }
            }
        }
    } else {
        send_to_char("You let out a squalk!\n\r", ch);
        act("$n lets out a squalk of a bellow then blushes.", FALSE, ch, 0,
            0, TO_ROOM);
        LearnFromMistake(ch, SKILL_BELLOW, 0, 95);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

/*
 * ranger skill 
 */
void do_carve(struct char_data *ch, char *argument, int cmd)
{
    char           *arg1;
    char            buffer[MAX_STRING_LENGTH];
    struct obj_data *corpse;
    struct obj_data *food;
    int             i,
                    r_num;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_RANGER)) {
        return;
    }

    if (!ch->skills[SKILL_RATION].learned) {
        send_to_char("Best leave the carving to the skilled.\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg1);

    if( !arg1 || 
        !(corpse = get_obj_in_list_vis(ch, arg1, 
                                       real_roomp(ch->in_room)->contents))) {
        send_to_char("That's not here.\n\r", ch);
        return;
    }

    if (!IS_CORPSE(corpse)) {
        send_to_char("You can't carve that!\n\r", ch);
        return;
    }

    if (corpse->obj_flags.weight < 70) {
        send_to_char("There is no good meat left on it.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) < 10) && !IS_IMMORTAL(ch)) {
        send_to_char("You don't have the concentration to do this.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_RATION].learned < dice(1, 101)) {
        send_to_char("You can't seem to locate the choicest parts of the "
                     "corpse.\n\r", ch);
        GET_MANA(ch) -= 5;
        LearnFromMistake(ch, SKILL_RATION, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    }

    act("$n carves up the $p and creates a healthy ration.", FALSE, ch,
        corpse, 0, TO_ROOM);
    send_to_char("You carve up a fat ration.\n\r", ch);

    if ((r_num = real_object(FOUND_FOOD)) >= 0) {
        food = read_object(r_num, REAL);
        food->name = (char *) strdup("ration slice filet food");

        sprintf(buffer, "a Ration%s", corpse->short_description + 10);
        food->short_description = (char *) strdup(buffer);
        food->action_description = (char *) strdup(buffer);
        sprintf(buffer, "%s is lying on the ground.", food->short_description);
        food->description = (char *) strdup(buffer);
        corpse->obj_flags.weight = corpse->obj_flags.weight - 50;

        i = number(1, 6);
        if (i == 6) {
            food->obj_flags.value[3] = 1;
        }
        obj_to_room(food, ch->in_room);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

int IS_FOLLOWING(struct char_data *tch, struct char_data *person)
{
    if (person->master) {
        person = person->master;
    }
    if (tch->master) {
        tch = tch->master;
    }
    return (person == tch && IS_AFFECTED(person, AFF_GROUP) && 
            IS_AFFECTED(tch, AFF_GROUP));
}


/*
 * BREW for mages, done by Greg Hovey..
 */


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

    if (!(obj = read_object(EMPTY_SCROLL, VIRTUAL))) {
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

        extract_obj(obj);
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

        if (obj->name) {
            free(obj->name);
        }
        sprintf(buf, "scroll %s", spells[sn]);
        obj->name = (char *) strdup(buf);

        if (obj->description) {
            free(obj->description);
        }
        obj->description = strdup("A scroll, bound with enchantments, lies on "
                                  "the ground.");

        if (IS_IMMORTAL(ch)) {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = MAX_MORT;
            
            /* 
             * set ego to level.
             */
            obj->level = MAX_MORT;
        } else {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = GetMaxLevel(ch);

            /*
             * set ego to level.
             */
            obj->level = GetMaxLevel(ch);
        }
        
        /* 
         * set spell in slot.
         */
        obj->obj_flags.value[1] = sn + 1;
        obj->obj_flags.timer = 42;

        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        obj_to_char(obj, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

void do_flurry(struct char_data *ch, char *argument, int cmd)
{
    int             percent = 0;
    struct affected_type af;

    dlog("in do_flurry");

    if (ch->specials.remortclass != MONK_LEVEL_IND + 1) {
        send_to_char("Err, you're likely to trip.\n\r", ch);
        return;
    }
    if (!ch->skills) {
        Log("Char without skills trying to flurry");
        return;
    }
    if (!ch->skills[SKILL_FLURRY].learned) {
        send_to_char("You need some guidance before attempting this.\n\r", ch);
        return;
    }
    if (!ch->specials.fighting) {
        send_to_char("You can only do this when engaged in battle.\n\r", ch);
        return;
    }
    if (affected_by_spell(ch, SKILL_FLURRY)) {
        send_to_char("You're doing the best you can!\n\r", ch);
        return;
    }
    percent = number(1, 101);

    if (ch->skills[SKILL_FLURRY].learned) {
        if (percent > ch->skills[SKILL_FLURRY].learned) {
            send_to_char("You try a daunting combat move, but trip and "
                         "stumble.\n\r", ch);
            act("$n tries a daunting combat move, but trips and stumbles.",
                FALSE, ch, 0, 0, TO_ROOM);
            LearnFromMistake(ch, SKILL_FLURRY, 0, 90);
        } else {
            act("$n starts delivering precision strikes at lightning speed!",
                TRUE, ch, 0, 0, TO_ROOM);
            act("You start delivering precision strikes at lightning speed!",
                TRUE, ch, 0, 0, TO_CHAR);
            af.type = SKILL_FLURRY;
            af.duration = 0;
            af.modifier = 5;
            af.location = APPLY_HITROLL;
            af.bitvector = 0;
            affect_to_char(ch, &af);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void do_flowerfist(struct char_data *ch, char *argument, int cmd)
{
    int             percent = 0,
                    dam = 0;
    struct char_data *tch;
    struct char_data *tempchar;
    
    dlog("in do_flowerfist");

    if (ch->specials.remortclass != MONK_LEVEL_IND + 1) {
        send_to_char("Err, are you sure you want to pick flowers at this "
                     "moment?\n\r", ch);
        return;
    }

    if (!ch->skills) {
        Log("Char without skills trying to flowerfist");
        return;
    }

    if (!ch->skills[SKILL_FLOWERFIST].learned) {
        send_to_char("You need some guidance before attempting this.\n\r", ch);
        return;
    }

    percent = number(1, 120);

    if (ch->skills[SKILL_FLOWERFIST].learned) {
        if (percent > ch->skills[SKILL_FLOWERFIST].learned + GET_DEX(ch)) {
            send_to_char("You can't seem to get it right.\n\r", ch);
            act("$n seems to want to pick some flowers.", FALSE, ch, 0, 0,
                TO_ROOM);
            LearnFromMistake(ch, SKILL_FLOWERFIST, 0, 90);
        } else {
            act("$n chants loudly, while $s fists seem to be all over the "
                "place!", TRUE, ch, 0, 0, TO_ROOM);
            act("You chant loudly, while sending hits throughout the area.",
                TRUE, ch, 0, 0, TO_CHAR);

            for (tch = real_roomp(ch->in_room)->people; tch;
                 tch = tempchar) {
                tempchar = tch->next_in_room;
                if (!in_group(tch, ch) && !IS_IMMORTAL(tch)) {
                    dam = dice(6, 6);
                    damage(ch, tch, dam, TYPE_HIT);
                }
            }

            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
