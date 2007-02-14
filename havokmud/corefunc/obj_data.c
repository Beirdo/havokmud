/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
 *
 *  havokmud is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*HEADER---------------------------------------------------
* $Id$
*
* Copyright 2007 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "structs.h"
#include "newstructs.h"
#include "utils.h"
#include "logging.h"
#include "interthread.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

#define GET_OBJ_NAME(obj)  ((obj)->name)

#define REAL 0
#define VIRTUAL 1

struct obj_data *object_list = NULL;       /* the global linked list of obj's */
long            obj_count = 0;
FILE           *obj_f;          /* obj prototypes */
struct index_data *obj_index;   /* index table for object file */
int             top_of_objt = 0;        /* top of object index table */
int             top_of_sort_objt = 0;
int             top_of_alloc_objt = 99999;

struct index_data *insert_objindex(struct index_data *index, void *data,
                                   long vnum);
char           *fread_string(FILE * f1);
int real_object(int virtual);
void clear_object(struct obj_data *obj);



void insert_object(struct obj_data *obj, long vnum)
{
    obj_index = insert_objindex(obj_index, (void *) obj, vnum);
}

struct index_data *insert_objindex(struct index_data *index, void *data,
                                   long vnum)
{
    if (top_of_objt >= top_of_alloc_objt) {
        if (!(index = (struct index_data *)realloc(index,
                      (top_of_objt + 50) * sizeof(struct index_data)))) {
            perror("load indices");
            assert(0);
        }
        top_of_alloc_objt += 50;
    }
    index[top_of_objt].virtual = vnum;
    index[top_of_objt].pos = -1;
    index[top_of_objt].name = strdup(GET_OBJ_NAME((struct obj_data *) data));
    index[top_of_objt].number = 0;
    index[top_of_objt].func = 0;
    top_of_objt++;
    return index;
}

#if 0
    Log("Generating index table for objects.");
    obj_index = db_generate_object_index(&top_of_objt, &top_of_sort_objt,
                                         &top_of_alloc_objt);

    if( !obj_index ) {
        Log( "No objects in the SQL Database, aborting" );
        exit( 0 );
    }
#endif


/*
 * generate index table for object or monster file
 */
struct index_data *generate_indices(FILE * fl, int *top, int *sort_top,
                                    int *alloc_top, char *dirname)
{
    FILE           *f;
    DIR            *dir;
    struct index_data *index = NULL;
    struct dirent  *ent;
    long            i = 0,
                    di = 0,
                    vnum,
                    j;
    long            bc = MAX_INDICES;
    long            dvnums[MAX_INDICES];
    char            buf[82],
                    tbuf[128];

    /*
     * scan main obj file
     */
    rewind(fl);
    for (;;) {
        if (fgets(buf, sizeof(buf), fl)) {
            if (*buf == '#') {
                if (!i) {
                    /*
                     * first cell
                     */
                    CREATE(index, struct index_data, bc);
                } else if (i >= bc) {
                    if (!(index = (struct index_data *) realloc(index,
                                      (i + 50) * sizeof(struct index_data)))) {
                        perror("load indices");
                        assert(0);
                    }
                    bc += 50;
                }
                sscanf(buf, "#%ld", &index[i].virtual);
                sprintf(tbuf, "%s/%ld", dirname, index[i].virtual);
                if ((f = fopen(tbuf, "rt")) == NULL) {
                    index[i].pos = ftell(fl);
                    index[i].name = (index[i].virtual < 99999) ?
                                     fread_string(fl) : strdup("omega");
                } else {
                    index[i].pos = -1;
                    fscanf(f, "#%*d\n");
                    index[i].name = (index[i].virtual < 99999) ?
                                     fread_string(f) : strdup("omega");
                    dvnums[di++] = index[i].virtual;
                    fclose(f);
                }
                index[i].number = 0;
                index[i].func = 0;
                index[i].data = NULL;
                i++;
            } else {
                if (*buf == '%') {
                    /* EOF */
                    break;
                }
            }
        } else {
            fprintf(stderr, "generate indices");
            assert(0);
        }
    }
    *sort_top = i - 1;
    *alloc_top = bc;
    *top = i;

    /*
     * scan for directory entrys
     */
    if ((dir = opendir(dirname)) == NULL) {
        LogPrint( LOG_CRIT, "unable to open index directory %s", dirname);
        return (index);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (*ent->d_name == '.') {
            continue;
        }
        vnum = atoi(ent->d_name);
        if (vnum == 0) {
            continue;
        }
        /*
         * search if vnum was already sorted in main database
         */
        for (j = 0; j < di; j++) {
            if (dvnums[j] == vnum) {
                break;
            }
        }

        if (dvnums[j] == vnum) {
            continue;
        }

        sprintf(buf, "%s/%s", dirname, ent->d_name);
        if ((f = fopen(buf, "rt")) == NULL) {
            LogPrint( LOG_CRIT, "Can't open file %s for reading\n", buf);
            continue;
        }

        if (!i) {
            CREATE(index, struct index_data, bc);
        } else if (i >= bc) {
            if (!(index = (struct index_data *) realloc(index,
                          (i + 50) * sizeof(struct index_data)))) {
                perror("load indices");
                assert(0);
            }
            bc += 50;
        }

        fscanf(f, "#%*d\n");
        index[i].virtual = vnum;
        index[i].pos = -1;
        index[i].name = (index[i].virtual < 99999) ? fread_string(f) :
                         strdup("omega");
        index[i].number = 0;
        index[i].func = 0;
        index[i].data = NULL;
        fclose(f);
        i++;
    }

    *alloc_top = bc;
    *top = i;
    closedir(dir);

    return (index);
}


