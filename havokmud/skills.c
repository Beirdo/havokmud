/*
 ***  DaleMUD
 */

#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "protos.h"

int             choose_exit(int in_room, int tgt_room, int dvar);

/*
 * struct room_data *real_roomp(int); 
 */

int             remove_trap(struct char_data *ch, struct obj_data *trap);

void            do_find_traps(struct char_data *ch, char *arg, int cmd);
void            do_find_food(struct char_data *ch, char *arg, int cmd);
void            do_find_water(struct char_data *ch, char *arg, int cmd);

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
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
extern char    *dirs[];
extern struct char_data *character_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];
extern long     SystemFlags;
extern char    *spells[];

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

#define STEED_TEMPLATE 44

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
    char            name[30];
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
    only_argument(argument, name);
    if (!(victim = get_char_room_vis(ch, name))) {
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
    char            name[256],
                    buf[256],
                    found = FALSE;
    int             dist,
                    code;
    struct hunting_data huntd;
    struct char_data *scan;
    extern struct char_data *character_list;

#if NOTRACK
    send_to_char("Sorry, tracking is disabled. Try again after reboot.\n\r",
                 ch);
    return;
#endif

    only_argument(argument, name);

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
                    type[128],
                    direction[128];

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
    for (; isspace(*arg); arg++) {
        /* 
         * Empty loop 
         */
    }

    argument_interpreter(arg, type, direction);

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
        do_look(ch, "", 0);

        DisplayMove(ch, dir, was_in, 1);
        if (check_falling(ch)) {
            return;
        }

        if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
            GetMaxLevel(ch) < LOW_IMMORTAL) {
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
                    do_look(ch, "", 0);

                    DisplayMove(ch, dir, was_in, 1);
                    if (check_falling(ch)) {
                        return;
                    }

                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                        GetMaxLevel(ch) < LOW_IMMORTAL) {
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
        if (ch->skills[SKILL_SWIM].learned < 95 &&
            ch->skills[SKILL_SWIM].learned > 0 &&
            number(1, 101) > ch->skills[SKILL_SWIM].learned) {

            send_to_char("You feel a bit braver, though\n\r", ch);
            ch->skills[SKILL_SWIM].learned++;
        }
        return;
    }

    af.type = SKILL_SWIM;
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

