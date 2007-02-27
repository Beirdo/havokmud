#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "protos.h"
#include "externs.h"


#define VOID_PULL_TIME   30
#define FORCE_RENT_TIME  40

char           *ClassTitles(struct char_data *ch)
{
    unsigned char   i,
                    count = 0;
    static char     buf[256];

    for (i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
        if (GET_LEVEL(ch, i)) {
            count++;
            if (count > 1) {
                sprintf(buf + strlen(buf), "/%s", 
                        GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
            } else {
                sprintf(buf, "%s",
                        GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
            }
        }
    }
    return (buf);
}

/*
 * When age < 15 return the value p0 
 * When age in 15..29 calculate the line between p1 & p2 
 * When age in 30..44 calculate the line between p2 & p3 
 * When age in 45..59 calculate the line between p3 & p4 
 * When age in 60..79 calculate the line between p4 & p5 
 * When age >= 80 return the value p6 
 */
int graf(int age, int race, int p0, int p1, int p2, int p3, int p4, int
         p5, int p6)
{
    if (age < races[race].start) {
        return (p0);
    } else if (age <= races[race].mature) {
        return (int) (p1 + (((age - races[race].start) * (p2 - p1)) /
                            (races[race].mature - races[race].start)));
    } else if (age <= races[race].middle) {
        return (int) (p2 + (((age - races[race].mature) * (p3 - p2)) /
                            (races[race].middle - races[race].mature)));
    } else if (age <= races[race].old) {
        return (int) (p3 + (((age - races[race].middle) * (p4 - p3)) /
                            (races[race].old - races[race].middle)));
    } else if (age <= races[race].ancient) {
        return (int) (p4 + (((age - races[race].old) * (p5 - p4)) /
                            (races[race].ancient - races[race].old)));
    } else if (age <= races[race].venerable) {
        return (int) (p5 + (((age - races[race].ancient) * (p6 - p5)) /
                            (races[race].venerable - races[race].ancient)));
    } else {
        return (int) (p6 + (((age - races[race].ancient) * (p6 - p6 - p5)) /
                            (races[race].venerable - races[race].ancient)));
    }
}

/*
 * The three MAX functions define a characters Effective maximum 
 * Which is NOT the same as the ch->points.max_xxxx !!! 
 */
int mana_limit(struct char_data *ch)
{
    int             max,
                    tmp;

    max = 0;

    if (IS_NPC(ch)) {
        return (100);
    }
    if (HasClass(ch, CLASS_MAGIC_USER)) {
        max += 100;
        max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
    }

    /*
     * actually this is worthless as Sorcerer's do not 
     * use mana at all.... 
     */
    if (HasClass(ch, CLASS_SORCERER)) {
        max += 100;
        max += GET_LEVEL(ch, SORCERER_LEVEL_IND) * 5;
    }

    if (HasClass(ch, CLASS_PSI)) {
        max += 100;
        max += GET_LEVEL(ch, PSI_LEVEL_IND) * 5;
    }

    if (HasClass(ch, CLASS_PALADIN)) {
        max += 100;
        max += (GET_LEVEL(ch, PALADIN_LEVEL_IND) / 4) * 5;
    }

    if (HasClass(ch, CLASS_RANGER)) {
        max += 100;
        max += (GET_LEVEL(ch, RANGER_LEVEL_IND) / 4) * 5;
    }

    if (HasClass(ch, CLASS_CLERIC)) {
        max += 100;
        max += (GET_LEVEL(ch, CLERIC_LEVEL_IND) / 3) * 5;
    }

    if (HasClass(ch, CLASS_DRUID)) {
        max += 100;
        max += (GET_LEVEL(ch, DRUID_LEVEL_IND) / 3) * 5;
    }

    if (HasClass(ch, CLASS_THIEF)) {
        max += 100;
    }

    if (HasClass(ch, CLASS_WARRIOR)) {
        max += 100;
    }

    if (HasClass(ch, CLASS_MONK)) {
        max += 100;
    }
    if (HasClass(ch, CLASS_NECROMANCER)) {
        max += 100;
        max += GET_LEVEL(ch, NECROMANCER_LEVEL_IND) * 4;
    }

    max /= HowManyClasses(ch);

    /*
     * new classes should be inserted here. 
     */

    tmp = 0;

    tmp = GET_INT(ch) / 3;
    tmp += 2;
    tmp = tmp * 3;

    max += tmp;
    max += ch->points.max_mana;

    /*
     * Add class mana maximums here...
     */

    if (OnlyClass(ch, CLASS_BARBARIAN)) {
        max = 100;
    }

    return (max);
}

int hit_limit(struct char_data *ch)
{
    int             max;
    int             race;

    if (IS_PC(ch)) {
        struct time_info_data ma;
        age2(ch, &ma);
        race = GET_RACE(ch);
        max = (ch->points.max_hit) + 
              (graf(ma.year, race, 2, 4, 17, 14, 8, 0, -10));
    } else {
        max = (ch->points.max_hit);
    }

    /*
     * Class/Level calculations 
     * Skill/Spell calculations 
     */

    return (max);
}

int move_limit(struct char_data *ch)
{
    int             max;

    if (!IS_NPC(ch)) {
        max = 100 + GET_CON(ch);
    } else {
        max = ch->points.max_move;
    }

    if (IsRideable(ch)) {
        max *= 2;
    }
    if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_ROCK_GNOME ||
        GET_RACE(ch) == RACE_DEEP_GNOME || GET_RACE(ch) == RACE_FOREST_GNOME) {
        max -= 35;
    } else if (GET_RACE(ch) == RACE_MOON_ELF || GET_RACE(ch) == RACE_DROW ||
             GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF
             || GET_RACE(ch) == RACE_SEA_ELF || GET_RACE(ch) == RACE_HALF_ELF){
        max += 20;
    } else if (GET_RACE(ch) == RACE_AVARIEL) {
        max += 35;
    } else if (GET_RACE(ch) == RACE_HALFLING) {
        max -= 45;
    } else if (GET_RACE(ch) == RACE_HALF_GIANT) {
        max += 60;
    } else if (GET_RACE(ch) == RACE_HALF_OGRE) {
        max += 50;
    } else if (GET_CLASS(ch) == CLASS_BARBARIAN) {
        max += 45;
    }

    max += ch->points.max_move;

    return (max);
}

/*
 * manapoint gain pr. game hour 
 */
int mana_gain(struct char_data *ch)
{
    int             gain;
    int             race;

    if ((IS_NPC(ch)) && (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
        /*
         * Neat and fast 
         */
        gain = 8;
    } else {
        struct time_info_data ma;
        age2(ch, &ma);
        race = GET_RACE(ch);
#ifdef NEWGAIN
        gain = graf(ma.year, race, 3, 9, 12, 16, 20, 16, 2);
#else
        gain = graf(ma.year, race, 2, 4, 6, 8, 10, 16, 2);
#endif
    }

    /*
     * Position calculations 
     */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
        gain += gain;
        break;
    case POSITION_RESTING:
        gain += (gain >> 1);
        break;
    case POSITION_SITTING:
        gain += (gain >> 2);
        break;
    }

    gain += gain;

    gain += wis_app[(int)GET_WIS(ch)].bonus * 2;

    gain += ch->points.mana_gain;

    if (IS_AFFECTED(ch, AFF_POISON)) {
        gain >>= 2;
    }
    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0)) {
        gain >>= 2;
    }
    if (GET_RACE(ch) == RACE_MOON_ELF || GET_RACE(ch) == RACE_ROCK_GNOME ||
        GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
        GET_RACE(ch) == RACE_SEA_ELF || GET_RACE(ch) == RACE_FOREST_GNOME || 
        GET_RACE(ch) == RACE_DROW || GET_RACE(ch) == RACE_HALF_ELF || 
        GET_RACE(ch) == RACE_DEEP_GNOME || GET_RACE(ch) == RACE_AVARIEL) {
        gain += 2;
    }
    if (GET_COND(ch, DRUNK) > 10) {
        gain += (gain >> 1);
    } else if (GET_COND(ch, DRUNK) > 0) {
        gain += (gain >> 2);
    }
    /*
     * Class calculations 
     */

    /*
     * magic type people get quicker mana re-gen, fighter/paladin/rangers
     * get it slower 
     */
    if (!HasClass (ch, CLASS_MAGIC_USER | CLASS_SORCERER | CLASS_CLERIC | 
                       CLASS_DRUID | CLASS_PSI)) {
        gain -= 2;
    }

    /*
     * these guys get mana even slower 
     */
    if (HasClass(ch, CLASS_BARBARIAN)) {
        gain -= 2;
    }
    /*
     * Skill/Spell calculations 
     */

    if (affected_by_spell(ch, SKILL_MEDITATE)) {
        gain += 3;
    }
    /*
     * main class stuff here 
     */
    if (ch->specials.remortclass == MAGE_LEVEL_IND + 1) {
        gain += 10;
    } else if (ch->specials.remortclass == DRUID_LEVEL_IND + 1) {
        gain += 5;
    } else if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
        gain += 5;
    }
    return (gain);
}

