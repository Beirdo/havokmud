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
 * @brief Thread to handle the login process for each user
 */

#include "environment.h"
#include <pthread.h>
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @brief Thread to handle the login process for each user
 * @param arg unused
 * @return never returns until shutdown
 *
 * Receives lines of input from the Input thread and feeds them to the
 * LoginStateMachine which will guide the user through login and character
 * creation.
 */
void *LoginThread( void *arg )
{
    InputStateItem_t   *item;
    PlayerStruct_t     *player;
    char               *line;
    InputStateType_t    type;

    LogPrintNoArg( LOG_NOTICE, "Started LoginThread" );

    while( 1 ) {
        item = (InputStateItem_t *)QueueDequeueItem( InputLoginQ, -1 );
        player = item->player;
        line = item->line;
        type = item->type;

        free( item );

        switch( type ) {
        case INPUT_INITIAL:
            /*
             * Brand new connection, display the banner
             */
            LoginSendBanner(player);
            break;
        case INPUT_AVAIL:
            /*
             * User input, feed it to the state machine
             */
            LoginStateMachine(player, line);
            break;
        default:
            break;
        }
    }

    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
