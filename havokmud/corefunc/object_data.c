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
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "oldstructs.h"
#include "structs.h"
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


BalancedBTree_t *objectMasterTree  = NULL;
BalancedBTree_t *objectKeywordTree = NULL;
BalancedBTree_t *objectTypeTree    = NULL;

long            obj_count = 0;

void objectCloneContainerLocked(struct obj_data *to, struct obj_data *obj);
void objectPutInObjectLocked(struct obj_data *obj, struct obj_data *obj_to);
struct obj_data *GetObjectInKeywordTree(struct char_data *ch, char *name,
                                        BalancedBTree_t *tree, int offset,
                                        int *count, int steps);
struct obj_data *GetObjectInList(struct char_data *ch, char *name,
                                 struct obj_data *list, int nextOffset,
                                 int *count, bool visible, int steps);
struct obj_data *GetObjectNumInListOld(int num, struct obj_data *list, 
                                       int nextOffset);
struct obj_data *GetObjectNumInList(int num, LinkedList_t *list, int offset);
int objectStoreChain(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                     int roomId, int itemNum, int parentItem, int delete);
int objectStoreList(LinkedList_t *list, PlayerStruct_t *player, int playerId,
                    int roomId, int itemNum, int parentItem, int delete);
int objectStore(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                int roomId, int itemNum, int parentItem, int delete, 
                Locked_t locked);
int contained_weight(struct obj_data *container);
void RecursivePrintLimitedItems(BalancedBTreeItem_t *node);
void save_room(int room);


void initializeObjects( void )
{
    objectMasterTree = BalancedBTreeCreate( NULL, BTREE_KEY_INT );
    objectKeywordTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    objectTypeTree = BalancedBTreeCreate( NULL, BTREE_KEY_INT );
    db_load_object_tree( objectMasterTree );
}

struct index_data *objectIndex( int vnum )
{
    struct index_data      *index;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( objectMasterTree, &vnum, UNLOCKED, FALSE );
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

    index = CREATEN(struct index_data, 1);
    item = CREATEN(BalancedBTreeItem_t, 1);

    index->vnum = vnum;
    index->keywords.count = obj->keywords.count;
    index->keywords.words = CREATEN(char *, obj->keywords.count);
    for( i = 0; i < obj->keywords.count; i++ ) {
        index->keywords.words[i] = strdup( obj->keywords.words[i] );
    }
    index->number = 0;
    index->func = NULL;
    index->list = LinkedListCreate(NULL);

    item->key = &index->vnum;
    item->item = (void *)index;
    BalancedBTreeAdd( objectMasterTree, item, UNLOCKED, TRUE );
}


struct obj_data *objectClone(struct obj_data *obj)
{
    struct obj_data *ocopy;
    char            *keywords;
    
    ocopy = objectRead(obj->item_number);

    FreeKeywords( &ocopy->keywords, FALSE );

    if (ocopy->short_description) {
        free(ocopy->short_description);
    }

    if (ocopy->description) {
        free(ocopy->description);
    }

    keywords = KeywordsToString( &obj->keywords, " " );
    StringToKeywords( keywords, &ocopy->keywords );
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
    LinkedListLock( to->containList );
    LinkedListLock( obj->containList );
    objectCloneContainerLocked( to, obj );
    LinkedListUnlock( obj->containList );
    LinkedListUnlock( to->containList );
}

void objectCloneContainerLocked(struct obj_data *to, struct obj_data *obj)
{
    LinkedListItem_t   *item;
    struct obj_data    *tmp,
                       *ocopy;

    /*
     * Assume that both to->containList and obj->containList are locked
     */
    for (item = obj->containList->head; item; item = item->next ) {
        tmp = CONTAIN_LINK_TO_OBJ(item);

        ocopy = objectClone(tmp);

        LinkedListLock( tmp->containList );
        if (tmp->containList->head) {
            objectCloneContainerLocked(ocopy, tmp);
        }
        LinkedListUnlock( tmp->containList );

        objectPutInObjectLocked(ocopy, to);
    }
}

