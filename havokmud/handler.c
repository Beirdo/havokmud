#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "protos.h"

#ifdef HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct index_data *mob_index;
extern struct descriptor_data *descriptor_list;
extern struct str_app_type str_app[];
extern struct dex_app_type dex_app[];
extern struct zone_data *zone_table;

extern long     SystemFlags;

int             GoodBlade(struct char_data *ch, int cmd, char *arg,
                          struct obj_data *tobj, int type);
int             NeutralBlade(struct char_data *ch, int cmd, char *arg,
                             struct obj_data *tobj, int type);


#define STATUE_ZONE 198
#define OBJ_VNUM_MONEY_SOME     1540
#define OBJ_VNUM_MONEY_ONE      9493

char           *fname(char *namelist)
{
    static char     holder[30];
    register char  *point;

    for (point = holder; isalpha((int)*namelist); namelist++, point++) {
        *point = *namelist;
    }
    *point = '\0';

    return (holder);
}

int split_string(char *str, char *sep, char **argv)
     /*
      * str must be writable 
      */
{
    char           *s;
    int             argc = 0;

    s = strtok(str, sep);
    if (s) {
        argv[argc++] = s;
    } else {
        *argv = str;
        return 1;
    }

    while ((s = strtok(NULL, sep))) {
        argv[argc++] = s;
    }
    return argc;
}

int isname(const char *str, const char *namelist)
{
    char           *argv[100],
                   *xargv[100];
    int             argc,
                    xargc,
                    i,
                    j,
                    exact;
    char            buf[MAX_INPUT_LENGTH + 40],
                    names[MAX_INPUT_LENGTH + 40],
                   *s;

    strcpy(buf, str);
    argc = split_string(buf, "- \t\n\r,", argv);

    strcpy(names, namelist);
    xargc = split_string(names, "- \t\n\r,", xargv);

    s = argv[argc - 1];
    s += strlen(s);
    if (*(--s) == '.') {
        exact = 1;
        *s = 0;
    } else {
        exact = 0;
    }

    /*
     * the string has now been split into separate words with the '-'
     * replaced by string terminators.  pointers to the beginning of each
     * word are in argv 
     */

    if (exact && argc != xargc) {
        return 0;
    }
    for (i = 0; i < argc; i++) {
        for (j = 0; j < xargc; j++) {
            if (argv[i] && xargv[j] && !strcasecmp(argv[i], xargv[j])) {
                xargv[j] = NULL;
                break;
            }
        }
        if (j >= xargc) {
            return 0;
        }
    }

    return 1;
}

int isname2(const char *str, const char *namelist)
{
    char           *argv[100],
                   *xargv[100];
    int             argc,
                    xargc,
                    i,
                    j,
                    exact;
    char            buf[8000],
                    names[8000],
                   *s;

    strcpy(buf, str);
    argc = split_string(buf, "- \t\n\r,", argv);

    strcpy(names, namelist);
    xargc = split_string(names, "- \t\n\r,", xargv);

    s = argv[argc - 1];
    s += strlen(s);
    if (*(--s) == '.') {
        exact = 1;
        *s = 0;
    } else {
        exact = 0;
    }

    /*
     * the string has now been split into separate words with the '-'
     * replaced by string terminators.  pointers to the beginning of each
     * word are in argv 
     */

    if (exact && argc != xargc) {
        return 0;
    }
    for (i = 0; i < argc; i++) {
        for (j = 0; j < xargc; j++) {
            if (argv[i] && xargv[j] &&
                !strncasecmp(argv[i], xargv[j], strlen(argv[i]))) {
                xargv[j] = NULL;
                break;
            }
        }
        if (j >= xargc) {
            return 0;
        }
    }   

    return 1;
}

void init_string_block(struct string_block *sb)
{
    sb->data = (char *) malloc(sb->size = 128);
    *sb->data = '\0';
}

void append_to_string_block(struct string_block *sb, char *str)
{
    int             len;

    len = strlen(sb->data) + strlen(str) + 1;
    if (len > sb->size) {
        if (len > (sb->size *= 2)) {
            sb->size = len;
        }
        sb->data = (char *) realloc(sb->data, sb->size);
    }
    strcat(sb->data, str);
}

void page_string_block(struct string_block *sb, struct char_data *ch)
{
    page_string(ch->desc, sb->data, 1);
}

void destroy_string_block(struct string_block *sb)
{
    if (sb->data) {
        free(sb->data);
    }
    sb->data = NULL;
}

