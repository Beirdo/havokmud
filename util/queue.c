/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005, 2010 Gavin Hurlbut
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
* Copyright 2005, 2010 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*
*----------------------------------------------------------
*/

/* INCLUDE FILES */
#include "environment.h"
#include "memory.h"
#include "queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "logging.h"
#include "interthread.h"
#include "linked_list.h"
#include <string.h>


/**
 * @file
 * @brief Queue Data Structure
 */

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */
extern bool GlobalAbort;
static void QueueConditionUpdate( QueueObject_t *queue );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

LinkedList_t   *QueueList = NULL;

QueueObject_t * QueueCreate( uint32 numElements )
{
    QueueObject_t *queue;
    QueueItem_t *items;
    int status;
    uint32 i;

    if( numElements == 0 )
    {
        return( NULL );
    }

    /* Round the numElements to a power of 2 for efficiency */
    for( i = 0; i < 32 && (uint64)numElements > (uint64)(1<<i); i++ );

    if( i == 32 )
    {
        LogPrint( LOG_CRIT, "Queue too large.  Rounded from %d to %d\n",
                  numElements, 1 << 31 );
        i = 31;
    }
    else
    {
        if( numElements != (1<<i) )
        {
            LogPrint( LOG_CRIT, "Queue item count rounded from %d to %d\n",
                      numElements, (1 << i) );
        }
    }
    numElements = 1 << i;

    queue = CREATE(QueueObject_t);
    items = CREATEN(QueueItem_t, numElements);

    queue->numElements = numElements;
    queue->numMask     = numElements - 1;
    queue->head = 0;
    queue->tail = 0;

    for( i = 0; i < numElements; i++ )
    {
        items[i] = NULL;
    }
    queue->itemTable = items;

    /* Initialize the mutex */
    queue->mutex = CREATE(pthread_mutex_t);
    status = thread_mutex_init( queue->mutex );

    /* Initialize the condition variables */
    queue->full = FALSE;
    queue->condNotFull = CREATEA(pthread_cond_t, 16);
    queue->cNotFull = ALIGN(pthread_cond_t, queue->condNotFull, 16);
    status = pthread_cond_init( queue->cNotFull, NULL );

    queue->empty = TRUE;
    queue->condNotEmpty = CREATEA(pthread_cond_t, 16);
    queue->cNotEmpty = ALIGN(pthread_cond_t, queue->condNotEmpty, 16);
    status = pthread_cond_init( queue->cNotEmpty, NULL );

    if( !QueueList ) {
        QueueList = LinkedListCreate( NULL );
        if( !QueueList ) {
            LogPrintNoArg( LOG_CRIT, "Couldn't create the list of queues!" );
            exit(1);
        }
    }

    /* Allows us to flush all queues at shutdown */
    LinkedListAdd( QueueList, (LinkedListItem_t *)queue, UNLOCKED, AT_TAIL );

    return( queue );
}


bool QueueEnqueueItem( QueueObject_t *queue, QueueItem_t item )
{
    int status;

    if( queue == NULL || item == NULL )
    {
        return( FALSE );
    }

    /* Lock the mutex */
    status = pthread_mutex_lock( queue->mutex );

    /* Wait for the appropriate condition variable (unless the condition being
     * waited for is already true).  Loop in case of false signals.
     */
    while( queue->full == TRUE )
    {
        /* Wait for the queue not to be full - note: MUST have the mutex
         * locked when calling pthread_cond_wait
         */
        status = pthread_cond_wait( queue->cNotFull, queue->mutex );
    }

    /* OK, at this point, we know there is space on the queue, and that we
     * have it locked for our use.  Time to insert the item.  We insert at the
     * head, and read from the tail.
     */
    queue->itemTable[queue->head] = item;

    /* Now adjust the head to point to the next item modulo numElements */
    queue->head++;
    queue->head &= queue->numMask;

    /* Now update the conditions and signal any thread waiting on them if they 
     * change.
     */
    QueueConditionUpdate( queue );

    /* We are done.  Unlock the mutex. */
    status = pthread_mutex_unlock( queue->mutex );

    return( TRUE );
}



