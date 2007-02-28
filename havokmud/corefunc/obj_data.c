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

struct obj_data *GetObjectInEquip(struct char_data *ch, char *arg,
                                  struct obj_data *equipment[], int *j,
                                  bool visible );
struct obj_data *GetObjectInList(struct char_data *ch, char *name,
                                 struct obj_data *list, int nextOffset,
                                 int *count, bool visible, int steps);
struct obj_data *GetObjectNumInList(int num, struct obj_data *list, 
                                    int nextOffset);
int CAN_SEE_OBJ(struct char_data *ch, struct obj_data *obj);


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

void objectInsert(struct obj_data *obj, long vnum)
{
    BalancedBTreeItem_t    *item;
    struct index_data      *index;
    int                     i;

    CREATE(index, struct index_data, 1);
    CREATE(item, BalancedBTreeItem_t, 1);

    index->vnum = vnum;
    index->keywords.count = obj->keywords.count;
    CREATE(index->keywords.words, char *, obj->keywords.count);
    for( i = 0; i < obj->keywords.count; i++ ) {
        index->keywords.words[i] = strdup( obj->keywords.words[i] );
    }
    index->number = 0;
    index->func = NULL;

    item->key = &index->vnum;
    item->item = (void *)index;
    BalancedBTreeAdd( objectTree, item, UNLOCKED, TRUE );
}


struct obj_data *objectClone(struct obj_data *obj)
{
    struct obj_data *ocopy;
    char            *keywords;
    
    ocopy = objectRead(obj->item_number, VIRTUAL);

    FreeKeywords( &ocopy->keywords, FALSE );

    if (ocopy->short_description) {
        free(ocopy->short_description);
    }

    if (ocopy->description) {
        free(ocopy->description);
    }

    keywords = KeywordsToString( &obj->keywords, " " );
    StringToKeywords( keywords, &obj->keywords );
    free( keywords );

    if (obj->short_description) {
        ocopy->short_description = strdup(obj->short_description);
    }

    if (obj->description) {
        ocopy->description = strdup(obj->description);
    }
    return( ocopy );
}

void objectCloneContainer(struct obj_data *to, struct obj_data *obj)
{
    struct obj_data *tmp,
                   *ocopy;

    for (tmp = obj->contains; tmp; tmp = tmp->next_content) {
        ocopy = objectClone(tmp);
        if (tmp->contains) {
            objectCloneContainer(ocopy, tmp);
        }
        objectPutInObject(ocopy, to);
    }
}

/**
 * @todo what is object_list being used for?!
 */
struct obj_data *objectRead(int nr, int type)
{
    struct obj_data    *obj;
    struct index_data  *index;

    index = objectIndex( nr );
    if( !index ) {
        return( NULL );
    }
    
    if( type != 1 ) {
        LogPrint( LOG_CRIT, "objectRead(%d, %d) failed", nr, type );
        return( NULL );
    }

    CREATE(obj, struct obj_data, 1);
    if (!obj) {
        LogPrintNoArg( LOG_CRIT, "Cannot create obj?!");
        return(NULL);
    }

    objectClear(obj);

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
 * @todo does this get it out of the object_list or does the caller do that?
 * @brief release memory allocated for an obj struct
 */
void objectFree(struct obj_data *obj)
{
    int                         i,
                                j;

    if (!obj) {
        LogPrintNoArg(LOG_CRIT, "!obj in objectFree, db.c");
        return;
    }

    for( i = 0; i < obj->keywords.count; i++ ) {
        if( obj->keywords.words[i] ) {
            free( obj->keywords.words[i] );
        }
    }
    free( obj->keywords.words );
    free( obj->keywords.length );
    free( obj->keywords.found );

    if (obj->description) {
        free(obj->description);
    }

    if (obj->short_description) {
        free(obj->short_description);
    }

    if (obj->action_description) {
        free(obj->action_description);
    }

    if (obj->modBy) {
        free(obj->modBy);
    }

    for( i = 0; i < obj->ex_description_count; i++ ) {
        if( obj->ex_description[i].description ) {
            free( obj->ex_description[i].description );
        }

        for( j = 0; j < obj->ex_description[i].count; j++ ) {
            if( obj->ex_description[i].words[j] ) {
                free( obj->ex_description[i].words[j] );
            }
        }
        free( obj->ex_description[i].words );
        free( obj->ex_description[i].length );
        free( obj->ex_description[i].length );
    }
    free( obj->ex_description );

    free(obj);
}

void objectClear(struct obj_data *obj)
{
    memset(obj, 0, sizeof(struct obj_data));
    obj->item_number = -1;
    obj->in_room = NOWHERE;
    obj->eq_pos = -1;
}

/**
 * @brief Extract an object from the world 
 */
void objectExtract(struct obj_data *obj)
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
            obj->equipped_by->equipment[obj->eq_pos] = 0;
        } else {
            LogPrint(LOG_CRIT, "Extract on equipped item in slot -1 on: "
                               "%s - %s (%d)",
                               obj->equipped_by->player.name, 
                               obj->short_description, obj->item_number);
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
    for (; obj->contains; objectExtract(obj->contains)) {
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
            LogPrintNoArg( LOG_CRIT, "Couldn't find object in object list.");
            assert(0);
        }
    }

    if (obj->item_number >= 0) {
        obj->index->number--;
        obj_count--;
    }
    objectFree(obj);

}


