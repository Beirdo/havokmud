/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2010 Gavin Hurlbut
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
 * Copyright 2010 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Thread to handle all protobuf access
 */

#include "environment.h"
#include <pthread.h>
#include "google/protobuf-c/protobuf-c.h"
#include "interthread.h"
#include "structs.h"
#include "queue.h"
#include "memory.h"
#include "protos.h"
#include "logging.h"

typedef void (*ProtobufResFunc_t)( ProtobufCMessage *result, void *arg );

typedef struct {
    ProtobufCMessage   *request;
    ProtobufResFunc_t   callback;
    void               *callbackArg;
    pthread_mutex_t    *mutex;
} ProtobufItem_t;



/* Internal protos */
void *protobuf_memalloc(void *allocator_data, size_t size);
void protobuf_memfree(void *allocator_data, void *pointer);
ProtobufCMessage   *protobufHandle( ProtobufCMessage *request );

/**
 * @brief Thread to handle all protobuf accesses
 * @param arg unused
 * @return never returns until shutdown
 *
 * Receives protobuf requests via a queue, returns results via callbacks
 */
void *ProtobufThread( void *arg )
{
    ProtobufItem_t     *item;
    ProtobufCMessage   *resp;

    pthread_mutex_lock( startupMutex );

    /* Setup to use our own memory allocator */
    protobuf_c_default_allocator.alloc     = protobuf_memalloc;
    protobuf_c_default_allocator.free      = protobuf_memfree;
    protobuf_c_default_allocator.tmp_alloc = NULL;

    pthread_mutex_unlock( startupMutex );

    while( !GlobalAbort ) {
        item = (ProtobufItem_t *)QueueDequeueItem( ProtobufQ, -1 );
        if( !item ) {
            continue;
        }

        resp = protobufHandle( item->request );
        if( item->callback ) {
            item->callback( resp, item->callbackArg );
        }

        if( item->mutex ) {
            pthread_mutex_unlock( item->mutex );
        }

        if( resp ) {
            protobuf_c_message_free_unpacked( resp, NULL );
        }

        if( item->request ) {
            protobuf_c_message_free_unpacked( item->request, NULL );
        }

        memfree( item );
    }

    LogPrintNoArg( LOG_NOTICE, "Ending ProtobufThread" );
    return(NULL);
}

void *protobuf_memalloc(void *allocator_data, size_t size)
{
    void       *buf;

    (void)allocator_data;

    if( size == 0 ) {
        return( NULL );
    }

    buf = CREATEN(char, size);
    if( !buf ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory in protobuf_memalloc" );
    }
    return( buf );
}

void protobuf_memfree(void *allocator_data, void *pointer)
{
    (void)allocator_data;

    if( pointer ) {
        memfree( pointer );
    }
}

ProtobufCMessage   *protobufHandle( ProtobufCMessage *request )
{
    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