struct obj_data *objectRead(int nr)
{
    struct obj_data    *obj;
    struct index_data  *index;

    index = objectIndex( nr );
    if( !index ) {
        return( NULL );
    }
    
    obj = CREATEN(struct obj_data, 1);
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
    obj->containList = LinkedListCreate(NULL);
    obj->item_number = nr;
    obj->index = index;
    obj->in_obj = NULL;

    objectKeywordTreeAdd( objectKeywordTree, obj );
    objectTypeTreeAdd( obj );

    index->number++;
    LinkedListAdd( index->list, &obj->globalLink, UNLOCKED, AT_HEAD );
    obj_count++;
    
    return (obj);
}

/**
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
        free( obj->ex_description[i].found );
    }
    free( obj->ex_description );

    LinkedListDestroy( obj->containList );
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
    objectExtractLocked( obj, UNLOCKED );
}

void objectExtractLocked(struct obj_data *obj, Locked_t locked )
{
    extern long     obj_count;
    LinkedListItem_t   *item;
    LinkedListItem_t   *nextItem;

    if (obj->in_room != NOWHERE) {
        objectTakeFromRoom(obj, UNLOCKED);
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
        locked = LinkedListIsLocked( obj->in_obj->containList );
        if( !locked ) {
            LinkedListLock( obj->in_obj->containList );
        }
        LinkedListRemove( obj->in_obj->containList, &obj->containLink, 
                          LOCKED );
        if( !locked ) {
            LinkedListUnlock( obj->in_obj->containList );
        }
        objectKeywordTreeRemove( obj->in_obj->containKeywordTree, obj );
    }

    /*
     * remove the contents too
     */
    LinkedListLock( obj->containList );
    for( item = obj->containList->head; item; item = nextItem ) {
        nextItem = item->next;
        objectExtractLocked( CONTAIN_LINK_TO_OBJ(item), locked );
    }
    LinkedListUnlock( obj->containList );

    objectKeywordTreeRemove( objectKeywordTree, obj );
    objectTypeTreeRemove( obj );

    if (obj->item_number >= 0) {
        obj->index->number--;
        LinkedListRemove( obj->index->list, &obj->globalLink, locked );
        obj_count--;
    }
    objectFree(obj);
}

struct obj_data *objectGetInEquip(struct char_data *ch, char *arg,
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

        if (obj && (!visible || objectIsVisible(ch, obj)) && 
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

        if (obj && (!visible || objectIsVisible(ch, obj)) && 
            KeywordsMatch(key, &obj->keywords)) {
            *j = i;
            FreeKeywords( key, TRUE );
            return(obj);
        }
    }

    FreeKeywords( key, TRUE );
    return (NULL);
}


struct obj_data *objectGetInRoom( struct char_data *ch, char *name,
                                  struct room_data *rm )
{
    static int  steps   = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInKeywordTree( ch, name, rm->contentKeywordTree, 
                                    CONTENT_ITEM_OFFSET, NULL, steps ) );
}

struct obj_data *objectGetOnChar( struct char_data *ch, char *name,
                                  struct char_data *onch )
{
    static int  offset  = OFFSETOF(next_content, struct obj_data);
    static int  steps   = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInList( ch, name, onch->carrying, offset, NULL, 
                             BOOL(ch != NULL), steps ) );
}

struct obj_data *objectGetInObject( struct char_data *ch, char *name,
                                    struct obj_data *obj )
{
    static int  steps   = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInKeywordTree( ch, name, obj->containKeywordTree, 
                                    CONTAIN_ITEM_OFFSET, NULL, steps ) );
}

struct obj_data *objectGetGlobal(struct char_data *ch, char *name, int *count)
{
    static int  steps  = KEYWORD_FULL_MATCH | KEYWORD_PARTIAL_MATCH;
    return( GetObjectInKeywordTree( ch, name, objectKeywordTree, 
                                    KEYWORD_ITEM_OFFSET, count, steps ) );
}

