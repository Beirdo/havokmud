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
 * Handles UNIX signals
 */


#include "config.h"
#include "environment.h"
#include "platform.h"
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "protos.h"
#include "externs.h"
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";

void            shutdown_request(int signal);
void            logsig(int signal);
void            hupsig(int signal);


void signal_setup(void)
{
    /*
     * struct itimerval itime; struct timeval interval; 
     */
    signal(SIGUSR2, shutdown_request);

    /*
     * just to be on the safe side: 
     */
    signal(SIGHUP, hupsig);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, hupsig);
    signal(SIGALRM, logsig);
    signal(SIGTERM, hupsig);
}


void shutdown_request(int signal)
{
    extern int      mudshutdown;

    LogPrintNoArg(LOG_CRIT, "Received USR2 - shutdown request");
    mudshutdown = 1;
}

/*
 * kick out players etc 
 */
void hupsig(int signal)
{
    int             i;

    LogPrintNoArg(LOG_CRIT, "Received SIGHUP, SIGINT, or SIGTERM. Shutting "
                            "down");

    raw_force_all("return");
    raw_force_all("save");

    for (i = 0; i < 30; i++) {
        SaveTheWorld();
    }

    mudshutdown = reboot_now = 1;
}

void logsig(int signal)
{
    LogPrintNoArg(LOG_CRIT, "Signal received. Ignoring.");
}

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


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
