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
 * @brief Thread to handle input and feed it to the consuming threads
 */

#include "environment.h"
#include <pthread.h>
#include "oldexterns.h"
#include "oldutils.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "interthread.h"
#include "queue.h"
#include "linked_list.h"
#include "logging.h"
#include "memory.h"
#include "protos.h"

static char ident[] _UNUSED_ =
    "$Id$";

static LinkedList_t *PlayerList;   /**< Linked list of all connected players */


/**
 * @brief Thread to handle the input from the sockets, feed to consumers
 * @param arg unused
 * @return never returns until shutdown
 *
 * This thread takes the raw input from the Connection thread, and creates a
 * buffer.  It then feeds the input line by line to the consuming threads which
 * include LoginThread, EditorThread, etc.  This allows for multiple lines or
 * partial lines to be recieved by the network socket.
 *
 * The consuming threads all have a queue to feed it input.  The Input thread
 * doesn't know which thread it's feeding, it just uses the handlingQ that has
 * been registered in the player's structure.  On a new connection, the 
 * handlingQ is set to feed the LoginThread.
 */
void *InputThread( void *arg )
{
    ConnInputItem_t    *connItem;
    ConnInputType_t     type;
    PlayerStruct_t     *player;
    InputStateItem_t   *stateItem;
    OutputBuffer_t     *outputItem;
    QueueObject_t      *outputQ;
    int                 totalLen, len;
    char               *buf, *bufend, *bufloc;
    int                 i;

    PlayerList = LinkedListCreate(NULL);

    while( 1 ) {
        connItem = (ConnInputItem_t *)QueueDequeueItem( ConnectInputQ, -1 );
        player = connItem->player;
        type = connItem->type;
        memfree(connItem);

        switch( type ) {
        case CONN_NEW_CONNECT:
            /*
             * Start the nanny
             */
            player->state = STATE_INITIAL;

            stateItem = CREATE(InputStateItem_t);
            if( !stateItem ) {
                /*
                 * out of memory, doh!
                 */
                close(player->connection->fd);
                continue;
            }

            LogPrint( LOG_INFO, "Initializing state for connection: %p", 
                      player );
            stateItem->player = player;
            stateItem->type   = INPUT_INITIAL;
            stateItem->line   = NULL;
            player->outputQ   = QueueCreate( 64 );
            player->handlingQ = InputLoginQ;
            QueueEnqueueItem( player->handlingQ, stateItem );

            LinkedListAdd( PlayerList, (LinkedListItem_t *)player, UNLOCKED,
                           AT_TAIL );
            break;

        case CONN_INPUT_AVAIL:
            /*
             * If we've been asked to flush for this user, flush
             */
            if( player->flush ) {
                if( player->in_remain ) {
                    free( player->in_remain );
                }
                player->in_remain = NULL;
                player->in_remain_len = 0;
                break;
            }

            /*
             * Find out how many bytes are available and lock the buffer if any
             * data available
             */
            totalLen = BufferAvailRead( player->in_buffer, TRUE );
            if( !totalLen ) {
                /*
                 * Hmmm, no input to process...  move along!
                 */
                LogPrint( LOG_INFO, "No input to process: %p", player );
                continue;
            }

            len = totalLen;

            if( player->in_remain ) {
                /* We had a remainder to prepend */
                len += player->in_remain_len;
            }


#ifdef DEBUG_INPUT
            LogPrint(LOG_INFO, "Got %d bytes", len);
#endif
            buf = CREATEN(char, len);
            bufend = buf;
            
            if( player->in_remain ) {
#ifdef DEBUG_INPUT
		LogPrint(LOG_INFO, "Added %d remaining", player->in_remain_len);
#endif
                memcpy( buf, player->in_remain, player->in_remain_len );
                bufend += player->in_remain_len;
                memfree( player->in_remain );
                player->in_remain = NULL;
                player->in_remain_len = 0;
            }

            memcpy( bufend, BufferGetRead( player->in_buffer ), totalLen );
            BufferReadBytes( player->in_buffer, totalLen );
            BufferUnlock( player->in_buffer );

            /*
             * OK, we got the total buffer, now we need to split it into lines.
             */
            for( bufend = buf + len; buf < bufend ; buf += i+1 ) {
                while( *buf && buf < bufend ) {
                    if( *buf != '\n' && *buf < 32 ) {
                        buf++;
                    } else {
                        break;
                    }
                }

                if( *buf == '\0' || buf >= bufend ) {
                    i = 0;
                    continue;
                }

                bufloc = strstr( buf, "\n" );
                if( bufloc ) {
                    i = bufloc - buf;
                    *bufloc = '\0';

                    while( *(--bufloc) == '\r' ) {
                        *bufloc = '\0';
                    }

                    stateItem = CREATE(InputStateItem_t);
                    if( !stateItem ) {
                        /*
                         * out of memory, doh!
                         */
                        continue;
                    }

#ifdef DEBUG_INPUT
                    LogPrint( LOG_INFO, "Sending input for %p", player );
#endif
                    stateItem->player = player;
                    stateItem->type   = INPUT_AVAIL;
                    stateItem->line   = memstrdup( buf );
                    QueueEnqueueItem( player->handlingQ, stateItem );
                } else {
                    i = bufend - buf;
                    player->in_remain = CREATEN(char, i);
                    if (player->in_remain) {
                        player->in_remain_len = i;
                        memcpy( player->in_remain, buf, i );
                    }
                }
            }

            buf = bufend - len;
            memfree( buf );
            break;
        case CONN_FLUSH_INPUT:
            /*
             * We hit the point we are supposed to flush to
             */
            player->flush = FALSE;
            FlushQueue( player->handlingQ, player );
            break;
        case CONN_DELETE_CONNECT:
            /*
             * This player is disappearing
             */
            LinkedListRemove( PlayerList, (LinkedListItem_t *)player, 
                              UNLOCKED );

            FlushQueue( player->handlingQ, player );

            /*
             * Remove the output queue
             */
            QueueLock( player->outputQ );
            outputQ = player->outputQ;
            player->outputQ = NULL;
            QueueUnlock( outputQ );

            while( (outputItem = 
                     (OutputBuffer_t *)QueueDequeueItem(outputQ, 0)) ) {
                if( outputItem->buf ) {
                    memfree( outputItem->buf );
                }
                memfree( outputItem );
            }
            QueueDestroy( outputQ );

            BufferDestroy( player->in_buffer );
            if( player->in_remain ) {
                memfree( player->in_remain );
            }

            if( player->charData ) {
                memfree( player->charData );
            }

            memfree( player );
            break;
        default:
            break;
        }

    }

    return( NULL );
}

