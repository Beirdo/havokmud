/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2009 Gavin Hurlbut
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
 * Copyright 2009 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Starts up the threads
 */

#include "environment.h"
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include "logging.h"
#include "memory.h"
#include "protos.h"
#include "version.h"

static char ident[] _UNUSED_ =
    "$Id$";


typedef void (*sigAction_t)(int, siginfo_t *, void *);

QueueObject_t *ConnectInputQ;   /**< between Connection and Input threads */
QueueObject_t *ConnectDnsQ;     /**< between Connection and DNS threads */
QueueObject_t *InputLoginQ;     /**< between Input and Login threads */
QueueObject_t *InputEditorQ;    /**< between Input and Editor threads */
QueueObject_t *InputPlayerQ;    /**< between Input and Player threads */
QueueObject_t *InputImmortQ;    /**< between Input and Immortal threads */
QueueObject_t *LoggingQ;        /**< feeds the Logging thread */
QueueObject_t *MailQ;           /**< outbound emails */
QueueObject_t *QueryQ;          /**< MySQL query queue */
QueueObject_t *ProtobufQ;       /**< protobuf queue */

pthread_t mainThreadId;
static pthread_t connectionThreadId;
static pthread_t inputThreadId;
static pthread_t loginThreadId;
static pthread_t editorThreadId;
static pthread_t dnsThreadId;
static pthread_t loggingThreadId;
static pthread_t mortalPlayingThreadId;
static pthread_t immortPlayingThreadId;
static pthread_t mysqlThreadId;
static pthread_t smtpThreadId;
static pthread_t memoryThreadId;
static pthread_t protobufThreadId;
static pthread_t webServiceThreadId;

static connectThreadArgs_t connectThreadArgs;
static PlayingThreadArgs_t mortalPlayingArgs = { "MortalPlayingThread", NULL };
static PlayingThreadArgs_t immortPlayingArgs = { "ImmortalPlayingThread", NULL};

char               *pthreadsVersion = NULL;


/**
 * @brief Starts all the MUD threads
 * @return nothing.  Will not return until all threads shutdown.
 *
 * Creates all the inter-thread queues and starts the threads.  Once the
 * threads are all started, waits for them all to close down.
 */

