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
#include "cJSON.h"

void *webServiceCallback(enum mg_event event, struct mg_connection *conn,
                         const struct mg_request_info *request_info);
void *webServiceLogin(struct mg_connection *conn, 
                      const struct mg_request_info *request_info);

static struct mg_allocs webServiceAllocs = { memcalloc, memalloc, memfree, 
                                             memrealloc };

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
        return( NULL );
    }

    logdir = pb_get_setting( "webServiceLogDir" );
    if( !logdir ) {
        logdir = memstrlink( "/tmp" );
    }

    LogPrint( LOG_DEBUG, "Web Service listening on port %s", port );
    LogPrint( LOG_DEBUG, "Web Logs in %s", logdir );

    options = CREATEN(char *, 8);
    options[0] = memstrlink("listening_ports");
    options[1] = memstrdup(port);
    options[2] = memstrlink("access_log_file");
    sprintf( buf, "%s/access.log", logdir );
    options[3] = memstrdup(buf);
    options[4] = memstrlink("error_log_file");
    sprintf( buf, "%s/error.log", logdir );
    options[5] = memstrdup(buf);
    options[6] = memstrlink("document_root");
    options[7] = memstrdup("./havokweb/");
    options[8] = NULL;

    memfree( port );
    memfree( logdir );

    mg_set_allocs( &webServiceAllocs );

    /*
     * Initialize mongoose context.
     * Start listening on port specified.
     */
    ctx = mg_start(webServiceCallback, NULL, (const char **)options);
    if( !ctx )
    {
        LogPrintNoArg( LOG_CRIT, "Web Service couldn't start!" );
        return( NULL );
    }

    pthread_mutex_unlock( startupMutex );

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
    switch( event ) {
    case MG_NEW_REQUEST:
        LogPrint(LOG_INFO, "mongoose: %s %s", 
                           request_info->request_method, request_info->uri );
        if( !strcmp( request_info->uri, "/login" ) &&
            !strcmp( request_info->request_method, "POST" ) ) {
            return webServiceLogin(conn, request_info);
        }
        return NULL;
    case MG_HTTP_ERROR:
        LogPrint(LOG_INFO, "mongoose: %s %s, code %d", 
                           request_info->request_method, request_info->uri,
                           request_info->status_code );
        return NULL;
    case MG_EVENT_LOG:
        LogPrint(LOG_INFO, "mongoose: %s", request_info->log_message );
        return NULL;
    case MG_INIT_SSL:
        LogPrintNoArg(LOG_INFO, "mongoose: Huh?  How did we just init SSL?");
        return NULL;
    }

    return NULL;
}

#define MAX_WEB_BUFSIZE 65536
void *webServiceLogin(struct mg_connection *conn, 
                      const struct mg_request_info *request_info)
{
    char       *buffer;
    char       *query;
    int         len;
    cJSON      *req;
    cJSON      *item;
    cJSON      *resp;
    char       *email = NULL;
    char       *passwd = NULL;
    PlayerAccount_t *acct;
    bool        success;
    char       *md5;
    char        date[64];
    time_t      curtime;

    buffer = CREATEN(char, MAX_WEB_BUFSIZE);
    len = mg_read(conn, buffer, MAX_WEB_BUFSIZE);
#ifdef DEBUG_LOGIN
    LogPrint( LOG_INFO, "mongoose: login len = %d, buffer = %s", len, buffer );
#endif

    query = CREATEN(char, len+1);
    len = mg_get_var(buffer, len, "q", query, len);
#ifdef DEBUG_LOGIN
    LogPrint( LOG_INFO, "mongoose: login len = %d, query = %s", len, query );
#endif
    memfree(buffer);

    if( len == -1 ) {
        LogPrintNoArg(LOG_INFO, "mongoose: no query");
        memfree(query);
        return NULL;
    }

    req = cJSON_Parse(query);
    memfree(query);

    if( !req ) {
        LogPrintNoArg(LOG_INFO, "mongoose: no JSON payload");
        return NULL;
    }

    item = cJSON_GetObjectItem(req, "email");
    if( item ) {
        email = memstrdup( item->valuestring );
    }

    item = cJSON_GetObjectItem(req, "passwd");
    if( item ) {
        passwd = memstrdup( item->valuestring );
    }
    cJSON_Delete(req);

    resp = cJSON_CreateObject();
    acct = pb_load_account(email);
    if( !acct ) {
        LogPrint(LOG_INFO, "mongoose: no account match: %s", email);
        success = FALSE;
    } else {
        md5 = MD5Password(email, passwd);
        if( strncmp(md5, acct->pwd, 32) ) {
            LogPrint(LOG_INFO, "mongoose: passwd mismatch: %s/%s", md5, 
                               acct->pwd);
            success = FALSE;
        } else {
            success = TRUE;
            cJSON_AddItemToObject( resp, "details", 
                                   item = cJSON_CreateObject() );
            cJSON_AddNumberToObject( item, "id", acct->id );
            cJSON_AddStringToObject( item, "email", acct->email );
            if( acct->ansi ) {
                cJSON_AddTrueToObject( item, "ansi" );
            } else {
                cJSON_AddFalseToObject( item, "ansi" );
            }
            if( acct->confirmed ) {
                cJSON_AddTrueToObject( item, "confirmed" );
            } else {
                cJSON_AddFalseToObject( item, "confirmed" );
            }
        }
        memfree( md5 );
    }

    if( success ) {
        cJSON_AddTrueToObject( resp, "success" );
    } else {
        cJSON_AddFalseToObject( resp, "success" );
    }
    buffer = cJSON_Print(resp);

    if( acct ) {
        if( acct->email ) {
            memfree( acct->email );
        }
        if( acct->pwd ) {
            memfree( acct->pwd );
        }
        if( acct->confcode ) {
            memfree( acct->confcode );
        }
        memfree( acct );
    }

    curtime = time(NULL);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", 
             localtime(&curtime));

    /* Send the response */
    mg_printf( conn, 
               "HTTP/1.1 200 OK\r\n"
               "Date: %s\r\n"
               "Content-Type: %s\r\n"
               "Content-Length: %d\r\n"
               "\r\n"
               "%s",
               date, "application/json", strlen(buffer), buffer );
    memfree( buffer );

    return (void *)1;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
