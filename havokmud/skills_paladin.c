/*
 * HavokMUD - paladin spells and skills
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

void spell_aura_power(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * +2 to hit +2 dam
     */

    if (affected_by_spell(victim, SPELL_AURA_POWER)) {
        send_to_char("You already feel an aura of power surrounding you.\n\r",
                     ch);
        return;
    }

    act("$n suddenly has a godly aura surrounding $m.", FALSE, victim, 0,
        0, TO_ROOM);
    send_to_char("You suddenly feel a godly aura surrounding you.\n\r", victim);

    af.type = SPELL_AURA_POWER;
    af.duration = 10;
    af.modifier = 2;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = 2;
    affect_to_char(victim, &af);
}

void cast_aura_power(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_aura_power(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in aura of power.");
        break;
    }
}

#if 0
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
#endif

void do_blessing(struct char_data *ch, char *argument, int cmd)
{
    int             rating,
                    factor,
                    level;
    struct char_data *test,
                   *dude;
    struct affected_type af;
    char           *dude_name;

    if (!ch->skills) {
        return;
    }

    argument = get_argument(argument, &dude_name);

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

    if (!dude_name || !(dude = get_char_room_vis(ch, dude_name))) {
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
#if 0
void spell_calm(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    /*
     * removes aggressive bit from monsters
     */
    if (IS_NPC(victim)) {
        if (IS_SET(victim->specials.act, ACT_AGGRESSIVE) ||
            IS_SET(victim->specials.act, ACT_META_AGG)) {
            if (HitOrMiss(ch, victim, CalcThaco(ch))) {
                if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
                    REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
                }
                if (IS_SET(victim->specials.act, ACT_META_AGG)) {
                    REMOVE_BIT(victim->specials.act, ACT_META_AGG);
                }
                send_to_char("You sense peace.\n\r", ch);
            }
        } else {
            send_to_char("You feel calm\n\r", victim);
        }
    } else {
        send_to_char("You feel calm.\n\r", victim);
    }
}


void cast_calm(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_calm(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_calm(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_calm(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            spell_calm(level, ch, tar_ch, 0);
        }
        break;
    default:
        Log("Serious screw-up in continual light!");
        break;
    }
}
#endif

void do_charge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    byte            percent;

    dlog("in do_charge");

    if (!ch->skills) {
        return;
    }
    if (check_peaceful(ch, "Naughty, naughty.  None of that here.\n\r")) {
        return;
    }

    argument = get_argument(argument, &name);

    if (!name || !(victim = get_char_room_vis(ch, name))) {
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

void spell_circle_protection(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (!victim) {
        victim = ch;
    }

    if (!affected_by_spell(ch, SPELL_CIRCLE_PROTECTION)) {
        act("$n prays in a strange language and is surrounded by a"
            " circle of protection!", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You pray in a strange language and are surrounded by a"
                     " circle of protection!\n\r", ch);

        if (!IS_AFFECTED(ch, SPELL_PROT_COLD)) {
            af.type = SPELL_CIRCLE_PROTECTION;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            af.bitvector = 0;
            af.location = APPLY_IMMUNE;
            af.modifier = IMM_COLD;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from cold!\n\r", ch);
        }
        if (GetMaxLevel(ch) >= 30 && !IS_AFFECTED(ch, SPELL_PROT_ELEC)) {
            af.type = SPELL_CIRCLE_PROTECTION;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            af.bitvector = 0;
            af.modifier = IMM_ELEC;
            af.location = APPLY_IMMUNE;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from electricity!\n\r", ch);
        }
        if (GetMaxLevel(ch) >= 35 && !IS_AFFECTED(ch, SPELL_PROT_ENERGY)) {
            af.type = SPELL_CIRCLE_PROTECTION;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            af.bitvector = 0;
            af.location = APPLY_IMMUNE;
            af.modifier = IMM_ENERGY;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from energy!\n\r", ch);
        }
        if (GetMaxLevel(ch) >= 40 && !IS_AFFECTED(ch,
                                                  SPELL_PROT_ENERGY_DRAIN)) {
            af.type = SPELL_CIRCLE_PROTECTION;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            af.bitvector = 0;
            af.location = APPLY_IMMUNE;
            af.modifier = IMM_DRAIN;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from vampires!\n\r", ch);
        }
        if (GetMaxLevel(ch) >= 45 && !IS_AFFECTED(ch, SPELL_PROT_FIRE)) {
            af.type = SPELL_CIRCLE_PROTECTION;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            af.bitvector = 0;
            af.location = APPLY_IMMUNE;
            af.modifier = IMM_FIRE;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from fire!\n\r", ch);
        }
        if (GetMaxLevel(ch) >= 50 && !IS_AFFECTED(ch,
                                                  SPELL_PROT_DRAGON_BREATH)) {
            af.type = SPELL_PROT_DRAGON_BREATH;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = 0;
            af.duration = !IS_IMMORTAL(ch) ? 3 : level;
            affect_to_char(victim, &af);
            send_to_char("You feel protected from dragon breath!\n\r", ch);
        }
    } else {
        send_to_char("You are allready protected!\n\r", ch);
        return;
    }
}

void cast_circle_protection(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }

    switch (type) {
        case SPELL_TYPE_SPELL:
            spell_circle_protection(level, ch, 0, 0);
            break;
        case SPELL_TYPE_SCROLL:
        case SPELL_TYPE_POTION:
        case SPELL_TYPE_WAND:
        case SPELL_TYPE_STAFF:
            Log("Someone just tried to use a circle of protection item!");
            break;
        default:
            Log("Serious screw-up in cast_circle_protection");
            break;
    }
}

#if 0
void spell_cure_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    healpoints = dice(1, 8);

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=1;
    }
}

