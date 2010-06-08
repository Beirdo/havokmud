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
#include "protobufs.h"
#include "havokrequest.pb-c.h"


/* Internal protos */
void *protobufMemalloc(void *allocator_data, size_t size);
void protobufMemfree(void *allocator_data, void *pointer);
ProtobufCMessage   *protobufHandle( HavokRequest *request );
void protobufDestroyItem( ProtobufItem_t *item );

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
    protobuf_c_default_allocator.alloc     = protobufMemalloc;
    protobuf_c_default_allocator.free      = protobufMemfree;
    protobuf_c_default_allocator.tmp_alloc = NULL;

    pthread_mutex_unlock( startupMutex );

    while( !GlobalAbort ) {
        item = (ProtobufItem_t *)QueueDequeueItem( ProtobufQ, -1 );
        if( !item ) {
            continue;
        }

        item->response = protobufHandle( item->request );
        if( item->callback ) {
            item->callback( item->response, item->callbackArg );
        }

        if( item->mutex ) {
            pthread_mutex_unlock( item->mutex );
        } else {
            protobufDestroyItem( item );
            protobufDestroyMessage( (ProtobufCMessage *)item->response );
        }
    }

    LogPrintNoArg( LOG_NOTICE, "Ending ProtobufThread" );
    return(NULL);
}

void protobufDestroyMessage( ProtobufCMessage *msg )
{
    if( msg ) {
        protobuf_c_message_free_unpacked( msg, NULL );
    }
}

void protobufDestroyItem( ProtobufItem_t *item )
{
    if( !item ) {
        return;
    }

    protobufDestroyMessage( (ProtobufCMessage *)item->request );

    if( item->mutex ) {
        pthread_mutex_unlock( item->mutex );
        pthread_mutex_destroy( item->mutex );
        memfree( item->mutex );
    }

    memfree( item );
}

HavokRequest *protobufCreateRequest( void )
{
    HavokRequest   *msg;

    msg = CREATE(HavokRequest);
    if( msg ) {
        havok_request__init( msg );
    }

    msg->protocol_version = PROTOBUF_API_VERSION;

    return( msg );
}

void *protobufMemalloc(void *allocator_data, size_t size)
{
    void       *buf;

    (void)allocator_data;

    if( size == 0 ) {
        return( NULL );
    }

    buf = CREATEN(char, size);
    if( !buf ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory in protobufMemalloc" );
    }
    return( buf );
}

void protobufMemfree(void *allocator_data, void *pointer)
{
    (void)allocator_data;

    if( pointer ) {
        memfree( pointer );
    }
}

ProtobufCMessage   *protobufHandle( HavokRequest *request )
{
    return( NULL );
}

ProtobufCMessage *protobufQueue( HavokRequest *request, 
                                 ProtobufResFunc_t callback,
                                 void *arg, bool block )
{
    ProtobufItem_t     *item;
    ProtobufCMessage   *response;

    if( !request ) {
        return;
    }

    item = CREATE(ProtobufItem_t);
    if( !item ) {
        return;
    }

    item->request     = request;
    item->callback    = callback;
    item->callbackArg = arg;
    
    if( block ) {
        item->mutex = CREATE(pthread_mutex_t);
        pthread_mutex_init( item->mutex, NULL );
        pthread_mutex_lock( item->mutex );
    }

    QueueEnqueueItem( ProtobufQ, item );

    if( !block ) {
        return( NULL );
    }

    pthread_mutex_lock( item->mutex );
    response = item->response;
    protobufDestroyItem( item );

    return( response );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