void affect_modify(struct char_data *ch, byte loc, long mod, long bitv,
                   bool add)
{
    int             i,
                    temp,
                    temp2;

    if (!ch) {
        return;
    }
    if (loc == APPLY_IMMUNE) {
        if (add) {
            SET_BIT(ch->immune, mod);
        } else {
            REMOVE_BIT(ch->immune, mod);
        }
    } else if (loc == APPLY_SUSC) {
        if (add) {
            SET_BIT(ch->susc, mod);
        } else {
            REMOVE_BIT(ch->susc, mod);
        }
    } else if (loc == APPLY_M_IMMUNE) {
        if (add) {
            SET_BIT(ch->M_immune, mod);
        } else {
            REMOVE_BIT(ch->M_immune, mod);
        }
    } else if (loc == APPLY_SPELL) {
        if (add) {
            SET_BIT(ch->specials.affected_by, mod);
        } else {
            REMOVE_BIT(ch->specials.affected_by, mod);
        }
    } else if (loc == APPLY_WEAPON_SPELL) {
        return;
    } else if (loc == APPLY_SPELL2 || loc == APPLY_BV2) {
        if (add) {
            /*
             * do em both? this creates odd combinations, may not be the
             * solution 
             */
            SET_BIT(ch->specials.affected_by2, bitv);
            /* 
             * mod takes care of 
             * the spells from items 
             */
        } else {
            REMOVE_BIT(ch->specials.affected_by2, bitv);
        }
        return;
    } else if (add) {
        SET_BIT(ch->specials.affected_by, bitv);
        if (IS_SET(ch->specials.act, PLR_NOFLY) && 
            IS_AFFECTED(ch, AFF_FLYING)) {
            REMOVE_BIT(ch->specials.affected_by, AFF_FLYING);
        }
    } else {
        REMOVE_BIT(ch->specials.affected_by, bitv);
        mod = -mod;
    }
   
    /*
     * This is for affects to apply to mobs, like the affects from blind,
     * and blast
     */
    if (!IS_PC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        switch (loc)
        {
        case APPLY_AC:
            GET_AC(ch) += mod;
            break;
        case APPLY_HITROLL:
            GET_HITROLL(ch) += mod;
            break;
        case APPLY_DAMROLL:
            GET_DAMROLL(ch) += mod;
            break;

            /*
             * negatives make saving throws better 
             */

        case APPLY_SAVING_PARA:
            ch->specials.apply_saving_throw[0] += mod;
            break;

        case APPLY_SAVING_ROD:
            ch->specials.apply_saving_throw[1] += mod;
            break;

        case APPLY_SAVING_PETRI:
            ch->specials.apply_saving_throw[2] += mod;
            break;

        case APPLY_SAVING_BREATH:
            ch->specials.apply_saving_throw[3] += mod;
            break;

        case APPLY_SAVING_SPELL:
            ch->specials.apply_saving_throw[4] += mod;
            break;

        case APPLY_SAVE_ALL:
            for (i = 0; i < MAX_SAVES; i++) {
                ch->specials.apply_saving_throw[i] += mod;
            }
            break;
        case APPLY_HITNDAM:
            GET_HITROLL(ch) += mod;
            GET_DAMROLL(ch) += mod;
            break;
        case APPLY_NONE:
            break;
        case APPLY_STR:

            /*
             * str greater than 18 ... 
             */
            if (GET_STR(ch) >= 18) {
                temp = GET_ADD(ch) + (mod * 10);
                if (temp > 100 && (GET_STR(ch) + 1 <= MaxStrForRace(ch))) {
                    /*
                     * subtract an extra 10 to account for the +1 
                     */
                    GET_STR(ch) += 1;
                    GET_ADD(ch) = (temp - 110);
                } else if (temp > 100) {
                    GET_ADD(ch) = 100;
                } else if (temp < 0 && (GET_STR(ch) - 1) < 18) {
                    /*
                     * adding a negative number 
                     */
                    GET_STR(ch) += (temp / 10);
                    GET_ADD(ch) = 0;
                } else if (temp < 0) {
                    /*
                     * add 10 to temp to account for the -1 here 
                     */
                    GET_STR(ch) -= 1;
                    /*
                     * eg: 19/10 mod(-3) == 18/90 again with the negative
                     * adding ;) 
                     */
                    GET_ADD(ch) = 110 + temp;
                } else {
                    GET_ADD(ch) = temp;
                }
            }
            /*
             * str less than 18 ... 
             */
            else {
                temp2 = GET_STR(ch) + mod;
                temp = GET_ADD(ch);
                if (temp2 >= 18 && 18 <= MaxStrForRace(ch)) {
                    temp = GET_ADD(ch) + ((temp2 - 18) * 10);
                    temp2 = 18;
                    if (temp > 100 && (temp2 + 1) <= MaxStrForRace(ch)) {
                        GET_STR(ch) = temp2 + 1;
                        GET_ADD(ch) = (temp - 110);
                    } else if (temp > 100) {
                        GET_STR(ch) = temp2;
                        GET_ADD(ch) = 100;
                        /*
                         * really shouldn't get here 
                         */
                    } else if (temp < 0) {
                        /*
                         * adding a negative number 
                         */
                        GET_STR(ch) = temp2 + (temp / 10);
                        GET_ADD(ch) = 0;
                    } else {
                        GET_STR(ch) = temp2;
                        GET_ADD(ch) = temp;
                    }
                } else if (temp2 >= MaxStrForRace(ch)) {
                    GET_STR(ch) = MaxStrForRace(ch);
                } else {
                    GET_STR(ch) = temp2;
                }
            }
            break;

        case APPLY_DEX:
        case APPLY_INT:
        case APPLY_WIS:
        case APPLY_CON:
        case APPLY_SEX:
        case APPLY_CHR:
        case APPLY_LEVEL:
        case APPLY_AGE:
        case APPLY_CHAR_WEIGHT:
        case APPLY_CHAR_HEIGHT:
        case APPLY_MANA:
        case APPLY_HIT:
        case APPLY_MOVE:
        case APPLY_GOLD:
        case APPLY_IMMUNE:
        case APPLY_SUSC:
        case APPLY_M_IMMUNE:
        case APPLY_SPELL:
        case APPLY_SPELL2:
        case APPLY_WEAPON_SPELL:
        case APPLY_EAT_SPELL:
        case APPLY_BACKSTAB:
        case APPLY_KICK:
        case APPLY_SNEAK:
        case APPLY_HIDE:
        case APPLY_BASH:
        case APPLY_PICK:
        case APPLY_STEAL:
        case APPLY_TRACK:
        case APPLY_SPELLFAIL:
        case APPLY_HASTE:
        case APPLY_SLOW:
        case APPLY_ATTACKS:
        case APPLY_FIND_TRAPS:
        case APPLY_RIDE:
        case APPLY_RACE_SLAYER:
        case APPLY_ALIGN_SLAYER:
        case APPLY_MANA_REGEN:
        case APPLY_HIT_REGEN:
        case APPLY_MOVE_REGEN:
        case APPLY_MOD_THIRST:
        case APPLY_MOD_HUNGER:
        case APPLY_MOD_DRUNK:
        case APPLY_T_STR:
        case APPLY_T_INT:
        case APPLY_T_DEX:
        case APPLY_T_WIS:
        case APPLY_T_CON:
        case APPLY_T_CHR:
        case APPLY_T_HPS:
        case APPLY_T_MOVE:
        case APPLY_T_MANA:
            break;

            break;
        default:

#ifdef LOG_DEBUG
            Log("Unknown apply adjust attempt on a mob in (handler.c, "
                "affect_modify).");
#endif
            break;

        }
    }
    /*
     * switch 
     */
    if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
        switch (loc) {
        case APPLY_NONE:
            break;

        case APPLY_STR:

            /*
             * str greater than 18 ... 
             */
            if (GET_STR(ch) >= 18) {
                temp = GET_ADD(ch) + (mod * 10);
                if (temp > 100 && (GET_STR(ch) + 1 <= MaxStrForRace(ch))) {
                    /*
                     * subtract an extra 10 to account for the +1 
                     */
                    GET_STR(ch) += 1;
                    GET_ADD(ch) = (temp - 110);
                } else if (temp > 100) {
                    GET_ADD(ch) = 100;
                } else if (temp < 0 && (GET_STR(ch) - 1) < 18) {
                    /*
                     * adding a negative number 
                     */
                    GET_STR(ch) += (temp / 10);
                    GET_ADD(ch) = 0;
                } else if (temp < 0) {
                    /*
                     * add 10 to temp to account for the -1 here 
                     */
                    GET_STR(ch) -= 1;
                    /*
                     * eg: 19/10 mod(-3) == 18/90 again with the negative
                     * adding ;) 
                     */
                    GET_ADD(ch) = 110 + temp;
                } else {
                    GET_ADD(ch) = temp;
                }
            }
            /*
             * str less than 18 ... 
             */
            else {
                temp2 = GET_STR(ch) + mod;
                temp = GET_ADD(ch);
                if (temp2 >= 18 && 18 <= MaxStrForRace(ch)) {
                    temp = GET_ADD(ch) + ((temp2 - 18) * 10);
                    temp2 = 18;
                    if (temp > 100 && (temp2 + 1) <= MaxStrForRace(ch)) {
                        GET_STR(ch) = temp2 + 1;
                        GET_ADD(ch) = (temp - 110);
                    } else if (temp > 100) {
                        GET_STR(ch) = temp2;
                        GET_ADD(ch) = 100;
                        /*
                         * really shouldn't get here 
                         */
                    } else if (temp < 0) {
                        /*
                         * adding a negative number 
                         */
                        GET_STR(ch) = temp2 + (temp / 10);
                        GET_ADD(ch) = 0;
                    } else {
                        GET_STR(ch) = temp2;
                        GET_ADD(ch) = temp;
                    }
                } else if (temp2 >= MaxStrForRace(ch)) {
                    GET_STR(ch) = MaxStrForRace(ch);
                } else {
                    GET_STR(ch) = temp2;
                }
            }
            break;

        case APPLY_DEX:
            temp = GET_DEX(ch);
            GET_DEX(ch) += mod;
            temp2 = GET_DEX(ch);

            if(temp > MaxDexForRace(ch)) {
                temp=MaxDexForRace(ch);
            }
            if(GET_DEX(ch) > MaxDexForRace(ch)) {
                GET_DEX(ch)=MaxDexForRace(ch);
            }

#if 0
            /* Temporary for debugging */
            Log("%s AC = %d, temp = %d, new dex = %d, -(%d) +(%d)",
                GET_NAME(ch), GET_AC(ch), temp, GET_DEX(ch),
                dex_app[temp].defensive,
                dex_app[(int)GET_DEX(ch)].defensive );
#endif

#if 0
            GET_AC(ch) -= dex_app[temp].defensive;
            GET_AC(ch) += dex_app[(int)GET_DEX(ch)].defensive; 

            /*
             * I think dex items cause this to wack peoples AC out... msw 
             */
            if (GET_AC(ch) < -100)
                GET_AC(ch) = -100;
            if (GET_AC(ch) > 100)
                GET_AC(ch) = 100;
#endif

            break;

        case APPLY_INT:
            GET_INT(ch) += mod;
            if (GET_INT(ch) > MaxIntForRace(ch)) {
                GET_INT(ch) = MaxIntForRace(ch);
            }
            break;

        case APPLY_WIS:
            GET_WIS(ch) += mod;
            if (GET_WIS(ch) > MaxWisForRace(ch)) {
                GET_WIS(ch) = MaxWisForRace(ch);
            }
            break;

        case APPLY_CON:
            GET_CON(ch) += mod;
            if (GET_CON(ch) > MaxConForRace(ch)) {
                GET_CON(ch) = MaxConForRace(ch);
            }
            break;

        case APPLY_SEX:
            GET_SEX(ch) = (!(ch->player.sex - 1)) + 1;
            break;

        case APPLY_CHR:
            GET_CHR(ch) += mod;
            if (GET_CHR(ch) > MaxChrForRace(ch)) {
                GET_CHR(ch) = MaxChrForRace(ch);
            }
            break;

        case APPLY_LEVEL:
            break;

        case APPLY_AGE:
            ch->player.time.birth -= SECS_PER_MUD_YEAR * mod;
            break;

        case APPLY_CHAR_WEIGHT:
            GET_WEIGHT(ch) += mod;
            break;

        case APPLY_CHAR_HEIGHT:
            GET_HEIGHT(ch) += mod;
            break;

        case APPLY_MANA:
            ch->points.max_mana += mod;
            break;

        case APPLY_HIT:
            ch->points.max_hit += mod;
            break;

        case APPLY_MOVE:
            ch->points.max_move += mod;
            break;

        case APPLY_GOLD:
            break;

        case APPLY_AC:
            GET_AC(ch) += mod;
            break;

        case APPLY_HITROLL:
            GET_HITROLL(ch) += mod;
            break;

        case APPLY_DAMROLL:
            GET_DAMROLL(ch) += mod;
            break;

            /*
             * negatives make saving throws better 
             */

        case APPLY_SAVING_PARA:
            ch->specials.apply_saving_throw[0] += mod;
            break;

        case APPLY_SAVING_ROD:
            ch->specials.apply_saving_throw[1] += mod;
            break;

        case APPLY_SAVING_PETRI:
            ch->specials.apply_saving_throw[2] += mod;
            break;

        case APPLY_SAVING_BREATH:
            ch->specials.apply_saving_throw[3] += mod;
            break;

        case APPLY_SAVING_SPELL:
            ch->specials.apply_saving_throw[4] += mod;
            break;

        case APPLY_SAVE_ALL:{
                for (i = 0; i < MAX_SAVES; i++)
                    ch->specials.apply_saving_throw[i] += mod;
            }
            break;
        case APPLY_IMMUNE:
            break;
        case APPLY_SUSC:
            break;
        case APPLY_M_IMMUNE:
            break;
        case APPLY_SPELL:
        case APPLY_SPELL2:
            break;
        case APPLY_HITNDAM:
            GET_HITROLL(ch) += mod;
            GET_DAMROLL(ch) += mod;
            break;
        case APPLY_WEAPON_SPELL:
        case APPLY_EAT_SPELL:
            break;
        case APPLY_BACKSTAB:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_BACKSTAB].learned + mod >= 100) {
                ch->skills[SKILL_BACKSTAB].learned = 100;
            } else {
                ch->skills[SKILL_BACKSTAB].learned += mod;
            }
            break;
        case APPLY_KICK:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_KICK].learned + mod >= 100) {
                ch->skills[SKILL_KICK].learned = 100;
            } else {
                ch->skills[SKILL_KICK].learned += mod;
            }
            break;
        case APPLY_SNEAK:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_SNEAK].learned + mod >= 100) {
                ch->skills[SKILL_SNEAK].learned = 100;
            } else {
                ch->skills[SKILL_SNEAK].learned += mod;
            }
            break;
        case APPLY_HIDE:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_HIDE].learned + mod >= 100) {
                ch->skills[SKILL_HIDE].learned = 100;
            } else {
                ch->skills[SKILL_HIDE].learned += mod;
            }
            break;
        case APPLY_BASH:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_BASH].learned + mod >= 100) {
                ch->skills[SKILL_BASH].learned = 100;
            } else {
                ch->skills[SKILL_BASH].learned += mod;
            }
            break;
        case APPLY_PICK:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_PICK_LOCK].learned + mod >= 100) {
                ch->skills[SKILL_PICK_LOCK].learned = 100;
            } else {
                ch->skills[SKILL_PICK_LOCK].learned += mod;
            }
            break;
        case APPLY_STEAL:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_STEAL].learned + mod >= 100) {
                ch->skills[SKILL_STEAL].learned = 100;
            } else {
                ch->skills[SKILL_STEAL].learned += mod;
            }
            break;
        case APPLY_TRACK:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_HUNT].learned + mod >= 100) {
                ch->skills[SKILL_HUNT].learned = 100;
            } else {
                ch->skills[SKILL_HUNT].learned += mod;
            }
            break;

        case APPLY_SPELLFAIL:
            ch->specials.spellfail += mod;
            break;

        case APPLY_HASTE:
            if (mod > 0) {
                SET_BIT(ch->specials.affected_by2, AFF2_HASTE);
            } else if (mod < 0) {
                REMOVE_BIT(ch->specials.affected_by2, AFF2_HASTE);
            }
            break;

        case APPLY_SLOW:
            if (mod > 0) {
                SET_BIT(ch->specials.affected_by2, AFF2_SLOW);
            } else if (mod < 0) {
                REMOVE_BIT(ch->specials.affected_by2, AFF2_SLOW);
            }
            break;

        case APPLY_ATTACKS:
            break;

        case APPLY_FIND_TRAPS:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_FIND_TRAP].learned + mod >= 100) {
                ch->skills[SKILL_FIND_TRAP].learned = 100;
            } else {
                ch->skills[SKILL_FIND_TRAP].learned += mod;
            }
            break;

        case APPLY_RIDE:
            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (ch->skills[SKILL_RIDE].learned + mod >= 100) {
                ch->skills[SKILL_RIDE].learned = 100;
            } else {
                ch->skills[SKILL_RIDE].learned += mod;
            }
            break;

        case APPLY_RACE_SLAYER:
        case APPLY_ALIGN_SLAYER:
            break;

        case APPLY_MANA_REGEN:
            ch->points.mana_gain += mod;
            break;

        case APPLY_HIT_REGEN:
            ch->points.hit_gain += mod;
            break;

        case APPLY_MOVE_REGEN:
            ch->points.move_gain += mod;
            break;

            /*
             * set hunger/thirst/drunk to MOD value 
             */

        case APPLY_MOD_THIRST:
            ch->specials.conditions[THIRST] = mod;
            break;
        case APPLY_MOD_HUNGER:
            ch->specials.conditions[FULL] = mod;
            break;
        case APPLY_MOD_DRUNK:
            ch->specials.conditions[DRUNK] = mod;
            break;

            /*
             * set these attribs to the mod givin the attrib setting the
             * VALUE, not mod 
             */
        case APPLY_T_STR:
        case APPLY_T_INT:
        case APPLY_T_DEX:
        case APPLY_T_WIS:
        case APPLY_T_CON:
        case APPLY_T_CHR:
        case APPLY_T_HPS:
        case APPLY_T_MOVE:
        case APPLY_T_MANA:
            break;

            break;
        default:

