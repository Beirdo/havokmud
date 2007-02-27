/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD 
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

/*
 * external stuff 
 */

void FreeHates(struct char_data *ch)
{
    struct char_list *k,
                   *n;

    for (k = ch->hates.clist; k; k = n) {
        n = k->next;
        if (k) {
            free(k);
        }
    }

}

void FreeFears(struct char_data *ch)
{
    struct char_list *k,
                   *n;

    for (k = ch->fears.clist; k; k = n) {
        n = k->next;
        if (k) {
            free(k);
        }
    }

}

int RemHated(struct char_data *ch, struct char_data *pud)
{
    struct char_list *oldpud,
                   *t;

    if (pud) {
        for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
            if (!oldpud) {
                return (FALSE);
            }
            if (oldpud->op_ch) {
                if (oldpud->op_ch == pud) {
                    t = oldpud;
                    if (ch->hates.clist == t) {
                        ch->hates.clist = 0;
                    } else {
                        for (oldpud = ch->hates.clist; oldpud->next != t;
                             oldpud = oldpud->next) {
                            /* 
                             * Empty loop 
                             */
                        }
                        oldpud->next = oldpud->next->next;
                    }
                    if (t) {
                        free(t);
                    }
                    break;
                }
            } else if (!strcmp(oldpud->name, GET_NAME(pud))) {
                t = oldpud;
                if (ch->hates.clist == t) {
                    ch->hates.clist = 0;
                } else {
                    for (oldpud = ch->hates.clist; oldpud->next != t;
                         oldpud = oldpud->next) {
                        /* 
                         * Empty loop 
                         */
                    }
                    oldpud->next = oldpud->next->next;
                }
                if (t) {
                    free(t);
                }
                break;
            }
        }
    }

    if (!ch->hates.clist) {
        REMOVE_BIT(ch->hatefield, HATE_CHAR);
    }
    if (IS_NPC(ch) && !ch->hatefield) {
        REMOVE_BIT(ch->specials.act, ACT_HATEFUL);
    }
    return ((pud) ? TRUE : FALSE);
}

int AddHated(struct char_data *ch, struct char_data *pud)
{
    struct char_list *newpud;

    if (ch == pud) {
        return (FALSE);
    }
    if (pud && ch) {

        if (GET_HIT(pud) <= 0 || GET_POS(pud) <= POSITION_DEAD) {
            return (FALSE);
        }
        if (!CAN_SEE(ch, pud)) {
            return (FALSE);
        }
        CREATE(newpud, struct char_list, 1);
        newpud->op_ch = pud;
        strcpy(newpud->name, GET_NAME(pud));
        newpud->next = ch->hates.clist;
        ch->hates.clist = newpud;

        if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_HATEFUL)) {
            SET_BIT(ch->specials.act, ACT_HATEFUL);
        }
        if (!IS_SET(ch->hatefield, HATE_CHAR)) {
            SET_BIT(ch->hatefield, HATE_CHAR);
        }

        if (pud->in_room != ch->in_room) {
            SetHunting(ch, pud);
        }

        if (IS_IMMORTAL(pud)) {
            send_to_char("---Someone hates you.\n\r", pud);
        }
    }
    return ((pud && ch) ? TRUE : FALSE);
}

int AddHatred(struct char_data *ch, OpinionType_t parm_type, int parm)
{
    if( !ch ) {
        return( FALSE );
    }

    switch (parm_type) {
    case OP_SEX:
        if (!IS_SET(ch->hatefield, HATE_SEX)) {
            SET_BIT(ch->hatefield, HATE_SEX);
        }
        ch->hates.sex = parm;
        break;
    case OP_RACE:
        if (!IS_SET(ch->hatefield, HATE_RACE)) {
            SET_BIT(ch->hatefield, HATE_RACE);
        }
        ch->hates.race = parm;
        break;
    case OP_GOOD:
        if (!IS_SET(ch->hatefield, HATE_GOOD)) {
            SET_BIT(ch->hatefield, HATE_GOOD);
        }
        ch->hates.good = parm;
        break;
    case OP_EVIL:
        if (!IS_SET(ch->hatefield, HATE_EVIL)) {
            SET_BIT(ch->hatefield, HATE_EVIL);
        }
        ch->hates.evil = parm;
        break;
    case OP_CLASS:
        if (!IS_SET(ch->hatefield, HATE_CLASS)) {
            SET_BIT(ch->hatefield, HATE_CLASS);
        }
        ch->hates.class = parm;
        break;
    case OP_VNUM:
        if (!IS_SET(ch->hatefield, HATE_VNUM)) {
            SET_BIT(ch->hatefield, HATE_VNUM);
        }
        ch->hates.vnum = parm;
        break;
    default:
        return( FALSE );
    }
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_HATEFUL)) {
        SET_BIT(ch->specials.act, ACT_HATEFUL);
    }
    return( TRUE );
}

int RemHatred(struct char_data *ch, unsigned short bitv)
{
    if( !ch ) {
        return( FALSE );
    }

    REMOVE_BIT(ch->hatefield, bitv);
    if (IS_NPC(ch) && !ch->hatefield) {
        REMOVE_BIT(ch->specials.act, ACT_HATEFUL);
    }
    return( TRUE );
}

int Hates(struct char_data *ch, struct char_data *v)
{
    char            buf[256];
    struct char_list *i;

    if (IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    if (ch == v) {
        return (FALSE);
    }
    if (IS_SET(ch->hatefield, HATE_CHAR)) {
        if (ch->hates.clist) {
            for (i = ch->hates.clist; i; i = i->next) {
                if (i->op_ch) {
                    if ((i->op_ch == v) && (!strcmp(i->name, GET_NAME(v)))) {
                        return (TRUE);
                    }
                } else {
                    if (!strcmp(i->name, GET_NAME(v))) {
                        return (TRUE);
                    }
                }
            }
        }
    }

    if (IS_SET(ch->hatefield, HATE_RACE) && ch->hates.race != -1 && 
        ch->hates.race == GET_RACE(v)) {
        sprintf(buf, "You hate %d", GET_RACE(v));
        send_to_char(buf, ch);
        return (TRUE);
    }

    if (IS_SET(ch->hatefield, HATE_SEX) && ch->hates.sex == GET_SEX(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->hatefield, HATE_GOOD) && ch->hates.good < GET_ALIGNMENT(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->hatefield, HATE_EVIL) && ch->hates.evil > GET_ALIGNMENT(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->hatefield, HATE_CLASS) && HasClass(v, ch->hates.class)) {
        return (TRUE);
    }

    if (IS_SET(ch->hatefield, HATE_VNUM) && 
        ch->hates.vnum == mob_index[v->nr].virtual) {
        return (TRUE);
    }

    return (FALSE);
}

void ShowHates(struct char_data *ch, char *buffer)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct char_list *i;
    struct char_data *mob;

    if( !ch || !buffer ) {
        return;
    }

    buf[0] = '\0';

    if (IS_SET(ch->hatefield, HATE_CHAR)) {
        strcat(buf, "Char: ");
        if (ch->hates.clist) {
            for (i = ch->hates.clist; i; i = i->next) {
                if( i->name ) {
                    strcat(buf, i->name);
                    if( i->next ) {
                        strcat( buf, ", " );
                    }
                }
            }
        } else {
            strcat(buf, "None");
        }
        strcat(buf, "  ");
    }

    if (IS_SET(ch->hatefield, HATE_RACE) && ch->hates.race != -1) {
        sprintf( buf, "%sRace: %s  ", buf, races[ch->hates.race].racename );
    }

    if (IS_SET(ch->hatefield, HATE_SEX)) {
        sprintf( buf, "%sSex: %s  ", buf, 
                (ch->hates.sex ? (ch->hates.sex == 1 ? "Male" : "Female") : 
                 "Neutral") );
    }

    if (IS_SET(ch->hatefield, HATE_GOOD)) {
        strcat( buf, "Align: Good  " );
    }

    if (IS_SET(ch->hatefield, HATE_EVIL)) {
        strcat( buf, "Align: Evil  " );
    }

    if (IS_SET(ch->hatefield, HATE_CLASS)) {
        sprintclasses((unsigned) ch->hates.class, buf2);
        sprintf( buf, "%sClass: %s  ", buf, buf2 );
    }

    if (IS_SET(ch->hatefield, HATE_VNUM)) {
        mob = read_mobile( ch->hates.vnum, VIRTUAL );
        if( mob ) {
            strcpy( buf2, GET_NAME(mob) );
            extract_char(mob);
        } else {
            strcpy( buf2, "Unknown" );
        }
        sprintf( buf, "%sMob: %s (%d)  ", buf, buf2, ch->hates.vnum );
    }

    if( !buf[0] ) {
        strcpy( buf, "None" );
    }

    strcat( buffer, buf );
}

int Fears(struct char_data *ch, struct char_data *v)
{
    struct char_list *i;

    if (IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
        return (FALSE);
    }
    if (IS_SET(ch->fearfield, FEAR_CHAR)) {
        if (ch->fears.clist) {
            for (i = ch->fears.clist; i; i = i->next) {
                if (i) {
                    if (i->op_ch) {
                        if (i->name[0] != '\0') {
                            if ((i->op_ch == v) &&
                                (!strcmp(i->name, GET_NAME(v)))) {
                                return (TRUE);
                            }
                        } else {
                            /*
                             * lets see if this clears the problem 
                             */
                            RemFeared(ch, i->op_ch);
                        }
                    } else if (i->name[0] != '\0' && 
                               !strcmp(i->name, GET_NAME(v))) {
                        return (TRUE);
                    }
                }
            }
        }
    }

    if (IS_SET(ch->fearfield, FEAR_RACE) && ch->fears.race != -1 && 
        ch->fears.race == GET_RACE(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->fearfield, FEAR_SEX) && ch->fears.sex == GET_SEX(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->fearfield, FEAR_GOOD) && ch->fears.good < GET_ALIGNMENT(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->fearfield, FEAR_EVIL) && ch->fears.evil > GET_ALIGNMENT(v)) {
        return (TRUE);
    }

    if (IS_SET(ch->fearfield, FEAR_CLASS) && HasClass(v, ch->hates.class)) {
        return (TRUE);
    }

    if (IS_SET(ch->fearfield, FEAR_VNUM) &&
        ch->fears.vnum == mob_index[v->nr].virtual) {
        return (TRUE);
    }

    return (FALSE);
}

void ShowFears(struct char_data *ch, char *buffer)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct char_list *i;
    struct char_data *mob;

    if( !ch || !buffer ) {
        return;
    }

    buf[0] = '\0';

    if (IS_SET(ch->fearfield, HATE_CHAR)) {
        strcat(buf, "Char: ");
        if (ch->fears.clist) {
            for (i = ch->fears.clist; i; i = i->next) {
                if( i->name ) {
                    strcat(buf, i->name);
                    if( i->next ) {
                        strcat( buf, ", " );
                    }
                }
            }
        } else {
            strcat(buf, "None");
        }
        strcat(buf, "  ");
    }

    if (IS_SET(ch->fearfield, HATE_RACE) && ch->fears.race != -1) {
        sprintf( buf, "%sRace: %s  ", buf, races[ch->fears.race].racename );
    }

    if (IS_SET(ch->fearfield, HATE_SEX)) {
        sprintf( buf, "%sSex: %s  ", buf, 
                (ch->fears.sex ? (ch->fears.sex == 1 ? "Male" : "Female") : 
                 "Neutral") );
    }

    if (IS_SET(ch->fearfield, HATE_GOOD)) {
        strcat( buf, "Align: Good  " );
    }

    if (IS_SET(ch->fearfield, HATE_EVIL)) {
        strcat( buf, "Align: Evil  " );
    }

    if (IS_SET(ch->fearfield, HATE_CLASS)) {
        sprintclasses((unsigned) ch->fears.class, buf2);
        sprintf( buf, "%sClass: %s  ", buf, buf2 );
    }

    if (IS_SET(ch->fearfield, HATE_VNUM)) {
        mob = read_mobile( ch->fears.vnum, VIRTUAL );
        if( mob ) {
            strcpy( buf2, GET_NAME(mob) );
            extract_char(mob);
        } else {
            strcpy( buf2, "Unknown" );
        }
        sprintf( buf, "%sMob: %s (%d)  ", buf, buf2, ch->fears.vnum );
    }

    if( !buf[0] ) {
        strcpy( buf, "None" );
    }

    strcat( buffer, buf );
}

int RemFeared(struct char_data *ch, struct char_data *pud)
{
    struct char_list *oldpud,
                   *t,
                   *tmp;

    if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
        return (FALSE);
    }
    if (pud && (ch->fears.clist != 0)) {
        tmp = ch->fears.clist;
        for (oldpud = ch->fears.clist; (oldpud != 0); oldpud = tmp) {
            if (oldpud == 0) {
                return (FALSE);
            }
            tmp = oldpud->next;
            if (oldpud->op_ch) {
                if (oldpud->op_ch == pud) {
                    t = oldpud;
                    if (ch->fears.clist == t) {
                        ch->fears.clist = 0;
                    } else {
                        for (oldpud = ch->fears.clist; oldpud->next != t;
                             oldpud = oldpud->next) {
                            /* 
                             * Empty loop 
                             */
                        }
                        oldpud->next = oldpud->next->next;
                    }
                    if (t) {
                        free(t);
                    }
                    break;
                }
            } else if (!strcmp(oldpud->name, GET_NAME(pud))) {
                t = oldpud;
                if (ch->fears.clist == t) {
                    ch->fears.clist = 0;
                } else {
                    for (oldpud = ch->fears.clist; oldpud->next != t;
                         oldpud = oldpud->next) {
                        /* 
                         * Empty loop 
                         */
                    }
                    oldpud->next = oldpud->next->next;
                }
                if (t) {
                    free(t);
                }
                break;
            }
        }
    }

    if (!ch->fears.clist) {
        REMOVE_BIT(ch->fearfield, FEAR_CHAR);
    }
    if (!ch->fearfield) {
        REMOVE_BIT(ch->specials.act, ACT_AFRAID);
    }
    return ((pud) ? TRUE : FALSE);
}

int AddFeared(struct char_data *ch, struct char_data *pud)
{
    struct char_list *newpud;

    if (pud) {
        if (!CAN_SEE(ch, pud)) {
            return (FALSE);
        }
        CREATE(newpud, struct char_list, 1);
        newpud->op_ch = pud;
        strcpy(newpud->name, GET_NAME(pud));
        newpud->next = ch->fears.clist;
        ch->fears.clist = newpud;

        if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
            SET_BIT(ch->specials.act, ACT_AFRAID);
        }
        if (!IS_SET(ch->fearfield, FEAR_CHAR)) {
            SET_BIT(ch->fearfield, FEAR_CHAR);
        }
        if (IS_IMMORTAL(pud)) {
            send_to_char("---Someone fears you.\n\r", pud);
        }
    }

    return ((pud) ? TRUE : FALSE);
}

int AddFears(struct char_data *ch, OpinionType_t parm_type, int parm)
{
    if( !ch ) {
        return( FALSE );
    }

    switch (parm_type) {
    case OP_SEX:
        if (!IS_SET(ch->fearfield, FEAR_SEX)) {
            SET_BIT(ch->fearfield, FEAR_SEX);
        }
        ch->fears.sex = parm;
        break;
    case OP_RACE:
        if (!IS_SET(ch->fearfield, FEAR_RACE)) {
            SET_BIT(ch->fearfield, FEAR_RACE);
        }
        ch->fears.race = parm;
        break;
    case OP_GOOD:
        if (!IS_SET(ch->fearfield, FEAR_GOOD)) {
            SET_BIT(ch->fearfield, FEAR_GOOD);
        }
        ch->fears.good = parm;
        break;
    case OP_EVIL:
        if (!IS_SET(ch->fearfield, FEAR_EVIL)) {
            SET_BIT(ch->fearfield, FEAR_EVIL);
        }
        ch->fears.evil = parm;
        break;
    case OP_CLASS:
        if (!IS_SET(ch->fearfield, FEAR_CLASS)) {
            SET_BIT(ch->fearfield, FEAR_CLASS);
        }
        ch->fears.class = parm;
        break;
    case OP_VNUM:
        if (!IS_SET(ch->fearfield, FEAR_VNUM)) {
            SET_BIT(ch->fearfield, FEAR_VNUM);
        }
        ch->fears.vnum = parm;
        break;
    default:
        return(FALSE);
        break;
    }
    if (!IS_SET(ch->specials.act, ACT_AFRAID)) {
        SET_BIT(ch->specials.act, ACT_AFRAID);
    }
    return(TRUE);
}

