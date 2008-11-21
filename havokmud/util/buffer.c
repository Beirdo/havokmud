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
*----------------------------------------------------------
*/

/* INCLUDE FILES */
#include "environment.h"
#include "memory.h"
#include "buffer.h"
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include "logging.h"

/**
 * @file
 * @brief Circular Buffer Data Structure
 */

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */
static void BufferConditionUpdate( BufferObject_t *buffer );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";


BufferObject_t * BufferCreate( uint32 size )
{
    BufferObject_t *buffer;
    int status;

    if( size == 0 )
    {
        return( NULL );
    }

    buffer = CREATE(BufferObject_t);

    buffer->data = CREATEN(char, size);
    buffer->size = size;
    buffer->head = 0;
    buffer->tail = 0;

    /* Initialize the mutex */
    buffer->mutex = CREATE(pthread_mutex_t);
    status = pthread_mutex_init( buffer->mutex, NULL );

    /* Initialize the condition variables */
    buffer->full = FALSE;
    buffer->cNotFull = CREATE(pthread_cond_t);
    status = pthread_cond_init( buffer->cNotFull, NULL );

    buffer->empty = TRUE;
    buffer->cNotEmpty = CREATE(pthread_cond_t);
    status = pthread_cond_init( buffer->cNotEmpty, NULL );

    return( buffer );
}

void BufferDestroy( BufferObject_t *buffer )
{
    if( !buffer ) {
        return;
    }

    memfree( buffer->data );
    pthread_cond_broadcast( buffer->cNotFull );
    pthread_cond_destroy( buffer->cNotFull );

    pthread_cond_broadcast( buffer->cNotEmpty );
    pthread_cond_destroy( buffer->cNotEmpty );

    pthread_mutex_unlock( buffer->mutex );
    pthread_mutex_destroy( buffer->mutex );
    memfree( buffer );
}

void BufferLock( BufferObject_t *buffer )
{
    if( buffer ) {
        pthread_mutex_lock( buffer->mutex );
    }
}

void BufferUnlock( BufferObject_t *buffer )
{
    if( buffer ) {
        pthread_mutex_unlock( buffer->mutex );
    }
}

char *BufferGetWrite( BufferObject_t *buffer )
{
    if( !buffer || buffer->full ) {
        return( NULL );
    }

    return( &(buffer->data[buffer->tail]) );
}

char *BufferGetRead( BufferObject_t *buffer ) 
{
    if( !buffer || buffer->empty ) {
        return( NULL );
    }

    return( &(buffer->data[buffer->head]) );
}

void BufferClear( BufferObject_t *buffer, bool lock )
{
    if( !buffer ) {
        return;
    }

    BufferLock(buffer);

    buffer->head = 0;
    buffer->tail = 0;
    BufferConditionUpdate(buffer);

    BufferUnlock(buffer);
}

uint32 BufferAvailWrite( BufferObject_t *buffer, bool lock )
{
    uint32 avail;

    if( !buffer ) {
        return( 0 );
    }

    if( lock ) {
        BufferLock(buffer);
    }

    avail = buffer->size - buffer->tail;
    if( avail <= 0 ) {
        avail = 0;
        if( lock ) {
            BufferUnlock(buffer);
        }
    }
    return(avail);
}

uint32 BufferAvailRead( BufferObject_t *buffer, bool lock )
{
    uint32 avail;

    if( !buffer ) {
        return( 0 );
    }

    if( lock ) {
        BufferLock(buffer);
    }

    avail = buffer->tail - buffer->head;
    if( avail <= 0 ) {
        avail = 0;
        if( lock ) {
            BufferUnlock(buffer);
        }
    }
    return( avail );
}
        
void BufferWroteBytes( BufferObject_t *buffer, uint32 count )
{
    if( !buffer ) {
        return;
    }

    buffer->tail += count;
    BufferConditionUpdate( buffer );
}


void BufferReadBytes( BufferObject_t *buffer, uint32 count )
{
    if( !buffer ) {
        return;
    }

    buffer->head += count;
    BufferConditionUpdate( buffer );
}



static void BufferConditionUpdate( BufferObject_t *buffer )
{
    bool oldFull;
    bool oldEmpty;

    /* NOTE !VERY! carefully:  The mutex for this queue MUST be locked
     * *before* calling this function!
     */

    oldFull = buffer->full;
    oldEmpty = buffer->empty;

    if( buffer->head == buffer->tail )
    {
        /* Head and tail point to the same point, the queue is empty */
        buffer->empty = TRUE;
        buffer->head = 0;
        buffer->tail = 0;
    } else {
        buffer->empty = FALSE;
    }

    if( buffer->tail >= buffer->size )
    {
        buffer->full = TRUE;
    } else {
        buffer->full = FALSE;
    }

    if( oldFull != buffer->full && buffer->full == FALSE )
    {
        /* The queue stopped being full.  We have to signal that condition */
        pthread_cond_signal( buffer->cNotFull );
    }

    if( oldEmpty != buffer->empty && buffer->empty == FALSE )
    {
        /* The queue is no longer empty.  We need to signal that condition */
        pthread_cond_signal( buffer->cNotEmpty );
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
