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
 * @brief Thread to handle SMTP connections to send mail to users.
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
#include <libesmtp.h>
#include <signal.h>
#include <stdarg.h>
#include <string.h>

static char ident[] _UNUSED_ =
    "$Id$";

void event_cb( smtp_session_t session, int event_no, void *arg, ... );
const char *message_cb( void **buf, int *len, void *arg );
void print_recipient_status( smtp_recipient_t recipient, const char *mailbox,
                             void *arg );

/**
 * @brief Thread to handle SMTP for sending mail to users
 * @return never returns until shutdown
 *
 * Receives message structures from other threads and uses libESMTP to send
 * them.
 */
void *SmtpThread( void *arg )
{
    char                buffer[256];
    MailItem_t         *item;
    PlayerStruct_t     *player;
    smtp_session_t      session;
    smtp_message_t      message;
    smtp_recipient_t    recipient;
    smtp_status_t      *status;
    char               *hostname;
    char               *server;
    char               *fromAddr;
    struct sigaction    sa;

    pthread_mutex_lock( startupMutex );
    pthread_mutex_unlock( startupMutex );

    smtp_version(buffer, 256, 0);
    LogPrint( LOG_INFO, "libESMTP Version %s", buffer );

    session = smtp_create_session();

    hostname = db_get_setting( "smtpHostname" );
    if( !hostname ) {
        LogPrintNoArg( LOG_CRIT, "No SMTP Hostname defined!" );
        return( NULL );
    }
    smtp_set_hostname( session, hostname );

    server = db_get_setting( "smtpServer" );
    if( !server ) {
        LogPrintNoArg( LOG_CRIT, "No SMTP Server defined!" );
        return( NULL );
    }
    smtp_set_server( session, server );

    fromAddr = db_get_setting( "smtpFrom" );
    if( !fromAddr ) {
        LogPrintNoArg( LOG_CRIT, "No SMTP From Address defined!" );
        return( NULL );
    }

    /** Todo: move this earlier?
     * Ignore SIGPIPE as the remote server could disconnect at any time
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction( SIGPIPE, &sa, NULL);

    smtp_set_eventcb( session, event_cb, NULL );

    while( 1 ) {
        item = (MailItem_t *)QueueDequeueItem( MailQ, -1 );
        player = item->player;

        message = smtp_add_message( session );
        smtp_set_reverse_path( message, fromAddr );
        recipient = smtp_add_recipient( message, 
                                        (const char *)player->account->email );
        smtp_set_header( message, "To", NULL, NULL );
        smtp_set_header( message, "Subject", item->subject );
        smtp_set_header_option( message, "Subject", Hdr_OVERRIDE, 1 );

        smtp_set_messagecb( message, message_cb, item );
        smtp_dsn_set_notify( recipient, 
                             Notify_SUCCESS | Notify_FAILURE | Notify_DELAY );

        if( !smtp_start_session(session) ) {
            char buf[128];
            LogPrint( LOG_INFO, "SMTP Server problem: %s",
                      smtp_strerror(smtp_errno(), buf, 128) );
        } else {
            status = (smtp_status_t *)smtp_message_transfer_status(message);
            LogPrint( LOG_INFO, "SMTP: %d %s", status->code,
                      (status->text ? status->text : "") );
            smtp_enumerate_recipients(message, print_recipient_status, NULL);
        }

/** TODO: add the rest of the code */
        memfree( item->body );
        memfree( item->subject );
        memfree( item );
    }

    smtp_destroy_session(session);

    return( NULL );
}

void event_cb( smtp_session_t session, int event_no, void *arg, ... )
{
    va_list     alist;

    va_start(alist, arg);
    switch( event_no ) {
    case SMTP_EV_CONNECT:
    case SMTP_EV_MAILSTATUS:
    case SMTP_EV_MESSAGEDATA:
    case SMTP_EV_MESSAGESENT:
    case SMTP_EV_DISCONNECT:
        break;

    case SMTP_EV_WEAK_CIPHER:
    case SMTP_EV_STARTTLS_OK:
    case SMTP_EV_INVALID_PEER_CERTIFICATE:
    case SMTP_EV_NO_PEER_CERTIFICATE:
    case SMTP_EV_WRONG_PEER_CERTIFICATE:
    case SMTP_EV_NO_CLIENT_CERTIFICATE:
        break;

    default:
        break;
    }

    LogPrint( LOG_INFO, "SMTP event %d", event_no );

    va_end(alist);
}

const char *message_cb( void **buf, int *len, void *arg )
{
    MailItem_t *item;
    int         bodylen;

    *buf = NULL;
    item = (MailItem_t *)arg;

    if( !len ) {
        /* This is a buffer rewind */
        item->bodyind = 0;
        return( NULL );
    }

    bodylen = strlen( item->body );
    if( item->bodyind >= bodylen ) {
        return( NULL );
    }

    item->bodyind = bodylen;
    *len = bodylen;
    return( (const char *)item->body );
}

void print_recipient_status( smtp_recipient_t recipient, const char *mailbox,
                             void *arg )
{
    const smtp_status_t *status;
    status = smtp_recipient_status(recipient);
    LogPrint( LOG_INFO, "SMTP: %s: %d %s", mailbox, status->code, 
              status->text );
}

void send_email( PlayerStruct_t *player, char *subject, char *body )
{
    MailItem_t    *item;

    if( !player || !subject || !body ) {
        return;
    }

    item = CREATE(MailItem_t);
    item->player  = player;
    item->subject = memstrlink( subject );
    item->body    = memstrlink( body );

    QueueEnqueueItem( MailQ, (QueueItem_t *)item );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