void cast_cure_light(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_light(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure light!");
        break;
    }
}
#endif
#if 0
void spell_cure_serious(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + 2;

    if ((dam + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += dam;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=1;
    }

}

void cast_cure_serious(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_serious(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_serious(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure serious!");
        break;
    }
}
#endif

#if 0
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
#endif


void spell_enlightenment(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_ENLIGHTENMENT)) {
        send_to_char("You are already enlightened!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
        send_to_char("Your mind is too feeble to enlighten!\n\r", ch);
        return;
    }

    send_to_char("You feel enlightened by the gods\n\r", victim);

    af.type = SPELL_ENLIGHTENMENT;
    af.duration = 24;
    af.modifier = 2;
    af.location = APPLY_INT;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_ENLIGHTENMENT;
    af.duration = 24;
    af.modifier = 2;
    af.location = APPLY_WIS;
    af.bitvector = 0;
    affect_to_char(victim, &af);
}

void cast_enlightenment(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_enlightenment(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in enlightenment.");
        break;
    }
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

void spell_holy_armor(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_HOLY_ARMOR)) {
        af.type = SPELL_HOLY_ARMOR;
        af.duration = 24;
        af.modifier = -20;
        af.location = APPLY_AC;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        send_to_char("You feel the spirits of gods protecting you.\n\r",
                     victim);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void cast_holy_armor(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }

        if (ch != tar_ch) {
            act("$N is protected by the spirits of god.", FALSE, ch, 0,
                tar_ch, TO_CHAR);
        }
        spell_holy_armor(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in holy armor!");
        break;
    }
}

void spell_holy_strength(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_HOLY_STRENGTH)) {
        act("The spirits of gods make you stronger.", FALSE, victim, 0, 0,
            TO_CHAR);
        act("$n's muscles seem to expand!", FALSE, victim, 0, 0, TO_ROOM);

        af.type = SPELL_HOLY_STRENGTH;
        af.duration = 2 * level;
        if (level >= CREATOR) {
            af.modifier = 0;
        } else {
            af.modifier = number(1, 8);
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void cast_holy_strength(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        spell_holy_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_holy_strength(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_holy_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_holy_strength(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in holy strength!");
        break;
    }
}

void spell_holyword(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             lev,
                    t_align;
    struct char_data *t,
                   *next;

    if (level > 0) {
        t_align = -300;
    } else {
        level = -level;
        t_align = 300;
    }

    for (t = real_roomp(ch->in_room)->people; t; t = next) {
        next = t->next_in_room;

        if (!IS_IMMORTAL(t)) {
            lev = GetMaxLevel(t);
            if (GET_ALIGNMENT(t) <= t_align) {
                if (lev <= 4) {
                    damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_HOLY_WORD);
                } else if (lev <= 8) {
                    damage(ch, t, 1, SPELL_HOLY_WORD);
                    spell_paralyze(level, ch, t, 0);
                } else if (lev <= 12) {
                    damage(ch, t, 1, SPELL_HOLY_WORD);
                    spell_blindness(level, ch, t, 0);
                } else if (lev <= 16) {
                    damage(ch, t, 0, SPELL_HOLY_WORD);
                    GET_POS(t) = POSITION_STUNNED;
                }
            } else if (GET_ALIGNMENT(t) > t_align) {
                if (lev <= 4) {
                    damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_UNHOLY_WORD);
                } else if (lev <= 8) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    spell_paralyze(level, ch, t, 0);
                } else if (lev <= 12) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    spell_blindness(level, ch, t, 0);
                } else if (lev <= 16) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    GET_POS(t) = POSITION_STUNNED;
                }
            }
        }
    }
}

