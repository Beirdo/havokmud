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
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "protected_data.h"

static char ident[] _UNUSED_ =
    "$Id$";


void *DnsThread( void *arg )
{
    ConnDnsItem_t      *item;
    ConnectionItem_t   *connect;
    struct sockaddr_in  sa;
    struct hostent     *from;

    while( 1 ) {
        item = QueueDequeueItem( ConnectDnsQ, -1 );
        if( !item ) {
            continue;
        }

        sa.sin_addr.s_addr = item->ipAddr;

        from = gethostbyaddr((char *)&sa.sin_addr, sizeof(sa.sin_addr), 
                             AF_INET);
        if( from ) {
            connect = item->connection;
            ProtectedDataLock(connect->hostName);
            if( connect->hostName->data ) {
                free( connect->hostName->data );
            }
            connect->hostName->data = strdup(from->h_name);
            ProtectedDataUnlock(connect->hostName);
        }

        free( item );
    }

    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
