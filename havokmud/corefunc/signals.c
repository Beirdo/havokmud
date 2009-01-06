/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2009 Gavin Hurlbut
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
 * Copyright 2009 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Handles UNIX signals
 */


#include "config.h"
#include "environment.h"
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <execinfo.h>
#include <ucontext.h>
#include <stdlib.h>

#include "oldexterns.h"
#include "interthread.h"
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";

void do_symbol( void *ptr );

extern pthread_t mainThreadId;

void signal_interrupt( int signum, void *sinfo, void *secret )
{
    extern const char *const    sys_siglist[];
    struct sigaction            sa;
    siginfo_t                  *info;

    info = (siginfo_t *)sinfo;

    if( pthread_equal( pthread_self(), mainThreadId ) ) {
        sa.sa_handler = SIG_DFL;
        sigemptyset( &sa.sa_mask );
        sa.sa_flags = SA_RESTART;
        sigaction( SIGINT, &sa, NULL );

        LogPrint( LOG_CRIT, "Received signal: %s", sys_siglist[signum] );
        exit( 0 );
    }
}

#ifdef REG_EIP
 #define OLD_IP REG_EIP
#else
 #ifdef REG_RIP
  #define OLD_IP REG_RIP
 #endif
#endif

void signal_everyone( int signum, void *sinfo, void *secret )
{
    extern const char *const    sys_siglist[];
    SigFunc_t                   sigFunc;
    pthread_t                   myThreadId;
    ucontext_t                 *uc;
    void                       *arg;
    siginfo_t                  *info;

    info = (siginfo_t *)sinfo;

    uc = (ucontext_t *)secret;
    myThreadId = pthread_self();

#if 0
    if( pthread_equal( myThreadId, mainThreadId ) ) {
        LogPrint( LOG_CRIT, "Received signal: %s", sys_siglist[signum] );
    }
#endif

    sigFunc = ThreadGetHandler( myThreadId, signum, &arg );
    if( sigFunc ) {
        if( signum == SIGUSR2 ) {
#ifdef OLD_IP
            arg = (void *)uc->uc_mcontext.gregs[OLD_IP];
#else
            arg = NULL;
#endif
        }
        sigFunc( signum, arg );
    }

    if( pthread_equal( myThreadId, mainThreadId ) ) {
        ThreadAllKill( signum );
    }
}

void signal_death( int signum, void *sinfo, void *secret )
{
    extern const char *const    sys_siglist[];
    ucontext_t                 *uc;
    struct sigaction            sa;
    siginfo_t                  *info;

    info = (siginfo_t *)sinfo;

    uc = (ucontext_t *)secret;

    /* Make it so another bad signal will just KILL it */
    sa.sa_handler = SIG_DFL;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;
    sigaction( SIGSEGV, &sa, NULL );
    sigaction( SIGILL, &sa, NULL );
    sigaction( SIGFPE, &sa, NULL );

    LogPrint( LOG_CRIT, "Received signal: %s", sys_siglist[signum] );
#ifdef OLD_IP
    LogPrint( LOG_CRIT, "Faulty Address: %p, from %p", info->si_addr,
                        uc->uc_mcontext.gregs[OLD_IP] );
#else
    LogPrint( LOG_CRIT, "Faulty Address %p, no discernable context",
                        info->si_addr );
#endif

#ifdef OLD_IP
    do_backtrace( signum, (void *)uc->uc_mcontext.gregs[OLD_IP] );
#else
    do_backtrace( signum, NULL );
#endif

    /* Spew all remaining messages */
    LogFlushOutput();

#if 0
    cursesAtExit();
#endif

    /* Kill this thing HARD! */
    abort();
}

void do_symbol( void *ptr )
{
    void               *array[1];
    char              **strings;

    array[0] = ptr;
    strings = backtrace_symbols( array, 1 );

    LogPrint( LOG_DEBUG, "%s", strings[0] );

    free( strings );
}

void do_backtrace( int signum, void *ip )
{
    void               *array[100];
    size_t              size;
    char              **strings;
    size_t              i;
    char               *name;
    static char        *unknown = "unknown";

    if( ip ) {
        /* This was a signal, so print the thread name */
        name = thread_name( pthread_self() );
        if( !name ) {
            name = unknown;
        }
        LogPrint( LOG_DEBUG, "Thread: %s backtrace", name );
    } else {
        name = NULL;
    }

    size = backtrace( array, 100 );

#if 0
    /* replace the sigaction/pthread_kill with the caller's address */
    if( ip ) {
        array[1] = ip;
    }
#endif

    strings = backtrace_symbols( array, size );

    LogPrint( LOG_DEBUG, "%s%sObtained %zd stack frames.", 
                         (name ? name : ""), (name ? ": " : ""), size );

    for( i = 0; i < size; i++ ) {
        LogPrint( LOG_DEBUG, "%s%s%s", (name ? name : ""), (name? ": " : ""),
                             strings[i] );
    }

    free( strings );
}


/**
 * @brief Masks a set of signals
 * @param set the set of signals to mask
 * @param count count of signals to mask
 *
 * Masks a set of signals
 */
void sigsetmaskset( int *set, int count )
{
    static sigset_t sigset;
    int i;

    sigemptyset(&sigset);
    if( set ) {
        for( i = 0; i < count; i++ ) {
            sigaddset(&sigset, set[i]);
        }
    }

    sigprocmask( SIG_SETMASK, &sigset, NULL );
}

void mainSighup( int signum, void *arg )
{
#if 0
    /*
     * Need to rescan the plugins
     */
    LogPrintNoArg( LOG_INFO, "Reloading plugins..." );
    plugins_sighup();

    /*
     * Reload server & channel info -- NOTE: this happens before the bot
     * threads get signalled
     */
    LogPrintNoArg( LOG_INFO, "Reloading servers & channels..." );
    BalancedBTreeLock( ServerTree );
    serverUnvisit( ServerTree->root );

    db_load_servers();
    db_load_channels();

    while( serverFlushUnvisited( ServerTree->root ) ) {
        /*
         * If an unvisited entry is found, we need to loop as removing it can
         * mess up the recursion
         */
    }

    BalancedBTreeAdd( ServerTree, NULL, LOCKED, TRUE );
    BalancedBTreeUnlock( ServerTree );
#endif
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