int hit_gain(struct char_data *ch)
{
    int             gain,
                    dam,
                    i,
                    race;

    /*
     * Hitpoint gain pr. game hour 
     */

    if (IS_NPC(ch)) {
        gain = 8;
        /*
         * Neat and fast 
         */
    } else {

        if (GET_POS(ch) == POSITION_FIGHTING) {
            gain = 0;
        } else {
            struct time_info_data ma;
            age2(ch, &ma);
            race = GET_RACE(ch);
#ifdef NEWGAIN

            gain = graf(ma.year, race, 3, 9, 12, 16, 12, 6, 1);
#else
            gain = graf(ma.year, race, 2, 4, 6, 8, 6, 3, 1);
#endif
        }
    }

    /*
     * Position calculations 
     */

    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
        gain += gain;
        break;
    case POSITION_RESTING:
        gain += gain >> 1;
        break;
    case POSITION_SITTING:
        gain += gain >> 2;
        break;
    }

    if (GET_RACE(ch) == RACE_DWARF) {
        gain += 2;
    }
    if (GET_RACE(ch) == RACE_HALFLING) {
        gain += 1;
    }
    if (GET_RACE(ch) == RACE_HALF_GIANT) {
        gain += 3;
    }
    if (GET_RACE(ch) == RACE_HALF_OGRE) {
        gain += 2;
    }
    if (GET_RACE(ch) == RACE_HALF_ORC) {
        gain += 1;
    }
    if (GET_CLASS(ch) == CLASS_BARBARIAN) {
        gain += 4;
    }
    gain += con_app[(int)GET_CON(ch)].hitp / 2;
    if (IS_AFFECTED(ch, AFF_POISON)) {
        dam = 0;
        gain = 0;
        if (GET_RACE(ch) == RACE_HALFLING) {
            dam += number(1, 20);
        } else {
            dam += number(10, 32);
        }
        if (affected_by_spell(ch, SPELL_SLOW_POISON)) {
            dam /= 4;
        }
        damage(ch, ch, dam, SPELL_POISON);
    }

    if (affected_by_spell(ch, SPELL_DECAY)) {
        dam = 0;
        gain = 0;
        if (GET_RACE(ch) == RACE_HALFLING) {
            dam += number(15, 35);
        } else {
            dam += number(35, 55);
        }
        damage(ch, ch, dam, SPELL_DECAY);
    }

    if (affected_by_spell(ch, SPELL_DISEASE)) {
        dam = 0;
        gain = 0;
        if (GET_RACE(ch) == RACE_HALFLING) {
            dam += number(1, 12);
        } else {
            dam += number(5, 22);
        }
        damage(ch, ch, dam, SPELL_DISEASE);
    }

    if (IS_AFFECTED2(ch, AFF2_HEAT_STUFF)) {
        dam = 0;
        /*
         * count items in eq 
         */
        for (i = 0; i <= HOLD; i++) {
            if (ch->equipment[i]) {
                dam += 2;
            }
        }
        damage(ch, ch, dam, SPELL_HEAT_STUFF);
    }

    gain += ch->points.hit_gain;

    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0)) {
        gain >>= 4;
    }
    if (GET_COND(ch, DRUNK) > 10) {
        gain += (gain >> 1);
    } else if (GET_COND(ch, DRUNK) > 0) {
        gain += (gain >> 2);
    }
    /*
     * Class/Level calculations 
     */

    /*
     * non-warrior types get slower hps re-gen 
     */
    if (!HasClass (ch, CLASS_WARRIOR | CLASS_PALADIN | CLASS_RANGER |
                       CLASS_BARBARIAN)) {
        gain -= 2;

        if (gain < 0 && !ch->specials.fighting) {
            /* 
             * give them a small break 
             */
            damage(ch, ch, gain * -1, TYPE_SUFFERING);
        }
    }

    /*
     * Skill/Spell calculations 
     */
    if (ch->specials.remortclass == WARRIOR_LEVEL_IND + 1) {
        gain += 8;
    } else if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
        gain += 5;
    } else if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
        gain += 5;
    }
    return (gain);
}

