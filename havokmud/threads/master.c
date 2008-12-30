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
 * @brief Starts up the threads
 */

#include "environment.h"
#include <pthread.h>
#include <unistd.h>
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";

QueueObject_t *ConnectInputQ;   /**< between Connection and Input threads */
QueueObject_t *ConnectDnsQ;     /**< between Connection and DNS threads */
QueueObject_t *InputLoginQ;     /**< between Input and Login threads */
QueueObject_t *InputEditorQ;    /**< between Input and Editor threads */
QueueObject_t *InputPlayerQ;    /**< between Input and Player threads */
QueueObject_t *InputImmortQ;    /**< between Input and Immortal threads */
QueueObject_t *LoggingQ;        /**< feeds the Logging thread */
QueueObject_t *MailQ;           /**< outbound emails */

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

static connectThreadArgs_t connectThreadArgs;
static PlayingThreadArgs_t mortalPlayingArgs = { "MortalPlayingThread", NULL };
static PlayingThreadArgs_t immortPlayingArgs = { "ImmortalPlayingThread", NULL};


/**
 * @brief Starts all the MUD threads
 * @return nothing.  Will not return until all threads shutdown.
 *
 * Creates all the inter-thread queues and starts the threads.  Once the
 * threads are all started, waits for them all to close down.
 */

void StartThreads( void )
{
    LoggingQ      = QueueCreate( 1024 );
    ConnectInputQ = QueueCreate( 256 );
    ConnectDnsQ   = QueueCreate( 64 );
    InputLoginQ   = QueueCreate( 256 );
    InputEditorQ  = QueueCreate( 256 );
    InputPlayerQ  = QueueCreate( 256 );
    InputImmortQ  = QueueCreate( 256 );
    MailQ         = QueueCreate( 128 );

    mainThreadId = pthread_self();
    thread_register( &mainThreadId, "MainThread", NULL );

    thread_create( &loggingThreadId, LoggingThread, NULL, "LoggingThread", 
                   NULL );
    thread_create( &dnsThreadId, DnsThread, NULL, "DnsThread", NULL );

    connectThreadArgs.port = 4000;
    connectThreadArgs.timeout_sec = 0;
    connectThreadArgs.timeout_usec = 100000;
    thread_create( &connectionThreadId, ConnectionThread, &connectThreadArgs,
                   "ConnectionThread", NULL );

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

    sleep(1);
    db_check_schema_main();

    thread_create( &smtpThreadId, SmtpThread, NULL, "SMTPThread", NULL );

    sleep(1);

    pthread_join( smtpThreadId, NULL );
    pthread_join( mysqlThreadId, NULL );
    pthread_join( immortPlayingThreadId, NULL );
    pthread_join( mortalPlayingThreadId, NULL );
    pthread_join( editorThreadId, NULL );
    pthread_join( loginThreadId, NULL );
    pthread_join( inputThreadId, NULL );
    pthread_join( connectionThreadId, NULL );
    pthread_join( dnsThreadId, NULL );
    pthread_join( loggingThreadId, NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
