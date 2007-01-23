/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
 * Copyright 2007 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Thread to handle the game play for each user.  Note, there will be
 *        two instances running, one for mortals, one for immortals.
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @brief Thread to handle the game play for each user
 * @param arg PlayingThreadArgs_t object which indicates which thread 
 *            (MortalPlayingThread or ImmortalPlayingThread) and an input queue
 * @return never returns until shutdown
 *
 * Receives lines of input from the Input thread and feeds them to the
 * CommandInterpreter which will do the commands.
 */
void *PlayingThread( void *arg )
{
    InputStateItem_t   *item;
    PlayerStruct_t     *player;
    char               *line;
    InputStateType_t    type;
    PlayingThreadArgs_t    *me;

    if( !arg ) {
        return( NULL );
    }

    me = (PlayingThreadArgs_t *)arg;

    LogPrint( LOG_NOTICE, "Started %s", me->name );

    while( 1 ) {
        item = (InputStateItem_t *)QueueDequeueItem( me->inputQ, -1 );
        player = item->player;
        line = item->line;
        type = item->type;

        free( item );

        switch( type ) {
        case INPUT_INITIAL:
            /*
             * Player just logged in, set them up
             */
            JustLoggedIn(player);
            break;
        case INPUT_AVAIL:
            /*
             * User input, feed it to the command parser
             */
            CommandParser(player, line);
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