int move_gain(struct char_data *ch)
{
    int             gain,
                    race;

    if (IS_NPC(ch)) {
        gain = 22;
        if (IsRideable(ch)) {
            gain += gain / 2;
        }
    } else {
        struct time_info_data ma;
        age2(ch, &ma);
        race = GET_RACE(ch);
        if (GET_POS(ch) != POSITION_FIGHTING) {
#ifdef NEWGAIN
            gain = graf(ma.year, race, 15, 21, 25, 28, 20, 10, 3);
#else
            gain = graf(ma.year, race, 10, 15, 20, 22, 15, 7, 1);
#endif

        } else {
            gain = 0;
        }
    }

    /*
     * Position calculations 
     */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
        gain += (gain >> 2);
        break;
    case POSITION_RESTING:
        gain += (gain >> 3);
        break;
    case POSITION_SITTING:
        gain += (gain >> 4);
        break;
    }

    if (GET_RACE(ch) == RACE_DWARF) {
        gain += 4;
    }
    if (GET_RACE(ch) == RACE_HALF_GIANT) {
        gain += 6;
    }
    if (GET_RACE(ch) == RACE_HALF_OGRE) {
        gain += 5;
    }
    if (GET_RACE(ch) == RACE_HALF_ORC) {
        gain += 4;
    }
    gain += ch->points.move_gain;

    if (IS_AFFECTED(ch, AFF_POISON)) {
        gain >>= 5;
    }
    if ((GET_COND(ch, FULL) == 0) || (GET_COND(ch, THIRST) == 0)) {
        gain >>= 3;
    }
    /*
     * Class specific stuff 
     */

    /*
     * non-thief/monks types regen move slower 
     */
    if (!HasClass(ch, CLASS_THIEF | CLASS_MONK)) {
        gain -= 2;
    }
    /*
     * They were regenning too slowly... 
     * added that for now -MW
     */
    gain += 15;

    if (ch->specials.remortclass == WARRIOR_LEVEL_IND + 1) {
        gain += 5;
    } else if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
        gain += 8;
    }
    return (gain);
}

