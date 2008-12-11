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
 * @brief Thread to handle network connections.
 */

#include "environment.h"
#include <pthread.h>
#include "externs.h"
#include "interthread.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"
#include "logging.h"
#include "memory.h"

static char ident[] _UNUSED_ =
    "$Id$";

static int listenFd = -1;   /**< file descriptor to listen() on */
static int maxFd = -1; /**< the maximum file descriptor in use, for select() */


#define MAX_BUFSIZE 8192


LinkedList_t *ConnectionList = NULL;  /**< List of all current connections */
static int recalcMaxFd = FALSE;  /**< Set when maxFd needs recalculating */
static fd_set readFds,   /**< set of file descriptors that can be read */
              writeFds,  /**< set of file descriptors that can be written */
              exceptFds; /**< set of file descriptors that have exceptions */
static fd_set saveReadFds,   /**< set of all file descriptors for read */
              saveWriteFds,  /**< set of all file descriptors for write */
              saveExceptFds; /**< set of all file descriptors for exceptions */

static ConnectionItem_t *connRemove(ConnectionItem_t *item);
static void connAddFd( int fd, fd_set *fds );

/**
 * @brief Handles all network connections
 * @param arg a pointer to a structure containing port number and timeout
 * @return never returns until shutdown
 *
 * @todo be sure this handles linkdead without removing all player structures
 *       so the player can log back in and be where they were
 *
 * Brings up a TCP listener on the MUD's assigned port and accepts connections.
 * Also reads all data from connected clients and hands the data off to the 
 * Input thread.  When there is output to be sent, this thread handles writing
 * to the sockets after the first write has completed.  This thread also
 * handles the disconnection of clients gracefully.
 */