#ifdef LOG_DEBUG
            Log("Unknown apply adjust attempt (handler.c, affect_modify).  %s",
                GET_NAME(ch));
#endif
            break;

        }
    }
}

/*
 * This updates a character by subtracting everything he is affected by 
 */
/*
 * restoring original abilities, and then affecting all again 
 */
void affect_total(struct char_data *ch)
{
    struct affected_type *af;
    int             i,
                    j;

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                affect_modify(ch, ch->equipment[i]->affected[j].location,
                              ch->equipment[i]->affected[j].modifier,
                              ch->equipment[i]->bitvector, FALSE);
            }
        }
    }

    for (af = ch->affected; af; af = af->next) {
        affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
    }

    ch->tmpabilities = ch->abilities;

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                affect_modify(ch, ch->equipment[i]->affected[j].location,
                              ch->equipment[i]->affected[j].modifier,
                              ch->equipment[i]->bitvector, TRUE);
            }
        }
    }

    for (af = ch->affected; af; af = af->next) {
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    }

    /*
     * Make certain values are between 0..25, not < 0 and not > 25! 
     */

    i = (IS_NPC(ch) || GetMaxLevel(ch) >= IMPLEMENTOR ? 25 : 18);

    if (IS_NPC(ch) || IS_IMMORTAL(ch)) {
        GET_DEX(ch) = MAX(3, MIN(GET_DEX(ch), 25));
        GET_INT(ch) = MAX(3, MIN(GET_INT(ch), 25));
        GET_WIS(ch) = MAX(3, MIN(GET_WIS(ch), 25));
        GET_CON(ch) = MAX(3, MIN(GET_CON(ch), 25));
        GET_STR(ch) = MAX(3, GET_STR(ch));
        GET_CHR(ch) = MAX(3, MIN(GET_CHR(ch), 25));
    } else {
        GET_DEX(ch) = MAX(3, MIN(GET_DEX(ch), MaxDexForRace(ch)));
        GET_INT(ch) = MAX(3, MIN(GET_INT(ch), MaxIntForRace(ch)));
        GET_WIS(ch) = MAX(3, MIN(GET_WIS(ch), MaxWisForRace(ch)));
        GET_CON(ch) = MAX(3, MIN(GET_CON(ch), MaxConForRace(ch)));
        GET_STR(ch) = MAX(3, GET_STR(ch));
        GET_CHR(ch) = MAX(3, MIN(GET_CHR(ch), MaxChrForRace(ch)));
    }
    if (IS_NPC(ch) || GetMaxLevel(ch) >= IMPLEMENTOR) {
        GET_STR(ch) = MIN(GET_STR(ch), i);
    } else if (GET_STR(ch) > 18) {
        i = GET_ADD(ch) + ((GET_STR(ch) - 18) * 10);
        GET_ADD(ch) = MIN(i, 100);
        if (GET_STR(ch) > MaxStrForRace(ch)) {
            GET_STR(ch) = MaxStrForRace(ch);
        }
    }
}