/*
 * Gain maximum in various points 
 */
void advance_level(struct char_data *ch, int class)
{
    int             add_hp,
                    i,
                    add_mana = 0,
                    add_move = 0;
    char            bufx[50];
    int             level;

    if (class > MAX_CLASS) {
        Log("Bad advance class.. no such class");
        return;
    }
    add_move = GET_MAX_MOVE(ch);
    add_mana = GET_MAX_MANA(ch);
    level = GET_LEVEL(ch, class);
    if (level > 0 && GET_EXP(ch) < classes[class].levels[level + 1].exp) {
        /*
         * they can't advance here 
         */
        Log("Bad advance_level, can't advance in this class.");
        return;
    }

    GET_LEVEL(ch, class) += 1;
    level++;

    if (class == WARRIOR_LEVEL_IND || class == BARBARIAN_LEVEL_IND || 
        class == PALADIN_LEVEL_IND || class == RANGER_LEVEL_IND) {
        add_hp = con_app[(int)GET_RCON(ch)].hitp;
    } else {
        add_hp = MIN(con_app[(int)GET_RCON(ch)].hitp, 2);
    }

    switch (class) {
    case MAGE_LEVEL_IND:
    case SORCERER_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 6);
        } else {
            add_hp += 1;
        }
        break;

    case CLERIC_LEVEL_IND:
        if (level < 12) {
            add_hp += number(3, 15);
        } else {
            add_hp += 3;
        }
        break;

    case THIEF_LEVEL_IND:
    case PSI_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 10);
        } else {
            add_hp += 2;
        }
        break;

    case WARRIOR_LEVEL_IND:
    case PALADIN_LEVEL_IND:
        if (level < 10) {
            add_hp += number(3, 16);
        } else {
            add_hp += 4;
        }
        break;

    case RANGER_LEVEL_IND:
        if (level < 11) {
            add_hp += number(3, 13);
        } else {
            add_hp += 4;
        }
        break;

    case BARBARIAN_LEVEL_IND:
        if (level < 15) {
            add_hp += number(3, 22);
        } else {
            add_hp += 6;
        }
        break;

    case DRUID_LEVEL_IND:
        if (level < 15) {
            add_hp += number(2, 14);
        } else {
            add_hp += 3;
        }
        break;

    case MONK_LEVEL_IND:
        if (level < 17) {
            add_hp += number(2, 10);
        } else {
            add_hp += 2;
        }
        break;

    case NECROMANCER_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 10);
        } else {
            add_hp += 2;
        }
        break;
    }

    add_hp /= HowManyClasses(ch);

    add_hp = MAX(1, add_hp);
    sprintf(bufx, "You feel healthier. +%d Health\n\r", add_hp);
    send_to_char(bufx, ch);
    ch->points.max_hit += add_hp;

    if (ch->specials.spells_to_learn < 70) {
        ch->specials.spells_to_learn += MAX(1, MAX(2, 
                    wis_app[(int)GET_RWIS(ch)].bonus) / HowManyClasses(ch));
    } else {
        send_to_char("Practices: Use them or lose them.\n\r", ch);
    }

    ClassSpecificStuff(ch);

    if (IS_IMMORTAL(ch)) {
        for (i = 0; i < 3; i++) {
            ch->specials.conditions[i] = -1;
        }
    }
    if (add_mana != GET_MAX_MANA(ch)) {
        sprintf(bufx, "You feel more magical.  +%d Mana\n\r",
                GET_MAX_MANA(ch) - add_mana);
        send_to_char(bufx, ch);
    }

    if (add_move != GET_MAX_MOVE(ch)) {
        sprintf(bufx, "You feel your endurance increasing.  +%d Movement\n\r",
                GET_MAX_MOVE(ch) - add_move);
        send_to_char(bufx, ch);
    }
}

