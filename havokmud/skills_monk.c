/*
 * HavokMUD - monk skills
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
/* dodge is an innate ability checked for when fighting */

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
#if 0
void do_hide(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_hide");

    if (cmd == 334 && !HasClass(ch, CLASS_BARBARIAN)) {
        send_to_char("Hey, you can't do that!\n\r", ch);
        return;
    }
    if (cmd == 153 && !HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_RANGER)) {
        send_to_char("Hey, you can't do that!\n\r", ch);
        return;
    }
    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }
    if (!HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_BARBARIAN |
                      CLASS_RANGER)) {
        send_to_char("You're not trained to hide!\n\r", ch);
        return;
    }
    if (!HasClass(ch, CLASS_BARBARIAN | CLASS_RANGER)) {
        send_to_char("You attempt to hide in the shadows.\n\r", ch);
    } else {
        send_to_char("You attempt to camouflage yourself.\n\r", ch);
    }
    if (HasClass(ch, CLASS_BARBARIAN | CLASS_RANGER) && !OUTSIDE(ch)) {
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
    if (percent > ch->skills[SKILL_HIDE].learned +
        dex_app_skill[(int)GET_DEX(ch)].hide) {
        LearnFromMistake(ch, SKILL_HIDE, 1, 90);
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
#endif
#if 0
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
#endif
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
                sprintf(buf, "$c000BYou receive $c000W100 $c000Bexperience "
                             "for using your combat abilities.$c0007\n\r");
                send_to_char(buf, ch);
                gain_exp(ch, 100);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            }
        }
    }
}
#if 0
void do_pick(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;
    int             door;
    char           *type,
                   *dir;
    struct room_direction_data *exitp;
    struct obj_data *obj;
    struct char_data *victim;

    dlog("in do_pick");

    argument = get_argument(argument, &type);
    argument = get_argument(argument, &dir);

    percent = number(1, 101);   /* 101% is a complete failure */

    if (!ch->skills) {
        send_to_char("You failed to pick the lock.\n\r", ch);
        return;
    }

    if (!HasClass(ch, CLASS_THIEF) && !HasClass(ch, CLASS_MONK)) {
        send_to_char("You're no thief!\n\r", ch);
        return;
    }

    if (percent > (ch->skills[SKILL_PICK_LOCK].learned)) {
        send_to_char("You failed to pick the lock.\n\r", ch);
        act("$n mutters as $s lockpicks jam in the lock.", TRUE, ch, 0, 0,
            TO_ROOM);
        LearnFromMistake(ch, SKILL_PICK_LOCK, 0, 90);
        WAIT_STATE(ch, PULSE_VIOLENCE * 4);
        return;
    }

    if (!type) {
        send_to_char("Pick what?\n\r", ch);
        return;
    } 
    
    if (generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, 
                     &obj)) {
        /*
         * this is an object 
         */
        if (obj->obj_flags.type_flag != ITEM_CONTAINER) {
            send_to_char("That's not a container.\n\r", ch);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) {
            send_to_char("Silly - it ain't even closed!\n\r", ch);
        } else if (obj->obj_flags.value[2] < 0) { 
            send_to_char("Odd - you can't seem to find a keyhole.\n\r", ch);
        } else if (!IS_SET(obj->obj_flags.value[1], CONT_LOCKED)) {
            send_to_char("Oho! This thing is NOT locked!\n\r", ch);
        } else if (IS_SET(obj->obj_flags.value[1], CONT_PICKPROOF)) {
            send_to_char("It resists your attempts at picking it.\n\r", ch);
        } else {
            REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
            send_to_char("*Click*\n\r", ch);
            act("$n fiddles with $p.", FALSE, ch, obj, 0, TO_ROOM);
        }
    } else if ((door = find_door(ch, type, dir)) >= 0) {
        exitp = EXIT(ch, door);
        if (!IS_SET(exitp->exit_info, EX_ISDOOR)) {
            send_to_char("That's absurd.\n\r", ch);
        } else if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
            send_to_char("You realize that the door is already open.\n\r", ch);
        } else if (exitp->key < 0) {
            send_to_char("You can't seem to spot any lock to pick.\n\r", ch);
        } else if (!IS_SET(exitp->exit_info, EX_LOCKED)) {
            send_to_char("Oh.. it wasn't locked at all.\n\r", ch);
        } else if (IS_SET(exitp->exit_info, EX_PICKPROOF)) {
            send_to_char("You seem to be unable to pick this lock.\n\r", ch);
        } else {
            if (exitp->keyword) {
                act("$n skillfully picks the lock of the $F.", 0, ch, 0,
                    exitp->keyword, TO_ROOM);
            } else {
                act("$n picks the lock.", TRUE, ch, 0, 0, TO_ROOM);
            }
            send_to_char("The lock quickly yields to your skills.\n\r", ch);
            raw_unlock_door(ch, exitp, door);

        }
    }
}
#endif
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
#if 0
void do_sneak(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_sneak");

    if (IS_AFFECTED2(ch, AFF2_SKILL_SNEAK)) {
        if (IS_AFFECTED(ch, AFF_HIDE)) {
            REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
        }
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        send_to_char("You are no longer sneaky.\n\r", ch);
        return;
    }
    if (!HasClass(ch, CLASS_THIEF | CLASS_MONK | CLASS_RANGER)) {
        send_to_char("You're not trained to walk silently!\n\r", ch);
        return;
    }
    if (HasClass(ch, CLASS_RANGER) && !OUTSIDE(ch)) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("You must do this outdoors!\n\r", ch);
            return;
        }
    }
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    if (!IS_AFFECTED(ch, AFF_SILENCE)) {
        if (HasWBits(ch, ITEM_HUM)) {
            send_to_char("Gonna be hard to sneak around with that thing "
                         "humming\n\r", ch);
            return;
        }
    }
    send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);
    percent = number(1, 101);
    /*
     * 101% is a complete failure
     */
    if (!ch->skills) {
        return;
    }
    if (IS_SET(ch->specials.affected_by2, AFF2_SKILL_SNEAK)) {
        send_to_char("You stop being sneaky!", ch);
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
    } else {
        send_to_char("You start jumping from shadow to shadow.", ch);
        SET_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }
}
#endif
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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
