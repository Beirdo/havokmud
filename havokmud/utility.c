#include "config.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/file.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"

#define NEWHELP_FILE      "ADD_HELP"    /* New help to add */
#define QUESTLOG_FILE     "quest_log"   /* Log of quest transactions */


int pc_class_num(int clss);
void load_one_room(FILE * fl, struct room_data *rp);


void            log_sev(char *s, int i);
extern char    *exits[];
extern long     SystemFlags;
#if 0
extern struct time_data time_info;
#endif
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct index_data *mob_index,
               *obj_index;
extern struct chr_app_type chr_apply[];
extern struct zone_data *zone_table;
extern const struct race_type race_list[];

#if HASH
extern struct hash_header room_db;      /* In db.c */
#else
extern struct room_data *room_db[];     /* In db.c */
#endif
extern char    *dirs[];
extern int      RacialMax[][MAX_CLASS];
extern int      top_of_zone_table;
extern int      top_of_world;
extern struct descriptor_data *descriptor_list;

/*
 * external functions
 */
void            stop_fighting(struct char_data *ch);
void            fake_setup_dir(FILE * fl, long room, int dir);
char           *fread_string(FILE * fl);
int             check_falling(struct char_data *ch);
int             spy_flag;
void            NailThisSucker(struct char_data *ch);

/*
 * interal move to protos.h
 */
int             IS_UNDERGROUND(struct char_data *ch);
FILE           *log_f;

void Log(char *s)
{
    log_sev(s, 1);
}


#if EGO
int EgoBladeSave(struct char_data *ch)
{
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
}
#endif

int MIN(int a, int b)
{
    return a < b ? a : b;
}

int MAX(int a, int b)
{
    return a > b ? a : b;
}

int OnlyClassItemValid(struct char_data *ch, struct obj_data *obj)
{
    if ((HasClass(ch, CLASS_MAGIC_USER) || HasClass(ch, CLASS_SORCERER)) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_THIEF) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_WARRIOR) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_CLERIC) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_BARBARIAN) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARBARIAN)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_RANGER) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_PALADIN) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_PSI) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_MONK) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MONK)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_DRUID) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
        return (FALSE);
    }
    if (HasClass(ch, CLASS_NECROMANCER) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NECROMANCER)) {
        return (FALSE);
    }
    return (TRUE);
}

int GetItemClassRestrictions(struct obj_data *obj)
{
    int             total = 0;

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
        total += CLASS_SORCERER;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MAGE)) {
        total += CLASS_MAGIC_USER;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_THIEF)) {
        total += CLASS_THIEF;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_FIGHTER)) {
        total += CLASS_WARRIOR;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_CLERIC)) {
        total += CLASS_CLERIC;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_BARBARIAN)) {
        total += CLASS_BARBARIAN;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_RANGER)) {
        total += CLASS_RANGER;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PALADIN)) {
        total += CLASS_PALADIN;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_PSI)) {
        total += CLASS_PSI;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_MONK)) {
        total += CLASS_MONK;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_DRUID)) {
        total += CLASS_DRUID;
    }

    if (IS_SET(obj->obj_flags.extra_flags, ITEM_ANTI_NECROMANCER)) {
        total += CLASS_NECROMANCER;
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

int CAN_SEE_OBJ(struct char_data *ch, struct obj_data *obj)
{
    int             num = 0;

    if (IS_IMMORTAL(ch)) {
        return (1);
    }
    /*
     * changed the act.info.c, hope this works on traps INSIDE chests
     * etc..
     */

    if (ITEM_TYPE(obj) == ITEM_TRAP && GET_TRAP_CHARGES(obj) > 0) {
        num = number(1, 101);
        if (CanSeeTrap(num, ch)) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (0);
    }
    if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        return (1);
    }
    if (IS_DARK(ch->in_room) && !IS_OBJ_STAT(obj, ITEM_GLOW)) {
        return (0);
    }
    if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) {
        return (1);
    }
    if (IS_OBJ_STAT(obj, ITEM_INVISIBLE)) {
        return (0);
    }
    return (1);
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
    *rpp = real_roomp(exit->to_room);
    return (*rpp != NULL);
}

long MobVnum(struct char_data *c)
{
    if (IS_NPC(c)) {
        return (mob_index[c->nr].virtual);
    } else {
        return (0);
    }
}

long ObjVnum(struct obj_data *o)
{
    if (o->item_number >= 0) {
        return (obj_index[o->item_number].virtual);
    } else {
        return (-1);
    }
}

void Zwrite(FILE * fp, char cmd, int tf, int arg1, int arg2, int arg3,
            char *desc)
{
    char            buf[100];

    if (*desc) {
        sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3,
                desc);
        fputs(buf, fp);
    } else {
        sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3);
        fputs(buf, fp);
    }
}

void RecZwriteObj(FILE * fp, struct obj_data *o)
{
    struct obj_data *t;

    if (ITEM_TYPE(o) == ITEM_CONTAINER) {
        for (t = o->contains; t; t = t->next_content) {
            Zwrite(fp, 'P', 1, ObjVnum(t),
                   obj_index[t->item_number].number, ObjVnum(o),
                   t->short_description);
            RecZwriteObj(fp, t);
        }
    }
}

int SaveZoneFile(FILE * fp, int start_room, int end_room)
{
    struct char_data *p;
    struct obj_data *o;
    struct room_data *room;
    char            cmd,
                    buf[80];
    int             i,
                    j,
                    arg1,
                    arg2,
                    arg3;

    for (i = start_room; i <= end_room; i++) {
        room = real_roomp(i);
        if (!room) {
            continue;
        }

        /*
         *  first write out monsters
         */
        for (p = room->people; p; p = p->next_in_room) {
            if (IS_NPC(p)) {
                cmd = 'M';
                arg1 = MobVnum(p);
                arg2 = mob_index[p->nr].number;
                arg3 = i;
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);

                for (j = 0; j < MAX_WEAR; j++) {
                    if (p->equipment[j] &&
                        p->equipment[j]->item_number >= 0) {
                        cmd = 'E';
                        arg1 = ObjVnum(p->equipment[j]);
                        if (obj_index[p->equipment[j]->item_number].
                                MaxObjCount) {
                            arg2 = obj_index[p->equipment[j]->item_number].
                                MaxObjCount;
                        } else {
                            arg2 = 65535;
                        }
                        arg3 = j;
                        strcpy(buf, p->equipment[j]->short_description);
                        Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
                        RecZwriteObj(fp, p->equipment[j]);
                    }
                }

                for (o = p->carrying; o; o = o->next_content) {
                    if (o->item_number >= 0) {
                        cmd = 'G';
                        arg1 = ObjVnum(o);
                        if (obj_index[o->item_number].MaxObjCount) {
                            arg2 = obj_index[o->item_number].MaxObjCount;
                        } else {
                            arg2 = 65535;
                        }
                        arg3 = 0;
                        strcpy(buf, o->short_description);
                        Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
                        RecZwriteObj(fp, o);
                    }
                }
            }
        }

        /*
         *  write out objects in rooms
         */
        for (o = room->contents; o; o = o->next_content) {
            if (o->item_number >= 0) {
                cmd = 'O';
                arg1 = ObjVnum(o);
                if (obj_index[o->item_number].MaxObjCount) {
                    arg2 = obj_index[o->item_number].MaxObjCount;
                } else {
                    arg2 = 65535;
                }
                arg3 = i;
                strcpy(buf, o->short_description);
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
                RecZwriteObj(fp, o);
            }
        }
        /*
         *  lastly.. doors
         */

        for (j = 0; j < 6; j++) {
            /*
             *  if there is an door type exit, write it.
             */
            if (room->dir_option[j] && room->dir_option[j]->exit_info) {
            /* is a door */
                cmd = 'D';
                arg1 = i;
                arg2 = j;
                arg3 = 0;
                if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
                    arg3 = 1;
                }
                if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
                    arg3 = 2;
                }
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
            }
        }
    }
    fprintf(fp, "S\n");
    return 1;
}

int LoadZoneFile(FILE * fl, int zon)
{
    int             cmd_no = 0,
                    expand,
                    tmp,
                    cc = 22;
    char            buf[81];

    if (zone_table[zon].cmd) {
        free(zone_table[zon].cmd);
    }

    /*
     * read the command table
     */
    cmd_no = 0;
    for (expand = 1; !feof(fl);) {
        if (expand) {
            if (!cmd_no) {
                CREATE(zone_table[zon].cmd, struct reset_com, cc);
            } else if (cmd_no >= cc) {
                cc += 5;
                if (!(zone_table[zon].cmd =
                      (struct reset_com *) realloc(zone_table[zon].cmd,
                                           (cc * sizeof(struct reset_com))))) {
                    perror("reset command load");
                    assert(0);
                }
            }
        }

        expand = 1;

        /* skip blanks */
        fscanf(fl, " ");
        fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

        if (zone_table[zon].cmd[cmd_no].command == 'S') {
            break;
        }
        if (zone_table[zon].cmd[cmd_no].command == '*') {
            expand = 0;
            /* skip command */
            fgets(buf, 80, fl);
            continue;
        }

        fscanf(fl, " %d %d %d", &tmp, &zone_table[zon].cmd[cmd_no].arg1,
               &zone_table[zon].cmd[cmd_no].arg2);
        zone_table[zon].cmd[cmd_no].if_flag = tmp;

        switch (zone_table[zon].cmd[cmd_no].command) {
        case 'M':
        case 'O':
        case 'C':
        case 'E':
        case 'P':
        case 'D':
            fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
            break;
        }
        /* read comment */
        fgets(buf, 80, fl);
        cmd_no++;
    }
    return 1;
}

void CleanZone(int zone)
{
    struct room_data *rp;
    struct char_data *vict,
                   *next_v;
    struct obj_data *obj,
                   *next_o;
    int             start,
                    end,
                    i;

    start = zone ? (zone_table[zone - 1].top + 1) : 0;
    end = zone_table[zone].top;

    for (i = start; i <= end; i++) {
        rp = real_roomp(i);
        if (!rp) {
            continue;
        }
        for (vict = rp->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF))) {
                extract_char(vict);
            }
        }

        for (obj = rp->contents; obj; obj = next_o) {
            next_o = obj->next_content;
            obj_index[obj->item_number].number--;
            /*
             * object
             * maxxing.(GH)
             *
             *
             * Do not clean out corpses, druid trees or quest items. Bit
             * of kludge to avoid deinit getting rid of quest items when a
             * scavenge quest is going on. Maybe make a SYS_NODEINIT? Hard
             * to use for small imms. -Lennya
             */
            if (!IS_CORPSE(obj) && ITEM_TYPE(obj) != ITEM_TREE &&
                !IS_SET(obj->obj_flags.extra_flags, ITEM_QUEST)) {
                extract_obj(obj);
            }
        }
    }
}

int ZoneCleanable(int zone)
{
    struct room_data *rp;
    struct char_data *vict;
    int             start,
                    end,
                    i;

    start = zone ? (zone_table[zone - 1].top + 1) : 0;
    end = zone_table[zone].top;

    for (i = start; i <= end; i++) {
        rp = real_roomp(i);
        if (!rp) {
            return (TRUE);
        }
        for (vict = rp->people; vict; vict = vict->next_in_room) {
            if (!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) {
                return (FALSE);
            }
        }

    }
    return (TRUE);
}

int FindZone(int zone)
{
    int             i;

    for (i = 0; i <= top_of_zone_table; i++) {
        if (zone_table[i].num == zone) {
            break;
        }
    }

    if (zone_table[i].num != zone) {
        return (-1);
    } else {
        return i;
    }
}

FILE           *MakeZoneFile(struct char_data * c, int zone)
{
    char            buf[256];
    FILE           *fp;

    sprintf(buf, "zones/%d.zon", zone);

    if ((fp = fopen(buf, "wt")) != NULL) {
        return (fp);
    } else {
        return (0);
    }
}

FILE           *OpenZoneFile(struct char_data * c, int zone)
{
    char            buf[256];
    FILE           *fp;

    sprintf(buf, "zones/%d.zon", zone);

    if ((fp = fopen(buf, "rt")) != NULL) {
        return (fp);
    } else {
        return (0);
    }
}

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

unsigned IsImmune(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->M_immune));
}

unsigned IsResist(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->immune) && !IS_SET(bit, ch->susc));
}

unsigned IsSusc(struct char_data *ch, int bit)
{
    return (IS_SET(bit, ch->susc) && !IS_SET(bit, ch->immune));
}

/*
 * creates a random number in interval [from;to]
 */
int number(int from, int to)
{
    if (to - from + 1) {
        return ((random() % (to - from + 1)) + from);
    } else {
        return (from);
    }
}

/*
 * simulates dice roll
 */
