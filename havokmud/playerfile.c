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
extern struct index_data *mob_index;
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
            obj = objectRead(st->objects[i].item_number, VIRTUAL);
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


void obj_store_to_room(int room, struct obj_file_u *st)
{
    struct obj_data *obj;
    int             i,
                    j;
    struct index_data *index;

    for (i = 0; i < st->number; i++) {
        if (st->objects[i].item_number > -1) {
            obj = objectRead(st->objects[i].item_number, VIRTUAL);
            if (IS_RARE(obj)) {
                obj->index->number--;
            }
            obj->value[0] = st->objects[i].value[0];
            obj->value[1] = st->objects[i].value[1];
            obj->value[2] = st->objects[i].value[2];
            obj->value[3] = st->objects[i].value[3];
            obj->extra_flags = st->objects[i].extra_flags;
            obj->weight = st->objects[i].weight;
            obj->timer = st->objects[i].timer;
            obj->bitvector = st->objects[i].bitvector;

            /*
             * new, saving names and descrips stuff o_s_t_r 
             */
            if (obj->name) {
                free(obj->name);
            }
            if (obj->short_description) {
                free(obj->short_description);
            }
            if (obj->description) {
                free(obj->description);
            }
            obj->name = (char *) malloc(strlen(st->objects[i].name) + 1);
            obj->short_description =
                (char *) malloc(strlen(st->objects[i].sd) + 1);
            obj->description =
                (char *) malloc(strlen(st->objects[i].desc) + 1);

            strcpy(obj->name, st->objects[i].name);
            strcpy(obj->short_description, st->objects[i].sd);
            strcpy(obj->description, st->objects[i].desc);
            /*
             * end of new, possibly buggy stuff 
             */

            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                obj->affected[j] = st->objects[i].affected[j];
            }
            obj_to_room2(obj, room);
        }
    }
}

void load_room_objs(int room)
{
    FILE           *fl;
    struct obj_file_u st;
    char            buf[200];

    sprintf(buf, "world/%d", room);

    /*
     * r+b is for Binary Reading/Writing 
     */
    if (!(fl = fopen(buf, "r+b"))) {
        Log("Room has no equipment");
        return;
    }

    rewind(fl);

    if (!ReadObjs(fl, &st)) {
        Log("No objects found");
        fclose(fl);
        return;
    }

    fclose(fl);

    obj_store_to_room(room, &st);
    save_room(room);
}

void save_room(int room)
{

#ifdef SAVEWORLD

    struct obj_file_u st;
    struct obj_data *obj;
    struct room_data *rm = 0;
    char            buf[255];
    static int      last_room = -1;
    static FILE    *f1 = 0;

    rm = real_roomp(room);

    obj = rm->contents;
    sprintf(buf, "world/%d", room);
    st.number = 0;

    if (obj) {
        if (room != last_room) {
            if (f1) {
                fclose(f1);
            }
            f1 = fopen(buf, "w");
        }

        if (!f1) {
            return;
        }
        rewind(f1);
        obj_to_store(obj, &st, NULL, 0);
        sprintf(buf, "Room %d", room);
        strcpy(st.owner, buf);
        st.gold_left = 0;
        st.total_cost = 0;
        st.last_update = 0;
        st.minimum_stay = 0;
        WriteObjs(f1, &st);
    }
#endif
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