void cast_holyword(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_holyword(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in holy word.");
        break;
    }
}

#if 0
void spell_know_alignment(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             ap;
    char            buf[200],
                    name[100];

    assert(victim && ch);

    if (IS_NPC(victim)) {
        strcpy(name, victim->player.short_descr);
    } else {
        strcpy(name, GET_NAME(victim));
    }
    ap = GET_ALIGNMENT(victim);

    if (ap > 700) {
        sprintf(buf, "%s has an aura as white as the driven snow.\n\r", name);
    } else if (ap > 350) {
        sprintf(buf, "%s is of excellent moral character.\n\r", name);
    } else if (ap > 100) {
        sprintf(buf, "%s is often kind and thoughtful.\n\r", name);
    } else if (ap > 25) {
        sprintf(buf, "%s isn't a bad sort...\n\r", name);
    } else if (ap > -25) {
        sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r",
                name);
    } else if (ap > -100) {
        sprintf(buf, "%s isn't the worst you've come across\n\r", name);
    } else if (ap > -350) {
        sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r", name);
    } else if (ap > -700) {
        sprintf(buf, "%s probably just had a bad childhood\n\r", name);
    } else {
        sprintf(buf, "I'd rather just not say anything at all about %s\n\r",
                name);
    }
    send_to_char(buf, ch);
}