/*
 * Lose in various points 
 */
void drop_level(struct char_data *ch, int class, int goddrain)
{
    int             add_hp;
    int             level;

    if (!goddrain && IS_IMMORTAL(ch)) {
            return;
    }

    if (GetMaxLevel(ch) == 1) {
        return;
    }

    add_hp = con_app[(int)GET_RCON(ch)].hitp;
    level = GET_LEVEL(ch, class);

    switch (class) {
    case MAGE_LEVEL_IND:
    case SORCERER_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 8);
        } else {
            add_hp += 2;
        }
        break;

    case CLERIC_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 12);
        } else {
            add_hp += 5;
        }
        break;

    case THIEF_LEVEL_IND:
    case PSI_LEVEL_IND:
        if (level < 12) {
            add_hp += number(2, 10);
        } else {
            add_hp += 4;
        }
        break;

    case WARRIOR_LEVEL_IND:
    case PALADIN_LEVEL_IND:
        if (level < 10) {
            add_hp += number(2, 16);
        } else {
            add_hp += 6;
        }
        break;

    case RANGER_LEVEL_IND:
        if (level < 11) {
            add_hp += number(2, 13);
        } else {
            add_hp += 6;
        }
        break;

    case DRUID_LEVEL_IND:
        if (level < 15) {
            add_hp += number(2, 12);
        } else {
            add_hp += 5;
        }
        break;

    case MONK_LEVEL_IND:
        if (level < 17) {
            add_hp += number(2, 10);
        } else {
            add_hp += 4;
        }
        break;

    case BARBARIAN_LEVEL_IND:
    case NECROMANCER_LEVEL_IND:
        if (level < 15) {
            add_hp += number(2, 14);
        } else {
            add_hp += 8;
        }
        break;
    }

    switch( class ) {
    case WARRIOR_LEVEL_IND:
    case PALADIN_LEVEL_IND:
        add_hp = MAX(add_hp, 6);
        break;

    case BARBARIAN_LEVEL_IND:
        add_hp = MAX(add_hp, 8);
        break;

    case RANGER_LEVEL_IND:
    case CLERIC_LEVEL_IND:
    case DRUID_LEVEL_IND:
        add_hp = MAX(add_hp, 5);
        break;

    case THIEF_LEVEL_IND:
    case PSI_LEVEL_IND:
    case MONK_LEVEL_IND:
        add_hp = MAX(add_hp, 4);
        break;

    case MAGE_LEVEL_IND:
    case SORCERER_LEVEL_IND:
    case NECROMANCER_LEVEL_IND:
        add_hp = MAX(add_hp, 3);
        break;
    }

    add_hp /= HowManyClasses(ch);

    if (add_hp <= 2) {
        add_hp = 3;
    }

    ch->points.max_hit -= MAX(1, add_hp);
    if (ch->points.max_hit < 1) {
        ch->points.max_hit = 1;
    }

    ch->specials.spells_to_learn -=
        MAX(1, MAX(2, wis_app[(int)GET_RWIS(ch)].bonus) / HowManyClasses(ch));


    if (level == 1) {
        if (ch->points.max_hit > 20) {
            ch->points.max_hit = 20;
        }
    } else {
        level--;
        GET_LEVEL(ch, class) = level;
    }

    if (ch->points.exp > MIN(classes[class].levels[level].exp, GET_EXP(ch))) {
        ch->points.exp = MIN(classes[class].levels[level].exp, GET_EXP(ch));
    }

    if (ch->points.exp < 0) {
        ch->points.exp = 0;
    }

    send_to_char("You lose a level.\n\r", ch);
}

void set_title(struct char_data *ch)
{

    char            buf[256];

    sprintf(buf, "%s the %s %s", GET_NAME(ch), races[ch->race].racename,
                 ClassTitles(ch));

    if (GET_TITLE(ch)) {
        free(GET_TITLE(ch));
        CREATE(GET_TITLE(ch), char, strlen(buf) + 1);
    } else {
        CREATE(GET_TITLE(ch), char, strlen(buf) + 1);
    }

    strcpy(GET_TITLE(ch), buf);
}