int dice(int number, int size)
{
    int             r;
    int             sum = 0;

    /*
     * instead of crashing the mud we set it to 1
     */
    if (size <= 0) {
        size = 1;
    }
    for (r = 1; r <= number; r++) {
        sum += ((random() % size) + 1);
    }
    return (sum);
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

    while ((orig + i) && (k < (maxlen - 1))) {
        if ((*(orig + i) == '$')
            && ((orig + i + 1) && (LOWER(*(orig + i + 1)) == 'c'))
            && ((orig + i + 2) && isdigit(*(orig + i + 2)))
            && ((orig + i + 3) && isdigit(*(orig + i + 3)))
            && ((orig + i + 4) && isdigit(*(orig + i + 4)))
            && ((orig + i + 5) && isdigit(*(orig + i + 5)))) {
            i += 6;
        }
        newbuf[k++] = orig[i];

        if (orig + i) {
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

/*
 * returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2
 */
/*
 * scan 'till found different or end of both
 */
int str_cmp(char *arg1, char *arg2)
{
#if 1
    int             chk,
                    i;

    if ((!arg2) || (!arg1)) {
        return (1);
    }
    for (i = 0; *(arg1 + i) || *(arg2 + i); i++) {
        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
            if (chk < 0) {
                return (-1);
            } else {
                return (1);
            }
        }
    }
    return (0);
#else
    return (strcmp(arg1, arg2));
#endif
}

/*
 * returns: 0 if equal, 1 if arg1 > arg2, -1 if arg1 < arg2
 * scan 'till found different, end of both, or n reached
 */
int strn_cmp(char *arg1, char *arg2, int n)
{
#if 1
    int             chk,
                    i;

    for (i = 0; (*(arg1 + i) || *(arg2 + i)) && (n > 0); i++, n--) {
        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
            if (chk < 0) {
                return (-1);
            } else {
                return (1);
            }
        }
    }
    return (0);
#else
    return (strncmp(arg1, arg2, n));
#endif

}

/*
 * writes a string to the log
 */
void log_sev(char *str, int sev)
{
    long            ct;
    char           *tmstr;
    static char     buf[500];
    struct descriptor_data *i;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sprintf(buf, "%s :: %s\n", tmstr, str);

    if (spy_flag) {
        fprintf(stderr, "%s", buf);
    }
    /*
     * My Addon to log into file... useful, he?
     */
    if (!log_f) {
        if (!(log_f = fopen("output.log", "w"))) {
            perror("log_sev");
            return;
        }
    }
    fputs(buf, log_f);
    fflush(log_f);

    if (sev > 1) {
        return;
    }
    if (str) {
        sprintf(buf, "/* %s */\n\r", str);
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected && GetMaxLevel(i->character) >= LOW_IMMORTAL &&
            i->character->specials.sev <= sev &&
            !IS_SET(i->character->specials.act, PLR_NOSHOUT)) {
            SEND_TO_Q(buf, i);
        }
    }
}

void slog(char *str)
{
    log_sev(str, 2);
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

    for (; (*newname = *name); name++, newname++) {
        /*
         * Empty loop
         */
    }
    return (num);
}

int DetermineExp(struct char_data *mob, int exp_flags)
{

    int             base;
    int             phit;
    int             sab;

    if (exp_flags == 0) {
        exp_flags = 1;
    }
    /*
     * reads in the monster, and adds the flags together for simplicity, 1
     * exceptional ability is 2 special abilities
     */

    if (GetMaxLevel(mob) < 0) {
        return (1);
    }
    switch (GetMaxLevel(mob)) {

    case 0:
        base = 5;
        phit = 1;
        sab = 10;
        break;

    case 1:
        base = 10;
        phit = 1;
        sab = 15;
        break;

    case 2:
        base = 20;
        phit = 2;
        sab = 20;
        break;

    case 3:
        base = 35;
        phit = 3;
        sab = 25;
        break;

    case 4:
        base = 60;
        phit = 4;
        sab = 30;
        break;

    case 5:
        base = 90;
        phit = 5;
        sab = 40;
        break;

    case 6:
        base = 150;
        phit = 6;
        sab = 75;
        break;

    case 7:
        base = 225;
        phit = 8;
        sab = 125;
        break;

    case 8:
        base = 600;
        phit = 12;
        sab = 175;
        break;

    case 9:
        base = 900;
        phit = 14;
        sab = 300;
        break;

    case 10:
        base = 1100;
        phit = 15;
        sab = 450;
        break;

    case 11:
        base = 1300;
        phit = 16;
        sab = 700;
        break;

    case 12:
        base = 1550;
        phit = 17;
        sab = 700;
        break;

    case 13:
        base = 1800;
        phit = 18;
        sab = 950;
        break;

    case 14:
        base = 2100;
        phit = 19;
        sab = 950;
        break;

    case 15:
        base = 2400;
        phit = 20;
        sab = 1250;
        break;

    case 16:
        base = 2700;
        phit = 23;
        sab = 1250;
        break;

    case 17:
        base = 3000;
        phit = 25;
        sab = 1550;
        break;

    case 18:
        base = 3500;
        phit = 28;
        sab = 1550;
        break;

    case 19:
        base = 4000;
        phit = 30;
        sab = 2100;
        break;

    case 20:
        base = 4500;
        phit = 33;
        sab = 2100;
        break;

    case 21:
        base = 5000;
        phit = 35;
        sab = 2600;
        break;

    case 22:
        base = 6000;
        phit = 40;
        sab = 3000;
        break;

    case 23:
        base = 7000;
        phit = 45;
        sab = 3500;
        break;

    case 24:
        base = 8000;
        phit = 50;
        sab = 4000;
        break;

    case 25:
        base = 9000;
        phit = 55;
        sab = 4500;
        break;

    case 26:
        base = 10000;
        phit = 60;
        sab = 5000;
        break;

    case 27:
        base = 12000;
        phit = 70;
        sab = 6000;
        break;

    case 28:
        base = 14000;
        phit = 80;
        sab = 7000;
        break;

    case 29:
        base = 16000;
        phit = 90;
        sab = 8000;
        break;

    case 30:
        base = 20000;
        phit = 100;
        sab = 10000;
        break;

    case 32:
    case 33:
    case 34:
    case 31:
        base = 22000;
        phit = 120;
        sab = 12000;
        break;

    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
        base = 32000;
        phit = 140;
        sab = 14000;
        break;

    case 40:
    case 41:
        base = 42000;
        phit = 160;
        sab = 16000;
        break;

    case 42:
    case 43:
        base = 52000;
        phit = 180;
        sab = 20000;
        break;

    case 44:
    case 45:
        base = 72000;
        phit = 200;
        sab = 24000;
        break;

    case 46:
    case 47:
        base = 92000;
        phit = 225;
        sab = 28000;
        break;

    case 48:
    case 49:
        base = 122000;
        phit = 250;
        sab = 32000;
        break;

    case 50:
        base = 150000;
        phit = 275;
        sab = 36000;
        break;

    default:
        base = 200000;
        phit = 300;
        sab = 40000;
        break;

    }

    return (base + (phit * GET_HIT(mob)) + (sab * exp_flags));
}

int GetExpFlags(struct char_data *mob, int exp)
{

    int             base;
    int             phit;
    int             sab;

    /*
     * reads in the monster, and adds the flags together for simplicity, 1
     * exceptional ability is 2 special abilities
     */

    if (GetMaxLevel(mob) < 0)
        return (1);

    switch (GetMaxLevel(mob)) {

    case 0:
        base = 5;
        phit = 1;
        sab = 10;
        break;

    case 1:
        base = 10;
        phit = 1;
        sab = 15;
        break;

    case 2:
        base = 20;
        phit = 2;
        sab = 20;
        break;

    case 3:
        base = 35;
        phit = 3;
        sab = 25;
        break;

    case 4:
        base = 60;
        phit = 4;
        sab = 30;
        break;

    case 5:
        base = 90;
        phit = 5;
        sab = 40;
        break;

    case 6:
        base = 150;
        phit = 6;
        sab = 75;
        break;

    case 7:
        base = 225;
        phit = 8;
        sab = 125;
        break;

    case 8:
        base = 600;
        phit = 12;
        sab = 175;
        break;

    case 9:
        base = 900;
        phit = 14;
        sab = 300;
        break;

    case 10:
        base = 1100;
        phit = 15;
        sab = 450;
        break;

    case 11:
        base = 1300;
        phit = 16;
        sab = 700;
        break;

    case 12:
        base = 1550;
        phit = 17;
        sab = 700;
        break;

    case 13:
        base = 1800;
        phit = 18;
        sab = 950;
        break;

    case 14:
        base = 2100;
        phit = 19;
        sab = 950;
        break;

    case 15:
        base = 2400;
        phit = 20;
        sab = 1250;
        break;

    case 16:
        base = 2700;
        phit = 23;
        sab = 1250;
        break;

    case 17:
        base = 3000;
        phit = 25;
        sab = 1550;
        break;

    case 18:
        base = 3500;
        phit = 28;
        sab = 1550;
        break;

    case 19:
        base = 4000;
        phit = 30;
        sab = 2100;
        break;

    case 20:
        base = 4500;
        phit = 33;
        sab = 2100;
        break;

    case 21:
        base = 5000;
        phit = 35;
        sab = 2600;
        break;

    case 22:
        base = 6000;
        phit = 40;
        sab = 3000;
        break;

    case 23:
        base = 7000;
        phit = 45;
        sab = 3500;
        break;

    case 24:
        base = 8000;
        phit = 50;
        sab = 4000;
        break;

    case 25:
        base = 9000;
        phit = 55;
        sab = 4500;
        break;

    case 26:
        base = 10000;
        phit = 60;
        sab = 5000;
        break;

    case 27:
        base = 12000;
        phit = 70;
        sab = 6000;
        break;

    case 28:
        base = 14000;
        phit = 80;
        sab = 7000;
        break;

    case 29:
        base = 16000;
        phit = 90;
        sab = 8000;
        break;

    case 30:
        base = 20000;
        phit = 100;
        sab = 10000;
        break;

    case 32:
    case 33:
    case 34:
    case 31:
        base = 22000;
        phit = 120;
        sab = 12000;
        break;

    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
        base = 32000;
        phit = 140;
        sab = 14000;
        break;

    case 40:
    case 41:
        base = 42000;
        phit = 160;
        sab = 16000;
        break;

    case 42:
    case 43:
        base = 52000;
        phit = 180;
        sab = 20000;
        break;

    case 44:
    case 45:
        base = 72000;
        phit = 200;
        sab = 24000;
        break;

    case 46:
    case 47:
        base = 92000;
        phit = 225;
        sab = 28000;
        break;

    case 48:
    case 49:
        base = 122000;
        phit = 250;
        sab = 32000;
        break;

    case 50:
        base = 150000;
        phit = 275;
        sab = 36000;
        break;

    default:
        base = 200000;
        phit = 300;
        sab = 40000;
        break;

    }

    return ((exp - base - (phit * GET_HIT(mob))) / sab);
}


void down_river(int pulse)
{
    struct char_data *ch,
                   *tmp;
    struct obj_data *obj_object,
                   *next_obj;
    int             rd,
                    or;
    char            buf[80];
    struct room_data *rp;

    if (pulse < 0) {
        return;
    }
    for (ch = character_list; ch; ch = tmp) {
        tmp = ch->next;
        if (!IS_NPC(ch) && ch->in_room != NOWHERE &&
            real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM &&
            !(pulse % (real_roomp(ch->in_room))->river_speed) &&
            real_roomp(ch->in_room)->river_dir <= 5 &&
            real_roomp(ch->in_room)->river_dir >= 0) {

            rd = (real_roomp(ch->in_room))->river_dir;
            for (obj_object = real_roomp(ch->in_room)->contents;
                 obj_object; obj_object = next_obj) {

                next_obj = obj_object->next_content;
                if ((real_roomp(ch->in_room))->dir_option[rd]) {
                    obj_from_room(obj_object);
                    obj_to_room(obj_object,
                            real_roomp(ch->in_room)->dir_option[rd]->to_room);
                }
            }

            /*
             * flyers don't get moved
             */
            if (!IS_AFFECTED(ch, AFF_FLYING) && !MOUNTED(ch)) {
                rp = real_roomp(ch->in_room);
                if (rp && rp->dir_option[rd] &&
                    rp->dir_option[rd]->to_room &&
                    (EXIT(ch, rd)->to_room != NOWHERE)) {
                    if (ch->specials.fighting) {
                        stop_fighting(ch);
                    }

                    if (IS_IMMORTAL(ch) &&
                        IS_SET(ch->specials.act, PLR_NOHASSLE)) {
                        send_to_char("The waters swirl beneath your feet.\n\r",
                                     ch);
                    } else {
                        sprintf(buf, "You drift %s...\n\r", dirs[rd]);
                        send_to_char(buf, ch);
                        if (RIDDEN(ch)) {
                            send_to_char(buf, RIDDEN(ch));
                        }

                        or = ch->in_room;
                        char_from_room(ch);
                        if (RIDDEN(ch)) {
                            char_from_room(RIDDEN(ch));
                            char_to_room(RIDDEN(ch),
                                    real_roomp(or)->dir_option[rd]->to_room);
                        }

                        char_to_room(ch,
                                     real_roomp(or)->dir_option[rd]->to_room);

                        do_look(ch, "\0", 15);
                        if (RIDDEN(ch)) {
                            do_look(RIDDEN(ch), "\0", 15);
                        }
                    }
                    if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                        GetMaxLevel(ch) < LOW_IMMORTAL) {
                        if (RIDDEN(ch)) {
                            NailThisSucker(RIDDEN(ch));
                        }
                        NailThisSucker(ch);
                    }
                }
            }
        }
    }
}

