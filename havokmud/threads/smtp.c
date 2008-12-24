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
 * $Id: playing.c 1737 2008-12-12 20:11:52Z gjhurlbu $
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

static char ident[] _UNUSED_ =
    "$Id: playing.c 1737 2008-12-12 20:11:52Z gjhurlbu $";


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
    char               *hostname;
    char               *server;
    char               *fromAddr;

    smtp_version(buffer, 256, 0);
    LogPrint( LOG_INFO, "SMTP Version %s", buffer );

    session = smtp_create_session();

    hostname = db_get_setting( "smtpHostname" );
    if( !hostname ) {
        LogPrint( LOG_CRIT, "No SMTP Hostname defined! );
        return( NULL );
    }
    smtp_set_hostname( session, hostname );

    server = db_get_setting( "smtpServer" );
    if( !server ) {
        LogPrint( LOG_CRIT, "No SMTP Server defined! );
        return( NULL );
    }
    smtp_set_server( session, server );

    fromAddr = db_get_setting( "smtpFrom" );
    if( !fromAddr ) {
        LogPrint( LOG_CRIT, "No SMTP From Address defined! );
        return( NULL );
    }
    smtp_set_reverse_path( session, fromAddr );


    while( 1 ) {
        item = (MailItem_t *)QueueDequeueItem( mailQ, -1 );
        player = item->player;

        message = smtp_add_message( session );
        recipient = smtp_add_recipient( message, 
                                        (const char *)player->account->email );

/** TODO: add the rest of the code */
        memfree( item );
    }

    smtp_destroy_session(session);

    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
