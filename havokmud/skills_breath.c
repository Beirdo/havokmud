/*
 * HavokMUD - dragon breaths
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"


#define MAX_BREATHS 3

struct pbreath {
    int             vnum,
                    spell[MAX_BREATHS];
};

struct pbreath breath_potions[] = {
    { 3970, {SPELL_FIRE_BREATH, 0}},
    { 3971, {SPELL_GAS_BREATH, 0}},
    { 3972, {SPELL_FROST_BREATH, 0}},
    { 3973, {SPELL_ACID_BREATH, 0}},
    { 3974, {SPELL_LIGHTNING_BREATH, 0}},
    {0}
};

void cast_dragon_breath(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *potion)
{
    struct pbreath *scan;
    int             i;
    struct affected_type af;
    char            buf[MAX_STRING_LENGTH];

    if (!potion) {
        return;
    }
    for (scan = breath_potions; 
         scan->vnum && scan->vnum != obj_index[potion->item_number].virtual;
         scan++) {
        /* 
         * Empty Loop 
         */
    }

    if (scan->vnum == 0) {
        send_to_char("Hey, this potion isn't in my list!\n\r", ch);
        sprintf(buf, "unlisted breath potion %s %ld", potion->short_description,
                obj_index[potion->item_number].virtual);
        Log(buf);
        return;
    }

    for (i = 0; i < MAX_BREATHS && scan->spell[i]; i++) {
        if (!affected_by_spell(ch, scan->spell[i])) {
            af.type = scan->spell[i];
            af.duration = 1 + dice(1, 2);
            if (GET_CON(ch) < 4) {
                send_to_char("You are too weak to stomach the potion and spew "
                             "it all over the floor.\n\r", ch);
                act("$n gags and pukes glowing goop all over the floor.",
                    FALSE, ch, 0, ch, TO_NOTVICT);
                break;
            }

            if (level > MIN(GET_CON(ch) - 1, GetMaxLevel(ch))) {
                send_to_char("!GACK! You are too weak to handle the full power"
                             " of the potion.\n\r", ch);
                act("$n gags and flops around on the floor a bit.", FALSE,
                    ch, 0, ch, TO_NOTVICT);
                level = MIN(GET_CON(ch) - 1, GetMaxLevel(ch));
            }

            af.modifier = -level;
            af.location = APPLY_CON;
            af.bitvector = 0;
            affect_to_char(ch, &af);
            send_to_char("You feel powerful forces build within your "
                         "stomach...\n\r", ch);
        }
    }
}

void spell_fire_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }
    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else if (!saves_spell(victim, SAVING_SPELL - 4)) {
        BurnWings(victim);
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying; burn && 
         burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p burns to cinders.", 0, victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void cast_fire_breath(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can't cast it! 
         */
        spell_fire_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in firebreath!");
        break;
    }
}

void spell_frost_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int              dam = 0;
    struct obj_data *frozen;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }
    
    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FROST_BREATH);

    for (frozen = victim->carrying;
         frozen && ITEM_TYPE(frozen) != ITEM_DRINKCON &&
         ITEM_TYPE(frozen) != ITEM_POTION;
         frozen = frozen->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && frozen) {
            act("$p shatters.", 0, victim, frozen, 0, TO_CHAR);
            extract_obj(frozen);
        }
    }
}

void cast_frost_breath(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can't cast it! 
         */
        spell_frost_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in frostbreath!");
        break;
    }
}
void spell_acid_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int                   dam = 0;
    struct      obj_data *burn;
    
    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_ACID_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is completely corroded.", 0, victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void cast_acid_breath(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can't cast it! 
         */
        spell_acid_breath(level, ch, tar_ch, 0);
        break; 
    default:
        Log("Serious screw-up in acidbreath!");
        break;
    }
}

void spell_gas_breath(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct      obj_data *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed by the cloud of gas.", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void cast_gas_breath(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_gas_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in gasbreath!");
        break;
    }
}

void spell_lightning_breath(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int                  dam = 0;
    struct  obj_data    *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is completely electrocuted.", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void cast_lightning_breath(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can't cast it! 
         */
        spell_lightning_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in lightningbreath!");
        break;
    }
}

void spell_dehydration_breath(int level, struct char_data *ch,
                              struct char_data *victim,
                              struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
        af.type = SPELL_WEAKNESS;
        af.duration = 5 + level;
        af.modifier = -dice(1,6);
        af.location = APPLY_STR;
        affect_join(victim, &af, FALSE, FALSE);
        
    } else {
        af.type = SPELL_WEAKNESS;
        af.duration = 5 + level;
        af.modifier = -dice(1,6) + 6;
        af.location = APPLY_STR;
        affect_join(victim, &af, FALSE, FALSE);
    }
        
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
}