void do_WorldSave(struct char_data *ch, char *argument, int cmd)
{
    char            temp[2048],
                    buf[128];
    long            rstart,
                    rend,
                    i,
                    j,
                    k,
                    x;
    struct extra_descr_data *exptr;
    FILE           *fp;
    struct room_data *rp;
    struct room_direction_data *rdd;

    if (!ch->desc) {
        return;
    }
    rstart = 0;
    rend = top_of_world;

    if ((fp = fopen("tinyworld.wld.new", "w")) == NULL) {
        send_to_char("Can't create .wld file\r\n", ch);
        return;
    }

    sprintf(buf, "%s resorts the world (The game will pause for a few "
                 "moments).\r\n", ch->player.name);
    send_to_all(buf);

    sprintf(buf, "Saving World (%ld rooms)\n\r", (long) top_of_world);
    send_to_char(buf, ch);

    for (i = rstart; i <= rend; i++) {
        rp = real_roomp(i);
        if (rp == NULL) {
            continue;
        }
        /*
         * strip ^Ms from description
         */
        x = 0;

        if (!rp->description) {
            CREATE(rp->description, char, 8);
            strcpy(rp->description, "Empty");
        }

        for (k = 0; k <= strlen(rp->description); k++) {
            if (rp->description[k] != 13) {
                temp[x++] = rp->description[k];
            }
        }
        temp[x] = '\0';

        if (temp[0] == '\0') {
            strcpy(temp, "Empty");
        }

        fprintf(fp, "#%ld\n%s~\n%s~\n", rp->number, rp->name, temp);
        if (!rp->tele_targ) {
            fprintf(fp, "%ld %ld %ld", rp->zone, rp->room_flags,
                    rp->sector_type);
        } else if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
            fprintf(fp, "%ld %ld -1 %d %d %d %ld", rp->zone,
                    rp->room_flags, rp->tele_time, rp->tele_targ,
                    rp->tele_mask, rp->sector_type);
        } else {
            fprintf(fp, "%ld %ld -1 %d %d %d %d %ld", rp->zone,
                    rp->room_flags, rp->tele_time, rp->tele_targ,
                    rp->tele_mask, rp->tele_cnt, rp->sector_type);
        }

        if (rp->sector_type == SECT_WATER_NOSWIM) {
            fprintf(fp, " %d %d", rp->river_speed, rp->river_dir);
        }

        if (rp->room_flags & TUNNEL) {
            fprintf(fp, " %d ", rp->moblim);
        }

        fprintf(fp, "\n");

        for (j = 0; j < 6; j++) {
            rdd = rp->dir_option[j];
            if (rdd) {
                fprintf(fp, "D%ld\n", j);

                if (rdd->general_description && *rdd->general_description) {
                    if (strlen(rdd->general_description) > 0) {
                        temp[0] = '\0';
                        x = 0;

                        for (k = 0; k <= strlen(rdd->general_description);
                             k++) {
                            if (rdd->general_description[k] != 13) {
                                temp[x++] = rdd->general_description[k];
                            }
                        }
                        temp[x] = '\0';
                        fprintf(fp, "%s~\n", temp);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                if (rdd->keyword) {
                    if (strlen(rdd->keyword) > 0) {
                        fprintf(fp, "%s~\n", rdd->keyword);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                fprintf(fp, "%ld ", rdd->exit_info);
                fprintf(fp, "%ld ", rdd->key);
                fprintf(fp, "%ld ", rdd->to_room);
                fprintf(fp, "%ld", rdd->open_cmd);
                fprintf(fp, "\n");
            }
        }

        /*
         * extra descriptions..
         */

        for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
            x = 0;

            if (exptr->description) {
                for (k = 0; k <= strlen(exptr->description); k++) {
                    if (exptr->description[k] != 13) {
                        temp[x++] = exptr->description[k];
                    }
                }
                temp[x] = '\0';

                fprintf(fp, "E\n%s~\n%s~\n", exptr->keyword, temp);
            }
        }

        fprintf(fp, "S\n");

    }
    fclose(fp);

    sprintf(buf, "The world returns to normal as %s finishes the job.\r\n",
            ch->player.name);
    send_to_all(buf);
    send_to_char("\n\rDone\n\r", ch);

    return;
}

void RoomSave(struct char_data *ch, long start, long end)
{
    int             countrooms = 0;
    char            fn[80],
                    temp[2048],
                    dots[500];
    int             rstart,
                    rend,
                    i,
                    j,
                    k,
                    x;
    struct extra_descr_data *exptr;
    FILE           *fp = NULL;
    struct room_data *rp;
    struct room_direction_data *rdd;

    rstart = start;
    rend = end;

    if (rstart <= -1 || rend <= -1 || rstart > WORLD_SIZE ||
        rend > WORLD_SIZE) {
        send_to_char("I don't know those room #s.  make sure they are all\n\r",
                     ch);
        send_to_char("contiguous.\n\r", ch);
        fclose(fp);
        return;
    }

    send_to_char("Saving\n", ch);
    strcpy(dots, "\0");

    for (i = rstart; i <= rend; i++) {
        rp = real_roomp(i);
        if (rp == NULL) {
            continue;
        }
        sprintf(fn, "rooms/%d", i);
        if ((fp = fopen(fn, "w")) == NULL) {
            send_to_char("Can't write to disk now..try later \n\r", ch);
            return;
        }
        countrooms++;
        strcat(dots, ".");

        /*
         * strip ^Ms from description
         */
        x = 0;

        if (!rp->description) {
            CREATE(rp->description, char, 8);
            strcpy(rp->description, "Empty");
        }

        for (k = 0; k <= strlen(rp->description); k++) {
            if (rp->description[k] != 13) {
                temp[x++] = rp->description[k];
            }
        }
        temp[x] = '\0';

        if (temp[0] == '\0') {
            strcpy(temp, "Empty");
        }

        fprintf(fp, "#%ld\n%s~\n%s~\n", rp->number, rp->name, temp);
        if (!rp->tele_targ) {
            fprintf(fp, "%ld %ld %ld", rp->zone, rp->room_flags,
                    rp->sector_type);
        } else {
            if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
                fprintf(fp, "%ld %ld -1 %d %d %d %ld", rp->zone,
                        rp->room_flags, rp->tele_time, rp->tele_targ,
                        rp->tele_mask, rp->sector_type);
            } else {
                fprintf(fp, "%ld %ld -1 %d %d %d %d %ld", rp->zone,
                        rp->room_flags, rp->tele_time, rp->tele_targ,
                        rp->tele_mask, rp->tele_cnt, rp->sector_type);
            }
        }
        if (rp->sector_type == SECT_WATER_NOSWIM) {
            fprintf(fp, " %d %d", rp->river_speed, rp->river_dir);
        }

        if (rp->room_flags & TUNNEL) {
            fprintf(fp, " %d ", (int) rp->moblim);
        }

        fprintf(fp, "\n");

        for (j = 0; j < 6; j++) {
            rdd = rp->dir_option[j];
            if (rdd) {
                fprintf(fp, "D%d\n", j);

                if (rdd->general_description && *rdd->general_description) {
                    if (strlen(rdd->general_description) > 0) {
                        temp[0] = '\0';
                        x = 0;

                        for (k = 0; k <= strlen(rdd->general_description);
                             k++) {
                            if (rdd->general_description[k] != 13) {
                                temp[x++] = rdd->general_description[k];
                            }
                        }
                        temp[x] = '\0';

                        fprintf(fp, "%s~\n", temp);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                if (rdd->keyword) {
                    if (strlen(rdd->keyword) > 0) {
                        fprintf(fp, "%s~\n", rdd->keyword);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                fprintf(fp, "%ld ", rdd->exit_info);
                fprintf(fp, "%ld ", rdd->key);
                fprintf(fp, "%ld ", rdd->to_room);
                fprintf(fp, "%ld", rdd->open_cmd);
                fprintf(fp, "\n");
            }
        }

        /*
         * extra descriptions..
         */

        for (exptr = rp->ex_description; exptr; exptr = exptr->next) {
            x = 0;

            if (exptr->description) {
                for (k = 0; k <= strlen(exptr->description); k++) {
                    if (exptr->description[k] != 13)
                        temp[x++] = exptr->description[k];
                }
                temp[x] = '\0';

                fprintf(fp, "E\n%s~\n%s~\n", exptr->keyword, temp);
            }
        }

        fprintf(fp, "S\n");
        fclose(fp);
    }

    send_to_char(dots, ch);
    sprintf(dots, "\n\rDone: %d rooms saved\n\r", countrooms);
    send_to_char(dots, ch);
}

void RoomLoad(struct char_data *ch, int start, int end)
{
    FILE           *fp;
    int             vnum,
                    found = TRUE;
    char            buf[80];
    struct room_data *rp;

    send_to_char("Searching and loading rooms\n\r", ch);

    for (vnum = start; vnum <= end; vnum++) {
        sprintf(buf, "rooms/%d", vnum);
        if (!(fp = fopen(buf, "r"))) {
            found = FALSE;
            send_to_char(".", ch);
            continue;
        }

        fscanf(fp, "#%*d\n");
        if (!(rp = real_roomp(vnum))) {
            /*
             * empty room
             */
            rp = (void *) malloc(sizeof(struct room_data));
            if (rp) {
                bzero(rp, sizeof(struct room_data));
            }
#if HASH
            /*
             * this still does not work and needs work by someone
             */
            room_enter(&room_db, vnum, rp);
#else
            room_enter(room_db, vnum, rp);
#endif
            send_to_char("+", ch);
        } else {
            if (rp->people) {
                act("$n reaches down and scrambles reality.", FALSE, ch,
                    NULL, rp->people, TO_ROOM);
            }
            cleanout_room(rp);
            send_to_char("-", ch);
        }

        rp->number = vnum;
        load_one_room(fp, rp);
        fclose(fp);
    }

    if (!found) {
        send_to_char("\n\rThe room number(s) that you specified could not all "
                     "be found.\n\r", ch);
    } else {
        send_to_char("\n\rDone.\n\r", ch);
    }
}

void fake_setup_dir(FILE * fl, long room, int dir)
{
    int             tmp;
    char           *temp;

    /* descr */
    temp = fread_string(fl);
    if (temp) {
        free(temp);
    }
    /* key */
    temp = fread_string(fl);
    if (temp) {
        free(temp);
    }
    fscanf(fl, " %d ", &tmp);
    fscanf(fl, " %d ", &tmp);
    fscanf(fl, " %d ", &tmp);
}

int IsHumanoid(struct char_data *ch)
{
    /*
     * these are all very arbitrary
     */
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_ROCK_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_AVARIEL:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ORC:
    case RACE_LYCANTH:
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_GOBLIN:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_VEGMAN:
    case RACE_MFLAYER:
    case RACE_ENFAN:
    case RACE_PATRYN:
    case RACE_SARTAN:
    case RACE_ROO:
    case RACE_SMURF:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
    case RACE_TYTAN:
    case RACE_DROW:
    case RACE_GOLEM:
    case RACE_DEMON:
    case RACE_DRAAGDIM:
    case RACE_ASTRAL:
    case RACE_GOD:
    case RACE_HALF_ELF:
    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
    case RACE_GNOLL:
        return (TRUE);
        break;

    default:
        return (FALSE);
        break;
    }
}

int HasWings(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_BIRD:
    case RACE_AVARIEL:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
        return (TRUE);
        break;
    default:
        return (FALSE);
    }
}

int IsRideable(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (IS_NPC(ch) && !IS_PC(ch)) {
        switch (GET_RACE(ch)) {
        case RACE_HORSE:
        case RACE_DRAGON:
        case RACE_DRAGON_RED:
        case RACE_DRAGON_BLACK:
        case RACE_DRAGON_GREEN:
        case RACE_DRAGON_WHITE:
        case RACE_DRAGON_BLUE:
        case RACE_DRAGON_SILVER:
        case RACE_DRAGON_GOLD:
        case RACE_DRAGON_BRONZE:
        case RACE_DRAGON_COPPER:
        case RACE_DRAGON_BRASS:
            return (TRUE);
            break;
        default:
            return (FALSE);
            break;
        }
    }
    return (FALSE);
}

int IsAnimal(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_PREDATOR:
    case RACE_FISH:
    case RACE_BIRD:
    case RACE_HERBIV:
    case RACE_REPTILE:
    case RACE_LABRAT:
    case RACE_ROO:
    case RACE_INSECT:
    case RACE_ARACHNID:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsVeggie(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_PARASITE:
    case RACE_SLIME:
    case RACE_TREE:
    case RACE_VEGGIE:
    case RACE_VEGMAN:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsUndead(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_UNDEAD:
    case RACE_GHOST:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
    case RACE_UNDEAD_GHOUL:

        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsLycanthrope(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_LYCANTH:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsDiabolic(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsReptile(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_REPTILE:
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_DINOSAUR:
    case RACE_SNAKE:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
    case RACE_SKEXIE:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int HasHands(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (IsHumanoid(ch)) {
        return (TRUE);
    }
    if (IsUndead(ch)) {
        return (TRUE);
    }
    if (IsLycanthrope(ch)) {
        return (TRUE);
    }
    if (IsDiabolic(ch)) {
        return (TRUE);
    }
    if (GET_RACE(ch) == RACE_GOLEM || GET_RACE(ch) == RACE_SPECIAL) {
        return (TRUE);
    }
    return (FALSE);
}

int IsPerson(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_HUMAN:
    case RACE_MOON_ELF:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_HALFLING:
    case RACE_ROCK_GNOME:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_GOBLIN:
    case RACE_ORC:
    case RACE_AVARIEL:
        return (TRUE);
        break;

    default:
        return (FALSE);
        break;

    }
}

int IsGiantish(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_ENFAN:

    case RACE_GOBLIN:           /* giantish for con's only... */
    case RACE_ORC:

    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_TYTAN:
    case RACE_TROLL:
    case RACE_DRAAGDIM:

    case RACE_HALF_ORC:
    case RACE_HALF_OGRE:
    case RACE_HALF_GIANT:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

int IsSmall(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_SMURF:
    case RACE_ROCK_GNOME:
    case RACE_HALFLING:
    case RACE_GOBLIN:
    case RACE_ENFAN:
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
        return (TRUE);
    default:
        return (FALSE);
    }
}

int IsGiant(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_HALF_GIANT:
    case RACE_TYTAN:
    case RACE_GOD:
        return (TRUE);
    default:
        return (FALSE);
    }
}

int IsExtraPlanar(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_PLANAR:
    case RACE_ELEMENT:
    case RACE_ASTRAL:
    case RACE_GOD:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}
int IsOther(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_MFLAYER:
    case RACE_SPECIAL:
    case RACE_GOLEM:
    case RACE_ELEMENT:
    case RACE_PLANAR:
    case RACE_LYCANTH:
        return (TRUE);
    default:
        return (FALSE);
        break;
    }
}

int IsDarkrace(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DROW:
    case RACE_DARK_DWARF:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int IsGodly(struct char_data *ch)
{
    if (!ch) {
        return (FALSE);
    }
    if (GET_RACE(ch) == RACE_GOD) {
        return (TRUE);
    }
    if (GET_RACE(ch) == RACE_DEMON || GET_RACE(ch) == RACE_DEVIL) {
        if (GetMaxLevel(ch) >= 45) {
            return (TRUE);
        }
    }
    return( FALSE );
}

int IsDragon(struct char_data *ch)
{

    if (!ch) {
        return (FALSE);
    }
    switch (GET_RACE(ch)) {
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

void SetHunting(struct char_data *ch, struct char_data *tch)
{
    int             persist,
                    dist;

#if NOTRACK
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
    if (GetMaxLevel(tch) >= IMMORTAL) {
        sprintf(buf, ">>%s is hunting you from %s\n\r",
                (ch->player.short_descr[0] ? ch->player.
                 short_descr : "(null)"),
                (real_roomp(ch->in_room)->
                 name[0] ? real_roomp(ch->in_room)->name : "(null)"));
        send_to_char(buf, tch);
    }
#endif
}

void CallForGuard(struct char_data *ch, struct char_data *vict,
                  int lev, int area)
{
    struct char_data *i;
    int             type1,
                    type2;

    switch (area) {
    case MIDGAARD:
        type1 = 3060;
        type2 = 3069;
        break;
    case NEWTHALOS:
        type1 = 3661;
        type2 = 3682;
        break;
    case TROGCAVES:
        type1 = 21114;
        type2 = 21118;
        break;
    case OUTPOST:
        type1 = 21138;
        type2 = 21139;
        break;
    case PRYDAIN:
        type1 = 6606;
        type2 = 6614;
        break;
    default:
        type1 = 3060;
        type2 = 3069;
        break;
    }

    if (lev == 0) {
        lev = 3;
    }
    for (i = character_list; i && lev > 0; i = i->next) {
        if (IS_NPC(i) && i != ch && !i->specials.fighting) {
            if (mob_index[i->nr].virtual == type1) {
                if (!number(0, 5) && !IS_SET(i->specials.act, ACT_HUNTING) &&
                    vict) {
                    SetHunting(i, vict);
                    lev--;
                }
            } else if (mob_index[i->nr].virtual == type2) {
                if (!number(0, 5) && !IS_SET(i->specials.act, ACT_HUNTING) &&
                    vict) {
                    SetHunting(i, vict);
                    lev -= 2;
                }
            }
        }
    }
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
    switch (num) {
    case 1:
    case 2:
        if (!ch->skills[SKILL_BASH].learned) {
            ch->skills[SKILL_BASH].learned = 10 + GetMaxLevel(ch) * 4;
        }
        do_bash(ch, GET_NAME(ch->specials.fighting), 0);
        break;
    case 3:
        if (ch->equipment[WIELD]) {
            if (!ch->skills[SKILL_DISARM].learned) {
                ch->skills[SKILL_DISARM].learned = 10 + GetMaxLevel(ch) * 4;
            }
            do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
        } else {
            if (!ch->skills[SKILL_KICK].learned) {
                ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
            }
            do_kick(ch, GET_NAME(ch->specials.fighting), 0);
        }
        break;
    case 4:
        if (!ch->skills[SKILL_KICK].learned) {
            ch->skills[SKILL_KICK].learned = 10 + GetMaxLevel(ch) * 4;
        }
        do_kick(ch, GET_NAME(ch->specials.fighting), 0);
        break;
    }
}

void FighterMove(struct char_data *ch)
{
    struct char_data *friend;

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
                    do_rescue(ch, GET_NAME(friend), 0);
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

    if (!ch->skills) {
        SpaceForSkills(ch);
        ch->skills[SKILL_DODGE].learned = GetMaxLevel(ch) + 50;
        SET_BIT(ch->player.class, CLASS_MONK);
    }

    if (!ch->specials.fighting) {
        return;
    }
    if (GET_POS(ch) < POSITION_FIGHTING) {
        if (!ch->skills[SKILL_SPRING_LEAP].learned) {
            ch->skills[SKILL_SPRING_LEAP].learned =
                (GetMaxLevel(ch) * 3) / 2 + 25;
        }
        do_springleap(ch, GET_NAME(ch->specials.fighting), 0);
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
                do_quivering_palm(ch, GET_NAME(ch->specials.  fighting), 0);
                return;
            }

            if (ch->specials.fighting->equipment[WIELD]) {
                if (!ch->skills[SKILL_DISARM].learned) {
                    ch->skills[SKILL_DISARM].learned =
                        (GetMaxLevel(ch) * 3) / 2 + 25;
                }
                do_disarm(ch, GET_NAME(ch->specials.fighting), 0);
                return;
            }

            if (!ch->skills[SKILL_KICK].learned) {
                ch->skills[SKILL_KICK].learned =
                    (GetMaxLevel(ch) * 3) / 2 + 25;
            }
            do_kick(ch, GET_NAME(ch->specials.fighting), 0);
        }
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

int room_of_object(struct obj_data *obj)
{
    if (obj->in_room != NOWHERE) {
        return obj->in_room;
    } else if (obj->carried_by) {
        return obj->carried_by->in_room;
    } else if (obj->equipped_by) {
        return obj->equipped_by->in_room;
    } else if (obj->in_obj) {
        return room_of_object(obj->in_obj);
    } else {
        return NOWHERE;
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

    if (obj_index[o->item_number].virtual == obj_num) {
        total = 1;
    }
    if (ITEM_TYPE(o) == ITEM_CONTAINER) {
        for (i = o->contains; i; i = i->next_content) {
            total += RecCompObjNum(i, obj_num);
        }
    }
    return (total);
}

void RestoreChar(struct char_data *ch)
{
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
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
    char            buf[256],
                    buf2[256];

    if (cmd > 6 || cmd < 1) {
        return (FALSE);
    }
    strcpy(buf, "The guard humiliates you, and block your way.\n\r");
    strcpy(buf2, "The guard humiliates $n, and blocks $s way.");

    if ((IS_NPC(ch) && (IS_POLICE(ch))) || (GetMaxLevel(ch) >= DEMIGOD) ||
        (IS_AFFECTED(ch, AFF_SNEAK))) {
        return (FALSE);
    }
    if (ch->in_room == room && cmd == dir + 1 && !HasClass(ch, class)) {
        act(buf2, FALSE, ch, 0, 0, TO_ROOM);
        send_to_char(buf, ch);
        return TRUE;
    }
    return FALSE;
}

void TeleportPulseStuff(int pulse)
{
    struct char_data *ch;
    struct char_data *next,
                   *tmp,
                   *bk,
                   *n2;
    int             tick = 0,
                    tm;
    struct room_data *rp,
                   *dest;
    struct obj_data *obj_object,
                   *temp_obj;

    /*
     * check_mobile_activity(pulse); Teleport(pulse);
     */


    /* this is dependent on P_M = 3*P_T */
    tm = pulse % PULSE_MOBILE;

    if (tm == 0) {
        tick = 0;
    } else if (tm == PULSE_TELEPORT) {
        tick = 1;
    } else if (tm == PULSE_TELEPORT * 2) {
        tick = 2;
    }

    for (ch = character_list; ch; ch = next) {
        next = ch->next;
        if (IS_MOB(ch)) {
            if (ch->specials.tick == tick && !ch->specials.fighting) {
#if 0
                Log("through here");
#endif
                mobile_activity(ch);
            }
        } else if (IS_PC(ch)) {
            rp = real_roomp(ch->in_room);
            if (rp && rp->tele_targ > 0 && rp->tele_targ != rp->number &&
                rp->tele_time > 0 && !(pulse % rp->tele_time)) {

                dest = real_roomp(rp->tele_targ);
                if (!dest) {
                    Log("invalid tele_targ");
                    continue;
                }

                obj_object = (rp)->contents;
                while (obj_object) {
                    temp_obj = obj_object->next_content;
                    obj_from_room(obj_object);
                    obj_to_room(obj_object, (rp)->tele_targ);
                    obj_object = temp_obj;
                }

                bk = 0;

                while (rp->people) {
                    tmp = rp->people;
                    if (!tmp) {
                        break;
                    }
                    if (tmp == bk) {
                        break;
                    }
                    bk = tmp;

                    /*
                     * the list of people in the room has changed
                     */
                    char_from_room(tmp);
                    char_to_room(tmp, rp->tele_targ);

                    if (IS_SET(TELE_LOOK, rp->tele_mask) && IS_PC(tmp)) {
                        do_look(tmp, "\0", 15);
                    }

                    if (IS_SET(dest->room_flags, DEATH) && !IS_IMMORTAL(tmp)) {
                        if (tmp == next) {
                            next = tmp->next;
                        }
                        NailThisSucker(tmp);
                        continue;
                    }

                    if (dest->sector_type == SECT_AIR) {
                        n2 = tmp->next;
                        if (check_falling(tmp)) {
                            if (tmp == next) {
                                next = n2;
                            }
                        }
                    }
                }

                if (IS_SET(TELE_COUNT, rp->tele_mask)) {
                    /*
                     * reset it for next count
                     */
                    rp->tele_time = 0;
                }

                if (IS_SET(TELE_RANDOM, rp->tele_mask)) {
                    rp->tele_time = number(1, 10) * 100;
                }
            }
        }
    }
}

char           *advicelist[] = {
    "Havok's webpage is located at $c000Whttp://havok.haaksman.org/",
    "Remember, when you want to leave, find an innkeeper to $c0015RENT$c0007 or"
        " ask her for an $c0015OFFER$c0007.(From Temple. $c000Ws2e4neu$c0007)",
    "After earning enough exp to level, remember to see your guildmaster to "
        "gain and learn new skills.",
    "To disable this advice channel, type $c000WSET ADVICE DISABLE",
    "Use the $c000WOOC $c000wcommand to talk out of character.",
    "Remember, the $c000WYELL$c000w command is used for zone communication and"
        " $c000WSHOUT$c000w is a world wide channel.",
    "If you have any ideas for the mud, please use the $c000WIDEA$c000w command"
        " to inform the gods of it.",
    "To see a list of all the socials in the game, type $c000WHELP "
        "SOCIAL$c0007.",
    "West of the Temple is a good place to get experience and some good "
        "starting gear.",
    "East of the Temple is donations. Feel free to grub around there. There is"
        " also another donations in New Thalos",
    "North of the Temple is the hall of rules.  Ignorance of these rules is "
        "unacceptable.",
    "Don't forget to eat and drink or you may die of hunger and thirst.",
    "To see who is grouped, type $c000WGWHO$c0007.",
    "To turn off any certain channel, type $c000WNOOOC, NOSHOUT, NOYELL$c000w,"
        " etc.",
    "If you have any problems, ask for a god and they may assist you at any "
        "time.",
    "Remember, no foul language over public channels please.",
    "To see if anyone is in the arena battling, type$c000W WHOARENA",
    "Karysinya Coach is a quick and easy way to travel the lands. (From "
        "Meeting Square go $c000Wsesu$c000w)",
    "Type $c000WWORLD $c000wto see some interesting facts about Havok.",
    "After level five, $c000WATTRIBUTES $c000wcommand is used to see what "
        "spell affects and stats your character has.",
    "Did you know that you can request immortality if your character gains "
        "200mil XP or more?",
    "Immortals/Gods are the people that rule the mud. Type $c000WWIZLIST$c000w"
        " to see a list of all of them.",
    "Before attacking someone, use the $c000WCONSIDER$c000w command to "
        "determine how strong they are.",
    "To see a list of people in the same zone as you, type "
        "$c000WWHOZONE$c000w.",
    "Some items have ego which means you have to be a certain level to use the"
        " item.",
    "See our discussion board, go 2 east & 1 north of meeting square. "
        "($c000Wlook board, read <message#>$c0007)",
    "At times, you may see our resident bot Seth on, whom will spell you up "
        "and summon you when needed.",
    "Typing $c000WQUIT$c000w will not save your character's equipment. Try "
        "$c0015HELP RENT$c0007.",
    "Bug the immortals to add more help and advice to this list.",
    "Type $c000WNEWS$c000w to see the last news and events that occurred on "
        "Havok",
    "Use $c000WBPROMPT$c000w to set your battle prompt the same way as the "
        "regular $c000WPROMPT$c000w command.",
    "Type $c000WHELP COLOR$c000w to get a list of the color code characters.",
    "You cannot $c000WSHOUT$c000w or use $c000WOOC$c000w until level 2.",
    "If you're curious about the function of a spell, type $c0015HELP SPELL "
        "<spellname>$c0007.",
    "If you're curious about the function of a skill, type $c0015HELP SKILL "
        "<skillname>$c0007.",
    "You may want to read $c0015HELP NEWBIE$c0007 for some useful information "
        "to get you started.",
    "When confused, try $c0015HELP <feature>$c0007, there may be a helpfile "
        "about it!",
    "Some of Havok's monsters are smart. They may attack you, talk to you, "
        "cast spells on you, or help each other out.",
    "In the $c0015ARENA$c0007, players can battle each other. Dont worry, "
        "there are no penalties for dying in the Arena.",
    "Use $c0015GET ALL.<ITEMNAME>$c0007 to pick up all of a specific item in "
        "a room, such as all.coins"
};

void AdvicePulseStuff(int pulse)
{
    int             numberadvice = 37;
    struct descriptor_data *i;
    register struct char_data *ch;
    char            buffer[150];

    if (pulse < 0 || number(0, 1)) {
        return;
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;

            if (IS_PC(ch) && ch->in_room != NOWHERE &&
                IS_SET(ch->player.user_flags, NEW_USER)) {
                sprintf(buffer, "$c000GAdvice: '$c000w%s$c000G'\n\r",
                        advicelist[number(0, numberadvice)]);
                if (AWAKE(ch)) {
                    send_to_char(buffer, ch);
                }
            }
        }
    }
}

void DarknessPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;
    int             j = 0;

    if (pulse < 0 || number(0, 1)) {
        return;
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            for (j = 0; j <= (MAX_WEAR - 1); j++) {
                /*
                 * scrap antisun before doing the darkness check
                 */
                if (ch->equipment[j] && ch->equipment[j]->item_number >= 0 &&
                    IS_SET(ch->equipment[j]->obj_flags.extra_flags,
                           ITEM_ANTI_SUN)) {
                    AntiSunItem(ch, 0, 0, ch->equipment[j], PULSE_COMMAND);
                }
            }

            if (IS_PC(ch) && IsDarkrace(ch) && AWAKE(ch) &&
                !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) &&
                !IS_AFFECTED(ch, AFF_DARKNESS) && !IS_UNDERGROUND(ch) &&
                !IS_DARK(ch->in_room)) {
                act("$n uses $s innate powers of darkness.", FALSE, ch,
                    0, 0, TO_ROOM);
                act("You use your innate powers of darkness.", FALSE,
                    ch, 0, 0, TO_CHAR);
                cast_globe_darkness(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
            }
        }
    }
}

struct obj_data *find_tqp(int tqp_nr)
{
    extern struct obj_data *object_list;
    register struct obj_data *t;
    struct obj_data *tqp = 0;
    int             nr = 0;

    for (t = object_list; t; t = t->next) {
        if (obj_index[t->item_number].virtual == TRAVELQP) {
            nr++;
            if (nr == tqp_nr) {
                tqp = t;
                return (tqp);
            }
        }
    }
    return (NULL);
}

int count_tqp(void)
{
    extern struct obj_data *object_list;
    register struct obj_data *t;
    int             tqp_nr = 0;

    for (t = object_list; t; t = t->next) {
        if (obj_index[t->item_number].virtual == TRAVELQP) {
            tqp_nr++;
        }
    }
    return (tqp_nr);
}

void traveling_qp(int pulse)
{
    char            buf[256];
    struct char_data *ch = NULL,
                   *newch;
    struct room_data *room;
    struct obj_data *travelqp = 0,
                   *qt;
    extern int      qp_patience;
    extern int      top_of_world;
    int             to_room = 0;
    int             k,
                    f;

    qp_patience++;              /* some secs have passed */

    for (k = 1; k <= TQP_AMOUNT; k++) {
        f = 1;
        if (!(travelqp = find_tqp(k))) {
            f = 0;
        } else if (init_counter() < MIN_INIT_TQP) {
            extract_obj(travelqp);
            f = 0;
        } else if (!(ch = travelqp->carried_by)) {
            Log("not carried, extracting");
            extract_obj(travelqp);
            f = 0;
        } else if (!IS_NPC(ch)) {
#if 0
            ch->player.q_points++;
#endif
            send_to_char("You found yourself some booty, and are rewarded by"
                         " the gods with a $c000Rq$c000Yu$c000Ge$c000Bs"
                         "$c000Ct$c000w token.\n\r", ch);
            if ((qt = read_object(QUEST_POTION, VIRTUAL))) {
                obj_to_char(qt, ch);
            }
            Log("carried by player, gained a QT");
            sprintf(buf, "%s just found a quest token.\n\r", GET_NAME(ch));
            qlog(buf);
            extract_obj(travelqp);
            f = 0;
        } else if (qp_patience < 8) {
            /*
             * hasn't been in inventory long enough yet
             */
            f = 0;
        } else if (qp_patience <= 60 && number(k, 3) == 3) {
            /*
             * hasn't been sitting here too long yet
             * 67, 50, 0% chance of not moving
             */
            f = 0;
        }

        /*
         * find a new mob
         */
        if (f) {
            newch = NULL;
            while (!newch) {
                /*
                 * this may cause endless loop, maybe use for(1..100)
                 */
                to_room = number(0, top_of_world);
                room = real_roomp(to_room);
                if (room && (newch = room->people) &&
                    (IS_PC(newch) || newch == ch || newch->specials.fighting ||
                     IS_SET(newch->specials.act, ACT_POLYSELF))) {
                    newch = NULL;
                }
            }

            obj_from_char(travelqp);
            act("$n ceases to be outlined by a $c000Rm$c000Yu$c000Gl$c000Bt"
                "$c000Ci$c000wcolored hue.", FALSE, ch, 0, 0, TO_ROOM);
            obj_to_char(travelqp, newch);
            act("$n is suddenly surrounded by a $c000Rm$c000Yu$c000Gl$c000Bt"
                "$c000Ci$c000wcolored hue!", FALSE, newch, 0, 0, TO_ROOM);
            qp_patience = 0;
        }
    }
}

#define ARENA_ZONE 124
void ArenaPulseStuff(int pulse)
{
    struct descriptor_data *i;
    struct char_data *ch = NULL;
    char            buf[80];
    int             location = 0;
    extern int      MinArenaLevel,
                    MaxArenaLevel;

    if (pulse < 0)
        return;

    if (MinArenaLevel == 0 && MaxArenaLevel == 0 &&
        countPeople(ARENA_ZONE) == 1) {
        /*
         * arena must be closed
         * last one standing in arena!
         * let's see who this is
         */
        for (i = descriptor_list; i; i = i->next) {
            if (!i->connected) {
                ch = i->character;
                if (!ch) {
                    Log("Weirdness. Found a char in arena, but now he's gone?");
                }

                if (IS_PC(ch) && real_roomp(ch->in_room)->zone == ARENA_ZONE) {
                    /*
                     * we have a winner - move and declare
                     */
                    if ((location = ch->player.hometown)) {
                        char_from_room(ch);
                        char_to_room(ch, location);
                        send_to_char("You have won the arena, and are sent "
                                     "back home.\n\r\n\r", ch);
                        act("$n appears in the middle of the room.",
                            TRUE, ch, 0, 0, TO_ROOM);
                        do_look(ch, "", 15);
                        send_to_char("\n\r", ch);
                    }
                    sprintf(buf, "%s has been declared winner of this "
                                 "Arena!!\n\r", GET_NAME(ch));
                    send_to_all(buf);
                }
            }
        }

        if (!ch) {
            Log("Weirdness. Found a char in arena, but now he's gone?");
        }
    }
}

void AuctionPulseStuff(int pulse)
{
    extern int      auct_loop;
    extern long     minbid;
    extern long     intbid;
    extern struct char_data *auctioneer;
    extern struct char_data *bidder;
    struct obj_data *auctionobj;
    char            buf[MAX_STRING_LENGTH];

    if (pulse < 0 || !auctioneer ||
        !(auctionobj = auctioneer->specials.auction)) {
        return;
    }

    switch (auct_loop) {
    case 1:
    case 2:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c.  Minimum bid set at "
                     "$c000w%ld$c000c coins.\n\r",
                auctionobj->short_description, minbid);
        send_to_all(buf);
        auct_loop++;
        break;
    case 3:
        sprintf(buf, "$c000cAuction:  No interest in $c000w%s$c000c.  Item "
                     "withdrawn.\n\r", auctionobj->short_description);
        send_to_all(buf);
        auct_loop = 0;
        intbid = 0;
        minbid = 0;

        /*
         * return item to auctioneer
         */
        auctioneer->specials.auction = 0;

        assert(!auctionobj->in_obj);
        assert(auctionobj->in_room == NOWHERE);
        assert(!auctionobj->carried_by);

        auctionobj->equipped_by = 0;
        auctionobj->eq_pos = -1;

        obj_to_char(auctionobj, auctioneer);
        send_to_char("Your item is returned to you.\n\r.", auctioneer);
        do_save(auctioneer, "", 0);

        auct_loop = 0;
        intbid = 0;
        minbid = 0;
        bidder = 0;
        auctioneer = 0;
        break;

    case 4:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c, current bid of "
                     "$c000w%ld$c000c coins, to $c000w%s$c000c.  Going "
                     "once..\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        send_to_all(buf);
        auct_loop++;
        break;

    case 5:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c, current bid of "
                     "$c000w%ld$c000c coins, to $c000w%s$c000c.  Going "
                     "twice...\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        send_to_all(buf);
        auct_loop++;
        break;

    case 6:
        sprintf(buf, "$c000cAuction:  Gone!  $c000w%s$c000c was sold for "
                     "$c000w%ld$c000c coins to $c000w%s$c000c.\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        send_to_all(buf);

        /*
         * return money to auctioneer
         */
        GET_GOLD(auctioneer) += intbid;
        ch_printf(auctioneer, "You receive %ld coins for the item you "
                              "auctioned.\n\r", intbid);
        /*
         * return item to bidder
         */
        ch_printf(bidder, "You receive %s.\n\r", auctionobj->short_description);
        auctioneer->specials.auction = 0;
        auctionobj->in_obj = 0;
        auctionobj->in_room = -1;
        auctionobj->carried_by = 0;

        auctionobj->equipped_by = 0;
        auctionobj->eq_pos = -1;

        obj_to_char(auctionobj, bidder);

        bidder->specials.minbid = 0;

        do_save(bidder, "", 0);
        do_save(auctioneer, "", 0);

        auct_loop = 0;
        intbid = 0;
        minbid = 0;
        bidder = 0;
        auctioneer = 0;
        break;

    default:
        break;
    }
}

void TrollRegenPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;

    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            if (IS_PC(ch) && GET_RACE(ch) == RACE_TROLL &&
                GET_HIT(ch) < GET_MAX_HIT(ch)) {
                troll_regen(ch);
            }
        }
    }
}

void RiverPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;
    struct char_data *tmp;
    register struct obj_data *obj_object;
    struct obj_data *next_obj;
    int             rd,
                    or;
    char            buf[80],
                    buffer[100];
    struct room_data *rp;

    /*
     * down_river(pulse); MakeSound();
     */

    if (pulse < 0) {
        return;
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            if ((IS_PC(ch) || RIDDEN(ch)) && ch->in_room != NOWHERE &&
                (real_roomp(ch->in_room)->sector_type == SECT_WATER_NOSWIM ||
                 real_roomp(ch->in_room)->sector_type == SECT_UNDERWATER) &&
                real_roomp(ch->in_room)->river_speed > 0 &&
                !(pulse % real_roomp(ch->in_room)->river_speed) &&
                real_roomp(ch->in_room)->river_dir <= 5 &&
                real_roomp(ch->in_room)->river_dir >= 0) {

                rd = real_roomp(ch->in_room)->river_dir;
                for (obj_object = real_roomp(ch->in_room)->contents;
                     obj_object; obj_object = next_obj) {
                    next_obj = obj_object->next_content;
                    if (real_roomp(ch->in_room)->dir_option[rd]) {
                        obj_from_room(obj_object);
                        obj_to_room(obj_object,
                            real_roomp(ch->in_room)->dir_option[rd]->to_room);
                    }
                }

                /*
                 * flyers don't get moved
                 */
                if (IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE)) {
                    send_to_char("The waters swirl and eddy about you.\n\r",
                                 ch);
                } else if ((!IS_AFFECTED(ch, AFF_FLYING) ||
                            real_roomp(ch->in_room)->sector_type ==
                               SECT_UNDERWATER) && !MOUNTED(ch)) {
                    rp = real_roomp(ch->in_room);
                    if (rp && rp->dir_option[rd] &&
                        rp->dir_option[rd]->to_room &&
                        EXIT(ch, rd)->to_room != NOWHERE) {
                        if (ch->specials.fighting) {
                            stop_fighting(ch);
                        }
                        sprintf(buf, "You drift %s...\n\r", dirs[rd]);
                        send_to_char(buf, ch);
                        if (RIDDEN(ch)) {
                            send_to_char(buf, RIDDEN(ch));
                        }
                        or = ch->in_room;
                        char_from_room(ch);
                        if (RIDDEN(ch)) {
                            char_from_room(RIDDEN(ch));
                            char_to_room(RIDDEN(ch),
                                     real_roomp(or)->dir_option[rd]->to_room);
                        }
                        char_to_room(ch,
                                     real_roomp(or)->dir_option[rd]->to_room);
                        do_look(ch, "\0", 15);
                        if (RIDDEN(ch)) {
                            do_look(RIDDEN(ch), "\0", 15);
                        }

                        if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                            GetMaxLevel(ch) < LOW_IMMORTAL) {
                            NailThisSucker(ch);
                            if (RIDDEN(ch)) {
                                NailThisSucker(RIDDEN(ch));
                            }
                        }
                    }
                }
            }
        }
    }

    if (!number(0, 4)) {
        for (ch = character_list; ch; ch = tmp) {
            tmp = ch->next;
            /*
             * mobiles
             */
            if (!IS_PC(ch) && ch->player.sounds && !number(0, 5) &&
                strcmp(ch->player.sounds, "")) {
                /* don't make sound if empty sound string */
                if (ch->specials.default_pos > POSITION_SLEEPING) {
                    if (GET_POS(ch) > POSITION_SLEEPING) {
                        /*
                         * Make the sound
                         */
                        MakeNoise(ch->in_room, ch->player.sounds,
                                  ch->player.distant_snds);
                    } else if (GET_POS(ch) == POSITION_SLEEPING) {
                        /*
                         * snore
                         */
                        sprintf(buffer, "%s snores loudly.\n\r",
                                ch->player.short_descr);
                        MakeNoise(ch->in_room, buffer,
                                  "You hear a loud snore nearby.\n\r");
                    }
                } else if (GET_POS(ch) == ch->specials.default_pos) {
                    /*
                     * Make the sound
                     */
                    MakeNoise(ch->in_room, ch->player.sounds,
                              ch->player.distant_snds);
                }
            }
        }
    }
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

    rp = real_roomp(ch->in_room);

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

    rp = real_roomp(ch->in_room);

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

int MobCountInRoom(struct char_data *list)
{
    int             i;
    struct char_data *tmp;

    for (i = 0, tmp = list; tmp; tmp = tmp->next_in_room, i++) {
        /*
         * Empty loop
         */
    }

    return (i);
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
    int             total = 0;

    if (!obj) {
        return (0);
    }
    if (obj->contains)
        total += CountLims(obj->contains);
    if (obj->next_content)
        total += CountLims(obj->next_content);
    if (IS_RARE(obj)) {
        /*
         * LIM_ITEM_COST_MIN)
         */
        total += 1;
    }
    return (total);
}

struct obj_data *find_a_rare(struct obj_data *obj)
{
    struct obj_data *rare;

    if (!obj) {
        return (NULL);
    }
    if (obj->contains) {
        /*
         * check contents
         */
        rare = find_a_rare(obj->contains);
        if (rare) {
            return (rare);
        }
    }

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

    return (0);
    /*
     * no rares found
     */
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
        send_to_all(buf);
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

void SetRacialStuff(struct char_data *mob)
{
    switch (GET_RACE(mob)) {
    case RACE_BIRD:
        SET_BIT(mob->specials.affected_by, AFF_FLYING);
        break;
    case RACE_FISH:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
        break;
    case RACE_SEA_ELF:
        SET_BIT(mob->specials.affected_by, AFF_WATERBREATH);
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->immune, IMM_CHARM);
    case RACE_MOON_ELF:
    case RACE_DROW:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_AVARIEL:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->immune, IMM_CHARM);
        break;
    case RACE_DWARF:
    case RACE_DARK_DWARF:
    case RACE_DEEP_GNOME:
    case RACE_ROCK_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_MFLAYER:
    case RACE_TROLL:
    case RACE_ORC:
    case RACE_GOBLIN:
    case RACE_HALFLING:
    case RACE_GNOLL:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        break;
    case RACE_INSECT:
    case RACE_ARACHNID:
        if (IS_PC(mob)) {
            GET_STR(mob) = 18;
            GET_ADD(mob) = 100;
        }
        break;
    case RACE_LYCANTH:
        SET_BIT(mob->M_immune, IMM_NONMAG);
        break;
    case RACE_PREDATOR:
        if (mob->skills) {
            mob->skills[SKILL_HUNT].learned = 100;
        }
        break;
    case RACE_GIANT_FROST:
        SET_BIT(mob->M_immune, IMM_COLD);
        SET_BIT(mob->susc, IMM_FIRE);
        break;
    case RACE_GIANT_FIRE:
        SET_BIT(mob->M_immune, IMM_FIRE);
        SET_BIT(mob->susc, IMM_COLD);
        break;
    case RACE_GIANT_CLOUD:
        /*
         * should be gas... but no IMM_GAS
         */
        SET_BIT(mob->M_immune, IMM_SLEEP);
        SET_BIT(mob->susc, IMM_ACID);
        break;
    case RACE_GIANT_STORM:
        SET_BIT(mob->M_immune, IMM_ELEC);
        break;
    case RACE_GIANT_STONE:
        SET_BIT(mob->M_immune, IMM_PIERCE);
        break;
    case RACE_UNDEAD:
    case RACE_UNDEAD_VAMPIRE:
    case RACE_UNDEAD_LICH:
    case RACE_UNDEAD_WIGHT:
    case RACE_UNDEAD_GHAST:
    case RACE_UNDEAD_GHOUL:
    case RACE_UNDEAD_SPECTRE:
    case RACE_UNDEAD_ZOMBIE:
    case RACE_UNDEAD_SKELETON:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        SET_BIT(mob->M_immune, IMM_POISON | IMM_DRAIN | IMM_SLEEP | IMM_HOLD |
                               IMM_CHARM);
        break;
    case RACE_DRAGON_RED:
        SET_BIT(mob->M_immune, IMM_FIRE);
        SET_BIT(mob->susc, IMM_COLD);
        break;
    case RACE_DRAGON_BLACK:
        SET_BIT(mob->M_immune, IMM_ACID);
        break;
    case RACE_DRAGON_GREEN:
        SET_BIT(mob->M_immune, IMM_SLEEP);
        break;
    case RACE_DRAGON_WHITE:
        SET_BIT(mob->M_immune, IMM_COLD);
        SET_BIT(mob->susc, IMM_FIRE);
        break;
    case RACE_DRAGON_BLUE:
        SET_BIT(mob->M_immune, IMM_ELEC);
        break;
    case RACE_DRAGON_SILVER:
        SET_BIT(mob->M_immune, IMM_ENERGY);
        break;
    case RACE_DRAGON_GOLD:
        SET_BIT(mob->M_immune, IMM_SLEEP + IMM_ENERGY);
        break;
    case RACE_DRAGON_BRONZE:
        SET_BIT(mob->M_immune, IMM_COLD + IMM_ACID);
        break;
    case RACE_DRAGON_COPPER:
        SET_BIT(mob->M_immune, IMM_FIRE);
        break;
    case RACE_DRAGON_BRASS:
        SET_BIT(mob->M_immune, IMM_ELEC);
        break;
    case RACE_HALF_ELF:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        SET_BIT(mob->specials.affected_by, AFF_INFRAVISION);
        break;
    default:
        break;
    }

    /*
     * height and weight / Hatred Foes! /
     */
    if (IS_NPC(mob)) {
        switch (GET_RACE(mob)) {
        case RACE_HUMAN:
            break;
        case RACE_MOON_ELF:
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_FOREST_GNOME:
            AddHatred(mob, OP_RACE, RACE_ORC);
            break;
        case RACE_SEA_ELF:
            break;
        case RACE_ROCK_GNOME:
            break;
        case RACE_DEEP_GNOME:
            AddHatred(mob, OP_RACE, RACE_DROW);
        case RACE_DWARF:
            AddHatred(mob, OP_RACE, RACE_GOBLIN);
            AddHatred(mob, OP_RACE, RACE_ORC);
            break;
        case RACE_HALFLING:
            break;
        case RACE_LYCANTH:
            AddHatred(mob, OP_RACE, RACE_HUMAN);
            break;
        case RACE_UNDEAD:
            break;
        case RACE_DARK_DWARF:
        case RACE_UNDEAD_VAMPIRE:
        case RACE_UNDEAD_LICH:
        case RACE_UNDEAD_WIGHT:
        case RACE_UNDEAD_GHAST:
        case RACE_UNDEAD_GHOUL:
        case RACE_UNDEAD_SPECTRE:
            AddHatred(mob, OP_GOOD, 1000);
            break;
        case RACE_UNDEAD_ZOMBIE:
        case RACE_UNDEAD_SKELETON:
        case RACE_VEGMAN:
        case RACE_MFLAYER:
            break;
        case RACE_DROW:
            /*
             * I think this doesn't work!!! - Beirdo
             */
            AddHatred(mob, OP_RACE, RACE_MOON_ELF | RACE_WILD_ELF |
                                    RACE_GOLD_ELF);
            break;
        case RACE_SKEXIE:
        case RACE_TROGMAN:
        case RACE_LIZARDMAN:
        case RACE_SARTAN:
        case RACE_PATRYN:
        case RACE_DRAAGDIM:
        case RACE_ASTRAL:
            break;

        case RACE_HORSE:
            mob->player.weight = 400;
            mob->player.height = 175;
            break;

        case RACE_ORC:
            AddHatred(mob, OP_GOOD, 1000);
            /*
             * I think this doesn't work!!! - Beirdo
             */
            AddHatred(mob, OP_RACE, RACE_MOON_ELF | RACE_GOLD_ELF |
                                    RACE_WILD_ELF);
            mob->player.weight = 150;
            mob->player.height = 140;
            break;

        case RACE_SMURF:
            mob->player.weight = 5;
            mob->player.height = 10;
            break;

        case RACE_GOBLIN:
        case RACE_GNOLL:
            AddHatred(mob, OP_GOOD, 1000);
            AddHatred(mob, OP_RACE, RACE_DWARF);
            break;

        case RACE_ENFAN:
            mob->player.weight = 120;
            mob->player.height = 100;
            break;

        case RACE_LABRAT:
        case RACE_INSECT:
        case RACE_ARACHNID:
        case RACE_REPTILE:
        case RACE_DINOSAUR:
        case RACE_FISH:
        case RACE_PREDATOR:
        case RACE_SNAKE:
        case RACE_HERBIV:
        case RACE_VEGGIE:
        case RACE_ELEMENT:
        case RACE_PRIMATE:
            break;

        case RACE_GOLEM:
            mob->player.weight = 10 + GetMaxLevel(mob) * GetMaxLevel(mob) * 2;
            mob->player.height = 20 + MIN(mob->player.weight, 600);
            break;

        case RACE_DRAGON:
        case RACE_DRAGON_RED:
        case RACE_DRAGON_BLACK:
        case RACE_DRAGON_GREEN:
        case RACE_DRAGON_WHITE:
        case RACE_DRAGON_BLUE:
        case RACE_DRAGON_SILVER:
        case RACE_DRAGON_GOLD:
        case RACE_DRAGON_BRONZE:
        case RACE_DRAGON_COPPER:
        case RACE_DRAGON_BRASS:
            mob->player.weight = MAX(60,
                                     GetMaxLevel(mob) * GetMaxLevel(mob) * 2);
            mob->player.height = 100 + MIN(mob->player.weight, 500);
            break;

        case RACE_BIRD:
        case RACE_PARASITE:
        case RACE_SLIME:
            mob->player.weight = GetMaxLevel(mob) * (GetMaxLevel(mob) / 5);
            mob->player.height = 10 * GetMaxLevel(mob);
            break;

        case RACE_GHOST:
            mob->player.weight = GetMaxLevel(mob) * (GetMaxLevel(mob) / 5);
            mob->player.height = 10 * GetMaxLevel(mob);
            break;

        case RACE_TROLL:
            AddHatred(mob, OP_GOOD, 1000);
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_GIANT:
        case RACE_GIANT_HILL:
        case RACE_GIANT_FROST:
        case RACE_GIANT_FIRE:
        case RACE_GIANT_CLOUD:
        case RACE_GIANT_STORM:
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            AddHatred(mob, OP_RACE, RACE_DWARF);
            break;

        case RACE_DEVIL:
        case RACE_DEMON:
            AddHatred(mob, OP_GOOD, 1000);
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_PLANAR:
            mob->player.height = 200 + GetMaxLevel(mob) * 15;
            mob->player.weight = (int) mob->player.height * 1.5;
            break;

        case RACE_GOD:
        case RACE_TREE:
            break;

        case RACE_TYTAN:
            mob->player.weight = MAX(500,
                                     GetMaxLevel(mob) * GetMaxLevel(mob) * 10);
            mob->player.height = GetMaxLevel(mob) / 2 * 100;
            break;

        case RACE_HALF_ELF:
        case RACE_HALF_OGRE:
        case RACE_HALF_ORC:
        case RACE_HALF_GIANT:
            break;
        }
    }
}