/*
 * Insert an affect_type in a char_data structure Automatically sets
 * apropriate bits and apply's 
 */
void affect_to_char(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *affected_alloc;

    if (!af) {
        Log("!af in affect_to_char");
        return;
    }

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);

    affect_total(ch);
}

/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL! Frees mem and calls
 * affect_location_apply 
 */

void affect_remove(struct char_data *ch, struct affected_type *af)
{
    struct affected_type *hjp;

    if (!ch->affected) {
        Log("affect removed from char without affect - %s", GET_NAME(ch));
        return;
    }

    /*
     * Curious to see if af is really non NULL 
     */
    if (!af) {
        Log("WARNING!!!! af is NULL! in affect_remove()");
        return;
    }

    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);

    /*
     * remove structure *af from linked list 
     */

    if (ch->affected == af) {
        /*
         * remove head of list 
         */
        ch->affected = af->next;
    } else {
        for (hjp = ch->affected; (hjp) && (hjp->next) && (hjp->next != af);
             hjp = hjp->next) {
            /* 
             * Empty loop 
             */
        }

        if (hjp->next != af) {
            Log("Could not locate affected_type in ch->affected. (handler.c, "
                "affect_remove)");
            return;
        }
        hjp->next = af->next;
    }

    if (af) {
        free(af);
        af = NULL;
    }
    affect_total(ch);
}

