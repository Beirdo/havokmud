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
 *
 * Copyright 2010 Gavin Hurlbut
 * All rights reserved
 *
 * Comments :
 *
 * Handles MySQL database connections
 */

#include "config.h"
#include "environment.h"

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdarg.h>

#include "oldstructs.h"
#include "oldutils.h"
#include "oldexterns.h"
#include "logging.h"
#include "interthread.h"
#include "protos.h"
#include "memory.h"
#include "db_api.h"
#include "protobuf_api.h"
#include "protected_data.h"

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @file
 * @brief Contains the API for the system to use to access the MySQL database.
 */

#define COL_INT(row, x)     (atoi((row)[(x)]))
#define COL_BOOL(row, x)    ((atoi((row)[(x)])) ? TRUE : FALSE)
#define COL_STRING(row, x)  (memstrlink((row)[(x)]))

HavokResponse *db_mysql_get_setting( HavokRequest *req );
HavokResponse *db_mysql_set_setting( HavokRequest *req );
HavokResponse *db_mysql_load_account( HavokRequest *req );
HavokResponse *db_mysql_save_account( HavokRequest *req );
HavokResponse *db_mysql_get_pc_list( HavokRequest *req );
HavokResponse *db_mysql_load_pc( HavokRequest *req );
HavokResponse *db_mysql_save_pc( HavokRequest *req );
HavokResponse *db_mysql_find_pc( HavokRequest *req );

char *db_mysql_load_pc_attribs( int pc_id );
void db_mysql_save_pc_attribs( int pc_id, char *json );

void chain_set_setting( MYSQL_RES *res, QueryItem_t *item );
void chain_save_account( MYSQL_RES *res, QueryItem_t *item );
void chain_save_pc( MYSQL_RES *res, QueryItem_t *item );
void chain_save_pc_attribs( MYSQL_RES *res, QueryItem_t *item );

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );
void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid );
void result_insert_id( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                       long insertid );
void result_get_pc_list( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );
void result_load_pc( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                     long insertid );
void result_load_pc_attribs( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                             long insertid );
void result_find_pc( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                     long insertid );

QueryTable_t QueryTable[] = {
    /* 0 */
    { "SELECT `value` FROM `settings` WHERE `name` = ? LIMIT 1", NULL, NULL,
      FALSE },
    /* 1 */
    { "SELECT `value` FROM `settings` WHERE `name` = ? LIMIT 1",
      chain_set_setting, NULL, FALSE },
    /* 2 */
    { "UPDATE `settings` SET `value` = ? WHERE `name` = ?", NULL, NULL, FALSE },
    /* 3 */
    { "INSERT INTO `settings` (`name`, `value`) VALUES (?, ?)", NULL, NULL,
      FALSE },
    /* 4 */
    { "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, `confcode` "
      "FROM `accounts` WHERE `email` = ?", NULL, NULL, FALSE },
    /* 5 */
    { "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, `confcode` "
      "FROM `accounts` WHERE `id` = ?", chain_save_account, NULL, FALSE },
    /* 6 */
    { "UPDATE `accounts` SET `email` = ?, `passwd` = ?, `ansi` = ?, "
      "`confirmed` = ?, `confcode` = ? WHERE `id` = ?", NULL, NULL, FALSE },
    /* 7 */
    { "INSERT INTO `accounts` (`email`, `passwd`, `ansi`, `confirmed`, "
      "`confcode`) VALUES (?, ?, ?, ?, ?)",
      NULL, NULL, FALSE },
    /* 8 */
    { "SELECT `id`, `account_id`, `name` FROM `pcs` "
      "WHERE `account_id` = ? ORDER BY `id`", NULL, NULL, FALSE },
    /* 9 */
    { "SELECT `id`, `account_id`, `name` FROM `pcs` "
      "WHERE `account_id` = ? AND `id` = ? ORDER BY `id`", NULL, NULL, FALSE },
    /* 10 */
    { "SELECT `id`, `account_id`, `name` FROM `pcs` "
      "WHERE `id` = ? ORDER BY `id` LIMIT 1", chain_save_pc, NULL, FALSE },
    /* 11 */
    { "UPDATE `pcs` SET `account_id` = ?, `name` = ? "
      "WHERE `id` = ?", NULL, NULL, FALSE },
    /* 12 */
    { "INSERT INTO `pcs` (`account_id`, `name`) VALUES (?, ?)", NULL, NULL,
      FALSE },
    /* 13 */
    { "SELECT `attribsrc`, `attribjson` FROM `pcattribs` WHERE `pc_id` = ?",
      NULL, NULL, FALSE },
    /* 14 */
    { "SELECT `attribjson` FROM `pcattribs` WHERE `pc_id` = ? "
      "AND `attribsrc` = ?", chain_save_pc_attribs, NULL, FALSE },
    /* 15 */
    { "UPDATE `pcattribs` SET `attribjson` = ? WHERE `pc_id` = ? "
      "AND `attribsrc` = ?", NULL, NULL, FALSE },
    /* 16 */
    { "INSERT INTO `pcattribs` (`pc_id`, `attribsrc`, `attribjson`) "
      "VALUES (?, ?, ?)", NULL, NULL, FALSE },
    /* 17 */
    { "SELECT `id`, `account_id`, `name` FROM `pcs` WHERE `name` = ?",
      NULL, NULL, FALSE },
    /* END */
    { NULL, NULL, NULL, FALSE }
};