void clone_obj_to_obj(struct obj_data *obj, struct obj_data *osrc)
{
    struct extra_descr_data *new_descr,
                   *tmp_descr;
    int             i;

    if (osrc->name) {
        obj->name = strdup(osrc->name);
    }
    if (osrc->short_description) {
        obj->short_description = strdup(osrc->short_description);
    }
    if (osrc->description) {
        obj->description = strdup(osrc->description);
    }
    if (osrc->action_description) {
        obj->action_description = strdup(osrc->action_description);
    }

    /*
     *** numeric data ***
     */

    obj->type_flag = osrc->type_flag;
    obj->extra_flags = osrc->extra_flags;
    obj->wear_flags = osrc->wear_flags;
    obj->value[0] = osrc->value[0];
    obj->value[1] = osrc->value[1];
    obj->value[2] = osrc->value[2];
    obj->value[3] = osrc->value[3];
    obj->weight = osrc->weight;
    obj->cost = osrc->cost;
    obj->cost_per_day = osrc->cost_per_day;

    /*
     *** extra descriptions ***
     */

    obj->ex_description = 0;

    if (osrc->ex_description) {
        for (tmp_descr = osrc->ex_description; tmp_descr;
             tmp_descr = tmp_descr->next) {
            CREATE(new_descr, struct extra_descr_data, 1);
            if (tmp_descr->keyword) {
                new_descr->keyword = strdup(tmp_descr->keyword);
            }
            if (tmp_descr->description) {
                new_descr->description = strdup(tmp_descr->description);
            }
            new_descr->next = obj->ex_description;
            obj->ex_description = new_descr;
        }
    }

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        obj->affected[i].location = osrc->affected[i].location;
        obj->affected[i].modifier = osrc->affected[i].modifier;
    }
}


/*
 * read an object from OBJ_FILE
 */
struct obj_data *read_object(int nr, int type)
{
    struct obj_data *obj;
    int             i;
    char            buf[100];

    i = nr;
    if (type == VIRTUAL) {
        nr = real_object(nr);
    }
    if (nr < 0 || nr > top_of_objt) {
        sprintf(buf, "Object (V) %d does not exist in database.", i);
        return (0);
    }

    CREATE(obj, struct obj_data, 1);

    if (!obj) {
        LogPrintNoArg( LOG_CRIT, "Cannot create obj?! db.c read_obj");
        return(NULL);
    }

    clear_object(obj);

    if( !db_read_object(obj, obj_index[nr].virtual, -1, -1) ) {
        free(obj);
        return(NULL);
    }

    obj->in_room = NOWHERE;
    obj->next_content = 0;
    obj->carried_by = 0;
    obj->equipped_by = 0;
    obj->eq_pos = -1;
    obj->in_obj = 0;
    obj->contains = 0;
    obj->item_number = nr;
    obj->in_obj = 0;

    obj->next = object_list;
    object_list = obj;

    obj_index[nr].number++;
    obj_count++;
    return (obj);
}

/*
 * release memory allocated for an obj struct
 */
void free_obj(struct obj_data *obj)
{
    struct extra_descr_data *this,
                   *next_one;

    if (!obj) {
        LogPrintNoArg(LOG_CRIT, "!obj in free_obj, db.c");
        return;
    }
    if (obj->name && *obj->name) {
        free(obj->name);
    }
    if (obj->description && *obj->description) {
        free(obj->description);
    }
    if (obj->short_description && *obj->short_description) {
        free(obj->short_description);
    }
    if (obj->action_description && *obj->action_description) {
        free(obj->action_description);
    }
    if (obj->modBy && *obj->modBy) {
        free(obj->modBy);
    }
    for (this = obj->ex_description; (this != 0); this = next_one) {
        next_one = this->next;
        if (this->keyword) {
            free(this->keyword);
        }
        if (this->description) {
            free(this->description);
        }
        free(this);
    }

    if (obj) {
        free(obj);
    }
}

void clear_object(struct obj_data *obj)
{
    memset(obj, 0, sizeof(struct obj_data));
    obj->item_number = -1;
    obj->in_room = NOWHERE;
    obj->eq_pos = -1;
}

/*
 * returns the real number of the object with given virtual number
 */
int real_object(int virtual)
{
    long            bot,
                    top,
                    mid;

    bot = 0;
    top = top_of_sort_objt;
    /*
     * perform binary search on obj-table
     */
    for (;;) {
        mid = (bot + top) / 2;

        if ((obj_index + mid)->virtual == virtual) {
            return (mid);
        }
        if (bot >= top) {
            /*
             * start unsorted search now
             */
            for (mid = top_of_sort_objt; mid < top_of_objt; mid++) {
                if ((obj_index + mid)->virtual == virtual) {
                    return (mid);
                }
            }
            return (-1);
        }
        if ((obj_index + mid)->virtual > virtual) {
            top = mid - 1;
        } else {
            bot = mid + 1;
        }
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
