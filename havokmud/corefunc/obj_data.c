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


BalancedBTree_t *objectTree = NULL;

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
int objectStoreChain(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                     int roomId, int itemNum, int parentItem, int delete);
int contained_weight(struct obj_data *container);
void RecursivePrintLimitedItems(BalancedBTreeItem_t *node);
void save_room(int room);


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

    if( !db_read_object(obj, nr, -1, -1, -1) ) {
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
        objectTakeFromRoom(obj);
    } else if (obj->carried_by) {
        objectTakeFromChar(obj);
    } else if (obj->equipped_by) {
        if (obj->eq_pos > -1) {
            /*
             * set players equipment slot to 0; that will avoid the garbage
             * items.
             */
            obj->equipped_by->equipment[obj->eq_pos] = NULL;
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

/**
 * @brief give an object to a char 
 */
void objectGiveToChar(struct obj_data *object, struct char_data *ch)
{
    if (!object || !ch) {
        return;
    }

    if(object->in_obj || object->carried_by || object->equipped_by ||
       object->in_room != NOWHERE) {
        return;
    }

    if (ch->carrying) {
        object->next_content = ch->carrying;
    } else {
        object->next_content = NULL;
    }
    ch->carrying = object;
    object->carried_by = ch;
    object->in_room = NOWHERE;
    object->equipped_by = 0;
    object->in_obj = 0;
    IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(ch)++;
}

/**
 * @brief take an object from a char 
 */
void objectTakeFromChar(struct obj_data *object)
{
    struct obj_data *tmp;

    if (!object) {
        LogPrintNoArg( LOG_CRIT, "No object to be take from char.");
        return;
    }

    if (!object->carried_by) {
        LogPrintNoArg( LOG_CRIT, "this object is not carried by anyone");
        return;
    }

    if (!object->carried_by->carrying) {
        LogPrintNoArg( LOG_CRIT, "No one is carrying this object");
        return;
    }

    if (object->in_obj) {
        return;
    }

    if (object->equipped_by) {
        return;
    }

    if (object->carried_by->carrying == object) {
        /* 
         * head of list 
         */
        object->carried_by->carrying = object->next_content;
    } else {
        /* 
         * locate previous 
         */
        for (tmp = object->carried_by->carrying; 
             tmp && (tmp->next_content != object); tmp = tmp->next_content) {
            /* 
             * Empty loop 
             */
        }

        if (!tmp) {
            LogPrintNoArg( LOG_CRIT, "Couldn't find object on character");
            return;
        }

        tmp->next_content = object->next_content;
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;
    object->carried_by = NULL;
    object->equipped_by = NULL;
    object->next_content = NULL;
    object->in_obj = NULL;
}

/*
 * put an object in a room 
 */
void objectPutInRoom(struct obj_data *object, long room)
{

    if (room == -1) {
        room = 4;
    }

    if(object->equipped_by || object->eq_pos != -1)
    {
        return;
    }

    if (object->in_room > NOWHERE) {
        objectTakeFromRoom(object);
    }

    object->next_content = real_roomp(room)->contents;
    real_roomp(room)->contents = object;
    object->in_room = room;
    object->carried_by = NULL;
    object->equipped_by = NULL;
    if (!IS_SET(real_roomp(room)->room_flags, DEATH)) {
        save_room(room);
    }
}

void obj_to_room2(struct obj_data *object, long room)
{

    if (room == -1) {
        room = 4;
    }

    if(object->equipped_by || object->eq_pos != -1) {
        return;
    }

    if (object->in_room > NOWHERE) {
        objectTakeFromRoom(object);
    }

    object->next_content = real_roomp(room)->contents;
    real_roomp(room)->contents = object;
    object->in_room = room;
    object->carried_by = NULL;
    object->equipped_by = NULL;
}

/*
 * Take an object from a room 
 */
void objectTakeFromRoom(struct obj_data *object)
{
    struct obj_data *i;

    /*
     * remove object from room 
     */

    if (object->in_room <= NOWHERE) {
        if (object->carried_by || object->equipped_by) {
            LogPrintNoArg( LOG_CRIT, "Eek.. an object was just taken from a "
                                     "char, instead of a room");
        }
        return;
    }

    if (object == real_roomp(object->in_room)->contents) {
        /* 
         * head of list 
         */
        real_roomp(object->in_room)->contents = object->next_content;
    } else {
        /* 
         * locate previous element in list 
         */
        for (i = real_roomp(object->in_room)->contents; i &&
             (i->next_content != object); i = i->next_content) {
            /* 
             * Empty loop 
             */
        }

        if (i) {
            i->next_content = object->next_content;
        } else {
            LogPrintNoArg( LOG_CRIT, "Couldn't find object in room");
            return;
        }
    }

    if (!IS_SET(real_roomp(object->in_room)->room_flags, DEATH)) {
        save_room(object->in_room);
    }
    object->in_room = NOWHERE;
    object->next_content = NULL;
}

/**
 * @brief write the vital data of a player to the player file 
 */
void objectSaveForChar(struct char_data *ch, struct obj_cost *cost, int delete)
{
    int                 i;
    int                 itemNum;
    struct obj_data    *obj;
    PlayerStruct_t     *player;

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

    SysLogPrint( LOG_INFO, "Saving %s (%d): %d gold", fname(ch->player.name), 
                           ch->playerId, GET_GOLD(ch) );

    /*
     * update the rent cost and gold
     */
    db_update_char_rent( ch->playerId, GET_GOLD(ch), cost->total_cost, 0 );

    itemNum = 1;

    for (i = 0; i < MAX_WEAR; i++) {
        obj = ch->equipment[i];
        if( !obj ) {
            continue;
        }

        if (obj->cost_per_day != -1) {
            obj->eq_pos = i + 1;
        }

        if (delete) {
            unequip_char(ch, i);
        }

        itemNum = objectStoreChain(obj, player, ch->playerId, -1, itemNum, -1, 
                                   delete);
    }

    itemNum = objectStoreChain(ch->carrying, player, ch->playerId, -1, itemNum,
                               -1, delete);
    if (delete) {
        ch->carrying = NULL;
    }

    /* Clear out any objects above the last item */
    db_clear_objects( ch->playerId, -1, itemNum );

    /*
     * write the aliases and bamfs 
     */
    db_write_char_extra(ch);
}

/**
 * @brief Destroy inventory after transferring it to "store inventory" 
 */
int objectStoreChain(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                     int roomId, int itemNum, int parentItem, int delete)
{
    int                 weight;
    struct obj_data    *next;
    int                 newParent;

    if (!obj) {
        return( itemNum );
    } 

    for( ; obj; obj = next ) {
        if( (obj->timer < 0 && obj->timer != OBJ_NOTIMER) ||
            (obj->cost_per_day < 0) ) {
#ifdef DUPLICATES
            if( delete && player) {
                SendOutput(player, "You're told: '%s is just old junk, I'll "
                                   "throw it away for you.'\n\r", 
                                   obj->short_description);
            }
#endif
            newParent = parentItem;
        } else if (obj->item_number != -1) {
            weight = contained_weight(obj);

            GET_OBJ_WEIGHT(obj) -= weight;
            db_save_object(obj, playerId, roomId, itemNum, parentItem);
            GET_OBJ_WEIGHT(obj) += weight;
            newParent = itemNum;
            itemNum++;
        } else {
            /* If this is an unrentable, save any contents as in the parent */
            newParent = parentItem;
        }

        if (obj->contains) {
            itemNum = objectStoreChain(obj->contains, player, playerId, roomId, 
                                       itemNum, newParent, delete);
        }

        next = obj->next_content;

        /*
         * and now we can destroy object 
         */
        if (delete) {
            if (obj->in_obj) {
                objectTakeFromObject(obj);
            }
            if (IS_RARE(obj)) {
                obj->index->number++;
            }
            objectExtract(obj);
        }
    }
    return( itemNum );
}


int contained_weight(struct obj_data *container)
{
    struct obj_data    *tmp;
    int                 rval = 0;

    for (tmp = container->contains; tmp; tmp = tmp->next_content) {
        rval += GET_OBJ_WEIGHT(tmp);
    }
    return( rval );
}

void PrintLimitedItems(void)
{
    BalancedBTreeLock( objectTree );
    RecursivePrintLimitedItems( objectTree->root );
    BalancedBTreeUnlock( objectTree );
}

void RecursivePrintLimitedItems(BalancedBTreeItem_t *node)
{
    struct index_data  *index;

    if( !node ) {
        return;
    }

    RecursivePrintLimitedItems(node->left);

    index = (struct index_data *)node->item;
    if( index && index->number ) {
        LogPrint( LOG_INFO, "Item> %d [%d]", index->vnum, index->number );
    }

    RecursivePrintLimitedItems(node->right);
}

void load_char_objs(struct char_data *ch)
{
    bool            found = FALSE;
    int             timegold;
    int             gold,
                    rentCost,
                    minStay,
                    lastUpdate;
    time_t          now;
    PlayerStruct_t *player;

    if( !ch ) {
        return;
    }

    player = (PlayerStruct_t *)ch->playerDesc;
    if( !player ) {
        return;
    }

    /*
     * load in aliases and poofs first 
     */
    db_load_char_extra(ch);


    /*
     * Get rent timing info
     */
    gold = 0;
    rentCost = 0;
    minStay = 0;
    lastUpdate = 0;
    db_get_char_rent( ch->playerId, &gold, &rentCost, &minStay, &lastUpdate );

    /*
     * if the character has been out for 12 real hours, they are fully
     * healed upon re-entry.  if they stay out for 24 full hours, all
     * affects are removed, including bad ones. 
     */
    now = time(0);
    if (lastUpdate + 12 * SECS_PER_REAL_HOUR < now) {
        RestoreChar(ch);
    }

    if (lastUpdate + 24 * SECS_PER_REAL_HOUR < now) {
        RemAllAffects(ch);
    }

    if (ch->in_room == NOWHERE && lastUpdate + 1 * SECS_PER_REAL_HOUR > now) {
        /*
         * you made it back from the crash in time, 1 hour grace period. 
         */
        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            LogPrint( LOG_INFO, "Character %s reconnecting.", GET_NAME(ch) );
        }
        found = TRUE;
    } else {
        if (ch->in_room == NOWHERE &&
            (!IS_IMMORTAL(ch) || ch->invis_level <= 58)) {
            LogPrint( LOG_INFO, "Char %s reconnecting after autorent",
                                GET_NAME(ch) );
        }
#ifdef NEW_RENT
        timegold = (int) ((100.0 * (now - lastUpdate)) / (SECS_PER_REAL_DAY));
#else
        timegold = (int) (((float)rentCost * (now - lastUpdate)) /
                          (SECS_PER_REAL_DAY));
#endif

        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            LogPrint(LOG_INFO, "Char %s ran up charges of %d gold in rent", 
                               GET_NAME(ch), timegold);
        }

        SendOutput(player, "You ran up charges of %d gold in rent.\n\r", 
                           timegold);
        GET_GOLD(ch) -= timegold;
        found = TRUE;

        if (GET_GOLD(ch) < 0) {
            if (GET_BANK(ch) + (int)(1.05 * GET_GOLD(ch)) < 0) {
                LogPrint( LOG_INFO, "** Char %s ran out of money in rent **",
                                    GET_NAME(ch) );
                SendOutput( player, "You ran out of money, you deadbeat.\n\r");
                SendOutput( player, "You don't even have enough in your bank "
                                    "account!!\n\r");
                GET_GOLD(ch) = 0;
                GET_BANK(ch) = 0;
                found = FALSE;
            } else {
                LogPrint( LOG_INFO, "** Char %s ran out of rent so lets dip "
                                    "into bank **", GET_NAME(ch) );
                SendOutput( player, "You ran out rent money.. lets dip into "
                                    "your bank account.\n\r");
                SendOutput( player, "I'm going to have to charge you a little "
                                    "extra for that though.\n\r");

                GET_BANK(ch) = GET_BANK(ch) + (int)(1.05 * GET_GOLD(ch));

                if (GET_BANK(ch) < 0) {
                    GET_BANK(ch) = 0;
                }
                GET_GOLD(ch) = 0;
                found = TRUE;
            }
        }

        if (db_has_mail((char *) GET_NAME(ch))) {
            SendOutput( player, "$c0013[$c0015The scribe$c0013] bespeaks you: "
                                "'You have mail waiting!'");
            ch->player.has_mail = TRUE;
        }
    }

    if (found) {
        /*
         * Get the objects from the database
         */
        db_load_char_objects(ch);
    } else {
        /*
         * Kiss goodbye to your objects, punk, you deadbeated.
         */
        db_clear_objects( ch->playerId, -1, 0 );
    }

    /*
     * Save char, to avoid strange data if crashing 
     */
    save_char(ch, AUTO_RENT);
}


void save_room(int room)
{
#ifdef SAVEWORLD
    struct room_data   *rm;
    int                 itemNum;

    rm = real_roomp(room);
    if( !rm ) {
        return;
    }

    itemNum = objectStoreChain(rm->contents, NULL, -1, room, itemNum, -1, 
                               FALSE);

    /* Clear out any objects above the last item */
    db_clear_objects( -1, room, itemNum );
#endif
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