/**
 * @brief Removes all items from an input queue that match a player
 * @param queue which queue to flush
 * @param player which player to remove from the queue
 *
 * This will lock the queue, and then traverse the queue, removing any item
 * that matches the desired player.  Note that removing items from the queue
 * is somewhat resource intensive as the items after it in the queue must be 
 * moved to fill the hole.  Luckily, only pointers to said items are stored
 * in the queue, so it could be a lot worse.
 */
void FlushQueue( QueueObject_t *queue, PlayerStruct_t *player )
{
    uint32 i;
    InputStateItem_t *item;

    if( !player || !queue ) {
        return;
    }

    QueueLock( queue );

    for( i = queue->head; i != queue->tail; i = ((i + 1) & queue->numMask) ) {
        item = queue->itemTable[i];
        if( !item ) {
            continue;
        }

        if( item->player == player ) {
            /*
             * Gotta remove this one!
             */
            memfree( item->line );
            i = QueueRemoveItem( queue, i, LOCKED );
            free( item );
        }
    }

    QueueUnlock( queue );
}


/**
 * @brief Searches the player list for a given player
 * @param name the player name to search for (case insensitive)
 * @param oldPlayer if the player found matches this, skip it
 * @return pointer to the matching player, or NULL if not found
 *
 * Searches the player list for a named player.  As the player could be 
 * polymorphed, we check for the "original" name additionally to the real name.
 * If it is found, and it is not the one passed in via oldPlayer, it will be
 * returned.
 *
 * This is used to detect a re-connecting player who still has the old
 * connection active.  If we find the old connection, it will be destroyed to
 * allow this new connection to take over.
 */
PlayerStruct_t *FindCharacterNamed( char *name, PlayerStruct_t *oldPlayer )
{
    LinkedListItem_t   *item;
    PlayerStruct_t     *player;

    LinkedListLock( PlayerList );

    for( item = PlayerList->head; item; item = item->next ) {
        player = (PlayerStruct_t *)item;
        if( player == oldPlayer ) {
            continue;
        }
        
        if( (player->originalData && 
             strcasecmp(GET_NAME(player->originalData), name) == 0) ||
            strcasecmp(GET_NAME(player->charData), name) == 0 ) {
            LinkedListUnlock( PlayerList );
            return( player );
        }
    }

    LinkedListUnlock( PlayerList );
    return( NULL );
}

/**
 * @brief Returns the count of players connected
 * @return count of connected players
 *
 * Returns the count of players connected and in the PlayerList
 */
int GetPlayerCount( void )
{
    return( LinkedListCount( PlayerList, UNLOCKED ) );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
