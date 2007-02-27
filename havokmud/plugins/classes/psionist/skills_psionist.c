/*
 * HavokMUD - psionist spells and skills
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

extern struct obj_data *object_list;
extern struct char_data *character_list;
extern long     SystemFlags;
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
extern char	*dirs[];

void do_adrenalize(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);
    if (!target_name || !(target = get_char_room_vis(ch, target_name))) {
        send_to_char("You can't seem to find that person anywhere.\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 15 && !IS_IMMORTAL(ch)) {
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
        act("$n touches you on the forehead lightly, you feel energy "
            "unlimited!", TRUE, ch, 0, target, TO_VICT);
    }
    send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using your "
                 "abilities.$c0007\n\r", ch);
    gain_exp(ch, 100);
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

void do_canibalize(struct char_data *ch, char *argument, int cmd)
{
    long            hit_points,
                    mana_points;
    char           *number;
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

    argument = get_argument(argument, &number);
    if( !number ) {
        send_to_char("Please include a number after the command.\n\r", ch);
        return;
    }

    for (count = 0; num_found && (count < 9) && (number[count] != '\0');
         count++) {
        if (number[count] < '0' || number[count] > '9') {
            /* 
             * leading zero is ok 
             */
            num_found = FALSE;
        }
    }

    if (!num_found) {
        send_to_char("Please include a number after the command.\n\r", ch);
        return;
    } 
    
    /* 
     * forced the string to be proper length 
     */
    number[count] = '\0';

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

void mind_clairvoyance(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_CLAIRVOYANCE)) {
        if (ch != victim) {
            act("", FALSE, ch, 0, victim, TO_CHAR);
            act("", FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("$n meditates for a moment.", TRUE, victim, 0, 0, TO_ROOM);
            act("You open your mind's eye to nearby visions and sights.",
                TRUE, victim, 0, 0, TO_CHAR);
        }

        af.type = SKILL_CLAIRVOYANCE;
        af.duration = (!IS_IMMORTAL(ch) ? 3 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SCRYING;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N can already sense nearby sights.", FALSE, ch, 0,
            victim, TO_CHAR);
    } else {
        act("You are already clairvoyant.", FALSE, ch, 0, victim, TO_CHAR);
    }

}

void mind_use_clairvoyance(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_clairvoyance(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_clairvoyance");
        break;
    }
}

void mind_cell_adjustment(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    if (!ch) {
        Log("!ch in cell_adjustment");
        return;
    }

    if (ch != victim) {
        send_to_char("You cannot use this skill on others.\n\r", ch);
        return;
    }

    act("You begin the process of altering your body's cells.", FALSE, ch,
        0, victim, TO_CHAR);
    act("$n goes into a deep trance.", FALSE, ch, 0, victim, TO_ROOM);

    if (GET_HIT(victim) + 100 > GET_MAX_HIT(victim)) {
        act("You completely heal your body.", FALSE, victim, 0, 0, TO_CHAR);
        GET_HIT(victim) = GET_MAX_HIT(victim);
    } else {
        act("You manage to heal some of your body through cell adjustment.", 
            FALSE, victim, 0, 0, TO_CHAR);
        GET_HIT(victim) += 100;
    }

    if (!IS_IMMORTAL(ch)) {
        act("You are overcome by exhaustion.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n slumps to the ground exhausted.", FALSE, ch, 0, 0, TO_ROOM);
        WAIT_STATE(ch, PULSE_VIOLENCE * 12);
        GET_POS(ch) = POSITION_STUNNED;
    }
}

void mind_use_cell_adjustment(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *victim,
                              struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_cell_adjustment(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_cell_adjustment");
        break;
    }
}

void mind_chameleon(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    if (!ch) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }

    act("You camoflauge yourself in others minds.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n's body wavers, then disappears into the surroundings.", FALSE,
        ch, 0, 0, TO_ROOM);
    SET_BIT(ch->specials.affected_by, AFF_HIDE);
}

void mind_use_chameleon(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_chameleon(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_chameleon");
        break;
    }
}

