/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "protos.h"

/*
 * extern variables
 */

extern struct descriptor_data *descriptor_list;
extern struct dex_app_type dex_app[];
extern char    *att_kick_hit_room[];
extern char    *att_kick_hit_victim[];
extern char    *att_kick_hit_ch[];
extern char    *att_kick_miss_room[];
extern char    *att_kick_miss_victim[];
extern char    *att_kick_miss_ch[];
extern char    *att_kick_kill_room[];
extern char    *att_kick_kill_victim[];
extern char    *att_kick_kill_ch[];
extern struct char_data *character_list;
extern const char *fight_styles[];
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

extern struct breather breath_monsters[];
extern struct index_data *mob_index;
extern const struct clan clan_list[MAX_CLAN];
extern long     SystemFlags;

extern const struct race_type race_list[];
extern struct str_app_type str_app[];

funcp           bweapons[] = {
    cast_geyser, cast_fire_breath, cast_gas_breath,
    cast_frost_breath, cast_acid_breath,
    cast_lightning_breath
};



void do_hit(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    struct char_data *victim;

    dlog("in do_hit");

    if (check_peaceful(ch,
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (arg) {
        victim = get_char_room_vis(ch, arg);
        if (victim) {
            if (victim == ch) {
                send_to_char("You hit yourself..OUCH!.\n\r", ch);
                act("$n hits $mself, and says OUCH!", FALSE, ch, 0, victim,
                    TO_ROOM);
            } else {
                if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
                    act("$N is just such a good friend, you simply can't "
                        "hit $M.", FALSE, ch, 0, victim, TO_CHAR);
                    return;
                }
                if ((GET_POS(ch) >= POSITION_STANDING) &&
                    (!ch->specials.fighting)) {
                    hit(ch, victim, TYPE_UNDEFINED);
                    WAIT_STATE(ch, PULSE_VIOLENCE + 2);
                } else {
                    if (victim != ch->specials.fighting) {
                        if (ch->skills &&
                            ch->skills[SKILL_SWITCH_OPP].learned) {
                            if (number(1, 101) <
                                ch->skills[SKILL_SWITCH_OPP].learned) {
                                stop_fighting(ch);
                                if (victim->attackers < 5) {
                                    set_fighting(ch, victim);
                                } else {
                                    send_to_char("There's no room to "
                                                 "switch!\n\r", ch);
                                }
                                send_to_char("You switch opponents\n\r", ch);
                                act("$n switches targets", FALSE, ch, 0, 0,
                                    TO_ROOM);
                                WAIT_STATE(ch, PULSE_VIOLENCE + 2);
                            } else {
                                send_to_char("You try to switch opponents, but"
                                             " you become confused!\n\r", ch);
                                stop_fighting(ch);
                                LearnFromMistake(ch, SKILL_SWITCH_OPP, 0, 95);
                                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                            }
                        } else {
                            send_to_char("You do the best you can!\n\r", ch);
                        }
                    } else {
                        send_to_char("You do the best you can!\n\r", ch);
                    }
                }
            }
        } else {
            send_to_char("They aren't here.\n\r", ch);
        }
    } else {
        send_to_char("Hit who?\n\r", ch);
    }
}

void do_kill(struct char_data *ch, char *argument, int cmd)
{
    static char    *arg;
    struct char_data *victim;

    dlog("in do_kill");

    if ((GetMaxLevel(ch) < GREATER_GOD) || IS_NPC(ch)) {
        do_hit(ch, argument, 0);
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Kill who?\n\r", ch);
    } else {
        if (!(victim = get_char_room_vis(ch, arg))) {
            send_to_char("They aren't here.\n\r", ch);
        } else if (ch == victim) {
            send_to_char("Your mother would be so sad.. :(\n\r", ch);
        } else {
            if (GetMaxLevel(ch) < GetMaxLevel(victim)) {
                act("Thou shalt not slay your superiors!", FALSE, ch, 0,
                    victim, TO_CHAR);
                return;
            }
            if (cmd != 1) {
                act("You chop $M to pieces! Ah! The blood!", FALSE, ch, 0,
                    victim, TO_CHAR);
                act("$N chops you to pieces!", FALSE, victim, 0, ch,
                    TO_CHAR);
                act("$n brutally slays $N", FALSE, ch, 0, victim,
                    TO_NOTVICT);
            }
            raw_kill(victim, 0);
        }
    }
}

void do_dismiss(struct char_data *ch, char *arg, int cmd)
{
    int             bits;
    char           *buf,
                    buf2[254];
    struct char_data *tmp_char;
    struct obj_data *dummy;
    struct obj_data *obj;

    if (!ch) {
        return;
    }
    if (cmd != 588) {
        /*
         * dismiss
         */
        return;
    }

    arg = get_argument(arg, &buf);
    if (!buf) {
        send_to_char("Dismiss whom?\n\r", ch);
        return;
    }

    bits = generic_find(buf, FIND_CHAR_ROOM, ch, &tmp_char, &dummy);
    if (tmp_char) {
        if (tmp_char != ch) {
            if (IS_NPC(tmp_char) &&
                !IS_SET(tmp_char->specials.act, ACT_POLYSELF)) {
                if (tmp_char->master == ch &&
                    IS_AFFECTED(tmp_char, AFF_CHARM)) {
                    /*
                     * valid target, let's get rid of it
                     */
                    act("You wave your hand, and dismiss $N from your "
                        "service.", FALSE, ch, 0, tmp_char, TO_CHAR);
                    act("$N wanders back to $S home.", FALSE, ch, 0,
                        tmp_char, TO_CHAR);
                    act("$n waves $s hands, dismissing $N from $s service.",
                        FALSE, ch, 0, tmp_char, TO_NOTVICT);
                    act("$N wanders back to $S home.", FALSE, ch, 0,
                        tmp_char, TO_NOTVICT);
                    act("$n waves $s hands at you, dismissing you from $s "
                        "service.", FALSE, ch, 0, tmp_char, TO_VICT);
                    act("You wander back to your home.", FALSE, ch, 0,
                        tmp_char, TO_VICT);
                    if (tmp_char->points.gold) {
                        obj = create_money(tmp_char->points.gold);
                        obj_to_room(obj, ch->in_room);
                    }
                    extract_char(tmp_char);
                } else {
                    sprintf(buf2, "%s isn't under your control.\n\r",
                            tmp_char->player.short_descr);
                    send_to_char(buf2, ch);
                }
            } else {
                send_to_char("Dismissing other players? I think not!\n\r", ch);
            }
        } else {
            send_to_char("Dismiss yourself? I think not!\n\r", ch);
        }
    } else {
        send_to_char("Dismiss whom?\n\r", ch);
    }
}

void do_order(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                   *message;
    char            buf[256];
    bool            found = FALSE;
    int             org_room;
    struct char_data *victim;
    struct follow_type *k;
    int             check;

    dlog("in do_order");

    if (apply_soundproof(ch)) {
        return;
    }

    argument = get_argument(argument, &name);
    message = skip_spaces(argument);

    if (!name || !message) {
        send_to_char("Order who to do what?\n\r", ch);
    } else if (!(victim = get_char_room_vis(ch, name)) &&
             strcasecmp("follower", name) && strcasecmp("followers", name)) {
        send_to_char("That person isn't here.\n\r", ch);
    } else if (ch == victim) {
        send_to_char("You obviously suffer from Multiple Personality "
                     "Disorder.\n\r", ch);
    } else {
        if (IS_AFFECTED(ch, AFF_CHARM)) {
            send_to_char("Your superior would not approve of you giving "
                         "orders.\n\r", ch);
            return;
        }

        if (victim) {
            if (check_soundproof(victim)) {
                return;
            }
            sprintf(buf, "$N orders you to '%s'", message);
            act(buf, FALSE, victim, 0, ch, TO_CHAR);
            act("$n gives $N an order.", FALSE, ch, 0, victim, TO_ROOM);

            if ((victim->master != ch) || !IS_AFFECTED(victim, AFF_CHARM)) {
                if (RIDDEN(victim) == ch) {
                    check = MountEgoCheck(ch, victim);
                    if (check > 5) {
                        if (RideCheck(ch, -5)) {
                            act("$n has an indifferent look.", FALSE,
                                victim, 0, 0, TO_ROOM);
                        } else {
                            Dismount(ch, victim, POSITION_SITTING);
                            act("$n gets pissed and $N falls on $S butt!",
                                FALSE, victim, 0, ch, TO_NOTVICT);
                            act("$n gets pissed you fall off!",
                                FALSE, victim, 0, ch, TO_VICT);
                        }
                    } else if (check > 0) {
                        act("$n has an indifferent look.", FALSE, victim,
                            0, 0, TO_ROOM);
                    } else {
                        send_to_char("Ok.\n\r", ch);
                        command_interpreter(victim, message);
                    }
                } else if (IS_AFFECTED2(victim, AFF2_CON_ORDER) &&
                           victim->master == ch) {
                    send_to_char("Ok.\n\r", ch);
                    command_interpreter(victim, message);
                } else {
                    act("$n has an indifferent look.", FALSE, victim, 0, 0,
                        TO_ROOM);
                }
            } else {
                send_to_char("Ok.\n\r", ch);
                WAIT_STATE(victim, (21 - GET_CHR(ch)));
                command_interpreter(victim, message);
            }
        } else {                /* This is order "followers" */
            sprintf(buf, "$n issues the order '%s'.", message);
            act(buf, FALSE, ch, 0, victim, TO_ROOM);
            org_room = ch->in_room;

            for (k = ch->followers; k; k = k->next) {
                if (org_room == k->follower->in_room) {
                    if (IS_AFFECTED(k->follower, AFF_CHARM)) {
                        found = TRUE;
                    }
                }
            }
            if (found) {
                for (k = ch->followers; k; k = k->next) {
                    if (org_room == k->follower->in_room) {
                        if (IS_AFFECTED(k->follower, AFF_CHARM)) {
                            command_interpreter(k->follower, message);
                        }
                    }
                }
                send_to_char("Ok.\n\r", ch);
            } else {
                send_to_char("Nobody here is a loyal subject of yours!\n\r",
                             ch);
            }
        }
    }
}

void do_flee(struct char_data *ch, char *argument, int cmd)
{
    int             i,
                    attempt,
                    loose,
                    die,
                    percent,
                    charm;
    struct room_data *rp;
    int             panic,
                    j;

    dlog("in do_flee");

    if (IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return;
    }
    if (!(rp = real_roomp(ch->in_room))) {
        return;
    }
    if (GET_POS(ch) < POSITION_SLEEPING) {
        send_to_char("Not like this you can't!\n\r", ch);
        return;
    }
    if (A_NOFLEE(ch)) {
        send_to_char("The arena rules do not permit you to flee!\n\r", ch);
        return;
    }
    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK) ||
        IS_SET(ch->specials.affected_by2, AFF2_STYLE_BERSERK)) {
        send_to_char("You can think of nothing but the battle!\n\r", ch);
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, NO_FLEE) &&
        !IS_IMMORTAL(ch)) {
        send_to_char("Your feet feel like lead, and refuse to move!\n\r", ch);
        return;
    }
    if (affected_by_spell(ch, SPELL_WEB)) {
        if (!saves_spell(ch, SAVING_PARA)) {
            WAIT_STATE(ch, PULSE_VIOLENCE);
            send_to_char("You are ensared in webs, you cannot move!\n\r", ch);
            act("$n struggles against the webs that hold $m", FALSE, ch, 0,
                0, TO_ROOM);
            return;
        } else {
            send_to_char("You pull free from the sticky webbing!\n\r", ch);
            act("$n manages to pull free from the sticky webbing!", FALSE,
                ch, 0, 0, TO_ROOM);
            GET_MOVE(ch) -= 50;
        }
    }
    if (GET_POS(ch) <= POSITION_SITTING) {
        GET_MOVE(ch) -= 10;
        act("$n scrambles madly to $s feet!", TRUE, ch, 0, 0, TO_ROOM);
        act("Panic-stricken, you scramble to your feet.", TRUE, ch, 0, 0,
            TO_CHAR);
        GET_POS(ch) = POSITION_STANDING;
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }
    if (!(ch->specials.fighting)) {
        for (i = 0; i < 6; i++) {
            /*
             * Select a random direction
             */
            attempt = number(0, 5);
            if (CAN_GO(ch, attempt) &&
                !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                        DEATH) &&
                !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                        NO_FLEE)) {
                act("$n panics, and attempts to flee.", TRUE, ch, 0, 0,
                    TO_ROOM);
                if (RIDDEN(ch)) {
                    if ((die = MoveOne(RIDDEN(ch), attempt)) == 1) {
                        /*
                         * The escape has succeeded
                         */
                        send_to_char("You flee head over heels.\n\r", ch);
                        return;
                    } else {
                        if (!die) {
                            act("$n tries to flee, but is too exhausted!",
                                TRUE, ch, 0, 0, TO_ROOM);
                        }
                        return;
                    }
                } else {
                    if ((die = MoveOne(ch, attempt)) == 1) {
                        /*
                         * The escape has succeeded
                         */
                        send_to_char("You flee head over heels.\n\r", ch);
                        return;
                    } else {
                        if (!die) {
                            act("$n tries to flee, but is too exhausted!",
                                TRUE, ch, 0, 0, TO_ROOM);
                        }
                        return;
                    }
                }
            }
        }
        send_to_char("PANIC! You couldn't escape!\n\r", ch);
        return;
    }

    for (i = 0; i < 3; i++) {
        /*
         * Select a random direction
         */
        attempt = number(0, 5);
        if (CAN_GO(ch, attempt) &&
            !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                    DEATH) &&
            !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                    NO_FLEE)) {

            if (!ch->skills ||
                ch->skills[SKILL_RETREAT].learned < number(1, 101)) {
                act("$n panics, and attempts to flee.", TRUE, ch, 0, 0,
                    TO_ROOM);
                panic = TRUE;
                LearnFromMistake(ch, SKILL_RETREAT, 0, 90);
            } else {
                /*
                 * find a legal exit
                 */
                for (j = 0; j < 6; j++) {
                    if (CAN_GO(ch, j) &&
                        !IS_SET(real_roomp(EXIT(ch, j)->to_room)->room_flags,
                                DEATH) &&
                        !IS_SET(real_roomp(EXIT(ch, j)->to_room)->room_flags,
                                NO_FLEE)) {
                        attempt = j;
                        j = 10;
                    }
                }
                act("$n skillfully retreats from battle", TRUE, ch, 0, 0,
                    TO_ROOM);
                panic = FALSE;
            }
            if (IS_AFFECTED(ch, AFF_CHARM)) {
                charm = TRUE;
                REMOVE_BIT(ch->specials.affected_by, AFF_CHARM);
            } else {
                charm = FALSE;
            }
            if (RIDDEN(ch)) {
                die = MoveOne(RIDDEN(ch), attempt);
            } else {
                die = MoveOne(ch, attempt);
            }
            if (charm) {
                SET_BIT(ch->specials.affected_by, AFF_CHARM);
            }
            if (die == 1) {
                /*
                 * The escape has succeeded. We'll be nice.
                 */
                loose = 0;
                if (GetMaxLevel(ch) > 3) {
                    if (panic ||
                        !HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                      CLASS_PALADIN | CLASS_RANGER)) {
                        loose = GetMaxLevel(ch) + (GetSecMaxLev(ch) / 2) +
                                (GetThirdMaxLev(ch) / 3);
                        loose -= GetMaxLevel(ch->specials.fighting) +
                                 (GetSecMaxLev(ch->specials.fighting) / 2) +
                                 (GetThirdMaxLev(ch->specials.fighting) / 3);
                        loose *= GetMaxLevel(ch);
                    }
                }
                if (loose < 0) {
                    loose = 1;
                }
                if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF) &&
                    !(IS_SET(ch->specials.act, ACT_AGGRESSIVE)))) {
                    AddFeared(ch, ch->specials.fighting);
                } else {
                    percent = (int)(100.0 *
                                    (float)GET_HIT(ch->specials.fighting) /
                                    (float)GET_MAX_HIT(ch->specials.fighting));
                    if (Hates(ch->specials.fighting, ch)) {
                        SetHunting(ch->specials.fighting, ch);
                    } else if ((IS_GOOD(ch) &&
                                IS_EVIL(ch->specials.fighting)) ||
                               (IS_EVIL(ch) &&
                                IS_GOOD(ch->specials.fighting))) {
                        AddHated(ch->specials.fighting, ch);
                        SetHunting(ch->specials.fighting, ch);
                    } else if (number(1, 101) < percent) {
                        AddHated(ch->specials.fighting, ch);
                        SetHunting(ch->specials.fighting, ch);
                    }
                }
                if (IS_PC(ch) && panic) {
                    if (HasClass(ch, CLASS_MONK) ||
                        !HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                      CLASS_PALADIN | CLASS_RANGER)) {
                        gain_exp(ch, -loose);
                    }
                }
                if (panic) {
                    send_to_char("You flee head over heels.\n\r", ch);
                } else {
                    send_to_char("You retreat skillfully\n\r", ch);
                    if (IS_PC(ch)) {
                        send_to_char("$c000BYou receive $c000W100 $c000B"
                                     "experience for using your combat "
                                     "abilities.$c0007\n\r", ch);
                        gain_exp(ch, 100);
                    }
                }
                /*
                 * gotta make ALL ch's attackers stop fighting, not just
                 * one
                 * for(tmp = rp->people; tmp; tmp->next_in_room) {
                 * if(tmp->specials.fighting == ch)
                 * stop_fighting(tmp);
                 * }
                 */

                if (ch->specials.fighting->specials.fighting == ch) {
                    stop_fighting(ch->specials.fighting);
                }

                if (ch->specials.fighting) {
                    stop_fighting(ch);
                }

