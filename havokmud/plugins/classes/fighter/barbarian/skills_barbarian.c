/*
 * HavokMUD - barbarian class skills
 * REQUIRED BY - barbarian
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

extern struct dex_skill_type dex_app_skill[];

/* avoid backattack is an innate skill checked for in backstab */

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

    arg = get_argument(arg, &name);
    if (!name || !(victim = get_char_room_vis(ch, name))) {
        if (ch->specials.fighting) {
            victim = ch->specials.fighting;
        } else {
            send_to_char
                ("You need to begin fighting before you can go berserk.\n\r",
                 ch);
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

    skillcheck = number(0, 101);
    if (skillcheck > ch->skills[SKILL_BERSERK].learned) {
        act("$c1012$n growls at $mself, and looks very angry!", FALSE, ch,
            0, victim, TO_ROOM);
        act("$c1012You can't seem to get mad enough right now.", FALSE, ch,
            0, 0, TO_CHAR);
        LearnFromMistake(ch, SKILL_BERSERK, 0, 90);
    } else {
        if (GET_POS(victim) > POSITION_DEAD) {
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

void do_camouflage(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_camouflage");
    if (!ch->skills[SKILL_CAMOUFLAGE].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }
    
    send_to_char("You attempt to camouflage yourself.\n\r", ch);
    
    if (!OUTSIDE(ch)) {
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
    if (percent > ch->skills[SKILL_CAMOUFLAGE].learned +
        dex_app_skill[(int)GET_DEX(ch)].hide) {
        LearnFromMistake(ch, SKILL_CAMOUFLAGE, 1, 90);
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */


