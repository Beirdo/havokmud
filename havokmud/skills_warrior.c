/*
 * HavokMUD - warrior skills
 */
#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

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
        chsize = race_list[ch->race].size;
        vicsize = race_list[victim->race].size;
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
 * doorbash start
 */
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

void do_mend(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct obj_data *cmp;
    char            buf[254],
                   *arg;
    int             perc = 0;
    extern struct index_data *obj_index;

    dlog("in do_mend");

    if (!ch) {
        return;
    }

    if (!IS_IMMORTAL(ch)
        && (ch->specials.remortclass != WARRIOR_LEVEL_IND + 1)) {
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
                sprintf(buf, "%s tried to mend an invalid armor value: %s, "
                             "vnum %ld.",
                        GET_NAME(ch), obj->short_description,
                        (obj->item_number >= 0) ?
                         obj_index[obj->item_number].virtual : 0);
                Log(buf);
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
                sprintf(buf, "You try to mend %s, but make matters worse.\n\r",
                        obj->short_description);
                send_to_char(buf, ch);
                LearnFromMistake(ch, SKILL_MEND, 0, 95);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                return;
            } 
            
            obj->obj_flags.value[0] = obj->obj_flags.value[1];
            sprintf(buf, "%s expertly mends %s.", GET_NAME(ch),
                    obj->short_description);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "You expertly mend %s.\n\r", obj->short_description);
            send_to_char(buf, ch);
            send_to_char("$c000BYou receive $c000W100 $c000Bexperience for "
                         "using your abilities.$c0007\n\r", ch);
            gain_exp(ch, 100);
            WAIT_STATE(ch, PULSE_VIOLENCE * 1);
            return;
        } 
        
        if (ITEM_TYPE(obj) == ITEM_WEAPON) {
            if (obj->obj_flags.value[2] == 0) {
                sprintf(buf, "%s tried to mend an weapon with invalid "
                             "value: %s, vnum %d.",
                        GET_NAME(ch), obj->short_description,
                        obj->item_number);
                Log(buf);
                return;
            }

            cmp = read_object(obj->item_number, REAL);
            if (cmp->obj_flags.value[2] == 0) {
                sprintf(buf, "%s tried to mend an weapon with invalid "
                             "value: %s, vnum %d.",
                        GET_NAME(ch), obj->short_description,
                        obj->item_number);
                Log(buf);
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
                    sprintf(buf, "You try to mend %s, but only make things "
                                 "worse.\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                    /*
                     * did this scrap the weapon?
                     */
                    if (obj->obj_flags.value[2] < 1) {
                        sprintf(buf, "%s screwed up so bad that %s is reduced "
                                     "to junk!",
                                GET_NAME(ch), obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);
                        sprintf(buf, "You screwed up so bad that %s is reduced"
                                     " to junk!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);
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
                    sprintf(buf, "You expertly mend %s.\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);
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
        sprintf(buf, "Mend what?\n\r");
        send_to_char(buf, ch);
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

    send_to_char("Banzai! To the rescue...\n\r", ch);
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
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

#define TAN_SHIELD   67
#define TAN_JACKET   68
#define TAN_BOOTS    69
#define TAN_GLOVES   70
#define TAN_LEGGINGS 71
#define TAN_SLEEVES  72
#define TAN_HELMET   73
#define TAN_BAG      14
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */










