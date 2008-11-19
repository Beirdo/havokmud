#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#include "protos.h"
#include "externs.h"


int pc_class_num(int clss);


int             spy_flag;

static char logBuf[MAX_STRING_LENGTH];


int EgoBladeSave(struct char_data *ch)
{
#ifndef USE_EGOS
    return( TRUE );
#else
    int             total;

    if (GetMaxLevel(ch) <= 10) {
        return (FALSE);
    }
    total = (GetMaxLevel(ch) + GET_STR(ch) + GET_CON(ch));
    if (GET_HIT(ch) == 0) {
        return (FALSE);
    }
    total = total - (GET_MAX_HIT(ch) / GET_HIT(ch));
    if (((int) number(1, 101)) > total) {
        return (FALSE);
    } else {
        return (TRUE);
    }
#endif
}

int OnlyClassItemValid(struct char_data *ch, struct obj_data *obj)
{
    if ((HasClass(ch, CLASS_MAGIC_USER) || HasClass(ch, CLASS_SORCERER)) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_MAGE)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_SORCERER) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_SORCERER)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_THIEF) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_THIEF)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_WARRIOR) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_FIGHTER)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_CLERIC) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_CLERIC)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_BARBARIAN) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_BARBARIAN)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_RANGER) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_RANGER)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_PALADIN) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_PALADIN)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_PSI) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_PSI)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_MONK) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_MONK)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_DRUID) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_DRUID)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_NECROMANCER) &&
        !IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_NECROMANCER)) {
        return (FALSE);
    }
    return (TRUE);
}

int GetItemClassRestrictions(struct obj_data *obj)
{
    int             total = 0;

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_SORCERER)) {
        total |= CLASS_SORCERER;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_MAGE)) {
        total |= CLASS_MAGIC_USER;
        total |= CLASS_SORCERER;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_THIEF)) {
        total |= CLASS_THIEF;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_FIGHTER)) {
        total |= CLASS_WARRIOR;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_CLERIC)) {
        total |= CLASS_CLERIC;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_BARBARIAN)) {
        total |= CLASS_BARBARIAN;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_RANGER)) {
        total |= CLASS_RANGER;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_PALADIN)) {
        total |= CLASS_PALADIN;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_PSI)) {
        total |= CLASS_PSI;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_MONK)) {
        total |= CLASS_MONK;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_DRUID)) {
        total |= CLASS_DRUID;
    }

    if (IS_OBJ_STAT(obj, anti_class, ITEM_ANTI_NECROMANCER)) {
        total |= CLASS_NECROMANCER;
    }

    return (total);
}

int CAN_SEE(struct char_data *s, struct char_data *o)
{
    if (!o) {
        return (FALSE);
    }
    if (IS_IMMORTAL(o) && !o->invis_level && o->in_room) {
        return (TRUE);
    }
    if (o->invis_level > GetMaxLevel(s)) {
        return (FALSE);
    }
    if (IS_IMMORTAL(s) && (s->in_room != 0)) {
        return (TRUE);
    }
    if (!o || s->in_room <= 0 || o->in_room <= 0) {
        return (FALSE);
    }

    if (IS_AFFECTED(s, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED2(o, AFF2_ANIMAL_INVIS) && IsAnimal(s)) {
        return (FALSE);
    }
    if (IS_AFFECTED2(o, AFF2_INVIS_TO_UNDEAD) && IsUndead(s)) {
        return (FALSE);
    }
    if (IS_AFFECTED(o, AFF_HIDE)) {
        if (IS_AFFECTED(s, AFF_SENSE_LIFE)) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    }

    if (IS_AFFECTED(s, AFF_TRUE_SIGHT)) {
        return (TRUE);
    }
    if (IS_AFFECTED(o, AFF_INVISIBLE)) {
        if (IS_IMMORTAL(o)) {
            return (FALSE);
        }
        if (!IS_AFFECTED(s, AFF_DETECT_INVISIBLE)) {
            return (FALSE);
        }
    }

    if ((IS_DARK(s->in_room) || IS_DARK(o->in_room)) &&
        (!IS_AFFECTED(s, AFF_INFRAVISION))) {
        return (FALSE);
    }
    return (TRUE);
}

int exit_ok(struct room_direction_data *exit, struct room_data **rpp)
{
    struct room_data *rp;
    if (rpp == NULL) {
        rpp = &rp;
    }
    if (!exit) {
        *rpp = NULL;
        return FALSE;
    }
    *rpp = roomFindNum(exit->to_room);
    return (*rpp != NULL);
}

/**
 * @todo Convert to a macro
 */
int WeaponImmune(struct char_data *ch)
{
    if (IS_SET(IMM_NONMAG, ch->M_immune) ||
        IS_SET(IMM_PLUS1, ch->M_immune) ||
        IS_SET(IMM_PLUS2, ch->M_immune) ||
        IS_SET(IMM_PLUS3, ch->M_immune) || IS_SET(IMM_PLUS4, ch->M_immune)) {
        return (TRUE);
    }
    return (FALSE);
}

/**
 * @todo Convert to a macro
 */
unsigned IsImmune(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->M_immune));
}

/**
 * @todo Convert to a macro
 */
unsigned IsResist(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->immune) && !IS_SET(bit, ch->susc));
}

/**
 * @todo Convert to a macro
 */
unsigned IsSusc(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->susc) && !IS_SET(bit, ch->immune));
}

int scan_number(char *text, int *rval)
{
    int             length;

    if (sscanf(text, " %i %n", rval, &length) != 1) {
        return 0;
    }
    if (text[length] != 0) {
        return 0;
    }
    return 1;
}

char           *strip_ansi(char *newbuf, const char *orig, size_t maxlen)
{
    int             i = 0,
                    k = 0;

    while (orig[i] && (k < (maxlen - 1))) {
        while (orig[i] && orig[i] == '$' && 
               orig[i + 1] && LOWER(orig[i + 1]) == 'c' && 
               orig[i + 2] && isdigit((int)orig[i + 2]) && 
               orig[i + 3] && isdigit((int)orig[i + 3]) && 
               orig[i + 4] && isdigit((int)orig[i + 4]) && 
               orig[i + 5] && isdigit((int)orig[i + 5])) {
            i += 6;
        }
        newbuf[k++] = orig[i];

        if (orig[i]) {
            i++;
        }
    }

    newbuf[k] = '\0';

    return newbuf;
}

/*
 * This procedure removes the '\r' from a string so that it may be saved
 * to a file correctly.
 */
char           *strip_cr(char *newbuf, const char *orig, size_t maxlen)
{
    int             i = 0,
                    k = 0;

    while (*(orig + i) && (k < (maxlen - 1))) {
        if (*(orig + i) != '\r') {
            newbuf[k++] = orig[i];
        }
        i++;
    }

    newbuf[k] = '\0';

    return newbuf;
}

void sprintbit(unsigned long vektor, char *names[], char *result)
{
    long            nr;

    *result = '\0';

    for (nr = 0; vektor; vektor >>= 1) {
        if (IS_SET(1, vektor)) {
            if (*names[nr] != '\n') {
                strcat(result, names[nr]);
                strcat(result, ", ");
            } else {
                strcat(result, "UNDEFINED");
                strcat(result, ", ");
            }
        }
        if (*names[nr] != '\n') {
            nr++;
        }
    }

    if (!*result) {
        strcat(result, "NOBITS");
    } else {
        result[strlen(result) - 2] = '\0';
    }
}

void sprintclasses(unsigned long vektor, char *result)
{
    long            nr;

    *result = '\0';

    for( nr = 0; vektor && nr < MAX_CLASS; vektor >>=1, nr++ ) {
        if( vektor & 1 ) {
            strcat( result, classes[nr].name );

            if( vektor & (~1) ) {
                strcat( result, "/" );
            }
        }
    }

    if( !*result ) {
        strcat( result, "NOBITS" );
    }
}

void sprinttype(int type, char *names[], char *result)
{
    int             nr;

    for (nr = 0; (*names[nr] != '\n'); nr++) {
        /*
         * Empty loop
         */
    }
    if (type < nr) {
        strcpy(result, names[type]);
    } else {
        strcpy(result, "UNDEFINED");
    }
}

/*
 * Calculate the REAL time passed over the last t2-t1 centuries (secs)
 */
struct time_info_data real_time_passed(time_t t2, time_t t1)
{
    long            secs;
    struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_REAL_HOUR) % 24;
    secs -= SECS_PER_REAL_HOUR * now.hours;

    /* 0..34 days */
    now.day = (secs / SECS_PER_REAL_DAY);
    secs -= SECS_PER_REAL_DAY * now.day;

    now.month = -1;
    now.year = -1;

    return now;
}

/*
 * Calculate the MUD time passed over the last t2-t1 centuries (secs)
 */
struct time_info_data mud_time_passed(time_t t2, time_t t1)
{
    long            secs,
                    monthsecs,
                    daysecs,
                    hoursecs;
    struct time_info_data now;

    if (t2 >= t1) {
        secs = (long) (t2 - t1);

        now.year = secs / SECS_PER_MUD_YEAR;

        monthsecs = secs % SECS_PER_MUD_YEAR;
        now.month = monthsecs / SECS_PER_MUD_MONTH;

        daysecs = monthsecs % SECS_PER_MUD_MONTH;
        now.day = daysecs / SECS_PER_MUD_DAY;

        hoursecs = daysecs % SECS_PER_MUD_DAY;
        now.hours = hoursecs / SECS_PER_MUD_HOUR;
    } else {
        secs = (long) (t1 - t2);

        now.year = secs / SECS_PER_MUD_YEAR;

        monthsecs = secs % SECS_PER_MUD_YEAR;
        now.month = monthsecs / SECS_PER_MUD_MONTH;

        daysecs = monthsecs % SECS_PER_MUD_MONTH;
        now.day = daysecs / SECS_PER_MUD_DAY;

        hoursecs = daysecs % SECS_PER_MUD_DAY;
        now.hours = hoursecs / SECS_PER_MUD_HOUR;

        if (now.hours) {
            now.hours = 24 - now.hours;
            now.day = now.day + 1;
        }
        if (now.day) {
            now.day = 35 - now.day;
            now.month = now.month + 1;
        }
        if (now.month) {
            now.month = 17 - now.month;
            now.year = now.year + 1;
        }
        if (now.year) {
            now.year = -now.year;
        }
    }
    return (now);
}

void mud_time_passed2(time_t t2, time_t t1, struct time_info_data *t)
{
    long            secs,
                    monthsecs,
                    daysecs,
                    hoursecs;

    if (t2 >= t1) {
        secs = (long) (t2 - t1);

        t->year = secs / SECS_PER_MUD_YEAR;

        monthsecs = secs % SECS_PER_MUD_YEAR;
        t->month = monthsecs / SECS_PER_MUD_MONTH;

        daysecs = monthsecs % SECS_PER_MUD_MONTH;
        t->day = daysecs / SECS_PER_MUD_DAY;

        hoursecs = daysecs % SECS_PER_MUD_DAY;
        t->hours = hoursecs / SECS_PER_MUD_HOUR;
    } else {

        secs = (long) (t1 - t2);

        t->year = secs / SECS_PER_MUD_YEAR;

        monthsecs = secs % SECS_PER_MUD_YEAR;
        t->month = monthsecs / SECS_PER_MUD_MONTH;

        daysecs = monthsecs % SECS_PER_MUD_MONTH;
        t->day = daysecs / SECS_PER_MUD_DAY;

        hoursecs = daysecs % SECS_PER_MUD_DAY;
        t->hours = hoursecs / SECS_PER_MUD_HOUR;

        if (t->hours) {
            t->hours = 24 - t->hours;
            t->day = t->day + 1;
        }
        if (t->day) {
            t->day = 35 - t->day;
            t->month = t->month + 1;
        }
        if (t->month) {
            t->month = 17 - t->month;
            t->year = t->year + 1;
        }
        if (t->year) {
            t->year = -t->year;
        }
    }
}

void age2(struct char_data *ch, struct time_info_data *g)
{

    mud_time_passed2(time(0), ch->player.time.birth, g);

    /*
     * All players start at 17
     */
    g->year += 17;

}

struct time_info_data age(struct char_data *ch)
{
    struct time_info_data player_age;

    player_age = mud_time_passed(time(0), ch->player.time.birth);

    /*
     * All players start at 17
     */
    player_age.year += 17;

    return (player_age);
}

int in_group(struct char_data *ch1, struct char_data *ch2)
{
    /*
     * possibilities -> 1.  char is char2's master 2.  char2 is char's
     * master 3.  char and char2 follow same. 4.  char rides char2 5..
     * char2 rides char
     *
     * otherwise not true.
     *
     */
    if (ch1 == ch2) {
        return (TRUE);
    }
    if ((!ch1) || (!ch2)) {
        return (FALSE);
    }
    if ((!ch1->master) && (!ch2->master)) {
        return (FALSE);
    }
    if (ch1 == ch2->master) {
        return (TRUE);
    }
    if (ch1->master == ch2) {
        return (TRUE);
    }
    if (ch1->master == ch2->master) {
        return (TRUE);
    }

    if (MOUNTED(ch1) == ch2 || RIDDEN(ch1) == ch2) {
        return (TRUE);
    }
    return (FALSE);
}

/*
 * more new procedures
 */

/*
 * these two procedures give the player the ability to buy 2*bread or put
 * all.bread in bag, or put 2*bread in bag...
 */

char getall(char *name, char *newname)
{
    char            arg[40],
                    tmpname[80],
                    otname[80];
    char            prd;

    arg[0] = '\0';
    tmpname[0] = '\0';
    otname[0] = '\0';

    /*
     * reads up to first space
     */
    sscanf(name, "%s ", otname);

    if (strlen(otname) < 5) {
        return (FALSE);
    }
    sscanf(otname, "%3s%c%s", arg, &prd, tmpname);

    if (prd != '.') {
        return (FALSE);
    }
    if (tmpname == NULL) {
        return (FALSE);
    }
    if (strcmp(arg, "all")) {
        return (FALSE);
    }
    while (*name != '.') {
        name++;
    }
    name++;

    strcpy( newname, name );
    return (TRUE);
}

int getabunch(char *name, char *newname)
{
    int             num = 0;
    char            tmpname[80];

    tmpname[0] = 0;
    sscanf(name, "%d*%s", &num, tmpname);
    if (tmpname[0] == '\0') {
        return (FALSE);
    }
    if (num < 1) {
        return (FALSE);
    }
    if (num > 9) {
        num = 9;
    }
    while (*name != '*') {
        name++;
    }
    name++;

    strcpy(newname, name);
    return (num);
}

typedef struct {
    int     base;
    int     phit;
    int     sab;
} ExperienceTable_t;

static ExperienceTable_t experienceTable[] = {
    { 5, 1, 10 },           /* 0 */
    { 10, 1, 15 },          /* 1 */
    { 20, 2, 20 },          /* 2 */
    { 35, 3, 25 },          /* 3 */
    { 60, 4, 30 },          /* 4 */
    { 90, 5, 40 },          /* 5 */
    { 150, 6, 75 },         /* 6 */
    { 225, 8, 125 },        /* 7 */
    { 600, 12, 175 },       /* 8 */
    { 900, 14, 300 },       /* 9 */
    { 1100, 15, 450 },      /* 10 */
    { 1300, 16, 700 },      /* 11 */
    { 1550, 17, 700 },      /* 12 */
    { 1800, 18, 950 },      /* 13 */
    { 2100, 19, 950 },      /* 14 */
    { 2400, 20, 1250 },     /* 15 */
    { 2700, 23, 1250 },     /* 16 */
    { 3000, 25, 1550 },     /* 17 */
    { 3500, 28, 1550 },     /* 18 */
    { 4000, 30, 2100 },     /* 19 */
    { 4500, 33, 2100 },     /* 20 */
    { 5000, 35, 2600 },     /* 21 */
    { 6000, 40, 3000 },     /* 22 */
    { 7000, 45, 3500 },     /* 23 */
    { 8000, 50, 4000 },     /* 24 */
    { 9000, 55, 4500 },     /* 25 */
    { 10000, 60, 5000 },    /* 26 */
    { 12000, 70, 6000 },    /* 27 */
    { 14000, 80, 7000 },    /* 28 */
    { 16000, 90, 8000 },    /* 29 */
    { 20000, 100, 10000 },  /* 30 */
    { 22000, 120, 12000 },  /* 31 */
    { 22000, 120, 12000 },  /* 32 */
    { 22000, 120, 12000 },  /* 33 */
    { 22000, 120, 12000 },  /* 34 */
    { 32000, 140, 14000 },  /* 35 */
    { 32000, 140, 14000 },  /* 36 */
    { 32000, 140, 14000 },  /* 37 */
    { 32000, 140, 14000 },  /* 38 */
    { 32000, 140, 14000 },  /* 39 */
    { 42000, 160, 16000 },  /* 40 */
    { 42000, 160, 16000 },  /* 41 */
    { 52000, 180, 20000 },  /* 42 */
    { 52000, 180, 20000 },  /* 43 */
    { 72000, 200, 24000 },  /* 44 */
    { 72000, 200, 24000 },  /* 45 */
    { 92000, 225, 28000 },  /* 46 */
    { 92000, 225, 28000 },  /* 47 */
    { 122000, 250, 32000 }, /* 48 */
    { 122000, 250, 32000 }, /* 49 */
    { 150000, 275, 36000 }, /* 50 */
    { 200000, 300, 40000 }, /* 51+ */
};
static int experienceTableCount = NELEMS(experienceTable);


int DetermineExp(struct char_data *mob, int exp_flags)
{
    ExperienceTable_t  *entry;
    int                 index;

    if (exp_flags == 0) {
        exp_flags = 1;
    }

    /*
     * reads in the monster, and adds the flags together for simplicity, 1
     * exceptional ability is 2 special abilities
     */
    index = GetMaxLevel(mob);
    if( index < 0 ) {
        return( 1 );
    }

    if( index > experienceTableCount ) {
        index = experienceTableCount - 1;
    }

    entry = &experienceTable[index];
    return( entry->base + (entry->phit * GET_HIT(mob)) + 
            (entry->sab * exp_flags) );
}