struct obj_data *GetObjectInKeywordTree(struct char_data *ch, char *name,
                                        BalancedBTree_t *tree, int offset,
                                        int *count, int steps)
{
    int                 number;
    struct obj_data    *obj;
    Keywords_t         *key;
    char               *arg;
    bool                visible;
    int                 i = 0;

    arg = name;
    if (!(number = get_number(&arg))) {
        return(NULL);
    }
    key = StringToKeywords( arg, NULL );
    visible = BOOL(ch != NULL);

    BalancedBTreeLock( tree );

    /* First do a full word match */
    if( steps & KEYWORD_FULL_MATCH ) {
        i = count ? *count : 1;
        key->partial = FALSE;
        for( obj = objectKeywordFindFirst( tree, key ); obj && i <= number; 
             obj = objectKeywordFindNext( tree, offset, key, obj ) ) {
            if (!visible || objectIsVisible(ch, obj)) {
                if (i == number) {
                    BalancedBTreeUnlock( tree );
                    return(obj);
                }
                i++;
            }
        }
    }

    /* full word match failed, try a partial word match */
    if( steps & KEYWORD_PARTIAL_MATCH ) {
        key->partial = TRUE;
        i = count ? *count : 1;
        for( obj = objectKeywordFindFirst( tree, key ); obj && i <= number; 
             obj = objectKeywordFindNext( tree, offset, key, obj ) ) {
            if (!visible || objectIsVisible(ch, obj)) {
                if (i == number) {
                    BalancedBTreeUnlock( tree );
                    return(obj);
                }
                i++;
            }
        }
    }

    BalancedBTreeUnlock( tree );

    if (count) {
        *count = i;
    }

    FreeKeywords( key, TRUE );
    return (NULL);
}

