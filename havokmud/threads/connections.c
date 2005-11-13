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

static char ident[] _UNUSED_ =
    "$Id$";

static int listenFd = -1;
static int maxFd = -1;


#define MAX_BUFSIZE 8192


LinkedList_t *ConnectionList = NULL;

static ConnectionItem_t *connRemove(ConnectionItem_t *item);
static void connAddFd( int fd, fd_set *fds );

void *ConnectionThread( void *arg )
{
    connectThreadArgs_t *argStruct;
    int portNum;
    struct sockaddr_in sa;
    fd_set readFds, writeFds, exceptFds;
    fd_set saveReadFds, saveWriteFds, saveExceptFds;
    int count;
    int fdCount;
    int newFd;
    socklen_t salen;
    struct timeval timeout;
    ConnectionItem_t *item;
    PlayerStruct_t *player;
    ConnInputItem_t *connItem;

    argStruct = (connectThreadArgs_t *)arg;
    portNum = argStruct->port;

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

    ConnectionList = LinkedListCreate();

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

        /*
         * Open a connection for listener
         */
        if( FD_ISSET(listenFd, &readFds) ) {
            newFd = accept(listenFd, (struct sockaddr *)&sa, &salen);

            connAddFd(newFd, &saveReadFds);
            connAddFd(newFd, &saveExceptFds);

            item = (ConnectionItem_t *)malloc(sizeof(ConnectionItem_t));
            if( !item ) {
                /*
                 * No memory!
                 */
                close(newFd);
            } else {
                memset(item, 0, sizeof(ConnectionItem_t));
                item->fd = newFd;
                item->buffer = BufferCreate(MAX_BUFSIZE);

                player = (PlayerStruct_t *)malloc(sizeof(PlayerStruct_t));
                if( !player ) {
                    /*
                     * No memory!
                     */
                    BufferDestroy(item->buffer);
                    close(newFd);
                    free(item);
                } else {
                    item->player = player;
                    player->connection = item;
                    player->in_buffer = item->buffer;

                    LinkedListAdd( ConnectionList, (LinkedListItem_t *)item, 
                                   UNLOCKED, AT_TAIL );
                    /*
                     * Pass the info on to the other threads...
                     */
                    connItem = (ConnInputItem_t *)
                                  malloc(sizeof(ConnInputItem_t));
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
                    connItem = (ConnInputItem_t *)
                                  malloc(sizeof(ConnInputItem_t));
                    if( connItem ) {
                        connItem->type = CONN_INPUT_AVAIL;
                        connItem->player = item->player;

                        QueueEnqueueItem(ConnectInputQ, (QueueItem_t)connItem);
                    }
                }

                if( item && FD_ISSET( item->fd, &writeFds ) ) {
                    /*
                     * This connection just finished a write?  Do I give a
                     * crap?  TODO
                     */
                    fdCount--;
                }
            }
            LinkedListUnlock( ConnectionList );
        }
    }

    return( NULL );
}

static void connAddFd( int fd, fd_set *fds )
{
    FD_SET(fd, fds);
    if( fd > maxFd ) {
        maxFd = fd;
    }
}

static ConnectionItem_t *connRemove(ConnectionItem_t *item)
{
    ConnectionItem_t *prev;

    close(item->fd);

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

    /*
     * must be locked before entering...
     */
    BufferDestroy( item->buffer );
    free( item );

    return( prev );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