int check_nomagic(struct char_data *ch, char *msg_ch, char *msg_rm)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
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

    rp = real_roomp(ch->in_room);
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

struct char_data *FindMobDiffZoneSameRace(struct char_data *ch)
{
    int             num;
    struct char_data *t;
    struct room_data *rp1,
                   *rp2;

    num = number(1, 100);

    for (t = character_list; t; t = t->next, num--) {
        if (GET_RACE(t) == GET_RACE(ch) && IS_NPC(t) && !IS_PC(t) && !num) {
            rp1 = real_roomp(ch->in_room);
            rp2 = real_roomp(t->in_room);
            if (rp1->zone != rp2->zone) {
                return (t);
            }
        }
    }
    return (NULL);
}

int NoSummon(struct char_data *ch)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
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

int GetNewRace(struct char_file_u *s)
{
    int             return_race,
                    try_again;

    do {
        return_race = number(1, MAX_RACE);

        switch (return_race) {
            /*
             * we allow these races to be used in reincarnations
             */
        case RACE_HUMAN:
        case RACE_MOON_ELF:
        case RACE_DWARF:
        case RACE_HALFLING:
        case RACE_ROCK_GNOME:
        case RACE_FOREST_GNOME:
        case RACE_ORC:
        case RACE_DROW:
        case RACE_MFLAYER:
        case RACE_DARK_DWARF:
        case RACE_DEEP_GNOME:
        case RACE_GNOLL:
        case RACE_GOLD_ELF:
        case RACE_WILD_ELF:
        case RACE_SEA_ELF:
        case RACE_LIZARDMAN:
        case RACE_HALF_ELF:
        case RACE_HALF_OGRE:
        case RACE_HALF_ORC:
        case RACE_HALF_GIANT:
        case RACE_GIANT_HILL:
        case RACE_GIANT_FROST:
        case RACE_GIANT_FIRE:
        case RACE_GIANT_CLOUD:
        case RACE_GIANT_STORM:
        case RACE_ROO:
        case RACE_PRIMATE:
        case RACE_GOBLIN:
        case RACE_TROLL:
        case RACE_AVARIEL:
            try_again = FALSE;
            break;
            /*
             * not a valid race, try again
             */
        default:
            try_again = TRUE;
            break;
        }
    } while (try_again);

    return (return_race);
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
    if (mob_index[horse->nr].virtual == STEED_TEMPLATE &&
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

int EqWBits(struct char_data *ch, int bits)
{
    int             i;

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i] &&
            IS_SET(ch->equipment[i]->obj_flags.extra_flags, bits)) {
            return (TRUE);
        }
    }
    return (FALSE);
}

