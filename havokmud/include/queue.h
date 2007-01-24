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

#ifndef queue_h_
#define queue_h_

#include "environment.h"
#include <pthread.h>

/* CVS generated ID string (optional for h files) */
static char queue_h_ident[] _UNUSED_ = 
    "$Id$";


typedef void * QueueItem_t;
typedef struct 
{ 
    uint32 numElements;
    uint32 numMask;
    uint32 head;
    uint32 tail;
    QueueItem_t *itemTable;
    pthread_mutex_t *mutex;
    bool full;
    pthread_cond_t *cNotFull;
    bool empty;
    pthread_cond_t *cNotEmpty;
} QueueObject_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
QueueObject_t * QueueCreate( uint32 numElements );
bool QueueEnqueueItem( QueueObject_t *queue, QueueItem_t item ); 
QueueItem_t QueueDequeueItem( QueueObject_t *queue, int32 ms_timeout );
void QueueClear( QueueObject_t *queue, bool freeItems );
uint32 QueueUsed( QueueObject_t *queue );

void QueueDestroy( QueueObject_t *queue );
void QueueLock( QueueObject_t *queue );
void QueueUnlock( QueueObject_t *queue );
uint32 QueueRemoveItem( QueueObject_t *queue, uint32 index, int locked );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
