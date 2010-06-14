/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005 Gavin Hurlbut
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
* Copyright 2005 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include <pthread.h>
#include "memory.h"
#include "linked_list.h"
#include "logging.h"
#include "interthread.h"
#include <stdlib.h>


/**
 * @file
 * @brief Doubly Linked List Data Structure
 */

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

void LinkedListLock( LinkedList_t *list )
{
    if( list == NULL )
    {
        return;
    }

    pthread_mutex_lock( &list->mutex );
    list->locked = TRUE;
}


void LinkedListUnlock( LinkedList_t *list )
{
    if( list == NULL )
    {
        return;
    }

    pthread_mutex_unlock( &list->mutex );
    list->locked = FALSE;
}

bool LinkedListIsLocked( LinkedList_t *list )
{
    if( list == NULL ) {
        return( FALSE );
    }

    return( list->locked );
}


LinkedList_t *LinkedListCreate( LinkedList_t *list )
{
    if( !list ) {
        list = CREATE(LinkedList_t);
        if( list == NULL )
        {
            LogPrintNoArg( LOG_UNKNOWN, "Couldn't create linked list" );
            return( NULL );
        }
    }

    list->head = NULL;
    list->tail = NULL;
    thread_mutex_init( &list->mutex );
    list->locked = FALSE;
    list->items = 0;

    return( list );
}

void LinkedListDestroy( LinkedList_t *list )
{
    /* Assumes the list is locked by the caller */
    pthread_mutex_unlock( &list->mutex );
    pthread_mutex_destroy( &list->mutex );

    memfree( list );
}

void LinkedListAdd( LinkedList_t *list, LinkedListItem_t *item,
                    LinkedListLocked_t locked, LinkedListLoc_t location )
{
    if( item == NULL || list == NULL )
    {
        return;
    }

    if( locked == UNLOCKED )
    {
        LinkedListLock( list );
    }

    item->list = list;

    if( location == AT_HEAD )
    {
        /* Link at head */
        item->prev = NULL;
        item->next = list->head;
        list->head = item;
    }
    else
    {
        /* Link at tail */
        item->prev = list->tail;
        item->next = NULL;
        list->tail = item;
    }

    /* Fix links for adjacent items */
    if( item->next != NULL )
    {
        item->next->prev = item;
    }

    if( item->prev != NULL )
    {
        item->prev->next = item;
    }

    /* Fix head & tail */
    if( list->head == NULL )
    {
        list->head = item;
    }

    if( list->tail == NULL )
    {
        list->tail = item;
    }

    list->items++;

    if( locked == UNLOCKED )
    {
        LinkedListUnlock( list );
    }
}


void LinkedListRemove( LinkedList_t *list, LinkedListItem_t *item, 
                       LinkedListLocked_t locked )
{
    if( item == NULL || list == NULL )
    {
        return;
    }

    if( item->list != list )
    {
        LogPrint( LOG_UNKNOWN, "Item %p not on list %p! (on %p)", item, list,
                  item->list );
        return;
    }

    if( locked == UNLOCKED )
    {
        LinkedListLock( list );
    }

    if( item->prev != NULL )
    {
        item->prev->next = item->next;
    }
    
    if( item->next != NULL )
    {
        item->next->prev = item->prev;
    }

    if( list->head == item )
    {
        list->head = item->next;
    }

    if( list->tail == item )
    {
        list->tail = item->prev;
    }

    item->list = NULL;

    list->items--;

    if( locked == UNLOCKED )
    {
        LinkedListUnlock( list );
    }
}

int LinkedListCount( LinkedList_t *list, LinkedListLocked_t locked )
{
    int         count;

    if( list == NULL )
    {
        return( 0 );
    }

    if( locked == UNLOCKED )
    {
        LinkedListLock( list );
    }

    count = list->items;

    if( locked == UNLOCKED )
    {
        LinkedListUnlock( list );
    }

    return( count );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
