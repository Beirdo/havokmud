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
 * @brief Mainline code for the MUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"
#include "version.h"
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id$";

void display_usage(char *progname);
void handleCmdLineArgs(int argc, char **argv);
void StartThreads( void );

/* 
 * max number of descriptors (connections) 
 * THIS IS SYSTEM DEPENDANT, use 64 is not sure! 
 */
#define MAX_CONNECTS    256        

#define DFLT_PORT       4000          /* default port */
#define MAX_HOSTNAME    256

#if 0
/* 
 * time delay corresponding to 4
 * passes/sec 
 */
#define OPT_USEC 250000         
#endif

int             mud_port;   /**< The TCP port the MUD will listen on */


#ifdef SITELOCK
char            hostlist[MAX_BAN_HOSTS][256];   /**< list of sites to ban */
int             numberhosts;                    /**< count of banned hosts */
#endif

bool            GlobalAbort = FALSE;
long            SystemFlags;

/*
 *********************************************************************
 *  main game loop and related stuff                                    *
 ********************************************************************* */

int __main()
{
    return (1);
}

/**
 * @brief Show command line arguments
 * @param progname the name of the program being run
 *
 * Shows the command line arguments used by the program onto stderr
 */
void display_usage(char *progname)
{
    fprintf(stderr, "Usage:\n"
                    "%s [-d libdir] [-s] [-D sqlDB] [-U sqlUser] "
                    "[-P sqlPass]\n"
                    "\t[-H sqlHost] [-p port] [-A] [-N] [-R] [-L] [-h] [-V] "
                    "[port]\n\n",
                    progname );
    fprintf(stderr, "\t-d\tDefine the library dir (default %s)\n"
                    "\t-s\tDisable special procedures\n"
                    "\t-D\tDefine the MySQL database (default %s)\n",
                    DFLT_DIR, DEF_MYSQL_DB );
    fprintf(stderr, "\t-U\tDefine the MySQL user (default %s)\n"
                    "\t-P\tDefine the MySQL password (default %s)\n"
                    "\t-H\tDefine the MySQL host (default %s)\n"
                    "\t-p\tDefine the MUD port (dsfault %d)\n",
                    DEF_MYSQL_USER, DEF_MYSQL_PASSWD, DEF_MYSQL_HOST,
                    DFLT_PORT );
    fprintf(stderr, "\t-A\tDisable ALL color\n"
                    "\t-N\tDisable DNS lookups\n"
                    "\t-R\tEnable newbie authorizing\n"
                    "\t-L\tLog all users\n"
                    "\t-h\tShow this help page\n"
                    "\t-V\tShow version and exit\n\n" );
    fprintf(stderr, "\tThe MUD port can be defined either with -p PORT or with "
                    "PORT at the\n"
                    "\tend of the command line\n\n" );
}

/**
 * @brief Parses the command line arguments
 * @param argc Count of arguments
 * @param argv Vector of the command line arguments
 *
 * Parses the command line arguments and sets up the defaults for the system
 */
