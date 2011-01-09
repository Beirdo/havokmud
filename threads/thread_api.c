/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008, 2011 Gavin Hurlbut
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
* Copyright 2008, 2011 Gavin Hurlbut
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
#include <string.h>

#define DEFAULT_STACK_SIZE  (1*1024*1024)

BalancedBTree_t    *ThreadTree = NULL;
extern pthread_t    mainThreadId;
extern pthread_t    cursesOutThreadId;
static int threadCount = 0;

pthread_mutex_t    *startupMutex = NULL;

#if 1
/** todo: add the curses interface **/
pthread_t    cursesOutThreadId;
#endif

typedef struct {
    pthread_t          *threadId;
    char               *name;
    ThreadCallback_t   *callbacks;
    int                 background;
    int                 foreground;
} Thread_t;

char backgroundColors[][8] = { BK_BLACK, BK_LT_GRAY, BK_RED, BK_GREEN, 
                              BK_BROWN, BK_BLUE, BK_MAGENTA, BK_CYAN };
char foregroundColors[][8] = { FG_BLACK, FG_LT_GRAY, FG_RED, FG_GREEN, 
                              FG_BROWN, FG_BLUE, FG_MAGENTA, FG_CYAN, 
                              FG_DK_GRAY, FG_LT_RED, FG_LT_GREEN, FG_YELLOW,
                              FG_LT_BLUE, FG_LT_MAGENTA, FG_LT_CYAN, FG_WHITE };
typedef struct {
    int     background;
    int     foreground;
} Color_t;

Color_t badColors[] = {
    { 1, 0 }, { 1, 10 }, { 1, 11 }
};
int badColorCount = NELEMENTS(badColors);

Color_t lastColor = { 0, 0 };

void ThreadRecurseKill( BalancedBTreeItem_t *node, int signum );
void ThreadGetNextColor( int *bg, int *fg );

void ThreadGetNextColor( int *bg, int *fg )
{
    bool    badColor;
    int     i;

    if( !fg || !bg ) {
        return;
    }

    *fg = lastColor.foreground;
    *bg = lastColor.background;
    badColor = TRUE;

    while( badColor ) {
        (*fg)++;
        if( *fg == 16 ) {
            (*bg)++;
            *fg = 0;
        }
        
        badColor = ((*bg == *fg) || (*fg == 15 && *bg == 1) || 
                    ((*fg <= 14 && *fg >= 9) && (*bg == *fg - 7)));

        for( i = 0; i < badColorCount && !badColor; i++ ) {
            if( badColors[i].background == *bg &&
                badColors[i].foreground == *fg ) {
                badColor = TRUE;
            }
        }
    }
    lastColor.foreground = *fg;
    lastColor.background = *bg;
}

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
    pthread_attr_t  attr;

    if( !startupMutex ) {
        startupMutex = CREATE(pthread_mutex_t);
        thread_mutex_init( startupMutex );
    }

    pthread_mutex_lock( startupMutex );

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, DEFAULT_STACK_SIZE);
    pthread_create( pthreadId, &attr, routine, arg );
    pthread_attr_destroy(&attr);
    thread_register( pthreadId, name, callbacks );
    pthread_mutex_unlock( startupMutex );
}

void thread_register( pthread_t *pthreadId, char *name, 
                      ThreadCallback_t *callbacks )
{
    BalancedBTreeItem_t    *item;
    Thread_t               *thread;
    Color_t                 color;

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
    if( callbacks ) {
        thread->callbacks  = CREATE(ThreadCallback_t);
        memcpy(thread->callbacks, callbacks, sizeof(ThreadCallback_t));
    }
    threadCount++;
    ThreadGetNextColor( &thread->background, &thread->foreground );

    item = CREATE(BalancedBTreeItem_t);
    item->item = (void *)thread;
    item->key  = (void *)thread->threadId;

    BalancedBTreeAdd( ThreadTree, item, UNLOCKED, TRUE );
    LogPrint( LOG_INFO, "Added thread as \"%s%s%s%s%s\" (%d/%d)", 
              backgroundColors[thread->background], 
              foregroundColors[thread->foreground], name,
              backgroundColors[0], foregroundColors[1],
              thread->background, thread->foreground );
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
    static char            *empty = "";

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

    if( thread->callbacks ) {
        memfree( thread->callbacks );
    }
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
        if( parg ) {
            if( thread->callbacks ) {
                *parg = thread->callbacks->sigusr2Arg;
            } else {
                *parg = NULL;
            }
        }

        if( !thread->callbacks || !thread->callbacks->sigusr2Func ) {
            return( do_backtrace );
        }

        return( thread->callbacks->sigusr2Func );
        break;
    case SIGHUP:
        if( parg ) {
            if( thread->callbacks ) {
                *parg = thread->callbacks->sighupArg;
            } else {
                *parg = NULL;
            }
        }

        if( !thread->callbacks || !thread->callbacks->sighupFunc ) {
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
