/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "protos.h"
#include "externs.h"

/*
 * extern variables
 */

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

void do_backstab(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct affected_type af;
    char           *name;
    int            percent;
    int            base = 0;
    double         lagmod = 1.0;
    int            damagemod = 0;
    int            stabbersize;
    int            victimsize;
    int            sizediff;
    int            howgood;
    int            stunned = 0;
    int            hitrollpenalty = 0;
    int            damrollpenalty = 0;
    char           buf[MAX_STRING_LENGTH];
    int            temp;


    int BSSizeDifferential [7][7] = {
    /* Stabber is vertical, victim is horizontal */
    /* Gen is for a general size mob, which is treated as tiny */
    /* Negative numbers are bad, positive numbers are good, because it is */
    /* subtracted from the backstab roll */
    /*        Gen    T     S    M    L    H    G  */
    /* Gen*/ {  0,   0,   10,  -5, -10, -15, -20},
    /* T */  {  0,   0,   10,  -5, -10, -15, -20},
    /* S */  {-10, -10,    0,  10,  -5, -10, -15},
    /* M */  {-15, -15,  -10,   0,  10,  -5, -10},
    /* L */  {-25, -25,  -15, -10,   0,  10,  -5},
    /* H */  {-40, -40,  -25, -15, -10,   0,  10},
    /* G */  {-60, -60,  -40, -25, -15, -10,   0}
    };

    buf[0] = '\0';


    dlog("in do_backstab");

    if (!ch->skills) {
        return;
    }

    if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n\r")) {
        return;
    }

    argument = get_argument(argument, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        send_to_char("Backstab whom?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("How can you sneak up on yourself?\n\r", ch);
        return;
    }

    if (!HasClass(ch, CLASS_THIEF)) {
        send_to_char("You're no thief!\n\r", ch);
        return;
    }

    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon, to make it a success.\n\r",
                     ch);
        return;
    }

    if (ch->attackers) {
        send_to_char("There's no way to reach that back while you're "
                     "fighting!\n\r", ch);
        return;
    }

    if (A_NOASSIST(ch, victim)) {
        act("$N is already engaged with someone else!", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (victim->attackers >= 3) {
        send_to_char("You can't get close enough to them to backstab!\n\r", ch);
        return;
    }

    if (ch->equipment[WIELD]->obj_flags.value[3] != 11 &&
        ch->equipment[WIELD]->obj_flags.value[3] != 1 &&
        ch->equipment[WIELD]->obj_flags.value[3] != 10) {
        send_to_char("Only piercing or stabbing weapons can be used for "
                     "backstabbing.\n\r", ch);
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("You're too busy to backstab\n\r", ch);
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("How can you surprise someone with all the racket that "
                     "beast makes?\n\r", ch);
        return;
    }

    if (victim->specials.fighting) {
        base = 0;
    } else {
        base = 4;
    }

    if (victim->skills && victim->skills[SKILL_AVOID_BACK_ATTACK].learned &&
        GET_POS(victim) > POSITION_SITTING && 
        !IS_AFFECTED(victim, AFF_PARALYSIS)) {
        percent = number(1, 100);       /* 101% is a complete failure */
        if (percent < victim->skills[SKILL_AVOID_BACK_ATTACK].learned) {
            act("You sense a back attack from $N and avoid it skillfully!",
                FALSE, victim, 0, ch, TO_CHAR);
            act("$n avoids a back attack from $N!", FALSE, victim, 0, ch,
                TO_ROOM);
            /* he avoided, so make him hit! */
            SetVictFighting(ch, victim);
            SetCharFighting(ch, victim);
            AddHated(victim, ch);
            return;
        }

        act("You failed to sense a back attack from $N!", FALSE, victim,
            0, ch, TO_CHAR);
        act("$n fails to avoid a back attack from $N!", FALSE, victim,
            0, ch, TO_ROOM);
        LearnFromMistake(victim, SKILL_AVOID_BACK_ATTACK, 0, 95);
    }

    stabbersize = races[ch->race].size;
    if(affected_by_spell(ch, SPELL_GIANT_GROWTH)) {
        stabbersize++;
    }

    if(IS_NPC(ch) && IS_SET(ch->specials.act, ACT_HUGE)) {
        stabbersize += 2;
    }
    stabbersize = MAX( MIN(stabbersize, 6), 0);


    victimsize = races[victim->race].size;
    if(affected_by_spell(victim, SPELL_GIANT_GROWTH)) {
        victimsize++;
    }

    if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
        victimsize += 2;
    }
    victimsize  = MAX( MIN(victimsize,  6), 0);


    if(victimsize - stabbersize >= 4 && !IS_AFFECTED(ch, AFF_FLYING)) {
    /* There are 4 sizes difference, a small trying to backstab a gigantic */
        send_to_char("$c000WYou would have to be flying to reach a "
                     "back that much out of reach.\n\r", ch);
        return;
    }

    percent = number(1, 100);

    if (ch->skills[SKILL_BACKSTAB].learned) {
        if(percent < 6) {
            /* auto success */
            percent = -1;
        } else if(percent > 95) {
            /* auto failure */
            percent = 101;
        } else {
            /* size, dexterity, level */
            sizediff = BSSizeDifferential[stabbersize][victimsize];
            percent -= sizediff;
            percent += (dex_app[(int) GET_DEX(ch)].defensive / 2);

            if(IS_NPC(ch)) {
                percent -= (GetMaxLevel(ch) - GetMaxLevel(victim));
            } else {
                percent -= (GET_LEVEL(ch, THIEF_LEVEL_IND) - 
                            GetMaxLevel(victim));
            }
        }

        howgood = ch->skills[SKILL_BACKSTAB].learned - percent;
        if(howgood > 0) {
            if(howgood > 50) {
                base += 4;
                strcat(buf, "$c000GYour stab is aimed in precisely "
                            "the right location, the victim will be "
                            "$c000Wstunned "
                            "$c000Gby the impact.");
                stunned = 1;
                hitrollpenalty = ((int) howgood / 10) + 5;
                damrollpenalty = ((int) howgood / 10) + 5;
            } else if(howgood > 20) {
                base += 2;
                strcat(buf,"$c000GYour stab lands in a vital spot on the "
                           "the victim, they will almost certainly be "
                           "$c000Wblinded "
                           "$c000Gby the pain.");
                hitrollpenalty = 5;
                damrollpenalty = 5;
            } else if(howgood > 10) {
                strcat(buf,"$c000gYour stab lands in the "
                           "$c000Wcenter "
                           "$c000gof the victim's back.");
            } else {
                base -= 2;
                strcat(buf,"$c000gYour stab is slightly "
                           "$c000Woff target, "
                           "$c000gbut still fairly potent.");
                temp = (int) GET_DAMROLL(ch) * .10;
                damagemod -= temp;
            }
            
            strcat(buf, "\n\r");
            if(victimsize - stabbersize >= 4) {
                /* if you have to fly to backstab, you only get 80% damroll */
                temp = (int) GET_DAMROLL(ch) * .20;
                damagemod -= temp;
            }

            /* Temporarily boost the hit & damage rolls */
            GET_HITROLL(ch) += base;
            GET_DAMROLL(ch) += damagemod;

            AddHated(victim, ch);
            temp = GET_HIT(victim);
            hit(ch, victim, SKILL_BACKSTAB);

            /* Now set it back to the original */
            GET_DAMROLL(ch) -= damagemod;
            GET_HITROLL(ch) -= base;

            if(victim && temp > GET_HIT(victim) && 
               GET_POS(victim) != POSITION_DEAD) {
                send_to_char(buf,ch);

                if(stunned && GET_POS(victim) > POSITION_STUNNED) {
                    GET_POS(victim) = POSITION_STUNNED;
                    WAIT_STATE(victim, PULSE_VIOLENCE * 2);
                }

                if(hitrollpenalty != 0) {
                    af.type = SKILL_BACKSTAB;
                    af.duration = 1;
                    af.modifier = -hitrollpenalty;
                    af.location = APPLY_HITROLL;
                    af.bitvector = 0;
                    affect_to_char(victim, &af);
                }

                if(damrollpenalty != 0) {
                    af.type = SKILL_BACKSTAB;
                    af.duration = 1;
                    af.modifier = -damrollpenalty;
                    af.location = APPLY_DAMROLL;
                    af.bitvector = 0;
                    affect_to_char(victim, &af);
                }
            }
            gain_exp(ch, 100);
        } else {
            damage(ch, victim, 0, SKILL_BACKSTAB);
            AddHated(victim, ch);

            if(howgood < -50) {
                GET_POS(ch) = POSITION_SITTING;
                lagmod = 1.5;
                strcat(buf,"$c000RYour stab misses horribly, so badly that "
                           "$c000Wyou fall to the ground.  "
                           "$c000RIt will take you some time to recover so "
                           "that you can stand up again.");
            } else if(howgood < -20) {
                lagmod = 1.5;
                strcat(buf,"$c000rYour stab misses badly, it will take you a "
                           "$c000Wlong time "
                           "$c000rto recover your balance.");
            } else if(howgood < -10) {
                lagmod = 1.0;
                strcat(buf,"$c000YYour stab misses, you might need a "
                           "$c000Wfew seconds "
                           "$c000Yto recover your poise.");
            } else {
                lagmod = 0.5;
                strcat(buf,"$c000yYou realize even as you start "
                           "your stroke that you will miss.  You "
                           "$c000Wdraw back, "
                           "$c000yjust in time, and recover quickly.");
            }

            strcat(buf, "\n\r");
            send_to_char(buf,ch);
            LearnFromMistake(ch, SKILL_BACKSTAB, 0, 90);
        }
    }

    if (OnlyClass(ch, CLASS_THIEF)) {
        /* one round lag for sc thieves */
        WAIT_STATE(ch, PULSE_VIOLENCE * lagmod);
    } else {
        /* two rounds lag for multi thieves */
        WAIT_STATE(ch, PULSE_VIOLENCE * 2 * lagmod);
    }
}