static void QueueConditionUpdate( QueueObject_t *queue )
{
    bool oldFull;
    bool oldEmpty;

    /* NOTE !VERY! carefully:  The mutex for this queue MUST be locked
     * *before* calling this function!
     */

    oldFull = queue->full;
    oldEmpty = queue->empty;

    if( queue->head == queue->tail )
    {
        /* Head and tail point to the same point, the queue is empty */
        queue->empty = TRUE;
        queue->full = FALSE;
    }
    else if( ((queue->head + 1) & queue->numMask) == queue->tail )
    {
        /* Head points to the item before tail, the queue is full */
        queue->empty = FALSE;
        queue->full = TRUE;
#ifdef VERBOSE_QUEUE_DEBUGGING
        LogPrint( LOG_UNKNOWN, "Queue %p is full\n", queue );
#endif
    }
    else
    {
        /* There are between 1 and THRESHOLD items in the queue */
        queue->empty = FALSE;
        queue->full = FALSE;
    }

    if( oldFull != queue->full && queue->full == FALSE )
    {
        /* The queue stopped being full.  We have to signal that condition */
        pthread_cond_signal( queue->cNotFull );
    }

    if( oldEmpty != queue->empty && queue->empty == FALSE )
    {
        /* The queue is no longer empty.  We need to signal that condition */
        pthread_cond_signal( queue->cNotEmpty );
    }
}



QueueItem_t QueueDequeueItem( QueueObject_t *queue, int32 ms_timeout )
{
    int status;
    QueueItem_t item;
    struct timeval tv;
    struct timespec timeout;
    bool timeoutDesired;

    if( queue == NULL )
    {
        return( NULL );
    }

    /* Lock the mutex */
    status = pthread_mutex_lock( queue->mutex );

    /* Setup the timeout value (note: POSIX threads uses absolute time) 
     * A negative timeout means block indefinitely
     */
    if( ms_timeout >= 0 )
    {
        status = 0;
        gettimeofday( &tv, NULL );
        timeout.tv_sec  = tv.tv_sec + (time_t)(ms_timeout / 1000);
        timeout.tv_nsec = (tv.tv_usec * 1000 ) +
                          (long int)((ms_timeout % 1000) * 1000000);
        timeoutDesired = TRUE;
    }
    else
    {
        timeoutDesired = FALSE;
    }

    /* Wait for the appropriate condition variable (unless the condition being
     * waited for is already true).  Loop in case of false signals.
     */
    while( queue->empty == TRUE && 
           ( status != ETIMEDOUT || timeoutDesired == FALSE ) )
    {
        /* Wait for the queue not to be empty - note: MUST have the mutex
         * locked when calling pthread_cond_wait
         */
        if( timeoutDesired == TRUE )
        {
            status = pthread_cond_timedwait( queue->cNotEmpty, queue->mutex,
                                             &timeout );
        }
        else
        {
            /* No timeout requested, block indefinitely */
            status = pthread_cond_wait( queue->cNotEmpty, queue->mutex );
        }

        if( GlobalAbort == TRUE )
        {
            /* The program is trying to shut down, don't dequeue anything */
            status = pthread_mutex_unlock( queue->mutex );
            return( NULL );
        }
    }

    if( timeoutDesired == TRUE && status == ETIMEDOUT )
    {
        /* The queue is still empty and we got a timeout, return NULL */
        status = pthread_mutex_unlock( queue->mutex );

        return( NULL );
    }

    /* OK, at this point, we know there is an item on the queue, and that we
     * have it locked for our use.  Time to remove the item.  We insert at the
     * head, and read from the tail.
     */
    item = queue->itemTable[queue->tail];
    queue->itemTable[queue->tail] = NULL;

    /* Now adjust the tail to point to the next item modulo numElements */
    queue->tail++;
    queue->tail &= queue->numMask;

    /* Now update the conditions and signal any thread waiting on them if they 
     * change.
     */
    QueueConditionUpdate( queue );

    /* We are done.  Unlock the mutex. */
    status = pthread_mutex_unlock( queue->mutex );

    return( item );
}


void QueueClear( QueueObject_t *queue, bool freeItems )
{
    int status;
    uint32 i;

    if( queue != NULL )
    {
        /* Lock the mutex */
        status = pthread_mutex_lock( queue->mutex );

        if( freeItems == TRUE )
        {
            /* Be sure to free all used items.  NOTE: this assumes the items 
             * have been memalloc'd.  If not, expect a segfault!
             */
            for( i = 0; i < queue->numElements; i++ )
            {
                if( queue->itemTable[i] != NULL )
                {
                    memfree( queue->itemTable[i] );
                    queue->itemTable[i] = NULL;
                }
            }
        }

        /* Reset head and tail, regenerate the conditions */
        queue->head = 0;
        queue->tail = 0;
        QueueConditionUpdate( queue );

        /* We are done.  Unlock the mutex. */
        status = pthread_mutex_unlock( queue->mutex );
    }
}


