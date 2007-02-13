/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
 * Copyright 2007 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Thread to handle all MySQL accesses
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include "logging.h"
#include <mysql.h>
#include <string.h>
#include "protected_data.h"

static char ident[] _UNUSED_ =
    "$Id$";

typedef struct {
    MYSQL  *sql;
    char    sqlbuf[MAX_STRING_LENGTH];
    int     buflen;
} MysqlData_t;

static ProtectedData_t *sql;
pthread_t       sqlThreadId;

/* Internal protos */
char *db_quote(char *string);
MYSQL_RES *db_query( const char *query, MYSQL_BIND *args, int arg_count );


QueueObject_t   *QueryQ;

char *mySQL_db     = NULL;
char *mySQL_user   = NULL;
char *mySQL_passwd = NULL;
char *mySQL_host   = NULL;
int   mySQL_port   = 3306;



/**
 * @brief Thread to handle all MySQL accesses
 * @param arg unused
 * @return never returns until shutdown
 *
 * Receives SQL requests via a queue, returns results via callbacks
 */
void *MysqlThread( void *arg )
{
    QueryItem_t        *item;
    QueryTable_t       *query;
    MYSQL_RES          *res;
    int                 i;

    LogPrintNoArg( LOG_NOTICE, "Starting MySQL thread" );
    mysql_thread_init();

    while( !GlobalAbort ) {
        item = (QueryItem_t *)QueueDequeueItem( QueryQ, -1 );
        if( !item ) {
            continue;
        }

        query = &item->queryTable[item->queryId];
        res = db_query( query->queryPattern, item->queryData, 
                        item->queryDataCount );
        if( res ) {
            if( item->queryCallback ) {
                item->queryCallback( res, item->queryData, 
                                     item->queryCallbackArg );
            } else if( query->queryChainFunc ) {
                query->queryChainFunc( res, item );
            }
            mysql_free_result(res);
        }

        if( item->queryMutex ) {
            pthread_mutex_unlock( item->queryMutex );
        }

        if( !query->queryChainFunc ) {
            for( i = 0; i < item->queryDataCount; i++ ) {
                if( !item->queryData[i].is_null || 
                    !(*item->queryData[i].is_null) ) {
                    free( item->queryData[i].buffer );
                }
            }
        }

        free( item );
    }

    LogPrintNoArg( LOG_NOTICE, "Ending MySQL thread" );
    return(NULL);
}


void db_setup(void)
{
    MysqlData_t    *item;
    my_bool         my_true;
    unsigned long   serverVers;

    if( !mySQL_db ) {
        mySQL_db = strdup(DEF_MYSQL_DB);
    }

    if( !mySQL_user ) {
        mySQL_user = strdup(DEF_MYSQL_USER);
    }

    if( !mySQL_passwd ) {
        mySQL_passwd = strdup(DEF_MYSQL_PASSWD);
    }

    if( !mySQL_host ) {
        mySQL_host = strdup(DEF_MYSQL_HOST);
    }

    item = (MysqlData_t *)malloc(sizeof(MysqlData_t));
    if( !item ) {
        LogPrintNoArg( LOG_CRIT, "Unable to create a MySQL structure!!");
        exit(1);
    }

    sql = ProtectedDataCreate();
    if( !sql ) {
        LogPrintNoArg( LOG_CRIT, "Unable to create a MySQL protected "
                                 "structure!!");
        exit(1);
    }

    sql->data = (void *)item;

    item->buflen = MAX_STRING_LENGTH;

    if( !(item->sql = mysql_init(NULL)) ) {
        LogPrintNoArg( LOG_CRIT, "Unable to initialize a MySQL structure!!");
        exit(1);
    }

    LogPrint( LOG_CRIT, "Using database %s at %s:%d", mySQL_db, mySQL_host, 
              mySQL_port);

    if( !mysql_real_connect(item->sql, mySQL_host, mySQL_user, mySQL_passwd, 
                            mySQL_db, mySQL_port, NULL, 0) ) {
        LogPrint(LOG_CRIT, "Unable to connect to the database - %s",
                           mysql_error(item->sql) );
        exit(1);
    }

#ifdef MYSQL_OPT_RECONNECT
    /* Only defined in MySQL 5.0.13 and above, before that, it was always on */
    my_true = TRUE;
    mysql_options( item->sql, MYSQL_OPT_RECONNECT, &my_true );
#else
    (void)my_true;
#endif

    LogPrint( LOG_CRIT, "MySQL client version %d.%d.%d", 
                        MYSQL_VERSION_ID / 10000,
                        (MYSQL_VERSION_ID / 100 ) % 100,
                        MYSQL_VERSION_ID % 100 );
    serverVers = mysql_get_server_version( item->sql );
    LogPrint( LOG_CRIT, "MySQL server version %d.%d.%d", 
                        serverVers / 10000, (serverVers / 100) % 100,
                        serverVers % 100 );

    QueryQ = QueueCreate( 1024 );

}