int InvWBits(struct char_data *ch, int bits)
{
    struct obj_data *o;

    for (o = ch->carrying; o; o = o->next_content) {
        if (IS_SET(o->obj_flags.extra_flags, bits)) {
            return (TRUE);
        }
    }
    return (FALSE);
}

int HasWBits(struct char_data *ch, int bits)
{
    if (EqWBits(ch, bits)) {
        return (TRUE);
    }
    if (InvWBits(ch, bits)) {
        return (TRUE);
    }
    return (FALSE);
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
    if (real_roomp(room_nr) &&
        !IS_SET(zone_table[real_roomp(room_nr)->zone].reset_mode,
                ZONE_ASTRAL)) {
        return (TRUE);
    }

    return (FALSE);
}

int GetSumRaceMaxLevInRoom(struct char_data *ch)
{
    struct room_data *rp;
    struct char_data *i;
    int             sum = 0;

    rp = real_roomp(ch->in_room);

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
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) ||
        (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))) {
        return (TRUE);
    }
    return (FALSE);
}

int ItemEgoClash(struct char_data *ch, struct obj_data *obj, int bon)
{
#ifndef EGO
    return (FALSE);
#else
    int             obj_ego,
                    p_ego,
                    tmp;

    obj_ego = obj->obj_flags.cost_per_day;

    if (strstr(obj->name, "scroll") || strstr(obj->name, "potion") ||
        strstr(obj->name, "bag") || strstr(obj->name, "tongue") ||
        strstr(obj->name, "key")) {
        /*
         * dark lord tongue fix kludge, should pretty this up -Lennya
         */
        return (FALSE);
    }

    if (obj_ego >= MAX(LIM_ITEM_COST_MIN, 14000) || obj_ego < 0) {
        if (obj_ego < 0) {
            obj_ego = 50000;
        }
        obj_ego /= 666;

        /*
         * alignment stuff
         */

        if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) ||
            IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) {
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

void IncrementZoneNr(int nr)
{
    struct char_data *c;
    extern struct char_data *character_list;

    if (nr > top_of_zone_table) {
        return;
    }
    if (nr >= 0) {
        for (c = character_list; c; c = c->next) {
            if (c->specials.zone >= nr) {
                c->specials.zone++;
            }
        }
    } else {
        for (c = character_list; c; c = c->next) {
            if (c->specials.zone >= nr) {
                c->specials.zone--;
            }
        }
    }
}

int IsDarkOutside(struct room_data *rp)
{
    extern int      gLightLevel;

    if (gLightLevel >= 4) {
        return (FALSE);
    }
    if (IS_SET(rp->room_flags, INDOORS) || IS_SET(rp->room_flags, DEATH)) {
        return (FALSE);
    }
    if (rp->sector_type == SECT_FOREST && gLightLevel <= 1) {
        return (TRUE);
    } else if (gLightLevel == 0) {
        return (TRUE);
    }
    return (FALSE);
}

int anti_barbarian_stuff(struct obj_data *obj_object)
{
    if (ITEM_TYPE(obj_object) != ITEM_KEY &&
        (IS_OBJ_STAT(obj_object, ITEM_GLOW) ||
         IS_OBJ_STAT(obj_object, ITEM_HUM) ||
         IS_OBJ_STAT(obj_object, ITEM_MAGIC) ||
         IS_OBJ_STAT(obj_object, ITEM_NODROP))) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int CheckGetBarbarianOK(struct char_data *ch, struct obj_data *obj_object)
{
#ifndef BARB_GET_DISABLE
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
        anti_barbarian_stuff(obj_object) && GetMaxLevel(ch) < LOW_IMMORTAL) {
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
        anti_barbarian_stuff(obj) && GetMaxLevel(vict) < LOW_IMMORTAL) {
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

#if DISABLE_EGO
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

#if DISABLE_EGO
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

int SiteLock(char *site)
{
#if SITELOCK
    int             i,
                    length;
    extern int      numberhosts;
    extern char     hostlist[MAX_BAN_HOSTS][30];

    length = strlen(site);

    for (i = 0; i < numberhosts; i++) {
        if (!strncmp(hostlist[i], site, length)) {
            return (TRUE);
        }
    }
    return (FALSE);
#else
    return (FALSE);
#endif
}

int MaxDexForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_MOON_ELF:
    case RACE_WILD_ELF:
    case RACE_GOLD_ELF:
    case RACE_HALFLING:
    case RACE_FOREST_GNOME:
        return (19);
        break;
    case RACE_DROW:
    case RACE_AVARIEL:
        return (20);
        break;
    case RACE_DWARF:
    case RACE_HALF_OGRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_HILL:
        return (17);
        break;
    case RACE_HALF_GIANT:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
        return (16);
        break;
    default:
        return (18);
        break;
    }
}

int MaxIntForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOLD_ELF:
    case RACE_ROCK_GNOME:
        return (19);
        break;
    case RACE_HALF_GIANT:
    case RACE_HALF_OGRE:
    case RACE_FOREST_GNOME:
        return (17);
        break;

    default:
        return (18);
        break;
    }
}

int MaxWisForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_ROCK_GNOME:
    case RACE_HALF_GIANT:
        return (17);
        break;
    case RACE_FOREST_GNOME:
        return (19);
        break;

    default:
        return (18);
        break;
    }
}

int MaxConForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_HALF_ORC:
    case RACE_DWARF:
    case RACE_HALF_OGRE:
    case RACE_DARK_DWARF:
        return (19);
        break;
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_SEA_ELF:
    case RACE_DROW:
    case RACE_AVARIEL:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

int MaxChrForRace(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_DEEP_GNOME:
        return (16);
        break;
    case RACE_HALF_ORC:
    case RACE_ORC:
    case RACE_DROW:
    case RACE_DWARF:
    case RACE_DARK_DWARF:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

int MaxStrForRace(struct char_data *ch)
{
    if (GetMaxLevel(ch) >= SILLYLORD) {
        return (25);
    }
    switch (GET_RACE(ch)) {
    case RACE_TROLL:
    case RACE_HALF_GIANT:
        return (19);
        break;

    case RACE_HALFLING:
    case RACE_GOBLIN:
        return (17);
        break;
    default:
        return (18);
        break;
    }
}

int IS_MURDER(struct char_data *ch)
{
#if 0
    char            buf[256];
    if (IS_PC(ch) && IS_SET(ch->player.user_flags, MURDER_1) &&
        !IS_IMMORTAL(ch)) {
        sprintf(buf, "%s has the MURDER set.", GET_NAME(ch));
        Log(buf);
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
        sprintf(buf, "%s has STOLE set.", GET_NAME(ch));
        Log(buf);
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
    } else if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 51) {
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
    extern struct zone_data *zone_table;

    if ((rp = real_roomp(ch->in_room)) &&
        IS_SET(zone_table[rp->zone].reset_mode, ZONE_UNDER_GROUND)) {
        return (TRUE);
    }

    return (FALSE);
}

void SetDefaultLang(struct char_data *ch)
{
    int             i;

    switch (GET_RACE(ch)) {
    case RACE_MOON_ELF:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_AVARIEL:
    case RACE_DROW:
        i = LANG_ELVISH;
        break;
    case RACE_TROLL:
    case RACE_HALF_GIANT:
        i = LANG_GIANTISH;
        break;
    case RACE_HALF_OGRE:
        i = LANG_OGRE;
        break;
    case RACE_HALFLING:
        i = LANG_HALFLING;
        break;
    case RACE_DWARF:
        i = LANG_DWARVISH;
        break;
    case RACE_DEEP_GNOME:
    case RACE_FOREST_GNOME:
    case RACE_ROCK_GNOME:
        i = LANG_GNOMISH;
        break;
    default:
        i = LANG_COMMON;
        break;
    }
    ch->skills[i].learned = 95;
    SET_BIT(ch->skills[i].flags, SKILL_KNOWN);
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


/*
 * Good side just means they are of the first races and on the
 * general good side of life, it does NOT refer to alignment
 * only good side people can kill bad side people PC's
 */
int IsGoodSide(struct char_data *ch)
{
    switch (GET_RACE(ch)) {

    case RACE_HUMAN:
    case RACE_MOON_ELF:
    case RACE_AVARIEL:
    case RACE_GOLD_ELF:
    case RACE_WILD_ELF:
    case RACE_SEA_ELF:
    case RACE_DWARF:
    case RACE_HALFLING:
    case RACE_ROCK_GNOME:
    case RACE_HALF_ELF:
    case RACE_HALF_OGRE:
    case RACE_HALF_ORC:
    case RACE_HALF_GIANT:
        return (TRUE);
    }

    return (FALSE);
}

/*
 * this just means the PC is a troll/orc or the like not related to
 * to alignment what-so-ever
 * only bad side people can hit and kill good side people PC's
 */
int IsBadSide(struct char_data *ch)
{
    switch (GET_RACE(ch)) {
    case RACE_GOBLIN:
    case RACE_DARK_DWARF:
    case RACE_ORC:
    case RACE_TROLL:
    case RACE_MFLAYER:
    case RACE_DROW:
        return (TRUE);
    }

    return (FALSE);
}

/*
 * good side can fight bad side people pc to pc fighting only, not used
 * for NPC fighting to pc fighting
 */
int CanFightEachOther(struct char_data *ch, struct char_data *ch2)
{
    if ((IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) ||
        (IS_NPC(ch2) && !IS_SET(ch2->specials.act, ACT_POLYSELF))) {
        return (TRUE);
    }
    if (IS_SET(SystemFlags, SYS_NOKILL)) {
        return (FALSE);
    }
    if (real_roomp(ch->in_room) && real_roomp(ch2->in_room)) {
        if (IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM) &&
            IS_SET(real_roomp(ch2->in_room)->room_flags, ARENA_ROOM)) {
            return (TRUE);
        }
    } else {
        return (FALSE);
    }

    if (IS_SET(SystemFlags, SYS_WLD_ARENA) && IS_AFFECTED2(ch, AFF2_QUEST) &&
        IS_AFFECTED2(ch2, AFF2_QUEST)) {
        return (TRUE);
    }

    if ((IsGoodSide(ch) && IsGoodSide(ch2)) ||
        (IsBadSide(ch) && IsBadSide(ch2))) {
        return (FALSE);
    } else if (IS_SET(ch->player.user_flags, RACE_WAR) &&
               IS_SET(ch2->player.user_flags, RACE_WAR)) {
        return (TRUE);
    }

    return (FALSE);
}

int fighting_in_room(int room_n)
{
    struct char_data *ch;
    struct room_data *r;

    r = real_roomp(room_n);

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

int str_cmp2(char *arg1, char *arg2)
{
    int             chk,
                    i;

    if (!arg2 || !arg1 || !strlen(arg1)) {
        return (1);
    }
    for (i = 0; i < strlen(arg1); i++) {
        if ((chk = LOWER(*(arg1 + i)) - LOWER(*(arg2 + i)))) {
            if (chk < 0) {
                return (-1);
            } else {
                return (1);
            }
        }
    }
    return (0);
}

int CheckSquare(struct char_data *ch, int dir)
{
    int             room = 0;

    if (dir == 0 || dir == 2) {
        /*
         * go east and check
         */
        if ((real_roomp(ch->in_room)->dir_option[1])) {
            room = real_roomp(ch->in_room)->dir_option[1]->to_room;
            if ((real_roomp(room)->dir_option[dir])) {
                room = real_roomp(room)->dir_option[dir]->to_room;
                if ((real_roomp(room)->dir_option[3])) {
                    room = real_roomp(room)->dir_option[3]->to_room;
                    return room;
                }
            }
        }

        if ((real_roomp(ch->in_room)->dir_option[3])) {
            room = real_roomp(ch->in_room)->dir_option[3]->to_room;
            if ((real_roomp(room)->dir_option[dir])) {
                room = real_roomp(room)->dir_option[dir]->to_room;
                if ((real_roomp(room)->dir_option[1])) {
                    room = real_roomp(room)->dir_option[1]->to_room;
                    return room;
                }
            }
        }
    }

    if (dir == 1 || dir == 3) {
        /*
         * go north and check
         */
        if ((real_roomp(ch->in_room)->dir_option[0])) {
            room = real_roomp(ch->in_room)->dir_option[0]->to_room;
            if ((real_roomp(room)->dir_option[dir])) {
                room = real_roomp(room)->dir_option[dir]->to_room;
                if ((real_roomp(room)->dir_option[2])) {
                    room = real_roomp(room)->dir_option[2]->to_room;
                    return room;
                }
            }
        }

        if ((real_roomp(ch->in_room)->dir_option[2])) {
            room = real_roomp(ch->in_room)->dir_option[2]->to_room;
            if ((real_roomp(room)->dir_option[dir])) {
                room = real_roomp(room)->dir_option[dir]->to_room;
                if ((real_roomp(room)->dir_option[0])) {
                    room = real_roomp(room)->dir_option[0]->to_room;
                    return room;
                }
            }
        }
    }

    return NOWHERE;
}

/*
 * Edit Fast rewriten by Greg Hovey(GH) makes an exit to a new room and
 * back
 */
int make_exit_ok(struct char_data *ch, struct room_data **rpp, int dir)
{
    int             current = 0;
    int             x,
                    sector;
    char            buf[255];
    struct zone_data *zd;
    struct room_data *rm = 0,
                   *new_rm = 0;
    int             square = 0;
    if (GetMaxLevel(ch) < 53 || !rpp || !ch->desc ||
        !IS_SET(ch->player.user_flags, FAST_AREA_EDIT))
        return (FALSE);

    current = ch->in_room;
    rm = real_roomp(ch->in_room);
    /*
     * lets find valid room..
     */
    zd = zone_table + (rm->zone - 1);
    sector = rm->sector_type;

    if (GetMaxLevel(ch) < 57 && rm->zone != GET_ZONE(ch)) {
        send_to_char("Sorry, you are not authorized to edit this zone. Get one"
                     " assigned to you.\n\r", ch);
        return (TRUE);
    }

    if (IS_SET(ch->player.user_flags, FAST_MAP_EDIT)) {
        square = CheckSquare(ch, dir);

        if (square != NOWHERE) {
            sprintf(buf, "exit %d 0 0 %d", dir, square);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);

            char_from_room(ch);
            char_to_room(ch, square);
            dir = opdir(dir);

            sprintf(buf, "exit %d 0 0 %d", dir, current);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);
            send_to_char("Reconnecting to existing room\n\r", ch);

            new_rm = real_roomp(ch->in_room);
            new_rm->room_flags = ROOM_WILDERNESS;

            do_look(ch, "", 15);

            return (TRUE);
        }
    }

    x = zd->top;
    zd = zone_table + rm->zone;

    for (x = x + 1; x < zd->top; x++) {
        if (real_roomp(x) == NULL) {
            CreateOneRoom(x);
            sprintf(buf, "$c0001Room exit created from room %d to %d.\n\r",
                    current, x);
            send_to_char(buf, ch);

            sprintf(buf, "exit %d 0 0 %d", dir, x);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);

            /*
             * move char to that room..
             */
            char_from_room(ch);
            char_to_room(ch, x);
            /*
             * opposite direction..
             */
            dir = opdir(dir);

            if (real_roomp(current) == NULL) {
                CreateOneRoom(current);
            }
            new_rm = real_roomp(ch->in_room);
            /*
             * let's set the sector to match the room we came from
             */
            new_rm->sector_type = sector;

            sprintf(buf, "exit %d 0 0 %d", dir, current);
            do_edit(ch, buf, 0);
            /*
             * If wizset map is enabled, set the roomflag MAP_ROOM -Lennya
             */
            if (IS_SET(ch->player.user_flags, FAST_MAP_EDIT)) {
                new_rm = real_roomp(ch->in_room);
                new_rm->room_flags = ROOM_WILDERNESS;
            }
            do_look(ch, "", 15);
            return (TRUE);
        }
    }
    send_to_char("No more empty rooms in your assigned zone!!\n\r", ch);
    return (TRUE);
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
#if DEBUG
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