#if 0
                if (ch->attackers) {
                    Log("fleeing dude still being attacked?! Could be bad.");
                }
#endif

                if (IS_PC(ch)) {
                    if (ch->specials.remortclass == (THIEF_LEVEL_IND + 1) &&
                        !IS_AFFECTED(ch, AFF_HIDE)) {
                        do_hide(ch, 0, 0);
                        if (!IS_AFFECTED(ch, AFF_HIDE)) {
                            do_hide(ch, 0, 0);
                        }
                    }
                }
                return;
            } else {
                if (!die) {
                    act("$n tries to flee, but is too exhausted!", TRUE,
                        ch, 0, 0, TO_ROOM);
                }
                return;
            }
        }
    }
    /*
     * No exits were found
     */
    send_to_char("PANIC! You couldn't escape!\n\r", ch);
}

void do_disengage(struct char_data *ch, char *argument, int cmd)
{
    int             percent;
    char            buf[256];

    if (argument && strcmp(argument, "-skill") == 0) {
        sprintf(buf, "Disengage Skill----->%s.\n\r",
                how_good(ch->skills[SKILL_DISENGAGE].learned));
        send_to_char(buf, ch);
        return;
    }

    if ((GET_POS(ch) == POSITION_FIGHTING) && !ch->specials.fighting) {
        stop_fighting(ch);
        send_to_char("You realize you look silly fighting nothing, and "
                     "disengage.\n\r", ch);
        act("$n looks around sheepishly and stops fighting $s shadow.",
            TRUE, ch, 0, 0, TO_ROOM);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    if (!ch->specials.fighting) {
        send_to_char("You are not fighting.\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_DISENGAGE].learned) {
        send_to_char("You don't seem to know this skill.", ch);
        return;
#if 0
        ch->skills[SKILL_DISENGAGE].learned = 50;
#endif
    }

    percent = number(1, 101);
    percent += (GET_DEX(ch) - GET_DEX(ch->specials.fighting));
    if (IS_AFFECTED(ch->specials.fighting, AFF_PARALYSIS)) {
        percent = 0;
    }

    if (percent > ch->skills[SKILL_DISENGAGE].learned) {
        send_to_char("You can't seem to get away.\n\r", ch);
        act("$n tries to back out of combat with $N, but seems to stumble.",
            TRUE, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n tries to back out of combat with you, but seems to stumble.",
            TRUE, ch, 0, ch->specials.fighting, TO_VICT);
        LearnFromMistake(ch, SKILL_DISENGAGE, 0, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    } else {
        send_to_char("You step back and try to disengage\n\r", ch);
        act("$n backs out of combat with $N in an attempt to disengage",
            TRUE, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n tries to back out of combat with you in an attempt to "
            "disengage.", TRUE, ch, 0, ch->specials.fighting, TO_VICT);
        stop_fighting(ch);
        sprintf(buf, "$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your combat abilities.$c0007\n\r");
        send_to_char(buf, ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
}

void do_assist(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim,
                   *tmp_ch;
    char           *victim_name;

    dlog("in do_assist");

    if (check_peaceful(ch, "Noone should need assistance here.\n\r")) {
        return;
    }

    argument = get_argument(argument, &victim_name);
    if (!victim_name || !(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Who do you want to assist?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Oh, by all means, help yourself...\n\r", ch);
        return;
    }

    if (ch->specials.fighting == victim) {
        send_to_char("That would be counterproductive?\n\r", ch);
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("You have your hands full right now\n\r", ch);
        return;
    }

    if (victim->attackers >= 6) {
        send_to_char("You can't get close enough to them to assist!\n\r", ch);
        return;
    }

    tmp_ch = victim->specials.fighting;
    if (!tmp_ch) {
        act("But $E's not fighting anyone.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (tmp_ch->in_room != ch->in_room) {
        send_to_char("Woops, they left in a hurry, must have scared them "
                     "off!\n\r", ch);
        return;
    }
    hit(ch, tmp_ch, TYPE_UNDEFINED);
    WAIT_STATE(ch, PULSE_VIOLENCE + 2);
}

void do_wimp(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_wimp");

    if (IS_SET(ch->specials.act, PLR_WIMPY)) {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        send_to_char("Ok, you are no longer a wimp...\n\r", ch);
    } else {
        SET_BIT(ch->specials.act, PLR_WIMPY);
        send_to_char("Ok, you are now in wimpy mode.\n\r", ch);
    }
}

void do_breath(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char            buf[MAX_STRING_LENGTH],
                   *name;
    int             count,
                    manacost;
    funcp           weapon;
    struct breather *scan;


    dlog("in do_breath");

    if (check_peaceful(ch, "That wouldn't be nice at all.\n\r"))
        return;

    argument = get_argument(argument, &name);

    for (count = FIRST_BREATH_WEAPON;
         count <= LAST_BREATH_WEAPON && !affected_by_spell(ch, count);
         count++) {
        /* Empty Loop */
    }

    if (count > LAST_BREATH_WEAPON) {
        for (scan = breath_monsters;
             scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual;
             scan++) {
            /*
             * Empty loop
             */
        }

        if (scan->vnum < 0) {
            send_to_char("You don't have a breath weapon, potatohead.\n\r", ch);
            return;
        }

        for (count = 0; scan->breaths[count]; count++) {
            /*
             * Empty loop
             */
        }

        if (count < 1) {
            sprintf(buf, "monster %s has no breath weapons",
                    ch->player.short_descr);
            Log(buf);
            send_to_char("Hey, why don't you have any breath weapons!?\n\r",
                         ch);
            return;
        }

        weapon = scan->breaths[dice(1, count) - 1];
        manacost = scan->cost;
        if (GET_MANA(ch) <= -3 * manacost) {
            weapon = NULL;
        }
    } else {
        manacost = 0;
        weapon = bweapons[count - FIRST_BREATH_WEAPON];
        affect_from_char(ch, count);
    }
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Breath on who?\n\r", ch);
            return;
        }
    }
    breath_weapon(ch, victim, manacost, weapon);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "breathing abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

void do_shoot(struct char_data *ch, char *argument, int cmd)
{
#if 0
    char           *arg,
                    dirstr[80],
                   *name;
    char            buf[255];
    struct char_data *victim;
    struct room_data *this_room,
                   *to_room,
                   *next_room;
    struct room_direction_data *exitp;
    struct char_data *mob;
    struct obj_data *weapon;
    int             i,
                    dir,
                    room_num = 0,
                    room_count,
                    MAX_DISTANCE_SHOOT;
    extern char    *listexits[];

    if (check_peaceful
        (ch, "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }
    argument = get_argument(argument, &arg);
    slog("begin do_shoot");
    if (arg) {
        victim = get_char_room_vis(ch, arg);
        if (!victim) {
            i = ch->in_room;
            room_count = 1;
            weapon = ch->equipment[HOLD];
            if (!weapon) {
                send_to_char("You do not hold a missile weapon?!?!\n\r",
                             ch);
                return;
            }
#if 0
            MAX_DISTANCE_SHOOT = weapon->obj_flags.value[1];
#else
            MAX_DISTANCE_SHOOT = 1;
#endif

            switch (*arg) {
            case 'N':
            case 'n':
                dir = 0;
                break;
            case 'S':
            case 's':
                dir = 2;
                break;
            case 'E':
            case 'e':
                dir = 1;
                break;
            case 'W':
            case 'w':
                dir = 3;
                break;
            case 'd':
            case 'D':
                dir = 4;
                break;
            case 'u':
            case 'U':
                dir = 5;
                break;

            default:
                send_to_char("What direction did you wish to fire?\n\r",
                             ch);
                return;
                break;
            }

            argument = get_argument(argument, &name);
            if (name && strncasecmp(name, "at", 2) && isspace(name[2])) {
                argument = get_argument(argument, &name);
            }
            if (!exit_ok(EXIT_NUM(i, dir), NULL)) {
                send_to_char("You can't shoot in that direction.\n\r", ch);
                return;
            }
            while (room_count <= MAX_DISTANCE_SHOOT && !victim &&
                   exit_ok(EXIT_NUM(i, dir), NULL)) {
                this_room = real_roomp(i);
                to_room = real_roomp(this_room->dir_option[dir]->to_room);
                room_num = this_room->dir_option[dir]->to_room;
                mob = get_char_near_room_vis(ch, name, room_num);
                if (mob) {
                    sprintf(buf, "You spot your quarry %s.\n",
                            listexits[dir]);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                    victim = mob;
                }
                i = room_num;
                room_count++;

            }
        }
        /*
         * !victim
         */
        if (victim) {
            if (victim == ch) {
                send_to_char("You can't shoot things at yourself!", ch);
                return;
            } else {
                if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == victim)) {
                    act("$N is just such a good friend, you simply "
                            "can't shoot at $M.",
                            FALSE, ch, 0, victim, TO_CHAR);
                    return;
                }
                if (ch->specials.fighting) {
                    send_to_char
                        ("You're at too close range to fire a weapon!\n\r",
                         ch);
                    return;
                }
                if (check_peaceful(victim, "")) {
                    send_to_char("That is a peaceful room\n\r", ch);
                    return;
                }
                shoot(ch, victim);
                send_to_char
                    ("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your shooting abilities.$c0007\n\r", ch);
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE);
            }
        } else {
            send_to_char("They aren't here.\n\r", ch);
        }
    } else {
        send_to_char("Shoot who?\n\r", ch);
    }
    slog("end do_shoot, act.off.c");

#endif
}

void kick_messages(struct char_data *ch, struct char_data *victim,
                   int damage)
{
    int             i;
    char            buf[300];
    char            buf2[300];
    char            color[] = "$c0003";

    switch (GET_RACE(victim)) {
    case RACE_HUMAN:
    case RACE_MOON_ELF:
    case RACE_WILD_ELF:
    case RACE_GOLD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_DROW:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_TROLL:
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_MFLAYER:
    case RACE_ASTRAL:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_DRAAGDIM:
    case RACE_GOLEM:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_HALF_ELF:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        i = number(0, 3);
        break;
    case RACE_PREDATOR:
    case RACE_HERBIV:
    case RACE_LABRAT:
        i = number(4, 6);
        break;
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_DRAGON_AMETHYST:
    case RACE_DRAGON_CRYSTAL:
    case RACE_DRAGON_EMERALD:
    case RACE_DRAGON_SAPPHIRE:
    case RACE_DRAGON_TOPAZ:
    case RACE_DRAGON_BROWN:
    case RACE_DRAGON_CLOUD:
    case RACE_DRAGON_DEEP:
    case RACE_DRAGON_MERCURY:
    case RACE_DRAGON_MIST:
    case RACE_DRAGON_SHADOW:
    case RACE_DRAGON_STEEL:
    case RACE_DRAGON_YELLOW:
    case RACE_DRAGON_TURTLE:
        i = number(4, 7);
        break;
    case RACE_TREE:
        i = 8;
        break;
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
        i = 9;
        break;
    case RACE_ROO:
    case RACE_DEEP_GNOME:
    case RACE_ROCK_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_SMURF:
    case RACE_ENFAN:
        i = 10;
        break;
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_TYTAN:
    case RACE_GOD:
        i = 11;
        break;
    case RACE_GHOST:
        i = 12;
        break;
    case RACE_BIRD:
    case RACE_AVARIEL:
    case RACE_SKEXIE:
        i = 13;
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
        i = 14;
        break;
    case RACE_DINOSAUR:
        i = 15;
        break;
    case RACE_INSECT:
    case RACE_ARACHNID:
        i = 16;
        break;
    case RACE_FISH:
        i = 17;
        break;
    default:
        i = 18;
    };
    if (!damage) {
        strcpy(buf2, att_kick_miss_ch[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        strcpy(buf2, att_kick_miss_victim[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
        strcpy(buf2, att_kick_miss_room[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
    } else if (GET_HIT(victim) -
               DamageTrivia(ch, victim, damage, SKILL_KICK) < -10) {
        strcpy(buf2, att_kick_kill_ch[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        strcpy(buf2, att_kick_kill_victim[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
        strcpy(buf2, att_kick_kill_room[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
    } else {
        strcpy(buf2, att_kick_hit_ch[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        strcpy(buf2, att_kick_hit_victim[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
        strcpy(buf2, att_kick_hit_room[i]);
        strcpy(buf, color);
        strcat(buf, buf2);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
    }
}

void throw_weapon(struct obj_data *o, int dir, struct char_data *targ,
                  struct char_data *ch)
{
    int             w = o->obj_flags.weight,
                    sz,
                    max_range,
                    range,
                    there;
    int             rm = ch->in_room;
    int             broken = FALSE;
    char            buf[MAX_STRING_LENGTH];
    struct char_data *spud,
                   *next_spud;
    char           *dir_name[] = {
        "the north",
        "the east",
        "the south",
        "the west",
        "above",
        "below"
    };

    if (w > 100) {
        sz = 3;
    } else if (w > 25) {
        sz = 2;
    } else if (w > 5) {
        sz = 1;
    } else {
        sz = 0;
    }
    max_range = (((GET_STR(ch) + GET_ADD(ch) / 30) - 3) / 8) + 2;
    max_range = max_range / (sz + 1);
    if (o->obj_flags.type_flag == ITEM_MISSILE && ch->equipment[WIELD] &&
        ch->equipment[WIELD]->obj_flags.type_flag == ITEM_FIREWEAPON) {
        /*
         * Add bow's range bonus
         */
        max_range += ch->equipment[WIELD]->obj_flags.value[2];
    }
    if (max_range == 0) {
        act("$p immediately hits the ground.  A truly pitiful throw.",
            TRUE, ch, o, 0, TO_ROOM);
        act("$p flew 2 feet then fell to earth.  Maybe a lighter weapon...",
            TRUE, ch, o, 0, TO_CHAR);
        obj_to_room(o, ch->in_room);
        return;
    }
    range = 0;
    while (range < max_range && broken == FALSE) {
        /*
         * Check for target
         */
        there = 0;
        for (spud = real_roomp(rm)->people; spud; spud = next_spud) {
            next_spud = spud->next_in_room;
            if (spud == targ) {
                there = 1;
                if (range_hit(ch, targ, range, o, dir, max_range)) {
                    if ((targ) && (GET_POS(targ) > POSITION_DEAD)) {
                        if (number(1, 100) < o->obj_flags.value[0]) {
                            act("$p breaks into splinters.", TRUE, targ, o,
                                0, TO_ROOM);
                            broken = TRUE;
                            /*
                             * storage for broken arrows
                             */
                            obj_to_room(o, 3);
                            /*
                             * for some reason this causes the obj to get
                             * placed in very weird places
                             */
#if 0
                              obj_from_room(o);
                              extract_obj(o);
#endif
                        } else {
                            obj_to_room(o, rm);
                        }
                    }
                    slog("Gobble 2");
                    return;
                }
            }
        }
        if (broken == FALSE) {
            if (clearpath(ch, rm, dir)) {
                if ((!there) && (rm != ch->in_room)) {
                    sprintf(buf, "%s from %s flies past!\n\r",
                            o->short_description, dir_name[opdir(dir)]);
                    send_to_room(buf, rm);
                } else {
                    there = 0;
                }
                rm = real_roomp(rm)->dir_option[dir]->to_room;
            } else {
                if (range > 1) {
                    sprintf(buf, "%s flies into the room from %s and hits a "
                                 "wall.\n\r",
                            o->short_description, dir_name[opdir(dir)]);
                } else {
                    sprintf(buf, "%s hits a wall.\n\r",
                            o->short_description);
                }
                send_to_room(buf, rm);
                if (number(1, 100) < o->obj_flags.value[0]) {
                    sprintf(buf, "%s breaks into splinters.\n\r",
                            o->short_description);
                    obj_to_room(o, 3);
                    /*
                     * storage for broken arrows
                     */
                    broken = TRUE;
                } else {
                    sprintf(buf, "%s falls to the ground.\n\r",
                            o->short_description);
                    obj_to_room(o, rm);
                }
                send_to_room(buf, rm);
                return;
            }
        }
        range++;
    }
    if (broken == FALSE) {
        sprintf(buf, "%s falls to the ground.\n\r", o->short_description);
        send_to_room(buf, rm);
        obj_to_room(o, rm);
    }
}

void throw_object(struct obj_data *o, int dir, int from)
{
    const char     *directions[][2] = {
        {"north", "south"},
        {"east", "west"},
        {"south", "north"},
        {"west", "east"},
        {"up", "down"},
        {"down", "up"}
    };

    char            buf1[100];
    int             distance = 0;

    while (distance < 20 && real_roomp(from)->dir_option[dir] &&
           real_roomp(from)->dir_option[dir]->exit_info < 2 &&
           real_roomp(from)->dir_option[dir]->to_room > 0) {
        if (distance) {
            sprintf(buf1, "%s flies into the room from the %s.\n\r",
                    o->short_description, directions[dir][1]);
            send_to_room(buf1, from);

#if 0
            for (catcher = real_roomp(from)->people; catcher;
                 catcher = catcher->next_in_room) {
                if (!strcmp(catcher->catch, o->name)) {
                    switch (number(1, 3)) {
                    case 1:
                        act("$n dives and catches $p", FALSE, catcher, o,
                            0, TO_ROOM);
                        break;
                    default:
                        act("$n catches $p", FALSE, catcher, o, 0,
                            TO_ROOM);
                    }
                    send_to_char("You caught it!\n\r", catcher);
                    obj_from_room(o);
                    obj_to_char(o, catcher);
                    return;
                }
            }
#endif

            sprintf(buf1, "%s flies out of the room to the %s.\n\r",
                    o->short_description, directions[dir][0]);
            send_to_room(buf1, from);
        }
        distance++;
        obj_from_room(o);
        from = real_roomp(from)->dir_option[dir]->to_room;
        obj_to_room(o, from);
    }
    if (distance == 20) {
        sprintf(buf1, "%s flies into the room from the %s and lands here.\n\r",
                o->short_description, directions[dir][1]);
        send_to_room(buf1, from);
        return;
    }
    sprintf(buf1, "%s flies into the room from the %s and smacks a wall.\n\r",
            o->short_description, directions[dir][1]);
    send_to_room(buf1, from);
}

int clearpath(struct char_data *ch, long room, int direc)
{
    struct room_direction_data *exitdata;

    exitdata = (real_roomp(room)->dir_option[direc]);

    if ((exitdata) && (!real_roomp(exitdata->to_room))) {
        return 0;
    }
    if (!CAN_GO(ch, direc)) {
        return 0;
    }
    if (!real_roomp(room)->dir_option[direc]) {
        return 0;
    }
    if (real_roomp(room)->dir_option[direc]->to_room < 1) {
        return 0;
    }
    if (real_roomp(room)->zone !=
        real_roomp(real_roomp(room)->dir_option[direc]->to_room)->zone) {
        return 0;
    }
    if (IS_SET(real_roomp(room)->dir_option[direc]->exit_info, EX_CLOSED)) {
        return 0;
    }
    if ((!IS_SET
         (real_roomp(room)->dir_option[direc]->exit_info, EX_ISDOOR))
        && (real_roomp(room)->dir_option[direc]->exit_info > 0)) {
        return 0;
    }
    /*
     * One-way windows are allowed... no see through 1-way exits
     */
    if (!real_roomp(real_roomp(room)->dir_option[direc]->to_room)->
        dir_option[opdir(direc)]) {
        return 0;
    }
    if (real_roomp(real_roomp(room)->dir_option[direc]->to_room)->
        dir_option[opdir(direc)]->to_room < 1) {
        return 0;
    }
    if (real_roomp((real_roomp(room)->dir_option[direc]->to_room))->
        dir_option[opdir(direc)]->to_room != room) {
        return 0;
    }
    return real_roomp(room)->dir_option[direc]->to_room;
}

void do_weapon_load(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *fw,
                   *ms;
    char           *arg1;

    dlog("in do_weapon_load");

    fw = ch->equipment[WIELD];
    if (!fw || fw->obj_flags.type_flag != ITEM_FIREWEAPON) {
        send_to_char("You must be wielding the projectile weapon you want to "
                     "load.\n\r", ch);
        return;
    }
    if ((GET_STR(ch) + (GET_ADD(ch) / 3)) < fw->obj_flags.value[0]) {
        sprintf(arg1, "(%s) can't load (%s) because it requires (%d) strength "
                      "to wield",
                GET_NAME(ch), fw->name, fw->obj_flags.value[0]);
        Log(arg1);
        send_to_char("You aren't strong enough to draw such a mighty "
                     "weapon.\n\r", ch);
        return;
    }
    if (ch->equipment[LOADED_WEAPON]) {
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
            ms = unequip_char(ch, LOADED_WEAPON);
            act("You first unload $p.", TRUE, ch, ms, 0, TO_CHAR);
            obj_to_char(ms, ch);
            act("$n unloads $p.", FALSE, ch, ms, 0, TO_ROOM);
        } else {
            send_to_char("Your hands are too full to unload.\n\r", ch);
            return;
        }
    }

    argument = get_argument(argument, &arg1);
    if (!arg1) {
        send_to_char("You must specify the projectile to load!\n\r", ch);
        return;
    }

    ms = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (!ms) {
        send_to_char("You don't seem to have that.\n\r", ch);
        return;
    }

    if (ms->obj_flags.type_flag != ITEM_MISSILE) {
        act("$p is not a valid missile.", TRUE, ch, ms, 0, TO_CHAR);
        return;
    }

    if (ms->obj_flags.value[3] != fw->obj_flags.value[2]) {
        act("You can't load $p in that sort of weapon.", TRUE, ch, ms, 0,
            TO_CHAR);
        return;
    }

    obj_from_char(ms);
    equip_char(ch, ms, LOADED_WEAPON);
    act("You load $p.", TRUE, ch, ms, 0, TO_CHAR);
    act("$n loads $p.", FALSE, ch, ms, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void do_fire(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *fw,
                   *ms;
    char           *arg;
    struct char_data *targ;
    int             tdir,
                    rng,
                    dr;

    dlog("in do_fire");

    fw = ch->equipment[WIELD];
    if ((!fw) || (fw->obj_flags.type_flag != ITEM_FIREWEAPON)) {
        send_to_char("You must be using a projectile weapon to fire "
                     "things!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("The proper format for fire is: fire <target>\n\r", ch);
        return;
    }

    targ = get_char_linear(ch, arg, &rng, &dr);
    if (targ && targ == ch) {
        send_to_char("You can't fire that at yourself!\n\r", ch);
        return;
    }

    if (!targ) {
        send_to_char("You don't see anybody around meeting that "
                     "description.\n\r", ch);
        return;
    } else {
        /*
         * add spot check before letting mortals get farther
         */
        if (rng > fw->obj_flags.value[1]) {
            send_to_char("You just can't fire that far!\n\r", ch);
            return;
        }

        tdir = dr;
    }

    if (check_peaceful(targ, "Sorry but you can not fire a range weapon into "
                             "a peaceful room.")) {
        send_to_char("Sorry but you can not fire a range weapon into a "
                     "peaceful room.", ch);
        return;
    }

    if (ch->equipment[LOADED_WEAPON]) {
        ms = unequip_char(ch, LOADED_WEAPON);
    } else {
        act("$p is not loaded!", TRUE, ch, fw, 0, TO_CHAR);
        return;
    }

    send_to_char("OK.\n\r", ch);
    act("$n fires $p!", TRUE, ch, ms, 0, TO_ROOM);
    throw_weapon(ms, tdir, targ, ch);
}

void do_throw(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *throw;
    char           *arg1,
                   *arg2;
    char           *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "\n"
    };
    int             rng,
                    dr,
                    tdir;
    struct char_data *targ;

    dlog("in do_throw");

    argument = get_argument(argument, &arg1);
    arg2 = skip_spaces(argument);
    if (!arg1 || !arg2) {
        send_to_char("The correct format for 'throw' is: throw <object> "
                     "<target>\n\r", ch);
        return;
    }

    throw = get_obj_in_list_vis(ch, arg1, ch->carrying);
    /*
     * Check if second argument is a character or direction
     */
    targ = get_char_linear(ch, arg2, &rng, &dr);
    if (targ && targ == ch) {
        send_to_char("You can't throw that at yourself!\n\r", ch);
        return;
    }

    if (!targ) {
        tdir = search_block(arg1, keywords, FALSE);
        if (tdir == -1) {
            send_to_char("You don't see anybody around meeting that "
                         "description.\n\r", ch);
            return;
        } else {
            /*
             * add spot check before letting mortals get father
             */

            if (rng > throw->obj_flags.value[1]) {
                send_to_char("You just can't throw it that far!\n\r", ch);
                return;
            }
        }
    } else {
        tdir = dr;
    }

    if (!throw) {
        send_to_char("You don't have that!\n\r", ch);
        return;
    }

    if (!IS_SET(throw->obj_flags.wear_flags, ITEM_THROW)) {
        send_to_char("You cant throw that!\n\r", ch);
        return;
    }

    if (throw->obj_flags.type_flag != ITEM_WEAPON) {
        /*
         * Friendly throw
         */

#if 0
        if (cmd!=263)
#endif
        send_to_char("OK.\n\r", ch);
        obj_from_char(throw);
#if 0
        if (cmd!=263)
#endif
        act("$n throws $p!", TRUE, ch, throw, 0, TO_ROOM);
        obj_to_room(throw, ch->in_room);
        throw_object(throw, tdir, ch->in_room);
    } else {
        if (check_peaceful(targ, "Someone just tried to throw a weapon at you,"
                                 " and you are in peaceful room! HA!")) {
            send_to_char("You can not throw a weapon into a peaceful room!",
                         ch);
            return;
        }
#if 0
        if (cmd!=263)
#endif
        send_to_char("OK.\n\r", ch);
        obj_from_char(throw);
#if 0
        if (cmd!=263)
#endif
        act("$n throws $p!", TRUE, ch, throw, 0, TO_ROOM);
        throw_weapon(throw, tdir, targ, ch);
#if 0
        if (cmd!=263)
#endif
        WAIT_STATE(ch, 1);
    }
}

void do_style(struct char_data *ch, char *argument, int cmd)
{
    char           *style,
                    temp[128],
                    buf[254],
                    buffer[MAX_STRING_LENGTH + 30];
    int             x = 0;
    extern struct skillset styleskillset[];

    if (IS_NPC(ch))
        return;

    buffer[0] = '\0';

    argument = get_argument(argument, &style);
    if (!style) {
        /*
         * If no style, list all styles and how good
         */
        sprintf(buffer, "You currently know the following fighting "
                        "styles.\n\r");
        while (styleskillset[x].level != -1) {
            if (IS_SET(ch->skills[styleskillset[x].skillnum].flags,
                       SKILL_KNOWN)) {
                sprintf(buf, "[%-2d] %-30s %-15s", styleskillset[x].level,
                        styleskillset[x].name,
                       how_good(ch->skills[styleskillset[x].skillnum].learned));
                if (IsSpecialized(
                            ch->skills[styleskillset[x].skillnum].special)) {
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
            x++;
        }
        page_string(ch->desc, buffer, 1);
        send_to_char("To choose a fighting style, type '$c000Wstyle <style "
                     "name>$c000w'\n\r", ch);
        ch_printf(ch, "You are currently fighting $c000W%s$c000w.\n\r",
                  fight_styles[ch->style]);
    } else {
        x = 0;
        while (styleskillset[x].level != -1) {
            if (is_abbrev(style, styleskillset[x].name) &&
                ch->skills[x + 298].learned > 0) {
                ch_printf(ch, "You change your stance and adopt %s %s style "
                              "of fighting.\n\r",
                          style[0] == 'A' || style[0] == 'E' ? "an" : "a",
                          fight_styles[x]);
                sprintf(temp, "$n changes $s stance and adopts %s %s style "
                              "of fighting.",
                        style[0] == 'A' || style[0] == 'E' ? "an" : "a",
                        fight_styles[x]);
                act(temp, TRUE, ch, 0, 0, TO_ROOM);
                ch->style = x;
                return;
            }
            x++;
        }
        send_to_char("You don't seem to know that fighting style.\n\r", ch);
    }
}

void do_induct(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;

    dlog("in do_induct");

    if (!ch->skills) {
        return;
    }
    if (!IS_SET(ch->specials.act, PLR_CLAN_LEADER)) {
        send_to_char("Only clan leaders can induct new members.\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);

    if (!name || !(victim = get_char_room_vis(ch, name))) {
        send_to_char("Who do you wish to induct into your clan??\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You can't induct a mob.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Your already part of the clan silly!!\n\r", ch);
        return;
    }

    if (GET_CLAN(victim) != 0) {
        send_to_char("They already belong to a clan.\n\r", ch);
        return;
    }

    GET_CLAN(victim) = GET_CLAN(ch);
    ch_printf(victim, "You have just been inducted into the clan, %s.\n\r",
              clan_list[GET_CLAN(victim)].name);
    ch_printf(ch, "You just initiated %s into the clan, %s.\n\r",
              GET_NAME(victim), clan_list[GET_CLAN(victim)].name);
}

void do_expel(struct char_data *ch, char *argument, int cmd)
{

    struct char_data *victim;
    char           *name;

    dlog("in do_expel");

    if (!ch->skills) {
        return;
    }
    if (!IS_SET(ch->specials.act, PLR_CLAN_LEADER)) {
        send_to_char("Only clan leaders can expel members.\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        send_to_char("Who do you wish to expel from your clan??\n\r", ch);
        return;

    }

    if (IS_NPC(victim)) {
        send_to_char("You can't expel a mob.\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("You can't expel yourself silly!!\n\r", ch);
        return;
    }

    if (GET_CLAN(victim) != GET_CLAN(ch)) {
        send_to_char("You can't expel someone that isn't in your clan.\n\r",
                     ch);
        return;
    }

    ch_printf(victim, "You have just been exiled from %s.\n\r",
              clan_list[GET_CLAN(victim)].name);
    ch_printf(ch, "You just exiled %s from %s.\n\r", GET_NAME(victim),
              clan_list[GET_CLAN(victim)].name);

    GET_CLAN(victim) = 1;
}

void do_chat(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_STRING_LENGTH + 80];
    struct descriptor_data *i;
    extern int      Silence;
    int             clannum = 0;

    dlog("in do_chat");

    if ((!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) ||
        IS_SET(ch->specials.act, PLR_NOOOC) ||
        IS_SET(ch->specials.act, PLR_WIZNOOOC)) {
        send_to_char("You can't use this command!!\n\r", ch);
        return;
    }
    if ((clannum = GET_CLAN(ch)) <= 1) {
        send_to_char("You can't clan chat. You don't belong to a clan.\n\r",
                     ch);
        return;
    }
    if (IS_SET(SystemFlags, SYS_NOOOC)) {
        send_to_char("The use of clan chat has been temporarilly banned.\n\r",
                     ch);
        return;
    }
    if (IS_NPC(ch) && Silence == 1 && IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Polymorphed worldwide comms has been banned.\n\r", ch);
        send_to_char("It may return after a bit.\n\r", ch);
        return;
    }
    if ((GET_MOVE(ch) < 5 || GET_MANA(ch) < 5) && !IS_IMMORTAL(ch)) {
        send_to_char("You do not have the strength to clan chat!\n\r", ch);
        return;
    }
    if (apply_soundproof(ch)) {
        return;
    }

    if ((ch->master && IS_AFFECTED(ch, AFF_CHARM)) &&
        !IS_IMMORTAL(ch->master)) {
        send_to_char("I don't think so :-)", ch->master);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Hrm... normally, you should CHAT something...\n\r", ch);
        return;
    }

    if (strlen(argument) > 150) {
        send_to_char("Line too long, truncated\n", ch);
        argument[151] = '\0';
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        sprintf(buf1, "$c000cYou chat '$c0008%s$c000c'", argument);
        act(buf1, FALSE, ch, 0, 0, TO_CHAR);
    }

    sprintf(buf1, "$c000c-=$c0008%s$c000c=- clan chats '$c0008%s$c000c'\n\r",
            GET_NAME(ch), argument);

    if (!IS_IMMORTAL(ch)) {
        GET_MOVE(ch) -= 5;
        GET_MANA(ch) -= 5;
    }

    for (i = descriptor_list; i; i = i->next) {
        if (i->character != ch && !i->connected &&
            (IS_NPC(i->character) ||
             (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
              !IS_SET(i->character->specials.act, PLR_NOOOC) &&
              !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
            !check_soundproof(i->character) &&
            GET_CLAN(i->character) == GET_CLAN(ch)) {

            send_to_char(buf1, i->character);
        }
    }
}

void do_qchat(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_STRING_LENGTH + 80];
    struct descriptor_data *i;
    extern int      Silence;

    dlog("in do_qchat");

    if ((!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) ||
        IS_SET(ch->specials.act, PLR_NOOOC) ||
        IS_SET(ch->specials.act, PLR_WIZNOOOC)) {
        send_to_char("You can't use this command!!\n\r", ch);
        return;
    }
    if (!IS_AFFECTED2(ch, AFF2_QUEST)) {
        send_to_char("You cannot quest chat. Gotta join first.\n\r", ch);
        return;
    }
    if (IS_SET(SystemFlags, SYS_NOOOC)) {
        send_to_char("The use of quest chat has been temporarilly banned.\n\r",
                     ch);
        return;
    }
    if (IS_NPC(ch) && Silence == 1 && IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Polymorphed worldwide comms has been banned.\n\r", ch);
        send_to_char("It may return after a bit.\n\r", ch);
        return;
    }
    if (apply_soundproof(ch)) {
        return;
    }

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM) &&
        !IS_IMMORTAL(ch->master)) {
        send_to_char("I don't think so :-)", ch->master);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Your fellow questees aren't interested in hearing "
                     "nothing.\n\r", ch);
        return;
    }

    if (strlen(argument) > 150) {
        send_to_char("Line too long, truncated\n", ch);
        argument[151] = '\0';
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        sprintf(buf1, "$c0008You $c000Rq$c000Yu$c000Ge$c000Bs$c000Ct "
                      "$c0008'$c000C%s$c0008'", argument);
        act(buf1, FALSE, ch, 0, 0, TO_CHAR);
    }

    sprintf(buf1, "$c0008-=$c000c%s$c0008=- $c000Rq$c000Yu$c000Ge$c000Bs"
                  "$c000Ct$c000Ps $c0008'$c000C%s$c0008'\n\r",
            GET_NAME(ch), argument);

    for (i = descriptor_list; i; i = i->next) {
        if (i->character != ch && !i->connected &&
            (IS_NPC(i->character) ||
             (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
              !IS_SET(i->character->specials.act, PLR_NOOOC) &&
              !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
            !check_soundproof(i->character) &&
            IS_AFFECTED2(i->character, AFF2_QUEST)) {
            send_to_char(buf1, i->character);
        }
    }
}

void do_dig(struct char_data *ch, char *argument, int cmd) {

    /*char buffer[MAX_STRING_LENGTH];*/

    struct obj_data *o, *prize;

    dlog("in do_dig (With a shovel)");

    if(!(o = ch->equipment[HOLD])) {
        send_to_char("Hmmm, what happen to that shovel?",ch);
        return;
    }

    if(ITEM_TYPE(o) != ITEM_SHOVEL) {
        send_to_char("How do you plan on shovelling with that thing?",ch);
        return;
    }

    if(o->obj_flags.value[0] < 1) {
        send_to_char("The shovel appears to be broken!", ch);
        return;
    }

    if(o->obj_flags.value[1] != ch->in_room) { /*not in currect room*/

        o->obj_flags.value[0]--;


        /* if the object is below 0, break it..*/
        if(o->obj_flags.value[0] < 1) {
            send_to_char("You start digging long and hard.  *SNAP*\n\r"
                        "Your shovel suddently breaks in two!",ch);
            /*Rename object afterwards-Not done*/
        } else
            send_to_char("You start to dig... After much digging,"
                         " you come up with a whole lot of nothing",ch);
        /*lag for a few seconds */

        return;
    } else {  /*Bingo! We are in the right spot!!*/
        if(o->obj_flags.value[3]==1) {

            send_to_char("You dig and dig, but find nothing more "
                    "than what you found earlier.",ch);
             /*Print message to all in room*/
            return;
        }

        if(!(prize = read_object(o->obj_flags.value[2], VIRTUAL))) {
            send_to_char("You dig and dig, but unfornuately, only find a shoe"
                         ,ch);
            /*Print message to all in room*/
            return;
        }

        ch_printf(ch,"You dig and dig long and hard... \n\r"
                "You just discovered %s!!", prize->short_description );
         /* Print info to room*/
        obj_to_room(prize, ch->in_room);
        o->obj_flags.value[3]=1;

    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
