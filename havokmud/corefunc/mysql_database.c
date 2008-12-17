/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008 Gavin Hurlbut
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
 * Copyright 2008 Gavin Hurlbut
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

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @file
 * @brief Contains the API for the system to use to access the MySQL database.
 */

void chain_set_setting( MYSQL_RES *res, QueryItem_t *item );
void chain_save_account( MYSQL_RES *res, QueryItem_t *item );

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );
void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid );
void result_insert_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
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
    { "SELECT `id`, `email`, `passwd`, `ansi`  FROM `accounts` "
      "WHERE `email` = ?", NULL, NULL, FALSE },
    /* 5 */
    { "SELECT `id`, `email`, `passwd`, `ansi`  FROM `accounts` "
      "WHERE `id` = ?", chain_save_account, NULL, FALSE },
    /* 6 */
    { "UPDATE `accounts` SET `email` = ?, `passwd` = ?, `ansi` = ? "
      "WHERE `id` = ?", NULL, NULL, FALSE },
    /* 7 */
    { "INSERT INTO `accounts` (`id`, `email`, `passwd`, `ansi`) "
      "VALUES (?, ?, ?, ?)",
      NULL, NULL, FALSE },
    /* END */
    { NULL, NULL, NULL, FALSE }
};

char *db_get_setting(char *name)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    char               *result;

    if( !name ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    pthread_mutex_init( mutex, NULL );

    data = CREATEN(MYSQL_BIND, 1);
    memset(data, 0, 1 * sizeof(MYSQL_BIND));

    bind_string( &data[0], name, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 0, QueryTable, data, 1, result_get_setting, (void *)&result,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( result );
}
        
void db_set_setting( char *name, char *format, ... )
{
    MYSQL_BIND     *data;
    char            value[256];
    va_list         arguments;
    
    if( !name || !format ) {
        return;
    }
    
    data = (MYSQL_BIND *)malloc( 2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    va_start( arguments, format );
    vsnprintf( value, 256, format, arguments );
    va_end( arguments );
    
    bind_string( &data[0], name, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[1], value, MYSQL_TYPE_VAR_STRING );
    
    db_queue_query( 1, QueryTable, data, 2, NULL, NULL, NULL);
}
    
PlayerAccount_t *db_load_account( char *email )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    PlayerAccount_t    *result;

    if( !email ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    pthread_mutex_init( mutex, NULL );

    data = CREATEN(MYSQL_BIND, 1);
    memset(data, 0, 1 * sizeof(MYSQL_BIND));

    bind_string( &data[0], email, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 4, QueryTable, data, 1, result_load_account, 
                    (void *)&result, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( result );
}

void db_save_account( PlayerAccount_t *account )
{
    MYSQL_BIND         *data;

    if( !account ) {
        return;
    }

    data = CREATEN(MYSQL_BIND, 5);
    memset(data, 0, 5 * sizeof(MYSQL_BIND));

    bind_numeric( &data[0], account->id, MYSQL_TYPE_LONG );
    bind_string( &data[1], account->email, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[2], account->pwd, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[3], (account->ansi ? 1 : 0), MYSQL_TYPE_TINY );
    bind_null_blob( &data[4], account );

    db_queue_query( 5, QueryTable, data, 5, NULL, NULL, NULL );
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
        /* Swap the order of the two parameters */
        memcpy( temp, data, sizeof( MYSQL_BIND ) );
        memcpy( data, &data[1], sizeof( MYSQL_BIND ) );
        memcpy( &data[1], temp, sizeof( MYSQL_BIND ) );
        db_queue_query( 2, QueryTable, data, 2, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 3, QueryTable, data, 2, NULL, NULL, NULL );
    }
}

void chain_save_account( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[1];
    PlayerAccount_t *account;
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    account = (PlayerAccount_t *)data[4].buffer;
    
    if( count ) {
        /* update */
        /* Swap the order of the two parameters */
        memcpy( temp, data, sizeof( MYSQL_BIND ) );
        memmove( data, &data[1], sizeof( MYSQL_BIND ) * 3 );
        memcpy( &data[3], temp, sizeof( MYSQL_BIND ) );
        db_queue_query( 6, QueryTable, data, 4, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 7, QueryTable, data, 4, result_insert_account, account,
                        NULL );
    }
}



/*
 * Query result callbacks
 */

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid )
{
    char          **resp;
    char           *value;
    int             count;
    MYSQL_ROW       row;

    resp = (char **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    value = memstrdup(row[0]);
    *resp = value;
}

void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid )
{
    PlayerAccount_t   **resp;
    PlayerAccount_t    *account;
    int                 count;
    MYSQL_ROW           row;

    resp = (PlayerAccount_t **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    account = CREATE(PlayerAccount_t);
    account->id    = atoi(row[0]);
    account->email = memstrdup(row[1]);
    account->pwd   = memstrdup(row[2]);
    account->ansi  = (atoi(row[3]) ? TRUE : FALSE);
    *resp = account;
}


void result_insert_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                            long insertid )
{
    PlayerAccount_t    *account;

    if( !arg ) {
        return;
    }

    account = (PlayerAccount_t *)arg;
    account->id = insertid;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