struct obj_data *get_object_in_equip_vis(struct char_data *ch, char *arg,
                                         struct obj_data *equipment[], int *j)
{
    return( GetObjectInEquip(ch, arg, equipment, j, TRUE) );
}

struct obj_data *get_object_in_equip(struct char_data *ch, char *arg,
                                     struct obj_data *equipment[], int *j)
{
    return( GetObjectInEquip(ch, arg, equipment, j, FALSE) );
}

struct obj_data *GetObjectInEquip(struct char_data *ch, char *arg,
                                  struct obj_data *equipment[], int *j,
                                  bool visible )
{
    int                 i;
    struct obj_data    *obj;
    Keywords_t         *key;

    key = StringToKeywords( arg, NULL );

    /* First do a full word match */
    key->partial = FALSE;
    for (i = 0; i < MAX_WEAR; i++) {
        obj = equipment[i];

        if (obj && (!visible || CAN_SEE_OBJ(ch, obj)) && 
            KeywordsMatch(key, &obj->keywords)) {
            *j = i;
            FreeKeywords( key, TRUE );
            return(obj);
        }
    }

    /* full word match failed, try a partial word match */
    key->partial = TRUE;
    for (i = 0; i < MAX_WEAR; i++) {
        obj = equipment[i];

        if (obj && (!visible || CAN_SEE_OBJ(ch, obj)) && 
            KeywordsMatch(key, &obj->keywords)) {
            *j = i;
            FreeKeywords( key, TRUE );
            return(obj);
        }
    }

    FreeKeywords( key, TRUE );
    return (NULL);
}

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
                                     struct obj_data *list)
{
    static int  offset = OFFSETOF(next_content, struct obj_data);
    static int  steps  = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInList( ch, name, list, offset, NULL, TRUE, steps ) );
}

struct obj_data *get_obj_in_list(char *name, struct obj_data *list)
{
    static int  offset = OFFSETOF(next_content, struct obj_data);
    static int  steps  = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInList( NULL, name, list, offset, NULL, FALSE, steps ) );
}


struct obj_data *get_obj_vis_world(struct char_data *ch, char *name,
                                   int *count)
{
    static int  offset = OFFSETOF(next, struct obj_data);
    static int  steps  = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInList( ch, name, object_list, offset, count, TRUE, 
                             steps ) );
}

struct obj_data *get_obj(char *name)
{
    static int  offset = OFFSETOF(next, struct obj_data);
    static int  steps  = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInList( NULL, name, object_list, offset, NULL, FALSE, 
                             steps ) );
}

