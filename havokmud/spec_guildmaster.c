#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"

/*
 * external vars 
 */

extern struct skillset warriorskills[];
extern struct skillset thiefskills[];
extern struct skillset barbskills[];
extern struct skillset monkskills[];
extern struct skillset mageskills[];
extern struct skillset sorcskills[];
extern struct skillset clericskills[];
extern struct skillset druidskills[];
extern struct skillset paladinskills[];
extern struct skillset rangerskills[];
extern struct skillset psiskills[];
extern struct skillset warninjaskills[];
extern struct skillset thfninjaskills[];
extern struct skillset allninjaskills[];
extern struct skillset warmonkskills[];
extern struct skillset archerskills[];
extern struct skillset loreskills[];
extern struct skillset mainwarriorskills[];
extern struct skillset mainthiefskills[];
extern struct skillset mainbarbskills[];
extern struct skillset mainnecroskills[];
extern struct skillset mainmonkskills[];
extern struct skillset mainmageskills[];
extern struct skillset mainsorcskills[];
extern struct skillset mainclericskills[];
extern struct skillset maindruidskills[];
extern struct skillset mainpaladinskills[];
extern struct skillset mainrangerskills[];
extern struct skillset mainpsiskills[];
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct int_app_type int_app[26];

void            page_string(struct descriptor_data *d, char *str,
                            int keep_internal);

void PrintSkills(struct char_data *ch, int level, struct skillset *skills,
                 char *buffer);
int LearnSkill(struct char_data *ch, struct skillset *skills, char *arg,
               int level, char *teacher, int charge);

#define SPELL_SPECIAL_COST 100000       /* 100k to specialize per spell */

/*
 * extern procedures 
 */



