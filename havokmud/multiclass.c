
/*
 **  0 = Mage, 1 = cleric, 3 = thief, 2 = fighter
 */

#include <stdio.h>
#include <string.h>

#include "protos.h"

/*
 * extern variables 
 */

extern struct room_data *world;
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern struct dex_app_type dex_app[];

int GetClassLevel(struct char_data *ch, int class)
{

    if (IS_SET(ch->player.class, class)) {
        return (GET_LEVEL(ch, CountBits(class) - 1));
    }
    return (0);
}

int CountBits(int class)
{
    if (class == 1) {
        return (1);
    }
    if (class == 2) {
        return (2);
    }
    if (class == 4) {
        return (3);
    }
    if (class == 8) {
        return (4);
    }
    if (class == 16) {
        return (5);
    }
    if (class == 32) {
        return (6);
    }
    if (class == 64) {
        return (7);
    }
    if (class == 128) {
        return (8);
    }
    if (class == 256) {
        return (9);
    }
    if (class == 512) {
        return (10);
    }
    if (class == 1024) {
        return (11);
    }
    if (class == 2048) {
        return (12);
    }
    if (class == CLASS_NECROMANCER) {
        return (13);
    }
}

int OnlyClass(struct char_data *ch, int class)
{
    int             i;

    for (i = 1; i <= CLASS_NECROMANCER; i *= 2) {
        if (GetClassLevel(ch, i) != 0 && i != class) {
            return (FALSE);
        }
    }
    return (TRUE);
}

int MainClass(struct char_data *ch, int indicator)
{
    if ((ch->specials.remortclass - 1) == indicator) {
        return (TRUE);
    }

    return (FALSE);
}

int HasClass(struct char_data *ch, int class)
{
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF) && 
        (!IS_SET(class, CLASS_MONK) || !IS_SET(class, CLASS_DRUID) ||
         !IS_SET(class, CLASS_BARBARIAN) || !IS_SET(class, CLASS_SORCERER) ||
         !IS_SET(class, CLASS_PALADIN) || !IS_SET(class, CLASS_RANGER) || 
         !IS_SET(class, CLASS_PSI) || !IS_SET(class, CLASS_NECROMANCER))) {
        /*
         * I have yet to figure out why we do this 
         * but is seems to be needed 
         */
        return (TRUE);
    }

    /*
     * was NPC 
     */
    if (IS_SET(ch->player.class, class)) {
        return (TRUE);
    }
    return FALSE;
}

int HowManyClasses(struct char_data *ch)
{
    short           i,
                    tot = 0;

    for (i = 0; i < MAX_CLASS; i++) {
        if (GET_LEVEL(ch, i)) {
            tot++;
        }
    }

    if (tot) {
        return (tot);
    } else {
        if (IS_SET(ch->player.class, CLASS_MAGIC_USER)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_WARRIOR)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_THIEF)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_CLERIC)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_DRUID)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_MONK)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_BARBARIAN)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_SORCERER)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_PALADIN)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_RANGER)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_PSI)) {
            tot++;
        }
        if (IS_SET(ch->player.class, CLASS_NECROMANCER)) {
            tot++;
        }
    }

    return (tot);
}

int BestFightingClass(struct char_data *ch)
{

    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
        return (WARRIOR_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
        return (PALADIN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
        return (BARBARIAN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
        return (RANGER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
        return (CLERIC_LEVEL_IND);
    }
    if (GET_LEVEL(ch, DRUID_LEVEL_IND)) {
        return (DRUID_LEVEL_IND);
    }
    if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
        return (MONK_LEVEL_IND);
    }
    if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
        return (THIEF_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PSI_LEVEL_IND)) {
        return (PSI_LEVEL_IND);
    }
    if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
        return (MAGE_LEVEL_IND);
    }
    if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) {
        return (SORCERER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND)) {
        return (NECROMANCER_LEVEL_IND);
    }
    Log("Massive error.. character has no recognized class.");
    Log(GET_NAME(ch));
    assert(0);

    return (1);
}

/*
 * If the char has a fighting type of class.. return TRUE (GH) 
 */
int HasFightingClass(struct char_data *ch)
{
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, DRUID_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
        return (TRUE);
    }
    if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
        return (TRUE);
    }
    return (FALSE);
}

int BestThiefClass(struct char_data *ch)
{
    if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
        return (THIEF_LEVEL_IND);
    }
    if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
        return (MONK_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PSI_LEVEL_IND)) {
        return (PSI_LEVEL_IND);
    }
    if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
        return (MAGE_LEVEL_IND);
    }
    if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) {
        return (SORCERER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
        return (WARRIOR_LEVEL_IND);
    }
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
        return (BARBARIAN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
        return (RANGER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
        return (PALADIN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, DRUID_LEVEL_IND)) {
        return (DRUID_LEVEL_IND);
    }
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
        return (CLERIC_LEVEL_IND);
    }
    if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND)) {
        return (NECROMANCER_LEVEL_IND);
    }
    Log("Massive error.. character has no recognized class.");
    Log(GET_NAME(ch));
    assert(0);

    return (1);
}

