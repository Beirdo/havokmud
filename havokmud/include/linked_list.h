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
*
*--------------------------------------------------------*/
#ifndef linked_list_h_
#define linked_list_h_

#include "environment.h"
#include "common_data.h"

/* CVS generated ID string (optional for h files) */
static char linked_list_h_ident[] _UNUSED_ = 
    "$Id$";


struct _LinkedList_t;
struct _LinkedListItem_t;

/* This *MUST* be embedded at the BEGINNING of any type that will be used in a 
 * linked list 
 */
typedef struct _LinkedListItem_t
{
    struct _LinkedListItem_t *prev;
    struct _LinkedListItem_t *next;
    struct _LinkedList_t     *list;
} LinkedListItem_t;

typedef struct _LinkedList_t
{
    LinkedListItem_t *head;
    LinkedListItem_t *tail;
    pthread_mutex_t  mutex;
    int              items;
} LinkedList_t;

typedef Locked_t LinkedListLocked_t;

typedef enum
{
    AT_HEAD,
    AT_TAIL
} LinkedListLoc_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
LinkedList_t *LinkedListCreate( void );
void LinkedListDestroy( LinkedList_t *list );
void LinkedListLock( LinkedList_t *list );
void LinkedListUnlock( LinkedList_t *list );
void LinkedListAdd( LinkedList_t *list, LinkedListItem_t *item, 
                    LinkedListLocked_t locked, LinkedListLoc_t location );
void LinkedListRemove( LinkedList_t *list, LinkedListItem_t *item, 
                       LinkedListLocked_t locked );
int LinkedListCount( LinkedList_t *list, LinkedListLocked_t locked );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
