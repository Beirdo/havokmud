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
#include "protos.h"
#include "utils.h"
#include "logging.h"
#include "interthread.h"
#include "balanced_btree.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Handles loading, saving of objects
 */


BalancedBTree_t *objectTree;

struct obj_data *object_list = NULL;       /* the global linked list of obj's */
long            obj_count = 0;

void clear_object(struct obj_data *obj);


void initializeObjects( void )
{
    objectTree = BalancedBTreeCreate( BTREE_KEY_INT );
    db_load_object_tree( objectTree );
}

/**
 * @todo if we put the pointer to the index item into every object, we may
 *       never need to do this search
 */
struct index_data *objectIndex( int vnum )
{
    struct index_data      *index;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( objectTree, &vnum, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    index = (struct index_data *)item->item;
    return( index );
}

/**
 * @todo rename to objectInsert
 */
void insert_object(struct obj_data *obj, long vnum)
{
    BalancedBTreeItem_t    *item;
    struct index_data      *index;

    CREATE(index, struct index_data, 1);
    CREATE(item, BalancedBTreeItem_t, 1);

    index->virtual = vnum;
    index->name = strdup(obj->name);
    index->number = 0;
    index->func = NULL;

    item->key = &index->virtual;
    item->item = (void *)index;
    BalancedBTreeAdd( objectTree, item, UNLOCKED, TRUE );
}


/**
 * @todo rename to objectClone
 */
void clone_obj_to_obj(struct obj_data *obj, struct obj_data *osrc)
{
    struct extra_descr_data *new_descr,
                   *tmp_descr;
    int             i;

    obj->index = osrc->index;

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


/**
 * @todo what is object_list being used for?!
 * @todo rename to objectRead
 */
struct obj_data *read_object(int nr, int type)
{
    struct obj_data    *obj;
    struct index_data  *index;

    index = objectIndex( nr );
    if( !index ) {
        return( NULL );
    }
    
    if( type != 1 ) {
        LogPrint( LOG_CRIT, "read_object(%d, %d) failed", nr, type );
        return( NULL );
    }

    CREATE(obj, struct obj_data, 1);
    if (!obj) {
        LogPrintNoArg( LOG_CRIT, "Cannot create obj?!");
        return(NULL);
    }

    clear_object(obj);

    if( !db_read_object(obj, nr, -1, -1) ) {
        free(obj);
        return(NULL);
    }

    obj->in_room = NOWHERE;
    obj->next_content = NULL;
    obj->carried_by = NULL;
    obj->equipped_by = NULL;
    obj->eq_pos = -1;
    obj->contains = NULL;
    obj->item_number = nr;
    obj->index = index;
    obj->in_obj = NULL;

    obj->next = object_list;
    object_list = obj;

    index->number++;
    obj_count++;
    return (obj);
}

/**
 * @todo rename to objectFree
 * @todo check why we are checking for *obj->name for freeing
 * @todo does this get it out of the object_list or does the caller do that?
 * @brief release memory allocated for an obj struct
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

/**
 * @todo rename to objectClear
 */
void clear_object(struct obj_data *obj)
{
    memset(obj, 0, sizeof(struct obj_data));
    obj->item_number = -1;
    obj->in_room = NOWHERE;
    obj->eq_pos = -1;
}

/**
 * @todo rename to objectExtract
 * @brief Extract an object from the world 
 */
void extract_obj(struct obj_data *obj)
{
    struct obj_data *temp1,
                   *temp2;
    extern long     obj_count;

    if (obj->in_room != NOWHERE) {
        obj_from_room(obj);
    } else if (obj->carried_by) {
        obj_from_char(obj);
    } else if (obj->equipped_by) {
        if (obj->eq_pos > -1) {
            /*
             * set players equipment slot to 0; that will avoid the garbage
             * items.
             */
            obj->equipped_by->equipment[(int)obj->eq_pos] = 0;
        } else {
            Log("Extract on equipped item in slot -1 on: %s - %s",
                obj->equipped_by->player.name, obj->name);
            return;
        }
    } else if (obj->in_obj) {
        temp1 = obj->in_obj;
        if (temp1->contains == obj) {
            /* head of list */
            temp1->contains = obj->next_content;
        } else {
            for (temp2 = temp1->contains;
                 temp2 && (temp2->next_content != obj);
                 temp2 = temp2->next_content) {
                /* 
                 * Empty loop 
                 */
            }

            if (temp2) {
                temp2->next_content = obj->next_content;
            }
        }
    }

    /*
     * leaves nothing ! 
     */
    for (; obj->contains; extract_obj(obj->contains)) {
        /* 
         * Empty loop 
         */
    }

    if (object_list == obj) {
        /* 
         * head of list 
         */
        object_list = obj->next;
    } else {
        for (temp1 = object_list;
             temp1 && (temp1->next != obj); temp1 = temp1->next) {
            /* 
             * Empty loop 
             */
        }

        if (temp1) {
            temp1->next = obj->next;
        } else {
            Log("Couldn't find object in object list.");
            assert(0);
        }
    }

    if (obj->item_number >= 0) {
        obj->index->number--;
        obj_count--;
    }
    free_obj(obj);

}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