void db_thread_init( void )
{
    mysql_thread_init();
}

char *db_quote(char *string)
{
    int             len,
                    i,
                    j,
                    count;
    char           *retString;

    len = strlen(string);

    for(i = 0, count = 0; i < len; i++) {
        if( string[i] == '\'' || string[i] == '\"' ) {
            count++;
        }
    }

    if( !count ) {
        return( strdup(string) );
    }

    retString = (char *)malloc(len + count + 1);
    for(i = 0, j = 0; i < len; i++, j++) {
        if( string[i] == '\'' || string[i] == '\"' ) {
            retString[j++] = '\\';
        }
        retString[j] = string[i];
    }
    retString[j] = '\0';

    return( retString );
}


MYSQL_RES *db_query( const char *query, MYSQL_BIND *args, int arg_count )
{
    MYSQL_RES      *res;
    MysqlData_t    *item;
    char           *insert;
    int             buflen;
    int             len;
    int             count;
    static char     buf[128];
    char           *string;
    char           *sqlbuf;

    ProtectedDataLock( sql );

    item = (MysqlData_t *)sql->data;
    sqlbuf = item->sqlbuf;
    sqlbuf[0] = '\0';
    buflen = item->buflen - 1;
    count = 0;

    do {
        insert = strchr( query, '?' );
        if( !insert || !args ) {
            strncat( sqlbuf, query, buflen );
            continue;
        }

        if( !args ) {
            LogPrintNoArg( LOG_CRIT, "SQL malformed query!!" );
            return( NULL );
        }

        count++;
        len = insert - query;
        if( buflen < len ) {
            /* Oh oh! */
            LogPrintNoArg( LOG_CRIT, "SQL buffer overflow!!" );
            return( NULL );
        }
        strncat( sqlbuf, query, len );
        query = insert + 1;

        if( count > arg_count ) {
            insert = NULL;
            continue;
        }

        string = NULL;
        buf[0] = '\0';

        if( args->is_null && *args->is_null ) {
            len = 4;
            if( buflen < len ) {
                /* Oh oh! */
                LogPrintNoArg( LOG_CRIT, "SQL buffer overflow!!" );
                return( NULL );
            }
            strncat( sqlbuf, "NULL", 4 );
            buflen -= len;
        } else {
            switch( args->buffer_type ) {
            case MYSQL_TYPE_SHORT:
                len = snprintf( buf, 128, "%d", *(short int *)(args->buffer) );
                break;
            case MYSQL_TYPE_LONG:
                len = snprintf( buf, 128, "%d", *(int *)(args->buffer) );
                break;
            case MYSQL_TYPE_LONGLONG:
                len = snprintf( buf, 128, "%lld", 
                                *(long long int *)(args->buffer) );
                break;
            case MYSQL_TYPE_STRING:
            case MYSQL_TYPE_VAR_STRING:
            case MYSQL_TYPE_TINY_BLOB:
            case MYSQL_TYPE_BLOB:
                if( args->buffer ) {
                    string = db_quote((char *)args->buffer);
                    len = strlen(string) + 2;
                } else {
                    len = 2;
                }
                break;
            default:
                continue;
                break;
            }

            if( buflen < len ) {
                /* Oh oh! */
                LogPrintNoArg( LOG_CRIT, "SQL buffer overflow!!" );
                return( NULL );
            }

            if( string || buf[0] == '\0' ) {
                strcat( sqlbuf, "'" );
                if( string ) {
                    strncat( sqlbuf, string, len );
                    free( string );
                }
                strcat( sqlbuf, "'" );
            } else {
                strncat( sqlbuf, buf, len );
            }
        }
        args++;
    } while( insert );

    mysql_query(item->sql, sqlbuf);
    res = mysql_store_result(item->sql);

    ProtectedDataUnlock( sql );

    return( res );
}