void db_mysql_init( void )
{
    db_api_funcs.get_setting  = db_mysql_get_setting;
    db_api_funcs.set_setting  = db_mysql_set_setting;
    db_api_funcs.load_account = db_mysql_load_account;
    db_api_funcs.save_account = db_mysql_save_account;
    db_api_funcs.get_pc_list  = db_mysql_get_pc_list;
    db_api_funcs.load_pc      = db_mysql_load_pc;
    db_api_funcs.save_pc      = db_mysql_save_pc;
    db_api_funcs.find_pc      = db_mysql_find_pc;
}

HavokResponse *db_mysql_get_setting( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;
    char               *result;

    if( !req || !req->settings_data->setting_name ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_string( &data[0], req->settings_data->setting_name, 
                 MYSQL_TYPE_VAR_STRING );
    result = NULL;

    db_queue_query( 0, QueryTable, data, 1, result_get_setting, (void *)&resp,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( resp );
}
        
HavokResponse *db_mysql_set_setting( HavokRequest *req )
{
    MYSQL_BIND     *data;
    
    if( !req || !req->settings_data->setting_name || 
        !req->settings_data->setting_value ) {
        return( NULL );
    }
    
    data = CREATEN(MYSQL_BIND, 2);

    bind_string( &data[0], req->settings_data->setting_name, 
                 MYSQL_TYPE_VAR_STRING );
    bind_string( &data[1], req->settings_data->setting_value, 
                 MYSQL_TYPE_VAR_STRING );
    
    db_queue_query( 1, QueryTable, data, 2, NULL, NULL, NULL);

    return( NULL );
}
    
HavokResponse *db_mysql_load_account( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;

    if( !req || !req->account_data->email ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_string( &data[0], req->account_data->email, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 4, QueryTable, data, 1, result_load_account, (void *)&resp,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( resp );
}

HavokResponse *db_mysql_save_account( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;
    ProtectedData_t    *protect;
    volatile int        id;

    if( !req || !req->account_data ) {
        return;
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    protect = ProtectedDataCreate();
    protect->data = (void *)&id;
    ProtectedDataLock( protect );

    data = CREATEN(MYSQL_BIND, 7);

    bind_numeric( &data[0], req->account_data->id, MYSQL_TYPE_LONG );
    bind_string( &data[1], req->account_data->email, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[2], req->account_data->passwd, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[3], (req->account_data->ansi ? 1 : 0), 
                            MYSQL_TYPE_TINY );
    bind_numeric( &data[4], (req->account_data->confirmed ? 1 : 0), 
                            MYSQL_TYPE_TINY );
    bind_string( &data[5], (req->account_data->confcode ? 
                            req->account_data->confcode : ""), 
                           MYSQL_TYPE_VAR_STRING );
    bind_null_blob( &data[6], protect );

    db_queue_query( 5, QueryTable, data, 7, NULL, NULL, mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    ProtectedDataLock( protect );
    ProtectedDataDestroy( protect );

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    resp->request_type = REQ_TYPE__SAVE_ACCOUNT;
    resp->account_data = CREATE(ReqAccountType);
    req_account_type__init( resp->account_data );
    memcpy( resp->account_data, req->account_data, sizeof(ReqAccountType) );
    resp->account_data->id = id;
    resp->account_data->email    = memstrlink( req->account_data->email );
    resp->account_data->passwd   = memstrlink( req->account_data->passwd );
    resp->account_data->confcode = memstrlink( req->account_data->confcode );

    return( resp );
}

HavokResponse *db_mysql_get_pc_list( HavokRequest *req)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;

    if( !req || !req->account_data->id ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_numeric( &data[0], req->account_data->id, MYSQL_TYPE_LONG );

    db_queue_query( 8, QueryTable, data, 1, result_get_pc_list, (void *)&resp, 
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( resp );
}

HavokResponse *db_mysql_load_pc( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;

    if( !req || !req->pc_data->account_id || !req->pc_data->id ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 2);

    bind_numeric( &data[0], req->pc_data->account_id, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], req->pc_data->id, MYSQL_TYPE_LONG );

    db_queue_query( 9, QueryTable, data, 2, result_load_pc, (void *)&resp, 
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    resp->pc_data[0]->attribs = db_mysql_load_pc_attribs( req->pc_data->id );
    return( resp );
}

HavokResponse *db_mysql_save_pc( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;
    ProtectedData_t    *protect;
    volatile int        id;

    if( !req || !req->pc_data ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    protect = ProtectedDataCreate();
    protect->data = (void *)&id;
    ProtectedDataLock( protect );

    data = CREATEN(MYSQL_BIND, 4);

    bind_numeric( &data[0], req->pc_data->id, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], req->pc_data->account_id, MYSQL_TYPE_LONG );
    bind_string( &data[2], req->pc_data->name, MYSQL_TYPE_VAR_STRING );
    bind_null_blob( &data[3], protect );

    db_queue_query( 10, QueryTable, data, 4, NULL, NULL, mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    ProtectedDataLock( protect );
    ProtectedDataDestroy( protect );

    db_mysql_save_pc_attribs( id, req->pc_data->attribs );

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    resp->request_type = REQ_TYPE__SAVE_PC;
    resp->n_pc_data = 1;
    resp->pc_data = CREATE(ReqPCType *);
    resp->pc_data[0] = CREATE(ReqPCType);
    req_pctype__init( resp->pc_data[0] );
    memcpy( resp->pc_data[0], req->pc_data, sizeof(ReqPCType) );
    resp->pc_data[0]->id   = id;
    resp->pc_data[0]->name = memstrlink( req->pc_data->name );

    return( resp );
}

char *db_mysql_load_pc_attribs( int pc_id )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    char               *resp;

    if( !pc_id ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_numeric( &data[0], pc_id, MYSQL_TYPE_LONG );

    db_queue_query( 13, QueryTable, data, 1, result_load_pc_attribs, 
                    (void *)&resp, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( resp );
}

void db_mysql_save_pc_attribs( int pc_id, char *json )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    JSONSource_t       *js;
    JSONSource_t       *jsItem;

    if( !pc_id || !json ) {
        return;
    }

    js = SplitJSON( json );
    if( !js ) {
        return;
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    for( jsItem = js; jsItem->source; jsItem++ ) {
        data = CREATEN(MYSQL_BIND, 3);

        bind_numeric( &data[0], pc_id, MYSQL_TYPE_LONG );
        bind_string( &data[1], jsItem->source, MYSQL_TYPE_VAR_STRING );
        bind_string( &data[2], jsItem->json, MYSQL_TYPE_VAR_STRING );

        db_queue_query( 14, QueryTable, data, 3, NULL, NULL, mutex );

        pthread_mutex_unlock( mutex );
    }

    pthread_mutex_destroy( mutex );
    memfree( mutex );

    DestroyJSONSource( js );
}

HavokResponse *db_mysql_find_pc( HavokRequest *req )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;

    if( !req || !req->pc_data || !req->pc_data->name ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_string( &data[0], req->pc_data->name, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 17, QueryTable, data, 1, result_find_pc, (void *)&resp, 
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( resp );
}

/*
 * Query chaining functions
 */

void chain_set_setting( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[1];
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }
    
    if( count ) {
        /* update */
        /* Swap the arguments around */
        memcpy( temp,     &data[0], sizeof(MYSQL_BIND) );
        memcpy( &data[0], &data[1], sizeof(MYSQL_BIND) );
        memcpy( &data[1], temp,     sizeof(MYSQL_BIND) );
        db_queue_query( 2, QueryTable, data, 2, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 3, QueryTable, data, 2, NULL, NULL, NULL );
    }
}

void chain_save_account( MYSQL_RES *res, QueryItem_t *item )
{
    int                 count;
    MYSQL_BIND         *data;
    MYSQL_BIND          temp[1];
    ProtectedData_t    *protect;
    int                *id;
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    protect = (ProtectedData_t *)data[6].buffer;
    id  = (int *)protect->data;
    *id = *(int *)data[0].buffer;

    /* swap argument order */
    memcpy(  temp,     &data[0], sizeof(MYSQL_BIND) );
    memmove( &data[0], &data[1], sizeof(MYSQL_BIND) * 5 );
    memcpy(  &data[5], temp,     sizeof(MYSQL_BIND) );
    
    if( count ) {
        /* update */
        ProtectedDataUnlock( protect );
        db_queue_query( 6, QueryTable, data, 6, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 7, QueryTable, data, 5, result_insert_id, protect,
                        NULL );
    }
}

void chain_save_pc( MYSQL_RES *res, QueryItem_t *item )
{
    int                 count;
    MYSQL_BIND         *data;
    MYSQL_BIND          temp[1];
    ProtectedData_t    *protect;
    int                *id;
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    protect = (ProtectedData_t *)data[3].buffer;
    id  = (int *)protect->data;
    *id = *(int *)data[0].buffer;

    /* swap argument order */
    memcpy(  temp,     &data[0], sizeof(MYSQL_BIND) );
    memmove( &data[0], &data[1], sizeof(MYSQL_BIND) * 2 );
    memcpy(  &data[2], temp,     sizeof(MYSQL_BIND) );
    
    if( count ) {
        /* update */
        ProtectedDataUnlock( protect );
        LogPrint(LOG_DEBUG, "Updating id %d", *id);
        db_queue_query( 11, QueryTable, data, 3, NULL, NULL, NULL );
    } else {
        /* insert */
        *id = 0;
        LogPrintNoArg(LOG_DEBUG, "Inserting");
        db_queue_query( 12, QueryTable, data, 2, result_insert_id, protect, 
                        NULL );
    }
}

void chain_save_pc_attribs( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[1];
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    if( count ) {
        /* update */
        /* swap argument order */
        memcpy(  temp,     &data[2], sizeof(MYSQL_BIND) );
        memmove( &data[1], &data[0], sizeof(MYSQL_BIND) * 2 );
        memcpy(  &data[0], temp,     sizeof(MYSQL_BIND) );
        db_queue_query( 15, QueryTable, data, 3, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 16, QueryTable, data, 3, NULL, NULL, NULL );
    }
}
/*
 * Query result callbacks
 */

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid )
{
    HavokResponse **resp;
    int             count;
    MYSQL_ROW       row;

    resp = (HavokResponse **)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    *resp = protobufCreateResponse();
    if( !*resp ) {
        return;
    }

    (*resp)->request_type = REQ_TYPE__GET_SETTING;
    (*resp)->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( (*resp)->settings_data );
    (*resp)->settings_data->setting_name  = memstrlink( input[0].buffer );
    (*resp)->settings_data->setting_value = COL_STRING(row, 0);
}

void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid )
{
    HavokResponse     **resp;
    int                 count;
    MYSQL_ROW           row;

    resp = (HavokResponse **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    *resp = protobufCreateResponse();
    if( !resp ) {
        return;
    }

    (*resp)->request_type = REQ_TYPE__LOAD_ACCOUNT;
    (*resp)->account_data = CREATE(ReqAccountType);
    req_account_type__init( (*resp)->account_data );
    (*resp)->account_data->id            = COL_INT(row, 0);
    (*resp)->account_data->email         = COL_STRING(row, 1);
    (*resp)->account_data->passwd        = COL_STRING(row, 2);
    (*resp)->account_data->ansi          = COL_BOOL(row, 3);
    (*resp)->account_data->confirmed     = COL_BOOL(row, 4);
    (*resp)->account_data->confcode      = COL_STRING(row, 5);
    (*resp)->account_data->has_id        = TRUE;
    (*resp)->account_data->has_ansi      = TRUE;
    (*resp)->account_data->has_confirmed = TRUE;
}


void result_insert_id( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                       long insertid )
{
    ProtectedData_t    *protect;
    int                *id;

    if( !arg ) {
        return;
    }

    protect = (ProtectedData_t *)arg;
    id = (int *)protect->data;
    *id = insertid;
    LogPrint(LOG_CRIT, "New ID: %d", insertid);
    ProtectedDataUnlock( protect );
}

void db_fill_row_load_pc( MYSQL_ROW row, HavokResponse *resp, int i )
{
    if( !resp || !resp->pc_data ) {
        return;
    }

    resp->pc_data[i] = CREATE(ReqPCType);
    req_pctype__init( resp->pc_data[i] );

    resp->pc_data[i]->id                  = COL_INT(row, 0);
    resp->pc_data[i]->account_id          = COL_INT(row, 1);
    resp->pc_data[i]->name                = COL_STRING(row, 2);
    resp->pc_data[i]->has_id              = TRUE;
    resp->pc_data[i]->has_account_id      = TRUE;
}

void result_get_pc_list( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid )
{
    HavokResponse     **resp;
    int                 count;
    MYSQL_ROW           row;
    int                 i;

    resp = (HavokResponse **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }

    *resp = protobufCreateResponse();
    if( !*resp ) {
        return;
    }

    (*resp)->request_type = REQ_TYPE__GET_PC_LIST;
    (*resp)->n_pc_data = count;
    (*resp)->pc_data = CREATEN(ReqPCType *, count);

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        db_fill_row_load_pc( row, *resp, i );
    }
}

void result_load_pc( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                     long insertid )
{
    HavokResponse     **resp;
    int                 count;
    MYSQL_ROW           row;

    resp = (HavokResponse **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }

    *resp = protobufCreateResponse();
    if( !*resp ) {
        return;
    }

    (*resp)->request_type = REQ_TYPE__GET_PC_LIST;
    (*resp)->n_pc_data = 1;
    (*resp)->pc_data = CREATE(ReqPCType *);

    row = mysql_fetch_row(res);
    db_fill_row_load_pc( row, *resp, 0 );
}

void result_load_pc_attribs( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                             long insertid )
{
    char              **json;
    JSONSource_t       *js;
    int                 count;
    int                 i;
    MYSQL_ROW           row;

    json = (char **)arg;
    *json = NULL;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        return;
    }

    js = CREATEN(JSONSource_t, count+1);
    if( !js ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        js[i].source = COL_STRING(row, 0);
        js[i].json   = COL_STRING(row, 1);
    }

    *json = CombineJSON(js);
    memfree(js);
}

void result_find_pc( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                     long insertid )
{
    HavokResponse     **resp;
    int                 count;
    MYSQL_ROW           row;

    resp = (HavokResponse **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }

    *resp = protobufCreateResponse();
    if( !*resp ) {
        return;
    }

    (*resp)->request_type = REQ_TYPE__FIND_PC;
    (*resp)->n_pc_data = 1;
    (*resp)->pc_data = CREATE(ReqPCType *);

    row = mysql_fetch_row(res);
    (*resp)->pc_data[0] = CREATE(ReqPCType);
    db_fill_row_load_pc( row, *resp, 0 );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