void cast_know_alignment(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_know_alignment(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_know_alignment(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_know_alignment(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in know alignment!");
        break;
    }
}
#endif
void do_lay_on_hands(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct affected_type af;
    int             wounds,
                    healing;
    char           *victim_name;

    if (!ch->skills) {
        return;
    }

    argument = get_argument(argument, &victim_name);

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PALADIN)) {
        send_to_char("You are not a holy warrior!\n\r", ch);
        return;
    }

    if (!victim_name || !(victim = get_char_room_vis(ch, victim_name))) {
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

void spell_pacifism(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    /*
     * removes aggressive bit from monsters
     */
    if (IS_NPC(victim)) {
        if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
            if (HitOrMiss(ch, victim, CalcThaco(ch))) {
                REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
                send_to_char("You feel peace and harmony surrounding you.\n\r",
                             ch);
            }
        } else {
            send_to_char("You feel at peace with the universe.\n\r", victim);
        }
    } else {
        send_to_char("You feel at peace with the universe.\n\r", victim);
    }
}

void cast_pacifism(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        act("$n calls upons the gods to calm down $N.", FALSE, ch, 0,
            tar_ch, TO_NOTVICT);
        act("You call upon the gods to calm down $N.", FALSE, ch, 0,
            tar_ch, TO_CHAR);
        spell_pacifism(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_pacifism(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_pacifism(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (in_group(ch, tar_ch)) {
                spell_pacifism(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in pacifism!");
        break;
    }
}
#if 0
void spell_protection_from_evil(int level, struct char_data *ch,
                                struct char_data *victim,
                                struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
        af.type = SPELL_PROTECT_FROM_EVIL;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_PROTECT_FROM_EVIL;
        affect_to_char(victim, &af);
        send_to_char("You have a righteous feeling!\n\r", victim);
    }
}

void cast_protection_from_evil(int level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
    }
}
#endif
#if 0
void spell_protection_from_evil_group(int level, struct char_data *ch,
                                      struct char_data *victim,
                                      struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *dude;

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (IS_FOLLOWING(ch, dude) &&
            !affected_by_spell(dude, SPELL_PROTECT_FROM_EVIL)) {

            af.type = SPELL_PROTECT_FROM_EVIL;
            af.duration = 24;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PROTECT_FROM_EVIL;
            affect_to_char(dude, &af);
            send_to_char("You have a righteous feeling!\n\r", dude);
        }
    }
}

void cast_protection_from_evil_group(int level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil_group(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil_group(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
    }
}
#endif
#if 0
void spell_remove_paralysis(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if (IS_AFFECTED(victim, AFF_PARALYSIS)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_PARALYSIS);
    }
    if (affected_by_spell(victim, SPELL_PARALYSIS)) {
        affect_from_char(victim, SPELL_PARALYSIS);
        act("A warm feeling runs through your body.", FALSE, victim, 0, 0,
            TO_CHAR);
        act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
    }
}

void cast_remove_paralysis(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_paralysis(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_paralysis(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_remove_paralysis(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_paralysis(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove paralysis!");
        break;
    }
}
#endif
#if 0
void spell_remove_poison(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if (affected_by_spell(victim, SPELL_POISON)) {
            affect_from_char(victim, SPELL_POISON);
            act("A warm feeling runs through your body.", FALSE, victim, 0,
                0, TO_CHAR);
            act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
        }
    } else {
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            obj->obj_flags.value[3] = 0;
            act("The $p steams briefly.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}

void cast_remove_poison(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_poison(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove poison!");
        break;
    }
}
#endif
#if 0
void spell_second_wind(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct affected_type *k;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 8) + level;

    if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
        GET_MOVE(victim) = move_limit(victim);
    } else {
        GET_MOVE(victim) += dam;
    }
    if (affected_by_spell(victim, COND_WINGS_TIRED)) {
        affect_from_char(victim, COND_WINGS_TIRED);
    }
    if (affected_by_spell(victim, COND_WINGS_FLY)) {
        for (k = victim->affected; k; k = k->next) {
            if (k->type == COND_WINGS_FLY) {
                k->duration = GET_CON(victim);
            }
        }
    }
    send_to_char("You feel less tired\n\r", victim);
}

void cast_second_wind(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_second_wind(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_second_wind(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:

    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_second_wind(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in second_wind!");
        break;
    }
}
#endif
#if 0
void spell_slow_poison(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (affected_by_spell(victim, SPELL_POISON)) {
        act("$n seems to fade slightly.", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You feel a bit better!.\n\r", victim);

        af.type = SPELL_SLOW_POISON;
        af.duration = 24;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}

void cast_slow_poison(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_slow_poison(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in slow_poison.");
        break;
    }
}
#endif

#if 0
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

/* spy */

#endif

#define STEED_TEMPLATE 44
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

#if 0
void do_switch(struct char_data *ch, char *argument, int cmd)
{
    char     *arg;
    struct char_data *victim;

    dlog("in do_switch");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Switch with who?\n\r", ch);
    } else if (!(victim = get_char(arg))) {
        send_to_char("They aren't here.\n\r", ch);
    } else {
        if (ch == victim) {
            send_to_char("He he he... We are jolly funny today, eh?\n\r",
                         ch);
            return;
        }

        if (!ch->desc || ch->desc->snoop.snoop_by ||
            ch->desc->snoop.snooping) {
            send_to_char("Mixing snoop & switch is bad for your "
                         "health.\n\r", ch);
            return;
        }

        if (victim->desc || !IS_NPC(victim)) {
            send_to_char("You can't do that, the body is already in "
                         "use!\n\r", ch);
        } else {
            if (GetMaxLevel(victim) > GetMaxLevel(ch) &&
                GetMaxLevel(ch) < IMPLEMENTOR) {
                send_to_char("That being is much more powerful than "
                             "you!\n\r", ch);
                return;
            }

            send_to_char("Ok.\n\r", ch);

            ch->desc->character = victim;
            ch->desc->original = ch;

            victim->desc = ch->desc;
            ch->desc = 0;
        }
    }
}

#endif


#if 0
void spell_turn(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             diff,
                    i;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (IsUndead(victim)) {
        diff = level - GetTotLevel(victim);
        if (diff <= 0) {
            act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
            return;
        } else {
            for (i = 1; i <= diff; i++) {
                if (!saves_spell(victim, SAVING_SPELL)) {
                    act("$n forces $N from this room.", TRUE, ch, 0,
                        victim, TO_NOTVICT);
                    act("You force $N from this room.", TRUE, ch, 0,
                        victim, TO_CHAR);
                    act("$n forces you from this room.", TRUE, ch, 0,
                        victim, TO_VICT);
                    do_flee(victim, "", 0);
                    break;
                }
            }
            if (i < diff) {
                act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
                act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
                act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
            }
        }
    } else {
        act("$n just tried to turn you, what a moron!", TRUE, ch, 0,
            victim, TO_VICT);
        act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
    }
}

void cast_turn(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_turn(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in turn");
        break;
    }
}
#endif

void do_holy_warcry(struct char_data *ch, char *argument, int cmd)
{
    char            *name;
    int             dam,
                    dif,
                    level;
    struct char_data *dude;

    if (!ch->skills) {
        return;
    }

    argument = get_argument(argument, &name);
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
    } else if (!name || !(dude = get_char_room_vis(ch, name))) {
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