void gain_exp(struct char_data *ch, int gain)
{
    int             i;
    char            buf[256];
    short           chrace;
    int             level;

    if( IS_IMMORTAL(ch) || gain == 0 ) {
        return;
    }

    save_char(ch, AUTO_RENT);

    if (!IS_PC(ch) && ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
        if (ch->master->in_room == ch->in_room && gain > 1) {
            gain /= 2;
            sprintf(buf, "you gain $N's share of %d exp", gain);
            act(buf, 0, ch->master, 0, ch, TO_CHAR);
            gain_exp(ch->master, gain);
        }
        return;
    }

    if(!IS_PC(ch)) {
        return;
    }

    if (gain > 0) {
        gain /= HowManyClasses(ch);

        if (GetMaxLevel(ch) == 1) {
            gain *= 2;
        }

        if (ch->desc && ch->desc->original) {
            chrace = ch->desc->original->race;
        } else {
            chrace = GET_RACE(ch);
        }

        for (i = 0; i < MAX_CLASS; i++) {
            level = GET_LEVEL(ch, i);
            if (level && level < races[chrace].racialMax[i] && 
                level < MAX_MORT) {
                if (GET_EXP(ch) >= classes[i].levels[level + 2].exp - 1) {
                    /* 
                     * is already maxxed 
                     */
                    send_to_char("You must gain at your guild before"
                                 " you can acquire more experience.\n\r", ch);
                    return;
                } else if (GET_EXP(ch) >= classes[i].levels[level + 1].exp &&
                           GET_EXP(ch) + gain >= 
                               classes[i].levels[level + 2].exp - 1) {
                    /* 
                     * char was levelled, not maxxed 
                     * but hits max with this one 
                     */
                    GET_EXP(ch) = classes[i].levels[level + 2].exp - 1;  

                    /* 
                     * Let's tell him then.  
                     */
                    send_to_char("You must gain at your guild before you "
                                 "can acquire more experience.\n\r", ch);
                    return;
                } else if (GET_EXP(ch) < classes[i].levels[level + 1].exp &&
                           GET_EXP(ch) + gain >= 
                               classes[i].levels[level + 1].exp) {
                    /* 
                     * this is the levelling stroke 
                     */
                    sprintf(buf, "You have gained enough to be a(n) %s.\n\r",
                            GET_CLASS_TITLE(ch, i, level + 1));
                    send_to_char(buf, ch);
                    send_to_char("You must return to a guild to earn "
                                 "the level.\n\r", ch);

                    if (GET_EXP(ch) + gain >= 
                            classes[i].levels[level + 2].exp) {
                        /* 
                         * this is the maxxing stroke 
                         */
                        GET_EXP(ch) = classes[i].levels[level + 2].exp - 1;
                        
                        /* 
                         * let's notify them 
                         */
                        send_to_char("You must gain at your guild before you "
                                     "can acquire more experience.\n\r", ch);
                        return;
                    }
                }
            }
        }

        /* Hard limit so we don't overflow */
        gain = MIN( gain, INT_MAX - GET_EXP(ch) );

        GET_EXP(ch) += gain;

        if (!IS_SET(ch->specials.act, PLR_LEGEND)) {
            CheckLegendStatus(ch);
        }
        
        for (i = 0; i < MAX_CLASS; i++) {
            level = GET_LEVEL(ch, i);
            if (level && level < races[chrace].racialMax[i] && 
                level < MAX_MORT &&
                GET_EXP(ch) > classes[i].levels[level + 2].exp) {
                /* Max the XP to one less than 2 levels above current, unless
                 * the level is MAX_MORT
                 */
                GET_EXP(ch) = classes[i].levels[level + 2].exp - 1;
            }
        }
    } else {
        /* 
         * Negative gain 
         */
        GET_EXP(ch) += gain;
        if (GET_EXP(ch) < 0) {
            gain = -GET_EXP(ch);
            GET_EXP(ch) = 0;
        }

        oldSendOutput(ch, "$c000RYou have lost $c000w%d $c000Rexperience!", -gain);
    }
}

void gain_exp_regardless(struct char_data *ch, int gain, int class)
{
    int             i;
    bool            is_altered = FALSE;

    save_char(ch, AUTO_RENT);
    if (!IS_NPC(ch)) {
        if (gain > 0) {
            GET_EXP(ch) += gain;

            for (i = 0; 
                 i < ABS_MAX_LVL && classes[class].levels[i].exp <= GET_EXP(ch);
                 i++) {
                if (i > GET_LEVEL(ch, class)) {
                    send_to_char("You raise a level\n\r", ch);
                    GET_LEVEL(ch, class) = i;
                    advance_level(ch, class);
                    is_altered = TRUE;
                }
            }
        }

        if (gain < 0) {
            GET_EXP(ch) += gain;
        }
        if (GET_EXP(ch) < 0) {
            GET_EXP(ch) = 0;
        }
    }

    if (is_altered) {
        set_title(ch);
    }
}