int GetExpFlags(struct char_data *mob, int exp)
{

    ExperienceTable_t  *entry;
    int                 index;

    /*
     * reads in the monster, and adds the flags together for simplicity, 1
     * exceptional ability is 2 special abilities
     */

    index = GetMaxLevel(mob);
    if( index < 0 ) {
        return( 1 );
    }

    if( index > experienceTableCount ) {
        index = experienceTableCount - 1;
    }

    entry = &experienceTable[index];
    return( (exp - entry->base - (entry->phit * GET_HIT(mob))) / entry->sab );
}

void SetHunting(struct char_data *ch, struct char_data *tch)
{
    int                 persist,
                        dist;
    struct room_data   *rp;

#ifndef USE_TRACK
    return;
#endif

    if (!ch || !tch) {
        Log("!ch || !tch in SetHunting");
        return;
    }

    persist = GetMaxLevel(ch);
    persist *= (int) GET_ALIGNMENT(ch) / 100;

    if (persist < 0) {
        persist = -persist;
    }
    dist = GET_ALIGNMENT(tch) - GET_ALIGNMENT(ch);
    dist = (dist > 0) ? dist : -dist;
    if (Hates(ch, tch)) {
        dist *= 2;
    }
    SET_BIT(ch->specials.act, ACT_HUNTING);
    ch->specials.hunting = tch;
    ch->hunt_dist = dist;
    ch->persist = persist;
    ch->old_room = ch->in_room;

#if 0
    if (IS_IMMORTAL(tch)) {
        rp = roomFindNum(ch->in_room);
        if( rp ) {
            sprintf(buf, ">>%s is hunting you from %s\n\r",
                    (ch->player.short_descr[0] ? ch->player.short_descr : 
                     "(null)"),
                    (rp->name[0] ? rp->name : "(null)"));
        send_to_char(buf, tch);
    }
#else
    (void)rp;
#endif
}

bool CallForGuard_cond( struct char_data *i, void *arg )
{
    struct char_data   *ch;

    if( !i || !arg ) {
        return( FALSE );
    }

    ch = (struct char_data *)arg;

    if( IS_NPC(i) && i != ch && !i->specials.fighting ) {
        return( TRUE );
    }
    return( FALSE );
}
    
struct CallForGuardArgs_t {
    bool                stop;
    int                 lev;
    int                 type[2];
    struct char_data   *victim;
};

bool CallForGuard_callback( struct char_data *i, void *arg )
{
    struct CallForGuardArgs_t  *args;

    if( !arg ) {
        return( FALSE );
    }

    args = (struct CallForGuardArgs_t *)arg;

    if( !args->victim ) {
        return( FALSE );
    }

    if (i->nr == args->type[0]) {
        if (!number(0, 5) && !IS_SET(i->specials.act, ACT_HUNTING) ) {
            SetHunting(i, args->vict);
            args->lev--;
            args->stop = ( args->lev <= 0 ? TRUE : FALSE );
            return( TRUE );
        }
    } else if (i->nr == args->type[1]) {
        if (!number(0, 5) && !IS_SET(i->specials.act, ACT_HUNTING) ) {
            SetHunting(i, args->vict);
            args->lev -= 2;
            args->stop = ( args->lev <= 0 ? TRUE : FALSE );
            return( TRUE );
        }
    }

    return( FALSE );
}

void CallForGuard(struct char_data *ch, struct char_data *vict,
                  int lev, int area)
{
    struct char_data *i;
    struct CallForGuardArgs_t   args;

    switch (area) {
    case MIDGAARD:
        args.type[0] = 3060;
        args.type[1] = 3069;
        break;
    case NEWTHALOS:
        args.type[0] = 3661;
        args.type[1] = 3682;
        break;
    case TROGCAVES:
        args.type[0] = 21114;
        args.type[1] = 21118;
        break;
    case OUTPOST:
        args.type[0] = 21138;
        args.type[1] = 21139;
        break;
    case PRYDAIN:
        args.type[0] = 6606;
        args.type[1] = 6614;
        break;
    default:
        args.type[0] = 3060;
        args.type[1] = 3069;
        break;
    }

    args.lev = ( lev == 0 ? 3 : lev );
    args.stop = ( args.lev <= 0 ? TRUE : FALSE );

    playerFindAll( CallForGuard_cond, ch, CallForGuard_callback, &args,
                   &args.stop );
}

void StandUp(struct char_data *ch)
{
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        if (ch->points.hit > (ch->points.max_hit / 2)) {
            act("$n quickly stands up.", 1, ch, 0, 0, TO_ROOM);
        } else if (ch->points.hit > (ch->points.max_hit / 6)) {
            act("$n slowly stands up.", 1, ch, 0, 0, TO_ROOM);
        } else {
            act("$n gets to $s feet very slowly.", 1, ch, 0, 0, TO_ROOM);
        }
        GET_POS(ch) = POSITION_STANDING;
    }
}

void MakeNiftyAttack(struct char_data *ch)
{
    int             num;
    char           *name;

    if (!ch->skills) {
        SpaceForSkills(ch);
        return;
    }

    if (!ch || !ch->skills) {
        Log("!or !ch-skills in MakeNiftyAttack() in utility.c");
        return;
    }

    if (!ch->specials.fighting) {
        return;
    }
    num = number(1, 4);
    name = strdup(GET_NAME(ch->specials.fighting));

    switch (num) {
    case 1:
    case 2:
        if (!ch->skills[SKILL_BASH].learned) {
            ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch) * 4;
        }
        do_bash(ch, name, 0);
        break;
    case 3:
        if (ch->equipment[WIELD]) {
            if (!ch->skills[SKILL_DISARM].learned) {
                ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch) * 4;
            }
            do_disarm(ch, name, 0);
        } else {
            if (!ch->skills[SKILL_KICK].learned) {
                ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
            }
            do_kick(ch, name, 0);
        }
        break;
    case 4:
        if (!ch->skills[SKILL_KICK].learned) {
            ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
        }
        do_kick(ch, name, 0);
        break;
    }

    if( name ) {
        free( name );
    }
}

void FighterMove(struct char_data *ch)
{
    struct char_data *friend;
    char           *name;

    if (!ch->skills) {
        SET_BIT(ch->player.class, CLASS_WARRIOR);
        SpaceForSkills(ch);
    }

    if (ch->specials.fighting &&
        ch->specials.fighting->specials.fighting != 0) {
        friend = ch->specials.fighting->specials.fighting;
        if (friend == ch) {
            MakeNiftyAttack(ch);
        } else {
            /*
             * rescue on a 1 or 2, other on a 3 or 4
             */
            if (GET_RACE(friend) == (GET_RACE(ch))) {
                if (GET_HIT(friend) < GET_HIT(ch)) {
                    if (!ch->skills[SKILL_RESCUE].learned) {
                        ch->skills[SKILL_RESCUE].learned =
                            GetMaxLevel(ch) * 3 + 30;
                    }
                    name = strdup(GET_NAME(friend));
                    do_rescue(ch, name, 0);
                    if( name ) {
                        free( name );
                    }
                } else {
                    MakeNiftyAttack(ch);
                }
            } else {
                MakeNiftyAttack(ch);
            }
        }
    }
}