void dlog(char *s)
{
#if DEBUG_LOG
    slog(s);
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

int pc_num_class(int clss)
{
    return( 1 << clss );
}

char           *DescAge(int age, int race)
{
    if (age > race_list[race].venerable) {
        return "Venerable";
    } else if (age > race_list[race].ancient) {
        return "Ancient";
    } else if (age > race_list[race].old) {
        return "Old";
    } else if (age > race_list[race].middle) {
        return "Middle Aged";
    } else if (age > race_list[race].mature) {
        return "Mature";
    } else if (age > race_list[race].young) {
        return "Young";
    } else {
        return "ERROR";
    }
}

/*
 * Quest Log - Basically responsible for keeping a log of all quest
 * transactions. param desc - This is the data to be entered into the log.
 * By: Greg Hovey.
 */
void qlog(char *desc)
{
    FILE           *fl;
    char            buf[256];
    long            ct;
    char           *tmstr;

    ct = time(0);
    tmstr = asctime(localtime(&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';

    if (!(fl = fopen(QUESTLOG_FILE, "a"))) {
        Log("Could not open the QuestLog-file.\n\r");
    } else {
        sprintf(buf, "**:%s-> %s", tmstr, desc);
        fputs(buf, fl);
        fclose(fl);
    }
}

void do_mrebuild(struct char_data *ch, char *argument, int cmd)
{
    char            buf[128];
    long            m_start,
                    m_end,
                    i,
                    nr;
    FILE           *mob_file;
    FILE           *vnum_f;
    extern int      top_of_mobt;
    struct char_data *mob;
    int             count = 0;

    if (!ch->desc) {
        return;
    }
    m_start = 0;
    m_end = top_of_mobt;

    if ((mob_file = fopen("tinyworld.mob.new", "w")) == NULL) {
        send_to_char("Can't create .mob.new file\r\n", ch);
        return;
    }

    sprintf(buf, "%s resorts the mobiles (The game will pause for a few "
                 "moments).\r\n", ch->player.name);
    send_to_all(buf);

    sprintf(buf, "Saving Mobiles (%ld mobiles)\n\r", (long) m_end);
    send_to_char(buf, ch);

    for (i = m_start; i <= WORLD_SIZE; i++) {
        if ((mob = read_mobile(i, VIRTUAL))) {

            nr = real_mobile(i);

            sprintf(buf, "mobiles/%ld", i);
            if ((vnum_f = fopen(buf, "wt")) == NULL) {
                send_to_char("Can't write to disk now..try later.\n\r", ch);
                return;
            }

            fprintf(vnum_f, "#%ld\n", i);
            save_new_mobile_structure(mob, vnum_f);
            fclose(vnum_f);
            if (nr == -1) {
                insert_mobile(mob, i);
            } else {
                mob_index[nr].pos = -1;
            }
            fprintf(mob_file, "#%ld\n", i);
            save_new_mobile_structure(mob, mob_file);
            count++;
            extract_char(mob);
        }
    }

    fwrite_string(mob_file, "#99999\n%");

    fclose(mob_file);

    sprintf(buf, "The world returns to normal as %s finishes the job.\r\n",
            ch->player.name);
    send_to_all(buf);
    send_to_char("\n\rDone.\n\r", ch);
    sprintf(buf, "(%d) mobiles saved!\n\r", count);
    send_to_char(buf, ch);
}

void do_orebuild(struct char_data *ch, char *argument, int cmd)
{
    char            buf[128],
                    buf2[511];
    long            rstart,
                    rend,
                    i;
    FILE           *fp;
    extern int      top_of_objt;
    struct obj_data *obj;
    int             count = 0;

    if (!ch->desc) {
        return;
    }
    rstart = 0;
    rend = top_of_objt;

    sprintf(buf, "%s resorts the objects (The game will pause for a few "
                 "moments).\r\n", ch->player.name);
    send_to_all(buf);

    sprintf(buf, "Saving Objects (%ld items)\n\r", (long) rend);
    send_to_char(buf, ch);

    for (i = rstart; i <= WORLD_SIZE; i++) {
        obj = read_object(i, VIRTUAL);
        if (obj) {
            sprintf(buf, "objects/%ld", i);
            if (!(fp = fopen(buf, "w"))) {
                sprintf(buf2, "Can't open obj file for %s\r\n", buf);
                send_to_char(buf2, ch);
            } else {
                fprintf(fp, "#%ld\n", i);
                save_new_object_structure(obj, fp);
#if 0
                write_obj_to_file(obj,fp);
#endif
                count++;
                extract_obj(obj);
                fclose(fp);
            }
        }
    }
#if 0
    fwrite_string(fp, "#99999\n\r$~\n\r$~\n\r$~\n\r$~\n\r14 0 1 0 0 0 0 "
                      "0\n\r%");
#endif
    sprintf(buf, "The world returns to normal as %s finishes the job.\r\n",
            ch->player.name);
    send_to_all(buf);
    send_to_char("\n\rDone\n\r", ch);
    sprintf(buf, "(%d) Objects saved!\n\r", count);
    send_to_char(buf, ch);
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
    char            string[MAX_INPUT_LENGTH];   /* a buffer to hold a copy
                                                 * of the argument */
    char           *stringptr = string; /* a pointer to the buffer so we
                                         * can move around */
    char            tempstring[2];      /* a small temp buffer to pass to
                                         * atoi */
    int             number = 0; /* number to be returned */
    int             multiplier = 0;     /* multiplier used to get the
                                         * extra digits right */

    /*
     * the pointer to buffer stuff is not really necessary, but originally
     * I modified the buffer, so I had to make a copy of it. What the
     * hell, it works:) (read: it seems to work:)
     */

    strcpy(string, s);          /* working copy */

    while (isdigit(*stringptr)) {
        /*
         * as long as the current character is a digit
         */

        /*
         * copy first digit
         */
        strncpy(tempstring, stringptr, 1);
        /*
         * add to current * number
         */
        number = (number * 10) + atoi(tempstring);
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
        return 0;
        /*
         * not k nor m nor NUL - return 0!
         */
    }

    while (isdigit(*stringptr) && (multiplier > 1)) {
        /*
         * if any digits follow k/m, add those too
         */
        /*
         * copy first digit
         */
        strncpy(tempstring, stringptr, 1);
        /*
         * the further we get to right, the less are the digit 'worth'
         */
        multiplier = multiplier / 10;
        number = number + (atoi(tempstring) * multiplier);
        stringptr++;
    }

    if (*stringptr != '\0' && !isdigit(*stringptr)) {
        /*
         * a non-digit character was found, other than NUL
         * If a digit is found, it means the multiplier is 1 - i.e. extra
         * digits that just have to be ignore, liked 14k4443 -> 3 is ignored
         */
        return 0;
    }

    return (number);
}

/*
 * Lennya 20030730 A little gadget to convert a string to a floating
 * number. Maybe it already exists, but I couldn't find it. It works,
 * anyway.
 */
float arg_to_float(char *arg)
{
#if 0
    /* There is a system call to do exactly this! */
    char            buf[120];   /* a buffer to hold a copy of the argument */
    float           number = 0.0;       /* number to be returned */
    int             multiplier = 1;     /* multiplier used to get the
                                         * extra digits right */
    int             abs = 0,
                    i = 0,
                    tmp = 0,
                    x = 0,
                    y = 10;

    if (!*arg)
        return (0.0);

    abs = strlen(arg);

    while (isdigit(arg[i])) {
        switch (arg[i]) {
        case '1':
            tmp = 1;
            break;
        case '2':
            tmp = 2;
            break;
        case '3':
            tmp = 3;
            break;
        case '4':
            tmp = 4;
            break;
        case '5':
            tmp = 5;
            break;
        case '6':
            tmp = 6;
            break;
        case '7':
            tmp = 7;
            break;
        case '8':
            tmp = 8;
            break;
        case '9':
            tmp = 9;
            break;
        case '0':
            tmp = 0;
            break;
        default:
            tmp = 0;
            break;
        }
        number = (number * 10) + tmp;
        i++;
    }
    if (arg[i]) {
        if (arg[i] == '\0') {   /* just an integer, not a float */
            return (number);
        }
        if (arg[i] != '.') {    /* this aint no float, mate! */
            return (0.0);
        }
    }
    i++;
    while (isdigit(arg[i])) {
        switch (arg[i]) {
        case '1':
            tmp = 1;
            break;
        case '2':
            tmp = 2;
            break;
        case '3':
            tmp = 3;
            break;
        case '4':
            tmp = 4;
            break;
        case '5':
            tmp = 5;
            break;
        case '6':
            tmp = 6;
            break;
        case '7':
            tmp = 7;
            break;
        case '8':
            tmp = 8;
            break;
        case '9':
            tmp = 9;
            break;
        case '0':
            tmp = 0;
            break;
        default:
            tmp = 0;
            break;
        }
        y = 1;
        for (x = 1; x <= multiplier; x++) {
            y *= 10;
        }
        number = number + (float) tmp / y;
        multiplier++;
        i++;
    }
    return (number);
#else
    return ((float)strtod(arg, NULL));
#endif
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

    if (IS_OBJ_STAT(obj, ITEM_IMMUNE)) {
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
                    if (mod > 3 && ITEM_TYPE(obj) != ITEM_WEAPON) {
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

    if (!(real_roomp(ch->in_room))) {
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

        if (person && real_roomp(person->in_room) &&
            real_roomp(person->in_room)->zone == zonenr) {
            count++;
        }
    }

    return count;
}

int count_People_in_room(int room)
{
    struct char_data *i;
    int             count = 0;

    if (real_roomp(room)) {
        for (i = real_roomp(room)->people; i; i = i->next_in_room) {
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

#if 0
/*
 * put in support for GNU-specific system calls that are missing in Cygwin
 */
#if defined(__CYGWIN__)

size_t strnlen(const char *s, size_t maxlen)
{
    int             i;

    for (i = 0; *s && i < maxlen; s++, i++) {
        /*
         * Empty loop 
         */
    }

    return (i);
}

#endif
#endif



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