int BestMagicClass(struct char_data *ch)
{
    if (GET_LEVEL(ch, MAGE_LEVEL_IND)) {
        return (MAGE_LEVEL_IND);
    }
    if (GET_LEVEL(ch, SORCERER_LEVEL_IND)) {
        return (SORCERER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, DRUID_LEVEL_IND)) {
        return (DRUID_LEVEL_IND);
    }
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
        return (CLERIC_LEVEL_IND);
    }
    if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND)) {
        return (NECROMANCER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
        return (PALADIN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, PSI_LEVEL_IND)) {
        return (PSI_LEVEL_IND);
    }
    if (GET_LEVEL(ch, RANGER_LEVEL_IND)) {
        return (RANGER_LEVEL_IND);
    }
    if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
        return (THIEF_LEVEL_IND);
    }
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
        return (WARRIOR_LEVEL_IND);
    }
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
        return (BARBARIAN_LEVEL_IND);
    }
    if (GET_LEVEL(ch, MONK_LEVEL_IND)) {
        return (MONK_LEVEL_IND);
    }
    Log("Massive error.. character has no recognized class.");
    Log(GET_NAME(ch));
    ch->player.class = 4;

    return (1);
}

int GetSecMaxLev(struct char_data *ch)
{
    return (GetALevel(ch, 2));
}

int GetALevel(struct char_data *ch, int which)
{
    byte            ind[MAX_CLASS],
                    j,
                    k,
                    i;

    for (i = 0; i < MAX_CLASS; i++) {
        ind[i] = GET_LEVEL(ch, i);
    }

    /*
     *  chintzy sort. (just to prove that I did learn something in college)
     */

    for (i = 0; i < MAX_CLASS - 1; i++) {
        for (j = i + 1; j < MAX_CLASS; j++) {
            if (ind[j] > ind[i]) {
                k = ind[i];
                ind[i] = ind[j];
                ind[j] = k;
            }
        }
    }

    if (which > -1 && which < 4) {
        return (ind[which]);
    }
}

int GetThirdMaxLev(struct char_data *ch)
{
    return (GetALevel(ch, 3));
}

int GetMaxLevel(struct char_data *ch)
{
    register int    max = 0,
                    i;

    for (i = 0; i <= MAX_CLASS - 1; i++) {
        if (GET_LEVEL(ch, i) > max) {
            max = GET_LEVEL(ch, i);
        }
    }

    return (max);
}

int GetTotLevel(struct char_data *ch)
{
    int             max = 0,
                    i;

    for (i = 0; i < MAX_CLASS; i++) {
        max += GET_LEVEL(ch, i);
    }

    return (max);

}

void StartLevels(struct char_data *ch)
{
    if (IS_SET(ch->player.class, CLASS_MAGIC_USER)) {
        advance_level(ch, MAGE_LEVEL_IND);
    }

    if (IS_SET(ch->player.class, CLASS_SORCERER)) {
        advance_level(ch, SORCERER_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_CLERIC)) {
        advance_level(ch, CLERIC_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_WARRIOR)) {
        advance_level(ch, WARRIOR_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_THIEF)) {
        advance_level(ch, THIEF_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_DRUID)) {
        advance_level(ch, DRUID_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_MONK)) {
        advance_level(ch, MONK_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_BARBARIAN)) {
        advance_level(ch, BARBARIAN_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_PALADIN)) {
        advance_level(ch, PALADIN_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_RANGER)) {
        advance_level(ch, RANGER_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_PSI)) {
        advance_level(ch, PSI_LEVEL_IND);
    }
    if (IS_SET(ch->player.class, CLASS_NECROMANCER)) {
        advance_level(ch, NECROMANCER_LEVEL_IND);
    }
}

int BestClassIND(struct char_data *ch)
{

    int             max = 0,
                    class = 0,
                    i;

    for (i = 0; i < MAX_CLASS; i++) {
        if (max < GET_LEVEL(ch, i)) {
            max = GET_LEVEL(ch, i);
            class = i;
        }
    }

    assert(max > 0);
    return (class);

}

int BestClassBIT(struct char_data *ch)
{

    int             max = 0,
                    class = 0,
                    i;

    for (i = 0; i < MAX_CLASS; i++)
        if (max < GET_LEVEL(ch, i)) {
            max = GET_LEVEL(ch, i);
            class = i;
        }

    assert(max > 0);

    switch (class)
    {
    case MAGE_LEVEL_IND:
        return (1);
        break;
    case CLERIC_LEVEL_IND:
        return (2);
        break;
    case WARRIOR_LEVEL_IND:
        return (4);
        break;
    case THIEF_LEVEL_IND:
        return (8);
        break;
    case DRUID_LEVEL_IND:
        return (16);
        break;
    case MONK_LEVEL_IND:
        return (32);
        break;
    case BARBARIAN_LEVEL_IND:
        return (64);
        break;
    case SORCERER_LEVEL_IND:
        return (128);
        break;
    case PALADIN_LEVEL_IND:
        return (256);
        break;
    case RANGER_LEVEL_IND:
        return (512);
        break;
    case PSI_LEVEL_IND:
        return (1024);
        break;
    case NECROMANCER_LEVEL_IND:
        return (2048);
        break;
    default:
        Log("Error in BestClassBIT");
        break;
    }

    return (class);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