void do_spy(struct char_data *ch, char *arg, int cmd)
{
    struct affected_type af;
    byte            percent;

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
        if (ch->skills[SKILL_SPY].learned < 95 && 
            ch->skills[SKILL_SPY].learned > 0 &&
            number(1, 101) > ch->skills[SKILL_SPY].learned) {

            ch->skills[SKILL_SPY].learned++;
        }

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

int remove_trap(struct char_data *ch, struct obj_data *trap)
{
    int             num;
    struct obj_data *t;
    /*
     * to disarm traps inside item 
     */
    if (ITEM_TYPE(trap) == ITEM_CONTAINER) {
        for (t = trap->contains; t; t = t->next_content) {
            if (ITEM_TYPE(t) == ITEM_TRAP && GET_TRAP_CHARGES(t) > 0) {
                return (remove_trap(ch, t));
            }
        }
    } else if (ITEM_TYPE(trap) != ITEM_TRAP) {
        send_to_char("That's no trap!\n\r", ch);
        return (FALSE);
    }

    if (GET_TRAP_CHARGES(trap) <= 0) {
        send_to_char("That trap is already sprung!\n\r", ch);
        return (FALSE);
    }

    num = number(1, 101);
    if (num < ch->skills[SKILL_REMOVE_TRAP].learned) {
        send_to_char("<Click>\n\r", ch);
        act("$n disarms $p", FALSE, ch, trap, 0, TO_ROOM);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using"
                     " your combat abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        GET_TRAP_CHARGES(trap) = 0;
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
        send_to_char("<Click>\n\r(uh oh)\n\r", ch);
        act("$n attempts to disarm $p", FALSE, ch, trap, 0, TO_ROOM);
        TriggerTrap(ch, trap);
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
        if (ch->skills[SKILL_FEIGN_DEATH].learned < 95 &&
            ch->skills[SKILL_FEIGN_DEATH].learned > 0) {
            if (number(1, 101) > ch->skills[SKILL_FEIGN_DEATH].learned) {
                ch->skills[SKILL_FEIGN_DEATH].learned++;
            }
        }
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
        if (ch->skills[SKILL_FIRST_AID].learned < 95 && 
            ch->skills[SKILL_FIRST_AID].learned > 0 &&
            number(1, 101) > ch->skills[SKILL_FIRST_AID].learned) {
            ch->skills[SKILL_FIRST_AID].learned++;
            send_to_char("You learn from your mistake.\n\r", ch);
        }
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
    } else if (ch->skills[SKILL_DISGUISE].learned < 95 &&
               ch->skills[SKILL_DISGUISE].learned > 0 &&
               number(1, 101) > ch->skills[SKILL_DISGUISE].learned) {
        ch->skills[SKILL_DISGUISE].learned++;
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
void do_climb(struct char_data *ch, char *arg, int cmd)
{
    extern char    *dirs[];
    int             dir;
    struct room_direction_data *exitp;
    int             was_in,
                    roll;
    extern char    *dirs[];

    char            buf[256],
                    direction[128];

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

    for (; isspace(*arg); arg++) {
        /* 
         * Empty loops 
         */
    }

    only_argument(arg, direction);

    if ((dir = search_block(direction, dirs, FALSE)) < 0) {
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
                do_look(ch, "", 0);
                DisplayMove(ch, dir, was_in, 1);
                if (!check_falling(ch) && 
                    IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                    GetMaxLevel(ch) < LOW_IMMORTAL) {
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
        do_look(ch, "", 0);
    }

    GET_POS(ch) = POSITION_SITTING;
    if (i > GET_HIT(ch)) {
        GET_HIT(ch) = 1;
    } else {
        GET_HIT(ch) -= i;
    }
}

void do_tan(struct char_data *ch, char *arg, int cmd)
{
    struct obj_data *j = 0;
    struct obj_data *hide;
    char            itemname[80],
                    itemtype[80],
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

    arg = one_argument(arg, itemname);
    arg = one_argument(arg, itemtype);

    if (!*itemname) {
        send_to_char("Tan what?\n\r", ch);
        return;
    }

    if (!*itemtype) {
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

void do_find_traps(struct char_data *ch, char *arg, int cmd)
{
    if (!ch->skills) {
        return;
    }
    if (!IS_PC(ch)) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_THIEF)) {
        send_to_char("What do you think you are?!?\n\r", ch);
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }

    send_to_char("You are already on the look out for those silly.\n\r", ch);
}

void do_find(struct char_data *ch, char *arg, int cmd)
{
    char            findwhat[30];

    if (!ch->skills) {
        return;
    }
    arg = one_argument(arg, findwhat);

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
    char            arg1[MAX_STRING_LENGTH];
    char            arg2[MAX_STRING_LENGTH];
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
        send_to_char("Hum, you wonder how you would do this...\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_RATION].learned) {
        send_to_char("Best leave the carving to the skilled.\n\r", ch);
        return;
    }

    half_chop(argument, arg1, arg2);
    corpse = get_obj_in_list_vis(ch, arg1, (real_roomp(ch->in_room)->contents));

    if (!corpse) {
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

    if ((GET_MANA(ch) < 10) && GetMaxLevel(ch) < LOW_IMMORTAL) {
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
        sprintf(arg2, "%s is lying on the ground.", buffer);
        food->description = (char *) strdup(arg2);
        corpse->obj_flags.weight = corpse->obj_flags.weight - 50;

        i = number(1, 6);
        if (i == 6) {
            food->obj_flags.value[3] = 1;
        }
        obj_to_room(food, ch->in_room);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

void do_doorway(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *target;
    int             location;
    struct room_data *rp;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Your mind is not developed enough to do this\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_DOORWAY].learned) {
        send_to_char("You have not trained your mind to do this\n\r", ch);
        return;
    }

    only_argument(argument, target_name);
    if (!(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    location = target->in_room;
    rp = real_roomp(location);
    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (GetMaxLevel(target) > MAX_MORT || !rp ||
        IS_SET(rp->room_flags, PRIVATE | NO_SUM | NO_MAGIC)) {
        send_to_char("Your mind is not yet strong enough.\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
        send_to_char("The planes are fuzzy, you cannot portal!\n", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (!IsOnPmp(target->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_LINKDEAD(target)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_IMMORTAL(target)) {
        send_to_char("You can't doorway to someone of that magnitude!\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 20 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You have a headache. Better rest before you try this "
                     "again.\n\r", ch);
        return;
    } else if (dice(1, 101) > ch->skills[SKILL_DOORWAY].learned) {
        send_to_char("You cannot open a portal at this time.\n\r", ch);
        act("$n seems to briefly disappear, then returns!", FALSE, ch, 0,
            0, TO_ROOM);
        GET_MANA(ch) -= 10;
        LearnFromMistake(ch, SKILL_DOORWAY, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    } else {
        GET_MANA(ch) -= 20;
        send_to_char("You close your eyes and open a portal and quickly step "
                     "through.\n\r", ch);
        act("$n closes $s eyes and a shimmering portal appears!", FALSE,
            ch, 0, 0, TO_ROOM);
        act("$n steps through the portal and the portal dissapears!",
            FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, location);
        act("A portal appears before you and $n steps through!", FALSE, ch,
            0, 0, TO_ROOM);
        do_look(ch, "", 15);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

void do_psi_portal(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *target;
    struct char_data *follower;
    struct char_data *leader;
    struct follow_type *f_list;
    int             location;
    int             check = 0;
    struct room_data *rp;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Your mind is not developed enough to do this.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_PORTAL].learned) {
        send_to_char("You have not trained your mind to do this.\n\r", ch);
        return;
    }

    only_argument(argument, target_name);
    if (!(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    location = target->in_room;
    rp = real_roomp(location);
    if (GetMaxLevel(target) > MAX_MORT || !rp ||
        IS_SET(rp->room_flags, PRIVATE | NO_SUM | NO_MAGIC)) {
        send_to_char("You cannot penetrate the auras surrounding that "
                     "person.\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
        send_to_char("The planes are fuzzy, you cannot portal!\n", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (!IsOnPmp(target->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_LINKDEAD(target)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_IMMORTAL(target)) {
        send_to_char("You can't portal to someone of that magnitude!\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 50 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You have a headache. Better rest before you try this "
                     "again.\n\r", ch);
        return;
    } else if (dice(1, 101) > ch->skills[SKILL_PORTAL].learned) {
        send_to_char("You fail to open a portal at this time.\n\r", ch);
        act("$n briefly summons a portal, then curses as it disappears.",
            FALSE, ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 25;
        LearnFromMistake(ch, SKILL_PORTAL, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    } else {
        GET_MANA(ch) -= 50;
        send_to_char("You close your eyes and open a portal and quickly step "
                     "through.\n\r", ch);
        act("$n closes their eyes and a shimmering portal appears!", FALSE,
            ch, 0, 0, TO_ROOM);
        act("A portal appears before you!", FALSE, target, 0, 0, TO_ROOM);
        leader = ch->master;
        if (!leader) {
            leader = ch;
            check = 1;
        }
        /*
         * leader goes first, otherwise we miss them 
         */
        if (leader != ch && !leader->specials.fighting && 
            IS_AFFECTED(leader, AFF_GROUP) &&
            (IS_PC(leader) || IS_SET(leader->specials.act, ACT_POLYSELF))) {
            act("$n steps through the portal and disappears!", FALSE,
                leader, 0, 0, TO_ROOM);
            send_to_char("You step through the shimmering portal.\n\r", leader);
            char_from_room(leader);
            char_to_room(leader, location);
            act("$n steps out of a portal before you!", FALSE, leader, 0,
                0, TO_ROOM);
            do_look(leader, "", 15);
        }

        for (f_list = leader->followers; f_list; f_list = f_list->next) {
            follower = f_list->follower;
            if (!follower) {
                Log("pointer error in portal follower loop");
                return;
            }

            if (follower && follower->in_room == ch->in_room &&
                follower != ch && !follower->specials.fighting &&
                (IS_PC(follower) || 
                 IS_SET(follower->specials.act, ACT_POLYSELF)) && 
                IS_AFFECTED(follower, AFF_GROUP)) {
                act("$n steps through the portal and disappears!", FALSE,
                    follower, 0, 0, TO_ROOM);
                send_to_char("You step through the shimmering portal.\n\r",
                             follower);
                char_from_room(follower);
                char_to_room(follower, location);
                act("$n steps out of a portal before you!", FALSE,
                    follower, 0, 0, TO_ROOM);
                do_look(follower, "", 15);
            }
        }

        if (check == 1) {
            send_to_char("Now that all your comrades are through, you follow "
                         "them and close the portal.\n\r", ch);
        }
        act("$n steps into the portal just before it disappears.", FALSE,
            ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, location);
        do_look(ch, "", 15);
        act("$n appears out of the portal as it disappears!", FALSE, ch, 0,
            0, TO_ROOM);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

void do_mindsummon(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *target;
    int             location;
    struct room_data *rp;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Your mind is not developed enough to do this\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_SUMMON].learned) {
        send_to_char("You have not trained your mind to do this\n\r", ch);
        return;
    }

    only_argument(argument, target_name);
    if (!(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }
    
    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }
    if (target == ch) {
        send_to_char("You're already in the room with yourself!\n\r", ch);
        return;
    }

    location = target->in_room;
    rp = real_roomp(location);
    if (!rp || IS_SET(rp->room_flags, PRIVATE | NO_SUM | NO_MAGIC)) {
        send_to_char("Your mind cannot seem to locate this individual.\n\r",
                     ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOSUMMON)) {
        send_to_char("A mistical fog blocks your attemps!\n", ch);
        return;
    }

    if (!IsOnPmp(target->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    location = target->in_room;
    rp = real_roomp(location);
    if (!rp || rp->sector_type == SECT_AIR ||
        rp->sector_type == SECT_WATER_SWIM) {
        send_to_char("You cannot seem to focus on the target.\n\r", ch);
        return;
    }

    location = ch->in_room;
    rp = real_roomp(location);
    if (!rp || IS_SET(rp->room_flags, PRIVATE | NO_SUM | NO_MAGIC)) {
        send_to_char("Arcane magics prevent you from summoning here.\n\r", ch);
        return;
    }

    location = ch->in_room;
    rp = real_roomp(location);
    if (!rp || rp->sector_type == SECT_AIR || 
        rp->sector_type == SECT_WATER_SWIM) {
        send_to_char("You cannot seem to focus correctly here.\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_LINKDEAD(target)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(target) && IS_IMMORTAL(target)) {
        send_to_char("You can't summon someone of that magnitude!\n\r", ch);
        return;
    }

    /*
     * we check hps on mobs summons 
     */

    if (!IS_SET(target->specials.act, ACT_POLYSELF) && !IS_PC(target)) {
        if (GetMaxLevel(target) > MAX_MORT || 
            GET_MAX_HIT(target) > GET_HIT(ch)) {
            send_to_char("Your mind is not yet strong enough to summon this "
                         "individual.\n\r", ch);
            return;
        }
    } else if (GetMaxLevel(target) > MAX_MORT) {
        send_to_char("Your mind is not yet strong enough to summon this "
                     "individual.\n\r", ch);
        return;
    }

    if (CanFightEachOther(ch, target) && saves_spell(target, SAVING_SPELL)) {
        act("You failed to summon $N!", FALSE, ch, 0, target, TO_CHAR);
        act("$n tried to summon you!", FALSE, ch, 0, target, TO_VICT);
        return;
    }

    if ((GET_MANA(ch) < 30) && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You have a headache. Better rest before you try this "
                     "again.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_SUMMON].learned < dice(1, 101) || 
        target->specials.fighting) {
        send_to_char("You have failed to open the portal to summon this "
                     "individual.\n\r", ch);
        act("$n seems to think really hard then gasps in anger.", FALSE,
            ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 15;
        LearnFromMistake(ch, SKILL_SUMMON, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    GET_MANA(ch) -= 30;

    if (saves_spell(target, SAVING_SPELL) && IS_NPC(target)) {
        act("$N resists your attempt to summon!", FALSE, ch, 0, target,
            TO_CHAR);
        return;
    }

    act("You open a portal and bring forth $N!", FALSE, ch, 0, target, TO_CHAR);
    if (GetMaxLevel(target) < GetMaxLevel(ch) + 2 && !IS_PC(target)) {
        send_to_char("Their head is reeling. Give them a moment to "
                     "recover.\n\r", ch);
    }
    act("$n disappears in a shimmering wave of light!", TRUE, target, 0, 0,
        TO_ROOM);

    if (IS_PC(target)) {
        act("You are summoned by $n!", TRUE, ch, 0, target, TO_VICT);
    }
    char_from_room(target);
    char_to_room(target, ch->in_room);
    do_look(target, "", 0);
    act("$n summons $N from nowhere!", TRUE, ch, 0, target, TO_NOTVICT);

    if (GetMaxLevel(target) < GetMaxLevel(ch) + 2 && !IS_PC(target)) {
        act("$N is lying on the ground stunned!", TRUE, ch, 0, target, TO_ROOM);
        target->specials.position = POSITION_STUNNED;
    }

    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_canibalize(struct char_data *ch, char *argument, int cmd)
{
    long            hit_points,
                    mana_points;
    char            number[80];
    int             count;
    bool            num_found = TRUE;

    if (!ch->skills) {
        return;
    }
    if (!HasClass(ch, CLASS_PSI) || !ch->skills[SKILL_CANIBALIZE].learned) {
        send_to_char("You don't have any kind of control over your body like "
                     "that!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    only_argument(argument, number);

    /*
     * polax version of number validation 
     * NOTE: i changed num_found to be initially TRUE 
     */

    for (count = 0; num_found && (count < 9) && (number[count] != '\0');
         count++) {
        if (number[count] < '0' || number[count] > '9') {
            /* 
             * leading zero is ok 
             */
            num_found = FALSE;
        }
    }

    /*
     * polax modification ends 
     */

    /*
     * for (count=0;(!num_found) && (count<9);count++) if
     * ((number[count]>='1') && (number[count]<='9')) num_found=TRUE; 
     */

    if (!num_found) {
        send_to_char("Please include a number after the command.\n\r", ch);
        return;
    } else {
        /* 
         * forced the string to be proper length 
         */
        number[count] = '\0';
    }

    /* 
     * long int conversion 
     */
    sscanf(number, "%ld", &hit_points);

    if ((hit_points < 1) || (hit_points > 65535)) {
        send_to_char("Invalid number to canibalize.\n\r", ch);
        return;
    }

    mana_points = (hit_points * 2);

    if (mana_points < 0) {
        send_to_char("You can't do that, You Knob!\n\r", ch);
    }

    if ((int) ch->points.hit < (hit_points + 5)) {
        send_to_char("You don't have enough physical stamina to "
                     "canibalize.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) + mana_points) > (GET_MAX_MANA(ch))) {
        send_to_char("Your mind cannot handle that much extra energy.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_CANIBALIZE].learned < dice(1, 101)) {
        send_to_char("You try to canibalize your stamina but the energy "
                     "escapes before you can harness it.\n\r", ch);
        act("$n yelps in pain.", FALSE, ch, 0, 0, TO_ROOM);
        ch->points.hit -= hit_points;
        update_pos(ch);
        if (GET_POS(ch) == POSITION_DEAD) {
            die(ch, SKILL_CANIBALIZE);
        }
        LearnFromMistake(ch, SKILL_CANIBALIZE, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    }

    send_to_char("You sucessfully convert your stamina to Mental power.\n\r",
                 ch);
    act("$n briefly is surrounded by a red aura.", FALSE, ch, 0, 0, TO_ROOM);
    GET_HIT(ch) -= hit_points;
    GET_MANA(ch) += mana_points;

    update_pos(ch);
    if (GET_POS(ch) == POSITION_DEAD) {
        die(ch, SKILL_CANIBALIZE);
    }
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_flame_shroud(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You couldn't even light a match!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_FLAME_SHROUD].learned)) {
        send_to_char("You haven't studdied your psycokinetics.\n\r", ch);
        return;
    }
    if (affected_by_spell(ch, SPELL_FIRESHIELD)) {
        send_to_char("You're already surrounded with flames.\n\r", ch);
        return;
    }
    if (GET_MANA(ch) < 40 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You'll need more psycic energy to attempt this.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_FLAME_SHROUD].learned < dice(1, 101)) {
        send_to_char("You failed and barely avoided burning yourself.\n\r", ch);
        act("$n pats at a small flame on $s arm.", FALSE, ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 20;
        LearnFromMistake(ch, SKILL_FLAME_SHROUD, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    send_to_char("You summon a flaming aura to deter attackers.\n\r", ch);
    act("$n summons a flaming aura that surrounds $mself.", TRUE, ch, 0, 0,
        TO_ROOM);
    GET_MANA(ch) -= 40;

    /*
     * I do not use spell_fireshield because I want psi's shield to last
     * longer 
     */

    af.type = SPELL_FIRESHIELD;
    af.duration = GET_LEVEL(ch, PSI_LEVEL_IND) / 5 + 10;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_FIRESHIELD;
    affect_to_char(ch, &af);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_aura_sight(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You better find a mage or cleric.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_AURA_SIGHT].learned)) {
        send_to_char("You haven't leanred how to detect auras yet.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_DETECT_EVIL | SPELL_DETECT_MAGIC)) {
        send_to_char("You already have partial aura sight.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 40) {
        send_to_char("You lack the energy to convert auras to visible "
                     "light.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_AURA_SIGHT].learned < dice(1, 101)) {
        send_to_char("You try to detect the auras around you but you "
                     "fail.\n\r", ch);
        act("$n blinks $s eyes then sighs.", FALSE, ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 20;
        LearnFromMistake(ch, SKILL_AURA_SIGHT, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    }

    GET_MANA(ch) -= 40;

    spell_detect_evil(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    spell_detect_magic(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    spell_detect_good(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_great_sight(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You need a cleric or mage for better sight.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_GREAT_SIGHT].learned)) {
        send_to_char("You haven't learned to enhance your sight yet.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_DETECT_INVISIBLE | SPELL_SENSE_LIFE | 
                              SPELL_TRUE_SIGHT)) {
        send_to_char("You already have partial great sight.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 50) {
        send_to_char("You haven't got the mental strength to try this.\n\r",
                     ch);
        return;
    }

    if (ch->skills[SKILL_GREAT_SIGHT].learned < dice(1, 101)) {
        send_to_char("You fail to enhance your sight.\n\r", ch);
        act("$n's eyes flash.", FALSE, ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 25;
        LearnFromMistake(ch, SKILL_GREAT_SIGHT, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    }

    GET_MANA(ch) -= 50;
    spell_detect_invisibility(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    spell_sense_life(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    spell_true_seeing(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    send_to_char("You succeed in enhancing your vision.\n\r"
                 "There's so much you've missed.\n\r", ch);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_blast(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char            name[240];
    int             potency = 0,
                    level,
                    dam = 0;
    struct affected_type af;
    char            buf[256];

    if (!ch->skills) {
        return;
    }
    only_argument(argument, name);

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You do not have the mental power!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (ch->specials.fighting) {
        victim = ch->specials.fighting;
    } else {
        victim = get_char_room_vis(ch, name);
        if (!victim) {
            send_to_char("Exactly whom did you wish to blast?\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("Blast yourself? Your mother would be sad!\n\r", ch);
        return;
    }

    if (check_peaceful(ch, 
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    if (A_NOASSIST(ch, victim)) {
        act("$N is already engaged with someone else!", FALSE, ch, 0,
            victim, TO_CHAR);
        return;
    }

    if ((GetMaxLevel(victim) >= LOW_IMMORTAL || IS_IMMORTAL(victim)) &&
        !IS_NPC(victim)) {
        send_to_char("They ignore your attempt at humor!\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 25 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("Your mind is not up to the challenge at the moment.\n\r",
                     ch);
        return;
    }

    if (number(1, 101) > ch->skills[SKILL_PSIONIC_BLAST].learned) {
        GET_MANA(ch) -= 12;
        send_to_char("You try and focus your energy but it fizzles!\n\r", ch);
        LearnFromMistake(ch, SKILL_PSIONIC_BLAST, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    if (!IS_IMMORTAL(victim)) {
        act("$n focuses $s mind on $N's mind.", TRUE, ch, 0, victim, TO_ROOM);
        act("$n scrambles your brains like eggs.", TRUE, ch, 0, victim,
            TO_VICT);
        act("You blast $N's mind with a psionic blast of energy!", FALSE,
            ch, 0, victim, TO_CHAR);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);

        GET_MANA(ch) -= 25;
        level = GET_LEVEL(ch, PSI_LEVEL_IND);
        if (level > 0) {
            potency = 1;
        }
        if (level > 1) {
            potency++;
        }
        if (level > 4) {
            potency++;
        }
        if (level > 7) {
            potency++;
        }
        if (level > 10) {
            potency++;
        }
        if (level > 20) {
            potency += 2;
        }
        if (level > 30) {
            potency += 2;
        }
        if (level > 40) {
            potency += 2;
        }
        if (level > 49) {
            potency += 2;
        }
        if (level > 50) {
            potency += 2;
        }
        if (GetMaxLevel(ch) > 57) {
            potency = 17;
        }
        if ((potency < 14) && (number(1, 50) < GetMaxLevel(victim))) {
            potency--;
        }
        switch (potency) {
        case 0:
            dam = 1;
            break;
        case 1:
            dam = dice(2, 4);
            break;
        case 2:
            dam = dice(2, 5);
            break;
        case 3:
            dam = dice(3, 5);
            break;
        case 4:
            dam = dice(4, 5);
            break;
        case 5:
            dam = 25;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            break;
        case 6:
            dam = 25;
            break;
        case 7:
            dam = 40;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_POS(victim) > POSITION_STUNNED) {
                GET_POS(victim) = POSITION_STUNNED;
            }
            break;
        case 8:
            dam = 50;
            break;
        case 9:
            dam = 80;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_POS(victim) > POSITION_STUNNED) {
                GET_POS(victim) = POSITION_STUNNED;
            }
            if (GET_HITROLL(victim) > -50) {
                af.type = SKILL_PSIONIC_BLAST;
                af.duration = 5;
                af.modifier = -5;
                af.location = APPLY_HITROLL;
                af.bitvector = 0;
                affect_join(victim, &af, FALSE, FALSE);
            }
            break;
        case 10:
            dam = 80;
            break;
        case 11:
            dam = 100;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_POS(victim) > POSITION_STUNNED) {
                GET_POS(victim) = POSITION_STUNNED;
            }
            if (GET_HITROLL(victim) > -50) {
                af.type = SKILL_PSIONIC_BLAST;
                af.duration = 5;
                af.modifier = -10;
                af.location = APPLY_HITROLL;
                af.bitvector = 0;
                affect_join(victim, &af, FALSE, FALSE);
            }
            break;
        case 12:
            dam = 150;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_HITROLL(victim) > -50) {
                af.type = SKILL_PSIONIC_BLAST;
                af.duration = 5;
                af.modifier = -10;
                af.location = APPLY_HITROLL;
                af.bitvector = 0;
                affect_join(victim, &af, FALSE, FALSE);
            }
            break;
        case 13:
            dam = 200;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_HITROLL(victim) > -50) {
                af.type = SKILL_PSIONIC_BLAST;
                af.duration = 5;
                af.modifier = -10;
                af.location = APPLY_HITROLL;
                af.bitvector = 0;
                affect_join(victim, &af, FALSE, FALSE);
            }
            if (GET_POS(victim) > POSITION_STUNNED) {
                GET_POS(victim) = POSITION_STUNNED;
            }
            if ((!IsImmune(victim, IMM_HOLD)) &&
                (!IS_AFFECTED(victim, AFF_PARALYSIS))) {
                af.type = SPELL_PARALYSIS;
                af.duration = level;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = AFF_PARALYSIS;
                affect_join(victim, &af, FALSE, FALSE);
            }
            break;
        case 14:
        case 15:
        case 16:
        case 17:
            dam = 200;
            if (!IS_AFFECTED(victim, AFF_BLIND)) {
                af.type = SPELL_BLINDNESS;
                af.duration = 5;
                af.modifier = -4;
                af.location = APPLY_HITROLL;
                af.bitvector = AFF_BLIND;
                affect_to_char(victim, &af);
                af.location = APPLY_AC;
                af.modifier = 20;
                affect_to_char(victim, &af);
            }
            if (GET_HITROLL(victim) > -50) {
                af.type = SKILL_PSIONIC_BLAST;
                af.duration = 5;
                af.modifier = -5;
                af.location = APPLY_HITROLL;
                af.bitvector = 0;
                affect_join(victim, &af, FALSE, FALSE);
            }

            if (GET_POS(victim) > POSITION_STUNNED) {
                GET_POS(victim) = POSITION_STUNNED;
            }
            af.type = SPELL_PARALYSIS;
            af.duration = 100;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARALYSIS;
            affect_join(victim, &af, FALSE, FALSE);
            send_to_char("Your brain is turned to jelly!\n\r", victim);

            act("You turn $N's brain to jelly!", FALSE, ch, 0, victim,
                TO_CHAR);
            break;
        }
    }

    if (GET_EXP(ch) > 200000000 || IS_IMMORTAL(ch) ||
        IS_SET(ch->specials.act, PLR_LEGEND)) {
        sprintf(buf, "You do $c0015%d$c0007 damage", dam);
        act(buf, FALSE, ch, NULL, NULL, TO_CHAR);
    }

    if (!damage(ch, victim, dam, SKILL_PSIONIC_BLAST)) {
#if 0
        if (GET_POS(victim) == POSITION_DEAD)   /* never get here */
            act("$n screams in pain as their head explodes!", FALSE,
                victim, 0, 0, TO_ROOM);
#endif

        if (!ch->specials.fighting) {
            set_fighting(ch, victim);
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_hypnosis(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *victim;
    struct affected_type af;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You're not capable of this.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_HYPNOSIS].learned) {
        send_to_char("You haven't learned the proper technique to do "
                     "this.\n\r", ch);
        return;
    }

    if (check_peaceful(ch, 
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    only_argument(argument, target_name);
    victim = get_char_room_vis(ch, target_name);

    if (!victim) {
        send_to_char("There's no one here by that name.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("You do whatever you say.\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(victim)) {
        send_to_char("Pah! You do not think that would be a very good "
                     "idea!\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 25 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("Your mind needs a rest.\n\r", ch);
        return;
    }

    if (circle_follow(victim, ch)) {
        send_to_char("Sorry, no following in circles.\n\r", ch);
        return;
    }

    if (victim->tmpabilities.intel < 8) {
        send_to_char("You'd be wasting your time on such a stupid creature.\n",
                     ch);
        return;
    }

    if (ch->skills[SKILL_HYPNOSIS].learned < number(1, 101)) {
        send_to_char("Your attempt at hypnosis was laughable.\n\r", ch);
        act("$n looks into the eyes of $N, $n looks sleepy!", FALSE, ch, 0,
            victim, TO_ROOM);
        GET_MANA(ch) -= 12;
        LearnFromMistake(ch, SKILL_HYPNOSIS, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    }

    /*
     * Steve's easy level check addition 
     */
    if (GetMaxLevel(victim) > GetMaxLevel(ch)) {
        send_to_char("You'd probably just get your head smashed in.\n\r", ch);
        GET_MANA(ch) -= 12;
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        return;
    }

    if (IS_IMMORTAL(victim)) {
        send_to_char("You could not hypnotize this person.\n\r", ch);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        return;
    }

    if (saves_spell(victim, SAVING_SPELL) ||
        (IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM))) {
        send_to_char("You could not hypnotize this person.\n\r", ch);
        GET_MANA(ch) -= 25;
        FailCharm(victim, ch);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        return;
    }

    GET_MANA(ch) -= 25;

    act("$n hypnotizes $N!", TRUE, ch, 0, victim, TO_ROOM);
    act("You hypnotize $N!", TRUE, ch, 0, victim, TO_CHAR);
    if (IS_PC(victim)) {
        act("$n hypotizes you!", TRUE, ch, 0, victim, TO_VICT);
    }
    add_follower(victim, ch);
    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(victim->specials.act, ACT_SENTINEL)) {
        SET_BIT(victim->specials.act, ACT_SENTINEL);
    }
    af.type = SPELL_CHARM_MONSTER;
    af.duration = 36;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);

    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void do_scry(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *target;
    int             location,
                    old_location;
    struct room_data *rp;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Your mind is not developed enough to do this\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_SCRY].learned) {
        send_to_char("You have not trained your mind to do this\n\r", ch);
        return;
    }

    only_argument(argument, target_name);
    if (!(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    old_location = ch->in_room;
    location = target->in_room;
    rp = real_roomp(location);

    if (IS_IMMORTAL(target) || !rp ||
        IS_SET(rp->room_flags, PRIVATE | NO_MAGIC)) {
        send_to_char("Your mind is not yet strong enough.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 20 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You have a headache. Better rest before you try this "
                     "again.\n\r", ch);
    } else if (dice(1, 101) > ch->skills[SKILL_SCRY].learned) {
        send_to_char("You cannot open a window at this time.\n\r", ch);
        GET_MANA(ch) -= 10;
        LearnFromMistake(ch, SKILL_SCRY, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    } else {
        GET_MANA(ch) -= 20;
        send_to_char("You close your eyes and envision your target.\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, location);
        do_look(ch, "", 15);
        char_from_room(ch);
        char_to_room(ch, old_location);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

void do_invisibililty(struct char_data *ch, char *argument, int cmd)
{
    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Get a mage if you want to go Invisible!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_INVIS].learned)) {
        send_to_char("You are unable to bend light.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 10 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You don't have enough mental power to hide yourself.\n\r",
                     ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_INVISIBLE)) {
        send_to_char("You're already invisible.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_INVIS].learned < number(1, 101)) {
        send_to_char("You cannot seem to bend light right now.\n\r", ch);
        act("$n fades from view briefly.", FALSE, ch, 0, 0, TO_ROOM);
        GET_MANA(ch) -= 5;
        LearnFromMistake(ch, SKILL_INVIS, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    GET_MANA(ch) -= 10;
    spell_invisibility(GET_LEVEL(ch, PSI_LEVEL_IND), ch, ch, 0);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
    WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_adrenalize(struct char_data *ch, char *argument, int cmd)
{
    char            target_name[140];
    struct char_data *target;
    struct affected_type af;
    char            strength;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You're no psionicist!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_ADRENALIZE].learned)) {
        send_to_char("You don't know how to energize people.\n\r", ch);
        return;
    }

    only_argument(argument, target_name);
    if (!(target = get_char_room_vis(ch, target_name))) {
        send_to_char("You can't seem to find that person anywhere.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 15 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You don't have the mental power to do this.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_ADRENALIZE].learned < dice(1, 101)) {
        send_to_char("You've failed your attempt.\n\r", ch);
        act("$n touches $N's head lightly, then sighs.", FALSE, ch, 0,
            target, TO_ROOM);
        GET_MANA(ch) -= 7;
        LearnFromMistake(ch, SKILL_ADRENALIZE, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    if (affected_by_spell(target, SKILL_ADRENALIZE)) {
        send_to_char("This person was already adrenalized!\n\r", ch);
        GET_MANA(ch) -= 15;
        return;
    }

    strength = 1 + (GET_LEVEL(ch, PSI_LEVEL_IND) / 10);
    if (strength > 4) {
        strength = 4;   
    }
    af.type = SKILL_ADRENALIZE;
    af.location = APPLY_HITROLL;
    af.modifier = -strength;
    af.duration = 5;
    af.bitvector = 0;
    affect_to_char(target, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = strength;
    affect_to_char(target, &af);

    af.location = APPLY_AC;
    af.modifier = 20;
    affect_to_char(target, &af);

    GET_MANA(ch) -= 15;
    if (ch == target) {
        act("You excite the chemicals in your body!", FALSE, ch, 0, 0,
            TO_CHAR);
        act("$n touches $mself lightly on the forehead.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n suddenly gets a wild look in $s eyes!", FALSE, ch, 0, 0,
            TO_ROOM);
    } else {
        act("You excite the chemicals in $N's body!", TRUE, ch, 0, target,
            TO_CHAR);
        act("$n touches $N lightly on the forehead.", TRUE, ch, 0, target,
            TO_NOTVICT);
        act("$N suddenly gets a wild look in $S eyes!", TRUE, ch, 0,
            target, TO_NOTVICT);
        act("$n touches you on the forehead lightly, you feel energy ulimited!",
            TRUE, ch, 0, target, TO_VICT);
    }
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
}

void do_meditate(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You can't stand sitting down and waiting like this.\n\r",
                     ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_MEDITATE].learned)) {
        send_to_char("You haven't yet learned to clear your mind.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_MEDITATE].learned < dice(1, 101)) {
        send_to_char("You can't clear your mind at this time.\n\r", ch);
        LearnFromMistake(ch, SKILL_MEDITATE, 0, 95);
        return;
    }

    if (ch->specials.conditions[FULL] == 0 || 
        ch->specials.conditions[THIRST] == 0 || 
        ch->specials.conditions[DRUNK] > 0) {
        send_to_char("Your body has certain needs that have to be met before "
                     "you can meditate.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SKILL_MEDITATE)) {
        send_to_char("Your mind is already prepared to meditate, so rest and "
                     "become one with nature.\n\r", ch);
        return;
    }

    ch->specials.position = POSITION_RESTING;
    send_to_char("You sit down and start resting and clear your mind of all "
                 "thoughts.\n\r", ch);
    act("$n sits down and begins humming,'Oooommmm... Ooooommmm.'", TRUE,
        ch, 0, 0, TO_ROOM);
    af.type = SKILL_MEDITATE;
    af.location = 0;
    af.modifier = 0;
    af.duration = 2;
    af.bitvector = 0;
    affect_to_char(ch, &af);
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

void do_heroic_rescue(struct char_data *ch, char *arguement, int command)
{
    struct char_data *dude,
                   *enemy;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PALADIN)) {
        send_to_char("You're not a holy warrior!\n\r", ch);
        return;
    }

    if (check_peaceful(ch, "But who would need to be rescued in such a "
                           "peaceful place?\n\r")) {
        return;
    }

    for (dude = real_roomp(ch->in_room)->people;
         dude && !(dude->specials.fighting); dude = dude->next_in_room) {
        /* 
         * Empty loop 
         */
    }

    if (!dude) {
        send_to_char("But there is no battle here!?!?\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_HEROIC_RESCUE].learned < number(1, 101)) {
        send_to_char("You try to plow your way to the front of the battle but "
                     "stumble.\n\r", ch);
        LearnFromMistake(ch, SKILL_HEROIC_RESCUE, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (dude->specials.fighting && dude->specials.fighting != ch && 
            ch->specials.fighting != dude && dude != ch && 
            IS_FOLLOWING(ch, dude)) {
            enemy = dude->specials.fighting;

            act("$n leaps heroically to the front of the battle!", FALSE, ch, 
                0, 0, TO_ROOM);
            send_to_char("CHARGE!! You streak to the front of the battle like "
                         "a runaway train!\n\r", ch);
            act("$n leaps to your rescue, you are confused!", TRUE, ch,
                0, dude, TO_VICT);
            act("You rescue $N!", TRUE, ch, 0, dude, TO_CHAR);
            act("$n rescues $N!", TRUE, ch, 0, dude, TO_NOTVICT);
            stop_fighting(dude);
            stop_fighting(enemy);
            set_fighting(enemy, ch);

            if (GET_ALIGNMENT(dude) >= 350) {
                GET_ALIGNMENT(ch) += 10;
            }
            if (GET_ALIGNMENT(dude) >= 950) {
                GET_ALIGNMENT(ch) += 10;
            }
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);

            WAIT_STATE(dude, 2 * PULSE_VIOLENCE);
            return;
        }
    }

    send_to_char("You can't seem to figure out whom to rescue!\n\r", ch);
}

void do_blessing(struct char_data *ch, char *argument, int cmd)
{
    int             rating,
                    factor,
                    level;
    struct char_data *test,
                   *dude;
    struct affected_type af;
    char            dude_name[140];

    if (!ch->skills) {
        return;
    }
    only_argument(argument, dude_name);

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PALADIN)) {
        send_to_char("I bet you think you are a paladin, don't you?\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < GET_LEVEL(ch, PALADIN_LEVEL_IND) * 2) {
        send_to_char("You haven't the spiritual resources to do that now.\n\r",
                     ch);
        return;
    }

    if (affected_by_spell(ch, SKILL_BLESSING)) {
        send_to_char("You can only request a blessing from your deity once "
                     "every 3 days.\n\r", ch);
        return;
    }

    if (number(1, 101) > ch->skills[SKILL_BLESSING].learned) {
        send_to_char("You fail to bestow your god's blessing.\n\r", ch);
        GET_MANA(ch) -= GET_LEVEL(ch, PALADIN_LEVEL_IND);
        LearnFromMistake(ch, SKILL_BLESSING, 0, 95);
        return;
    }

    if (!(dude = get_char_room_vis(ch, dude_name))) {
        send_to_char("WHO do you wish to bless?\n\r", ch);
        return;
    }

    GET_MANA(ch) -= GET_LEVEL(ch, PALADIN_LEVEL_IND) * 2;
    factor = 0;
    if (ch == dude) {
        factor++;
    }
    if (dude->specials.alignment > 350) {
        factor++;
    }
    if (dude->specials.alignment == 1000) {
        factor++;
    }

    level = GET_LEVEL(ch, PALADIN_LEVEL_IND);
    rating = (int) ((level) * (GET_ALIGNMENT(ch)) / 1000) + factor;
    factor = 0;
    for (test = real_roomp(ch->in_room)->people; test; test = test->next) {
        if (test != ch) {
            if (ch->master) {
                if (circle_follow(ch->master, test)) {
                    factor++;
                }
            } else if (circle_follow(ch, test)) {
                factor++;
            }
        }
    }

    rating += MIN(factor, 3);
    if (rating < 0) {
        send_to_char("You are so despised by your god that he punishes "
                     "you!\n\r", ch);
        spell_blindness(level, ch, ch, 0);
        spell_paralyze(level, ch, ch, 0);
        return;
    }
    if (rating == 0) {
        send_to_char("There's no one in your group to bless", ch);
        return;
    }
    if (!(affected_by_spell(dude, SPELL_BLESS))) {
        spell_bless(level, ch, dude, 0);
    }
    if (rating > 1 && !(affected_by_spell(dude, SPELL_ARMOR))) {
        spell_armor(level, ch, dude, 0);
    }
    if (rating > 4 && !(affected_by_spell(dude, SPELL_STRENGTH))) {
        spell_strength(level, ch, dude, 0);
    }
    if (rating > 6) {
        spell_second_wind(level, ch, dude, 0);
    }
    if (rating > 9 && !(affected_by_spell(dude, SPELL_SENSE_LIFE))) {
        spell_sense_life(level, ch, dude, 0);
    }
    if (rating > 14 && !(affected_by_spell(dude, SPELL_TRUE_SIGHT))) {
        spell_true_seeing(level, ch, dude, 0);
    }
    if (rating > 19) {
        spell_cure_critic(level, ch, dude, 0);
    }
    if (rating > 24 && !(affected_by_spell(dude, SPELL_SANCTUARY))) {
            spell_sanctuary(level, ch, dude, 0);
    }
    if (rating > 29) {
        spell_heal(level, ch, dude, 0);
    }
    if (rating > 34) {
        spell_remove_poison(level, ch, dude, 0);
        spell_remove_paralysis(level, ch, dude, 0);
    }
    if (rating > 39) {
        spell_heal(level, ch, dude, 0);
    }
    if (rating > 44) {
        if (dude->specials.conditions[FULL] != -1) {
            dude->specials.conditions[FULL] = 24;
        }
        if (dude->specials.conditions[THIRST] != -1) {
            dude->specials.conditions[THIRST] = 24;
        }
    }
    if (rating > 54) {
        spell_heal(level, ch, dude, 0);
        send_to_char("An awesome feeling of holy power overcomes you!\n\r",
                     dude);
    }
    act("$n asks $s deity to bless $N!", TRUE, ch, 0, dude, TO_NOTVICT);
    act("You pray for a blessing on $N!", TRUE, ch, 0, dude, TO_CHAR);
    act("$n's deity blesses you!", TRUE, ch, 0, dude, TO_VICT);

    af.type = SKILL_BLESSING;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.duration = 24 * 3;
    affect_to_char(ch, &af);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
}

void do_lay_on_hands(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct affected_type af;
    int             wounds,
                    healing;
    char            victim_name[240];

    if (!ch->skills) {
        return;
    }
    only_argument(argument, victim_name);

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PALADIN)) {
        send_to_char("You are not a holy warrior!\n\r", ch);
        return;
    }
    if (!(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Your hands cannot reach that person\n\r", ch);
        return;
    }
    if (affected_by_spell(ch, SKILL_LAY_ON_HANDS)) {
        send_to_char("You have already healed once today.\n\r", ch);
        return;
    }
    wounds = GET_MAX_HIT(victim) - GET_HIT(victim);
    if (!wounds) {
        send_to_char("Don't try to heal what ain't hurt!\n\r", ch);
        return;
    }
    if (ch->skills[SKILL_LAY_ON_HANDS].learned < number(1, 101)) {
        send_to_char("You cannot seem to call on your deity right now.\n\r",
                     ch);
        LearnFromMistake(ch, SKILL_LAY_ON_HANDS, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }
    act("$n lays hands on $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    act("You lay hands on $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n lays hands on you.", FALSE, ch, 0, victim, TO_VICT);

    if (GET_ALIGNMENT(victim) < 0) {
        act("You are too evil to benefit from this treatment.", FALSE, ch,
            0, victim, TO_VICT);
        act("$n is too evil to benefit from this treatment.", FALSE,
            victim, 0, ch, TO_ROOM);
        return;
    }
    if (GET_ALIGNMENT(victim) < 350) {
        /* 
         * should never be since they get converted after 349 
         */
        healing = GET_LEVEL(ch, PALADIN_LEVEL_IND);
    } else {
        healing = GET_LEVEL(ch, PALADIN_LEVEL_IND) * 2;
    }

    if (healing > wounds) {
        GET_HIT(victim) = GET_MAX_HIT(victim);
    } else {
        GET_HIT(victim) += healing;
    }
    af.type = SKILL_LAY_ON_HANDS;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    af.duration = 24;
    affect_to_char(ch, &af);
    WAIT_STATE(ch, PULSE_VIOLENCE);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
}

void do_holy_warcry(struct char_data *ch, char *argument, int cmd)
{
    char            name[140];
    int             dam,
                    dif,
                    level;
    struct char_data *dude;

    if (!ch->skills) {
        return;
    }
    only_argument(argument, name);
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PALADIN)) {
        send_to_char("Your feeble attempt at a war cry makes your victim "
                     "laugh at you.\n\r", ch);
        return;
    }
    if (GET_ALIGNMENT(ch) < 350) {
        send_to_char("You're too ashamed of your behavior to warcry.\n\r",
                     ch);
        return;
    }
    if (check_peaceful(ch, "You warcry is completely silenced by the "
                           "tranquility of this room.\n\r")) {
        return;
    }
    if (ch->specials.fighting) {
        dude = ch->specials.fighting;
    } else if (!(dude = get_char_room_vis(ch, name))) {
        send_to_char("You bellow at the top of your lungs, to bad your victim"
                     " wasn't here to hear it.\n\r", ch);
        return;
    }
    if (IS_IMMORTAL(dude)) {
        send_to_char("The gods are not impressed by people shouting at them.\n",
                     ch);
        return;
    }
    if (ch->skills[SKILL_HOLY_WARCRY].learned < number(1, 101)) {
        send_to_char("Your mighty warcry emerges from your throat as a tiny "
                     "squeak.\n\r", ch);
        LearnFromMistake(ch, SKILL_HOLY_WARCRY, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        set_fighting(dude, ch);
    } else {
        if (IS_PC(dude)) {
            act("$n surprises you with a painful warcry!", FALSE, ch, 0,
                dude, TO_VICT);
        }

        dif = (level = GET_LEVEL(ch, PALADIN_LEVEL_IND) - GetMaxLevel(dude));
        if (dif > 19) {
            spell_paralyze(0, ch, dude, 0);
            dam = (int) (level * 2.5);
        } else if (dif > 14) {
            dam = (int) (level * 2.5);
        } else if (dif > 10) {
            dam = (int) (level * 2);
        } else if (dif > 6) {
            dam = (int) (level * 1.5);
        } else if (dif > -6) {
            dam = (int) (level);
        } else if (dif > -11) {
            dam = (int) (level * .5);
        } else {
            dam = 0;
        }
        if (saves_spell(dude, SAVING_SPELL)) {
            dam /= 2;
        }
        act("You are attacked by $n who shouts a heroic warcry!", TRUE, ch,
            0, dude, TO_VICT);
        act("$n screams a warcry at $N with a tremendous fury!", TRUE, ch,
            0, dude, TO_ROOM);
        act("You fly into battle with $N as you emit a holy warcry!", TRUE,
            ch, 0, dude, TO_CHAR);
        if (!damage(ch, dude, dam, SKILL_HOLY_WARCRY) && 
            !ch->specials.fighting) {
                set_fighting(ch, dude);
        }

        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}

void do_psi_shield(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You do not have the mental power to bring forth a "
                     "shield!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_PSI_SHIELD].learned)) {
        send_to_char("You are unable to use this skill.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 10 && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You don't have enough mental power to protect "
                     "yourself.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SKILL_PSI_SHIELD)) {
        send_to_char("You're already protected.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_PSI_SHIELD].learned < number(1, 101)) {
        send_to_char("You failed to bring forth the protective shield.\n\r", 
                     ch);
        GET_MANA(ch) -= 5;
        LearnFromMistake(ch, SKILL_PSI_SHIELD, 0, 95);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
        return;
    }

    GET_MANA(ch) -= 10;
    act("$n summons a protective shield about $s body!", FALSE, ch, 0, 0,
        TO_ROOM);
    act("You erect a protective shield about your body.", FALSE, ch, 0, 0,
        TO_CHAR);
    af.type = SKILL_PSI_SHIELD;
    af.location = APPLY_AC;
    af.modifier = ((int) GetMaxLevel(ch) / 10) * -10;
    af.duration = GetMaxLevel(ch);
    af.bitvector = 0;
    affect_to_char(ch, &af);

    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_esp(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;

    if (!ch->skills) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You do not have the mental power to do this!\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_ESP].learned)) {
        send_to_char("You are unable to use this skill.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) < 10) && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You don't have enough mental power to do that.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SKILL_ESP)) {
        send_to_char("You're already listening to others thoughts.\n\r", ch);
        return;
    }

    if (ch->skills[SKILL_ESP].learned < number(1, 101)) {
        send_to_char("You failed open you mind to read others thoughts.\n\r",
                     ch);
        GET_MANA(ch) -= 5;
        LearnFromMistake(ch, SKILL_ESP, 0, 95);
        return;
    }

    GET_MANA(ch) -= 10;
    act("You open your mind to read others thoughts.", FALSE, ch, 0, 0,
        TO_CHAR);
    af.type = SKILL_ESP;
    af.location = APPLY_NONE;
    af.modifier = 0;
    af.duration = (int) GetMaxLevel(ch) / 2;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void do_sending(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *target;
    int             skill_check = 0;
    char            target_name[140],
                    buf[1024],
                    message[MAX_INPUT_LENGTH + 20];

    if (!ch->skills) {
        return;
    }
    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SPELL_SENDING].learned &&
        !ch->skills[SPELL_MESSENGER].learned) {
        send_to_char("You are unable to use this skill.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) < 5) && GetMaxLevel(ch) < LOW_IMMORTAL) {
        send_to_char("You don't have the power to do that.\n\r", ch);
        return;
    }

    if (ch->skills[SPELL_SENDING].learned >
        ch->skills[SPELL_MESSENGER].learned) {
        skill_check = ch->skills[SPELL_SENDING].learned;
    } else {
        skill_check = ch->skills[SPELL_MESSENGER].learned;
    }

    if (skill_check < number(1, 101)) {
        send_to_char("You fumble and screw up the spell.\n\r", ch);
        if (GetMaxLevel(ch) < LOW_IMMORTAL) {
            GET_MANA(ch) -= 3;
        }
        if (ch->skills[SPELL_SENDING].learned >
            ch->skills[SPELL_MESSENGER].learned) {
            LearnFromMistake(ch, SPELL_SENDING, 0, 95);
        } else {
            LearnFromMistake(ch, SPELL_MESSENGER, 0, 95);
        }
        return;
    }

    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
        GET_MANA(ch) -= 5;
    }
    half_chop(argument, target_name, message);
    if (!(target = get_char_vis_world(ch, target_name, NULL))) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if (IS_NPC(target) && !IS_SET(target->specials.act, ACT_POLYSELF)) {
        send_to_char("You can't sense that person anywhere.\n\r", ch);
        return;
    }

    if (check_soundproof(target)) {
        send_to_char("In a silenced room, try again later.\n\r", ch);
        return;
    }

    if (IS_SET(target->specials.act, PLR_NOTELL)) {
        send_to_char("They are ignoring messages at this time.\n\r", ch);
        return;
    }

    sprintf(buf, "$n sends you a mystic message:$c0013 %s", message);
    act(buf, TRUE, ch, 0, target, TO_VICT);
    sprintf(buf, "You send $N%s the message:$c0013 %s",
            (IS_AFFECTED2(target, AFF2_AFK) ? " (Who is AFK)" : ""), message);
    act(buf, TRUE, ch, 0, target, TO_CHAR);
}

/*
 * BREW for mages, done by Greg Hovey..
 */


void do_scribe(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH];
    char            arg[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int             sn = -1,
                    x,
                    index,
                    formula = 0,
                    qend;

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

    if (GET_MANA(ch) < 50 && GetMaxLevel(ch) <= 50) {
        send_to_char("You don't have enough mana to scribe that spell.\n\r",
                     ch);
        return;
    }

    if (!(obj = read_object(EMPTY_SCROLL, VIRTUAL))) {
        Log("no default scroll could be found for scribe");
        send_to_char("woops, something's wrong.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);

    /*
     * Check for beginning quote 
     */
    if (*argument != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    /*
     * Locate the last quote && lowercase the magic words (if any) 
     */
    for (qend = 1; argument[qend] && argument[qend] != '\''; qend++)
        argument[qend] = LOWER(argument[qend]);

    if (argument[qend] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    sn = old_search_block(argument, 1, qend - 1, spells, 0);
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

        if (GetMaxLevel(ch) > 50) {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = 50;
            
            /* 
             * set ego to level.
             */
            obj->level = 50;
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

void do_brew(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH];
    struct obj_data *obj;
    int             sn = -1,
                    index,
                    formula = 0,
                    qend;

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!MainClass(ch, MAGE_LEVEL_IND) && !IS_IMMORTAL(ch)) {
        send_to_char("Alas, you can only dream of brewing your own "
                     "potions.\n\r", ch);
        return;
    }

    if (!ch->skills) {
        send_to_char("You don't seem to have any skills.\n\r", ch);
        return;
    }

    if (!(ch->skills[SKILL_BREW].learned)) {
        send_to_char("You haven't been properly trained in the art of "
                     "brewing.\n\r", ch);
        return;
    }

    if (!argument) {
        send_to_char("What would you like to brew?\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 50 && GetMaxLevel(ch) <= 50) {
        send_to_char("You don't have enough mana to brew that spell.\n\r", ch);
        return;
    }

    if (!(obj = read_object(EMPTY_POTION, VIRTUAL))) {
        Log("no default potion could be found for brew");
        send_to_char("woops, something's wrong.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);

    /*
     * Check for beginning qoute 
     */
    if (*argument != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    /*
     * Locate the last quote && lowercase the magic words (if any) 
     */
    for (qend = 1; argument[qend] && argument[qend] != '\''; qend++)
        argument[qend] = LOWER(argument[qend]);
    if (argument[qend] != '\'') {
        send_to_char("Magic must always be enclosed by the holy magic symbols :"
                     " '\n\r", ch);
        return;
    }

    sn = old_search_block(argument, 1, qend - 1, spells, 0);
    sn = sn - 1;

    if (sn == -1) {
        /* 
         * no spell found?
         */
        send_to_char("Brew what??.\n\r", ch);
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

    if (!(spell_info[index].min_level_magic)) {
        /*
         * is it a mage spell?
         */
        send_to_char("You cannot concoct that spell!\n\r", ch);
        return;
    }

    if (spell_info[index].brewable == 0 && !IS_IMMORTAL(ch)) {
        send_to_char("You can't brew that spell.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < spell_info[index].min_usesmana * 2 && 
        !IS_IMMORTAL(ch)) {
        send_to_char("You don't have enough mana to brew that spell.\n\r", ch);
        return;
    }

    act("$n begins preparing a potion.", TRUE, ch, obj, NULL, TO_ROOM);
    act("You start preparing a potion.", TRUE, ch, obj, NULL, TO_CHAR);

    formula = ((ch->skills[SKILL_BREW].learned) + GET_INT(ch) / 3) +
              (GET_WIS(ch) / 3);

    if (formula > 98) {
        formula = 98;
    }
    if ((number(1, 101) >= formula || ch->skills[SKILL_BREW].learned < 10) &&
        !IS_IMMORTAL(ch)) {
        WAIT_STATE(ch, PULSE_VIOLENCE * 5);
        act("$p explodes violently!", TRUE, ch, obj, NULL, TO_CHAR);
        act("$p explodes violently!", TRUE, ch, obj, NULL, TO_ROOM);
        GET_HIT(ch) -= spell_info[index].min_level_magic;
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        act("$n screams in pain as $p exploded on $m.", TRUE, ch, obj,
            NULL, TO_ROOM);
        act("You scream in pain as $p explodes.", TRUE, ch, obj, NULL, TO_CHAR);
        LearnFromMistake(ch, SKILL_BREW, 0, 90);
        extract_obj(obj);
    } else {
        GET_MANA(ch) -= spell_info[index].min_usesmana * 2;
        sprintf(buf, "You have imbued a new spell to %s.\n\r",
                obj->short_description);
        send_to_char(buf, ch);
        send_to_char("The brew was a success!\n\r", ch);

        if (obj->short_description) {
            free(obj->short_description);
        }
        sprintf(buf, "a potion of %s", spells[sn]);
        obj->short_description = (char *) strdup(buf);

        if (obj->name) {
            free(obj->name);
        }
        sprintf(buf, "potion %s", spells[sn]);
        obj->name = (char *) strdup(buf);

        if (obj->description) {
            free(obj->description);
        }
        sprintf(buf, "%s", "A weird coloured potion is on the ground.");
        obj->description = (char *) strdup(buf);

        if (GetMaxLevel(ch) > 50) {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = 50;
        } else {
            /* 
             * set spell level.
             */
            obj->obj_flags.value[0] = GetMaxLevel(ch);
        }

        /*
         * set spell in slot.
         */
        obj->obj_flags.value[1] = sn + 1;
        obj->obj_flags.timer = 60;

        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        obj_to_char(obj, ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    }
}


/*
 * New paladin charge skill.  Must be mounted.. Basically backstab code
 * @usage : charge <target> 
 * @date : May 14, 2002 @author: Greg Hovey (Banon)
 */

void do_charge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char            name[256];
    byte            percent;

    dlog("in do_charge");

    if (!ch->skills) {
        return;
    }
    if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n\r")) {
        return;
    }
    only_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("charge who?\n\r", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("How can you charge yourself?\n\r", ch);
        return;
    }
    if (!HasClass(ch, CLASS_PALADIN)) {
        send_to_char("You're no paladin!\n\r", ch);
        return;
    }
    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon, to make it a success.\n\r",
                     ch);
        return;
    }
    if (!MOUNTED(ch)) {
        send_to_char("You need a mount to that.\n", ch);
        return;
    }
    if (ch->attackers) {
        send_to_char("There's no way to reach that charge while you're "
                     "fighting!\n\r", ch);
        return;
    }
    if (victim->attackers >= 3) {
        send_to_char("There is to many people in the way to do that!\n\r", ch);
        return;
    }
    if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE) && 
        !IsGiant(ch)) {
        act("$N is too big to charge at", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }
    if (ch->equipment[WIELD]->obj_flags.value[3] != 13) {
        /* 
         * TYPE_IMPALE 
         */
        send_to_char("Only true and honourable lances should be used for "
                     "charging.\n\r", ch);
        return;
    }
    if (ch->specials.fighting) {
        send_to_char("You're too busy to charge on them\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);   

    if (ch->skills[SKILL_CHARGE].learned) {
        if (percent > ch->skills[SKILL_CHARGE].learned) {
            send_to_char("You totally miss your target!\n\r", ch);
            act("$n charges at $N on his steed and totally misses $M.",
                FALSE, ch, 0, victim, TO_ROOM);
            LearnFromMistake(ch, SKILL_CHARGE, 0, 95);
        } else {
            GET_HITROLL(ch) += 100;
            act("You charge agressively at $N on your sturdy steed.\n\r",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n charges agressively forward on his steed at $N", FALSE,
                ch, 0, victim, TO_ROOM);
#if 0                
            AddHated(victim, ch);
#endif            
            damage(ch, victim, GET_DAMROLL(ch) * 6, SKILL_CHARGE);
#if 0            
            hit(ch,victim,SKILL_CHARGE);
#endif            
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);

            GET_HITROLL(ch) -= 100;
        }
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

/*
 * Paladin calls a steed skill -Lennya 
 */
void do_steed(struct char_data *ch, char *argument, int cmd)
{
    int             percent = 0;
    int             mhit = 1;
    struct char_data *steed;

    dlog("in do_steed");

    if (!HasClass(ch, CLASS_PALADIN)) {
        send_to_char("The Guardians of Law laugh at your feeble request!\n\r",
                     ch);
        return;
    }
    if (!ch->skills) {
        Log("Char without skills trying to call a steed");
        return;
    }
    if (!ch->skills[SKILL_STEED].learned) {
        send_to_char("The Guardians of Righteousness do not see fit to grant "
                     "you a mount just yet.\n\r", ch);
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("Call a mount indoors? Where's your manners!\n\r", ch);
        return;
    }
    if (ch->specials.fighting) {
        send_to_char("Not while yer fighting, lad!\n\r", ch);
        return;
    }
    if (MOUNTED(ch)) {
        send_to_char("Hey now, you're already riding a courageous steed!\n\r",
                     ch);
        return;
    }
    percent = number(1, 101);
    if (ch->skills[SKILL_STEED].learned) {
        if (percent > ch->skills[SKILL_STEED].learned) {
            send_to_char("You whistle a lively tune.\n\r", ch);
            act("$n whistles a lively tune.", FALSE, ch, 0, 0, TO_ROOM);
            LearnFromMistake(ch, SKILL_STEED, 0, 95);
        } else {
            if ((steed = read_mobile(STEED_TEMPLATE, VIRTUAL))) {
                act("You whistle loudly.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n whistles loudly.", FALSE, ch, 0, 0, TO_ROOM);
                mhit = GET_HIT(ch);
                mhit *= 2;
                steed->points.max_hit = mhit;
                GET_HIT(steed) = GET_MAX_HIT(steed);

                GET_AC(steed) = GET_AC(ch);
                GET_LEVEL(steed, WARRIOR_LEVEL_IND) =
                    GET_LEVEL(ch, PALADIN_LEVEL_IND) - 10;
                GET_HITROLL(steed) = GET_HITROLL(ch);
                char_to_room(steed, ch->in_room);

                act("Suddenly, $N comes galloping in, and you jump on $S back.",
                    FALSE, ch, 0, steed, TO_CHAR);
                act("Suddenly, $N comes galloping in, and $n jumps on $S back.",
                    FALSE, ch, 0, steed, TO_ROOM);
                MOUNTED(ch) = steed;
                RIDDEN(steed) = ch;
                GET_POS(ch) = POSITION_MOUNTED;
            } else {
                Log("No mount found for paladin's call steed skill");
                send_to_char("Woops, code screwup, can't find your horse. Use "
                             "the bug command.\n\r", ch);
                return;
            }
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
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
                 tch = tch->next_in_room) {
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