int sailor(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    char            buf[256];
    static char    *n_skills[] = {
        "swim",
        "\n",
    };
    int             percent = 0,
                    number = 0;
    int             charge,
                    sk_num;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
        return (FALSE);
    }

    if (!ch->skills) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    for (; *arg == ' '; arg++) {
        /* 
         * ditch spaces 
         */
    }

    if ((cmd == 164) || (cmd == 170)) {
        if (!arg || (strlen(arg) == 0)) {
            sprintf(buf, " swim   :  %s\n\r",
                    how_good(ch->skills[SKILL_SWIM].learned));
            send_to_char(buf, ch);
            return (TRUE);
        } else {
            number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
            send_to_char("The sailor says ", ch);
            if (number == -1) {
                send_to_char("'I do not know of this skill.'\n\r", ch);
                return (TRUE);
            }
            charge = GetMaxLevel(ch) * 100;
            switch (number) {
            case 0:
            case 1:
                sk_num = SKILL_SWIM;
                break;
            default:
                sprintf(buf, "Strangeness in sailor (%d)", number);
                log(buf);
                send_to_char("'Ack!  I feel faint!'\n\r", ch);
                return;
                break;
            }
        }

        if (GET_GOLD(ch) < charge) {
            send_to_char("'Ah, but you do not have enough money to pay.'\n\r",
                         ch);
            return (TRUE);
        }

        if (ch->skills[sk_num].learned > 60) {
            send_to_char("You must learn from practice and experience now.\n\r",
                         ch);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn <= 0) {
            send_to_char("'You must first earn more practices you already "
                         "have.\n\r", ch);
            return (TRUE);
        }

        GET_GOLD(ch) -= charge;
        send_to_char("'We will now begin.'\n\r", ch);
        ch->specials.spells_to_learn--;

        percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;
        ch->skills[sk_num].learned = MIN(95, percent);

        if (ch->skills[sk_num].learned >= 95) {
            send_to_char("'You are now a master of this art.'\n\r", ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

int loremaster(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH],
                    skillname[254];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0,
                    charge = 0,
                    skillnum = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise,
     */
    if (cmd == 164 || cmd == 170 || cmd == 582) {
        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these lores:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, -1, loreskills, buffer);
            page_string(ch->desc, buffer, 1);
        } else if( !LearnSkill(ch, loreskills, arg, GetMaxLevel(ch),
                               "The loremaster", GetMaxLevel(ch) * 100) ) {
            send_to_char("$c0013[$c0015The loremaster$c0013] tells you '"
                         "I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int hunter(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    char            buf[256];
    static char    *n_skills[] = {
        "hunt",
        "find traps",
        "remove traps",
        "value item",
        "find food",            /* 5 */
        "find water",           /* 6 */
        "\n",
    };
    int             percent = 0,
                    number = 0;
    int             charge,
                    sk_num;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
        return (FALSE);
    }

    if (!ch->skills) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    for (; *arg == ' '; arg++) {
        /* 
         * ditch spaces 
         */
    }

    if ((cmd == 164) || (cmd == 170)) {
        if (!arg || (strlen(arg) == 0)) {
            sprintf(buf, " hunt           :  %s\n\r",
                    how_good(ch->skills[SKILL_HUNT].learned));
            send_to_char(buf, ch);
            sprintf(buf, " find traps     :  %s\n\r",
                    how_good(ch->skills[SKILL_FIND_TRAP].learned));
            send_to_char(buf, ch);
            sprintf(buf, " remove traps   :  %s\n\r",
                    how_good(ch->skills[SKILL_REMOVE_TRAP].learned));
            send_to_char(buf, ch);
            sprintf(buf, " value item     :  %s\n\r",
                    how_good(ch->skills[SKILL_EVALUATE].learned));
            send_to_char(buf, ch);
            sprintf(buf, " find food      :  %s\n\r",
                    how_good(ch->skills[SKILL_FIND_FOOD].learned));
            send_to_char(buf, ch);
            sprintf(buf, " find water     :  %s\n\r",
                    how_good(ch->skills[SKILL_FIND_WATER].learned));
            send_to_char(buf, ch);

            return (TRUE);
        } else {
            number = old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
            send_to_char("The hunter says ", ch);
            if (number == -1) {
                send_to_char("'I do not know of this skill.'\n\r", ch);
                return (TRUE);
            }

            charge = GetMaxLevel(ch) * 100;
            switch (number) {
            case 0:
            case 1:
                sk_num = SKILL_HUNT;
                break;
            case 2:
                sk_num = SKILL_FIND_TRAP;
                break;
            case 3:
                sk_num = SKILL_REMOVE_TRAP;
                break;
            case 4:
                sk_num = SKILL_EVALUATE;
                break;
            case 5:
                sk_num = SKILL_FIND_FOOD;
                break;
            case 6:
                sk_num = SKILL_FIND_WATER;
                break;
            default:
                sprintf(buf, "Strangeness in hunter (%d)", number);
                log(buf);
                send_to_char("'Ack!  I feel faint!'\n\r", ch);
                return (FALSE);
            }
        }

        if (!HasClass(ch, CLASS_THIEF) &&
            (sk_num == SKILL_FIND_TRAP || sk_num == SKILL_REMOVE_TRAP || 
             sk_num == SKILL_EVALUATE)) {
            send_to_char("'You're not the sort I'll teach trap or value "
                         "skills to.'\n\r", ch);
            return (TRUE);
        }

        if ((sk_num == SKILL_HUNT) && (!HasClass(ch, CLASS_THIEF) ||
                                       !HasClass(ch, CLASS_BARBARIAN))) {
            send_to_char("'You're not the sort I'll teach track to.'\n\r", ch);
            return (TRUE);
        }

        if (GET_GOLD(ch) < charge) {
            send_to_char("'Ah, but you do not have enough money to pay.'\n\r",
                         ch);
            return (TRUE);
        }

        if (ch->skills[sk_num].learned >= 95) {
            send_to_char("'You are a master of this art, I can teach you no "
                         "more.'\n\r", ch);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn <= 0) {
            send_to_char("'You must first earn more practices you already "
                         "have.\n\r", ch);
            return (TRUE);
        }

        GET_GOLD(ch) -= charge;
        send_to_char("'We will now begin.'\n\r", ch);
        ch->specials.spells_to_learn--;

        percent = ch->skills[sk_num].learned + int_app[GET_INT(ch)].learn;

        if (!IS_SET(ch->skills[sk_num].flags, SKILL_KNOWN)) {
            SET_BIT(ch->skills[sk_num].flags, SKILL_KNOWN);
        }

        ch->skills[sk_num].learned = MIN(95, percent);

        if (ch->skills[sk_num].learned >= 95) {
            send_to_char("'You are now a master of this art.'\n\r", ch);
            return (TRUE);
        }
    }

    return (FALSE);
}

int archer_instructor(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0,
                    charge = 0;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170) {
        if (!HasClass(ch, CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_RANGER |
                          CLASS_PALADIN)) {
            send_to_char("$c0013[$c0015The archer instructor$c0013] tells you"
                         " 'I'm not teaching the likes of you, get lost!'\n\r",
                         ch);
            return (TRUE);
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice this skill:\n\r\n\r");
            strcat(buffer, buf);

            PrintSkills(ch, 50, archerskills, buffer);
            page_string(ch->desc, buffer, 1);
        } else if( !LearnSkill(ch, archerskills, arg, GetMaxLevel(ch), 
                              "The archer instructor", 
                              GetMaxLevel(ch) * 100) ) {
            send_to_char("$c0013[$c0015The archer instructor$c0013] tells you"
                         " 'I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int monk_master(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 243->gain
     */
    if (cmd == 243) {
        if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                         " 'You're not a monk.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room, monk_master);
        } else {
            guildmaster = mob;
        }

        if (GET_LEVEL(ch, MONK_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                         " 'You must learn from another, I can no longer train"
                         " you.'\n\r", ch);
            return (TRUE);
        }

        if( GET_EXP(ch) < 
            titles[MONK_LEVEL_IND][GET_LEVEL(ch, MONK_LEVEL_IND) + 1].exp) {
            send_to_char("You're not ready to gain yet!\n\r", ch);
            return (FALSE);
        }

        if (GET_LEVEL(ch, MONK_LEVEL_IND) <= 9) {
            GainLevel(ch, MONK_LEVEL_IND);
        } else {
            send_to_char("You must fight another monk for this title.\n\r", ch);
        }
        return (TRUE);
    } else if (cmd == 164 || cmd == 170 || cmd == 582) {
        if (HasClass(ch, CLASS_MONK)) {
            if (!*arg) {
                sprintf(buffer, "You have got %d practice sessions "
                                "left.\n\r\n\r", ch->specials.spells_to_learn);
                sprintf(buf, "You can practice any of these skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, MONK_LEVEL_IND), monkskills,
                            buffer);

                if (ch->specials.remortclass == MONK_LEVEL_IND + 1) {
                    sprintf(buf, "\n\rSince you picked monk as your main "
                                 "class, you get these bonus skills:\n\r\n\r");
                    strcat(buffer, buf);
                    PrintSkills(ch, GET_LEVEL(ch, MONK_LEVEL_IND), 
                                mainmonkskills, buffer);
                }
                page_string(ch->desc, buffer, 1);
            } else {
                if( LearnSkill(ch, monkskills, arg, 
                               GET_LEVEL(ch, MONK_LEVEL_IND), 
                               "The Monk Guildmaster", 0) ) {
                    return( TRUE );
                }

                if (ch->specials.remortclass == MONK_LEVEL_IND + 1 &&
                    !LearnSkill(ch, mainmonkskills, arg, 
                                GET_LEVEL(ch, MONK_LEVEL_IND),
                                "The Monk Guildmaster", 0) ) {
                    send_to_char("$c0013[$c0015The Monk Guildmaster$c0013] "
                                 "tells you 'I do not know of that skill!'\n\r",
                                 ch);
                }
            }
            return (TRUE);
        } else if (HasClass(ch, CLASS_WARRIOR)) {
            if (!*arg) {
                sprintf(buffer, "You have got %d practice sessions "
                                "left.\n\r\n\r", ch->specials.spells_to_learn);
                sprintf(buf, "You can practice these skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, WARRIOR_LEVEL_IND), 
                            warmonkskills, buffer);
                page_string(ch->desc, buffer, 1);
            } else {
                if( LearnSkill(ch, warmonkskills, arg, 
                               GET_LEVEL(ch, WARRIOR_LEVEL_IND), 
                               "The Monk Guildmaster", 0) ) {
                    return (TRUE);
                }
                send_to_char("$c0013[$c0015The Monk Guildmaster$c0013] "
                             "tells you 'I do not know of that skill!'\n\r",
                             ch);
            }
            return (TRUE);
        } else {
            send_to_char("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
                         "I don't teach the likes of you!'\n\r", ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

int DruidGuildMaster(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_DRUID)) {
            send_to_char("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                         " 'You're not a druid.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                                    DruidGuildMaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                         " 'You must learn from another, I can no longer train"
                         " you.'\n\r", ch);
            return (TRUE);
        }

        if (cmd == 243) {
            if (GET_EXP(ch) <
                titles[DRUID_LEVEL_IND]
                      [GET_LEVEL(ch, DRUID_LEVEL_IND) + 1].exp) {
                send_to_char("You're not ready to gain yet!\n\r", ch);
                return (FALSE);
            } else if (GET_LEVEL(ch, DRUID_LEVEL_IND) <= 9) {
                GainLevel(ch, DRUID_LEVEL_IND);
            } else {
                send_to_char("You must fight another druid for this title.\n\r",
                             ch);
            }
            return (TRUE);
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, GET_LEVEL(ch, DRUID_LEVEL_IND), druidskills, 
                        buffer);

            if (ch->specials.remortclass == DRUID_LEVEL_IND + 1) {
                sprintf(buf, "\n\rSince you picked druid as your main class, "
                             "you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, DRUID_LEVEL_IND), maindruidskills,
                            buffer);
            }
            page_string(ch->desc, buffer, 1);
        } else {
            if( LearnSkill(ch, monkskills, arg, GET_LEVEL(ch, DRUID_LEVEL_IND), 
                           "The Druid Guildmaster", 0) ) {
                return( TRUE );
            }
            if (ch->specials.remortclass == DRUID_LEVEL_IND + 1 &&
                LearnSkill(ch, mainmonkskills, arg, 
                           GET_LEVEL(ch, DRUID_LEVEL_IND), 
                          "The Druid Guildmaster", 0) ) {
                return( TRUE );
            }

            send_to_char("$c0013[$c0015The Druid Guildmaster$c0013] tells you '"
                         "I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}


int barbarian_guildmaster(struct char_data *ch, int cmd, char *arg,
                          struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_BARBARIAN)) {
            send_to_char("$c0013[$c0015The Barbarian Guildmaster$c0013] tells"
                         " you 'You're not a barbarian.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                                    barbarian_guildmaster);
        } else {
            guildmaster = mob;
        }

        if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Barbarian Guildmaster$c0013] tells "
                         "you 'You must learn from another, I can no longer "
                         "train you.'\n\r", ch);
            return (TRUE);
        }

        /* 
         * gain
         */
        if (cmd == 243) {
            if ( GET_EXP(ch) <
                 titles[BARBARIAN_LEVEL_IND]
                       [GET_LEVEL(ch, BARBARIAN_LEVEL_IND) + 1].exp) {
                send_to_char("Your not ready to gain yet!", ch);
                return (FALSE);
            } else {
                GainLevel(ch, BARBARIAN_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, GET_LEVEL(ch, BARBARIAN_LEVEL_IND), barbskills,
                        buffer);
            if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
                sprintf(buf, "\n\rSince you picked barbarian as your main "
                             "class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, BARBARIAN_LEVEL_IND), 
                            mainbarbskills, buffer);
            }
            page_string(ch->desc, buffer, 1);
        } else {
            if( LearnSkill(ch, barbskills, arg, 
                           GET_LEVEL(ch, BARBARIAN_LEVEL_IND), 
                           "The Barbarian Guildmaster", 0) ) {
                return( TRUE );
            }

            if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1 &&
                LearnSkill(ch, mainbarbskills, arg, 
                           GET_LEVEL(ch, BARBARIAN_LEVEL_IND), 
                           "The Barbarian Guildmaster", 0) ) {
                return( TRUE );
            }

            send_to_char ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells"
                          " you 'I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int RangerGuildmaster(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_RANGER)) {
            send_to_char("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                         " 'You're not a ranger.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room, 
                                                    RangerGuildmaster);
        } else {
            guildmaster = mob;
        }

        if (GET_LEVEL(ch, RANGER_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                         " 'You must learn from another, I can no longer train"
                         " you.'\n\r", ch);
            return (TRUE);
        }
        
        /* 
         * gain
         */
        if (cmd == 243) {
            if (GET_EXP(ch) <
                titles[RANGER_LEVEL_IND]
                      [GET_LEVEL(ch, RANGER_LEVEL_IND) + 1].exp) {
                send_to_char("Your not ready to gain yet!", ch);
                return (FALSE);
            } else {
                GainLevel(ch, RANGER_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells and "
                         "skills:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, GET_LEVEL(ch, RANGER_LEVEL_IND), rangerskills,
                        buffer);

            if (ch->specials.remortclass == RANGER_LEVEL_IND + 1) {
                sprintf(buf, "\n\rSince you picked ranger as your main class, "
                             "you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, RANGER_LEVEL_IND),
                            mainrangerskills, buffer);
            }
            page_string(ch->desc, buffer, 1);
        } else {
            if( LearnSkill(ch, rangerskills, arg, 
                           GET_LEVEL(ch, RANGER_LEVEL_IND), 
                           "The Ranger Guildmaster", 0) ) {
                return( TRUE );
            }

            if (ch->specials.remortclass == RANGER_LEVEL_IND + 1 &&
                LearnSkill(ch, mainrangerskills, arg, 
                           GET_LEVEL(ch, RANGER_LEVEL_IND), 
                           "The Ranger Guildmaster", 0) ) {
                return( TRUE );
            }

            send_to_char("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                         " 'I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int PsiGuildmaster(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_PSI)) {
            send_to_char("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                         " 'You're not a psi.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                                    PsiGuildmaster);
        } else {
            guildmaster = mob;
        }

        if (GET_LEVEL(ch, PSI_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                         " 'You must learn from another, I can no longer train"
                         " you.'\n\r", ch);
            return (TRUE);
        }

        /* 
         * gain
         */
        if (cmd == 243) {
            if (GET_EXP(ch) <
                titles[PSI_LEVEL_IND][GET_LEVEL(ch, PSI_LEVEL_IND) + 1].exp) {
                send_to_char("Your not ready to gain yet!", ch);
                return (FALSE);
            } else {
                GainLevel(ch, PSI_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, GET_LEVEL(ch, PSI_LEVEL_IND), psiskills, buffer);

            if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
                sprintf(buf, "\n\rSince you picked psi as your main class, you"
                             " get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, PSI_LEVEL_IND), mainpsiskills,
                            buffer);
            }
            page_string(ch->desc, buffer, 1);
        } else {
            if( LearnSkill(ch, psiskills, arg, 
                           GET_LEVEL(ch, PSI_LEVEL_IND), 
                           "The Psi Guildmaster", 0) ) {
                return( TRUE );
            }

            if (ch->specials.remortclass == PSI_LEVEL_IND + 1 &&
                LearnSkill(ch, psiskills, arg, 
                           GET_LEVEL(ch, PSI_LEVEL_IND), 
                           "The Psi Guildmaster", 0) ) {
                return( TRUE );
            }

            send_to_char("$c0013[$c0015The Psi Guildmaster$c0013] tells you '"
                         "I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int PaladinGuildmaster(struct char_data *ch, int cmd, char *arg,
                       struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise, 243->gain
     */
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_PALADIN)) {
            send_to_char("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                         " 'You're not a paladin.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                                    PaladinGuildmaster);
        } else {
            guildmaster = mob;
        }

        if (GET_LEVEL(ch, PALADIN_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                         " 'You must learn from another, I can no longer train"
                         " you.'\n\r", ch);
            return (TRUE);
        }

        /* 
         * gain
         */
        if (cmd == 243) {
            if (GET_EXP(ch) <
                titles[PALADIN_LEVEL_IND]
                      [GET_LEVEL(ch, PALADIN_LEVEL_IND) + 1].exp) {
                send_to_char("Your not ready to gain yet!", ch);
                return (FALSE);
            } else {
                GainLevel(ch, PALADIN_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells and "
                         "skills:\n\r\n\r");
            strcat(buffer, buf);
            PrintSkills(ch, GET_LEVEL(ch, PALADIN_LEVEL_IND), paladinskills,
                        buffer);
            if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
                sprintf(buf, "\n\rSince you picked paladin as your main class,"
                             " you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                PrintSkills(ch, GET_LEVEL(ch, PALADIN_LEVEL_IND),
                            mainpaladinskills, buffer);
            }
            page_string(ch->desc, buffer, 1);
        } else {
            if( LearnSkill(ch, paladinskills, arg, 
                           GET_LEVEL(ch, PALADIN_LEVEL_IND), 
                           "The Paladin Guildmaster", 0) ) {
                return( TRUE );
            }
            if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1 &&
                LearnSkill(ch, paladinskills, arg, 
                           GET_LEVEL(ch, PALADIN_LEVEL_IND), 
                           "The Paladin Guildmaster", 0) ) {
                return( TRUE );
            }
            send_to_char("$c0013[$c0015The Paladin Guildmaster$c0013] tells "
                         "you 'I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int mage_specialist_guildmaster(struct char_data *ch, int cmd, char *arg,
                                struct char_data *mob, int type)
{
    char            buf[256];
    int             i,
                    percent = 0,
                    number = 0;
    struct char_data *guildmaster;
    extern char    *spells[];
    extern struct spell_info_type spell_info[MAX_SPL_LIST];

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            return (magic_user(ch, cmd, arg, ch, 0));
        }
        return (FALSE);
    }

    if (!ch->skills) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                            mage_specialist_guildmaster);

    if (!guildmaster) {
        return (FALSE);
    }
    for (; *arg == ' '; arg++) {
        /* 
         * ditch spaces 
         */
    }

    if ((cmd == 164) || (cmd == 170) || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_MAGIC_USER)) {
            do_say(guildmaster, "I do not teach heathens!", 0);
            return (TRUE);
        }

        if (HasFightingClass(ch) == TRUE) {
            do_say(guildmaster, "Your warrior instincts are rather scary.. "
                                "Begone...", 0);
            return (FALSE);
        }

        if (IS_NPC(ch)) {
            do_say(guildmaster, "What do I look like? An animal trainer?", 0);
            return (FALSE);
        }

        if (!*arg) {
            sprintf(buf, "You have got %d practice sessions left.\n\r",
                    ch->specials.spells_to_learn);
            send_to_char(buf, ch);
            send_to_char("You can practise any of these spells:\n\r", ch);

            for (i = 0; *spells[i] != '\n'; i++) {
                if (spell_info[i + 1].spell_pointer && 
                    spell_info[i + 1].min_level_magic <= 
                    GET_LEVEL(ch, MAGE_LEVEL_IND) &&
                    spell_info[i + 1].min_level_magic <=
                     GetMaxLevel(guildmaster) - 10) {

                    sprintf(buf, "[%d] %-15s %s \n\r",
                            spell_info[i + 1].min_level_magic,
                            spells[i], how_good(ch->skills[i + 1].learned));
                    send_to_char(buf, ch);
                }
            }
            return (TRUE);
        }

        for (; isspace(*arg); arg++) {
            /* 
             * Empty loop 
             */
        }

        number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
        if (number == -1) {
            do_say(guildmaster, "You do not know of this spell.", 0);
            return (TRUE);
        }

        if (GET_LEVEL(ch, MAGE_LEVEL_IND) <
            spell_info[number].min_level_magic) {
            do_say(guildmaster, "You do not know of this spell.", 0);
            return (TRUE);
        }

        if (GetMaxLevel(guildmaster) - 10 < 
            spell_info[number].min_level_magic) {
            do_say(guildmaster, "I don't know of this spell.", 0);
            return (TRUE);
        }

        if (GET_GOLD(ch) < SPELL_SPECIAL_COST) {
            do_say(guildmaster, "Ah, but you do not have enough money to pay.",
                   0);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn < 4) {
            do_say(guildmaster, "You must first use the knowledge you already "
                                "have.", 0);
            return (TRUE);
        }

        if (IsSpecialized(ch->skills[number].special)) {
            do_say(guildmaster, "You are already proficient in this spell!", 0);
            return (TRUE);
        }

        if (HowManySpecials(ch) > MAX_SPECIALS(ch)) {
            do_say(guildmaster, "You are already specialized in several "
                                "skills.", 0);
            return (TRUE);
        }

        if (ch->skills[number].learned < 95) {
            do_say(guildmaster, "You must fully learn this spell first.", 0);
            return (TRUE);
        }

        do_say(guildmaster, "Here is how you do that...", 0);
        ch->specials.spells_to_learn -= 4;
        GET_GOLD(ch) -= SPELL_SPECIAL_COST;
        SET_BIT(ch->skills[number].special, SKILL_SPECIALIZED);
        return (TRUE);
    }
    return (magic_user(ch, cmd, arg, ch, 0));
}