/*
 * FindAHatee crashes alot.. fix it!! 
 */
struct char_data *FindAHatee(struct char_data *ch)
{
    struct char_data *tmp_ch;
    struct room_data *rp;
#if 0
    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch; 
         tmp_ch =tmp_ch->next_in_room) {
#endif
    if (!ch) {
        Log("2NO CHAR!!!");
        return (0);
    }
    if (ch->in_room < 0) {
        return (0);
    }
    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0        
        Log("No room data in FindAHatee ??Crash???");
#endif
        return (0);
    }

    tmp_ch = rp->people;
    if (!tmp_ch) {
        return (0);
    }

    while (tmp_ch) {
        if (Hates(ch, tmp_ch) && CAN_SEE(ch, tmp_ch) &&
            ch->in_room == tmp_ch->in_room) {
#if 1
            if (ch != tmp_ch || (IS_PC(tmp_ch) && IS_IMMORTAL(tmp_ch))) {
#else
            if (ch != tmp_ch) {
#endif
                return (tmp_ch);
            } else {
                RemHated(ch, tmp_ch);
                return (0);
            }
        }
        tmp_ch = tmp_ch->next_in_room;
    }
    return (0);
}

#if 0
/*
 * Stock muds FindAHatee 
 */
struct char_data *FindAHatee(struct char_data *ch)
{
    struct char_data *tmp_ch;

    if (ch->in_room == NULL) {
        return NULL;
    }
    for (tmp_ch = ch->in_room->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (Hates(ch, tmp_ch) && CAN_SEE(ch, tmp_ch) &&
            ch->in_room == tmp_ch->in_room) {
            if (ch != tmp_ch) {
                return (tmp_ch);
            } else {
                RemHated(ch, tmp_ch);
                return NULL;
            }
        }
    }

    return NULL;
}
#endif

struct char_data *FindAFearee(struct char_data *ch)
{
    struct char_data *tmp_ch;

    if (ch->in_room < 0) {
        return (0);
    }
    for (tmp_ch = real_roomp(ch->in_room)->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (Fears(ch, tmp_ch) && CAN_SEE(ch, tmp_ch) &&
            ch->in_room == tmp_ch->in_room && ch != tmp_ch) {
            return (tmp_ch);
        }
    }
    return (0);
}

/*
 * these two procedures zero out the character pointer for quiting
 * players, without removing names thus the monsters will still hate them 
 */

void ZeroHatred(struct char_data *ch, struct char_data *v)
{
    struct char_list *oldpud;

    for (oldpud = ch->hates.clist; oldpud; oldpud = oldpud->next) {
        if (oldpud && oldpud->op_ch && oldpud->op_ch == v) {
            oldpud->op_ch = 0;
        }
    }
}

void ZeroFeared(struct char_data *ch, struct char_data *v)
{

    struct char_list *oldpud;

    for (oldpud = ch->fears.clist; oldpud; oldpud = oldpud->next) {
        if (oldpud && oldpud->op_ch && oldpud->op_ch == v) {
            oldpud->op_ch = 0;
        }
    }
}

/*
 * these two are to make the monsters completely forget about them. 
 */
void DeleteHatreds(struct char_data *ch)
{
    struct char_data *i;
    extern struct char_data *character_list;

    for (i = character_list; i; i = i->next) {
        if (Hates(i, ch)) {
            RemHated(i, ch);
        }
    }
}

void DeleteFears(struct char_data *ch)
{
    struct char_data *i;
    extern struct char_data *character_list;

    for (i = character_list; i; i = i->next) {
        if (Fears(i, ch)) {
            RemFeared(i, ch);
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