uint32 QueueUsed( QueueObject_t *queue )
{
    uint32 used;

    if( queue == NULL )
    {
        return( 0 );
    }
    used = (queue->head + queue->numElements - queue->tail) & queue->numMask;

    return( used );
}

void QueueDestroy( QueueObject_t *queue )
{
    /*
     * Assumes: queue is empty and locked.  Any items still in the queue will
     * be not be freed
     */

    /*
     * get rid of the two condition vars
     */
    pthread_cond_broadcast( queue->cNotFull );
    pthread_cond_destroy( queue->cNotFull );
    memfree( queue->condNotFull );

    pthread_cond_broadcast( queue->cNotEmpty );
    pthread_cond_destroy( queue->cNotEmpty );
    memfree( queue->condNotEmpty );

    /*
     * get rid of the mutex
     */
    pthread_mutex_unlock( queue->mutex );
    pthread_mutex_destroy( queue->mutex );
    memfree( queue->mutex );

    /*
     * get rid of the item table
     */
    memfree( queue->itemTable );

    LinkedListRemove( QueueList, (LinkedListItem_t *)queue, UNLOCKED );

    /*
     * get rid of the queue object
     */
    memfree( queue );
}

void QueueLock( QueueObject_t *queue )
{
    pthread_mutex_lock( queue->mutex );
}

void QueueUnlock( QueueObject_t *queue )
{
    pthread_mutex_unlock( queue->mutex );
}

uint32 QueueRemoveItem( QueueObject_t *queue, uint32 index, int locked )
{
    /*
     * Assumes queue is locked!  We will be messing with it!
     */

    if( index == queue->head ) {
        queue->itemTable[queue->head] = NULL;
        queue->head++;
        queue->head &= queue->numMask;
        QueueConditionUpdate( queue );
        return( index );
    }

    if( index == queue->tail ) {
        queue->itemTable[queue->tail] = NULL;
        queue->tail--;
        queue->tail &= queue->numMask;
        QueueConditionUpdate( queue );
        return( (index - 1) & queue->numMask );
    }
    
    /* 
     * OK, those were the simple cases.
     */
    if( queue->tail < index ) {
        /*
         * The tail is wrapped, we have to move 3 times
         */
        if( index != queue->numElements - 1 ) {
            memmove( &(queue->itemTable[index]), &(queue->itemTable[index+1]),
                     sizeof(QueueItem_t) * ( queue->numElements - index - 1 ));
        }

        memcpy( &(queue->itemTable[queue->numElements - 1]), 
                &(queue->itemTable[0]), sizeof(QueueItem_t) );

        if( queue->tail ) {
            memmove( &(queue->itemTable[0]), &(queue->itemTable[1]),
                     sizeof(QueueItem_t) * queue->tail );
        }
    } else {
        /*
         * tail is not wrapped from our current position, much easier
         */
        memmove( &(queue->itemTable[index]), &(queue->itemTable[index+1]),
                 sizeof(QueueItem_t) * ( queue->tail - index ) );
    }

    queue->itemTable[queue->tail] = NULL;
    queue->tail--;
    queue->tail &= queue->numMask;
    QueueConditionUpdate( queue );
    return( (index - 1) & queue->numMask );
}

void QueueKillAll( void )
{
    LinkedListItem_t *listItem;
    QueueObject_t  *queue;

    LinkedListLock(QueueList);
    for (listItem = QueueList->head; listItem; listItem = listItem->next) {
        queue = (QueueObject_t *) listItem;

        /* We don't want to flush the log messages */
        if( queue == LoggingQ ) {
            continue;
        }

        /*
         * To allow all listeners to wake up and hear the GlobalAbort,
         * signal both the cNotEmpty and the cNotFull to all 
         */
        pthread_cond_broadcast(queue->cNotEmpty);
        pthread_cond_broadcast(queue->cNotFull);
    }
    LinkedListUnlock(QueueList);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