void spell_vapor_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    
    af.type = SPELL_BLINDNESS;
    af.location = APPLY_HITROLL;
    af.modifier = -12;
    af.duration = level / 2;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +40;
    affect_to_char(victim, &af);
}

void spell_sound_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);    

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    if (!saves_spell(victim, SAVING_BREATH)) {
       if (GET_POS(victim) > POSITION_STUNNED) {
           send_to_char("$c000WYou are stunned by the painful vibrations!\n\r",
                        victim);
           act("$n is stunned by the vibrations!", 
               FALSE, victim, NULL, NULL, TO_ROOM);
           GET_POS(victim) = POSITION_STUNNED;
        }
    } else {
        act("$n ceases to make noise!", TRUE, victim, NULL, NULL, TO_ROOM);
        send_to_char("You can't hear anything!\n\r", victim);
        af.type = SPELL_SILENCE;
        af.duration = 1;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SILENCE;
        affect_to_char(victim, &af);

        do_flee(victim, "", 0);
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
}

void spell_shard_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -12;
        af.duration = level;
        af.bitvector = AFF_BLIND;
        affect_to_char(victim, &af);

        af.location = APPLY_AC;
        af.modifier = +40;
        affect_to_char(victim, &af);
    }
        
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

    /*
     * And now for the damage on inventory
     */

    /*
     * DamageStuff(victim, FIRE_DAMAGE);
     */

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed by the projectiles", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_sleep_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        af.type = SPELL_SLEEP;
        af.duration = 5 + level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SLEEP;
        affect_join(victim, &af, FALSE, FALSE);
        
        act("$n goes to sleep.", TRUE, victim, NULL, NULL, TO_ROOM);
        GET_POS(victim) = POSITION_SLEEPING;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH); 
}

void spell_light_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    spell_blindness(level, ch, victim, 0);
    spell_blindness(level, ch, victim, 0);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blinding light", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_dark_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    
    if (saves_spell(victim, SAVING_BREATH)) {
        dam = GET_HIT(victim) * .50;
    } else {
        dam = GET_HIT(victim) * .75;
        
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -12;
        af.duration = level / 2;
        af.bitvector = AFF_BLIND;
        affect_to_char(victim, &af);

        af.location = APPLY_AC;
        af.modifier = +40;
        affect_to_char(victim, &af);
    }

    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
}

void spell_desertheat_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blistering heat", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_repulsion_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0,
                    attempt,
                    i;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
        Log("saved");
    } else {
        Log("failed save");
        if (GET_POS(victim) > POSITION_SLEEPING && 
            !IS_AFFECTED(victim, AFF_PARALYSIS) &&
            !IS_SET(victim->specials.affected_by2, AFF2_BERSERK) &&
            !IS_SET(victim->specials.affected_by2, AFF2_STYLE_BERSERK)) {
            Log("good to go");    
            for (i = 0; i < 6; i++) {
                attempt = i;
                if (CAN_GO(victim, attempt) &&
                    !IS_SET(real_roomp(EXIT(victim, 
                                            attempt)->to_room)->room_flags,
                            DEATH) &&
                    !IS_SET(real_roomp(EXIT(victim, 
                                            attempt)->to_room)->room_flags,
                            NO_FLEE)) {
                    act("$n is repulsed and flees!", 
                        TRUE, victim, 0, 0, TO_ROOM);
                    send_to_char("You are repulsed and flee head over "
                                 "heels.\n\r", victim);
                    if (victim->specials.fighting) {
                        stop_fighting(victim);
                    }
                    if (ch->specials.fighting == victim) {
                        stop_fighting(ch);
                    }
                    GET_POS(victim) = POSITION_STANDING;
                    MoveOne(victim, attempt);
                    Log("moving victim");
                    GET_MOVE(victim) = -100;
                    RemHated(ch, victim);
                }
            }
        }
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    for (burn = victim->carrying;
        burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
        burn->obj_flags.type_flag != ITEM_WAND &&
        burn->obj_flags.type_flag != ITEM_STAFF &&            
        burn->obj_flags.type_flag != ITEM_BOAT;
        burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blistering heat", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_slow_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        if (!affected_by_spell(victim, SPELL_SLOW)) {
            
            send_to_char("You feel very slow!\n\r", victim);
            
            af.type = SPELL_SLOW;
            af.duration = 10;
            af.modifier = 1;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_SLOW;
            affect_to_char(victim, &af);
        }
    }
        
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed.", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);

        }
    }
}

void spell_paralyze_breath(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int              dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        if (!affected_by_spell(victim, SPELL_PARALYSIS)) {
            
            send_to_char("Your limbs freeze in place!\n\r", victim);
            
            af.type = SPELL_PARALYSIS;
            af.duration = 4 + level;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARALYSIS;
            affect_join(victim, &af, FALSE, FALSE);
        }
    }
        
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed.", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);

        }
    }
}

void spell_lozenge_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed by the exploding lozenge", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
