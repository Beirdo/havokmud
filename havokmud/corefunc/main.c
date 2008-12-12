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
#include "memory.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "oldexterns.h"
#include "oldutils.h"
#include "version.h"
#include "logging.h"
#include "balanced_btree.h"
#include "interthread.h"

static char ident[] _UNUSED_ =
    "$Id$";

void display_usage(char *progname);
void handleCmdLineArgs(int argc, char **argv);
void StartThreads( void );
void boot_db(void);
void reset_time(void);

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

int             mud_port = 6969;    /**< The TCP port the MUD will listen on */
int             no_specials = 0;    /**< Disable special functions completely */


#ifdef SITELOCK
BalancedBTree_t    *banHostTree;    /**< Balanced BTree of banned hosts, sorted
                                         by hostname */
#endif

BalancedBTree_t    *descNameTree;   /**< Balanced BTree of descriptors, sorted 
                                         by player name */
BalancedBTree_t    *descNumTree;    /**< Balanced BTree of descriptors, sorted
                                         by socket descriptor number */

bool            GlobalAbort = FALSE;
long            SystemFlags;
long            total_connections;
long            total_max_players;

struct time_info_data time_info;        /* the infomation about the time */
struct weather_data weather_info;       /* the infomation about the
                                         * weather */
int             mudshutdown = 0;        /* clean shutdown */
int             reboot_now = 0;     /* reboot the game after a shutdown */
time_t          Uptime;         /* time that the game has been up */

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
    fprintf(stderr, "\t-s\tDisable special procedures\n"
                    "\t-D\tDefine the MySQL database (default %s)\n",
                    DEF_MYSQL_DB );
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

    mud_port = -1;

    while( (opt = getopt(argc, argv, "lsD:U:P:H:p:ANRLhV")) != -1 ) {
        switch (opt) {
        case 's':
            no_specials = 1;
            LogPrintNoArg(LOG_CRIT, "Suppressing assignment of special "
                                    "routines.");
            break;

        case 'D':
            /* Database */
            if( mySQL_db ) {
                memfree( mySQL_db );
            }
            mySQL_db = strdup(optarg);
            break;

        case 'U':
            /* Database user */
            if( mySQL_user ) {
                memfree( mySQL_user );
            }
            mySQL_user = strdup(optarg);
            break;

        case 'P':
            /* Database password */
            if( mySQL_passwd ) {
                memfree( mySQL_passwd );
            }
            mySQL_passwd = strdup(optarg);
            break;

        case 'H':
            /* Database host */
            if( mySQL_host ) {
                memfree( mySQL_host );
            }
            mySQL_host = strdup(optarg);
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
}

#if 1
int      spy_flag;
#endif 


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

    LogPrintNoArg(LOG_CRIT, "Starting threads.");
    StartThreads();


#ifdef SITELOCK
    banHostTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
#endif

    /*
     * close stdin 
     */
    close(0);

    db_setup();
    db_initial_load();

    descNameTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    descNumTree  = BalancedBTreeCreate( NULL, BTREE_KEY_INT );

    LogPrintNoArg(LOG_CRIT, "Signal trapping.");
    signal_setup();

    boot_db();

    spy_flag = FALSE;

    if (reboot_now) {
        LogPrintNoArg(LOG_CRIT, "Rebooting.");
    }

    LogPrintNoArg(LOG_CRIT, "Normal termination of game.");

    return (0);
}

