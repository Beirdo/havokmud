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
#include "protobuf_api.h"
#include "havokrequest.pb-c.h"
#include "havokresponse.pb-c.h"
#include "db_api.h"


/* Internal protos */
void *protobufMemalloc(void *allocator_data, size_t size);
void protobufMemfree(void *allocator_data, void *pointer);
HavokResponse *protobufHandle( HavokRequest *req );
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

HavokResponse *protobufCreateResponse( void )
{
    HavokResponse  *msg;

    msg = CREATE(HavokResponse);
    if( msg ) {
        havok_response__init( msg );
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

HavokResponse *protobufHandle( HavokRequest *req )
{
    int                 retval;
    PlayerAccount_t    *acct;
    HavokResponse      *resp;

    if( !req ) {
        return( NULL );
    }

    if( req->protocol_version != PROTOBUF_API_VERSION ) {
        LogPrint( LOG_DEBUG, "Bad protobuf API version: %d, expected %d",
                             req->protocol_version, PROTOBUF_API_VERSION );
        return( NULL );
    }

    switch( req->request_type ) {
        case REQ_TYPE__GET_SETTING:
            if( !req->settings_data ) {
                LogPrintNoArg( LOG_DEBUG, "No settings data on GET_SETTING" );
                return( NULL );
            }

            return( db_get_setting( req->settings_data->setting_name ) );
            break;
        case REQ_TYPE__SET_SETTING:
            if( !req->settings_data ) {
                LogPrintNoArg( LOG_DEBUG, "No settings data on SET_SETTING" );
                return( NULL );
            }
            db_set_setting( req->settings_data->setting_name,
                            req->settings_data->setting_value );
            return( NULL );
            break;
        case REQ_TYPE__LOAD_ACCOUNT:
            if( !req->account_data ) {
                LogPrintNoArg( LOG_DEBUG, "No account data on LOAD_ACCOUNT" );
                return( NULL );
            }

            return( db_load_account( req->account_data->email ) );
            break;
        case REQ_TYPE__SAVE_ACCOUNT:
            if( !req->account_data ) {
                LogPrintNoArg( LOG_DEBUG, "No account data on SAVE_ACCOUNT" );
                return( NULL );
            }

            acct = CREATE(PlayerAccount_t);
            acct->email = memstrlink( req->account_data->email );
            acct->id = req->account_data->id;
            acct->pwd = memstrlink( req->account_data->passwd );
            acct->ansi = req->account_data->ansi;
            acct->confirmed = req->account_data->confirmed;
            acct->confcode = memstrlink( req->account_data->confcode );

            return( db_save_account( acct ) );
            break;
        case REQ_TYPE__GET_PC_LIST:
            if( !req->account_data ) {
                LogPrintNoArg( LOG_DEBUG, "No account data on GET_PC_LIST" );
                return( NULL );
            }
            return( db_get_pc_list( req->account_data->id ) );
            break;
        default:
            /* Not handled yet */
            return( NULL );
    }

    /* Should never get here, but just in case! */
    return( NULL );
}

HavokResponse *protobufQueue( HavokRequest *request, 
                              ProtobufResFunc_t callback,
                              void *arg, bool block )
{
    ProtobufItem_t     *item;
    HavokResponse      *response;

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