void gain_condition(struct char_data *ch, int condition, int value)
{
    bool            intoxicated;

    if (GET_COND(ch, condition) == -1) {
        /* 
         * No change 
         */
        return;
    }

    intoxicated = (GET_COND(ch, DRUNK) > 0);

    GET_COND(ch, condition) += value;

    GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
    GET_COND(ch, condition) = MIN(24, GET_COND(ch, condition));

    if (GET_COND(ch, condition)) {
        return;
    }
    switch (condition) {
    case FULL:
        send_to_char("$c0009You are hungry.$c0007\n\r", ch);
        break;
    case THIRST:
        send_to_char("$c0009You are thirsty.$c0007\n\r", ch);
        break;
    case DRUNK:
        if (intoxicated)
            send_to_char("You are now sober.\n\r", ch);
        break;
    default:
        break;
    }
}

void check_idling(struct char_data *ch)
{
    char            buf[255];
    struct obj_cost cost;

    if (++(ch->specials.timer) == 8) {
        do_save(ch, "", 0);

    } else if (ch->specials.timer == VOID_PULL_TIME &&
               ch->in_room != NOWHERE && ch->in_room != 0) {
        ch->specials.was_in_room = ch->in_room;
        if (ch->specials.fighting) {
            stop_fighting(ch->specials.fighting);
            stop_fighting(ch);
        }
        act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
        send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, 0);
    } else if (ch->specials.timer == FORCE_RENT_TIME) {
        do_save(ch, "", 0);
        if (ch->in_room != NOWHERE) {
            char_from_room(ch);
        }
        char_to_room(ch, 4);
        if (ch->desc) {
            close_socket(ch->desc);
        }
        ch->desc = 0;

        if (recep_offer(ch, NULL, &cost, TRUE)) {
            /*
             * if above fails we lose their EQ! 
             */
            cost.total_cost = 100;
            save_obj(ch, &cost, 1);
        } else {
            sprintf(buf, "%s had a failed recp_offer, they are losing EQ!",
                    GET_NAME(ch));
            Log(buf);
            slog(buf);
        }

        extract_char(ch);
    }
}

void ObjFromCorpse(struct obj_data *c)
{
    struct obj_data *jj,
                   *next_thing;

    for (jj = c->contains; jj; jj = next_thing) {
        /* 
         * Next in inventory 
         */
        next_thing = jj->next_content;
        if (jj->in_obj) {
            obj_from_obj(jj);
            if (c->in_obj) {
                obj_to_obj(jj, c->in_obj);
            } else if (c->carried_by) {
                obj_to_room(jj, c->carried_by->in_room);
                check_falling_obj(jj, c->carried_by->in_room);
            } else if (c->in_room != NOWHERE) {
                obj_to_room(jj, c->in_room);
                check_falling_obj(jj, c->in_room);
            } else {
                assert(FALSE);
            }
        } else {
            /*
             * hmm..  it isn't in the object it says it is in.
             * don't extract it.
             */
            c->contains = 0;
            Log("Memory lost in ObjFromCorpse.");
        }
    }
    extract_obj(c);
}