void boot_db(void)
{
#if 0
    int             i;

    char           *s;
    long            d,
                    e;
#endif

    LogPrintNoArg( LOG_CRIT, "Resetting the game time:");
    reset_time();

    LogPrintNoArg( LOG_CRIT, "Reading newsfile, credits, info and motd.");
    db_load_textfiles();

    LogPrintNoArg( LOG_CRIT, "Initializing Script Files.");

#if 0
    /*
     * some machines are pre-allocation specific when dealing with realloc
     */
    script_data = CREATE(struct scripts);
    CommandSetup();
    mobileInitScripts();
#endif

#ifdef NOTYET
    LogPrintNoArg( LOG_CRIT, "Opening mobile files.");
    if (!(mob_f = fopen(MOB_FILE, "r"))) {
        perror("boot");
        assert(0);
    }
#endif

#ifdef CLEAN_AT_BOOT
    LogPrintNoArg( LOG_CRIT, "Clearing inactive players");
    clean_playerfile();
#endif

#if 0
    LogPrintNoArg( LOG_CRIT, "Loading zone table.");
    boot_zones();

    LogPrintNoArg( LOG_CRIT, "Loading saved zone table.");
    boot_saved_zones();
#endif

    LogPrintNoArg( LOG_CRIT, "Loading rooms.");
    initializeRooms();

    LogPrintNoArg( LOG_CRIT, "Generating binary tree of mobiles.");
    initializeMobiles();

    LogPrintNoArg( LOG_CRIT, "Generating binary tree of objects.");
    initializeObjects();

#if 0
    LogPrintNoArg( LOG_CRIT, "Renumbering zone table.");
    renum_zone_table(0);

    LogPrintNoArg( LOG_CRIT, "Generating player index.");
    build_player_index();
#endif

    LogPrintNoArg( LOG_CRIT, "Assigning function pointers:");
    if (!no_specials) {
        LogPrintNoArg( LOG_CRIT, "   Mobiles.");
        assign_mobiles();

#if 0
        boot_the_shops();
        assign_the_shopkeepers();
#endif

        LogPrintNoArg( LOG_CRIT, "   Objects.");
        assign_objects();

        LogPrintNoArg( LOG_CRIT, "   Room.");
        assign_rooms();
    }

#if 0
    LogPrintNoArg( LOG_CRIT, "   Commands.");
    assign_command_pointers();

    LogPrintNoArg( LOG_CRIT, "Updating characters with saved items:");
    update_obj_file();
#endif

#ifdef LIMITED_ITEMS
    PrintLimitedItems();
#endif

#if 0
    for (i = 0; i <= top_of_zone_table; i++) {
        s = zone_table[i].name;
        d = (i ? (zone_table[i - 1].top + 1) : 0);
        e = zone_table[i].top;
        LogPrint( LOG_CRIT, "Performing boot-time init of %d:%s "
                            "(rooms %ld-%ld)", zone_table[i].num, s, d, e);
        zone_table[i].start = 0;

        if (i == 0) {
            LogPrintNoArg( LOG_CRIT, "Performing boot-time reload of static "
                                     "mobs");
            reset_zone(0, 0);
        }

        if (i == 1) {
            LogPrint( LOG_CRIT, "Automatic initialization of  %s\n", s);
            reset_zone(1, 0);
        }
    }
#endif

#if 0
    reset_q.head = reset_q.tail = 0;
#endif

    /*
     * cycle through pfiles to see if anyone
     * get a statue
     */
#if 0
    generate_legend_statue();
#endif
}

/*
 * reset the time in the game from file
 */
void reset_time(void)
{
#if 0
    long            beginning_of_time = 650336715;
#endif

    struct time_info_data mud_time_passed(time_t t2, time_t t1);

#if 0
    time_info = mud_time_passed(time(0), beginning_of_time);
#endif

    moontype = time_info.day;

    switch (time_info.hours) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        {
            weather_info.sunlight = SUN_DARK;
            switch_light(MOON_SET);
            break;
        }
    case 5:
    case 6:
        {
            weather_info.sunlight = SUN_RISE;
            switch_light(SUN_RISE);
            break;
        }
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        {
            weather_info.sunlight = SUN_LIGHT;
            break;
        }
    case 19:
    case 20:
        {
            weather_info.sunlight = SUN_SET;
            break;
        }
    case 21:
    case 22:
    case 23:
    default:
        {
            switch_light(SUN_DARK);
            weather_info.sunlight = SUN_DARK;
            break;
        }
    }

    LogPrint(LOG_CRIT, "   Current Gametime: %dH %dD %dM %dY.", time_info.hours,
             time_info.day, time_info.month, time_info.year);

    weather_info.pressure = 960;
    if ((time_info.month >= 7) && (time_info.month <= 12)) {
        weather_info.pressure += dice(1, 50);
    } else {
        weather_info.pressure += dice(1, 80);
    }

    weather_info.change = 0;

    if (weather_info.pressure <= 980) {
        if ((time_info.month >= 3) && (time_info.month <= 14)) {
            weather_info.sky = SKY_LIGHTNING;
        } else {
            weather_info.sky = SKY_LIGHTNING;
        }
    } else if (weather_info.pressure <= 1000) {
        if ((time_info.month >= 3) && (time_info.month <= 14)) {
            weather_info.sky = SKY_RAINING;
        } else {
            weather_info.sky = SKY_RAINING;
        }
    } else if (weather_info.pressure <= 1020) {
        weather_info.sky = SKY_CLOUDY;
    } else {
        weather_info.sky = SKY_CLOUDLESS;
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
