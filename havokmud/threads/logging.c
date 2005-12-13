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
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "logging.h"
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */
#define LOGLINE_MAX 256
#define _LogLevelNames_

/* INTERNAL FUNCTION PROTOTYPES */

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";


void LogPrintLine( LogLevel_t level, char *file, int line, char *function,
                   char *format, ... )
{
    LoggingItem_t *item;
    struct timeval tv;
    va_list arguments;

    item = (LoggingItem_t *)malloc(sizeof(LoggingItem_t));
    if( !item ) {
        return;
    }

    item->level     = level;
    item->threadId  = pthread_self();
    item->file      = file;
    item->line      = line;
    item->function  = function;
    gettimeofday( &tv, NULL );
    item->time_sec  = tv.tv_sec;
    item->time_usec = tv.tv_usec;
    item->message   = (char *)malloc(LOGLINE_MAX+1);
    if( !item->message ) {
        free( item );
        return;
    }

    va_start(arguments, format);
    vsnprintf(item->message, LOGLINE_MAX, format, arguments);
    va_end(arguments);

    QueueEnqueueItem( LoggingQ, item );
}


void *LoggingThread( void *arg )
{
    LoggingItem_t      *item;

    while( 1 ) {
        item = (LoggingItem_t *)QueueDequeueItem( LoggingQ, -1 );

        printf( "%d.%06d %s:%d (%s) - %s\n", item->time_sec, item->time_usec,
                item->file, item->line, item->function, item->message );

        free( item->message );
        free( item );
    }

    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