void handleCmdLineArgs(int argc, char **argv)
{
    int             opt;
    extern char    *optarg;
    extern int      optind;
    char           *dir;

    mud_port = -1;
    dir = NULL;

    while( (opt = getopt(argc, argv, "ld:sD:U:P:H:p:ANRLhV")) != -1 ) {
        switch (opt) {
        case 'd':
            if( dir ) {
                free( dir );
            }
            dir = strdup(optarg);
            break;

        case 's':
            no_specials = 1;
            LogPrintNoArg(LOG_CRIT, "Suppressing assignment of special "
                                    "routines.");
            break;

        case 'D':
            /* Database */
            if( mysql_db ) {
                free( mysql_db );
            }
            mysql_db = strdup(optarg);
            break;

        case 'U':
            /* Database user */
            if( mysql_user ) {
                free( mysql_user );
            }
            mysql_user = strdup(optarg);
            break;

        case 'P':
            /* Database password */
            if( mysql_passwd ) {
                free( mysql_passwd );
            }
            mysql_passwd = strdup(optarg);
            break;

        case 'H':
            /* Database host */
            if( mysql_host ) {
                free( mysql_host );
            }
            mysql_host = strdup(optarg);
            break;

        case 'p':
            /* MUD Port */
            mud_port = atoi(optarg);
            break;

        case 'A':
            SET_BIT(SystemFlags, SYS_NOANSI);
            LogPrintNoArg(LOG_CRIT, "Disabling ALL color");
            break;

        case 'N':
            SET_BIT(SystemFlags, SYS_SKIPDNS);
            LogPrintNoArg(LOG_CRIT, "Disabling DNS");
            break;

        case 'R':
            SET_BIT(SystemFlags, SYS_REQAPPROVE);
            LogPrintNoArg(LOG_CRIT, "Newbie authorizes enabled");
            break;

        case 'L':
            SET_BIT(SystemFlags, SYS_LOGALL);
            LogPrintNoArg(LOG_CRIT, "Logging all users");
            break;

        case 'V':
            LogPrint(LOG_CRIT, "HavokMUD code version: %s", VERSION);
            exit(0);
            break;

        case ':':
        case '?':
        case 'h':
        default:
            display_usage(argv[0]);
            exit(1);
            break;
        }
    }

    if( argv[optind] ) {
        if (!isdigit((int)*argv[optind])) {
            display_usage(argv[0]);
            exit(1);
        } 

        mud_port = atoi(argv[optind]);
    }

    if( mud_port == -1 ) {
        mud_port = DFLT_PORT;
    }

    if (mud_port <= 1024) {
        printf("Illegal port #\n");
        exit(1);
    }

    if( !dir ) {
        dir = strdup(DFLT_DIR);
    }

    LogPrint(LOG_CRIT, "Using %s as data directory.", dir);

    if (chdir(dir) < 0) {
        perror("chdir");
        exit(1);
    }

    free( dir );
}

/**
 * @brief MUD mainline
 * @param argc Count of arguments
 * @param argv Vector of the command line arguments
 *
 * MUD's mainline.  Parses the command line, sets up the database, then 
 * launches the threads that run the MUD.
 */
int main(int argc, char **argv)
{
    extern int      spy_flag;
    extern long     SystemFlags;
    void            signal_setup(void);
    int             load(void);

#ifdef SITELOCK
    int             a;
#endif
#if defined(__sun__) || defined(__NetBSD__)
    struct rlimit   rl;
    int             res;
#endif

    spy_flag = TRUE;

#ifdef MALLOC_DEBUG
    malloc_debug(1);            /* some systems might not have this lib */
#endif

    SystemFlags = 0;

#ifdef LOG_ALL
    SET_BIT(SystemFlags, SYS_LOGALL);
#endif

#if defined(__sun__) || defined(__NetBSD__)
    /*
     **  this block sets the max # of connections.
     */
#if defined(__sun__)
    res = getrlimit(RLIMIT_NOFILE, &rl);
    rl.rlim_cur = MAX_CONNECTS;
    res = setrlimit(RLIMIT_NOFILE, &rl);
#endif

#if defined(__NetBSD__)
    res = getrlimit(RLIMIT_OFILE, &rl);
    rl.rlim_cur = MAX_CONNECTS;
    res = setrlimit(RLIMIT_OFILE, &rl);
#endif

#endif

    handleCmdLineArgs(argc, argv);

    Uptime = time(0);
    LogPrint(LOG_CRIT, "HavokMUD code version: %s", VERSION);
    LogPrint(LOG_CRIT, "Running game on port %d.", mud_port);

    srandom(time(0));
    REMOVE_BIT(SystemFlags, SYS_WIZLOCKED);

#ifdef SITELOCK
    LogPrintNoArg(LOG_CRIT, "Blanking denied hosts.");
    for (a = 0; a < MAX_BAN_HOSTS; a++) {
        strcpy(hostlist[a], " \0\0\0\0");
    }
    numberhosts = 0;
#endif

    /*
     * close stdin 
     */
    close(0);

    db_setup();
    db_initial_load();

    descriptor_list = NULL;

    LogPrintNoArg(LOG_CRIT, "Signal trapping.");
    signal_setup();

    boot_db();

    LogPrintNoArg(LOG_CRIT, "Starting threads.");
    spy_flag = FALSE;

    StartThreads();

    if (reboot_now) {
        LogPrintNoArg(LOG_CRIT, "Rebooting.");
    }

    LogPrintNoArg(LOG_CRIT, "Normal termination of game.");

    return (0);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