/*
 * Query Queuing
 */
void db_queue_query( int queryId, QueryTable_t *queryTable,
                     MYSQL_BIND *queryData, int queryDataCount,
                     QueryResFunc_t queryCallback, void *queryCallbackArg,
                     pthread_mutex_t *queryMutex )
{
    static unsigned int sequence = 0;
    QueryItem_t        *item;

    item = (QueryItem_t *)malloc(sizeof(QueryItem_t));
    memset( item, 0, sizeof(QueryItem_t) );

    item->queryId = queryId;
    item->queryTable = queryTable;
    item->queryCallback = queryCallback;
    item->queryCallbackArg = queryCallbackArg;
    item->queryData = queryData;
    item->queryDataCount = queryDataCount;
    item->queryMutex = queryMutex;
    item->querySequence = (++sequence);

    if( queryMutex ) {
        pthread_mutex_lock( queryMutex );
    }

    QueueEnqueueItem( QueryQ, item );

    if( queryMutex ) {
        pthread_mutex_lock( queryMutex );
    }
}

/*
 * Bind values to MYSQL_BIND structures
 */

void bind_numeric( MYSQL_BIND *data, long long int value, 
                   enum enum_field_types type )
{
    void           *ptr;
    int             len;
    static my_bool  isnull = TRUE;

    if( !data ) {
        return;
    }

    data->buffer_type = type;

    switch( type ) {
    case MYSQL_TYPE_TINY:
        len = 1;
        break;
    case MYSQL_TYPE_SHORT:
        len = 2;
        break;
    case MYSQL_TYPE_LONG:
        len = 4;
        break;
    case MYSQL_TYPE_LONGLONG:
        len = 8;
        break;
    default:
        data->is_null = &isnull;
        return;
    }

    ptr = malloc(len);
    if( !ptr ) {
        data->is_null = &isnull;
        return;
    }
    data->buffer = ptr;

    switch( type ) {
    case MYSQL_TYPE_TINY:
        *(char *)ptr = (char)value;
        break;
    case MYSQL_TYPE_SHORT:
        *(short int *)ptr = (short int)value;
        break;
    case MYSQL_TYPE_LONG:
        *(int *)ptr = (int)value;
        break;
    case MYSQL_TYPE_LONGLONG:
        *(long long int *)ptr = (long long int)value;
        break;
    default:
        break;
    }
}

void bind_string( MYSQL_BIND *data, char *value, enum enum_field_types type )
{
    char           *ptr;
    static my_bool  isnull = TRUE;

    if( !data ) {
        return;
    }

    data->buffer_type = type;

    switch( type ) {
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_BLOB:
        break;
    default:
        data->is_null = &isnull;
        return;
    }

    if( !value ) {
        data->is_null = &isnull;
        return;
    }

    ptr = strdup( value );
    if( !ptr ) {
        data->is_null = &isnull;
        return;
    }
    data->buffer = ptr;
    data->buffer_length = strlen( value );
}

void bind_null_blob( MYSQL_BIND *data, void *value )
{
    static my_bool  isnull = TRUE;

    if( !data ) {
        return;
    }

    data->buffer = value;
    data->buffer_type = MYSQL_TYPE_BLOB;
    data->is_null = &isnull;
}



/*
 * Helper functions to duplicate what's in newer versions of libmysqlclient
 */
#if ( MYSQL_VERSION_ID < 40100 )
unsigned long mysql_get_server_version(MYSQL *mysql)
{
    char           *orig;
    char           *verstring;
    char           *dot;
    unsigned long   version;

    verstring = strdup( mysql_get_server_info(mysql) );
    orig = verstring;

    dot = strchr( verstring, '.' );
    *dot = '\0';
    version = atol( verstring ) * 10000;
    verstring = dot + 1;

    dot = strchr( verstring, '.' );
    *dot = '\0';
    version += atol( verstring ) * 100;
    verstring = dot + 1;

    dot = strchr( verstring, '-' );
    if( dot ) {
        *dot = '\0';
    }
    version += atol( verstring );

    free( orig );

    return( version );
}
#endif

#if ( MYSQL_VERSION_ID < 40000 )
my_bool mysql_thread_init(void)
{
    return( 0 );
}
#endif



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