void StartThreads( void )
{
    pthread_mutex_t     spinLockMutex;
    pid_t               childPid;
    struct sigaction    sa;
    sigset_t            sigmsk;
    size_t              len;
    ThreadCallback_t    callbacks;

    GlobalAbort = FALSE;

#ifndef __CYGWIN__
    len = confstr( _CS_GNU_LIBPTHREAD_VERSION, NULL, 0 );
    if( len ) {
        pthreadsVersion = CREATEN(char, len);
        confstr( _CS_GNU_LIBPTHREAD_VERSION, pthreadsVersion, len );
    }
#else
    (void)len;
    pthreadsVersion = memstrlink( "Cygwin" );
#endif

    if( !pthreadsVersion || strstr( pthreadsVersion, "linuxthreads" ) ) {
        fprintf( stderr, "havokmud requires NPTL to operate correctly.\n\n"
                         "The signal handling in linuxthreads is just too "
                         "broken to use.\n\n" );
        exit( 1 );
    }

    /* Do we need to detach? */
    if( Daemon ) {
        childPid = fork();
        if( childPid < 0 ) {
            perror( "Couldn't detach in daemon mode" );
            _exit( 1 );
        }

        if( childPid != 0 ) {
            /* This is still the parent, report the child's pid and exit */
            printf( "[Detached as PID %d]\n", childPid );
            /* And exit the parent */
            _exit( 0 );
        }

        /* After this is in the detached child */

        /* Close stdin, stdout, stderr to release the tty */
        close(0);
        close(1);
        close(2);
    }

    LoggingQ      = QueueCreate( 1024 );
    ConnectInputQ = QueueCreate( 256 );
    ConnectDnsQ   = QueueCreate( 64 );
    InputLoginQ   = QueueCreate( 256 );
    InputEditorQ  = QueueCreate( 256 );
    InputPlayerQ  = QueueCreate( 256 );
    InputImmortQ  = QueueCreate( 256 );
    MailQ         = QueueCreate( 128 );
    QueryQ        = QueueCreate( 1024 );
    ProtobufQ     = QueueCreate( 1024 );

    mainThreadId = pthread_self();
    /* 
     * Setup the sigmasks for this thread (which is the parent to all others).
     * This will propogate to all children.
     */
    sigfillset( &sigmsk );
    sigdelset( &sigmsk, SIGUSR1 );
    sigdelset( &sigmsk, SIGUSR2 );
    sigdelset( &sigmsk, SIGHUP );
    sigdelset( &sigmsk, SIGWINCH );
    sigdelset( &sigmsk, SIGINT );
    sigdelset( &sigmsk, SIGSEGV );
    sigdelset( &sigmsk, SIGILL );
    sigdelset( &sigmsk, SIGFPE );
    pthread_sigmask( SIG_SETMASK, &sigmsk, NULL );

    memset( &callbacks, 0, sizeof(ThreadCallback_t) );
    callbacks.sighupFunc = mainSighup;
    thread_register( &mainThreadId, "MainThread", NULL );

    /* Setup signal handler for SIGUSR1 (toggles Debug) */
    sa.sa_sigaction = (sigAction_t)logging_toggle_debug;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    sigaction( SIGUSR1, &sa, NULL );

    /* Setup the exit handler */
    atexit( MainDelayExit );

    /* Setup signal handler for SIGINT (shut down cleanly) */
    sa.sa_sigaction = (sigAction_t)signal_interrupt;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    sigaction( SIGINT, &sa, NULL );
    
    /* Setup signal handlers that are to be propogated to all threads */
    sa.sa_sigaction = (sigAction_t)signal_everyone;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction( SIGUSR2, &sa, NULL );
    sigaction( SIGHUP, &sa, NULL );
    sigaction( SIGWINCH, &sa, NULL );

    /* Setup signal handlers for SEGV, ILL, FPE */
    sa.sa_sigaction = (sigAction_t)signal_death;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sigaction( SIGSEGV, &sa, NULL );
    sigaction( SIGILL, &sa, NULL );
    sigaction( SIGFPE, &sa, NULL );

    versionAdd( "pthreads", pthreadsVersion );
    versionAdd( "TERM", getenv("TERM") );

    thread_create( &loggingThreadId, LoggingThread, NULL, "LoggingThread", 
                   NULL );

#if 0
    curses_start();
    cursesMenuItemAdd( 2, MENU_SYSTEM, "About", mainAbout, NULL );
    cursesMenuItemAdd( 2, MENU_SYSTEM, "Licensing", mainLicensing, NULL );
    cursesMenuItemAdd( 2, MENU_SYSTEM, "Versions", mainVersions, NULL );
    cursesMenuItemAdd( 2, MENU_SYSTEM, "Reload All", mainReloadAll, NULL );
#endif

    LogBanner();

    db_init();
    db_mysql_init();

    memset( &callbacks, 0, sizeof(ThreadCallback_t) );
    callbacks.sigusr2Func = memoryStats;
    thread_create( &memoryThreadId, MemoryCoalesceThread, NULL, 
                   "MemoryCoalesceThread", &callbacks );
    thread_create( &dnsThreadId, DnsThread, NULL, "DnsThread", NULL );
    thread_create( &inputThreadId, InputThread, NULL, "InputThread", NULL );
    thread_create( &loginThreadId, LoginThread, NULL, "LoginThread", NULL );
    thread_create( &editorThreadId, EditorThread, NULL, "EditorThread", NULL );

    mortalPlayingArgs.inputQ = InputPlayerQ;
    thread_create( &mortalPlayingThreadId, PlayingThread, &mortalPlayingArgs,
                   "MortalPlayingThread", NULL );
    
    immortPlayingArgs.inputQ = InputImmortQ;
    thread_create( &immortPlayingThreadId, PlayingThread, &immortPlayingArgs,
                   "ImmortPlayingThread", NULL );

    thread_create( &mysqlThreadId, MysqlThread, NULL, "MySQLThread", NULL );
    thread_create( &protobufThreadId, ProtobufThread, NULL, "ProtobufThread", 
                   NULL );

    pthread_mutex_lock( startupMutex );
    pthread_mutex_unlock( startupMutex );
    db_check_schema_main();

    thread_create( &smtpThreadId, SmtpThread, NULL, "SMTPThread", NULL );

    connectThreadArgs.port = mud_port;
    connectThreadArgs.timeout_sec = 0;
    connectThreadArgs.timeout_usec = 100000;
    thread_create( &connectionThreadId, ConnectionThread, &connectThreadArgs,
                   "ConnectionThread", NULL );

    thread_create( &webServiceThreadId, WebServiceThread, NULL, 
                   "WebServiceThread", NULL );

    pthread_mutex_lock( startupMutex );
    pthread_mutex_unlock( startupMutex );

    /* Sit on this and rotate - this causes an intentional deadlock, this
     * thread should stop dead in its tracks
     */
    thread_mutex_init( &spinLockMutex );
    pthread_mutex_lock( &spinLockMutex );
    pthread_mutex_lock( &spinLockMutex );
}

/**
 * TODO: replace with the release.h from beirdobot
 */
char *svn_version( void ) {
    return( SRC_VERSION );
}

void LogBanner( void )
{
    LogPrintNoArg( LOG_CRIT, "havokmud  (c) 2010 Gavin Hurlbut" );
    LogPrint( LOG_CRIT, "%s", svn_version() );

#if 0
    cursesTextAdd( WINDOW_HEADER, ALIGN_LEFT, 1, 0, "havokmud" );
    cursesTextAdd( WINDOW_HEADER, ALIGN_LEFT, 10, 0, (char *)svn_version() );
    cursesTextAdd( WINDOW_HEADER, ALIGN_FROM_CENTER, 1, 0, 
                   "(c) 2009 Gavin Hurlbut" );
    cursesTextAdd( WINDOW_TAILER, ALIGN_RIGHT, 1, 0, "Ctrl-C to exit" );
    cursesTextAdd( WINDOW_TAILER, ALIGN_LEFT, 1, 0, 
                   "Use arrow keys for menus" );
    cursesTextAdd( WINDOW_TAILER, ALIGN_CENTER, 0, 0, 
                   "PgUp/PgDn to scroll logs" );
#endif

    versionAdd( "havokmud", (char *)svn_version() );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
