/*
 * To calc last time online...
 *  computeminuteslast = (time(0)-st.last_update)/SECS_PER_REAL_MIN;
 *  computehourslast = (time(0)-st.last_update)/SECS_PER_REAL_HOUR;
 *  The last time you were on was %d minutes (%d hrs) ago according to rent.
 , computeminuteslast, computehourslast
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include "protos.h"

#define OBJ_SAVE_FILE "pcobjs.obj"
#define OBJ_FILE_FREE "\0\0\0"

extern struct room_data *world;
extern int      top_of_objt;
extern struct player_index_element *player_table;
extern int      top_of_p_table;

int             cur_depth = 0;

void WriteObjs(FILE * fl, struct obj_file_u *st);
int ReadObjs(FILE * fl, struct obj_file_u *st);
int             find_name(char *name);


/*
 ************************************************************************
 * General save/load routines                                              *
 ************************************************************************* */


/*
 ************************************************************************
 * Routines used to load a characters equipment from disk                  *
 ************************************************************************* */




/*
 ************************************************************************
 * Routines used to save a characters equipment from disk                  *
 ************************************************************************* */



/*
 ************************************************************************
 * Routines used to update object file, upon boot time                     *
 ************************************************************************* */

void update_obj_file(void)
{
    FILE           *fl,
                   *char_file;
    struct obj_file_u st;
    struct char_file_u ch_st;
    long            i;
    long            days_passed,
                    secs_lost;
    char            buf[200];


    if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
        perror("Opening player file for reading. (reception.c, "
               "update_obj_file)");
        assert(0);
    }

    for (i = 0; i <= top_of_p_table; i++) {
        sprintf(buf, "rent/%s", lower(player_table[i].name));
        /*
         * r+b is for Binary Reading/Writing 
         */
        if ((fl = fopen(buf, "r+b")) != '\0' && ReadObjs(fl, &st)) {
            if (strcasecmp(st.owner, player_table[i].name) != 0) {
                Log("Ack!  Wrong person written into object file! (%s/%s)",
                    st.owner, player_table[i].name);
                abort();
            } else {
                Log("   Processing %s[%ld].", st.owner, i);
                days_passed = (time(0) - st.last_update) / 
                              SECS_PER_REAL_DAY;
                secs_lost = (time(0) - st.last_update) % SECS_PER_REAL_DAY;

                rewind(char_file);
                fseek(char_file, (long) (player_table[i].nr *
                                         sizeof(struct char_file_u)), 0);
                fread(&ch_st, sizeof(struct char_file_u), 1, char_file);

                if (ch_st.load_room == AUTO_RENT) {
                    /* 
                     * this person was autorented 
                     */
                    ch_st.load_room = NOWHERE;

                    /* 
                     * one hour grace period 
                     */
                    st.last_update = time(0) + 3600;

                    Log("   Deautorenting %s", st.owner);

#ifdef LIMITED_ITEMS
                    CountLimitedItems(&st);
#endif
                    rewind(char_file);
                    fseek(char_file, (long) (player_table[i].nr *
                                             sizeof(struct char_file_u)),
                          0);
                    fwrite(&ch_st, sizeof(struct char_file_u), 1,
                           char_file);

                    rewind(fl);
                    WriteObjs(fl, &st);

                    fclose(fl);
                } else if (days_passed > 0) {
                    if ((st.total_cost * days_passed) > st.gold_left) {
                        Log("   Dumping %s from object file.", ch_st.name);

                        ch_st.points.gold = 0;
                        ch_st.load_room = NOWHERE;
                        rewind(char_file);
                        fseek(char_file, 
                              (long) (player_table[i].nr *
                                      sizeof(struct char_file_u)), 0);
                        fwrite(&ch_st, sizeof(struct char_file_u),
                               1, char_file);

                        fclose(fl);
                        ZeroRent(ch_st.name);
                    } else {
                        Log("   Updating %s", st.owner);
                        st.gold_left -= (st.total_cost * days_passed);
                        st.last_update = time(0) - secs_lost;
                        rewind(fl);
                        WriteObjs(fl, &st);
                        fclose(fl);
#ifdef LIMITED_ITEMS
                        CountLimitedItems(&st);
#endif
                    }
                } else {
#ifdef LIMITED_ITEMS
                    CountLimitedItems(&st);
#endif
                    Log("  same day update on %s", st.owner);
                    rewind(fl);
                    WriteObjs(fl, &st);
                    fclose(fl);
                }
            }
        }
    }
    fclose(char_file);
}

void CountLimitedItems(struct obj_file_u *st)
{
    int             i,
                    cost_per_day;
    struct obj_data *obj;
    struct index_data *index;

    if (!st->owner[0]) {
         /* 
          * don't count empty rent units 
          */
        return;
    }

    for (i = 0; i < st->number; i++) {
        if (st->objects[i].item_number > -1) {
            /*
             * eek.. read in the object, and then extract it.
             * (all this just to find rent cost.)  *sigh*
             */
            obj = objectRead(st->objects[i].item_number);
            cost_per_day = obj->cost_per_day;

            /*
             * if the cost is > LIM_ITEM_COST_MIN, 
             * then mark before extractin
             */
            if (IS_RARE(obj)) {
                if (obj->item_number < 0) {
                    abort();
                }
                obj->index->number++;
            }
            objectExtract(obj);
        }
    }
}


/*
 * removes a player from the list of renters 
 */

void zero_rent(struct char_data *ch)
{
    if (IS_NPC(ch)) {
        return;
    }
    ZeroRent(GET_NAME(ch));
}

void ZeroRent(char *n)
{
    FILE           *fl;
    char            buf[200];

    sprintf(buf, "rent/%s", lower(n));

    if (!(fl = fopen(buf, "w"))) {
        perror("saving PC's objects");
        assert(0);
    }

    fclose(fl);
    return;
}

int ReadObjs(FILE * fl, struct obj_file_u *st)
{
    int             i;

    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(st->owner, sizeof(st->owner), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->last_update, sizeof(st->last_update), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->number, sizeof(st->number), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    for (i = 0; i < st->number; i++) {
        fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    }
    return( TRUE );
}

void WriteObjs(FILE * fl, struct obj_file_u *st)
{
    int             i;

    fwrite(st->owner, sizeof(st->owner), 1, fl);
    fwrite(&st->gold_left, sizeof(st->gold_left), 1, fl);
    fwrite(&st->total_cost, sizeof(st->total_cost), 1, fl);
    fwrite(&st->last_update, sizeof(st->last_update), 1, fl);
    fwrite(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    fwrite(&st->number, sizeof(st->number), 1, fl);

    for (i = 0; i < st->number; i++) {
        fwrite(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