/**
 * @todo check if object_list can be done in a better way... like btrees
 */
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
            if ((!visible || objectIsVisible(ch, obj)) && 
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
            if ((!visible || objectIsVisible(ch, obj)) && 
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

struct obj_data *objectGetOnCharNum(int num, struct char_data *ch)
{
    static int  offset = OFFSETOF(next_content, struct obj_data);
    return( GetObjectNumInListOld( num, ch->carrying, offset ) );
}

struct obj_data *objectGetInRoomNum(int num, struct room_data *rm)
{
    return( GetObjectNumInList( num, rm->contentList, CONTENT_LINK_OFFSET ) );
}

struct obj_data *objectGetNumLastCreated(int num)
{
    struct index_data  *index;
    struct obj_data    *obj;
    LinkedListItem_t   *item;

    index = objectIndex(num);
    if( !index ) {
        return( NULL );
    }

    LinkedListLock( index->list );

    item = index->list->head;
    obj = GLOBAL_LINK_TO_OBJ(item);

    LinkedListUnlock( index->list );

    return( obj );
}

/**
 * @todo check if object_list can be done in a better way... like btrees
 */
struct obj_data *GetObjectNumInListOld(int num, struct obj_data *list, 
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

struct obj_data *GetObjectNumInList(int num, LinkedList_t *list, int offset)
{
    struct obj_data    *obj;
    LinkedListItem_t   *item;

    LinkedListLock( list );
    for( item = list->head; item; item = item->next ) {
        obj = PTR_AT_OFFSET( -offset, item );
        if (obj->item_number == num) {
            LinkedListUnlock( list );
            return( obj );
        }
    }
    LinkedListUnlock( list );
    return (NULL);
}


/**
 * @todo the old get_obj_vis_accessible did full match on char, full match on
 *       room, then partial match on char, partial match on room.  Here we do
 *       full then partial on char, then full then partial on room.  Make sure
 *       this new behavior is OK
 */
struct obj_data *objectGetInCharOrRoom(struct char_data *ch, char *name)
{
    struct obj_data *obj;

    /*
     * scan items carried 
     */
    obj = objectGetOnChar(ch, name, ch);
    if( obj ) {
        return(obj);
    }

    /*
     * scan room 
     */
    obj = objectGetInRoom(ch, name, roomFindNum(ch->in_room));
    return(obj);
}


bool objectIsVisible(struct char_data *ch, struct obj_data *obj)
{
#if 0
    int             num = 0;
#endif

    if (IS_IMMORTAL(ch)) {
        return (TRUE);
    }

#if 0
    if (ITEM_TYPE(obj) == ITEM_TYPE_TRAP && GET_TRAP_CHARGES(obj) > 0) {
        num = number(1, 101);
        if (CanSeeTrap(num, ch)) {
            return (TRUE);
        } else {
            return (FALSE);
        }
    }
#endif

    if (ch->specials.blind) {
        return (FALSE);
    }
    if (ch->specials.true_sight) {
        return (TRUE);
    }
    if (IS_DARK(ch->in_room) && !IS_OBJ_STAT(obj, extra_flags, ITEM_GLOW)) {
        return (FALSE);
    }
    if (ch->specials.detect_invis) {
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
    LinkedListLock( obj_to->containList );
    objectPutInObjectLocked( obj, obj_to );
    LinkedListUnlock( obj_to->containList );
}

void objectPutInObjectLocked(struct obj_data *obj, struct obj_data *obj_to)
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

    objectKeywordTreeAdd( obj_to->containKeywordTree, obj );
    LinkedListAdd( obj_to->containList, &obj->containLink, UNLOCKED, AT_TAIL );
    obj->in_obj = obj_to;

    obj->carried_by = NULL;
    obj->equipped_by = NULL;

    for (tmp_obj = obj_to; tmp_obj; tmp_obj = tmp_obj->in_obj) {
        GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
    }

    if (obj_to->in_room != NOWHERE &&
        !IS_SET(roomFindNum(obj_to->in_room)->room_flags, DEATH)) {

        save_room(obj_to->in_room);
    }
}

/**
 * @brief remove an object from an object 
 * @param obj object to remove from its containing object
 * @param locked whether or not parent->containList is locked
 */
void objectTakeFromObject(struct obj_data *obj, Locked_t locked)
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
    objectKeywordTreeRemove( obj_from->containKeywordTree, obj );
    LinkedListRemove( obj_from->containList, &obj->containLink, locked );

    /*
     * Subtract weight from containers container 
     */
    for (tmp = obj->in_obj; tmp->in_obj; tmp = tmp->in_obj) {
        GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    }
    GET_OBJ_WEIGHT(tmp) -= GET_OBJ_WEIGHT(obj);
    obj->in_obj = NULL;

    /*
     * Subtract weight from char that carries the object 
     */
    if (tmp->carried_by) {
        IS_CARRYING_W(tmp->carried_by) -= GET_OBJ_WEIGHT(obj);
    }

    if (obj_from->in_room != NOWHERE &&
        !IS_SET(roomFindNum(obj_from->in_room)->room_flags, DEATH)) {

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
void objectPutInRoom(struct obj_data *object, long room, Locked_t locked)
{
    struct room_data   *rm;

    if (room == -1) {
        room = 4;
    }

    if(object->equipped_by || object->eq_pos != -1)
    {
        return;
    }

    rm = roomFindNum(room);
    if( !rm ) {
        return;
    }

    if( object->in_room == room ) {
        return;
    }

    if (object->in_room > NOWHERE) {
        objectTakeFromRoom(object, UNLOCKED);
    }

    LinkedListAdd( rm->contentList, &object->contentLink, locked, AT_TAIL );
    objectKeywordTreeAdd( rm->contentKeywordTree, object );

    object->in_room = room;
    object->carried_by = NULL;
    object->equipped_by = NULL;
    if (!IS_SET(rm->room_flags, DEATH)) {
        save_room(room);
    }
}

/*
 * Take an object from a room 
 */
void objectTakeFromRoom(struct obj_data *object, Locked_t locked)
{
    struct room_data   *rm;

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

    rm = roomFindNum(object->in_room);
    if( !rm ) {
        return;
    }

    LinkedListRemove( rm->contentList, &object->contentLink, locked );
    objectKeywordTreeRemove( rm->contentKeywordTree, object );

    if (!IS_SET(rm->room_flags, DEATH)) {
        save_room(object->in_room);
    }
    object->in_room = NOWHERE;
    object->next_content = NULL;
}

/**
 * @brief write the vital data of a player to the player file 
 */
void objectSaveForChar(struct char_data *ch, int delete)
{
    int                 i;
    int                 itemNum;
    struct obj_data    *obj;
    PlayerStruct_t     *player;

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

#if 0
    SysLogPrint( LOG_INFO, "Saving %s (%d): %d gold", fname(ch->player.name), 
                           ch->playerId, GET_GOLD(ch) );
#endif

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
#if 0
            unequip_char(ch, i);
#endif
        }

        itemNum = objectStore(obj, player, ch->playerId, -1, itemNum, -1, 
                              delete, UNLOCKED);
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
int objectStoreList(LinkedList_t *list, PlayerStruct_t *player, int playerId,
                    int roomId, int itemNum, int parentItem, int delete)
{
    LinkedListItem_t   *item;
    LinkedListItem_t   *nextItem;
    struct obj_data    *obj;

    if (!list) {
        return( itemNum );
    } 

    LinkedListLock( list );

    for( item = list->head ; item; item = nextItem ) {
        nextItem = item->next;
        obj = CONTAIN_LINK_TO_OBJ(item);

        itemNum = objectStore( obj, player, playerId, roomId, itemNum, 
                               parentItem, delete, LOCKED );
    }

    LinkedListUnlock( list );
    return( itemNum );
}

/**
 * @todo this will be completely replaced with objectStore and objectStoreList
 */
int objectStoreChain(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                     int roomId, int itemNum, int parentItem, int delete)
{
    return( itemNum );
}

int objectStore(struct obj_data *obj, PlayerStruct_t *player, int playerId,
                int roomId, int itemNum, int parentItem, int delete, 
                Locked_t locked)
{
    int                 weight;
    int                 newParent;

    if( !obj ) {
        return( itemNum );
    }

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

    if( LinkedListCount( obj->containList, UNLOCKED ) ) {
        itemNum = objectStoreList(obj->containList, player, playerId, 
                                  roomId, itemNum, newParent, delete);
    }

    /*
     * and now we can destroy object 
     */
    if (delete) {
        if (obj->in_obj) {
            objectTakeFromObject(obj, locked);
        }

        if (IS_RARE(obj)) {
            obj->index->number++;
            LinkedListAdd( obj->index->list, &obj->globalLink, UNLOCKED,
                           AT_HEAD );
        }

        objectExtract(obj);
    }

    return( itemNum );
}

int contained_weight(struct obj_data *container)
{
    struct obj_data    *tmp;
    int                 rval = 0;
    LinkedListItem_t   *item;
    
    if( !container ) {
        return( 0 );
    }

    LinkedListLock( container->containList );
    for( item = container->containList->head; item; item = item->next ) {
        tmp = CONTAIN_LINK_TO_OBJ(item);
        rval += GET_OBJ_WEIGHT(tmp);
    }
    LinkedListUnlock( container->containList );
    return( rval );
}

void PrintLimitedItems(void)
{
    BalancedBTreeLock( objectMasterTree );
    RecursivePrintLimitedItems( objectMasterTree->root );
    BalancedBTreeUnlock( objectMasterTree );
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
     * healed upon re-entry.
     */
    now = time(0);
    if (lastUpdate + 12 * SECS_PER_REAL_HOUR < now) {
#if 0
        RestoreChar(ch);
#endif
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
#if 0
    save_char(ch, AUTO_RENT);
#endif
}


void save_room(int room)
{
#ifdef SAVEWORLD
    struct room_data   *rm;
    int                 itemNum;

    rm = roomFindNum(room);
    if( !rm ) {
        return;
    }

    itemNum = objectStoreList(rm->contentList, NULL, -1, room, itemNum, -1, 
                              FALSE);

    /* Clear out any objects above the last item */
    db_clear_objects( -1, room, itemNum );
#endif
}

void objectKeywordTreeAdd( BalancedBTree_t *tree, struct obj_data *obj )
{
    int                     i;
    Keywords_t             *key;
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTree_t        *subtree;
    char                   *keyword;

    key = &obj->keywords;

    if( obj->keywordItem == NULL ) {
        obj->keywordItem = CREATEN( BalancedBTreeItem_t, key->count );
    }

    BalancedBTreeLock( tree );

    for( i = 0; i < key->count; i++ ) {
        if( key->words[i] ) {
            item = BalancedBTreeFind( tree, &key->words[i], LOCKED, FALSE );
            if( !item ) {
                subtree = BalancedBTreeCreate( NULL, BTREE_KEY_POINTER );
                item = CREATEN(BalancedBTreeItem_t, 1);
                keyword = strdup( key->words[i] );
                item->item = subtree;
                item->key  = &keyword;
                BalancedBTreeAdd( tree, item, LOCKED, TRUE );
            } else {
                subtree = (BalancedBTree_t *)item->item;
            }

            objItem = &obj->keywordItem[i];
            objItem->item = obj;
            objItem->key  = &obj;
            BalancedBTreeAdd( subtree, objItem, UNLOCKED, TRUE );
        }
    }

    BalancedBTreeUnlock( tree );
}

void objectKeywordTreeRemove( BalancedBTree_t *tree, struct obj_data *obj )
{
    int                     i;
    Keywords_t             *key;
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTree_t        *subtree;

    BalancedBTreeLock( objectKeywordTree );

    key = &obj->keywords;
    for( i = 0; i < key->count; i++ ) {
        if( key->words[i] ) {
            item = BalancedBTreeFind( tree, &key->words[i], LOCKED, FALSE );
            if( !item ) {
                continue;
            }

            subtree = (BalancedBTree_t *)item->item;

            objItem = BalancedBTreeFind( subtree, &obj, UNLOCKED, FALSE );
            if( !objItem ) {
                continue;
            }
            BalancedBTreeRemove( subtree, objItem, UNLOCKED, TRUE );
            memset( objItem, 0, sizeof(BalancedBTreeItem_t) );

            if( subtree->root == NULL ) {
                BalancedBTreeRemove( tree, item, LOCKED, TRUE );
                BalancedBTreeDestroy( subtree );
                free( item->key );
                free( item );
            }
        }
    }

    BalancedBTreeUnlock( objectKeywordTree );

    free( obj->keywordItem );
    obj->keywordItem = NULL;
}

struct obj_data *objectKeywordFindFirst( BalancedBTree_t *tree, 
                                         Keywords_t *key )
{
    return( objectKeywordFindNext( tree, 0, key, NULL ) );
}

struct obj_data *objectKeywordFindNext( BalancedBTree_t *tree, int offset,
                                        Keywords_t *key, 
                                        struct obj_data *lastobj )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTreeItem_t    *objItemA;
    BalancedBTree_t        *treeA;
    BalancedBTree_t        *treeB;
    struct obj_data        *obj;
    int                     i;

    treeA    = NULL;
    objItemA = NULL;
    obj      = lastobj;

    for( i = 0; i < key->count; i++ ) {
        if( i == 0 ) {
            if( !treeA ) {
                item = BalancedBTreeFind( tree, &key->words[i], LOCKED, 
                                          key->partial );
                if( !item ) {
                    return( NULL );
                }

                treeA = (BalancedBTree_t *)item->item;
            }

            BalancedBTreeLock( treeA );
            if( !obj ) {
                BalancedBTreeClearVisited( treeA, LOCKED );
                objItemA = BalancedBTreeFindLeast( treeA->root );
            } else {
                item = (BalancedBTreeItem_t *)PTR_AT_OFFSET(offset, obj);
                objItemA = BalancedBTreeFindNext( treeA, &item[i], LOCKED );
            }
            BalancedBTreeUnlock( treeA );

            if( !objItemA ) {
                return( NULL );
            }

            obj = (struct obj_data *)objItemA->item;
        } else {
            item = BalancedBTreeFind( tree, &key->words[i], LOCKED,
                                      key->partial );
            if( !item ) {
                /* This objA is useless, try again with the next one */
                i = -1;
                continue;
            }

            treeB = (BalancedBTree_t *)item->item;
            objItem = BalancedBTreeFind( treeB, &obj, UNLOCKED, key->partial );
            if( !objItem ) {
                i = -1;
                continue;
            }
        }
    }

    return( obj );
}

void objectTypeTreeAdd( struct obj_data *obj )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTree_t        *tree;
    uint32                 *key;

    BalancedBTreeLock( objectTypeTree );

    item = BalancedBTreeFind( objectTypeTree, &obj->type_flag, LOCKED, FALSE );
    if( !item ) {
        tree = BalancedBTreeCreate( NULL, BTREE_KEY_POINTER );
        item = CREATEN(BalancedBTreeItem_t, 1);
        key = CREATEN(uint32, 1);
        *key = (uint32)obj->type_flag;
        item->item = tree;
        item->key  = key;
        BalancedBTreeAdd( objectTypeTree, item, LOCKED, TRUE );
    } else {
        tree = (BalancedBTree_t *)item->item;
    }

    objItem = &obj->typeItem;
    objItem->item = obj;
    objItem->key  = &obj;
    BalancedBTreeAdd( tree, objItem, UNLOCKED, TRUE );

    BalancedBTreeUnlock( objectTypeTree );
}

void objectTypeTreeRemove( struct obj_data *obj )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTree_t        *tree;

    BalancedBTreeLock( objectTypeTree );

    item = BalancedBTreeFind( objectKeywordTree, &obj->type_flag, LOCKED, 
                              FALSE );
    if( !item ) {
        BalancedBTreeUnlock( objectTypeTree );
        return;
    }

    tree = (BalancedBTree_t *)item->item;

    objItem = BalancedBTreeFind( tree, &obj, UNLOCKED, FALSE );
    if( !objItem ) {
        BalancedBTreeUnlock( objectTypeTree );
        return;
    }
    BalancedBTreeRemove( tree, objItem, UNLOCKED, TRUE );
    memset( objItem, 0, sizeof(BalancedBTreeItem_t) );

    if( tree->root == NULL ) {
        BalancedBTreeRemove( objectTypeTree, item, LOCKED, TRUE );
        BalancedBTreeDestroy( tree );
        free( item->key );
        free( item );
    }

    BalancedBTreeUnlock( objectTypeTree );
}

struct obj_data *objectTypeFindFirst( ItemType_t type )
{
    return( objectTypeFindNext( type, NULL ) );
}

struct obj_data *objectTypeFindNext( ItemType_t type,
                                     struct obj_data *lastobj )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *objItem;
    BalancedBTree_t        *tree;
    struct obj_data        *obj;

    item = BalancedBTreeFind( objectTypeTree, &type, LOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    tree = (BalancedBTree_t *)item->item;

    BalancedBTreeLock( tree );
    if( !lastobj ) {
        BalancedBTreeClearVisited( tree, LOCKED );
        objItem = BalancedBTreeFindLeast( tree->root );
    } else {
        objItem = BalancedBTreeFindNext( tree, &lastobj->typeItem, LOCKED );
    }
    BalancedBTreeUnlock( tree );

    if( !objItem ) {
        return( NULL );
    }

    obj = (struct obj_data *)objItem->item;

    return( obj );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
