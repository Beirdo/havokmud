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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "interthread.h"
#include "queue.h"
#include "linked_list.h"

static char ident[] _UNUSED_ =
    "$Id$";

static LinkedList_t *PlayerList;

void *InputThread( void *arg )
{
    ConnInputItem_t    *connItem;
    ConnInputType_t     type;
    PlayerStruct_t     *player;
    InputStateItem_t   *stateItem;
    int                 totalLen, len;
    char               *buf, *bufend, *bufloc;
    int                 i;

    PlayerList = LinkedListCreate();

    while( 1 ) {
        connItem = (ConnInputItem_t *)QueueDequeueItem( ConnectInputQ, -1 );
        player = connItem->player;
        type = connItem->type;
        free(connItem);

        stateItem = (InputStateItem_t *)malloc(sizeof(InputStateItem_t));
        if( !stateItem ) {
            /*
             * out of memory, doh!
             */
            continue;
        }

        switch( type ) {
        case CONN_NEW_CONNECT:
            /*
             * Start the nanny
             */
            player->state = STATE_INITIAL;

            stateItem = (InputStateItem_t *)malloc(sizeof(InputStateItem_t));
            if( !stateItem ) {
                /*
                 * out of memory, doh!
                 */
                close(player->connection->fd);
                continue;
            }

            stateItem->player = player;
            stateItem->type   = INPUT_INITIAL;
            player->handlingQ = InputLoginQ;
            QueueEnqueueItem( player->handlingQ, stateItem );

            LinkedListAdd( PlayerList, (LinkedListItem_t *)player, UNLOCKED,
                           AT_TAIL );
            break;
        case CONN_INPUT_AVAIL:
            /*
             * Find out how many bytes are available and lock the buffer if any
             * data available
             */
            totalLen = BufferAvailRead( player->in_buffer, TRUE );
            if( !totalLen ) {
                /*
                 * Hmmm, no input to process...  move along!
                 */
                continue;
            }

            len = totalLen;

            if( player->in_remain ) {
                /* We had a remainder to prepend */
                len += player->in_remain_len;
            }

            buf = (char *)malloc(len);
            bufend = buf;
            
            if( player->in_remain ) {
                memcpy( buf, player->in_remain, player->in_remain_len );
                bufend += player->in_remain_len;
                free( player->in_remain );
                player->in_remain = NULL;
                player->in_remain_len = 0;
            }

            memcpy( bufend, BufferGetRead( player->in_buffer ), totalLen );
            BufferReadBytes( player->in_buffer, totalLen );
            BufferUnlock( player->in_buffer );

            /*
             * OK, we got the total buffer, now we need to split it into lines.
             */
            for( bufend = buf + len; buf != bufend ; buf += i+1 ) {
                while( *buf == '\n' || *buf == '\r' || *buf < 32 ) {
                    if( buf != bufend ) {
                        buf++;
                    }
                }

                bufloc = strstr( buf, "\n" );
                if( bufloc ) {
                    i = bufloc - buf;
                    *bufloc = '\0';

                    stateItem = (InputStateItem_t *)
                                   malloc(sizeof(InputStateItem_t));
                    if( !stateItem ) {
                        /*
                         * out of memory, doh!
                         */
                        continue;
                    }

                    stateItem->player = player;
                    stateItem->type   = INPUT_AVAIL;
                    stateItem->line   = strdup( buf );
                    QueueEnqueueItem( player->handlingQ, stateItem );
                } else {
                    i = bufend - buf;
                    player->in_remain     = (char *)malloc(i);
                    if (player->in_remain) {
                        player->in_remain_len = i;
                        memcpy( player->in_remain, buf, i );
                    }
                }
            }

            buf = bufend - len;
            free( buf );
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