int cleric_specialist_guildmaster(struct char_data *ch, int cmd, char *arg,
                                  struct char_data *mob, int type)
{
    char            buf[256];
    int             i,
                    percent = 0,
                    number = 0;
    struct char_data *guildmaster;
    extern char    *spells[];
    extern struct spell_info_type spell_info[MAX_SPL_LIST];

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            return (cleric(ch, cmd, arg, ch, 0));
        }
        return (FALSE);
    }

    if (!ch->skills) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    guildmaster = FindMobInRoomWithFunction(ch->in_room,
                                            cleric_specialist_guildmaster);

    if (!guildmaster) {
        return (FALSE);
    }
    for (; *arg == ' '; arg++) {
        /* 
         * ditch spaces 
         */
    }

    if ((cmd == 164) || (cmd == 170) || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_CLERIC)) {
            do_say(guildmaster, "I do not teach heathens!", 0);
            return (TRUE);
        }

        if (HasFightingClass(ch) == TRUE) {
            do_say(guildmaster, "Your warrior instincts are rather scary.. "
                                "Begone...", 0);
            return (FALSE);
        }

        if (IS_NPC(ch)) {
            do_say(guildmaster, "What do I look like? An animal trainer?", 0);
            return (FALSE);
        }

        if (!*arg) {
            sprintf(buf, "You have got %d practice sessions left.\n\r",
                    ch->specials.spells_to_learn);
            send_to_char(buf, ch);
            send_to_char("You can practise any of these spells:\n\r", ch);

            for (i = 0; *spells[i] != '\n'; i++) {
                if (spell_info[i + 1].spell_pointer &&
                    spell_info[i + 1].min_level_cleric <=
                    GET_LEVEL(ch, CLERIC_LEVEL_IND) &&
                    spell_info[i + 1].min_level_cleric <=
                     GetMaxLevel(guildmaster) - 10) {
                    sprintf(buf, "[%d] %-15s %s \n\r",
                            spell_info[i + 1].min_level_cleric,
                            spells[i], how_good(ch->skills[i + 1].learned));
                    send_to_char(buf, ch);
                }
            }

            return (TRUE);
        }

        for (; isspace(*arg); arg++) {
            /* 
             * Empty loop 
             */
        }

        number = old_search_block(arg, 0, strlen(arg), spells, FALSE);
        if (number == -1) {
            do_say(guildmaster, "You do not know of this spell.", 0);
            return (TRUE);
        }

        if (GET_LEVEL(ch, CLERIC_LEVEL_IND) <
            spell_info[number].min_level_cleric) {
            do_say(guildmaster, "You do not know of this spell.", 0);
            return (TRUE);
        }

        if (GetMaxLevel(guildmaster) - 10 <
            spell_info[number].min_level_cleric) {
            do_say(guildmaster, "I don't know of this spell.", 0);
            return (TRUE);
        }

        if (GET_GOLD(ch) < SPELL_SPECIAL_COST) {
            do_say(guildmaster, "Ah, but you do not have enough money to pay.",
                   0);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn < 4) {
            do_say(guildmaster, "You do not seem to be able to practice at the"
                                " moment.", 0);
            return (TRUE);
        }

        if (IsSpecialized(ch->skills[number].special)) {
            do_say(guildmaster, "You are already proficient in this spell!", 0);
            return (TRUE);
        }

        if (HowManySpecials(ch) > MAX_SPECIALS(ch)) {
            do_say(guildmaster, "You are already specialized in several "
                                "skills.", 0);
            return (TRUE);
        }

        if (ch->skills[number].learned < 95) {
            do_say(guildmaster, "You must fully learn this spell first.", 0);
            return (TRUE);
        }

        do_say(guildmaster, "Here is how you do that...", 0);
        ch->specials.spells_to_learn -= 4;
        GET_GOLD(ch) -= SPELL_SPECIAL_COST;
        SET_BIT(ch->skills[number].special, SKILL_SPECIALIZED);
        return (TRUE);
    }
    return (cleric(ch, cmd, arg, ch, 0));
}