void ClassSpecificStuff(struct char_data *ch)
{
    if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_MONK) ||
        HasClass(ch, CLASS_BARBARIAN) || HasClass(ch, CLASS_PALADIN) ||
        HasClass(ch, CLASS_RANGER)) {
        /* reset all attacks to 1.0 */
        ch->mult_att = 1.0;

        /* apply modifiers for warrior-type classes */
        if (HasClass(ch, CLASS_BARBARIAN)) {
            ch->mult_att +=
                (MIN(30, (GET_LEVEL(ch, BARBARIAN_LEVEL_IND))) * .05);
        } else if (HasClass(ch, CLASS_RANGER)) {
            ch->mult_att +=
                (MIN(30, (GET_LEVEL(ch, RANGER_LEVEL_IND))) * .05);
        } else if (HasClass(ch, CLASS_PALADIN)) {
            ch->mult_att +=
                (MIN(30, (GET_LEVEL(ch, PALADIN_LEVEL_IND))) * .05);
            if (GET_ALIGNMENT(ch) >= 350) {
                SET_BIT(ch->specials.affected_by, AFF_DETECT_EVIL);
                SET_BIT(ch->specials.affected2_by, AFF2_PROTECT_FROM_EVIL);
            }
        } else if (HasClass(ch, CLASS_WARRIOR)) {
            ch->mult_att +=
                (MIN(30, (GET_LEVEL(ch, WARRIOR_LEVEL_IND))) * .05);
        } else if (HasClass(ch, CLASS_THIEF)) {
            /* 
             * give thief classes (without warrior) 
             * a little more attacks: 1.6 
             */
            ch->mult_att +=
                (MIN(30, (GET_LEVEL(ch, THIEF_LEVEL_IND))) * .02);
        } else if (HasClass(ch, CLASS_MONK)) {
            ch->mult_att += (GET_LEVEL(ch, MONK_LEVEL_IND) / 16.0);

            /*
             * fix up damage stuff 
             */
            switch (GET_LEVEL(ch, MONK_LEVEL_IND)) {
            case 1:
            case 2:
            case 3:
                ch->specials.damnodice = 1;
                ch->specials.damsizedice = 3;
                break;
            case 4:
            case 5:
                ch->specials.damnodice = 1;
                ch->specials.damsizedice = 4;
                break;
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                ch->specials.damnodice = 1;
                ch->specials.damsizedice = 6;
                break;
            case 12:
            case 13:
            case 14:
                ch->specials.damnodice = 2;
                ch->specials.damsizedice = 3;
                break;
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                ch->specials.damnodice = 2;
                ch->specials.damsizedice = 4;
                break;
            case 20:
            case 21:
                ch->specials.damnodice = 3;
                ch->specials.damsizedice = 3;
                break;
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
                ch->specials.damnodice = 3;
                ch->specials.damsizedice = 4;
                break;
            case 27:
            case 28:
            case 29:
                ch->specials.damnodice = 4;
                ch->specials.damsizedice = 3;
                break;
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
                ch->specials.damnodice = 4;
                ch->specials.damsizedice = 4;
                break;
            case 35:
            case 36:
                ch->specials.damnodice = 4;
                ch->specials.damsizedice = 5;
                break;
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
                ch->specials.damnodice = 5;
                ch->specials.damsizedice = 4;
                break;
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
                ch->specials.damnodice = 6;
                ch->specials.damsizedice = 5;
                break;
            case 50:
                ch->specials.damnodice = 8;
                ch->specials.damsizedice = 4;
                break;
            default:
                ch->specials.damnodice = 1;
                ch->specials.damsizedice = 2;
                break;
            }
        }
    }

    /*
     * other stuff.. immunities, etc, are set here 
     */
    if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1 && 
        GET_LEVEL(ch, BARBARIAN_LEVEL_IND) > 19) {
        SET_BIT(ch->M_immune, IMM_POISON);
    } else if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
        if (GET_LEVEL(ch, PSI_LEVEL_IND) > 16) {
            SET_BIT(ch->M_immune, IMM_CHARM);
        }
        if (GET_LEVEL(ch, PSI_LEVEL_IND) > 27) {
            SET_BIT(ch->M_immune, IMM_SLEEP);
        }
    }

    if (HasClass(ch, CLASS_MONK)) {
        if (GET_LEVEL(ch, MONK_LEVEL_IND) > 10) {
            SET_BIT(ch->M_immune, IMM_HOLD);
        }
        if (GET_LEVEL(ch, MONK_LEVEL_IND) > 18) {
            SET_BIT(ch->immune, IMM_CHARM);
        }
        if (GET_LEVEL(ch, MONK_LEVEL_IND) > 22) {
            SET_BIT(ch->M_immune, IMM_POISON);
        }
        if (GET_LEVEL(ch, MONK_LEVEL_IND) > 36) {
            SET_BIT(ch->M_immune, IMM_CHARM);
        }
    }

    if (HasClass(ch, CLASS_DRUID)) {
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 14) {
            SET_BIT(ch->immune, IMM_CHARM);
        }
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 32) {
            SET_BIT(ch->M_immune, IMM_POISON);
        }
    }

    if (HasClass(ch, CLASS_NECROMANCER)) {
        if (GET_ALIGNMENT(ch) < 350) {
            SET_BIT(ch->specials.affected_by, AFF_DETECT_GOOD);
        }
        if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND) >= 15) {
            SET_BIT(ch->immune, IMM_DRAIN);
        }
        if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND) >= 23) {
            SET_BIT(ch->immune, IMM_POISON);
        }
        if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND) >= 35) {
            SET_BIT(ch->M_immune, IMM_POISON);
        }
        if (GET_LEVEL(ch, NECROMANCER_LEVEL_IND) >= 41) {
            SET_BIT(ch->M_immune, IMM_DRAIN);
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
