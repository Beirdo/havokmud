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
 * Thread to handle network connections.
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"

static char ident[] _UNUSED_ =
    "$Id$";

QueueObject_t *ConnectInputQ;
QueueObject_t *ConnectDnsQ;
QueueObject_t *InputLoginQ;
QueueObject_t *InputPlayerQ;
QueueObject_t *InputImmortQ;
QueueObject_t *LoggingQ;

static pthread_t connectionThreadId;
static pthread_t inputThreadId;
static pthread_t loginThreadId;
static pthread_t dnsThreadId;
static pthread_t loggingThreadId;

static connectThreadArgs_t connectThreadArgs;

void StartThreads( void )
{
    LoggingQ      = QueueCreate( 1024 );
    ConnectInputQ = QueueCreate( 256 );
    ConnectDnsQ   = QueueCreate( 64 );
    InputLoginQ   = QueueCreate( 256 );
    InputPlayerQ  = QueueCreate( 256 );
    InputImmortQ  = QueueCreate( 256 );

    pthread_create( &loggingThreadId, NULL, LoggingThread, NULL );
    pthread_create( &dnsThreadId, NULL, DnsThread, NULL );

    connectThreadArgs.port = 4000;
    connectThreadArgs.timeout_sec = 0;
    connectThreadArgs.timeout_usec = 100000;
    pthread_create( &connectionThreadId, NULL, ConnectionThread, 
                    &connectThreadArgs );

    pthread_create( &inputThreadId, NULL, InputThread, NULL );
    pthread_create( &loginThreadId, NULL, LoginThread, NULL );

    pthread_join( inputThreadId, NULL );
    pthread_join( connectionThreadId, NULL );
    pthread_join( dnsThreadId, NULL );
    pthread_join( loggingThreadId, NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
