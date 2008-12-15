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


void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );

QueryTable_t QueryTable[] = {
    /* 0 */
    { "SELECT `value` FROM `settings` WHERE `name` = ?", NULL, NULL,
      FALSE },
    /* 1 */
    { "INSERT INTO `settings` (`name`, `value`) VALUES (?, ?)", NULL, NULL,
      FALSE },
    /* 2 */
    { "UPDATE `settings` SET `value` = ? WHERE `name` = ?", NULL, NULL, FALSE },
    /* 3 */
    { "SELECT `id`, `email`, `password` FROM `accounts` WHERE `email` = ?",
      NULL, NULL, FALSE },
    /* 4 */
    { "INSERT INTO `accounts (`id`, `email`, `password`) VALUES (?, ?, ?)",
      NULL, NULL, FALSE },
    /* 5 */
    { "UPDATE `accounts` SET `password` = ? WHERE `id` = ?", NULL, NULL, 
       FALSE },
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
    memset(data, 0, 4 * sizeof(MYSQL_BIND));

    bind_string( &data[0], name, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 0, QueryTable, data, 1, result_get_setting, (void *)&result,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    return( result );
}


/*
 * Query chaining functions
 */


void chain_load_classes( MYSQL_RES *res, QueryItem_t *item )
{
#if 0
    int                 count;
    int                 classId;
    int                 i;
    MYSQL_ROW           row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(count = mysql_num_rows(res))) {
        /* No classes!? */
        LogPrintNoArg( LOG_CRIT, "No classes defined in the database!" );
        exit(1);
    }

    classCount = count;
    classes = CREATEN(struct class_def, classCount);
    if( !classes ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating classes[]" );
        exit(1);
    }

    mutex = CREATE(pthread_mutex_t);
    pthread_mutex_init( mutex, NULL );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        classId = atoi(row[0]);
        classes[i].name = strdup(row[1]);
        classes[i].abbrev = strdup(row[2]);

        /* 5 */
        data = CREATEN(MYSQL_BIND, 1);
        memset( data, 0, 1 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], classId, MYSQL_TYPE_LONG );
        db_queue_query( 4, QueryTable, data, 1, result_load_classes_3, 
                        (void *)&i, mutex );

        pthread_mutex_unlock( mutex );
    }
    
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    LogPrintNoArg(LOG_CRIT, "Finished loading classes[] from SQL");
#endif
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
    row = mysql_fetch_row(res);

    value = memstrdup(row[0]);
    *resp = value;
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