struct obj_data *GetObjectInList(struct char_data *ch, char *name,
                                 struct obj_data *list, int nextOffset,
                                 int *count, bool visible, int steps)
{
    int                 i;
    int                 number;
    struct obj_data    *obj;
    Keywords_t         *key;
    char               *arg;

    arg = name;
    if (!(number = get_number(&arg))) {
        return(NULL);
    }
    key = StringToKeywords( arg, NULL );
    i = count ? *count : 1;

    /* First do a full word match */
    if( steps & KEYWORD_FULL_MATCH ) {
        key->partial = FALSE;
        for (obj = list, i = count ? *count : 1; 
             obj && i <= number; 
             obj = (struct obj_data *)PTR_AT_OFFSET(nextOffset, obj) ) {
            if ((!visible || CAN_SEE_OBJ(ch, obj)) && 
                KeywordsMatch(key, &obj->keywords)) {
                if (i == number) {
                    return(obj);
                }
                i++;
            }
        }
    }

    /* full word match failed, try a partial word match */
    if( steps & KEYWORD_PARTIAL_MATCH ) {
        key->partial = TRUE;
        for (obj = list, i = count ? *count : 1; 
             obj && i <= number; 
             obj = (struct obj_data *)PTR_AT_OFFSET(nextOffset, obj) ) {
            if ((!visible || CAN_SEE_OBJ(ch, obj)) && 
                KeywordsMatch(key, &obj->keywords)) {
                if (i == number) {
                    return(obj);
                }
                i++;
            }
        }
    }

    if (count) {
        *count = i;
    }

    FreeKeywords( key, TRUE );
    return (NULL);
}

/*
 * Search a given list for an object number, and return a ptr to that obj 
 */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    static int  offset = OFFSETOF(next_content, struct obj_data);
    return( GetObjectNumInList( num, list, offset ) );
}

struct obj_data *get_obj_num(int num)
{
    static int  offset = OFFSETOF(next, struct obj_data);
    return( GetObjectNumInList( num, object_list, offset ) );
}

struct obj_data *GetObjectNumInList(int num, struct obj_data *list, 
                                    int nextOffset)
{
    struct obj_data *obj;

    for (obj = list; obj; 
         obj = (struct obj_data *)PTR_AT_OFFSET(nextOffset, obj) ) {
        if (obj->item_number == num) {
            return (obj);
        }
    }
    return (NULL);
}


/*
 * search the entire world for an object, and return a pointer 
 */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *obj;

    /*
     * scan items carried 
     */
    if ((obj = get_obj_in_list_vis(ch, name, ch->carrying))) {
        return(obj);
    }
    /*
     * scan room 
     */
    if ((obj = get_obj_in_list_vis(ch, name, 
                                 real_roomp(ch->in_room)->contents))) {
        return(obj);
    }
    return( get_obj_vis_world(ch, name, NULL) );
}

struct obj_data *get_obj_vis_accessible(struct char_data *ch, char *name)
{
    static int          offset = OFFSETOF(next_content, struct obj_data);
    int                 count;
    struct obj_data    *obj;

    count = 1;

    obj = GetObjectInList( ch, name, ch->carrying, offset, &count, TRUE, 
                           KEYWORD_FULL_MATCH );
    if( obj ) {
        return( obj );
    }

    obj = GetObjectInList( ch, name, real_roomp(ch->in_room)->contents, offset,
                           &count, TRUE, KEYWORD_FULL_MATCH );
    if( obj ) {
        return( obj );
    }

    count = 1;

    obj = GetObjectInList( ch, name, ch->carrying, offset, &count, TRUE, 
                           KEYWORD_PARTIAL_MATCH );
    if( obj ) {
        return( obj );
    }

    obj = GetObjectInList( ch, name, real_roomp(ch->in_room)->contents, offset,
                           &count, TRUE, KEYWORD_PARTIAL_MATCH );
    return( obj );
}

int CAN_SEE_OBJ(struct char_data *ch, struct obj_data *obj)
{
    int             num = 0;

    if (IS_IMMORTAL(ch)) {
        return (TRUE);
    }

    if (ITEM_TYPE(obj) == ITEM_TYPE_TRAP && GET_TRAP_CHARGES(obj) > 0) {
        num = number(1, 101);
        if (CanSeeTrap(num, ch)) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        return (TRUE);
    }
    if (IS_DARK(ch->in_room) && !IS_OBJ_STAT(obj, extra_flags, ITEM_GLOW)) {
        return (FALSE);
    }
    if (IS_AFFECTED(ch, AFF_DETECT_INVISIBLE)) {
        return (TRUE);
    }
    if (IS_OBJ_STAT(obj, extra_flags, ITEM_INVISIBLE)) {
        return (FALSE);
    }
    return (TRUE);
}