/*
 * Call affect_remove with every spell of spelltype "skill" 
 */
void affect_from_char(struct char_data *ch, int skill)
{
    struct affected_type *hjp,
                   *hjp1;

    for (hjp1 = hjp = ch->affected; hjp; hjp = hjp1) {
        hjp1 = hjp->next;
        if (hjp->type == skill) {
            affect_remove(ch, hjp);
        }
    }
}

/*
 * Return if a char is affected by a spell (SPELL_XXX), NULL indicates not 
 * affected 
 */
bool affected_by_spell(struct char_data *ch, int skill)
{
    struct affected_type *hjp,
                   *old_af;

    for (hjp = ch->affected; hjp; old_af = hjp, hjp = hjp->next) {
        if (!hjp) {
            return (FALSE);
        }
        if (hjp->type) {
            if (hjp->type == skill) {
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

void affect_join(struct char_data *ch, struct affected_type *af,
                 bool avg_dur, bool avg_mod)
{
    struct affected_type *hjp;
    bool            found = FALSE;

    for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
        if (hjp->type == af->type) {

            af->duration += hjp->duration;
            if (avg_dur) {
                af->duration /= 2;
            }
            af->modifier += hjp->modifier;
            if (avg_mod) {
                af->modifier /= 2;
            }
            affect_remove(ch, hjp);
            affect_to_char(ch, af);
            found = TRUE;
        }
    }
    if (!found) {
        affect_to_char(ch, af);
    }
}

/*
 * move a player out of a room 
 */
void char_from_room(struct char_data *ch)
{
    struct char_data *i;
    struct room_data *rp;

    if (ch->in_room == NOWHERE) {
        Log("NOWHERE extracting char from room (handler.c, char_from_room)");
        return;
    }

    if (ch->equipment[WEAR_LIGHT] &&
        ch->equipment[WEAR_LIGHT]->type_flag == ITEM_TYPE_LIGHT && 
        ch->equipment[WEAR_LIGHT]->value[2]) {
        /* 
         * Light is ON 
         */
        real_roomp(ch->in_room)->light--;
    }

    rp = real_roomp(ch->in_room);
    if (rp == NULL) {
        Log("ERROR: char_from_room: %s was not in a valid room (%ld)",
            (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
             ch->in_room);
        return;
    }

    if (ch == rp->people) {
        /* 
         * head of list 
         */
        rp->people = ch->next_in_room;
    } else {
        /* 
         * locate the previous element 
         */
        for (i = rp->people; i && i->next_in_room != ch;
             i = i->next_in_room) {
            /* 
             * Empty loop 
             */
        }
        if (i) {
            i->next_in_room = ch->next_in_room;
        } else {
            Log("Oops! %s was not in people list of his room %ld!",
                (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr),
                ch->in_room);
        }
    }

    ch->in_room = NOWHERE;
    ch->next_in_room = 0;
}

/*
 * place a character in a room 
 */
void char_to_room(struct char_data *ch, long room)
{
    struct room_data *rp;
    extern int      pulse;

    rp = real_roomp(room);
    if (!rp) {
        room = 0;
        rp = real_roomp(room);
        if (!rp) {
            assert(0);
        }
    }
    ch->next_in_room = rp->people;
    rp->people = ch;
    ch->in_room = room;

    if (ch->equipment[WEAR_LIGHT] && 
        ch->equipment[WEAR_LIGHT]->type_flag == ITEM_TYPE_LIGHT) {
        if (rp->sector_type != SECT_UNDERWATER) {
            if (ch->equipment[WEAR_LIGHT]->value[2]) {
                /* 
                 * Light is ON 
                 */
                rp->light++;
            }
            if (rp->light < 1) {
                rp->light = 1;
            }
        } else if (ch->equipment[WEAR_LIGHT]->value[2] > 0) {
            send_to_char("Your light source is extinguished instantly!\n\r",
                         ch);
            ch->equipment[WEAR_LIGHT]->value[2] = 0;
        } else {
            rp->light++;
            if (rp->light < 1) {
                rp->light = 1;
            }
        }
    }

    if (IS_AFFECTED(ch, AFF_FIRESHIELD) && rp->sector_type == SECT_UNDERWATER) {
        affect_from_char(ch, AFF_FIRESHIELD);
#if 0        
        send_to_char("When it touches water, your fireshield is "
                     "extinguished!\n\r", ch);
#endif
    }

    if (IS_PC(ch)) {
        if (rp->tele_cnt > 0 && rp->tele_time == 0) {
            /*
             * this is a teleport countdown room 
             */
            /* 
             * now round up 
             */
            rp->tele_time = pulse + rp->tele_cnt;
            if (rp->tele_time % 10) {
                rp->tele_time += 10 - (rp->tele_time % 10);
            }

            if (rp->tele_time > 2400) { 
                rp->tele_time = rp->tele_cnt;   
                /* 
                 * start of next day 
                 */
            }
        }

        if (zone_table[rp->zone].start == 0) {
            /*
             * start up the zone. 
             */
            reset_zone(rp->zone, 0);
            if (rp->zone == STATUE_ZONE) {
                generate_legend_statue();
            }
        }
    }
}


/*
 * Return the effect of a piece of armor in position eq_pos 
 */
int apply_ac(struct char_data *ch, int eq_pos)
{
    assert(ch->equipment[eq_pos]);

    if (!(ITEM_TYPE(ch->equipment[eq_pos]) == ITEM_TYPE_ARMOR)) {
        return 0;
    }
    switch (eq_pos) {

    case WEAR_BODY:
        /* 30% */
        return (3 * ch->equipment[eq_pos]->value[0]);
    case WEAR_HEAD:
        /* 20% */
        return (2 * ch->equipment[eq_pos]->value[0]);
    case WEAR_LEGS:
        /* 20% */
        return (2 * ch->equipment[eq_pos]->value[0]);
    case WEAR_FEET:
        /* 10% */
        return (ch->equipment[eq_pos]->value[0]);
    case WEAR_HANDS:
        /* 10% */
        return (ch->equipment[eq_pos]->value[0]);
    case WEAR_ARMS:
        /* 10% */
        return (ch->equipment[eq_pos]->value[0]);
    case WEAR_SHIELD:
        /* 10% */
        return (ch->equipment[eq_pos]->value[0]);
    case WEAR_ABOUT:
        /* 10% */
        return (ch->equipment[eq_pos]->value[0]);
    }
    return 0;
}

void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int             j;
    struct index_data *index;

    if (pos < 0 || pos > MAX_WEAR) {
        Log("wear pos > MAX_WEAR or < 0 in handler.c");
        return;
    }

    /*
     * assert(pos>=0 && pos<MAX_WEAR); 
     */
    assert(!(ch->equipment[pos]));

    if (obj->carried_by) {
        Log("EQUIP: Obj is carried_by when equip.");
        assert(0);
    }

    if (obj->in_room != NOWHERE) {
        Log("EQUIP: Obj is in_room when equip.");
        assert(0);
        return;
    }

    if (ch->in_room != NOWHERE) {
        /*
         * no checks on super ego items, they do it already 
         */
        if (obj->index->func != EvilBlade && obj->index->func != NeutralBlade &&
            obj->index->func != GoodBlade && !CheckEgo(ch, obj)) {
            if (ch->in_room != NOWHERE) {
                objectPutInRoom(obj, ch->in_room);
                do_save(ch, "", 0);
            } else {
                Log("Ch->in_room = NOWHERE on anti-ego item!");
            }
            return;
        }

        if (!CheckGetBarbarianOK(ch, obj)) {
            if (ch->in_room != NOWHERE) {
                objectPutInRoom(obj, ch->in_room);
                do_save(ch, "", 0);
                return;
            } else {
                Log("Ch->in_room = NOWHERE on anti-barb item!");
            }
            return;
        }

        if (ItemAlignClash(ch, obj) && (GetMaxLevel(ch) < IMPLEMENTOR)) {
            if (ch->in_room != NOWHERE) {
                act("You are zapped by $p and instantly drop it.",
                    FALSE, ch, obj, 0, TO_CHAR);
                act("$n is zapped by $p and instantly drops it.",
                    FALSE, ch, obj, 0, TO_ROOM);
                objectPutInRoom(obj, ch->in_room);
                do_save(ch, "", 0);
                return;
            } else {
                Log("ch->in_room = NOWHERE when equipping char.");
                assert(0);
            }
        }
    }
    if (IS_AFFECTED(ch, AFF_SNEAK) &&
        IsRestricted(GetItemClassRestrictions(obj), CLASS_THIEF)) {
        affect_from_char(ch, SKILL_SNEAK);
    }

    ch->equipment[pos] = obj;
    obj->equipped_by = ch;
    obj->eq_pos = pos;

    if (ITEM_TYPE(obj) == ITEM_TYPE_ARMOR) {
        GET_AC(ch) -= apply_ac(ch, pos);
    }
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        affect_modify(ch, obj->affected[j].location,
                      obj->affected[j].modifier,
                      obj->bitvector, TRUE);
    }

    if (ITEM_TYPE(obj) == ITEM_TYPE_WEAPON) {
        /*
         * some nifty manuevering for strength 
         */
        if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
            GiveMinStrToWield(obj, ch);
        }
    }

    affect_total(ch);
}

int GiveMinStrToWield(struct obj_data *obj, struct char_data *ch)
{
    int             str = 0;

    GET_STR(ch) = 16;           /* nice, semi-reasonable start */
    /*
     * will have a problem with except. str, that i do not care to solve 
     */

    while (GET_OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
        GET_STR(ch)++;

    return (str);

}

struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int             j;
    struct obj_data *obj;

    if (pos > MAX_WEAR || pos < 0) {
        Log("pos > MAX_WEAR || pos < 0 in handler.c unequip");
        return (NULL);
    }

    /*
     * assert(pos>=0 && pos<MAX_WEAR); 
     */

    assert(ch->equipment[pos]);

    obj = ch->equipment[pos];

    assert(!obj->in_obj && obj->in_room == NOWHERE && !obj->carried_by);

    if (ITEM_TYPE(obj) == ITEM_TYPE_ARMOR) {
        GET_AC(ch) += apply_ac(ch, pos);
    }
    ch->equipment[pos] = 0;
    obj->equipped_by = 0;
    obj->eq_pos = -1;

    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        affect_modify(ch, obj->affected[j].location,
                      obj->affected[j].modifier, obj->bitvector,
                      FALSE);
    }
    affect_total(ch);

    return (obj);
}


