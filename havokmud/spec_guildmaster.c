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

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int      top_of_world;
extern struct int_app_type int_app[26];

extern struct title_type titles[4][ABS_MAX_LVL];
extern char    *dirs[];

extern int      gSeason;        /* what season is it ? */

void            page_string(struct descriptor_data *d, char *str,
                            int keep_internal);

#define MONK_CHALLENGE_ROOM 551
#define DRUID_CHALLENGE_ROOM 501
/*
 * extern procedures 
 */

/*
 * Bjs Shit Begin 
 */

#define Bandits_Path   2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10
#define Fountain_Level 20

#define CMD_SAY 17
#define CMD_ASAY 169


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

    if (!AWAKE(ch))
        return (FALSE);

    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
        return (FALSE);
    }

    if (!ch->skills)
        return (FALSE);

    if (check_soundproof(ch))
        return (FALSE);

    for (; *arg == ' '; arg++); /* ditch spaces */

    if ((cmd == 164) || (cmd == 170)) {
        if (!arg || (strlen(arg) == 0)) {
            sprintf(buf, " swim   :  %s\n\r",
                    how_good(ch->skills[SKILL_SWIM].learned));
            send_to_char(buf, ch);
            return (TRUE);
        } else {
            number =
                old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
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
            }
        }

        if (GET_GOLD(ch) < charge) {
            send_to_char
                ("'Ah, but you do not have enough money to pay.'\n\r", ch);
            return (TRUE);
        }

        if (ch->skills[sk_num].learned > 60) {
            send_to_char
                ("You must learn from practice and experience now.\n\r",
                 ch);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn <= 0) {
            send_to_char
                ("'You must first earn more practices you already have.\n\r",
                 ch);
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
    } else {
        return (FALSE);
    }
}

/*
 * case 8: if (!HasClass(ch, CLASS_CLERIC) && !HasClass(ch,
 * CLASS_MAGIC_USER)) { sk_num = SKILL_READ_MAGIC; } else {
 * send_to_char("'You already have this skill!'\n\r",ch); if (ch->skills)
 * if (!ch->skills[SKILL_READ_MAGIC].learned)
 * ch->skills[SKILL_READ_MAGIC].learned = 95; return; } break; 
 */
int loremaster(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH],
                    skillname[254];
    static int      percent = 0;
    static int      x = 0;      // for loop
    int             i = 0,
                    charge = 0,
                    skillnum = 0;       // while loop
    struct char_data *guildmaster;