int ninja_master(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH],
                    skillname[254];
    static int      percent = 0;
    static int      x = 0;
    int             i = 0,
                    charge = 0,
                    skillnum = 0;
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /* 
     * 170->Practice,164->Practise,
     */
    if (cmd == 164 || cmd == 170 || cmd == 582) {
        if (!*arg) {
            sprintf(buffer, "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these skills:\n\r\n\r");
            strcat(buffer, buf);

            if (HasClass(ch, CLASS_WARRIOR)) {
                PrintSkills(ch, -1, warninjaskills, buffer);
            }

            if (HasClass(ch, CLASS_THIEF)) {
                PrintSkills(ch, -1, thfninjaskills, buffer);
            }

            PrintSkills(ch, -1, allninjaskills, buffer);
            page_string(ch->desc, buffer, 1);
        } else {
            if (HasClass(ch, CLASS_WARRIOR) &&
                LearnSkill(ch, warninjaskills, arg, GetMaxLevel(ch),
                           "The Ninja Master", 0) ) {
                return( TRUE );
            }

            if (HasClass(ch, CLASS_THIEF) &&
                LearnSkill(ch, thfninjaskills, arg, GetMaxLevel(ch),
                           "The Ninja Master", 0) ) {
                return( TRUE );
            }

            if (LearnSkill(ch, allninjaskills, arg, GetMaxLevel(ch),
                           "The Ninja Master", 0) ) {
                return( TRUE );
            }

            send_to_char("$c0013[$c0015The Ninja Master$c0013] tells you '"
                         "I do not know of that skill!'\n\r", ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

void PrintSkills(struct char_data *ch, int level, struct skillset *skills,
                 char *buffer)
{
    int             i;
    char            buf[256];
                    

    if( !ch || !skills ) {
        return;
    }

    /*
     * list by level, so new skills show at top of list 
     */
    i = 0;

    while (level != 0) {
        while (skills[i].level != -1) {
            if (level == -1 || skills[i].level == level) {
                sprintf(buf, "[%-2d] %-30s %-15s",
                        skills[i].level, skills[i].name,
                        how_good(ch->skills[skills[i].skillnum].learned));

                if (IsSpecialized(ch->skills[skills[i].skillnum].special)) {
                    strcat(buf, " (special)");
                }
                strcat(buf, " \n\r");
                if (strlen(buf) + strlen(buffer) > 
                    (MAX_STRING_LENGTH * 2) - 2) {
                    return;
                }
                strcat(buffer, buf);
                strcat(buffer, "\r");
            }
            i++;
        }

        if( level == -1 ) {
            level = 0;
        } else {
            level--;
        }
    }
}


int LearnSkill(struct char_data *ch, struct skillset *skills, char *arg,
               int level, char *teacher, int charge)
{
    int             i;
    int             percent = 0;

    for(i = 0; skills[i].level != -1; i++) {
        if (is_abbrev(arg, skills[i].name)) {
            if (skills[i].level > level) {
                ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'You're not "
                              "experienced enough to learn this skill.'",
                          teacher);
                return (TRUE);
            }

            if (ch->specials.spells_to_learn <= 0) {
                ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'You don't "
                              "have enough practice points.'\n\r", teacher);
                return (TRUE);
            }

            if (charge && GET_GOLD(ch) < charge) {
                ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'Ah, but you "
                              "do not have enough money to pay.'\n\r", teacher);
                return (TRUE);
            }

            if (!charge && 
                ch->skills[skills[i].skillnum].learned > skills[i].maxlearn ) {
                if( skills[i].maxlearn < 94 ) {
                    ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'You must "
                                  "learn from experience and practice to get"
                                  " any better at that skill.'\n\r", teacher);
                } else {
                    ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'You are a"
                                  " master of this art, I can teach you no "
                                  "more.'\n\r", ch);
                }
                return (TRUE);
            }

            if (ch->skills[skills[i].skillnum].learned >= 95) {
                ch_printf(ch, "$c0013[$c0015%s$c0013] tells you 'I have "
                              "taught you all I can about that skill.'\n\r", 
                          teacher);
                return (TRUE);
            }

            ch_printf(ch, "You practice %s for a while.\n\r", skills[i].name);
            ch->specials.spells_to_learn--;
            GET_GOLD(ch) -= charge;

            if (!IS_SET(ch->skills[skills[i].skillnum].flags, SKILL_KNOWN)) {
                SET_BIT(ch->skills[skills[i].skillnum].flags, SKILL_KNOWN);
            }

            percent = ch->skills[skills[i].skillnum].learned +
                      int_app[GET_INT(ch)].learn;

            ch->skills[skills[i].skillnum].learned = MIN(95, percent);
            if (ch->skills[skills[i].skillnum].learned >= 95) {
                send_to_char("'You are now a master of this art.'\n\r", ch);
            }
            return (TRUE);
        }
    }
    return(FALSE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
