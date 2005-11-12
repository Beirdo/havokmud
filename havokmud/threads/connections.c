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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

static char ident[] _UNUSED_ =
    "$Id$";

static int listenFd = -1;
static int maxFd = -1;

static void connAddFd( int fd, fd_set *fds );

typedef struct {
    int port;
    int timeoutSec;
    int timeoutUsec;
} connectThreadArgs_t;

void *ConnectionThread( void *arg )
{
    connectThreadArgs_t *argStruct;
    int portNum;
    struct sockaddr_in sa;
    fd_set readFds, writeFds, exceptFds;
    fd_set saveReadFds, saveWriteFds, saveExceptFds;
    struct timeval saveTimeout, timeout, lastTime, now;
    int count;
    int ticked;
    int newFd;
    socklen_t salen;

    argStruct = (connectThreadArgs_t *)arg;
    portNum = argStruct->port;
    saveTimeout.tv_sec = argStruct->timeoutSec;
    saveTimeout.tv_usec = argStruct->timeoutUsec;

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
    memcpy(&timeout, &saveTimeout, sizeof(timeout));
    gettimeofday(&lastTime, NULL);

    while( 1 ) {
        /*
         * Select on connected and listener
         */
        readFds = saveReadFds;
        writeFds = saveWriteFds;
        exceptFds = saveExceptFds;
        ticked = 0;
        
        count = select(maxFd+1, &readFds, &writeFds, &exceptFds, &timeout);
        gettimeofday(&now, NULL);

        /*
         * Adjust the tick timer
         */
        timeout.tv_sec  -= (now.tv_sec - lastTime.tv_sec);
        timeout.tv_usec -= (now.tv_usec - lastTime.tv_usec);
        while(timeout.tv_usec >= 1000000) {
            timeout.tv_usec -= 1000000;
            timeout.tv_sec++;
        }
        while(timeout.tv_usec < 0) {
            timeout.tv_usec += 1000000;
            timeout.tv_sec--;
        }

        memcpy(&lastTime, &now, sizeof(lastTime));
        if( (timeout.tv_sec < 0) || 
            (timeout.tv_sec == 0 && timeout.tv_usec == 0)) {
            ticked = 1;
            memcpy(&timeout, &saveTimeout, sizeof(timeout));
        }

        /*
         * Open a connection for listener
         */
        if( FD_ISSET(listenFd, &readFds) ) {
            newFd = accept(listenFd, (struct sockaddr *)&sa, &salen);

        }

        /*
         * Close closed and funky connections
         */

        /*
         * If tick timer clicked over, send the prompt, etc
         */
        if( ticked ) {
        }

        /*
         * read all available inputs
         */

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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
