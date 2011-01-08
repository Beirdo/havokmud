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
#include <string.h>
#include <errno.h>
#define _LogLevelNames_
#include "logging.h"
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include "memory.h"

/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */
#define LOGLINE_MAX 1024
#define TIMESTAMP_MAX 128
#define DEBUG_FILE "/var/log/havokmud/debug.log"

/* INTERNAL FUNCTION PROTOTYPES */
void LogItemOutput( void *vitem );
void LogWrite( LogFileChain_t *logfile, char *text, int length );
void LogOutputAdd( int fd, LogFileType_t type, void *identifier );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

LogLevel_t LogLevel = LOG_UNKNOWN;  /**< The log level mask to apply, messages
                                         must be at at least this priority to
                                         be output */

LinkedList_t       *LogList;
extern pthread_t    mainThreadId;
char               *dbg, *dfg;
extern bool         Debug;
extern bool         Daemon;


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

    item = CREATER(LoggingItem_t);
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
    item->message   = CREATERN(char, LOGLINE_MAX+1);
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
    struct timespec     delay;

    /* 100ms delay */
    delay.tv_sec = 0;
    delay.tv_nsec = 100000L;

    pthread_mutex_lock( startupMutex );
    LogList = LinkedListCreate( NULL );
#if 1
    LogStdoutAdd();
#else
    LogNcursesAdd();
#endif

#ifndef __CYGWIN__
    LogSyslogAdd( LOG_LOCAL7 );
#endif

    if( Debug ) {
        LogFileAdd( DEBUG_FILE );
    }
    pthread_mutex_unlock( startupMutex );

    thread_colors( mainThreadId, &dbg, &dfg );

    while( 1 ) {
        item = (LoggingItem_t *)QueueDequeueItem( LoggingQ, -1 );
        if( !item ) {
            nanosleep( &delay, NULL );
            continue;
        }

        LogItemOutput( (void *)item );
    }

    LogPrintNoArg(LOG_INFO, "Ending LoggingThread");
    return( NULL );
}

void logging_toggle_debug( int signum, void *info, void *secret )
{
    if( !pthread_equal( pthread_self(), mainThreadId ) ) {
        return;
    }

    if( Debug ) {
        /* We are turning OFF debug logging */
        LogPrintNoArg( LOG_CRIT, "Received SIGUSR1, disabling debug logging" );
        LogFileRemove( DEBUG_FILE );
        Debug = FALSE;
    } else {
        /* We are turning ON debug logging */
        LogPrintNoArg( LOG_CRIT, "Received SIGUSR1, enabling debug logging" );
        LogFileAdd( DEBUG_FILE );
        Debug = TRUE;
    }
}


bool LogStdoutAdd( void )
{
    /* STDOUT corresponds to file descriptor 1 */
    if( Daemon ) {
        return( FALSE );
    }

    LogOutputAdd( 1, LT_CONSOLE, NULL );
    LogPrintNoArg( LOG_INFO, "Added console logging" );
    return( TRUE );
}


bool LogSyslogAdd( int facility )
{
    openlog( "beirdobot", LOG_NDELAY | LOG_PID, facility );
    LogOutputAdd( -1, LT_SYSLOG, NULL );
    LogPrintNoArg( LOG_INFO, "Added syslog logging" );
    return( TRUE );
}


void LogOutputAdd( int fd, LogFileType_t type, void *identifier )
{
    LogFileChain_t *item;

    item = CREATE(LogFileChain_t);
    item->type    = type;
    item->aborted = FALSE;
    switch( type )
    {
        case LT_SYSLOG:
            item->fd = -1;
            break;
        case LT_FILE:
            item->fd = fd;
            item->identifier.filename = memstrlink( (char *)identifier );
            break;
        case LT_CONSOLE:
            item->fd = fd;
            break;
        case LT_NCURSES:
            item->fd = -1;
            break;
        default:
            /* UNKNOWN! */
            memfree( item );
            return;
            break;
    }

    /* Add it to the Log File List (note, the function contains the mutex
     * handling
     */
    LinkedListAdd( LogList, (LinkedListItem_t *)item, UNLOCKED, AT_TAIL );
}


bool LogOutputRemove( LogFileChain_t *logfile )
{
    if( logfile == NULL )
    {
        return( FALSE );
    }

    /* logfile will be pointing at the offending member, close then 
     * remove it.  It is assumed that the caller already has the Mutex
     * locked.
     */
    switch( logfile->type )
    {
        case LT_FILE:
        case LT_CONSOLE:
            close( logfile->fd );
            if( logfile->identifier.filename != NULL )
            {
                memfree( logfile->identifier.filename );
            }
            break;
        case LT_SYSLOG:
            /* Nothing to do */
            break;
        default:
            break;
    }

    /* Remove the log file from the linked list */
    LinkedListRemove( LogList, (LinkedListItem_t *)logfile, LOCKED );

    memfree( logfile );
    return( TRUE );
}

