/*
 * HavokMUD - ranger spells and skills
 * REQUIRED BY - ranger
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

#define FOUND_FOOD 21
void do_carve(struct char_data *ch, char *argument, int cmd)
{
    char           *arg1;
    char            buffer[MAX_STRING_LENGTH];
    struct obj_data *corpse;
    struct obj_data *food;
    int             i,
                    r_num;

    if (!ch->skills) {
        return;
    }

    if (!ch->skills[SKILL_RATION].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg1);

    if( !arg1 || 
        !(corpse = objectGetInRoom(ch, arg1, roomFindNum(ch->in_room)))) {
        send_to_char("That's not here.\n\r", ch);
        return;
    }

    if (!IS_CORPSE(corpse)) {
        send_to_char("You can't carve that!\n\r", ch);
        return;
    }

    if (corpse->weight < 70) {
        send_to_char("There is no good meat left on it.\n\r", ch);
        return;
    }

    if ((GET_MANA(ch) < 10) && !IS_IMMORTAL(ch)) {
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

    food = objectRead(FOUND_FOOD);

    StringToKeywords( "ration slice filet food", &food->keywords );
    sprintf(buffer, "a Ration%s", corpse->short_description + 10);
    food->short_description = (char *) strdup(buffer);
    food->action_description = (char *) strdup(buffer);
    sprintf(buffer, "%s is lying on the ground.", food->short_description);
    food->description = (char *) strdup(buffer);
    corpse->weight = corpse->weight - 50;

    i = number(1, 6);
    if (i == 6) {
        food->value[3] = 1;
    }
    objectPutInRoom(food, ch->in_room, UNLOCKED);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
}

void do_stalk(struct char_data *ch, char *argument, int cmd)
{
    byte            percent;

    dlog("in do_stalk");

    if (!ch->skills[SKILL_STALK].learned) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }
    
    if (IS_AFFECTED2(ch, AFF2_SKILL_SNEAK)) {
        if (IS_AFFECTED(ch, AFF_HIDE)) {
            REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
        }
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        send_to_char("You are no longer stalking.\n\r", ch);
        return;
    }
    
    if (MOUNTED(ch)) {
        send_to_char("Yeah... right... while mounted\n\r", ch);
        return;
    }
    
    if (!IS_AFFECTED(ch, AFF_SILENCE)) {
        if (HasExtraBits(ch, ITEM_HUM)) {
            send_to_char("Gonna be hard to stalk anyone with that thing "
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
        send_to_char("You stop stalking!", ch);
        REMOVE_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
    } else {
        send_to_char("You start walking softly.", ch);
        SET_BIT(ch->specials.affected_by2, AFF2_SKILL_SNEAK);
        WAIT_STATE(ch, PULSE_VIOLENCE * 1);
    }
}

void spell_giant_growth(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * +3 to hit +3 dam
     */

    if (affected_by_spell(victim, SPELL_GIANT_GROWTH)) {
        send_to_char("Already in effect\n\r", ch);
        return;
    }

    GET_HIT(victim) += number(1, 8);

    update_pos(victim);

    act("$n grows in size.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel larger!\n\r", victim);

    af.type = SPELL_GIANT_GROWTH;
    af.duration = 10;
    af.modifier = 3;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    af.location = APPLY_DAMROLL;
    af.modifier = 3;
    affect_to_char(victim, &af);
}

void cast_giant_growth(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_giant_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in giant growth.");
        break;
    }
}
/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