void *ConnectionThread( void *arg )
{
    connectThreadArgs_t *argStruct;
    int portNum;
    struct sockaddr_in sa;
    int count;
    int fdCount;
    int newFd;
    socklen_t salen;
    struct timeval timeout;
    ConnectionItem_t *item;
    PlayerStruct_t *player;
    ConnInputItem_t *connItem;
    ConnDnsItem_t *dnsItem;
    uint32 i;

    argStruct = (connectThreadArgs_t *)arg;
    portNum = argStruct->port;

    LogPrint( LOG_NOTICE, "Listening on port %d", portNum );

    /*
     * Start listening
     */

    listenFd = socket( PF_INET, SOCK_STREAM, 0 );
    if( listenFd < 0 ) {
        perror("Opening listener socket");
        exit(1);
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(portNum);

    if (bind(listenFd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("Binding listener socket");
        close(listenFd);
        exit(1);
    }

    if (listen(listenFd, 10)) {
        perror("Listening to socket");
        close(listenFd);
        exit(1);
    }

    FD_ZERO(&saveReadFds);
    FD_ZERO(&saveWriteFds);
    FD_ZERO(&saveExceptFds);

    connAddFd(listenFd, &saveReadFds);

    ConnectionList = LinkedListCreate(NULL);
    LogPrint( LOG_DEBUG, "ConnectionList = %p", ConnectionList );

    while( 1 ) {
        /*
         * Select on connected and listener
         */
        readFds   = saveReadFds;
        writeFds  = saveWriteFds;
        exceptFds = saveExceptFds;
        timeout.tv_sec  = argStruct->timeout_sec;
        timeout.tv_usec = argStruct->timeout_usec;
        
        fdCount = select(maxFd+1, &readFds, &writeFds, &exceptFds, &timeout);

        recalcMaxFd = FALSE;

        /*
         * Open a connection for listener
         */
        if( FD_ISSET(listenFd, &readFds) ) {
            newFd = accept(listenFd, (struct sockaddr *)&sa, &salen);

            connAddFd(newFd, &saveReadFds);
            connAddFd(newFd, &saveExceptFds);

            item = CREATE(ConnectionItem_t);
            if( !item ) {
                /*
                 * No memory!
                 */
                LogPrintNoArg( LOG_EMERG, "Out of memory!" );
                close(newFd);
            } else {
                memset(item, 0, sizeof(ConnectionItem_t));
                item->fd = newFd;
                item->buffer = BufferCreate(MAX_BUFSIZE);

                item->hostName = ProtectedDataCreate();
                ProtectedDataLock( item->hostName );
                item->hostName->data = CREATEN(char, 16);
                i = sa.sin_addr.s_addr;
                sprintf((char *)item->hostName->data, "%d.%d.%d.%d", 
                        (i & 0x000000FF), (i & 0x0000FF00) >> 8, 
                        (i & 0x00FF0000) >> 16, (i & 0xFF000000) >> 24);
                ProtectedDataUnlock( item->hostName );

                if (!IS_SET(SystemFlags, SYS_SKIPDNS)) {
                    dnsItem = CREATE(ConnDnsItem_t);
                    if( dnsItem ) {
                        dnsItem->connection = item;
                        dnsItem->ipAddr = sa.sin_addr.s_addr;
                        QueueEnqueueItem(ConnectDnsQ, dnsItem);
                    }
                }

                player = CREATE(PlayerStruct_t);
                if( !player ) {
                    /*
                     * No memory!
                     */
                    LogPrintNoArg( LOG_EMERG, "Out of memory!" );
                    BufferDestroy(item->buffer);
                    close(newFd);
                    memfree(item);
                } else {
                    memset(player, 0, sizeof(PlayerStruct_t));
                    item->player = player;
                    player->connection = item;
                    player->in_buffer = item->buffer;

                    LinkedListAdd( ConnectionList, (LinkedListItem_t *)item, 
                                   UNLOCKED, AT_TAIL );
                    /*
                     * Pass the info on to the other threads...
                     */
                    LogPrint( LOG_INFO, "New connection: %p", player );
                    connItem = CREATE(ConnInputItem_t);
                    if( connItem ) {
                        connItem->type = CONN_NEW_CONNECT;
                        connItem->player = player;

                        QueueEnqueueItem(ConnectInputQ, (QueueItem_t)connItem);
                    }
                }
            }

            fdCount--;
        }

        if( fdCount ) {
            LinkedListLock( ConnectionList );

            for( item = (ConnectionItem_t *)(ConnectionList->head); 
                   item && fdCount; 
                   item = (item ? (ConnectionItem_t *)item->link.next
                                : (ConnectionItem_t *)ConnectionList->head) ) {
                if( FD_ISSET( item->fd, &exceptFds ) ) {
                    /*
                     * This connection's borked, close it, remove it, move on
                     */
                    if( FD_ISSET( item->fd, &readFds ) ) {
                        fdCount--;
                    }

                    if( FD_ISSET( item->fd, &writeFds ) ) {
                        fdCount--;
                    }

                    BufferLock( item->buffer );
                    item = connRemove(item);
                    fdCount--;
                    continue;
                }

                if( item && FD_ISSET( item->fd, &readFds ) ) {
                    /*
                     * This connection has data ready
                     */
                    count = BufferAvailWrite( item->buffer, TRUE );
                    if( !count ) {
                        /*
                         * No buffer space, the buffer's unlocked, move on
                         */
                        LogPrint( LOG_INFO, "No buffer space: %p", item );
                        continue;
                    }

                    /*
                     * The buffer's locked
                     */
                    count = read( item->fd, BufferGetWrite( item->buffer ),
                                  count );
                    if( !count ) {
                        /*
                         * We hit EOF, close and remove
                         */
                        if( FD_ISSET( item->fd, &writeFds ) ) {
                            fdCount--;
                        }

                        item = connRemove(item);
                        fdCount--;
                        continue;
                    }

                    BufferWroteBytes( item->buffer, count );
                    BufferUnlock( item->buffer );

                    /*
                     * Tell the input thread
                     */
                    connItem = CREATE(ConnInputItem_t);
                    if( connItem ) {
#ifdef DEBUG_INPUT
                        LogPrint( LOG_INFO, "New data: %p", item->player );
#endif
                        connItem->type = CONN_INPUT_AVAIL;
                        connItem->player = item->player;

                        QueueEnqueueItem(ConnectInputQ, (QueueItem_t)connItem);
                    }
                }

                if( item && FD_ISSET( item->fd, &writeFds ) ) {
                    /*
                     * We have space to output, so write if we have anything
                     * to write
                     */
#ifdef DEBUG_OUTPUT
                    LogPrint( LOG_INFO, "Output sent to: %p", item );
#endif
                    if( item->outBufDesc ) {
                        write( item->fd, item->outBufDesc->buf, 
                               item->outBufDesc->len );
                        memfree( item->outBufDesc->buf );
                        memfree( item->outBufDesc );
                        item->outBufDesc = NULL;
                    }
                    /*
                     * Kick the next output
                     */
                    connKickOutput( item );
                    fdCount--;
                }
            }
            LinkedListUnlock( ConnectionList );
        }

        if( recalcMaxFd ) {
            LinkedListLock( ConnectionList );
            maxFd = listenFd;

            for( item = (ConnectionItem_t *)(ConnectionList->head); item; 
                 item = (ConnectionItem_t *)item->link.next ) {
                if( item->fd > maxFd ) {
                    maxFd = item->fd;
                }
            }
            LinkedListUnlock( ConnectionList );
        }
    }

    return( NULL );
}

/**
 * @brief Adds a new file descriptor to a set
 * @param fd the file descriptor to add
 * @param fds a pointer to the file descriptor set to be added to
 * @return no value
 *
 * Adds a new file descriptor to a set and recalculates maxFd
 */
static void connAddFd( int fd, fd_set *fds )
{
    FD_SET(fd, fds);
    if( fd > maxFd ) {
        maxFd = fd;
    }
}

/**
 * @brief Removes a file descriptor from a set
 * @param fd the file descriptor to remove
 * @param fds a pointer to the file descriptor set to be removed from
 * @return no value
 *
 * Removes a file descriptor from a set.  maxFd is not recalculated until the
 * descriptor is closed completely.
 */
static void connDelFd( int fd, fd_set *fds )
{
    FD_CLR(fd, fds);
}

/**
 * @brief Removes a connection from the connection list
 * @param item the connection item to remove from the list
 * @return the previous connection item in the list so the ->next will work in
 *         a loop.  NULL if this was the first item on the list.
 *
 * Removes a connection from the connection linked list and re-indexes the
 * list.  The Input thread is then notified to remove the player, and all
 * input for that player is flushed.  The maxFd is marked for recalculation 
 * after the next select is finished, and the socket is closed.
 */
static ConnectionItem_t *connRemove(ConnectionItem_t *item)
{
    ConnectionItem_t *prev;
    PlayerStruct_t *player;
    ConnInputItem_t *connItem;

    close(item->fd);
    connDelFd( item->fd, &saveReadFds );
    connDelFd( item->fd, &saveWriteFds );
    connDelFd( item->fd, &saveExceptFds );

    player = item->player;

    if( item->link.prev ) {
        item->link.prev->next = item->link.next;
    } else {
        ConnectionList->head = item->link.next;
    }

    if( item->link.next ) {
        item->link.next->prev = item->link.prev;
    } else {
        ConnectionList->tail = item->link.prev;
    }

    prev = (ConnectionItem_t *)(item->link.prev);

    ProtectedDataLock(item->hostName);
    if( item->hostName->data ) {
        memfree( item->hostName->data );
    }
    ProtectedDataDestroy(item->hostName);

    /*
     * must be locked before entering...
     */
    BufferDestroy( item->buffer );
    memfree( item );

    connItem = CREATE(ConnInputItem_t);
    if( connItem ) {
        connItem->type = CONN_DELETE_CONNECT;
        connItem->player = player;

        /*
         * Ignore any remaining input
         */
        player->flush = TRUE;

        QueueEnqueueItem(ConnectInputQ, (QueueItem_t)connItem);
    }

    recalcMaxFd = TRUE;

    return( prev );
}

/**
 * @brief closes a connection
 * @param connItem oconnection item to close
 * @return no value
 *
 * Allows another thread to close a connection.  This is useful when the game
 * wishes to disconnect someone.
 */
void connClose( ConnectionItem_t *connItem )
{
    LinkedListLock( ConnectionList );
    connRemove( connItem );
    LinkedListUnlock( ConnectionList );
}

/**
 * @brief starts transmitting data to a socket
 * @param connItem connection to send data to
 * @return no value
 *
 * When output is being sent to a socket, it must be dequeued from the player's
 * output queue, then sent out the socket.  The actual write is done by the
 * Connection thread, when the select wakes up for writing to that socket.
 * Once the socket is ready to be written to, the Connection thread will write
 * a buffer to it, then kick the output again to get the next buffer to write
 * once the socket is again ready.  This allows the socket's built-in flow
 * control to work without much overhead.  Once all the buffers are done being
 * transmitted, the socket will be removed from the writing file descriptor set
 * and the select will no longer wake up for it.
 */
void connKickOutput( ConnectionItem_t *connItem )
{
    PlayerStruct_t *player;
    OutputBuffer_t *bufDesc;

    if( !connItem ) {
        return;
    }

    player = connItem->player;
#ifdef DEBUG_OUTPUT
    LogPrint( LOG_INFO, "Kicking output Q: %p", player );
#endif

    if( connItem->outBufDesc ) {
        connAddFd( connItem->fd, &saveWriteFds );
        return;
    }

    bufDesc = (OutputBuffer_t *)QueueDequeueItem( player->outputQ, 0 );
    if( !bufDesc ) {
        if( !connItem->outBufDesc ) {
            connDelFd( connItem->fd, &saveWriteFds );
        }
        connItem->outBufDesc = NULL;
    } else {
        connAddFd( connItem->fd, &saveWriteFds );
        connItem->outBufDesc = bufDesc;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
