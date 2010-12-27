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
 * Copyright 2010 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Thread to handle WebService requests
 */

#include "environment.h"
#include <pthread.h>
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include "logging.h"
#include "memory.h"
#include "protos.h"
#include <signal.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "mongoose.h"

void *webServiceCallback(enum mg_event event, struct mg_connection *conn,
                         const struct mg_request_info *request_info);

/**
 * @brief Thread to handle WebService requests
 * @return never returns until shutdown
 *
 * Receives requests over a web socket and acts on them.
 */
void *WebServiceThread( void *arg )
{
    char               *port;
    char               *logdir;
    char                buf[1024];
    char              **options;
    struct mg_context  *ctx;

    pthread_mutex_lock( startupMutex );

    port = pb_get_setting( "webServicePort" );
    if( !port || !atoi(port) ) {
        LogPrintNoArg( LOG_CRIT, "No WebService Port defined.  Aborting!" );
        return;
    }

    logdir = pb_get_setting( "webServiceLogDir" );
    if( !logdir ) {
        logdir = memstrlink( "/tmp" );
    }

    LogPrint( LOG_DEBUG, "Web Service listening on port %s", port );
    LogPrint( LOG_DEBUG, "Web Logs in %s", logdir );

    options = CREATEN(char *, 7);
    options[0] = memstrlink("ports");
    options[1] = memstrdup(port);
    options[2] = memstrlink("access_log");
    sprintf( buf, "%s/access.log", logdir );
    options[3] = memstrdup(buf);
    options[4] = memstrlink("error_log");
    sprintf( buf, "%s/error.log", logdir );
    options[5] = memstrdup(buf);
    options[6] = NULL;

    memfree( port );
    memfree( logdir );

    /*
     * Initialize mongoose context.
     * Set WWW root to current directory.
     * Start listening on port specified.
     */
    ctx = mg_start(webServiceCallback, NULL, (const char **)options);

    while( !GlobalAbort ) {
        sleep(1);
    }
    
    mg_stop(ctx);

    LogPrintNoArg(LOG_INFO, "Ending WebServiceThread");
    return( NULL );
}

void *webServiceCallback(enum mg_event event, struct mg_connection *conn,
                         const struct mg_request_info *request_info)
{
    return NULL;
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