void MonkMove(struct char_data *ch)
{
    char            buf[100];
    char           *name;

    if (!ch->skills) {
        SpaceForSkills(ch);
        ch->skills[SKILL_DODGE].learned = GetMaxLevel(ch) + 50;
        SET_BIT(ch->player.class, CLASS_MONK);
    }

    if (!ch->specials.fighting) {
        return;
    }

    name = strdup(GET_NAME(ch->specials.fighting));
    if (GET_POS(ch) < POSITION_FIGHTING) {
        if (!ch->skills[SKILL_SPRING_LEAP].learned) {
            ch->skills[SKILL_SPRING_LEAP].learned =
                (GetMaxLevel(ch) * 3) / 2 + 25;
        }
        do_springleap(ch, name, 0);
        return;
    } else {
        /*
         * Commented out as a temporary fix to monks fleeing challenges.
         * Was easier than rooting around in the spec_proc for the monk
         * challenge for it, which is proobably what should be done.
         */
        if (GET_HIT(ch) < GET_MAX_HIT(ch) / 20) {
            if (!ch->skills[SKILL_RETREAT].learned) {
                ch->skills[SKILL_RETREAT].learned = GetMaxLevel(ch) * 2 + 10;
            }
            strcpy(buf, "flee");
            command_interpreter(ch, buf);
            return;
        } else {
            if (GetMaxLevel(ch) > 30 && !number(0, 4) &&
                GetMaxLevel(ch->specials.fighting) <= GetMaxLevel(ch) &&
                GET_MAX_HIT(ch->specials.fighting) < 2 * GET_MAX_HIT(ch) &&
                !affected_by_spell(ch->specials.fighting, SKILL_QUIV_PALM) &&
                !affected_by_spell(ch, SKILL_QUIV_PALM) && ch->in_room == 551 &&
                ch->specials.fighting->skills[SKILL_QUIV_PALM].learned) {
                if (!ch->skills[SKILL_QUIV_PALM].learned) {
                    ch->skills[SKILL_QUIV_PALM].learned =
                        GetMaxLevel(ch) * 2 - 5;
                }
                do_quivering_palm(ch, name, 0);
                return;
            }

            if (ch->specials.fighting->equipment[WIELD]) {
                if (!ch->skills[SKILL_DISARM].learned) {
                    ch->skills[SKILL_DISARM].learned =
                        (GetMaxLevel(ch) * 3) / 2 + 25;
                }
    }

    if( name ) {
        free( name );
    }
}

void DevelopHatred(struct char_data *ch, struct char_data *v)
{
    int             diff,
                    patience,
                    var;

    if (!ch || !v || Hates(ch, v) || ch == v) {
        return;
    }
    diff = GET_ALIGNMENT(ch) - GET_ALIGNMENT(v);
    if (diff < 0) {
        diff = -diff;
    }
    diff /= 20;

    if (GET_MAX_HIT(ch)) {
        patience = (int) 100 *(float) (GET_HIT(ch) / GET_MAX_HIT(ch));
    } else {
        patience = 10;
    }

    var = number(1, 40) - 20;

    if (patience + var < diff) {
        AddHated(ch, v);
    }
}

int HasObject(struct char_data *ch, int ob_num)
{
    int             j,
                    found;
    struct obj_data *i;

    /*
     * equipment too
     */

    found = 0;

    for (j = 0; j < MAX_WEAR; j++) {
        if (ch->equipment[j]) {
            found += RecCompObjNum(ch->equipment[j], ob_num);
        }
    }

    if (found > 0) {
        return (TRUE);
    }
    /*
     * carrying
     */
    for (i = ch->carrying; i; i = i->next_content) {
        found += RecCompObjNum(i, ob_num);
    }

    if (found > 0) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

struct char_data *char_holding(struct obj_data *obj)
{
    if (obj->in_room != NOWHERE) {
        return NULL;
    } else if (obj->carried_by) {
        return obj->carried_by;
    } else if (obj->equipped_by) {
        return obj->equipped_by;
    } else if (obj->in_obj) {
        return char_holding(obj->in_obj);
    } else {
        return NULL;
    }
}

int RecCompObjNum(struct obj_data *o, int obj_num)
{

    int             total = 0;
    struct obj_data *i;
    LinkedListItem_t *item;

    if (o->item_number == obj_num) {
        total = 1;
    }
    if (ITEM_TYPE(o) == ITEM_TYPE_CONTAINER) {
        LinkedListLock( o->containList );
        for( item = o->containList->head; item; item = item->next ) {
            i = CONTAIN_LINK_TO_OBJ(item);
            total += RecCompObjNum(i, obj_num);
        }
        LinkedListUnlock( o->containList );
    }
    return (total);
}

void RestoreChar(struct char_data *ch)
{
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    if (!IS_IMMORTAL(ch)) {
        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) = 24;
    } else {
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, FULL) = -1;
    }
}

void RemAllAffects(struct char_data *ch)
{
    spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);

}

int CheckForBlockedMove(struct char_data *ch, int cmd, char *arg, int room,
                        int dir, int class)
{
    if (cmd > 6 || cmd < 1) {
        return (FALSE);
    }

    if ((IS_NPC(ch) && IS_POLICE(ch)) || GetMaxLevel(ch) >= DEMIGOD ||
        IS_AFFECTED(ch, AFF_SNEAK)) {
        return (FALSE);
    }


    if (ch->in_room == room && cmd == dir + 1 && !HasClass(ch, class)) {
        act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, 
            TO_ROOM);
        send_to_char("The guard humiliates you, and blocks your way.\n\r", ch);
        return TRUE;
    }
    return FALSE;
}

/*
 * Apply soundproof is for ch making noise
 */
int apply_soundproof(struct char_data *ch)
{
    struct room_data *rp;

    if (IS_IMMORTAL(ch)) {
        return (FALSE);
    }
    if (IS_AFFECTED(ch, AFF_SILENCE)) {
        send_to_char("You are silenced, you can't make a sound!\n\r", ch);
        return (TRUE);
    }

    rp = roomFindNum(ch->in_room);

    if (!rp) {
        return (FALSE);
    }
    if (IS_SET(rp->room_flags, SILENCE)) {
        /*
         * for shouts, emotes, etc
         */
        send_to_char("You are in a silence zone, you can't make a sound!\n\r",
                     ch);
        return (TRUE);
    }

    if (rp->sector_type == SECT_UNDERWATER) {
        send_to_char("Speak underwater, are you mad????\n\r", ch);
        return (TRUE);
    }
    return (FALSE);
}

/*
 * check_soundproof is for others making noise
 */
int check_soundproof(struct char_data *ch)
{
    struct room_data *rp;

    if (IS_AFFECTED(ch, AFF_SILENCE)) {
        return (TRUE);
    }

    rp = roomFindNum(ch->in_room);

    if (!rp) {
        return (FALSE);
    }
    if (IS_SET(rp->room_flags, SILENCE)) {
        /*
         * for shouts, emotes, etc
         */
        return (TRUE);
    }

    if (rp->sector_type == SECT_UNDERWATER) {
        return (TRUE);
    }
    return (FALSE);
}

void           *Mymalloc(long size)
{
    if (size < 1) {
        fprintf(stderr, "attempt to malloc negative memory - %ld\n", size);
        assert(0);
    }
    return (malloc(size));
}

void SpaceForSkills(struct char_data *ch)
{
    /*
     * create space for the skills for some mobile or character.
     */

    ch->skills = (struct char_skill_data *)
        malloc(MAX_SKILLS * sizeof(struct char_skill_data));

    if (ch->skills == 0) {
        assert(0);
    }
}

int CountLims(struct obj_data *obj)
{
    int                 total = 0;
    LinkedListItem_t   *item;
    struct obj_data    *subobj;

    if (!obj) {
        return (0);
    }

    LinkedListLock( obj->containList );
    for( item = obj->containList->head; item; item = item->next ) {
        subobj = CONTAIN_LINK_TO_OBJ(item);
        total += CountLims(subobj);
    }
    LinkedListUnlock( obj->containList );

    if (obj->next_content) {
        total += CountLims(obj->next_content);
    }

    if (IS_RARE(obj)) {
        /*
         * LIM_ITEM_COST_MIN)
         */
        total += 1;
    }

    return (total);
}

/**
 * @todo rewrite this crap to be iterative.  No need to recurse on a linked
 *       list!
 */
struct obj_data *find_a_rare(struct obj_data *obj)
{
    struct obj_data    *rare;
    struct obj_data    *subobj;
    LinkedListItem_t   *item;

    if (!obj) {
        return (NULL);
    }

    LinkedListLock( obj->containList );
    for( item = obj->containList->head; item; item = item->next ) {
        subobj = CONTAIN_LINK_TO_OBJ(item);
        rare = find_a_rare( subobj );
        if( rare ) {
            LinkedListUnlock( obj->containList );
            return( rare );
        }
    }
    LinkedListUnlock( obj->containList );

    if (IS_RARE(obj)) {
        /*
         * check self
         */
        return (obj);
    }

    if (obj->next_content) {
        /*
         * move to next item in list
         */
        rare = find_a_rare(obj->next_content);
        if (rare) {
            return (rare);
        }
    }

    return (NULL);
}

void CheckLegendStatus(struct char_data *ch)
{
    char            buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        return;
    }
    if (IS_SET(ch->specials.act, PLR_LEGEND) || IS_IMMORTAL(ch)) {
        return;
    }
    if (GET_EXP(ch) > 200000000 && ch->specials.m_kills >= 5000 &&
        GET_LEADERSHIP_EXP(ch) > 200000000) {
        /*
         * set legend status
         */
        SET_BIT(ch->specials.act, PLR_LEGEND);
        sprintf(buf, "The Gods have seen fit to raise %s to the status of "
                "Legend!\n\r", GET_NAME(ch));
        SendToAll(buf);
    }
}

char           *lower(char *s)
{
    static char     c[1000];
    char            *p;

    for(p = &(c[0]); *s; p++, s++) {
        if (*s >= 'A' && *s <= 'Z') {
            *p = *s + 32;
        } else {
            *p = *s;
        }
    }
    *p = '\0';
    return (c);
}

int getFreeAffSlot(struct obj_data *obj)
{
    int             i;

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].location == APPLY_NONE) {
            return (i);
        }
    }

    assert(0);
    return(0);
}

int check_nomagic(struct char_data *ch, char *msg_ch, char *msg_rm)
{
    struct room_data *rp;

    rp = roomFindNum(ch->in_room);
    if (rp && rp->room_flags & NO_MAGIC) {
        if (msg_ch) {
            send_to_char(msg_ch, ch);
        }
        if (msg_rm) {
            act(msg_rm, FALSE, ch, 0, 0, TO_ROOM);
        }
        return TRUE;
    }
    return FALSE;
}