void do_dismiss(struct char_data *ch, char *arg, int cmd)
{
    int             bits;
    char           *buf;
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
                    ch_printf(ch, "%s isn't under your control.\n\r",
                                  tmp_char->player.short_descr);
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
                    }
                    
                    if (!die) {
                        act("$n tries to flee, but is too exhausted!",
                            TRUE, ch, 0, 0, TO_ROOM);
                    }
                    return;
                } 
                
                if ((die = MoveOne(ch, attempt)) == 1) {
                    /*
                     * The escape has succeeded
                     */
                    send_to_char("You flee head over heels.\n\r", ch);
                    return;
                }
                
                if (!die) {
                    act("$n tries to flee, but is too exhausted!",
                        TRUE, ch, 0, 0, TO_ROOM);
                }
                return;
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

                if (ch->specials.fighting->specials.fighting == ch) {
                    stop_fighting(ch->specials.fighting);
                }

                if (ch->specials.fighting) {
                    stop_fighting(ch);
                }

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

void do_bash(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;

    int             percent;
    int             goodbash;
    int             bashlearned;
    double          vicwait;
    double          chwait;
    int             vicsize;
    int             chsize;
    int             strbonus;

    int BashSizeDifferential [7][7] = {
    /* Basher is vertical, bashee is horizontal */
    /* Gen is for a general size mob, which is treated as tiny */
    /*        Gen    T     S    M    L    H    G  */
    /* Gen*/ {  0,   0,   -5, -10, -20, -30, -40},
    /* T */  {  0,   0,   -5, -10, -20, -30, -40},
    /* S */  { 10,  10,    0,  -5, -10, -20, -30},
    /* M */  { -5,  -5,   10,   0,  -5, -10, -20},
    /* L */  {-10, -10,   -5,  10,   0,  -5, -10},
    /* H */  {-20, -20,  -10,  -5,  10,   0,  -5},
    /* G */  {-30, -30,  -20, -10,  -5,  10,   0}
    };

    dlog("in do_bash");

    if (!ch->skills) {
        return;
    }

    if (check_peaceful(ch,
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }
    if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        if (!HasClass(ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
                          CLASS_BARBARIAN)) {
            send_to_char("You're no warrior!\n\r", ch);
            return;
        }
    }

    if (A_NOBASH(ch)) {
        send_to_char("The arena rules do not permit you to bash!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);

    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Bash who?\n\r", ch);
            return;
        }
    }

    if(!victim->specials.fighting) {
        set_fighting(victim,ch);
    }
    if(!ch->specials.fighting) {
        set_fighting(ch,victim);
    }

    if (A_NOASSIST(ch, victim)) {
        act("$N is already engaged with someone else!", FALSE, ch, 0,
            victim, TO_CHAR);
        return;
    }

    if (victim == ch || IS_IMMORTAL(victim)) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    if (MOUNTED(victim)) {
        send_to_char("You can't bash a mounted target!\n\r", ch);
        return;
    }
    if (MOUNTED(ch)) {
        send_to_char("You can't bash while mounted!\n\r", ch);
        return;
    }

    if (ch->attackers > 3) {
        send_to_char("There's no room to bash!\n\r", ch);
        return;
    }
    if (victim->attackers >= 6) {
        send_to_char("You can't get close enough to them to bash!\n\r", ch);
        return;
    }
    percent = number(1, 100);   /* 100 is a complete failure, 1 is success */

    if(percent < 6) {
        goodbash = 8;  /* Middle of the road a 2 round lag for both */
    }
    else if(percent > 95) {
        goodbash = -4; /* Middle of the road a 2 round lag miss */
    }
    else {
        /* Dexterity adjustment, 5% per reaction bonus */
        percent += dex_app[(int)GET_DEX(victim)].reaction * 5;
        percent -= dex_app[(int)GET_DEX(ch)].reaction *5;

        /* Size difference between basher and bashee */
        chsize  = races[ch->race].size;
        vicsize = races[victim->race].size;
        if(affected_by_spell(ch, SPELL_GIANT_GROWTH)) {
            chsize++;
        }
        if(IS_NPC(ch) && IS_SET(ch->specials.act, ACT_HUGE)) {
            chsize += 2;
        }
        if(IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
            vicsize += 2;
        }
        if(affected_by_spell(victim, SPELL_GIANT_GROWTH)) {
            vicsize++;
        }
        if(chsize > 6) {
            chsize = 6;
        }
        if(chsize < 0) {
            chsize = 0;
        }
        if(vicsize > 6) {
            vicsize = 6;
        }
        if(vicsize < 0) {
            vicsize = 0;
        }
        /* 062104 - Changed to half differential by request of Gordon */
        percent -= BashSizeDifferential[chsize][vicsize] >> 1;

        /* Level difference between basher and bashee */
        /* 062104 - Changed to only 1% penalty/bonus from 2% per
                    level difference  */
        if(OnlyClass(ch, CLASS_WARRIOR) || OnlyClass(ch, CLASS_BARBARIAN) ||
            OnlyClass(ch, CLASS_PALADIN) || OnlyClass(ch, CLASS_RANGER)) {
            percent -= (GET_LEVEL(ch,BestFightingClass(ch)) -
                        GetMaxLevel(victim)) * 1;
        }
        /* 071204 - Added in check for main class, worse than s/c,
           better than m/c */
        else if(ch->specials.remortclass == WARRIOR_LEVEL_IND + 1 ||
                ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1 ||
                ch->specials.remortclass == PALADIN_LEVEL_IND + 1 ||
                ch->specials.remortclass == RANGER_LEVEL_IND + 1){
            percent -= (((int) (GET_LEVEL(ch,BestFightingClass(ch)) * 0.95)) -
                         GetMaxLevel(victim)) * 1;
        }
        /* 062104 - Changed to only 10% penalty from 20% for multi-class */
        else {
            percent -= (((int) (GET_LEVEL(ch,BestFightingClass(ch)) * 0.9)) -
                         GetMaxLevel(victim)) * 1;
    }

        bashlearned = ch->skills[SKILL_BASH].learned;

        if(percent < bashlearned) {
            if(percent * 3 < bashlearned) {
                /* Wow */
                goodbash = 6;
            }
            else if(percent * 2.5 < bashlearned) {
                /* Very Good */
                goodbash = 5;
            }
            else if(percent * 2.0 < bashlearned) {
                /* Good */
                goodbash = 4;
            }
            else if(percent * 1.5 < bashlearned) {
                /* Not Bad */
                goodbash = 3;
            }
            else if(percent * 1.25 < bashlearned) {
                goodbash = 2;
                /* Made It */
            }
            else {
                goodbash = 1;
                /* Squeaked By */
            }
        }
        else {
            if(percent - bashlearned > 40) {
                /* Ouch! */
                goodbash = -6;
            }
            else if(percent - bashlearned > 30) {
                /* Whoops! */
                goodbash = -5;
            }
            else if(percent - bashlearned > 20) {
                /* Missed! */
                goodbash = -4;
            }
            else if(percent - bashlearned > 10) {
                /* Just Missed! */
                goodbash = -3;
            }
            else if(percent - bashlearned > 5) {
                goodbash = -2;
                /* How Did I Miss? */
            }
            else {
                goodbash = -1;
                /* The Gods Must Smile On My Opponent */
            }
        }
    }

    if(goodbash > 0) {
        if(GET_POS(victim) > POSITION_SITTING) {
        GET_POS(victim) = POSITION_SITTING;
        }
        strbonus = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
        if(strbonus > 0) {
            goodbash += (strbonus >> 1) + (strbonus & 1);
            goodbash += number(1,(strbonus >> 1) + (strbonus & 1));
        }
        if(strbonus < 0) {
            strbonus = strbonus * -1;
            goodbash -= (strbonus >> 1) + (strbonus & 1);
            goodbash -= number(1,(strbonus >> 1) + (strbonus & 1));
        }

        /* 
         *  if your strength damage bonus is 0, then your range for
         *  successful bashes is: 1 through 6, which means you will
         *  never have a chance at a really good bash.
         *  the minimum is calculated using this formula:
         *  x = 1/2 damage bonus rounded up (+1 = 1, +2 = 1, +3 = 2)
         *  minimum = 2 + x
         *  so for a 16 strength, with a +1 damage bonus your minimum is:
         *  2 + 1 = 3
         *  an 18/80 strength is: 2 + 2 = 4
         *  a 19 strength is 2 + 4 = 6
         *  for maximums you use this formula:
         *  maximum = 6 + 2x
         *  so for a 16 strength, with a +1 damage bonus your maximum is:
         *  6 + 2 = 8
         *  an 18/80 strength is: 6 + 4 = 10
         *  a 19 strength is: 6 + 8 = 14
         */
        if(goodbash < 3) {
            /* Dark Green */
            vicwait = 1;
            chwait = 1.5;
            act("$c000gYou push $N, sending $M to the ground but you "
                "lose your balance and $c000Walmost hit the ground "
                "$c000gvery hard yourself!",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000g$n pushes you to the ground, but $e loses $s"
                " balance badly, and $c000Walmost hits the ground.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000g$n pushes $N to the ground, but loses $s "
                "balance and $c000Walmost hits the ground $c000g$mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        }
        else if(goodbash < 6) {
            /* Dark Green */
            vicwait = 1;
            chwait = 1;
            act("$c000gYou barely manage to slam into $N, sending $M "
                "to the ground, but $E $c000Wrecovers quickly.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000g$n stumbles and slams into you, pushing you "
                "to the ground, but you $c000Wrecover quickly, "
                "$c000geven faster than $m.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000g$n slams $N to the ground, but stumbles and "
                "$N $c000Wrecovers quickly.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else if(goodbash < 9) {
            /* Bright Green */
            vicwait = 2;
            chwait = 2;
            act("$c000GYou slam into $N, sending $M $c000Wsprawling "
                "$c000Gto the ground.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000G$n slams into you, sending you $c000Wsprawling "
                "$c000Gto the ground.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000G$n slams into $N, sending $M $c000Wsprawling "
                "$c000Gto the ground.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else { 
            /* Bright Green */
            vicwait = 3;
            chwait = 2;
            act("$c000GYou slam into $N, sending $M $c000Wsoaring "
                "$c000Gthrough the air to land in a crumpled heap on the "
                "ground, $E won't be getting up anytime soon.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000G$n slams into you, you go $c000Wsoaring $c000Gthrough "
                "the air to hit the ground very hard, fireworks go off in "
                "your head.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000G$n slams into $N, who goes $c000Wsoaring "
                "$c000Gthrough the air to land in a crumpled heap several "
                "yards away.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        }
    } else {
        if(GET_POS(ch) > POSITION_SITTING) {
        GET_POS(ch) = POSITION_SITTING;
        }
        if(goodbash <= -6) {
            /* Red */
            vicwait = 0;
            chwait = 3;
            act("$c000RYou throw yourself at $N, who sidesteps you neatly."
                "You end up $c000Wfalling flat on your face"
                "$c000R, making you dizzy and disoriented.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000R$n attempts to slam into you, but you sidestep "
                "the clumsy fool and $e $c000Wfalls flat on $s face.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000R$n attempts to slam $N to the ground, but $N "
                "sidesteps $m neatly, and $n ends up $c000Wfalling flat on "
                "$s face.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else if(goodbash <= -5) {
            /* Red */
            vicwait = 0;
            chwait = 2.5;
            act("$c000RYou aim to slam into $N, but stumble short of "
                "your target and end up $c000Wsitting on the ground "
                "$c000Rwithout putting $M in any danger.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000R$n tries to charge at you, but stumbles and ends up "
                "$c000Wsitting on the ground $c000Rin front of you.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000R$n tries to slam into $N, but trips and doesn't "
                "come even close and actually ends up $c000Wsiting on the "
                "ground $c000Rin front of $N looking stupid.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else if(goodbash <= -4) {
            /* Red */
            vicwait = 0;
            chwait = 2;
            act("$c000RYou attempt to slam $N to the ground, but miss "
                "$c000Wbadly$c000R, which puts you on the ground instead.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000R$n attempts to ram $s shoulder into you, "
                "but $e misjudges $c000Wbadly $c000Rand ends up sprawled on "
                "the ground.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000R$n attempts to slam into $N, but misses $c000Wbadly "
                "$c000Rand ends up sprawled on the ground instead.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else if(goodbash <= -3) {
            /* Yellow */
            vicwait = 0;
            chwait = 1.5;
            act("$c000YYou attempt to slam $N to the ground, but miss, which "
                "puts you on the ground, but in a $c000Wgood position "
                "$c000Yto recover.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000Y$n attempts to ram $s shoulder into you, but $e "
                "misjudges and ends up on the ground, but in a "
                "$c000Wgood position $c000Yto recover",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000Y$n attempt to slam into $N, but misses and ends up "
                "on the ground instead, but in $c000Wgood position "
                "$c000Yto scramble and recover.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else if(goodbash <= -2) {
            /* Yellow */
            vicwait = 0;
            chwait = 1;
            act("$c000YYou attempt to slam $N to the ground, but just "
                "miss, which puts you on the ground, but already you are "
                "almost ready to $c000Wsurge back to your feet.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000Y$n attempts to ram $s shoulder into you, "
                "but $e misjudges and ends up on the ground, but is "
                "$c000Wsurging back to $s feet.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000Y$n attempt to slam into $N, but misses and ends up "
                "on the ground instead.  However, $e is recovering quickly "
                "and already $c000Wsurging back to their feet.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        } else {
            /* Yellow */
            vicwait = 0;
            chwait = .5;
            act("$c000YYou barely miss slamming into $N, which sends "
                "you stumbling to one knee, but you are almost "
                "$c000Winstantly ready to try again.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$c000Y$n just misses slamming into you, he drops to one "
                "knee to recover, and is almost $c000Winstantly ready to try "
                "again.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$c000Y$n just misses slamming into $N, but $e "
                "recovers quickly, dropping to one knee and is "
                "$c000Winstantly ready to try again.",
                FALSE, ch, 0, victim, TO_NOTVICT);
        }
        LearnFromMistake(ch, SKILL_BASH, 0, 90);
    }
    
    if(chwait > 0) {
        WAIT_STATE(ch, chwait * PULSE_VIOLENCE);
    }

    if(vicwait > 0) {
        WAIT_STATE(victim, vicwait * PULSE_VIOLENCE);
    }
}

void do_leg_sweep(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    byte            percent;

    dlog("in do_leg_sweep");

    if (!ch->skills) {
        return;
    }

    if (check_peaceful
        (ch, "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_MONK)) {
        send_to_char("You're no monk!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Who do you wish to legsweep??\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE) &&
        !IsGiant(ch)) {
        act("$N is MUCH too large to legsweep!", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (MOUNTED(victim)) {
        send_to_char("You'll end up booting your targets mount if you do "
                     "that!\n\r", ch);
        return;
    }

    if (MOUNTED(ch)) {
        send_to_char("You'll end up booting your mount if you do that!\n\r",
                     ch);
        return;
    }

    if (ch->attackers > 2) {
        send_to_char("There are too many people around to do that!\n\r", ch);
        return;
    }

    if (victim->attackers >= 4) {
        send_to_char("You'll end up giving your buddies a good boot if you "
                     "do that!\n\r", ch);
        return;
    }

    percent = number(1, 101);
    /*
     * 101% is a complete failure
     *
     *
     * some modifications to account for dexterity, and level
     */
    percent -= dex_app[(int)GET_DEX(ch)].reaction * 10;
    percent += dex_app[(int)GET_DEX(victim)].reaction * 10;
    if (GetMaxLevel(victim) > 12) {
        percent += ((GetMaxLevel(victim) - 10) * 5);
    }

    if (percent > ch->skills[SKILL_LEG_SWEEP].learned) {
        if (GET_POS(victim) > POSITION_DEAD) {
            damage(ch, victim, 0, SKILL_LEG_SWEEP);
            GET_POS(ch) = POSITION_SITTING;
        }
        act("$c000CYou try to do quick spin and knock $N's legs out, but "
            "miss.", FALSE, ch, 0, victim, TO_CHAR);
        act("$c000C$n tries to do a quick spin and knock $N's legs out, "
            "but misses $M.", FALSE, ch, 0, victim, TO_ROOM);
        LearnFromMistake(ch, SKILL_LEG_SWEEP, 0, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    } else {
        if (GET_POS(victim) > POSITION_DEAD) {
            GET_POS(victim) = POSITION_SITTING;
            if (!damage(ch, victim, 9, SKILL_LEG_SWEEP)) {
                WAIT_STATE(victim, PULSE_VIOLENCE * 2);
                GET_POS(victim) = POSITION_SITTING;
                act("$c000CYou do a quick spin and knock $N's legs out from "
                    "underneath $M.", FALSE, ch, 0, victim, TO_CHAR);
                act("$c000C$n does a quick spin and knocks $N's legs out from "
                    "underneath $M.", FALSE, ch, 0, victim, TO_NOTVICT);
                act("$c000C$n does a quick spin and knocks your legs out from "
                    "underneath you.", FALSE, ch, 0, victim, TO_VICT);
                ch_printf(ch, "$c000BYou receive $c000W100 $c000Bexperience "
                              "for using your combat abilities.$c0007\n\r");
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            } else {
                act("$c000CYour legsweep lands a killing blow to $M.",
                    FALSE, ch, 0, victim, TO_CHAR);
                act("$c000C$n's legsweep lands a killing blow to $M.",
                    FALSE, ch, 0, victim, TO_ROOM);
                ch_printf(ch, "$c000BYou receive $c000W100 $c000Bexperience "
                              "for using your combat abilities.$c0007\n\r");
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            }
        }
    }
}

void do_mend(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct obj_data *cmp;
    char            buf[254],
                   *arg;
    int             perc = 0;

    dlog("in do_mend");

    if (!ch) {
        return;
    }

    if (!IS_IMMORTAL(ch) && ch->specials.remortclass != WARRIOR_LEVEL_IND + 1) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (!IS_SET(ch->skills[SKILL_MEND].flags, SKILL_KNOWN)) {
        send_to_char("You don't know this skill.\n\r", ch);
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
                Log("%s tried to mend an invalid armor value: %s, vnum %ld.",
                    GET_NAME(ch), obj->short_description,
                    (obj->item_number >= 0) ?
                     obj_index[obj->item_number].virtual : 0);
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
                ch_printf(ch, "You try to mend %s, but make matters worse.\n\r",
                              obj->short_description);
                LearnFromMistake(ch, SKILL_MEND, 0, 95);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                return;
            } 
            
            obj->obj_flags.value[0] = obj->obj_flags.value[1];
            sprintf(buf, "%s expertly mends %s.", GET_NAME(ch),
                    obj->short_description);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            ch_printf(ch, "You expertly mend %s.\n\r", obj->short_description);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
            WAIT_STATE(ch, PULSE_VIOLENCE * 1);
            return;
        } 
        
        if (ITEM_TYPE(obj) == ITEM_WEAPON) {
            if (obj->obj_flags.value[2] == 0) {
                Log("%s tried to mend an weapon with invalid value: %s, "
                    "vnum %d.", GET_NAME(ch), obj->short_description,
                                obj->item_number);
                return;
            }

            cmp = read_object(obj->item_number, REAL);
            if (cmp->obj_flags.value[2] == 0) {
                Log("%s tried to mend an weapon with invalid value: %s, "
                    "vnum %d.", GET_NAME(ch), obj->short_description,
                                obj->item_number);
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
                    ch_printf(ch, "You try to mend %s, but only make things "
                                  "worse.\n\r", obj->short_description);
                    /*
                     * did this scrap the weapon?
                     */
                    if (obj->obj_flags.value[2] < 1) {
                        sprintf(buf, "%s screwed up so bad that %s is reduced "
                                     "to junk!",
                                GET_NAME(ch), obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);
                        ch_printf(ch, "You screwed up so bad that %s is reduced"
                                      " to junk!\n\r", obj->short_description);
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
                    ch_printf(ch, "You expertly mend %s.\n\r",
                                  obj->short_description);
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
        send_to_char("Mend what?\n\r", ch);
    }
}

void do_rescue(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim,
                   *tmp_ch;
    int             percent;
    char           *victim_name;

    dlog("in do_rescue");
    if (!ch->skills) {
        send_to_char("You fail the rescue.\n\r", ch);
        return;
    }
#if 0
    if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act, ACT_POLYSELF))
        return;
#endif

    if (check_peaceful(ch, "No one should need rescuing here.\n\r"))
        return;

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_PALADIN |
                      CLASS_RANGER)) {
        send_to_char("You're no warrior!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &victim_name);

    if (!victim_name || !(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Who do you want to rescue?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("What about fleeing instead?\n\r", ch);
        return;
    }

    if (MOUNTED(victim)) {
        send_to_char("You can't rescue a mounted person!\n\r", ch);
        return;
    }

    if (ch->specials.fighting == victim) {
        send_to_char("How can you rescue someone you are trying to kill?\n\r",
                     ch);
        return;
    }

    if (victim->attackers >= 3) {
        send_to_char("You can't get close enough to them to rescue!\n\r",
                     ch);
        return;
    }

    for (tmp_ch = real_roomp(ch->in_room)->people;
         tmp_ch && tmp_ch->specials.fighting != victim;
         tmp_ch = tmp_ch->next_in_room){
        /*
         * Yes, this is an empty loop body
         */
    }
    if (!tmp_ch) {
        act("But nobody is fighting $M?", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    percent = number(1, 101);
    /*
     * 101% is a complete failure
     */

    if ((percent > ch->skills[SKILL_RESCUE].learned)) {
        send_to_char("You fail the rescue.\n\r", ch);
        LearnFromMistake(ch, SKILL_RESCUE, 0, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    send_to_char("Banzai! To the rescue...\n\r"
                 "$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "combat abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
    act("You are rescued by $N, you are confused!", FALSE, victim, 0, ch,
        TO_CHAR);
    act("$n heroically rescues $N.", FALSE, ch, 0, victim, TO_NOTVICT);
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) += 20;
    }
    if (victim->specials.fighting == tmp_ch) {
        stop_fighting(victim);
    }
    if (tmp_ch->specials.fighting) {
        stop_fighting(tmp_ch);
    }
    if (ch->specials.fighting) {
        stop_fighting(ch);
    }
    set_fighting(ch, tmp_ch);
    set_fighting(tmp_ch, ch);

    WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
}

void do_disengage(struct char_data *ch, char *argument, int cmd)
{
    int             percent;
    char            buf[256];

    if (argument && strcmp(argument, "-skill") == 0) {
        ch_printf(ch, "Disengage Skill----->%s.\n\r",
                      how_good(ch->skills[SKILL_DISENGAGE].learned));
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

void do_kick(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    int             dam;
    byte            percent;

    dlog("in do_kick");

    if (!ch->skills) {
        return;
    }
    if (check_peaceful(ch,
                    "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_RANGER |
                      CLASS_PALADIN) && !HasClass(ch, CLASS_MONK)) {
        send_to_char("You're no warrior or monk!\n\r", ch);
        return;
    }

#if 0
    if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        return;
    }
#endif

    argument = get_argument(argument, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Kick who?\n\r", ch);
            return;
        }
    }
    
    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }
    
    if (MOUNTED(victim)) {
        send_to_char("You can't kick while mounted!\n\r", ch);
        return;
    }
    
    if (ch->attackers > 3) {
        send_to_char("There's no room to kick!\n\r", ch);
        return;
    }
    
    if (victim->attackers >= 4) {
        send_to_char("You can't get close enough to them to kick!\n\r", ch);
        return;
    }
    
    percent = ((10 - (GET_AC(victim) / 10))) + number(1, 101);
    
    /*
     * 101% is a complete failure
     */
    if (GET_RACE(victim) == RACE_GHOST) {
        kick_messages(ch, victim, 0);
        SetVictFighting(ch, victim);
        return;
    } else if (!IS_NPC(victim) && (GetMaxLevel(victim) > MAX_MORT)) {
        kick_messages(ch, victim, 0);
        SetVictFighting(ch, victim);
        SetCharFighting(ch, victim);
        return;
    }
    
    if (GET_POS(victim) <= POSITION_SLEEPING) {
        percent = 1;
    }
    
    if (percent > ch->skills[SKILL_KICK].learned) {
        if (GET_POS(victim) > POSITION_DEAD) {
            damage(ch, victim, 0, SKILL_KICK);
            kick_messages(ch, victim, 0);
        }
        LearnFromMistake(ch, SKILL_KICK, 0, 90);
    } else {
        if (GET_POS(victim) > POSITION_DEAD) {
            dam = GET_LEVEL(ch, BestFightingClass(ch));
            if (!HasClass(ch, CLASS_MONK) || IS_NPC(ch)) {
                /*
                 * so guards use fighter dam
                 */
                dam = dam >> 1;
            }
            kick_messages(ch, victim, dam);
            damage(ch, victim, dam, SKILL_KICK);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
        WAIT_STATE(victim, PULSE_VIOLENCE);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
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
    char           *name;
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
            Log("monster %s has no breath weapons", ch->player.short_descr);
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

void do_springleap(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    byte            percent;

    dlog("in do_springleap");

    if (!ch->skills) {
        return;
    }

    if (check_peaceful(ch,
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_MONK)) {
        send_to_char("You're no monk!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Spring-leap at who?\n\r", ch);
            return;
        }
    }

    if (GET_POS(ch) > POSITION_SITTING || !ch->specials.fighting) {
        send_to_char("You're not in position for that!\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    if (ch->attackers > 3) {
        send_to_char("There's no room to spring-leap!\n\r", ch);
        return;
    }

    if (victim->attackers >= 3) {
        send_to_char("You can't get close enough\n\r", ch);
        return;
    }

    percent = number(1, 101);
    act("$n does a really nifty move, and aims a leg towards $N.", FALSE,
        ch, 0, victim, TO_ROOM);
    act("You leap off the ground at $N.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n leaps off the ground at you.", FALSE, ch, 0, victim, TO_VICT);

    if (percent > ch->skills[SKILL_SPRING_LEAP].learned) {
        if (GET_POS(victim) > POSITION_DEAD) {
            damage(ch, victim, 0, SKILL_KICK);
            LearnFromMistake(ch, SKILL_SPRING_LEAP, 0, 90);
            send_to_char("You fall on your butt\n\r", ch);
            act("$n falls on $s butt", FALSE, ch, 0, 0, TO_ROOM);
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    } else {
        if (HitOrMiss(ch, victim, CalcThaco(ch))) {
            if (GET_POS(victim) > POSITION_DEAD) {
                damage(ch, victim,
                       GET_LEVEL(ch, BestFightingClass(ch)) >> 1,
                       SKILL_KICK);
            }
            kick_messages(ch, victim,
                          GET_LEVEL(ch, BestFightingClass(ch)) >> 1);
        } else {
            kick_messages(ch, victim, 0);
            damage(ch, victim, 0, SKILL_KICK);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
        WAIT_STATE(victim, PULSE_VIOLENCE);
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    GET_POS(ch) = POSITION_STANDING;
    update_pos(ch);
}

void do_quivering_palm(struct char_data *ch, char *arg, int cmd)
{
    struct char_data *victim;
    struct affected_type af;
    byte            percent;
    char           *name;

    dlog("in do_quivering");

    if (!ch->skills) {
        return;
    }

    if (check_peaceful(ch,
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_MONK)) {
        send_to_char("You're no monk!\n\r", ch);
        return;
    }

    arg = get_argument(arg, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("Use the fabled quivering palm on who?\n\r", ch);
            return;
        }
    }

    if (ch->attackers > 3) {
        send_to_char("There's no room to use that skill!\n\r", ch);
        return;
    }

    if (victim->attackers >= 3) {
        send_to_char("You can't get close enough\n\r", ch);
        return;
    }

    if (!IsHumanoid(victim)) {
        send_to_char("You can only do this to humanoid opponents\n\r", ch);
        return;
    }

    send_to_char("You begin to work on the vibrations\n\r", ch);
    if (affected_by_spell(ch, SKILL_QUIV_PALM)) {
        send_to_char("You can only do this once per week\n\r", ch);
        return;
    }

    percent = number(1, 101);
    if (percent > ch->skills[SKILL_QUIV_PALM].learned) {
        send_to_char("The vibrations fade ineffectively\n\r", ch);
        if (GET_POS(victim) > POSITION_DEAD) {
            LearnFromMistake(ch, SKILL_QUIV_PALM, 0, 95);
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return;
    } else {
        if (GET_MAX_HIT(victim) > GET_MAX_HIT(ch) * 2
            || GetMaxLevel(victim) > GetMaxLevel(ch)) {
            damage(ch, victim, 0, SKILL_QUIV_PALM);
            return;
        }
        if (HitOrMiss(ch, victim, CalcThaco(ch))) {
            if (GET_POS(victim) > POSITION_DEAD) {
                damage(ch, victim, GET_MAX_HIT(victim) * 20,
                       SKILL_QUIV_PALM);
            }
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
    }
    WAIT_STATE(ch, PULSE_VIOLENCE * 1);

    af.type = SKILL_QUIV_PALM;
    af.duration = 168;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void kick_messages(struct char_data *ch, struct char_data *victim,
                   int damage)
{
    int             i;
    char            buf[MAX_STRING_LENGTH];
    char            color[] = "$c0003";
    int             index;

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
        index = MSG_KICK_MISS_SELF;
    } else if (GET_HIT(victim) -
               DamageTrivia(ch, victim, damage, SKILL_KICK) < -10) {
        index = MSG_KICK_KILL_SELF;
    } else {
        index = MSG_KICK_HIT_SELF;
    }

    if( kickMessages[i].msg[index] ) {
        strcpy(buf, color);
        strcat(buf, kickMessages[i].msg[index]);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_CHAR);
    }

    if( kickMessages[i].msg[index+1] ) {
        strcpy(buf, color);
        strcat(buf, kickMessages[i].msg[index+1]);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_VICT);
    }

    if( kickMessages[i].msg[index+2] ) {
        strcpy(buf, color);
        strcat(buf, kickMessages[i].msg[index+2]);
        act(buf, FALSE, ch, ch->equipment[WIELD], victim, TO_NOTVICT);
    }
}

void do_berserk(struct char_data *ch, char *arg, int cmd)
{
    int             skillcheck = 0;
    char           *name;
    struct char_data *victim;

    dlog("in do_berserk");

    if (!ch->skills) {
        send_to_char("You do not know any skills!\n\r", ch);
        return;
    }

    if (check_peaceful(ch,
                       "You feel too peaceful to contemplate violence.\n\r")) {
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_BARBARIAN) && cmd != 0) {
        send_to_char("You're no berserker!\n\r", ch);
        return;
    }

#if 0
    if (!IS_PC(ch) && cmd && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        return;
    }
#endif

    arg = get_argument(arg, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char("You need to begin fighting before you can go "
                         "berserk.\n\r", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    if (MOUNTED(victim)) {
        send_to_char("You can't berserk while mounted!\n\r", ch);
        return;
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
        send_to_char("You are already berserked!\n\r", ch);
        return;
    }

    if (!GET_MANA(ch) >= 15) {
        send_to_char("You do not have the energy to go berserk!\n\r", ch);
        return;
    }

    /*
     * all the checks passed, now get on with it!
     */

    skillcheck = number(0, 101);
    if (skillcheck > ch->skills[SKILL_BERSERK].learned) {
        act("$c1012$n growls at $mself, and looks very angry!", FALSE, ch,
            0, victim, TO_ROOM);
        act("$c1012You can't seem to get mad enough right now.", FALSE, ch,
            0, 0, TO_CHAR);
        LearnFromMistake(ch, SKILL_BERSERK, 0, 90);
    } else {
        if (GET_POS(victim) > POSITION_DEAD) {
            /*
             * cost 15 mana to do it..
             */
            GET_MANA(ch) = (GET_MANA(ch) - 15);
            SET_BIT(ch->specials.affected_by2, AFF2_BERSERK);
            act("$c1012$n growls at $mself, and whirls into a killing frenzy!",
                FALSE, ch, 0, victim, TO_ROOM);
            act("$c1012The madness overtakes you quickly!", FALSE, ch, 0,
                0, TO_CHAR);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your combat abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
        }
        WAIT_STATE(victim, PULSE_VIOLENCE);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
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
        Log("(%s) can't load (%s) because it requires (%d) strength to wield",
            GET_NAME(ch), fw->name, fw->obj_flags.value[0]);
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
            send_to_char("Your hands are to full to unload.\n\r", ch);
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

        send_to_char("OK.\n\r", ch);
        obj_from_char(throw);

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

        send_to_char("OK.\n\r", ch);
        obj_from_char(throw);
        act("$n throws $p!", TRUE, ch, throw, 0, TO_ROOM);
        throw_weapon(throw, tdir, targ, ch);
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
    ch_printf(ch, "You just initated %s into the clan, %s.\n\r",
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
        send_to_char("Polymorphed worldwide comms has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
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
        send_to_char("Polymorphed worldwide comms has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