bool HasAntiBitsEquipment(struct char_data *ch, int bits)
{
    return( HasBitsEquipment(ch, bits, OFFSETOF(anti_class, struct obj_data)) );
}

bool HasBitsEquipment(struct char_data *ch, int bits, int offset)
{
    int                 i;
    struct obj_data    *obj;

    for (i = 0; i < MAX_WEAR; i++) {
        obj = ch->equipment[i];
        if( !obj ) {
            continue;
        }
        
        if( IS_SET_FLAG(obj, offset, bits) ) {
            return (TRUE);
        }
    }
    return (FALSE);
}

bool HasBitsInventory(struct char_data *ch, int bits, int offset)
{
    struct obj_data *o;

    for (o = ch->carrying; o; o = o->next_content) {
        if (IS_SET_FLAG(o, offset, bits)) {
            return (TRUE);
        }
    }
    return (FALSE);
}

bool HasBits(struct char_data *ch, int bits, int offset)
{
    if (HasBitsEquipment(ch, bits, offset) || 
        HasBitsInventory(ch, bits, offset)) {
        return (TRUE);
    }
    return (FALSE);
}

bool HasExtraBits(struct char_data *ch, int bits)
{
    return( HasBits(ch, bits, OFFSETOF(extra_flags, struct obj_data)) );
}

/**
 * @brief put an object in an object
 * @todo rename to objectPutInObject
 */
void objectPutInObject(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj;

    if( !obj || !obj_to ) {
        return;
    }

    if( obj == obj_to ) {
        LogPrint( LOG_CRIT, "Trying to fold object %s into itself!", 
                            obj->short_description );
        return;
    }

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    obj->carried_by = NULL;
    obj->equipped_by = NULL;

    for (tmp_obj = obj_to; tmp_obj; tmp_obj = tmp_obj->in_obj) {
        GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
    }

    if (obj_to->in_room != NOWHERE &&
        !IS_SET(real_roomp(obj_to->in_room)->room_flags, DEATH)) {

        save_room(obj_to->in_room);
    }
}

/**
 * @brief remove an object from an object 
 * @todo rename to objectTakeFromObject
 */
void objectTakeFromObject(struct obj_data *obj)
{
    struct obj_data    *tmp,
                       *obj_from = NULL;
    char               *objname;

    if( !obj ) {
        return;
    }

    objname = KeywordsToString( &obj->keywords, NULL );

    if (obj->carried_by) {
        LogPrint( LOG_INFO, "%s carried by %s in objectTakeFromObject", objname,
                            obj->carried_by->player.name);
    }

    if (obj->equipped_by) {
        LogPrint( LOG_INFO, "%s equipped by %s in objectTakeFromObject", 
                            objname, obj->equipped_by->player.name);
    }

    if (obj->in_room != NOWHERE) {
        LogPrint( LOG_INFO, "%s in room %d in objectTakeFromObject", objname, 
                            obj->in_room);
    }

    if (!obj->in_obj) {
        LogPrint( LOG_INFO, "Trying to take object %s from no object.", 
                            objname);
    }

    free( objname );

    if(obj->carried_by || obj->equipped_by || obj->in_room != NOWHERE || 
       !obj->in_obj) {
        return;
    }

    obj_from = obj->in_obj;
    if (obj == obj_from->contains) {
        /* 
         * head of list 
         */
        obj_from->contains = obj->next_content;
    } else {
        /* 
         * locate previous 
         */
        for (tmp = obj_from->contains; tmp && (tmp->next_content != obj);
             tmp = tmp->next_content) {
            /* 
             * Empty loop 
             */
        }

        if (!tmp) {
            LogPrintNoArg( LOG_CRIT, "No previous object in chain!");
            return;
        }

        tmp->next_content = obj->next_content;
    }

    /*
     * Subtract weight from containers container 
     */
    for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj) {
        GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    }
    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);

    /*
     * Subtract weight from char that carries the object 
     */
    if (tmp->carried_by) {
        IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
    }
    obj->in_obj = NULL;
    obj->next_content = NULL;

    if (obj_from->in_room != NOWHERE &&
        !IS_SET(real_roomp(obj_from->in_room)->room_flags, DEATH)) {

        save_room(obj_from->in_room);
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
