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
 */


/**
 * @file
 * @brief Logs messages to the console and/or logfiles
 * @todo add a system logging that will log not only to the logfile(s) but also
 *       in game to any immorts logged in
 */

/* INCLUDE FILES */
#include "environment.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "logging.h"
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

LogLevel_t LogLevel = LOG_UNKNOWN;  /**< The log level mask to apply, messages
                                         must be at at least this priority to
                                         be output */

/**
 * @brief Formats and enqueues a log message for the Logging thread
 * @param level the logging level to log at
 * @param file the sourcefile the message is from
 * @param line the line in the sourcefile the message is from
 * @param function the function the message is from
 * @param format the printf-style format string
 *
 * Creates a log message (up to LOGLINE_MAX) in length using vsnprintf, and
 * enqueues it with a timestamp, thread and sourcefile info.  These messages 
 * go onto the LoggingQ which is then read by the Logging thread.  When this
 * function returns, all strings passed in can be reused or freed.
 */
void LogPrintLine( LogLevel_t level, char *file, int line, const char *function,
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
    item->function  = (char *)function;
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


/**
 * @brief Prints the log messages to the console (and logfile)
 * @param arg unused
 * @return never returns until shutdown
 * @todo Add support for a logfile as well as console output.
 *
 * Dequeues log messages from the LoggingQ and outputs them to the console.
 * If the message's log level is lower (higher numerically) than the current
 * system log level, the message will be dumped and not displayed.
 * In the future, it will also log to a logfile.
 */
void *LoggingThread( void *arg )
{
    LoggingItem_t      *item;

    while( 1 ) {
        item = (LoggingItem_t *)QueueDequeueItem( LoggingQ, -1 );

        if( item->level <= LogLevel ) {
            printf( "%d.%06d %s:%d (%s) - %s\n", item->time_sec, 
                    item->time_usec, item->file, item->line, item->function, 
                    item->message );
        }

        free( item->message );
        free( item );
    }

    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