/*
 * search a room for a char, and return a pointer if found..  
 */
struct char_data *get_char_room(char *name, int room)
{
    struct char_data *i;
    int             j,
                    number;
    char            tmpname[MAX_INPUT_LENGTH + 40];
    char           *tmp;
    struct room_data *rp;

    if( !name || !(rp = real_roomp(room)) ) {
        return( NULL );
    }

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp))) {
        return (NULL);
    }
    for (i = rp->people, j = 1; i && (j <= number);
         i = i->next_in_room) {
        if (isname(tmp, GET_NAME(i))) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }

    for (i = rp->people, j = 1; i && (j <= number);
         i = i->next_in_room) {
        if (isname2(tmp, GET_NAME(i))) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }

    return (NULL);
}

/*
 * search all over the world for a char, and return a pointer if found 
 */
struct char_data *get_char(char *name)
{
    struct char_data *i;
    int             j,
                    number;
    char            tmpname[MAX_INPUT_LENGTH + 40];
    char           *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp))) {
        return (0);
    }
    for (i = character_list, j = 1; i && (j <= number); i = i->next) {
        if (isname(tmp, GET_NAME(i))) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }
    for (i = character_list, j = 1; i && (j <= number); i = i->next) {
        if (isname2(tmp, GET_NAME(i))) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }
    return (0);
}

