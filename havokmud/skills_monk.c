#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

/* disarm */
/* dodge */

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

/* hide */
/* kick */

void do_leg_sweep(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    byte            percent;
    char            buf[100];

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
                sprintf(buf, "$c000BYou receive $c000W100 $c000Bexperience "
                             "for using your combat abilities.$c0007\n\r");
                send_to_char(buf, ch);
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            } else {
                act("$c000CYour legsweep lands a killing blow to $M.",
                    FALSE, ch, 0, victim, TO_CHAR);
                act("$c000C$n's legsweep lands a killing blow to $M.",
                    FALSE, ch, 0, victim, TO_ROOM);
#if 0
                   act("$c000C$n's legsweep lands a killing blow to your "
                   "head.",FALSE, ch,0,victim,TO_VICT);
#endif
                sprintf(buf, "$c000BYou receive $c000W100 $c000Bexperience "
                             "for using your combat abilities.$c0007\n\r");
                send_to_char(buf, ch);
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            }
        }
    }
}
/* pick lock */

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
/* retreat is built into flee */
/* safe fall is an inate ability with only a check in !fly rooms */
/* sneak */

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