int NumCharmedFollowersInRoom(struct char_data *ch)
{
    struct char_data *t;
    long            count = 0;
    struct room_data *rp;

    rp = roomFindNum(ch->in_room);
    if (rp) {
        count = 0;
        for (t = rp->people; t; t = t->next_in_room) {
            if (IS_AFFECTED(t, AFF_CHARM) && t->master == ch) {
                count++;
            }
        }
        return (count);
    }

    return (0);
}

int NoSummon(struct char_data *ch)
{
    struct room_data *rp;

    rp = roomFindNum(ch->in_room);
    if (!rp) {
        return (TRUE);
    }
    if (IS_SET(rp->room_flags, NO_SUM)) {
        send_to_char("Cryptic powers block your summons.\n\r", ch);
        return (TRUE);
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("Strange forces collide in your brain,\n\r", ch);
        send_to_char("Laws of nature twist, and dissapate before\n\r", ch);
        send_to_char("your eyes, strange ideas wrestle with green furry\n\r",
                     ch);
        send_to_char("things, which are crawling up your super-ego...\n\r", ch);
        send_to_char("  You lose a sanity point.\n\r\n\r", ch);
        send_to_char("  OOPS!  Sorry, wronge Genre.  :-) \n\r", ch);
        return (TRUE);
    }

    return (FALSE);
}

int GetApprox(int num, int perc)
{
    /*
     * perc = 0 - 100
     */
    int             adj,
                    r;
    float           fnum,
                    fadj;

    adj = 100 - perc;
    if (adj < 0) {
        adj = 0;
    }
    /* percentage of play (+- x%) */
    adj *= 2;

    r = number(1, adj);

    perc += r;

    fnum = (float) num;
    fadj = (float) perc *2;
    fnum *= (float) (fadj / (200.0));

    num = (int) fnum;

    return (num);
}

#define STEED_TEMPLATE 44
int MountEgoCheck(struct char_data *ch, struct char_data *horse)
{
    int             ride_ego,
                    drag_ego,
                    align,
                    check;

    /*
     * called steed check
     */
    if (horse->nr == STEED_TEMPLATE &&
        HasClass(ch, CLASS_PALADIN) &&
        GetMaxLevel(ch) >= GetMaxLevel(horse) + 10) {
        /*
         * ok, this horse likes you
         */
        return (-1);
    }

    if (IsDragon(horse)) {
        if (ch->skills) {
            drag_ego = GetMaxLevel(horse) * 2;
            if (IS_SET(horse->specials.act, ACT_AGGRESSIVE) ||
                IS_SET(horse->specials.act, ACT_META_AGG)) {
                drag_ego += GetMaxLevel(horse);
            }

            ride_ego = ch->skills[SKILL_RIDE].learned / 10 +
                       GetMaxLevel(ch) / 2;

            if (IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
                ride_ego += ((GET_INT(ch) + GET_WIS(ch)) / 2);
            }

            align = GET_ALIGNMENT(ch) - GET_ALIGNMENT(horse);
            if (align < 0) {
                align = -align;
            }
            align /= 100;
            align -= 5;
            drag_ego += align;
            if (GET_HIT(horse)) {
                drag_ego -= GET_MAX_HIT(horse) / GET_HIT(horse);
            } else {
                drag_ego = 0;
            }

            if (GET_HIT(ch)) {
                ride_ego -= GET_MAX_HIT(ch) / GET_HIT(ch);
            } else {
                ride_ego = 0;
            }

            check = drag_ego + number(1, 10) - (ride_ego + number(1, 10));
            return (check);
        } else {
            return (-GetMaxLevel(horse));
        }
    } else {
        if (!ch->skills) {
            return (-GetMaxLevel(horse));
        }
        drag_ego = GetMaxLevel(horse);

        if (drag_ego > 15) {
            drag_ego *= 2;
        }
        ride_ego = ch->skills[SKILL_RIDE].learned / 10 + GetMaxLevel(ch);

        if (IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
            ride_ego += (GET_INT(ch) + GET_WIS(ch));
        }
        check = drag_ego + number(1, 5) - (ride_ego + number(1, 10));
        return (check);
    }
}

int RideCheck(struct char_data *ch, int mod)
{
    if (ch->skills) {
        if (!IS_AFFECTED(ch, AFF_DRAGON_RIDE)) {
            if (number(1, 90) > ch->skills[SKILL_RIDE].learned + mod &&
                number(1, 91 - mod) > ch->skills[SKILL_RIDE].learned / 2 &&
                ch->skills[SKILL_RIDE].learned < 90) {
                send_to_char("You learn from your mistake\n\r", ch);
                ch->skills[SKILL_RIDE].learned += 2;
                return (FALSE);
            }
            return (TRUE);
        } else if (number(1, 90) > ch->skills[SKILL_RIDE].learned +
                                   GET_LEVEL(ch, BestMagicClass(ch)) + mod) {
            return (FALSE);
        }
        return (TRUE);
    }
    return (FALSE);
}

void FallOffMount(struct char_data *ch, struct char_data *h)
{
    act("$n loses control and falls off of $N", FALSE, ch, 0, h, TO_NOTVICT);
    act("$n loses control and falls off of you", FALSE, ch, 0, h, TO_VICT);
    act("You lose control and fall off of $N", FALSE, ch, 0, h, TO_CHAR);
}

int LearnFromMistake(struct char_data *ch, int sknum, int silent, int max)
{
    if (!ch->skills) {
        return (0);
    }

    if (!IS_SET(ch->skills[sknum].flags, SKILL_KNOWN)) {
        return (0);
    }

    if (ch->skills[sknum].learned < max && ch->skills[sknum].learned > 0 &&
        number(1, 101) > ch->skills[sknum].learned / 2) {
        if (!silent) {
            send_to_char("You learn from your mistake\n\r", ch);
        }
        ch->skills[sknum].learned += 1;

        if (ch->skills[sknum].learned >= max && !silent) {
            send_to_char("You are now learned in this skill!\n\r", ch);
        }
        return( TRUE );
    }
    return( FALSE );
}

/*
 * if (!IsOnPmp(roomnumber)) then they are on another plane!
 */
int IsOnPmp(int room_nr)
{
    struct room_data   *rp;

    rp = roomFindNum(room_nr);
    if (rp && !IS_SET(zone_table[rp->zone].reset_mode, ZONE_ASTRAL)) {
        return (TRUE);
    }

    return (FALSE);
}

int GetSumRaceMaxLevInRoom(struct char_data *ch)
{
    struct room_data *rp;
    struct char_data *i;
    int             sum = 0;

    rp = roomFindNum(ch->in_room);

    if (!rp) {
        return (0);
    }
    for (i = rp->people; i; i = i->next_in_room) {
        if (GET_RACE(i) == GET_RACE(ch)) {
            sum += GetMaxLevel(i);
        }
    }
    return (sum);
}

int too_many_followers(struct char_data *ch)
{
    struct follow_type *k;
    int             max_followers,
                    actual_fol;

    max_followers = (int) chr_apply[(int)GET_CHR(ch)].num_fol;

    for (k = ch->followers, actual_fol = 0; k; k = k->next) {
        if (IS_AFFECTED(k->follower, AFF_CHARM)) {
            actual_fol++;
        }
    }

    return (actual_fol >= max_followers);
}

int follow_time(struct char_data *ch)
{
    int             fol_time = 0;
    fol_time = (int) (24 * GET_CHR(ch) / 11);
    return fol_time;
}