/*
 * search all over the world for a char num, and return a pointer if found 
 */
struct char_data *get_char_num(int nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next) {
        if (i->nr == nr) {
            return (i);
        }
    }
    return (0);
}

/**
 * @todo this uses object_list-like behavior and needs rethinking
 * Set all carried_by to point to new owner 
 */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{
    if (list) {
        object_list_new_owner(list->contains, ch);
        object_list_new_owner(list->next_content, ch);
        list->carried_by = ch;
    }
}


void update_object(struct obj_data *obj, int use)
{
    if (obj->timer > 0) {
        obj->timer -= use;
    }
    if (obj->contains) {
        update_object(obj->contains, use);
    }
    if (obj->next_content && obj->next_content != obj) {
        update_object(obj->next_content, use);
    }
}

void update_char_objects(struct char_data *ch)
{
    int             i;

    if (ch->equipment[WEAR_LIGHT] && 
        ch->equipment[WEAR_LIGHT]->type_flag == ITEM_TYPE_LIGHT &&
        ch->equipment[WEAR_LIGHT]->value[2] > 0) {
        (ch->equipment[WEAR_LIGHT]->value[2])--;
    }

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            update_object(ch->equipment[i], 2);
        }
    }

    if (ch->carrying) {
        update_object(ch->carrying, 1);
    }
}

/**
 * @todo WTF is this for?
 */
void extract_char(struct char_data *ch)
{
    extract_char_smarter(ch, NOWHERE);
}

/*
 * Extract a ch completely from the world, and leave his stuff behind 
 */

void extract_char_smarter(struct char_data *ch, long saveroom)
{
    struct obj_data *i;
    struct char_data *k,
                   *next_char;
    struct descriptor_data *t_desc;
    int             l,
                    was_in,
                    j;

    extern long     mob_count;
    extern struct char_data *combat_list;

    if (!IS_NPC(ch) && !ch->desc) {
        for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next) {
            if (t_desc->original == ch) {
                do_return(t_desc->character, "", 0);
            }
        }
    }

    if (ch->in_room == NOWHERE) {
#if 0        
        Log("NOWHERE extracting char. (handler.c, extract_char)");
        /*
         * problem from linkdeath
         */
#endif        
        char_to_room(ch, 4);    
        /* 
         * 4 == all purpose store 
         */
    }

    if (ch->followers || ch->master) {
        die_follower(ch);
    }
    if (ch->desc) {
        /*
         * Forget snooping 
         */
        if ((ch->desc->snoop.snooping) && (ch->desc->snoop.snooping->desc)) {
            ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
        }
        if (ch->desc->snoop.snoop_by) {
            send_to_char("Your victim is no longer among us.\n\r",
                         ch->desc->snoop.snoop_by);
            if (ch->desc->snoop.snoop_by->desc) {
                ch->desc->snoop.snoop_by->desc->snoop.snooping = 0;
            }
        }

        ch->desc->snoop.snooping = ch->desc->snoop.snoop_by = 0;
    }

    if (ch->carrying) {
        /*
         * transfer ch's objects to room 
         */

        if (!IS_IMMORTAL(ch)) {
            while (ch->carrying) {
                i = ch->carrying;
                objectTakeFromChar(i);
                objectPutInRoom(i, ch->in_room);
                check_falling_obj(i, ch->in_room);
            }
        } else {
            send_to_char("Here, you dropped some stuff, let me help you get"
                         " rid of that.\n\r", ch);

            /*
             * equipment too 
             */
            for (j = 0; j < MAX_WEAR; j++) {
                if (ch->equipment[j]) {
                    objectGiveToChar(unequip_char(ch, j), ch);
                }
            }

            while (ch->carrying) {
                i = ch->carrying;
                objectTakeFromChar(i);
                objectExtract(i);
            }
        }

    }

    if (ch->specials.fighting) {
        stop_fighting(ch);
    }
    for (k = combat_list; k; k = next_char) {
        next_char = k->next_fighting;
        if (k->specials.fighting == ch) {
            stop_fighting(k);
        }
    }

    if (MOUNTED(ch)) {
        Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }

    if (RIDDEN(ch)) {
        Dismount(RIDDEN(ch), ch, POSITION_STANDING);
    }

    /*
     * Must remove from room before removing the equipment! 
     */
    was_in = ch->in_room;
    char_from_room(ch);

    /*
     * clear equipment_list 
     */
    for (l = 0; l < MAX_WEAR; l++) {
        if (ch->equipment[l]) {
            objectPutInRoom(unequip_char(ch, l), was_in);
        }
    }

    if (IS_NPC(ch)) {
        for (k = character_list; k; k = k->next) {
            if (k->specials.hunting) {
                if (k->specials.hunting == ch) {
                    k->specials.hunting = 0;
                }
            }
            if (Hates(k, ch)) {
                RemHated(k, ch);
            }
            if (Fears(k, ch)) {
                RemFeared(k, ch);
            }
            if (k->orig == ch) {
                k->orig = 0;
            }
        }
    } else {
        for (k = character_list; k; k = k->next) {
            if (k->specials.hunting && k->specials.hunting == ch) {
                k->specials.hunting = 0;
            }
            if (Hates(k, ch)) {
                ZeroHatred(k, ch);
            }
            if (Fears(k, ch)) {
                ZeroFeared(k, ch);
            }
            if (k->orig == ch) {
                k->orig = 0;
            }
        }

    }
    /*
     * pull the char from the list 
     */

    if (ch == character_list) {
        character_list = ch->next;
    } else {
        for (k = character_list; (k) && (k->next != ch); k = k->next) {
            /* 
             * Empty loop 
             */
        }
        if (k) {
            k->next = ch->next;
        } else {
            Log("Trying to remove ?? from character_list.(handler.c,"
                "extract_char)");
            exit(0);
        }
    }

    if (ch->specials.group_name) {
        free(ch->specials.group_name);
        ch->specials.group_name = 0;
    }
    GET_AC(ch) = 100;

    if (ch->desc) {
        if (ch->desc->original) {
            do_return(ch, "", 0);
        }

#if 0
        if (!strcmp(GET_NAME(ch), "Odin's heroic minion")) {
            free(GET_NAME(ch));
            GET_NAME(ch) = strdup("111111");
        }
#endif

        save_char(ch, saveroom);
    }

    t_desc = ch->desc;

    if (ch->term) {
        ScreenOff(ch);
        ch->term = 0;
    }

    if (IS_NPC(ch)) {
        if (ch->nr > -1) {
            /* 
             * if mobile 
             */
            mob_index[ch->nr].number--;
        }
        FreeHates(ch);
        FreeFears(ch);
        mob_count--;
        free_char(ch);
    }

    if (t_desc) {
        t_desc->connected = CON_SLCT;
        send_to_char(MENU, t_desc->character);
    }
}

