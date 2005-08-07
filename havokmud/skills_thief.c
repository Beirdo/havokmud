/*
 * HavokMUD - thief skills
 * REQUIRED BY - thief
 */
#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "protos.h"
#include "externs.h"

extern struct dex_skill_type dex_app_skill[];

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

    if (!ch->skills[SKILL_BACKSTAB].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (!ch->equipment[WIELD]) {
        send_to_char("You need to wield a weapon to make it a success.\n\r",
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

    stabbersize = race_list[ch->race].size;
    if(affected_by_spell(ch, SPELL_GIANT_GROWTH)) {
        stabbersize++;
    }

    if(IS_NPC(ch) && IS_SET(ch->specials.act, ACT_HUGE)) {
        stabbersize += 2;
    }
    stabbersize = MAX( MIN(stabbersize, 6), 0);


    victimsize = race_list[victim->race].size;
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
                strcat(buf,"$c000GYour stab lands in a vital spot on "
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

void do_steal(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct obj_data *obj;
    char           *victim_name;
    char           *obj_name;
    char            buf[240];
    int             percent;
    int             gold,
                    eq_pos;
    bool            ohoh = FALSE;

    dlog("in do_steal");

    if (!ch->skills) {
        return;
    }
    if (!ch->skills[SKILL_STEAL].learned){
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    if (check_peaceful(ch, "What if he caught you?\n\r")) {
        return;
    }

    argument = get_argument(argument, &obj_name);
    victim_name = argument;


    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }

    if (!obj_name || !victim_name ||
        !(victim = get_char_room_vis(ch, victim_name))) {
        send_to_char("Steal what from who?\n\r", ch);
        return;
    } 
    
    if (victim == ch) {
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

    if (strcasecmp(obj_name, "coins") && strcasecmp(obj_name, "gold")) {
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
                act("$E has not got that item.", FALSE, ch, 0, victim, TO_CHAR);
                return;
            } 
            
            /*
             * It is equipment
             */
            if ((GET_POS(victim) > POSITION_STUNNED)) {
                send_to_char("Steal the equipment now? Impossible!\n\r", ch);
                return;
            } 
            
            act("You unequip $p and steal it.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n steals $p from $N.", FALSE, ch, obj, victim, TO_NOTVICT);
            obj_to_char(unequip_char(victim, eq_pos), ch);
#ifndef DUPLICATES
            do_save(ch, "", 0);
            do_save(victim, "", 0);
#endif
            if (IS_PC(ch) && IS_PC(victim)) {
                GET_ALIGNMENT(ch) -= 20;
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
#ifndef DUPLICATES
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