void do_doorway(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);
    if (!target_name || !(target = get_char_vis_world(ch, target_name, NULL))) {
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

    if (GET_MANA(ch) < 20 && !IS_IMMORTAL(ch)) {
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
        act("$n steps through the portal and the portal disappears!",
            FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, location);
        act("A portal appears before you and $n steps through!", FALSE, ch,
            0, 0, TO_ROOM);
        do_look(ch, NULL, 15);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
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

    if ((GET_MANA(ch) < 10) && !IS_IMMORTAL(ch)) {
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
    if (GET_MANA(ch) < 40 && !IS_IMMORTAL(ch)) {
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
    
    if (affected_by_spell(ch, SPELL_DETECT_INVISIBLE) &&
	affected_by_spell(ch, SPELL_SENSE_LIFE) &&
	affected_by_spell(ch, SPELL_TRUE_SIGHT)) {
        send_to_char("You cannot seem to benefit anything from this.\n\r",
		     ch);
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

void do_hypnosis(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);

    if( !target_name || !(victim = get_char_room_vis(ch, target_name))) {
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

    if (GET_MANA(ch) < 25 && !IS_IMMORTAL(ch)) {
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

void mind_kinolock(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *tar_ch, struct obj_data *tar_obj)
{
    int             door;
    char           *dir;
    char           *otype;
    struct obj_data *obj;
    struct char_data *victim;

    arg = get_argument(arg, &otype);
    arg = get_argument(arg, &dir);

    if (!otype) {
        send_to_char("Kinolock what?\n\r", ch);
        return;
    }

    if (generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj)) {
        send_to_char("Sorry.. this skill can only be used on doors\n", ch);
    } else if ((door = find_door(ch, otype, dir)) >= 0) {
        if (!IS_SET(EXIT(ch, door)->exit_info, EX_ISDOOR)) {
            send_to_char("That's absurd.\n\r", ch);
        } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
            send_to_char("You realize that the door is already open.\n\r", ch);
        } else if (EXIT(ch, door)->key < 0) {
            send_to_char("You can't seem to spot any lock to unlock.\n\r", ch);
        } else if (IS_SET(EXIT(ch, door)->exit_info, EX_PICKPROOF)) {
            if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
                send_to_char("You seem to be unable to lock this...\n\r", ch);
            } else {
                send_to_char("You seem to be unable to unlock this...\n\r", ch);
            }
        } else if (!IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED)) {
            send_to_char("You engage the lock with your mind.\n\r", ch);
            act("You here a click as $n engages the lock with $s mind.", TRUE,
                ch, 0, 0, TO_ROOM);
            raw_lock_door(ch, EXIT(ch, door), door);
        } else  {
            if (EXIT(ch, door)->keyword && 
                strcmp("secret", fname(EXIT(ch, door)->keyword))) {
                act("$n uses $s mind to open the lock of the $F.", 0,
                    ch, 0, EXIT(ch, door)->keyword, TO_ROOM);
            } else {
                act("$n uses $s mind to open the lock.", TRUE, ch, 0,
                    0, TO_ROOM);
            }
            send_to_char("The lock quickly yields to your skills.\n\r", ch);
            raw_unlock_door(ch, EXIT(ch, door), door);
        }
    }
}

void mind_use_kinolock(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_kinolock(level, ch, arg, 0, tar_ch, tar_obj);
        break;
    default:
        Log("Serious screw-up in mind_kinolock");
        break;
    }
}

void mind_levitation(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (victim && IS_SET(victim->specials.act, PLR_NOFLY)) {
        REMOVE_BIT(victim->specials.act, PLR_NOFLY);
    }

    if (!affected_by_spell(victim, SKILL_LEVITATION)) {
        if (ch != victim) {
            act("You lift $N with a simple thought.", FALSE, ch, 0, victim,
                TO_CHAR);
            act("$N is lifted into the air by a simple thought from $n.",
                FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("$n lifts $s own body with a thought.", TRUE, victim, 0, 0,
                TO_ROOM);
            act("You lift yourself with your mind", TRUE, victim, 0, 0,
                TO_CHAR);
        }

        af.type = SKILL_LEVITATION;
        af.duration = (int) (level * 2) / 10;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_FLYING;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N is already levitating.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You are already levitating.", FALSE, ch, 0, victim, TO_CHAR);
    }
}

void mind_use_levitation(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *victim,
                         struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_levitation(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_levitation");
        break;
    }
}

void mind_psi_strength(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!victim || !ch) {
        return;
    }
    if (!affected_by_spell(victim, SKILL_PSI_STRENGTH)) {
        act("You feel stronger.", FALSE, victim, 0, 0, TO_CHAR);
        act("$n seems stronger!\n\r", FALSE, victim, 0, 0, TO_ROOM);
        af.type = SKILL_PSI_STRENGTH;
        af.duration = 2 * level;
        if (IS_NPC(victim)) {
            if (level >= CREATOR) {
                af.modifier = 25 - GET_STR(victim);
            } else {
                af.modifier = number(1, 6);
            }
        } else {
            if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_BARBARIAN)) {
                af.modifier = number(1, 8);
            } else if (HasClass(ch, CLASS_CLERIC | CLASS_THIEF | CLASS_PSI)) {
                af.modifier = number(1, 6);
            } else {
                af.modifier = number(1, 4);
            }
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void mind_use_psi_strength(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psi_strength(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psi_strength");
        break;
    }
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

void mind_burn(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    if (!ch) {
        return;
    }
    dam = dice(1, 4) + level / 2 + 1;

    send_to_char("Gouts of flame shoot forth from your mind!\n\r", ch);
    act("$n sends a gout of flame shooting from $s mind!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        rdam = dam;
        if (ch->in_room == tmp_victim->in_room && ch != tmp_victim) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are seared by the burning flame!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam = 1;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    BurnWings(tmp_victim);
                }
                MissileDamage(ch, tmp_victim, rdam, SKILL_MIND_BURN);
            } else {
                act("You are able to avoid the flames!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        }
    }
}

void mind_use_burn(int level, struct char_data *ch, char *arg, int type,
                   struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_burn(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in mind_burn");
        break;
    }
}

void mind_mind_over_body(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_MIND_OVER_BODY)) {
        if (ch != victim) {
            act("", FALSE, ch, 0, victim, TO_CHAR);
            act("", FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("$n meditates for awhile.", TRUE, victim, 0, 0, TO_ROOM);
            act("You force your body to obey your mind and not require food "
                "or water!", TRUE, victim, 0, 0, TO_CHAR);
        }

        af.type = SKILL_MIND_OVER_BODY;
        af.duration = 12;
        af.modifier = -1;
        af.location = APPLY_MOD_THIRST;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SKILL_MIND_OVER_BODY;
        af.duration = 12;
        af.modifier = -1;
        af.location = APPLY_MOD_HUNGER;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N does not require your help.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("Your mind is already forcing your body to obey!", FALSE,
            ch, 0, victim, TO_CHAR);
    }
}

void mind_use_mind_over_body(int level, struct char_data *ch, char *arg,
                             int type, struct char_data *victim,
                             struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_over_body(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mind_over_body");
        break;
    }
}

void mind_mind_wipe(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    spell_feeblemind(level, ch, victim, obj);
}

void mind_use_mind_wipe(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_wipe(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_wipe");
        break;
    }
}

void mind_mindblank(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_MINDBLANK)) {
        if (ch != victim) {
            act("", FALSE, ch, 0, victim, TO_CHAR);
            act("", FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("$n meditates for a few moments.", TRUE, victim, 0, 0,
                TO_ROOM);
            act("You begin to shift the patterns of your thoughts for "
                "protection.", TRUE, victim, 0, 0, TO_CHAR);
        }

        af.type = SKILL_MINDBLANK;
        af.duration = (int) (level * 2) / 10;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N is already protected.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You are already protected.", FALSE, ch, 0, victim, TO_CHAR);
    }
}

void mind_use_mindblank(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *victim,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mindblank(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mindblank");
        break;
    }
}

void mind_mind_tap(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    int             mana,
                    hit;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }

    mana = dice(0, GET_INT(ch) / 2);
    hit = dice(0, GET_INT(ch) / 2);

    ch_printf(ch, "You seem to tap his mind of vital energy!!");

    if (IS_PC(ch)) {
        GET_ALIGNMENT(ch) -= 4;
    }
}

void mind_use_mind_tap(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_mind_tap(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_mind_tap");
        break;
    }
}

#define PROBABILITY_TRAVEL_ENTRANCE   2701
void mind_probability_travel(int level, struct char_data *ch,
                             struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp,
                   *tmp2;
    struct room_data *rp;

    if (IS_SET(SystemFlags, SYS_NOASTRAL)) {
        send_to_char("The astral planes are shifting, you cannot!\n", ch);
        return;
    }

    rp = real_roomp(ch->in_room);

    for (tmp = rp->people; tmp; tmp = tmp2) {
        tmp2 = tmp->next_in_room;
        if (in_group(ch, tmp) && !tmp->specials.fighting &&
            (IS_PC(tmp) || IS_SET(tmp->specials.act, ACT_POLYSELF)) &&
            IS_AFFECTED(tmp, AFF_GROUP)) {
            act("$n wavers as $N sends $m to another plane.", FALSE, tmp,
                0, ch, TO_ROOM);
            char_from_room(tmp);
            char_to_room(tmp, PROBABILITY_TRAVEL_ENTRANCE);
            do_look(tmp, NULL, 0);
            act("$n wavers into existance", FALSE, tmp, 0, 0, TO_ROOM);
        }
    }
}

void mind_use_probability_travel(int level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *victim,
                                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_probability_travel(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_probability_travel");
        break;
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

    if (GET_MANA(ch) < 10 && !IS_IMMORTAL(ch)) {
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

void do_psi_portal(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);
    if (!target_name || !(target = get_char_vis_world(ch, target_name, NULL))) {
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

    if (GET_MANA(ch) < 50 && !IS_IMMORTAL(ch)) {
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
            do_look(leader, NULL, 15);
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
                do_look(follower, NULL, 15);
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
        do_look(ch, NULL, 15);
        act("$n appears out of the portal as it disappears!", FALSE, ch, 0,
            0, TO_ROOM);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
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

    if (GET_MANA(ch) < 10 && !IS_IMMORTAL(ch)) {
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

void do_mindsummon(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);
    if (!target_name || !(target = get_char_vis_world(ch, target_name, NULL))) {
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

    if ((GET_MANA(ch) < 30) && !IS_IMMORTAL(ch)) {
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
    command_interpreter(target, "look");
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

void do_blast(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    int             potency = 0,
                    level,
                    dam = 0;
    struct affected_type af;
    char            buf[256];

    if (!ch->skills) {
        return;
    }

    argument = get_argument(argument, &name);

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
    } else if ( !name || !(victim = get_char_room_vis(ch, name))) {
        send_to_char("Exactly whom did you wish to blast?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Blast yourself? Your mother would be sad!\n\r", ch);
        return;
    }
    if (!IS_NPC(victim)) {
        send_to_char("You can't use this on other players!\n\r", ch);
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

    if (IS_IMMORTAL(victim)) {
        send_to_char("They ignore your attempt at humor!\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 25 && !IS_IMMORTAL(ch)) {
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
        if (level > MAX_MORT) {
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

void mind_danger_sense(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_DANGER_SENSE)) {
        if (ch != victim) {
            act("$n opens $N's eyes to dangers.", FALSE, ch, 0, victim,
                TO_ROOM);
            act("You open $N's eyes to dangers.", FALSE, ch, 0, victim,
                TO_ROOM);
        } else {
            act("$n seems to look more intently about.", TRUE, victim, 0,
                0, TO_ROOM);
            act("You open your mind and eyes for hidden dangers.", TRUE,
                victim, 0, 0, TO_CHAR);
        }

        af.type = SKILL_DANGER_SENSE;
        af.duration = (int) level / 10;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N can already sense dangers.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You already sense hidden dangers.", FALSE, ch, 0, victim, TO_CHAR);
    }
}

void mind_use_danger_sense(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_danger_sense(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_danger_sense");
        break;
    }
}

void mind_disintegrate(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    spell_disintegrate(level, ch, victim, obj);
}

void mind_use_disintegrate(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *victim,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_disintegrate(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_disintegrate");
        break;
    }
}

void mind_teleport(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    int             to_room,
                    try = 0;
    extern int      top_of_world;
    struct room_data *room;

    if (!ch || !victim) {
        return;
    }
    if (victim != ch) {
        if (saves_spell(victim, SAVING_SPELL)) {
            send_to_char("You can't seem to force them to blink out.\n\r", ch);
            if (IS_NPC(victim)) {
                if (!victim->specials.fighting) {
                    set_fighting(victim, ch);
                }
            } else {
                send_to_char("You feel strange, but the effect fades.\n\r",
                             victim);
            }
            return;
        } else {
            /* 
             * the character (target) is now the victim 
             */
            ch = victim;
        }
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    do {
        to_room = number(0, top_of_world);
        room = real_roomp(to_room);
        if (room && (IS_SET(room->room_flags, PRIVATE) ||
                     IS_SET(room->room_flags, TUNNEL) ||
                     IS_SET(room->room_flags, NO_SUM) ||
                     IS_SET(room->room_flags, NO_MAGIC) ||
                     !IsOnPmp(to_room))) {
            room = 0;
            try++;
        }
    } while (!room && try < 10);

    if (try >= 10) {
        send_to_char("The skill fails.\n\r", ch);
        return;
    }

    act("$n seems to scatter into tiny particles and is gone!", FALSE, ch,
        0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("A mass of flying particles form into $n!", FALSE, ch, 0, 0, TO_ROOM);

    do_look(ch, NULL, 0);

    if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
        !IS_IMMORTAL(ch)) {
        NailThisSucker(ch);
        return;
    }

    check_falling(ch);
}

void mind_use_teleport(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *victim,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_teleport(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_teleport");
        break;
    }
}

void mind_psychic_crush(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    /*
     * damage = level d6, +1 for every two levels of the psionist 
     */

    dam = dice(level, 6);
    dam += (int) level / 2;

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
        if (affected_by_spell(victim, SKILL_TOWER_IRON_WILL)) {
            dam = 0;
        }
    }

    /*
     * half dam if tower up 
     */
    if (affected_by_spell(victim, SKILL_TOWER_IRON_WILL)) {
        dam >>= 1;
    }

    MissileDamage(ch, victim, dam, SKILL_PSYCHIC_CRUSH);
}

void mind_use_psychic_crush(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *victim,
                            struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psychic_crush(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psyic_crush");
        break;
    }
}

void mind_psychic_impersonation(int level, struct char_data *ch,
                                struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *k;

    act("You attempt to make others see you as someone else.", FALSE, ch,
        0, victim, TO_VICT);
    act("$N's features shift and waver before your eyes!", FALSE, ch, 0,
        victim, TO_NOTVICT);

    if (affected_by_spell(victim, SKILL_PSYCHIC_IMPERSONATION)) {
        send_to_char("You are already attempting to do that\n\r", victim);
        return;
    }

    for (k = character_list; k; k = k->next) {
        if (k->specials.hunting == victim) {
            k->specials.hunting = 0;
        }
        
        if (Hates(k, victim)) {
            ZeroHatred(k, victim);
        }
        
        if (Fears(k, victim)) {
            ZeroFeared(k, victim);
        }
    }

    af.type = SKILL_PSYCHIC_IMPERSONATION;
    af.duration = (int) (level * 2) / 10;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(victim, &af);

}

void mind_use_psychic_impersonation(int level, struct char_data *ch,
                                    char *arg, int type,
                                    struct char_data *victim,
                                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_psychic_impersonation(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_psychic_impersonation");
        break;
    }
}

void do_scry(struct char_data *ch, char *argument, int cmd)
{
    char           *target_name;
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

    argument = get_argument(argument, &target_name);
    if (!target_name || !(target = get_char_vis_world(ch, target_name, NULL))) {
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

    if (GET_MANA(ch) < 20 && !IS_IMMORTAL(ch)) {
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
        do_look(ch, NULL, 15);
        char_from_room(ch);
        char_to_room(ch, old_location);
        send_to_char("$c000BYou receive $c000W100 $c000Bexperience for using "
                     "your abilities.$c0007\n\r", ch);
        gain_exp(ch, 100);
        WAIT_STATE(ch, PULSE_VIOLENCE);
    }
}

void mind_sense_object(int level, struct char_data *ch,
                       struct char_data *victim, char *arg)
{
    char            buf[MAX_STRING_LENGTH];
    int             room = 0;
    int             old_location;
    struct obj_data *i;
    struct char_data *target = NULL;

    if( !ch || !ch->skills ) {
        return;
    }

    buf[0] = '\0';

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("Your mind is not developed enough to do this\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that ?\n\r", ch);
        return;
    }

    if (!ch->skills[SKILL_SENSE_OBJECT].learned) {
        send_to_char("You have not trained your mind to do this\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
        send_to_char("Some powerful magic interference provide you from "
                     "finding this object\n", ch);
        return;
    } 

    key = StringToKeywords( arg, NULL );
    
    for (i = object_list; i; i = i->next) {
        if ( IS_OBJ_STAT(i, extra_flags, ITEM_QUEST) ||
             !KeywordsMatch(key, &i->keywords) ) {
            continue;
        }

        if (i->carried_by) {
            target = i->carried_by;
            if (((IS_SET(SystemFlags, SYS_ZONELOCATE) && 
                 real_roomp(ch->in_room)->zone ==
                 real_roomp(target->in_room)->zone) || 
                (!IS_SET(SystemFlags, SYS_ZONELOCATE))) && 
                !IS_IMMORTAL(target) && 
                !(IS_SET(target->specials.act, ACT_PSI) && 
                  GetMaxLevel(target) > GetMaxLevel(ch))) {
                room = target->in_room;
            }
        } else if (i->equipped_by) {
            target = i->equipped_by;
            if (!IS_IMMORTAL(target) && 
                !(IS_SET(target->specials.act, ACT_PSI) && 
                  GetMaxLevel(target) > GetMaxLevel(ch)) && 
                ((IS_SET(SystemFlags, SYS_ZONELOCATE) && 
                 real_roomp(ch->in_room)->zone ==
                 real_roomp(target->in_room)->zone) ||
                 (!IS_SET(SystemFlags, SYS_ZONELOCATE)))) {
                room = target->in_room;
            }
        } else if (i->in_obj) {
            if ((IS_SET(SystemFlags, SYS_ZONELOCATE) && 
                 real_roomp(ch->in_room)->zone ==
                 real_roomp(i->in_obj->in_room)->zone) ||
                (!IS_SET(SystemFlags, SYS_ZONELOCATE))) {
                room = (i->in_obj->in_room);
            }
        } else if (i->in_room && 
                   ((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                    real_roomp(ch->in_room)->zone ==
                    real_roomp(target->in_room)->zone) || 
                    (!IS_SET(SystemFlags, SYS_ZONELOCATE)))) {
            room = (i->in_room);
        }
    }

    if (room == 0 || room == NOWHERE) {
        send_to_char("You cannot sense that item.\n\r", ch);
        return;
    } 
    
    /* 
     * a valid room check
     */
    if (real_roomp(room)) {
        send_to_char("You close your eyes and envision your target.\n\r", ch);
        old_location = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, room);
        do_look(ch, NULL, 15);
        char_from_room(ch);
        char_to_room(ch, old_location);
    }
}

void mind_use_sense_object(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_sense_object(level, ch, NULL, arg);
        break;
    default:
        Log("Serious screw-up in mind_sense_object");
        break;
    }
}

void mind_telekinesis(int level, struct char_data *ch,
                      struct char_data *victim, int dir_num)
{
    int             percent = 0;

    if (!ch) {
        Log("!ch in telekenisis");
        return;
    }

    if (!victim) {
        Log("!victim in telekenisis");
        return;
    }

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        !HasClass(ch, CLASS_PSI)) {
        send_to_char("You're no psionist!\n\r", ch);
        return;
    }

    if (!CAN_SEE(ch, victim)) {
        send_to_char("Who do you wish to move with your mind?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Aren't we funny today...\n\r", ch);
        return;
    }

    /* 
     * 101% is a complete failure 
     */
    percent = number(1, 101);

    /*
     * see if we can shove him 
     */
    if (!ch->specials.fighting && dir_num > -1) {
        if (percent > ch->skills[SKILL_TELEKINESIS].learned ||
            saves_spell(victim, SAVING_SPELL) || 
            (IS_SET(victim->specials.act, ACT_SENTINEL) && 
             IS_SET(victim->specials.act, ACT_HUGE))) {
            act("Your mind suffers a brief weakness that forces you to drop "
                "$N.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n tries to telekinesis you, but your mind resists.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$n tries to telekinesis $N out of the area, but fails.",
                FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("You lift $N with a thought and force $M from the area!",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n lifts you with $s mind, flinging you out the area!",
                FALSE, ch, 0, victim, TO_VICT);
            act("$n summons great mental powers and lifts $N, only to toss $M"
                " from the area!", FALSE, ch, 0, victim, TO_ROOM);
            do_move(victim, "\0", dir_num);
        }
    } else {
        /* 
         * fighting move
         */
        if (percent > ch->skills[SKILL_TELEKINESIS].learned) {
            act("You cannot seem to focus your mind enough for the telekinetic"
                " force.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n fails to move you with $s mind!", FALSE, ch, 0, victim,
                TO_VICT);
            act("$n attemps to use $s telekinetic powers on $N, but fails!",
                FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("You slam $N to the ground with a single thought!", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n lifts you with $s mind, then slams you to the ground!",
                FALSE, ch, 0, victim, TO_VICT);
            act("$n slams $N to the ground with $s telekinetic powers!",
                FALSE, ch, 0, victim, TO_ROOM);
            GET_POS(victim) = POSITION_SITTING;
            if (!victim->specials.fighting) {
                set_fighting(victim, ch);
            }
            WAIT_STATE(victim, PULSE_VIOLENCE * 4);
        }
    }
}

void mind_use_telekinesis(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *victim,
                          struct obj_data *tar_obj)
{
    char           *p;
    int             i = -1;
    int             found;

    found = 0;

    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        if (!ch->specials.fighting) {
            /*
             * get the argument, parse it into a direction 
             */
            arg = skip_spaces(arg);
            if (!arg) {
                mind_telekinesis(level, ch, victim, i);
                return;
            }

            p = fname(arg);
            for (i = 0; i < 6 && !found; i++) {
                if (strncmp(p, dirs[i], strlen(p)) == 0) {
                    found = 1;
                }
            }

            /* The array direction (0-5) will now match
               the direction number (1-6) */

            if (!found) {
                send_to_char("You must supply a direction!\n\r", ch);
                return;
            }
        }
        mind_telekinesis(level, ch, victim, i);
        break;
    default:
        Log("Serious screw-up in mind_telekinesis");
        break;
    }
}

void mind_tower_iron_will(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_TOWER_IRON_WILL)) {
        if (ch != victim) {
            act("", FALSE, ch, 0, victim, TO_CHAR);
            act("", FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("$n meditates for a few moments.", TRUE, victim, 0, 0, TO_ROOM);
            act("You erect a tower of iron will to protect you!", TRUE,
                victim, 0, 0, TO_CHAR);
        }

        af.type = SKILL_TOWER_IRON_WILL;
        af.duration = (int) level / 10;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N is already protected.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You are already protected.", FALSE, ch, 0, victim, TO_CHAR);
    }
}

void mind_use_tower_iron_will(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *victim,
                              struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_tower_iron_will(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_tower_iron_will");
        break;
    }
}

void mind_ultra_blast(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam,
                    count = 0;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4) + level;
    rdam = dam;

    act("You blast out a massive wave of destructive psionic energy!",
        FALSE, ch, 0, victim, TO_CHAR);
    act("$n blasts out a massive wave of destructive psionic energy!",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim; 
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        rdam = dam;
        if (count >= 7) {
            break;
        }
        if (ch->in_room == tmp_victim->in_room && ch != tmp_victim &&
            !IS_IMMORTAL(tmp_victim)) {
            if (!in_group(ch, tmp_victim)) {
                
                count ++; 
                if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                    if (affected_by_spell(tmp_victim, SKILL_TOWER_IRON_WILL)) {
                        rdam >>= 1;
                    }
                    MissileDamage(ch, tmp_victim, rdam, SKILL_ULTRA_BLAST);
                } else {
                    rdam >>= 1;  
                    if (affected_by_spell(tmp_victim, SKILL_TOWER_IRON_WILL)) {
                        rdam = 0;
                    }
                    MissileDamage(ch, tmp_victim, rdam, SKILL_ULTRA_BLAST);
                }
            
            } else {
                act("You manage to get out of the way of the massive psionic "
                    "blast!", FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
    }
}

void mind_use_ultra_blast(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *victim,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        mind_ultra_blast(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in mind_ultra_blast");
        break;
    }
}

void mind_wall_of_thought(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert (ch);

    if (!affected_by_spell(ch, SKILL_WALL_OF_THOUGHT)) {
        act("A wall of thought surrounds $n.", TRUE, ch, 0, 0, TO_ROOM);
        act("You surround yourself with a wall of thought.", 
           TRUE, ch, 0, 0, TO_CHAR);
        
        af.type = SKILL_WALL_OF_THOUGHT;
        af.duration = level/2;
        af.modifier = IMM_BLUNT;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }
}

void mind_use_wall_of_thought(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj)
{
    switch(type) {
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(ch, SKILL_WALL_OF_THOUGHT)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        } else {
            mind_wall_of_thought(level, ch, ch, 0);
            break;
        }
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SKILL_WALL_OF_THOUGHT)) {
            return;
        } else {
            mind_wall_of_thought(level, ch, ch, 0);
            break;
        }
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:    
        if (tar_obj || affected_by_spell(ch, SKILL_WALL_OF_THOUGHT)) {
            return;
        } else {
            mind_wall_of_thought(level, ch, ch, 0);
            break;
        }
    default:
        Log("Serious screwup in wall_of_thought!");
        break;
    }
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