void LogWrite( LogFileChain_t *logfile, char *text, int length )
{
    int result;

    if( logfile->aborted == FALSE )
    {
        result = write( logfile->fd, text, length );
        if( result == -1 )
        {
            LogPrint( LOG_UNKNOWN, "Closed Log output on fd %d due to errors", 
                      logfile->fd );
            logfile->aborted = TRUE;
        }
    }
}


bool LogFileAdd( char * filename )
{
    int fd;

    if( filename == NULL )
    {
        return( FALSE );
    }

    fd = open( filename, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK,
               S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );
    if( fd == -1 )
    {
        /* Couldn't open the log file.  Gak! */
        LogPrint( LOG_CRIT, "Couldn't open %s: %s", filename, strerror(errno) );
        return( FALSE );
    }

    LogOutputAdd( fd, LT_FILE, filename );
    LogPrint( LOG_INFO, "Added log file: %s", filename );

    return( TRUE );
}

bool LogNcursesAdd( void )
{
    if( Daemon ) {
        return( FALSE );
    }

    LogOutputAdd( -1, LT_NCURSES, NULL );
    LogPrintNoArg( LOG_INFO, "Added logging to ncurses" );

    return( TRUE );
}

bool LogFileRemove( char *filename )
{
    LogFileChain_t *logfile;
    LinkedListItem_t *listItem;
    bool found;

    if( filename == NULL )
    {
        return( FALSE );
    }

    LinkedListLock( LogList );

    for( listItem = LogList->head, found = FALSE; 
         listItem != NULL;
         listItem = listItem->next )
    {
        logfile = (LogFileChain_t *)listItem;
        if( logfile->type == LT_FILE && 
            strcmp( filename, logfile->identifier.filename ) == 0 )
        {
            LogOutputRemove( logfile );
            LogPrint( LOG_INFO, "Removed log file: %s", filename );
            found = TRUE;
            /* Take an early exit from the loop */
            break;
        }
    }

    if( found == FALSE )
    {
        LogPrint( LOG_UNKNOWN, "Can't find log file: %s", filename );
    }

    LinkedListUnlock( LogList );
    return( found );
}

void LogItemOutput( void *vitem )
{
    LoggingItem_t      *item;
    struct tm           ts;
    char                line[MAX_STRING_LENGTH];
    char                usPart[9];
    char                timestamp[TIMESTAMP_MAX];
    int                 length;
    int                 msglen;
    LinkedListItem_t   *listItem, *next;
    LogFileChain_t     *logFile;
    static char        *unknown = "thread_unknown";
    char               *threadName;
    char               *bg, *fg;
    time_t              sec;

    if( !vitem ) {
        return;
    }

    item = (LoggingItem_t *)vitem;

    if( item->level > LogLevel ) {
        free( item->message );
        free( item );
        return;
    }

    thread_colors( item->threadId, &bg, &fg );

    sec = (time_t)item->time_sec;
    localtime_r( (const time_t *)&sec, &ts );
    strftime( timestamp, TIMESTAMP_MAX-8, "%Y-%b-%d %H:%M:%S",
              (const struct tm *)&ts );
    snprintf( usPart, 9, ".%06d ", (int)(item->time_usec) );
    strcat( timestamp, usPart );
    length = strlen( timestamp );
    
    /* Trim the trailing \n\r from tog messages */
    for( msglen = strlen( item->message ) ; 
         msglen && (item->message[msglen-1] == '\n' || 
                    item->message[msglen-1] == '\r'); 
         item->message[msglen-1] = '\0' );

    LinkedListLock( LogList );
    
    for( listItem = LogList->head; listItem; listItem = next ) {
        logFile = (LogFileChain_t *)listItem;
        next = listItem->next;

        switch( logFile->type ) {
        case LT_SYSLOG:
            syslog( item->level, "%s", item->message );
            break;
        case LT_CONSOLE:
            sprintf( line, "%s%s%s %s%s%s\n", bg, fg, timestamp, item->message,
                     dbg, dfg );
            LogWrite( logFile, line, strlen(line) );
            break;
        case LT_FILE:
            threadName = thread_name( item->threadId );
            if( !threadName ) {
                threadName = unknown;
            }
            sprintf( line, "%s%s%s %s %s:%d (%s) - %s%s%s\n", bg, fg, 
                     timestamp, threadName, item->file, item->line, 
                     item->function, item->message, dbg, dfg );
            LogWrite( logFile, line, strlen(line) );
            break;
        case LT_NCURSES:
#if 0
            sprintf( line, "%s%s%s %s%s%s\n", bg, fg, timestamp, 
                     item->message, dbg, dfg );

            cursesLogWrite( line );
#endif
            break;
        default:
            break;
        }

        if( logFile->aborted ) {
            LogOutputRemove( logFile );
        }
    }

    LinkedListUnlock( LogList );

    free( item->message );
    free( item );
}

void LogFlushOutput( void )
{
    LoggingItem_t      *item;

    while( 1 ) {
        item = (LoggingItem_t *)QueueDequeueItem( LoggingQ, 0 );
        if( !item ) {
            return;
        }

        LogItemOutput( (void *)item );
    }
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