#if 1
    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise,
    if (cmd == 164 || cmd == 170 || cmd == 582) {
        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these lores:\n\r\n\r");
            strcat(buffer, buf);
            while (loreskills[i].level != -1) {
                sprintf(buf, "[%-2d] %-30s %-15s", loreskills[i].level,
                        loreskills[i].name,
                        how_good(ch->skills[loreskills[i].skillnum].
                                 learned));
                if (IsSpecialized
                    (ch->skills[loreskills[i].skillnum].special))
                    strcat(buf, " (special)");
                strcat(buf, " \n\r");
                if (strlen(buf) + strlen(buffer) >
                    (MAX_STRING_LENGTH * 2) - 2)
                    break;
                strcat(buffer, buf);
                strcat(buffer, "\r");
                i++;
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {                /* includes arg.. */
            x = 0;
            while (loreskills[x].level != -1) {
                if (is_abbrev(arg, loreskills[x].name)) {
                    if (loreskills[x].level > GetMaxLevel(ch)) {
                        send_to_char
                            ("$c0013[$c0015The loremaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (loreskills[x].skillnum == SKILL_READ_MAGIC) {
                        if (HasClass(ch, CLASS_CLERIC)
                            || HasClass(ch, CLASS_MAGIC_USER)
                            || HasClass(ch, CLASS_DRUID)
                            || HasClass(ch, CLASS_SORCERER)
                            || HasClass(ch, CLASS_PSI)
                            || HasClass(ch, CLASS_NECROMANCER)) {
                            send_to_char
                                ("$c0013[$c0015The loremaster$c0013] tells you"
                                 " 'Heeheehee, you wanna pay me for knowledge you already posess?'",
                                 ch);
                            if (ch->skills)
                                if (!ch->skills[SKILL_READ_MAGIC].learned)
                                    ch->skills[SKILL_READ_MAGIC].learned =
                                        95;
                            return (TRUE);
                        }
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The loremaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[loreskills[x].skillnum].learned >= 95) {
                        send_to_char
                            ("$c0013[$c0015The loremaster$c0013] tells you"
                             " 'You're already a master of this art!'\n\r",
                             ch);
                        return (TRUE);
                    }

                    charge = GetMaxLevel(ch) * 100;
                    if (GET_GOLD(ch) < charge) {
                        send_to_char
                            ("$c0013[$c0015The loremaster$c0013] tells you"
                             " 'Ah, but you do not have enough money to pay.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    GET_GOLD(ch) -= charge;
                    sprintf(buf, "You practice %s for a while.\n\r",
                            loreskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn =
                        ch->specials.spells_to_learn - 1;

                    if (!IS_SET
                        (ch->skills[loreskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[loreskills[x].skillnum].flags,
                                SKILL_KNOWN);
                    }
                    percent =
                        ch->skills[loreskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[loreskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[loreskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            send_to_char("$c0013[$c0015The loremaster$c0013] tells you '"
                         "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
    }
#endif
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

    if (!AWAKE(ch))
        return (FALSE);

    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
        return (FALSE);
    }

    if (!ch->skills)
        return (FALSE);

    if (check_soundproof(ch))
        return (FALSE);

    for (; *arg == ' '; arg++); /* ditch spaces */

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

            number =
                old_search_block(arg, 0, strlen(arg), n_skills, FALSE);
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

        if (!HasClass(ch, CLASS_THIEF))
            if (sk_num == SKILL_FIND_TRAP ||
                sk_num == SKILL_REMOVE_TRAP || sk_num == SKILL_EVALUATE) {
                send_to_char
                    ("'You're not the sort I'll teach trap or value skills to.'\n\r",
                     ch);
                return (TRUE);
            }

        if ((sk_num == SKILL_HUNT) && (!HasClass(ch, CLASS_THIEF) ||
                                       !HasClass(ch, CLASS_BARBARIAN))) {
            send_to_char("'You're not the sort I'll teach track to.'\n\r",
                         ch);
            return (TRUE);
        }

        if (GET_GOLD(ch) < charge) {
            send_to_char
                ("'Ah, but you do not have enough money to pay.'\n\r", ch);
            return (TRUE);
        }

        if (ch->skills[sk_num].learned >= 95) {
            send_to_char
                ("'You are a master of this art, I can teach you no more.'\n\r",
                 ch);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn <= 0) {
            send_to_char
                ("'You must first earn more practices you already have.\n\r",
                 ch);
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
    } else {
        return (FALSE);
    }
}

int archer_instructor(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    int             count = 0;
    char            buf[256],
                    buffer[MAX_STRING_LENGTH];
    static int      percent = 0;
    static int      x = 0;      // for loop
    int             i = 0,
                    charge = 0; // while loop

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170) {

        if (!HasClass
            (ch,
             CLASS_WARRIOR | CLASS_BARBARIAN | CLASS_RANGER |
             CLASS_PALADIN)) {
            send_to_char
                ("$c0013[$c0015The archer instructor$c0013] tells you"
                 " 'I'm not teaching the likes of you, get lost!'\n\r",
                 ch);
            return (TRUE);
        }

        if (!*arg && (cmd == 170 || cmd == 164)) {      /* practice||practise, 
                                                         * without
                                                         * argument */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice this skill:\n\r\n\r");
            strcat(buffer, buf);
            x = 50;
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (archerskills[i].level != -1) {
                    if (archerskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                archerskills[i].level,
                                archerskills[i].name,
                                how_good(ch->
                                         skills[archerskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[archerskills[i].skillnum].special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (archerskills[x].level != -1) {
                if (is_abbrev(arg, archerskills[x].name)) {     // !str_cmp(arg,n_skills[x])){
                    if (archerskills[x].level > GetMaxLevel(ch)) {
                        send_to_char
                            ("$c0013[$c0015The archer instructor$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The archer instructor$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }
                    charge = GetMaxLevel(ch) * 100;
                    if (GET_GOLD(ch) < charge) {
                        send_to_char
                            ("$c0013[$c0015The archer instructor$c0013] tells you"
                             " 'Ah, but you do not have enough money to pay.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[archerskills[x].skillnum].learned >= 95) {
                        send_to_char
                            ("$c0013[$c0015The archer instructor$c0013] tells you"
                             " 'I can taught you all I can about that skill.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            archerskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;
                    GET_GOLD(ch) -= charge;

                    if (!IS_SET
                        (ch->skills[archerskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[archerskills[x].skillnum].flags,
                                SKILL_KNOWN);
                    }
                    percent =
                        ch->skills[archerskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[archerskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[archerskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            send_to_char
                ("$c0013[$c0015The archer instructor$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 243->gain
    if (cmd == 243) {

        if (!HasClass(ch, CLASS_MONK)) {
            send_to_char
                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                 " 'You're not a monk.'\n\r", ch);
            return (TRUE);
        }
        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room, monk_master);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, MONK_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[MONK_LEVEL_IND][GET_LEVEL(ch, MONK_LEVEL_IND) +
                                   1].exp) {
            send_to_char("You're not ready to gain yet!\n\r", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                if (GET_LEVEL(ch, MONK_LEVEL_IND) <= 9) {
                    GainLevel(ch, MONK_LEVEL_IND);
                } else {
                    send_to_char
                        ("You must fight another monk for this title.\n\r",
                         ch);
                }
                return (TRUE);
            }
        }
    } else if (cmd == 164 || cmd == 170 || cmd == 582) {        /* prac */
        if (HasClass(ch, CLASS_MONK)) {
            if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {    /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
                sprintf(buffer,
                        "You have got %d practice sessions left.\n\r\n\r",
                        ch->specials.spells_to_learn);
                sprintf(buf,
                        "You can practice any of these skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, MONK_LEVEL_IND);
                /*
                 * list by level, so new skills show at top of list 
                 */
                while (x != 0) {
                    while (monkskills[i].level != -1) {
                        if (monkskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    monkskills[i].level,
                                    monkskills[i].name,
                                    how_good(ch->
                                             skills[monkskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[monkskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
                if (ch->specials.remortclass == MONK_LEVEL_IND + 1) {
                    sprintf(buf,
                            "\n\rSince you picked monk as your main class, you get these bonus skills:\n\r\n\r");
                    strcat(buffer, buf);
                    x = GET_LEVEL(ch, MONK_LEVEL_IND);
                    while (x != 0) {
                        while (mainmonkskills[i].level != -1) {
                            if (mainmonkskills[i].level == x) {
                                sprintf(buf, "[%-2d] %-30s %-15s",
                                        mainmonkskills[i].level,
                                        mainmonkskills[i].name,
                                        how_good(ch->
                                                 skills[mainmonkskills[i].
                                                        skillnum].
                                                 learned));
                                if (IsSpecialized
                                    (ch->
                                     skills[mainmonkskills[i].skillnum].
                                     special))
                                    strcat(buf, " (special)");
                                strcat(buf, " \n\r");
                                if (strlen(buf) + strlen(buffer) >
                                    (MAX_STRING_LENGTH * 2) - 2)
                                    break;
                                strcat(buffer, buf);
                                strcat(buffer, "\r");
                            }
                            i++;
                        }
                        i = 0;
                        x--;
                    }
                }
                page_string(ch->desc, buffer, 1);
                return (TRUE);
            } else {
                x = 0;
                while (monkskills[x].level != -1) {
                    if (is_abbrev(arg, monkskills[x].name)) {   // !str_cmp(arg,n_skills[x])){
                        if (monkskills[x].level >
                            GET_LEVEL(ch, MONK_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }

                        if (monkskills[x].skillnum == SKILL_SAFE_FALL
                            || monkskills[x].skillnum == SKILL_DODGE) {
                            if (ch->skills[monkskills[x].skillnum].
                                learned >= 95) {
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You are a master of this art, I can teach you no more.'\n\r",
                                     ch);
                                SET_BIT(ch->skills[monkskills[x].skillnum].
                                        flags, SKILL_KNOWN);
                                return (TRUE);
                            }
                        } else {
                            if (ch->skills[monkskills[x].skillnum].
                                learned > 45) {
                                // check if skill already practiced
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You must learn from experience and practice to get"
                                     " any better at that skill.'\n\r",
                                     ch);
                                return (TRUE);
                            }
                        }

                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }

                        sprintf(buf, "You practice %s for a while.\n\r",
                                monkskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;

                        if (!IS_SET
                            (ch->skills[monkskills[x].skillnum].flags,
                             SKILL_KNOWN)) {
                            SET_BIT(ch->skills[monkskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->skills[monkskills[x].skillnum].
                                    flags, SKILL_KNOWN_MONK);
                        }
                        percent =
                            ch->skills[monkskills[x].skillnum].learned +
                            int_app[GET_INT(ch)].learn;
                        ch->skills[monkskills[x].skillnum].learned =
                            MIN(95, percent);

                        if (ch->skills[monkskills[x].skillnum].learned >=
                            95)
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
                                 " 'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
                if (ch->specials.remortclass == MONK_LEVEL_IND + 1) {
                    x = 0;
                    while (mainmonkskills[x].level != -1) {
                        if (is_abbrev(arg, mainmonkskills[x].name)) {   // !str_cmp(arg,n_skills[x])){
                            if (mainmonkskills[x].level >
                                GET_LEVEL(ch, MONK_LEVEL_IND)) {
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You're not experienced enough to learn this skill.'",
                                     ch);
                                return (TRUE);
                            }
                            if (ch->skills[mainmonkskills[x].skillnum].
                                learned > 45) {
                                // check if skill already practiced
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You must learn from experience and practice to get"
                                     " any better at that skill.'\n\r",
                                     ch);
                                return (TRUE);
                            }
                            if (ch->specials.spells_to_learn <= 0) {
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You don't have enough practice points.'\n\r",
                                     ch);
                                return (TRUE);
                            }
                            sprintf(buf,
                                    "You practice %s for a while.\n\r",
                                    mainmonkskills[x].name);
                            send_to_char(buf, ch);
                            ch->specials.spells_to_learn--;
                            if (!IS_SET
                                (ch->skills[mainmonkskills[x].skillnum].
                                 flags, SKILL_KNOWN)) {
                                SET_BIT(ch->
                                        skills[mainmonkskills[x].skillnum].
                                        flags, SKILL_KNOWN);
                                SET_BIT(ch->
                                        skills[mainmonkskills[x].skillnum].
                                        flags, SKILL_KNOWN_MONK);
                            }
                            percent =
                                ch->skills[mainmonkskills[x].skillnum].
                                learned + int_app[GET_INT(ch)].learn;
                            ch->skills[mainmonkskills[x].skillnum].
                                learned = MIN(95, percent);
                            if (ch->skills[mainmonkskills[x].skillnum].
                                learned >= 95)
                                send_to_char
                                    ("'You are now a master of this art.'\n\r",
                                     ch);
                            return (TRUE);
                        }
                        x++;
                    }
                }
                send_to_char
                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
                     "I do not know of that skill!'\n\r", ch);
                return (TRUE);
            }
        } else if (HasClass(ch, CLASS_WARRIOR)) {
            if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {    /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
                sprintf(buffer,
                        "You have got %d practice sessions left.\n\r\n\r",
                        ch->specials.spells_to_learn);
                sprintf(buf, "You can practice these skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, WARRIOR_LEVEL_IND);
                /*
                 * list by level, so new skills show at top of list 
                 */
                while (x != 0) {
                    while (warmonkskills[i].level != -1) {
                        if (warmonkskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    warmonkskills[i].level,
                                    warmonkskills[i].name,
                                    how_good(ch->
                                             skills[warmonkskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[warmonkskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
                page_string(ch->desc, buffer, 1);
                return (TRUE);
            } else {
                x = 0;
                while (warmonkskills[x].level != -1) {
                    if (is_abbrev(arg, warmonkskills[x].name)) {        // !str_cmp(arg,n_skills[x])){
                        if (warmonkskills[x].level >
                            GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }

                        if (warmonkskills[x].skillnum == SKILL_DODGE) {
                            if (ch->skills[warmonkskills[x].skillnum].
                                learned >= 95) {
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You are a master of this art, I can teach you no more.'\n\r",
                                     ch);
                                SET_BIT(ch->
                                        skills[warmonkskills[x].skillnum].
                                        flags, SKILL_KNOWN);
                                return (TRUE);
                            }
                        } else {
                            if (ch->skills[warmonkskills[x].skillnum].
                                learned > 45) {
                                // check if skill already practiced
                                send_to_char
                                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                     " 'You must learn from experience and practice to get"
                                     " any better at that skill.'\n\r",
                                     ch);
                                return (TRUE);
                            }
                        }

                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }

                        sprintf(buf, "You practice %s for a while.\n\r",
                                warmonkskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;

                        if (!IS_SET
                            (ch->skills[warmonkskills[x].skillnum].flags,
                             SKILL_KNOWN)) {
                            SET_BIT(ch->skills[warmonkskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->skills[warmonkskills[x].skillnum].
                                    flags, SKILL_KNOWN_WARRIOR);
                        }
                        percent =
                            ch->skills[warmonkskills[x].skillnum].learned +
                            int_app[GET_INT(ch)].learn;
                        ch->skills[warmonkskills[x].skillnum].learned =
                            MIN(95, percent);

                        if (ch->skills[warmonkskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
                                 " 'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
                send_to_char
                    ("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
                     "I do not know of that skill!'\n\r", ch);
                return (TRUE);
            }
        } else {
            send_to_char
                ("$c0013[$c0015The Monk Guildmaster$c0013] tells you '"
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {

        if (!HasClass(ch, CLASS_DRUID)) {
            send_to_char
                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                 " 'You're not a druid.'\n\r", ch);
            return (TRUE);
        }

        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room, DruidGuildMaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, DRUID_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[DRUID_LEVEL_IND][GET_LEVEL(ch, DRUID_LEVEL_IND) +
                                    1].exp) {
            send_to_char("You're not ready to gain yet!\n\r", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                if (GET_LEVEL(ch, DRUID_LEVEL_IND) <= 9) {
                    GainLevel(ch, DRUID_LEVEL_IND);
                } else {
                    send_to_char
                        ("You must fight another druid for this title.\n\r",
                         ch);
                }
                return (TRUE);
            }
        }

        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            x = GET_LEVEL(ch, DRUID_LEVEL_IND);
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (druidskills[i].level != -1) {
                    if (druidskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                druidskills[i].level, druidskills[i].name,
                                how_good(ch->
                                         skills[druidskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[druidskills[i].skillnum].special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            if (ch->specials.remortclass == DRUID_LEVEL_IND + 1) {
                sprintf(buf,
                        "\n\rSince you picked druid as your main class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, DRUID_LEVEL_IND);
                while (x != 0) {
                    while (maindruidskills[i].level != -1) {
                        if (maindruidskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    maindruidskills[i].level,
                                    maindruidskills[i].name,
                                    how_good(ch->
                                             skills[maindruidskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[maindruidskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (druidskills[x].level != -1) {
                if (is_abbrev(arg, druidskills[x].name)) {      // !str_cmp(arg,n_skills[x])){
                    if (druidskills[x].level >
                        GET_LEVEL(ch, DRUID_LEVEL_IND)) {
                        send_to_char
                            ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[druidskills[x].skillnum].learned > 45) {
                        // check if skill already practiced
                        send_to_char
                            ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                             " 'You must learn from experience and practice to get"
                             " any better at that skill.'\n\r", ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            druidskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;

                    if (!IS_SET
                        (ch->skills[druidskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[druidskills[x].skillnum].flags,
                                SKILL_KNOWN);
                        SET_BIT(ch->skills[druidskills[x].skillnum].flags,
                                SKILL_KNOWN_DRUID);
                    }
                    percent =
                        ch->skills[druidskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[druidskills[x].skillnum].learned =
                        MIN(95, percent);

                    if (ch->skills[druidskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            x = 0;
            if (ch->specials.remortclass == DRUID_LEVEL_IND + 1) {
                while (maindruidskills[x].level != -1) {
                    if (is_abbrev(arg, maindruidskills[x].name)) {      // !str_cmp(arg,n_skills[x])){
                        if (maindruidskills[x].level >
                            GET_LEVEL(ch, DRUID_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }

                        if (ch->skills[maindruidskills[x].skillnum].
                            learned > 45) {
                            // check if skill already practiced
                            send_to_char
                                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                                 " 'You must learn from experience and practice to get"
                                 " any better at that skill.'\n\r", ch);
                            return (TRUE);
                        }

                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }

                        sprintf(buf, "You practice %s for a while.\n\r",
                                maindruidskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;

                        if (!IS_SET
                            (ch->skills[maindruidskills[x].skillnum].flags,
                             SKILL_KNOWN)) {
                            SET_BIT(ch->
                                    skills[maindruidskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->
                                    skills[maindruidskills[x].skillnum].
                                    flags, SKILL_KNOWN_DRUID);
                        }
                        percent =
                            ch->skills[maindruidskills[x].skillnum].
                            learned + int_app[GET_INT(ch)].learn;
                        ch->skills[maindruidskills[x].skillnum].learned =
                            MIN(95, percent);

                        if (ch->skills[maindruidskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
            }
            send_to_char
                ("$c0013[$c0015The Druid Guildmaster$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {

        if (!HasClass(ch, CLASS_BARBARIAN)) {
            send_to_char
                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                 " 'You're not a barbarian.'\n\r", ch);
            return (TRUE);
        }
        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room,
                                          barbarian_guildmaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[BARBARIAN_LEVEL_IND][GET_LEVEL(ch, BARBARIAN_LEVEL_IND)
                                        + 1].exp) {
            send_to_char("Your not ready to gain yet!", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                GainLevel(ch, BARBARIAN_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            x = GET_LEVEL(ch, BARBARIAN_LEVEL_IND);
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (barbskills[i].level != -1) {
                    if (barbskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                barbskills[i].level, barbskills[i].name,
                                how_good(ch->
                                         skills[barbskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[barbskills[i].skillnum].special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
                sprintf(buf,
                        "\n\rSince you picked barbarian as your main class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, BARBARIAN_LEVEL_IND);
                while (x != 0) {
                    while (mainbarbskills[i].level != -1) {
                        if (mainbarbskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    mainbarbskills[i].level,
                                    mainbarbskills[i].name,
                                    how_good(ch->
                                             skills[mainbarbskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[mainbarbskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (barbskills[x].level != -1) {
                if (is_abbrev(arg, barbskills[x].name)) {       // !str_cmp(arg,n_skills[x])){
                    if (barbskills[x].level >
                        GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
                        send_to_char
                            ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[barbskills[x].skillnum].learned > 45) {
                        // check if skill already practiced
                        send_to_char
                            ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                             " 'You must learn from experience and practice to get"
                             " any better at that skill.'\n\r", ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            barbskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;

                    if (!IS_SET
                        (ch->skills[barbskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[barbskills[x].skillnum].flags,
                                SKILL_KNOWN);
                        SET_BIT(ch->skills[barbskills[x].skillnum].flags,
                                SKILL_KNOWN_BARBARIAN);
                    }
                    percent =
                        ch->skills[barbskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[barbskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[barbskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            if (ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
                x = 0;
                while (mainbarbskills[x].level != -1) {
                    if (is_abbrev(arg, mainbarbskills[x].name)) {       // !str_cmp(arg,n_skills[x])){
                        if (mainbarbskills[x].level >
                            GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }
                        if (ch->skills[mainbarbskills[x].skillnum].
                            learned > 45) {
                            // check if skill already practiced
                            send_to_char
                                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                                 " 'You must learn from experience and practice to get"
                                 " any better at that skill.'\n\r", ch);
                            return (TRUE);
                        }
                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }
                        sprintf(buf, "You practice %s for a while.\n\r",
                                mainbarbskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;
                        if (!IS_SET
                            (ch->skills[mainbarbskills[x].skillnum].flags,
                             SKILL_KNOWN)) {
                            SET_BIT(ch->skills[mainbarbskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->skills[mainbarbskills[x].skillnum].
                                    flags, SKILL_KNOWN_BARBARIAN);
                        }
                        percent =
                            ch->skills[mainbarbskills[x].skillnum].
                            learned + int_app[GET_INT(ch)].learn;
                        ch->skills[mainbarbskills[x].skillnum].learned =
                            MIN(95, percent);
                        if (ch->skills[mainbarbskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
            }
            send_to_char
                ("$c0013[$c0015The Barbarian Guildmaster$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {

        if (!HasClass(ch, CLASS_RANGER)) {
            send_to_char
                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                 " 'You're not a ranger.'\n\r", ch);
            return (TRUE);
        }
        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room, RangerGuildmaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, RANGER_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[RANGER_LEVEL_IND][GET_LEVEL(ch, RANGER_LEVEL_IND) +
                                     1].exp) {
            send_to_char("Your not ready to gain yet!", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                GainLevel(ch, RANGER_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf,
                    "You can practice any of these spells and skills:\n\r\n\r");
            strcat(buffer, buf);
            x = GET_LEVEL(ch, RANGER_LEVEL_IND);
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (rangerskills[i].level != -1) {
                    if (rangerskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                rangerskills[i].level,
                                rangerskills[i].name,
                                how_good(ch->
                                         skills[rangerskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[rangerskills[i].skillnum].special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            if (ch->specials.remortclass == RANGER_LEVEL_IND + 1) {
                sprintf(buf,
                        "\n\rSince you picked ranger as your main class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, RANGER_LEVEL_IND);
                while (x != 0) {
                    while (mainrangerskills[i].level != -1) {
                        if (mainrangerskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    mainrangerskills[i].level,
                                    mainrangerskills[i].name,
                                    how_good(ch->
                                             skills[mainrangerskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[mainrangerskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (rangerskills[x].level != -1) {
                if (is_abbrev(arg, rangerskills[x].name)) {     // !str_cmp(arg,n_skills[x])){
                    if (rangerskills[x].level >
                        GET_LEVEL(ch, RANGER_LEVEL_IND)) {
                        send_to_char
                            ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[rangerskills[x].skillnum].learned > 45) {
                        // check if skill already practiced
                        send_to_char
                            ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                             " 'You must learn from experience and practice to get"
                             " any better at that skill.'\n\r", ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            rangerskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;

                    if (!IS_SET
                        (ch->skills[rangerskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[rangerskills[x].skillnum].flags,
                                SKILL_KNOWN);
                        SET_BIT(ch->skills[rangerskills[x].skillnum].flags,
                                SKILL_KNOWN_RANGER);
                    }
                    percent =
                        ch->skills[rangerskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[rangerskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[rangerskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            if (ch->specials.remortclass == RANGER_LEVEL_IND + 1) {
                x = 0;
                while (mainrangerskills[x].level != -1) {
                    if (is_abbrev(arg, mainrangerskills[x].name)) {     // !str_cmp(arg,n_skills[x])){
                        if (mainrangerskills[x].level >
                            GET_LEVEL(ch, RANGER_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }
                        if (ch->skills[mainrangerskills[x].skillnum].
                            learned > 45) {
                            // check if skill already practiced
                            send_to_char
                                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                                 " 'You must learn from experience and practice to get"
                                 " any better at that skill.'\n\r", ch);
                            return (TRUE);
                        }
                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }
                        sprintf(buf, "You practice %s for a while.\n\r",
                                mainrangerskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;
                        if (!IS_SET
                            (ch->skills[mainrangerskills[x].skillnum].
                             flags, SKILL_KNOWN)) {
                            SET_BIT(ch->
                                    skills[mainrangerskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->
                                    skills[mainrangerskills[x].skillnum].
                                    flags, SKILL_KNOWN_RANGER);
                        }
                        percent =
                            ch->skills[mainrangerskills[x].skillnum].
                            learned + int_app[GET_INT(ch)].learn;
                        ch->skills[mainrangerskills[x].skillnum].learned =
                            MIN(95, percent);
                        if (ch->skills[mainrangerskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
            }
            send_to_char
                ("$c0013[$c0015The Ranger Guildmaster$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {

        if (!HasClass(ch, CLASS_PSI)) {
            send_to_char
                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                 " 'You're not a psi.'\n\r", ch);
            return (TRUE);
        }
        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room, PsiGuildmaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, PSI_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[PSI_LEVEL_IND][GET_LEVEL(ch, PSI_LEVEL_IND) + 1].exp) {
            send_to_char("Your not ready to gain yet!", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                GainLevel(ch, PSI_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these spells:\n\r\n\r");
            strcat(buffer, buf);
            x = GET_LEVEL(ch, PSI_LEVEL_IND);
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (psiskills[i].level != -1) {
                    if (psiskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                psiskills[i].level, psiskills[i].name,
                                how_good(ch->skills[psiskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[psiskills[i].skillnum].special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
                sprintf(buf,
                        "\n\rSince you picked psi as your main class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, PSI_LEVEL_IND);
                while (x != 0) {
                    while (mainpsiskills[i].level != -1) {
                        if (mainpsiskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    mainpsiskills[i].level,
                                    mainpsiskills[i].name,
                                    how_good(ch->
                                             skills[mainpsiskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[mainpsiskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (psiskills[x].level != -1) {
                if (is_abbrev(arg, psiskills[x].name)) {        // !str_cmp(arg,n_skills[x])){
                    if (psiskills[x].level > GET_LEVEL(ch, PSI_LEVEL_IND)) {
                        send_to_char
                            ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[psiskills[x].skillnum].learned > 45) {
                        // check if skill already practiced
                        send_to_char
                            ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                             " 'You must learn from experience and practice to get"
                             " any better at that skill.'\n\r", ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            psiskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;

                    if (!IS_SET
                        (ch->skills[psiskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[psiskills[x].skillnum].flags,
                                SKILL_KNOWN);
                        SET_BIT(ch->skills[psiskills[x].skillnum].flags,
                                SKILL_KNOWN_PSI);
                    }
                    percent =
                        ch->skills[psiskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[psiskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[psiskills[x].skillnum].learned >= 95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            if (ch->specials.remortclass == PSI_LEVEL_IND + 1) {
                x = 0;
                while (mainpsiskills[x].level != -1) {
                    if (is_abbrev(arg, mainpsiskills[x].name)) {        // !str_cmp(arg,n_skills[x])){
                        if (mainpsiskills[x].level >
                            GET_LEVEL(ch, PSI_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }
                        if (ch->skills[mainpsiskills[x].skillnum].learned >
                            45) {
                            // check if skill already practiced
                            send_to_char
                                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                                 " 'You must learn from experience and practice to get"
                                 " any better at that skill.'\n\r", ch);
                            return (TRUE);
                        }
                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }
                        sprintf(buf, "You practice %s for a while.\n\r",
                                mainpsiskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;
                        if (!IS_SET
                            (ch->skills[mainpsiskills[x].skillnum].flags,
                             SKILL_KNOWN)) {
                            SET_BIT(ch->skills[mainpsiskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->skills[mainpsiskills[x].skillnum].
                                    flags, SKILL_KNOWN_PSI);
                        }
                        percent =
                            ch->skills[mainpsiskills[x].skillnum].learned +
                            int_app[GET_INT(ch)].learn;
                        ch->skills[mainpsiskills[x].skillnum].learned =
                            MIN(95, percent);
                        if (ch->skills[mainpsiskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
            }
            send_to_char
                ("$c0013[$c0015The Psi Guildmaster$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
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
    static int      x = 0;      // for loop
    int             i = 0;      // while loop
    struct char_data *guildmaster;

    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise, 243->gain
    if (cmd == 164 || cmd == 170 || cmd == 243 || cmd == 582) {

        if (!HasClass(ch, CLASS_PALADIN)) {
            send_to_char
                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                 " 'You're not a paladin.'\n\r", ch);
            return (TRUE);
        }
        if (!mob) {
            guildmaster =
                FindMobInRoomWithFunction(ch->in_room, PaladinGuildmaster);
        } else {
            guildmaster = mob;
        }
        if (GET_LEVEL(ch, PALADIN_LEVEL_IND) > GetMaxLevel(guildmaster)) {
            send_to_char
                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                 " 'You must learn from another, I can no longer train you.'\n\r",
                 ch);
            return (TRUE);
        }
        // gain
        if (cmd == 243
            && GET_EXP(ch) <
            titles[PALADIN_LEVEL_IND][GET_LEVEL(ch, PALADIN_LEVEL_IND) +
                                      1].exp) {
            send_to_char("Your not ready to gain yet!", ch);
            return (FALSE);
        } else {
            if (cmd == 243) {   // gain
                GainLevel(ch, PALADIN_LEVEL_IND);
                return (TRUE);
            }
        }

        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf,
                    "You can practice any of these spells and skills:\n\r\n\r");
            strcat(buffer, buf);
            x = GET_LEVEL(ch, PALADIN_LEVEL_IND);
            /*
             * list by level, so new skills show at top of list 
             */
            while (x != 0) {
                while (paladinskills[i].level != -1) {
                    if (paladinskills[i].level == x) {
                        sprintf(buf, "[%-2d] %-30s %-15s",
                                paladinskills[i].level,
                                paladinskills[i].name,
                                how_good(ch->
                                         skills[paladinskills[i].skillnum].
                                         learned));
                        if (IsSpecialized
                            (ch->skills[paladinskills[i].skillnum].
                             special))
                            strcat(buf, " (special)");
                        strcat(buf, " \n\r");
                        if (strlen(buf) + strlen(buffer) >
                            (MAX_STRING_LENGTH * 2) - 2)
                            break;
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                    i++;
                }
                i = 0;
                x--;
            }
            if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
                sprintf(buf,
                        "\n\rSince you picked paladin as your main class, you get these bonus skills:\n\r\n\r");
                strcat(buffer, buf);
                x = GET_LEVEL(ch, PALADIN_LEVEL_IND);
                while (x != 0) {
                    while (mainpaladinskills[i].level != -1) {
                        if (mainpaladinskills[i].level == x) {
                            sprintf(buf, "[%-2d] %-30s %-15s",
                                    mainpaladinskills[i].level,
                                    mainpaladinskills[i].name,
                                    how_good(ch->
                                             skills[mainpaladinskills[i].
                                                    skillnum].learned));
                            if (IsSpecialized
                                (ch->skills[mainpaladinskills[i].skillnum].
                                 special))
                                strcat(buf, " (special)");
                            strcat(buf, " \n\r");
                            if (strlen(buf) + strlen(buffer) >
                                (MAX_STRING_LENGTH * 2) - 2)
                                break;
                            strcat(buffer, buf);
                            strcat(buffer, "\r");
                        }
                        i++;
                    }
                    i = 0;
                    x--;
                }
            }
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {
            x = 0;
            while (paladinskills[x].level != -1) {
                if (is_abbrev(arg, paladinskills[x].name)) {    // !str_cmp(arg,n_skills[x])){
                    if (paladinskills[x].level >
                        GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
                        send_to_char
                            ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                             " 'You're not experienced enough to learn this skill.'",
                             ch);
                        return (TRUE);
                    }

                    if (ch->skills[paladinskills[x].skillnum].learned > 45) {
                        // check if skill already practiced
                        send_to_char
                            ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                             " 'You must learn from experience and practice to get"
                             " any better at that skill.'\n\r", ch);
                        return (TRUE);
                    }

                    if (ch->specials.spells_to_learn <= 0) {
                        send_to_char
                            ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                             " 'You don't have enough practice points.'\n\r",
                             ch);
                        return (TRUE);
                    }

                    sprintf(buf, "You practice %s for a while.\n\r",
                            paladinskills[x].name);
                    send_to_char(buf, ch);
                    ch->specials.spells_to_learn--;

                    if (!IS_SET
                        (ch->skills[paladinskills[x].skillnum].flags,
                         SKILL_KNOWN)) {
                        SET_BIT(ch->skills[paladinskills[x].skillnum].
                                flags, SKILL_KNOWN);
                        SET_BIT(ch->skills[paladinskills[x].skillnum].
                                flags, SKILL_KNOWN_PALADIN);
                    }
                    percent =
                        ch->skills[paladinskills[x].skillnum].learned +
                        int_app[GET_INT(ch)].learn;
                    ch->skills[paladinskills[x].skillnum].learned =
                        MIN(95, percent);
                    if (ch->skills[paladinskills[x].skillnum].learned >=
                        95)
                        send_to_char
                            ("'You are now a master of this art.'\n\r",
                             ch);
                    return (TRUE);
                }
                x++;
            }
            if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
                x = 0;
                while (mainpaladinskills[x].level != -1) {
                    if (is_abbrev(arg, mainpaladinskills[x].name)) {    // !str_cmp(arg,n_skills[x])){
                        if (mainpaladinskills[x].level >
                            GET_LEVEL(ch, PALADIN_LEVEL_IND)) {
                            send_to_char
                                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                                 " 'You're not experienced enough to learn this skill.'",
                                 ch);
                            return (TRUE);
                        }
                        if (ch->skills[mainpaladinskills[x].skillnum].
                            learned > 45) {
                            // check if skill already practiced
                            send_to_char
                                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                                 " 'You must learn from experience and practice to get"
                                 " any better at that skill.'\n\r", ch);
                            return (TRUE);
                        }
                        if (ch->specials.spells_to_learn <= 0) {
                            send_to_char
                                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you"
                                 " 'You don't have enough practice points.'\n\r",
                                 ch);
                            return (TRUE);
                        }
                        sprintf(buf, "You practice %s for a while.\n\r",
                                mainpaladinskills[x].name);
                        send_to_char(buf, ch);
                        ch->specials.spells_to_learn--;
                        if (!IS_SET
                            (ch->skills[mainpaladinskills[x].skillnum].
                             flags, SKILL_KNOWN)) {
                            SET_BIT(ch->
                                    skills[mainpaladinskills[x].skillnum].
                                    flags, SKILL_KNOWN);
                            SET_BIT(ch->
                                    skills[mainpaladinskills[x].skillnum].
                                    flags, SKILL_KNOWN_PALADIN);
                        }
                        percent =
                            ch->skills[mainpaladinskills[x].skillnum].
                            learned + int_app[GET_INT(ch)].learn;
                        ch->skills[mainpaladinskills[x].skillnum].learned =
                            MIN(95, percent);
                        if (ch->skills[mainpaladinskills[x].skillnum].
                            learned >= 95)
                            send_to_char
                                ("'You are now a master of this art.'\n\r",
                                 ch);
                        return (TRUE);
                    }
                    x++;
                }
            }
            send_to_char
                ("$c0013[$c0015The Paladin Guildmaster$c0013] tells you '"
                 "I do not know of that skill!'\n\r", ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

#define SPELL_SPECIAL_COST 100000       /* 100k to specialize per spell */
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

    if (!AWAKE(ch))
        return (FALSE);

    if (!cmd) {
        if (ch->specials.fighting) {
            return (magic_user(ch, cmd, arg, ch, 0));
        }
        return (FALSE);
    }

    if (!ch->skills)
        return (FALSE);

    if (check_soundproof(ch))
        return (FALSE);

    guildmaster =
        FindMobInRoomWithFunction(ch->in_room,
                                  mage_specialist_guildmaster);

    if (!guildmaster)
        return (FALSE);

    for (; *arg == ' '; arg++); /* ditch spaces */

    if ((cmd == 164) || (cmd == 170) || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_MAGIC_USER)) {
            do_say(guildmaster, "I do not teach heathens!", 0);
            return (TRUE);
        }
        if (HasFightingClass(ch) == TRUE) {
            do_say(guildmaster,
                   "Your warrior instincts are rather scary.. Begone...",
                   0);
            return (FALSE);
        }
        if (IS_NPC(ch)) {
            do_say(guildmaster, "What do I look like? An animal trainer?",
                   0);
            return (FALSE);
        }

        if (!*arg) {
            sprintf(buf, "You have got %d practice sessions left.\n\r",
                    ch->specials.spells_to_learn);
            send_to_char(buf, ch);
            send_to_char("You can practise any of these spells:\n\r", ch);
            for (i = 0; *spells[i] != '\n'; i++)
                if (spell_info[i + 1].spell_pointer &&
                    (spell_info[i + 1].min_level_magic <=
                     GET_LEVEL(ch, MAGE_LEVEL_IND)) &&
                    (spell_info[i + 1].min_level_magic <=
                     GetMaxLevel(guildmaster) - 10)) {

                    sprintf(buf, "[%d] %-15s %s \n\r",
                            spell_info[i + 1].min_level_magic,
                            spells[i],
                            how_good(ch->skills[i + 1].learned));
                    send_to_char(buf, ch);
                }

            return (TRUE);

        }
        for (; isspace(*arg); arg++);
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
            do_say(guildmaster,
                   "Ah, but you do not have enough money to pay.", 0);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn < 4) {
            do_say(guildmaster,
                   "You must first use the knowledge you already have.",
                   0);
            return (TRUE);
        }
        if (IsSpecialized(ch->skills[number].special)) {
            do_say(guildmaster,
                   "You are already proficient in this spell!", 0);
            return (TRUE);
        }

        if (HowManySpecials(ch) > MAX_SPECIALS(ch)) {
            do_say(guildmaster,
                   "You are already specialized in several skills.", 0);
            return (TRUE);
        }

        if (ch->skills[number].learned < 95) {
            do_say(guildmaster,
                   "You must fully learn this spell first.", 0);
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

    if (!AWAKE(ch))
        return (FALSE);

    if (!cmd) {
        if (ch->specials.fighting) {
            return (cleric(ch, cmd, arg, ch, 0));
        }
        return (FALSE);
    }

    if (!ch->skills)
        return (FALSE);

    if (check_soundproof(ch))
        return (FALSE);

    guildmaster =
        FindMobInRoomWithFunction(ch->in_room,
                                  cleric_specialist_guildmaster);

    if (!guildmaster)
        return (FALSE);

    for (; *arg == ' '; arg++); /* ditch spaces */

    if ((cmd == 164) || (cmd == 170) || cmd == 243 || cmd == 582) {
        if (!HasClass(ch, CLASS_CLERIC)) {
            do_say(guildmaster, "I do not teach heathens!", 0);
            return (TRUE);
        }
        if (HasFightingClass(ch) == TRUE) {
            do_say(guildmaster,
                   "Your warrior instincts are rather scary.. Begone...",
                   0);
            return (FALSE);
        }
        if (IS_NPC(ch)) {
            do_say(guildmaster, "What do I look like? An animal trainer?",
                   0);
            return (FALSE);
        }

        if (!*arg) {
            sprintf(buf, "You have got %d practice sessions left.\n\r",
                    ch->specials.spells_to_learn);
            send_to_char(buf, ch);
            send_to_char("You can practise any of these spells:\n\r", ch);
            for (i = 0; *spells[i] != '\n'; i++)
                if (spell_info[i + 1].spell_pointer &&
                    (spell_info[i + 1].min_level_cleric <=
                     GET_LEVEL(ch, CLERIC_LEVEL_IND)) &&
                    (spell_info[i + 1].min_level_cleric <=
                     GetMaxLevel(guildmaster) - 10)) {

                    sprintf(buf, "[%d] %-15s %s \n\r",
                            spell_info[i + 1].min_level_cleric,
                            spells[i],
                            how_good(ch->skills[i + 1].learned));
                    send_to_char(buf, ch);
                }

            return (TRUE);

        }
        for (; isspace(*arg); arg++);
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
            do_say(guildmaster,
                   "Ah, but you do not have enough money to pay.", 0);
            return (TRUE);
        }

        if (ch->specials.spells_to_learn < 4) {
            do_say(guildmaster,
                   "You do not seem to be able to practice at the moment.",
                   0);
            return (TRUE);
        }
        if (IsSpecialized(ch->skills[number].special)) {
            do_say(guildmaster,
                   "You are already proficient in this spell!", 0);
            return (TRUE);
        }

        if (HowManySpecials(ch) > MAX_SPECIALS(ch)) {
            do_say(guildmaster,
                   "You are already specialized in several skills.", 0);
            return (TRUE);
        }

        if (ch->skills[number].learned < 95) {
            do_say(guildmaster,
                   "You must fully learn this spell first.", 0);
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
    static int      x = 0;      // for loop
    int             i = 0,
                    charge = 0,
                    skillnum = 0;       // while loop
    struct char_data *guildmaster;

#if 1
    if (!AWAKE(ch) || IS_NPC(ch))
        return (FALSE);

    // 170->Practice,164->Practise,
    if (cmd == 164 || cmd == 170 || cmd == 582) {
        if (!*arg && (cmd == 170 || cmd == 164 || cmd == 582)) {        /* practice||practise, 
                                                                         * without 
                                                                         * argument 
                                                                         */
            sprintf(buffer,
                    "You have got %d practice sessions left.\n\r\n\r",
                    ch->specials.spells_to_learn);
            sprintf(buf, "You can practice any of these skills:\n\r\n\r");
            strcat(buffer, buf);
            if (HasClass(ch, CLASS_WARRIOR)) {
                while (warninjaskills[i].level != -1) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            warninjaskills[i].level,
                            warninjaskills[i].name,
                            how_good(ch->
                                     skills[warninjaskills[i].skillnum].
                                     learned));
                    if (IsSpecialized
                        (ch->skills[warninjaskills[i].skillnum].special))
                        strcat(buf, " (special)");
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2)
                        break;
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                    i++;
                }
                i = 0;
            }
            if (HasClass(ch, CLASS_THIEF)) {
                while (thfninjaskills[i].level != -1) {
                    sprintf(buf, "[%-2d] %-30s %-15s",
                            thfninjaskills[i].level,
                            thfninjaskills[i].name,
                            (IS_SET
                             (ch->skills[thfninjaskills[i].skillnum].flags,
                              SKILL_KNOWN)) ? (how_good(ch->
                                                        skills
                                                        [thfninjaskills[i].
                                                         skillnum].
                                                        learned)) :
                            " (not learned)");
                    if (IsSpecialized
                        (ch->skills[thfninjaskills[i].skillnum].special))
                        strcat(buf, " (special)");
                    strcat(buf, " \n\r");
                    if (strlen(buf) + strlen(buffer) >
                        (MAX_STRING_LENGTH * 2) - 2)
                        break;
                    strcat(buffer, buf);
                    strcat(buffer, "\r");
                    i++;
                }
                i = 0;
            }
            sprintf(buf, "[%-2d] %-30s %-15s", allninjaskills[i].level,
                    allninjaskills[i].name,
                    how_good(ch->skills[allninjaskills[i].skillnum].
                             learned));
            if (IsSpecialized
                (ch->skills[allninjaskills[i].skillnum].special))
                strcat(buf, " (special)");
            strcat(buf, " \n\r");
            strcat(buffer, buf);
            strcat(buffer, "\r");
            page_string(ch->desc, buffer, 1);
            return (TRUE);
        } else {                /* includes arg.. */
            x = 0;
            if (HasClass(ch, CLASS_WARRIOR)) {
                while (warninjaskills[x].level != -1) {
                    if (is_abbrev(arg, warninjaskills[x].name)) {
                        skillnum = warninjaskills[x].skillnum;
                        sprintf(skillname, "%s", warninjaskills[x].name);
                    }
                    x++;
                }
                x = 0;
            }
            if (HasClass(ch, CLASS_THIEF)) {
                while (thfninjaskills[x].level != -1) {
                    if (is_abbrev(arg, thfninjaskills[x].name)) {
                        skillnum = thfninjaskills[x].skillnum;
                        sprintf(skillname, "%s", thfninjaskills[x].name);
                    }
                    x++;
                }
                x = 0;
            }
            while (allninjaskills[x].level != -1) {
                if (is_abbrev(arg, allninjaskills[x].name)) {
                    skillnum = allninjaskills[x].skillnum;
                    sprintf(skillname, "%s", allninjaskills[x].name);
                }
                x++;
            }
            x = 0;
            if (skillnum != 0) {        /* arg matches a skillnum */
                if (ch->specials.spells_to_learn <= 0) {
                    send_to_char
                        ("$c0013[$c0015The Ninja Master$c0013] tells you"
                         " 'You don't have enough practice points.'\n\r",
                         ch);
                    return (TRUE);
                }

                charge = GetMaxLevel(ch) * 100;
                if (GET_GOLD(ch) < charge) {
                    send_to_char
                        ("$c0013[$c0015The Ninja Master$c0013] tells you"
                         " 'Ah, but you do not have enough money to pay.'\n\r",
                         ch);
                    return (TRUE);
                }

                if (ch->skills[skillnum].learned >= 96) {
                    send_to_char
                        ("$c0013[$c0015The Ninja Master$c0013] tells you"
                         " 'I have taught you all that i can about that skill.'\n\r",
                         ch);
                    return (TRUE);
                }

                GET_GOLD(ch) -= charge;
                sprintf(buf, "You practice %s for a while.\n\r",
                        skillname);
                send_to_char(buf, ch);
                ch->specials.spells_to_learn--;

                if (!IS_SET(ch->skills[skillnum].flags, SKILL_KNOWN)) {
                    SET_BIT(ch->skills[skillnum].flags, SKILL_KNOWN);
                }

                percent =
                    ch->skills[skillnum].learned +
                    int_app[GET_INT(ch)].learn;
                ch->skills[skillnum].learned = MIN(95, percent);

                if (ch->skills[skillnum].learned >= 95)
                    send_to_char("'You are now a master of this art.'\n\r",
                                 ch);

                return (TRUE);
            } else {            /* arg doesn't match a skillnum */
                send_to_char
                    ("$c0013[$c0015The Ninja Master$c0013] tells you '"
                     "I do not know of that skill!'\n\r", ch);
                return (TRUE);
            }
        }
    }
#endif
    return (FALSE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
