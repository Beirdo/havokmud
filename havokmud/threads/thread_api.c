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
*
*/

#include "environment.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "interthread.h"
#include "balanced_btree.h"
#include "logging.h"
#include "memory.h"
#include "ansi_vt100.h"


static char ident[] _UNUSED_= 
    "$Id$";

BalancedBTree_t    *ThreadTree = NULL;
extern pthread_t    mainThreadId;
extern pthread_t    cursesOutThreadId;
static int threadCount = 0;

#if 1
/** todo: add the curses interface **/
pthread_t    cursesOutThreadId;
#endif

typedef struct {
    pthread_t          *threadId;
    char               *name;
    ThreadCallback_t   *callbacks;
    int			background;
    int                 foreground;
} Thread_t;

char backgroundColors[][8] = { BK_BLACK, BK_LT_GRAY, BK_RED, BK_GREEN, BK_BROWN,
                              BK_BLUE, BK_MAGENTA, BK_CYAN };
char foregroundColors[][8] = { FG_BLACK, FG_LT_GRAY, FG_RED, FG_GREEN, FG_BROWN,
                              FG_BLUE, FG_MAGENTA, FG_CYAN, 
                              FG_DK_GRAY, FG_LT_RED, FG_LT_GREEN, FG_YELLOW,
                              FG_LT_BLUE, FG_LT_MAGENTA, FG_LT_CYAN, FG_WHITE };

void ThreadRecurseKill( BalancedBTreeItem_t *node, int signum );

int thread_mutex_init( pthread_mutex_t *mutex )
{
    pthread_mutexattr_t attr;
    int                 status;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);

    status = pthread_mutex_init( mutex, &attr);

    pthread_mutexattr_destroy(&attr);

    return( status );
}

void thread_create( pthread_t *pthreadId, void * (*routine)(void *), 
                    void *arg, char *name, ThreadCallback_t *callbacks )
{
    pthread_create( pthreadId, NULL, routine, arg );
    thread_register( pthreadId, name, callbacks );
    usleep(100);
}

void thread_register( pthread_t *pthreadId, char *name, 
                      ThreadCallback_t *callbacks )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;

    if( !ThreadTree ) {
        ThreadTree = BalancedBTreeCreate( NULL, BTREE_KEY_PTHREAD );
        if( !ThreadTree ) {
            fprintf( stderr, "Couldn't create thread tree!\n" );
            _exit( 1 );
        }
    }

    thread = CREATE(Thread_t);
    thread->threadId   = pthreadId;
    thread->name       = name;
    thread->callbacks  = callbacks;
    threadCount++;
    thread->background = (threadCount - 1) / 15;
    thread->foreground = (threadCount - 1) % 15;
    if( thread->foreground >= thread->background) {
	thread->foreground++;
    }

    item = CREATE(BalancedBTreeItem_t);
    item->item = (void *)thread;
    item->key  = (void *)thread->threadId;

    BalancedBTreeAdd( ThreadTree, item, UNLOCKED, TRUE );
    LogPrint( LOG_INFO, "Added Thread %ld as \"%s%s%s%s%s\" (%d/%d)", 
              *pthreadId, 
              backgroundColors[thread->background], 
              foregroundColors[thread->foreground], name,
              backgroundColors[0], foregroundColors[1],
              thread->background,
              thread->foreground );
}

char *thread_name( pthread_t pthreadId )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;

    item = BalancedBTreeFind( ThreadTree, (void *)&pthreadId, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    thread = (Thread_t *)item->item;

    return( thread->name );
}

void thread_colors( pthread_t pthreadId, char **bg, char **fg )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;
    static char		   *empty = "";

    item = BalancedBTreeFind( ThreadTree, (void *)&pthreadId, UNLOCKED, FALSE );
    if( !item ) {
	*bg = empty;
        *fg = empty;
        return;
    }

    thread = (Thread_t *)item->item;

    *bg = backgroundColors[thread->background];
    *fg = foregroundColors[thread->foreground];
}

void thread_deregister( pthread_t pthreadId )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;

    item = BalancedBTreeFind( ThreadTree, (void *)&pthreadId, UNLOCKED, FALSE );
    if( !item ) {
        return;
    }

    BalancedBTreeRemove( ThreadTree, item, UNLOCKED, TRUE );
    thread = (Thread_t *)item->item;

    LogPrint( LOG_INFO, "Removed Thread %ld as \"%s\"", pthreadId, 
                        (char *)thread->name );

    memfree( thread );
    memfree( item );

    if( !pthread_equal( pthread_self(), pthreadId ) ) {
        pthread_join( pthreadId, NULL );
    }
}


void ThreadAllKill( int signum )
{
    BalancedBTreeLock( ThreadTree );

    ThreadRecurseKill( ThreadTree->root, signum );

    BalancedBTreeUnlock( ThreadTree );
}

void ThreadRecurseKill( BalancedBTreeItem_t *node, int signum )
{
    Thread_t       *thread;

    if( !node ) {
        return;
    }

    ThreadRecurseKill( node->left, signum );

    thread = (Thread_t *)node->item;

    if( !pthread_equal( *thread->threadId, mainThreadId ) ) {
        switch( signum ) {
        case SIGUSR2:
        case SIGHUP:
        case SIGWINCH:
#if 0
            LogPrint( LOG_DEBUG, "Killing thread %s with signal %d",
                                 thread->name, signum );
#endif
            pthread_kill( *thread->threadId, signum );
            break;
        default:
            break;
        }
    }

    ThreadRecurseKill( node->right, signum );
}

SigFunc_t ThreadGetHandler( pthread_t threadId, int signum, void **parg )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;

    item = BalancedBTreeFind( ThreadTree, (void *)&threadId, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    thread = (Thread_t *)item->item;

    if( parg ) {
        *parg = NULL;
    }

    switch( signum ) {
    case SIGUSR2:
#if 0
        return( do_backtrace );
#endif
        break;
    case SIGHUP:
        if( parg ) {
            if( thread->callbacks ) {
                *parg = thread->callbacks->sighupArg;
            } else {
                *parg = NULL;
            }
        }

        if( !thread->callbacks ) {
            return( NULL );
        }

        return( thread->callbacks->sighupFunc );
        break;
    case SIGWINCH:
#if 0
        if( !Daemon && pthread_equal( threadId, cursesOutThreadId ) ) {
            return( cursesSigwinch );
        }
#endif
        break;
    default:
        break;
    }

    return( NULL );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

