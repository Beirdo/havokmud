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
#include "oldstructs.h"
#include "structs.h"
#include "oldutils.h"
#include "logging.h"
#include "interthread.h"
#include "balanced_btree.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Handles loading, saving of rooms
 */

BalancedBTree_t *roomTree = NULL;

void            setup_dir(FILE * fl, long room, int dir);

void initializeRooms( void )
{
    roomTree = BalancedBTreeCreate( NULL, BTREE_KEY_INT );
    db_load_rooms(roomTree);
}


void completely_cleanout_room(struct room_data *rp)
{
    struct char_data       *ch;
    struct obj_data        *obj;
    PlayerStruct_t         *player; 
    LinkedListItem_t       *item;
    LinkedListItem_t       *nextItem;
    BalancedBTreeItem_t    *node;
    BalancedBTreeItem_t    *node2;
    BalancedBTree_t        *tree;

    while (rp->people) {
        ch = rp->people;
        player = (PlayerStruct_t *)ch->playerDesc;
        if( player ) {
            SendOutput( player, "The hand of god sweeps across the land and "
                                "you are swept into the Void.\n");
        }
#if 0
        char_from_room(ch);
#endif

        /*
         * send character to the void
         */
#if 0
        char_to_room(ch, 0);
#endif
    }

    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = nextItem ) {
        nextItem = item->next;
        obj = CONTENT_LINK_TO_OBJ(item);
        objectTakeFromRoom(obj, LOCKED);
        objectPutInRoom(obj, 0, UNLOCKED);
    }
    LinkedListUnlock( rp->contentList );

    BalancedBTreeLock( rp->contentKeywordTree );
    for( node = BalancedBTreeFindLeast( rp->contentKeywordTree->root ); node;
         node = BalancedBTreeFindLeast( rp->contentKeywordTree->root )) {
        tree = (BalancedBTree_t *)node->item;
        BalancedBTreeLock( tree );
        for( node2 = BalancedBTreeFindLeast( tree->root ); node2;
             node2 = BalancedBTreeFindLeast( tree->root ) ) {
            BalancedBTreeRemove( tree, node2, LOCKED, FALSE );
            free( node2 );
        }
        BalancedBTreeUnlock( tree );
        BalancedBTreeDestroy( tree );

        BalancedBTreeRemove( rp->contentKeywordTree, node, LOCKED, FALSE );
        free( node );
    }
    BalancedBTreeUnlock( rp->contentKeywordTree );

    cleanout_room(rp);
}


void cleanout_room(struct room_data *rp)
{
    int             i;
    Keywords_t     *exptr;

    if (rp->name) {
        free(rp->name);
        rp->name = NULL;
    }
    if (rp->description) {
        free(rp->description);
        rp->description = NULL;
    }
    for (i = 0; i < 6; i++) {
        if (rp->dir_option[i]) {
            if (rp->dir_option[i]->general_description) {
                free(rp->dir_option[i]->general_description);
                rp->dir_option[i]->general_description = NULL;
            }
            if (rp->dir_option[i]->keywords) {
                FreeKeywords( rp->dir_option[i]->keywords, TRUE );
                rp->dir_option[i]->keywords = NULL;
            }
            if (rp->dir_option[i]) {
                free(rp->dir_option[i]);
            }
            rp->dir_option[i] = NULL;
        }
    }

    for (i = 0, exptr = rp->ex_description; 
         i < rp->ex_description_count; i++, exptr++) {
        FreeKeywords( exptr, FALSE );
    }
    free( rp->ex_description );
    rp->ex_description = NULL;
}

struct room_data *roomFindNum(int virtual)
{
    struct room_data       *room;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( roomTree, &virtual, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    room = (struct room_data *)item->item;
    return( room );
}


int roomCountObject(int nr, struct room_data *rp)
{
    struct obj_data    *o;
    int                 count = 0;
    LinkedListItem_t   *item;

    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = item->next ) {
        o = CONTENT_LINK_TO_OBJ(item);
        if (o->item_number == nr) {
            count++;
        }
    }
    LinkedListUnlock( rp->contentList );
    return (count);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