int ItemAlignClash(struct char_data *ch, struct obj_data *obj)
{
    if ((IS_OBJ_STAT(obj, anti_flags, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
        (IS_OBJ_STAT(obj, anti_flags, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
        (IS_OBJ_STAT(obj, anti_flags, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
        return (TRUE);
    }
    return (FALSE);
}

/**
 * @todo remove the ugly kludge Lennya put in
 */
int ItemEgoClash(struct char_data *ch, struct obj_data *obj, int bon)
{
#ifndef USE_EGOS
    return (FALSE);
#else
    int             obj_ego,
                    p_ego,
                    tmp;
    static char    *check[] = { "scroll", "potion", "bag", "tongue", "key" };
    int             i;
    Keywords_t     *key;

    obj_ego = obj->cost_per_day;

    /*
     * dark lord tongue fix kludge, should pretty this up -Lennya
     */
    for( i = 0; i < NELEMENTS(check); i++ ) {
        key = StringToKeywords( check[i], NULL );
        if( KeywordsMatch( key, &obj->keywords ) ) {
            FreeKeywords(key, TRUE);
            return( FALSE );
        }
        FreeKeywords(key, TRUE);
    }

    if (obj_ego >= MAX(LIM_ITEM_COST_MIN, 14000) || obj_ego < 0) {
        if (obj_ego < 0) {
            obj_ego = 50000;
        }
        obj_ego /= 666;

        /*
         * alignment stuff
         */

        if (IS_OBJ_STAT(obj, anti_flags, ITEM_ANTI_GOOD) ||
            IS_OBJ_STAT(obj, anti_flags, ITEM_ANTI_EVIL)) {
            if (IS_NEUTRAL(ch)) {
                obj_ego += obj_ego / 4;
            }
        }

        if (IS_PC(ch)) {
            p_ego = GetMaxLevel(ch) + HowManyClasses(ch);

            if (p_ego > 40) {
                p_ego *= (p_ego - 39);
            } else if (p_ego > 20) {
                p_ego += (p_ego - 20);
            }
        } else {
            p_ego = 10000;
        }

        tmp = GET_INT(ch) + GET_WIS(ch) + GET_CHR(ch);
        tmp /= 3;

        tmp *= GET_HIT(ch);
        tmp /= GET_MAX_HIT(ch);

        return ((p_ego + tmp + bon + number(1, 6)) - (obj_ego + number(1, 6)));
    }

    return (TRUE);
#endif
}

int anti_barbarian_stuff(struct obj_data *obj_object)
{
    if (ITEM_TYPE(obj_object) != ITEM_TYPE_KEY &&
        (IS_OBJ_STAT(obj_object, extra_flags, ITEM_GLOW) ||
         IS_OBJ_STAT(obj_object, extra_flags, ITEM_HUM) ||
         IS_OBJ_STAT(obj_object, extra_flags, ITEM_MAGIC) ||
         IS_OBJ_STAT(obj_object, extra_flags, ITEM_NODROP))) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int CheckGetBarbarianOK(struct char_data *ch, struct obj_data *obj_object)
{
#ifndef BARB_GET_DISABLE
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
        anti_barbarian_stuff(obj_object) && !IS_IMMORTAL(ch)) {
        act("You sense magic on $p and drop it.", FALSE, ch, obj_object, 0,
            TO_CHAR);
        act("$n shakes $s head and refuses to take $p.", FALSE, ch,
            obj_object, 0, TO_ROOM);
        return (FALSE);
    }
#endif
    return (TRUE);
}

int CheckGiveBarbarianOK(struct char_data *ch, struct char_data *vict,
                         struct obj_data *obj)
{
    char            buf[MAX_STRING_LENGTH];

#ifndef BARB_GET_DISABLE
    if (GET_LEVEL(vict, BARBARIAN_LEVEL_IND) != 0 &&
        anti_barbarian_stuff(obj) && !IS_IMMORTAL(vict)) {
        if (GET_POS(vict) <= POSITION_SLEEPING) {
            sprintf(buf, "You think it best to not give this item to %s.\n\r",
                    GET_NAME(vict));
            return (FALSE);
        } else {
            sprintf(buf, "%s senses magic on the object and refuses it!\n\r",
                    GET_NAME(vict));
        }
        send_to_char(buf, ch);
        return (FALSE);
    }
#endif
    return (TRUE);
}

int CheckEgo(struct char_data *ch, struct obj_data *obj)
{
    int             j = 0;

#ifndef USE_EGOS
    return (TRUE);
#else
    if (!obj || !ch) {
        Log("!obj || !ch in CheckEgo, utility.c");
        return (FALSE);
    }

    /*
     * if the item is limited, check its ego. use some funky function to
     * determine if pc's ego is higher than objs' ego.. if it is,
     * proceed.. otherwise, deny.
     */
    if (obj->level != 0) {
        if (obj->level <= GetMaxLevel(ch) || IS_IMMORTAL(ch)) {
            return (TRUE);
        } else {
            act("$p almost seems to say 'You're pretty puny.  I don't want to "
                "be seen with you!\n", 0, ch, obj, 0, TO_CHAR);
            return (FALSE);
        }
    }

    j = ItemEgoClash(ch, obj, 0);
    if (j < -5 && GetMaxLevel(ch) < IMPLEMENTOR) {
        act("$p almost seems to say 'You're much too puny to use me, twerp!'",
            0, ch, obj, 0, TO_CHAR);

        if (obj->in_obj == NULL) {
            act("$p falls to the floor", 0, ch, obj, 0, TO_CHAR);
            act("$p removes itself from $n, and falls to the floor", 0, ch,
                obj, 0, TO_ROOM);
        }
        return (FALSE);
    }

    if (j < 0 && GetMaxLevel(ch) < IMPLEMENTOR) {
        act("$p almost seems to say 'You're pretty puny.  I don't want to be "
            "seen with you!\n", 0, ch, obj, 0, TO_CHAR);

        if (obj->in_obj == NULL) {
            act("$p falls to the floor", 0, ch, obj, 0, TO_CHAR);
            act("$p removes itself from $n, and falls to the floor", 0, ch,
                obj, 0, TO_ROOM);
        }
        return (FALSE);
    }
    return (TRUE);
#endif
}

int CheckGetEgo(struct char_data *ch, struct obj_data *obj)
{
    return (TRUE);
}

int CheckEgoGive(struct char_data *ch, struct char_data *vict,
                 struct obj_data *obj)
{
    int             j = 0;

#ifndef USE_EGOS
    return (TRUE);
#else
    /*
     * if the item is limited, check its ego. use some funky function to
     * determine if pc's ego is higher than objs' ego.. if it is,
     * proceed.. otherwise, deny.
     */
    j = ItemEgoClash(vict, obj, 0);
    if (j < -5 && GetMaxLevel(vict) < IMPLEMENTOR) {
        if (AWAKE(vict)) {
            act("$p almost seems to say 'You're much too puny to use me, "
                "twerp!'", 0, vict, obj, 0, TO_CHAR);
            act("$p refuses to leave $N's hands", 0, vict, obj, ch, TO_CHAR);
        }
        act("$p refuses to be given to $n by $N ", 0, vict, obj, ch, TO_ROOM);
        return (FALSE);
    }

    if (j < 0 && GetMaxLevel(vict) < IMPLEMENTOR) {
        if (AWAKE(vict)) {
            act("$p almost seems to say 'You're pretty puny.  I don't want to "
                "be seen with you!\n", 0, vict, obj, 0, TO_CHAR);
            act("$p refuses to leave $N's hands", 0, vict, obj, ch, TO_CHAR);
        }
        act("$p refuses to be given to $n by $N.", 0, vict, obj, ch, TO_ROOM);
        return (FALSE);
    } else {
        return (TRUE);
    }

    return (FALSE);
#endif
}

int IsSpecialized(int sk_num)
{
    return (IS_SET(sk_num, SKILL_SPECIALIZED));
}


/*
 * this persons max specialized skills
 */
int HowManySpecials(struct char_data *ch)
{
    int             i,
                    ii = 0;

    for (i = 0; i < MAX_SPL_LIST; i++) {
        if (IsSpecialized(ch->skills[i].special)) {
            ii++;
        }
    }
    return (ii);
}

int MAX_SPECIALS(struct char_data *ch)
{
    return (GET_INT(ch));
}

int CanSeeTrap(int num, struct char_data *ch)
{
    if (HasClass(ch, CLASS_THIEF)) {
        return (affected_by_spell(ch, SPELL_FIND_TRAPS) ||
                (ch->skills && num < (ch->skills[SKILL_FIND_TRAP].learned) &&
                 !MOUNTED(ch)));
    }
    if (HasClass(ch, CLASS_RANGER) && OUTSIDE(ch)) {
        return (affected_by_spell(ch, SPELL_FIND_TRAPS) ||
                (ch->skills && num < (ch->skills[SKILL_FIND_TRAP].learned) &&
                 !MOUNTED(ch)));
    }

    if (affected_by_spell(ch, SPELL_FIND_TRAPS) && !MOUNTED(ch))
        return (TRUE);

    return (FALSE);
}

/*
 * this is where we figure the max limited items the char may rent with
 * for his/her current level, pc's wanted it this way. MAX_LIM_ITEMS is in
 * structs.h
 */
int MaxLimited(int lev)
{
    if (lev <= 10) {
        return (3);
    } else if (lev <= 15) {
        return (6);
    } else if (lev <= 20) {
        return (9);
    } else if (lev <= 25) {
        return (11);
    } else if (lev <= 30) {
        return (16);
    } else if (lev <= 35) {
        return (17);
    } else if (lev <= 40) {
        return (18);
    } else if (lev <= 45) {
        return (20);
    } else {
        return (MAX_LIM_ITEMS);
    }
}
int IS_MURDER(struct char_data *ch)
{
#if 0
    char            buf[256];
    if (IS_PC(ch) && IS_SET(ch->player.user_flags, MURDER_1) &&
        !IS_IMMORTAL(ch)) {
        Log("%s has the MURDER set.", GET_NAME(ch));
        return (TRUE);
    }
#endif
    return (FALSE);
}

int IS_STEALER(struct char_data *ch)
{
#if 0
    char            buf[256];

    if (IS_PC(ch) && IS_SET(ch->player.user_flags, STOLE_1) &&
        !IS_IMMORTAL(ch)) {
        Log("%s has STOLE set.", GET_NAME(ch));
        return (TRUE);
    }
#endif
    return (FALSE);
}

int MEMORIZED(struct char_data *ch, int spl)
{
    if (ch->skills[spl].nummem > 0) {
        return (TRUE);
    } else if (ch->skills[spl].nummem < 0) {
        ch->skills[spl].nummem = 0;
    }
    return (FALSE);
}

void FORGET(struct char_data *ch, int spl)
{
    if (ch->skills[spl].nummem) {
        ch->skills[spl].nummem -= 1;
    }
}

/*
 * return the amount max a person can memorize a single spell
 */
int MaxCanMemorize(struct char_data *ch, int spell)
{
    int             BONUS;      /* use this later to figure item bonuses
                                 * or something */

    if (OnlyClass(ch, CLASS_SORCERER)) {
        BONUS = 2;
    } else {
        BONUS = 0;
    }
    /*
     * multies get less...
     */

    if (GET_INT(ch) > 18) {
        /*
         * +1 spell per intel over 18
         */
        BONUS += (GET_INT(ch) - 18);
    }

    if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 4) {
        return (3 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 11) {
        return (3 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 18) {
        return (3 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 21) {
        return (4 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 26) {
        return (4 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 21) {
        return (5 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 23) {
        return (5 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 26) {
        return (6 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 28) {
        return (6 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 31) {
        return (7 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 34) {
        return (7 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 36) {
        return (7 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 41) {
        return (8 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 46) {
        return (9 + BONUS);
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < IMMORTAL) {
        return (10 + BONUS);
    } else {
        /*
         * should never get here, cept for immos
         */
        return ((int) GetMaxLevel(ch) / 10);
    }
}

int IS_LINKDEAD(struct char_data *ch)
{
    if (IS_PC(ch) && !ch->desc) {
        return (TRUE);
    }
    if (IS_SET(ch->specials.act, ACT_POLYSELF) && !ch->desc) {
        return (TRUE);
    }
    return (FALSE);
}

int IS_UNDERGROUND(struct char_data *ch)
{
    struct room_data *rp;

    if ((rp = roomFindNum(ch->in_room)) &&
        IS_SET(zone_table[rp->zone].reset_mode, ZONE_UNDER_GROUND)) {
        return (TRUE);
    }

    return (FALSE);
}

int IsMagicSpell(int spell_num)
{
    int             tmp = FALSE;

    /*
     * using non magic items, since everything else is almost magic
     */
    /*
     * lot smaller switch this way
     */
    switch (spell_num) {
    case SKILL_BACKSTAB:
    case SKILL_SNEAK:
    case SKILL_HIDE:
    case SKILL_PICK_LOCK:
    case SKILL_KICK:
    case SKILL_BASH:
    case SKILL_RESCUE:
    case SKILL_FIRST_AID:
    case SKILL_SIGN:
    case SKILL_RIDE:
    case SKILL_SWITCH_OPP:
    case SKILL_DODGE:
    case SKILL_REMOVE_TRAP:
    case SKILL_RETREAT:
    case SKILL_QUIV_PALM:
    case SKILL_SAFE_FALL:
    case SKILL_FEIGN_DEATH:
    case SKILL_HUNT:
    case SKILL_FIND_TRAP:
    case SKILL_SPRING_LEAP:
    case SKILL_DISARM:
    case SKILL_EVALUATE:
    case SKILL_SPY:
    case SKILL_DOORBASH:
    case SKILL_SWIM:
    case SKILL_CONS_UNDEAD:
    case SKILL_CONS_VEGGIE:
    case SKILL_CONS_DEMON:
    case SKILL_CONS_ANIMAL:
    case SKILL_CONS_REPTILE:
    case SKILL_CONS_PEOPLE:
    case SKILL_CONS_GIANT:
    case SKILL_CONS_OTHER:
    case SKILL_DISGUISE:
    case SKILL_CLIMB:
    case SKILL_BERSERK:
    case SKILL_TAN:
    case SKILL_AVOID_BACK_ATTACK:
    case SKILL_FIND_FOOD:
    case SKILL_FIND_WATER:
    case SPELL_PRAYER:
    case SKILL_MEMORIZE:
    case SKILL_BELLOW:
    case SKILL_DOORWAY:
    case SKILL_PORTAL:
    case SKILL_SUMMON:
    case SKILL_INVIS:
    case SKILL_CANIBALIZE:
    case SKILL_FLAME_SHROUD:
    case SKILL_AURA_SIGHT:
    case SKILL_GREAT_SIGHT:
    case SKILL_PSIONIC_BLAST:
    case SKILL_HYPNOSIS:
    case SKILL_MEDITATE:
    case SKILL_SCRY:
    case SKILL_ADRENALIZE:
    case SKILL_RATION:
    case SKILL_HOLY_WARCRY:
    case SKILL_HEROIC_RESCUE:
    case SKILL_DUAL_WIELD:
    case SKILL_PSI_SHIELD:
    case LANG_COMMON:
    case LANG_ELVISH:
    case LANG_HALFLING:
    case LANG_DWARVISH:
    case LANG_ORCISH:
    case LANG_GIANTISH:
    case LANG_OGRE:
    case LANG_GNOMISH:
    case SKILL_ESP:             /* end skills */

    case TYPE_HIT:              /* weapon types here */
    case TYPE_BLUDGEON:
    case TYPE_PIERCE:
    case TYPE_SLASH:
    case TYPE_WHIP:
    case TYPE_CLAW:
    case TYPE_BITE:
    case TYPE_STING:
    case TYPE_CRUSH:
    case TYPE_CLEAVE:
    case TYPE_STAB:
    case TYPE_SMASH:
    case TYPE_SMITE:
    case TYPE_BLAST:
    case TYPE_SUFFERING:
    case TYPE_RANGE_WEAPON:
    case TYPE_JAB:
    case TYPE_PUNCH:
    case TYPE_STRIKE:
        tmp = FALSE;            /* these are NOT magical! */
        break;

    default:
        tmp = TRUE;             /* default to IS MAGIC */
        break;
    }                           /* end switch */

    return (tmp);
}

int exist(char *s)
{
    int             f;

    f = open(s, O_RDONLY);
    close(f);

    if (f > 0) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int fighting_in_room(int room_n)
{
    struct char_data *ch;
    struct room_data *r;

    r = roomFindNum(room_n);

    if (!r) {
        return FALSE;
    }
    for (ch = r->people; ch; ch = ch->next_in_room) {
        if (ch->specials.fighting) {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * finds oposite direction of a direction.. ex. south->north.. 0->2
 */

int opdir(int dir)
{
    switch (dir) {
    case 0:
    case 1:
        dir = dir + 2;
        break;
    case 2:
    case 3:
        dir = dir - 2;
        break;
    case 4:
        dir = 5;
        break;
    case 5:
        dir = 4;
        break;
    default:
        dir = 0;
        break;
    }
    return dir;
}

/*
 * Used to search for corrupted memory, call before and after suspect
 * functions
 */
void memory_check(char *p)
{
#ifdef DEBUG
    if (!malloc_verify()) {
        /*
         * some systems might not have this lib
         */
        fprintf(stderr, "memory_check failed: %s!\r\n", p);
        fflush(stderr);
        abort();
    }
#endif
}

/*
 * added 2001 (GH)
 */
int bitvector_num(int temp)
{
    return pc_class_num(temp);
}

int pc_class_num(int clss)
{
    int             i,
                    j;

    for (i = 1, j = 0; j <= 31; i <<= 1, j++) {
        if (i == clss) {
            return (j);
        }
    }
    return (0);
}

int num_bitvector(int temp)
{
    return pc_num_class(temp);

}

/*
 * util function, converts an 'advanced' ASCII-number-string into a
 * number. Used by parsebet() but could also be used by do_give or
 * do_wimpy.
 *
 * Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not
 * just numbers. The letters multiply whatever is left of them by 1,000
 * and 1,000,000 respectively. Example:
 *
 * 14k = 14 * 1,000 = 14,000
 * 23m = 23 * 1,000,0000 = 23,000,000
 *
 * If any digits follow the 'k' or 'm', the are also added, but the number
 * which they are multiplied is divided by ten, each time we get one left.
 * This is best illustrated in an example :)
 *
 * 14k42 = 14 * 1000 + 4 * 100 + 2 * 10 = 14420
 *
 * Of course, it only pays off to use that notation when you can skip many
 * 0's. There is not much point in writing 66k666 instead of 66666, except
 * maybe when you want to make sure that you get 66,666.
 *
 * More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are
 * automatically disregarded. Example:
 *
 * 14k1234 = 14,123
 *
 * If the number contains any other characters than digits, 'k' or 'm', the
 * function returns 0. It also returns 0 if 'k' or 'm' appear more than
 * once.
 *
 */

int advatoi(const char *s)
{
    char           *string;   /* a buffer to hold a copy of the argument */
    char           *stringptr; /* a pointer to the buffer so we
                               * can move around */
    int             number = 0; /* number to be returned */
    int             multiplier = 0;     /* multiplier used to get the
                                         * extra digits right */

    /*
     * the pointer to buffer stuff is not really necessary, but originally
     * I modified the buffer, so I had to make a copy of it. What the
     * hell, it works:) (read: it seems to work:)
     */

    if( !s ) {
        return( 0 );
    }

    string = strdup(s);
    if( !string ) {
        return( 0 );
    }
    stringptr = string;

    while (isdigit((int)*stringptr)) {
        /*
         * as long as the current character is a digit
         */

        number = (number * 10) + (int)(*stringptr - 0x30);
        /*
         * advance
         */
        stringptr++;
    }

    switch (UPPER(*stringptr)) {
    case 'K':
        multiplier = 1000;
        number *= multiplier;
        stringptr++;
        break;
    case 'M':
        multiplier = 1000000;
        number *= multiplier;
        stringptr++;
        break;
    case '\0':
        break;
    default:
        free( string );
        return 0;
        /*
         * not k nor m nor NUL - return 0!
         */
    }

    while (isdigit((int)*stringptr) && (multiplier > 1)) {
        /*
         * if any digits follow k/m, add those too
         * the further we get to right, the less are the digit 'worth'
         */
        multiplier = multiplier / 10;
        number = number + ((int)(*stringptr - 0x30) * multiplier);
        stringptr++;
    }

    if (*stringptr != '\0' && !isdigit((int)*stringptr)) {
        /*
         * a non-digit character was found, other than NUL
         * If a digit is found, it means the multiplier is 1 - i.e. extra
         * digits that just have to be ignore, liked 14k4443 -> 3 is ignored
         */
        free( string );
        return 0;
    }

    free( string );
    return (number);
}

/**********************************************************************
*   Function to format big numbers, so you can easy understand it.    *
*    Added by Desden, el Chaman Tibetano (J.L.Sogorb) in Oct-1998     *
*                Email: jlalbatros@mx2.redestb.es                     *
*                                                                                     *
**********************************************************************/

char           *formatNum(int foo)
{
    int             index,
                    index_new,
                    rest;
    char            buf[16];
    static char     buf_new[16];

    sprintf(buf, "%d", foo);
    rest = strlen(buf) % 3;

    for (index = index_new = 0; index < strlen(buf); index++, index_new++) {
        if (index != 0 && (index - rest) % 3 == 0) {
            buf_new[index_new] = ',';
            index_new++;
            buf_new[index_new] = buf[index];
        } else {
            buf_new[index_new] = buf[index];
        }
    }
    buf_new[index_new] = '\0';
    return buf_new;
}

/*
 * TWEAKING OBJECTS - Lennya 20030730
 *
 * The idea is to make items that are loaded at a zone reset, will not
 * always be 100% identical. The object structure needs a new element, the
 * tweak rate (%). This is the chance of the item stats not being regular
 * but tweaked. Tweaking is subject to some regulations. For instance, a
 * +2dam ring will never tweak better than +3dam, or worse than 0dam. To
 * keep things simple, only the special affects can tweak, not Armor Class
 * of armor or damdice/damage type of a weapon.
 */

void tweak(struct obj_data *obj)
{
    int             i = 0,
                    roll = 0,
                    mod = 0;

    dlog("in tweak");

    if (!obj) {
        Log("No object found in tweak func?!");
        return;
    }

    if (IS_OBJ_STAT(obj, extra_flags, ITEM_IMMUNE)) {
        /*
         * don't tweak artifacts
         */
        return;
    }

    /*
     * here goes
     */
    for (i = 0; i <= 4; i++) {
        if (obj->affected[i].location != APPLY_NONE) {
            switch (obj->affected[i].location) {
            case APPLY_STR:
            case APPLY_DEX:
            case APPLY_INT:
            case APPLY_WIS:
            case APPLY_CON:
            case APPLY_CHR:
            case APPLY_HITROLL:
            case APPLY_DAMROLL:
            case APPLY_HITNDAM:
                if (number(0, 4)) {
                    if (number(0, 1)) {
                        /*
                         * good tweak
                         */
                        roll = -1;
                    } else {
                        /*
                         * bad tweak
                         */
                        roll = number(1, 2);
                    }
                    mod = obj->affected[i].modifier - roll;

                    /*
                     * stick to certain limits
                     */
                    if (mod > 3 && ITEM_TYPE(obj) != ITEM_TYPE_WEAPON) {
                        mod = 3;
                    } else if (mod > 5) {
                        mod = 5;
                    }
                    obj->affected[i].modifier = mod;
                }
                break;
            case APPLY_SAVING_PARA:
            case APPLY_SAVING_ROD:
            case APPLY_SAVING_PETRI:
            case APPLY_SAVING_BREATH:
            case APPLY_SAVING_SPELL:
            case APPLY_SAVE_ALL:
                if (number(0, 4)) {
                    if (number(0, 1)) {
                        /*
                         * good tweak
                         */
                        roll = -1;
                    } else {
                        /*
                         * bad tweak
                         */
                        roll = number(1, 2);
                    }
                    obj->affected[i].modifier += roll;
                }
                break;
            case APPLY_AGE:
            case APPLY_MANA:
            case APPLY_HIT:
            case APPLY_MOVE:
            case APPLY_ARMOR:
            case APPLY_BACKSTAB:
            case APPLY_KICK:
            case APPLY_SNEAK:
            case APPLY_HIDE:
            case APPLY_BASH:
            case APPLY_PICK:
            case APPLY_STEAL:
            case APPLY_TRACK:
            case APPLY_FIND_TRAPS:
            case APPLY_RIDE:
            case APPLY_MANA_REGEN:
            case APPLY_HIT_REGEN:
            case APPLY_MOVE_REGEN:
            case APPLY_SPELLFAIL:
                /*
                 * +/- 1..40%
                 */
                if (number(0, 4)) {
                    roll = (number(1, 40));
                    mod = (int) obj->affected[i].modifier * roll / 100;
                    if (number(0, 1)) {
                        obj->affected[i].modifier += mod;
                    } else if (number(0, 1)) {
                        obj->affected[i].modifier -= mod;
                    }
                }
                break;
            default:
                /*
                 * don't change the others
                 */
                break;
            }
        }
    }
}

int MobCastCheck(struct char_data *ch, int psi)
{
    struct char_data *vict;

    if (!ch) {
        Log("no char in MobCastCheck");
        return (FALSE);
    }

    if (ch->in_room < 1) {
        Log("invalid room in MobCastCheck");
        return (FALSE);
    }

    if (!(roomFindNum(ch->in_room))) {
        Log("non-existant room for MobCastCheck");
        return (FALSE);
    }

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Muh?\n\r", ch);
        act("$n seems to want to cast a spell, but nothing sensible comes out.",
            FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    }

    if (affected_by_spell(ch, SPELL_SILENCE) && !psi) {
        send_to_char("You can't cast, you're silenced!\n\r", ch);
        act("$n seems to want to cast a spell, but $s voice seems to fail $m.",
            FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    }

    if (affected_by_spell(ch, SPELL_ANTI_MAGIC_SHELL)) {
        act("Your magic fizzles against your anti-magic shell!", FALSE, ch,
            0, 0, TO_CHAR);
        act("$n tries to cast a spell within a anti-magic shell, muhahaha!",
            FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    }

    if (ch->specials.fighting && (vict = ch->specials.fighting) &&
        affected_by_spell(vict, SPELL_ANTI_MAGIC_SHELL)) {
        act("Your magic fizzles against $N's anti-magic shell!",
            FALSE, ch, 0, vict, TO_CHAR);
        act("$n wastes a spell on $N's anti-magic shell!", FALSE,
            ch, 0, vict, TO_NOTVICT);
        act("$n casts a spell and growls as it fizzles against your "
            "anti-magic shell!", FALSE, ch, 0, vict, TO_VICT);
        return (FALSE);
    }
    return (TRUE);
}

/*
 * @Name: CountPeople in zone Function @description: This function counts
 * the number of people in a zone. @Author: Greg Hovey (Banon) @Used: used
 * in chestproc and preparationproc
 */
int countPeople(int zonenr)
{
    int             count = 0;
    struct descriptor_data *d;
    struct char_data *person;

    for (d = descriptor_list; d; d = d->next) {
        person = (d->original ? d->original : d->character);

        if (person && roomFindNum(person->in_room) &&
            roomFindNum(person->in_room)->zone == zonenr) {
            count++;
        }
    }

    return count;
}

int count_People_in_room(int room)
{
    struct char_data *i;
    int             count = 0;

    if (roomFindNum(room)) {
        for (i = roomFindNum(room)->people; i; i = i->next_in_room) {
            if (i) {
                /*
                 * this counts the number of people in just this room.
                 */
                count++;
            }
        }
    }
    return count;
}

char           *skip_word(char *string)
{
    for (; *string && !isspace((int)*string); string++) {
        /* 
         * Empty loop 
         */
    }

    return (string);
}

/*
 * determine if a given string is an abbreviation of another
 */
int is_abbrev(char *arg1, char *arg2)
{
    if (!arg1 || !*arg1 || !arg2 || !*arg2) {
        return (0);
    }

    if( strncasecmp(arg1, arg2, strlen(arg1)) ) {
        /* No match */
        return( 0 );
    }
    return (1);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