/*
 ***********************************************************************
 Here follows high-level versions of some earlier routines, ie functionst
 which incorporate the actual player-data.
 *********************************************************************** */

struct char_data *get_char_near_room_vis(struct char_data *ch, char *name,
                                         long next_room)
{
    long            store_room;
    struct char_data *i;

    store_room = ch->in_room;
    ch->in_room = next_room;
    i = get_char_room_vis(ch, name);
    ch->in_room = store_room;

    return (i);
}

struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int             j,
                    number;
    char            tmpname[MAX_INPUT_LENGTH + 40];
    char           *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp))) {
        return (0);
    }
    for (i = real_roomp(ch->in_room)->people, j = 1;
         i && (j <= number); i = i->next_in_room) {
        if (isname(tmp, GET_NAME(i)) && CAN_SEE(ch, i)) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }

    for (i = real_roomp(ch->in_room)->people, j = 1;
         i && (j <= number); i = i->next_in_room) {
        if (isname2(tmp, GET_NAME(i)) && CAN_SEE(ch, i)) {
            if (j == number) {
                return (i);
            }
            j++;
        }
    }

    return (0);
}

/*
 * get a character from anywhere in the world, doesn't care much about
 * being in the same room... 
 */
struct char_data *get_char_vis_world(struct char_data *ch, char *name,
                                     int *count)
{
    struct char_data *i;
    int             j,
                    number;
    char            tmpname[MAX_INPUT_LENGTH + 40];
    char           *tmp;

    strcpy(tmpname, name);
    tmp = tmpname;
    if (!(number = get_number(&tmp))) {
        return (0);
    }
    j = count ? *count : 1;
    for (i = character_list; i && (j <= number); i = i->next) {
        if (isname(tmp, GET_NAME(i))) {
            if (CAN_SEE(ch, i)) {
                if (j == number) {
                    return (i);
                }
                j++;
            }
        }
    }
    j = count ? *count : 1;
    for (i = character_list; i && (j <= number); i = i->next) {
        if (isname2(tmp, GET_NAME(i))) {
            if (CAN_SEE(ch, i)) {
                if (j == number) {
                    return (i);
                }
                j++;
            }
        }
    }
    if (count) {
        *count = j;
    }
    return 0;
}

struct char_data *get_char_vis(struct char_data *ch, char *name)
{
    struct char_data *i;

    /*
     * check location 
     */
    if ((i = get_char_room_vis(ch, name))) {
        return (i);
    }
    return get_char_vis_world(ch, name, NULL);
}



struct obj_data *create_money(int amount)
{
    char            buf[MAX_STRING_LENGTH];
    struct obj_data *obj;

    if (amount <= 0) {
        Log("Create_money: zero or negative money. handler.c");
        amount = 1;
    }

    if (amount == 1) {
        obj = objectRead(OBJ_VNUM_MONEY_ONE);
    } else {
        obj = objectRead(OBJ_VNUM_MONEY_SOME);
        sprintf(buf, obj->short_description, amount);
        if (obj->short_description) {
            free(obj->short_description);
        }
        obj->short_description = (char *) strdup(buf);
        obj->value[0] = amount;
    }

    return obj;
}

/*
 * Generic Find, designed to find any object/character 
 * Calling : 
 * arg     is the sting containing the string to be searched for.
 *  This string doesn't have to be a single word, the routine 
 *  extracts the next word itself.  
 * bitv..  All those bits that you want to "search through".  
 *  Bit found will be result of the function 
 * ch      This is the person that is trying to "find" 
 * *tar_ch Will be NULL if no character was found, otherwise points
 * *tar_obj Will be NULL if no object was found, otherwise points 
 * The routine returns a pointer to the next word in *arg 
 */

int generic_find(char *arg, int bitvector, struct char_data *ch,
                 struct char_data **tar_ch, struct obj_data **tar_obj)
{
    static char    *ignore[] = {
        "the",
        "in",
        "on",
        "at",
        "\n"
    };

    int             i;
    char            name[256];
    bool            found;

    found = FALSE;

    /*
     * Eliminate spaces and "ignore" words 
     */
    while (arg && *arg && !found) {
        arg = skip_spaces(arg);
        if( !arg ) {
            break;
        }

        for (i = 0; (name[i] = arg[i]) && (name[i] != ' '); i++) {
            /* 
             * Empty loop 
             */
        }

        name[i] = 0;
        arg += i;
        if (search_block(name, ignore, TRUE) > -1) {
            found = TRUE;
        }
    }

    if (!name[0]) {
        return (0);
    }
    *tar_ch = 0;
    *tar_obj = 0;

    /* 
     * Find person in room 
     */
    if (IS_SET(bitvector, FIND_CHAR_ROOM) && 
        (*tar_ch = get_char_room_vis(ch, name))) {
        return (FIND_CHAR_ROOM);
    }

    if (IS_SET(bitvector, FIND_CHAR_WORLD) && 
        (*tar_ch = get_char_vis(ch, name))) {
        return (FIND_CHAR_WORLD);
    }

    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
        if( (*tar_obj = get_object_in_equip( ch, name, name, &i ) ) ) {
            return( FIND_OBJ_EQUIP );
        }
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)) {
        if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
            if ((*tar_obj = get_obj_vis_accessible(ch, name))) {
                return (FIND_OBJ_INV);
            }
        } else {
            if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
                return (FIND_OBJ_INV);
            }
        }
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM) &&
        (*tar_obj = get_obj_in_list_vis(ch, name,
                                        real_roomp(ch->in_room)->contents))) {
        return (FIND_OBJ_ROOM);
    }

    if (IS_SET(bitvector, FIND_OBJ_WORLD) && 
        (*tar_obj = get_obj_vis(ch, name))) {
        return (FIND_OBJ_WORLD);
    }

    return (0);
}

void AddAffects(struct char_data *ch, struct obj_data *o)
{
    int             i;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (o->affected[i].location != APPLY_NONE) {
            affect_modify(ch, o->affected[i].location, o->affected[i].modifier,
                          o->bitvector, TRUE);
        } else {
            return;
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
