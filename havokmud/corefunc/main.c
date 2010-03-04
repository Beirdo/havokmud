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
#include <getopt.h>

#include "oldexterns.h"
#include "oldutils.h"
#include "version.h"
#include "logging.h"
#include "balanced_btree.h"
#include "interthread.h"
#include "protos.h"

static char ident[] _UNUSED_ =
    "$Id$";

void StartThreads( void );
void boot_db(void);
void reset_time(void);

void MainParseArgs( int argc, char **argv );
void MainDisplayUsage( char *program, char *errorMsg );
void mainAbout( void *arg );
void mainLicensing( void *arg );
void mainVersions( void *arg );
void mainReloadAll( void *arg );
int versionShowRecurse( BalancedBTreeItem_t *node, int line );


#if 0
extern char    *optarg;
extern int      optind;
#endif

bool                verbose;
bool                Daemon;
bool                Debug;
bool                MudDone = FALSE;
bool                GlobalAbort = FALSE;
BalancedBTree_t    *versionTree;
int        mud_port;            /**< The TCP port the MUD will listen on */
bool       no_specials = FALSE; /**< Disable special functions completely */

/* 
 * max number of descriptors (connections) 
 * THIS IS SYSTEM DEPENDANT, use 64 is not sure! 
 */
#define MAX_CONNECTS    256        

#define MAX_HOSTNAME    256

#if 0
/* 
 * time delay corresponding to 4
 * passes/sec 
 */
#define OPT_USEC 250000         
#endif



#ifdef SITELOCK
BalancedBTree_t    *banHostTree;    /**< Balanced BTree of banned hosts, sorted
                                         by hostname */
#endif

BalancedBTree_t    *descNameTree;   /**< Balanced BTree of descriptors, sorted 
                                         by player name */
BalancedBTree_t    *descNumTree;    /**< Balanced BTree of descriptors, sorted
                                         by socket descriptor number */

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
 * @brief Parses the command line arguments
 * @param argc Count of arguments
 * @param argv Vector of the command line arguments
 *
 * Parses the command line arguments and sets up the defaults for the system
 */
void MainParseArgs( int argc, char **argv )
{
#if 0
    extern char *optarg;
    extern int optind, opterr, optopt;
#endif
    int opt;
    int optIndex = 0;
    static struct option longOpts[] = {
        {"help", 0, 0, 'h'},
        {"version", 0, 0, 'V'},
        {"host", 1, 0, 'H'},
        {"user", 1, 0, 'u'},
        {"password", 1, 0, 'p'},
        {"port", 1, 0, 'P'},
        {"mudport", 1, 0, 'm'},
        {"database", 1, 0, 'd'},
        {"daemon", 0, 0, 'D'},
        {"verbose", 0, 0, 'v'},
        {"debug", 0, 0, 'g'},
        {"nospecial", 0, 0, 's'},
        {"nocolor", 0, 0, 'A'},
        {"nodns", 0, 0, 'N'},
        {"auth", 0, 0, 'R'},
        {"logall", 0, 0, 'L'},
        {0, 0, 0, 0}
    };

    mud_port = -1;
    mySQL_host = NULL;
    mySQL_port = 0;
    mySQL_user = NULL;
    mySQL_passwd = NULL;
    mySQL_db = NULL;
    verbose = FALSE;
    Debug = FALSE;
    Daemon = FALSE;

    while( (opt = getopt_long( argc, argv, "hVH:P:u:p:m:d:Dgvs", longOpts, 
                               &optIndex )) != -1 )
    {
        switch( opt )
        {
            case 'h':
                MainDisplayUsage( argv[0], NULL );
                exit( 0 );
                break;
            case 'D':
                Daemon = TRUE;
                break;
            case 'g':
                Debug = TRUE;
                break;
            case 'v':
                verbose = TRUE;
                break;
            case 'H':
                if( mySQL_host != NULL )
                {
                    memfree( mySQL_host );
                }
                mySQL_host = memstrlink(optarg);
                break;
            case 'P':
                mySQL_port = atoi(optarg);
                break;
            case 'u':
                if( mySQL_user != NULL )
                {
                    memfree( mySQL_user );
                }
                mySQL_user = memstrlink(optarg);
                break;
            case 'p':
                if( mySQL_passwd != NULL )
                {
                    memfree( mySQL_passwd );
                }
                mySQL_passwd = memstrlink(optarg);
                break;
            case 'd':
                if( mySQL_db != NULL )
                {
                    memfree( mySQL_db );
                }
                mySQL_db = memstrlink(optarg);
                break;
            case 'V':
                LogBanner();
                exit( 0 );
                break;
            case 's':
                no_specials = TRUE;
                LogPrintNoArg(LOG_CRIT, "Suppressing assignment of special "
                                        "routines.");
                break;
            case 'm':
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

            case '?':
            case ':':
            default:
                MainDisplayUsage( argv[0], "Unknown option" );
                exit( 1 );
                break;
        }
    }

    if( mySQL_host == NULL )
    {
        mySQL_host = memstrlink(DEF_MYSQL_HOST);
    }

    if( mySQL_port == 0 )
    {
        mySQL_port = 3306;
    }

    if( mySQL_user == NULL )
    {
        mySQL_user = memstrlink(DEF_MYSQL_USER);
    }

    if( mySQL_passwd == NULL )
    {
        mySQL_passwd = memstrlink(DEF_MYSQL_PASSWD);
    }

    if( mySQL_db == NULL )
    {
        mySQL_db = memstrlink(DEF_MYSQL_DB);
    }

    if( Daemon ) {
        verbose = FALSE;
    }

    if (mud_port >= 0  && mud_port <= 1024) {
        printf("Illegal port #\n");
        exit(1);
    }
}

/**
 * @brief Show command line arguments
 * @param progname the name of the program being run
 *
 * Shows the command line arguments used by the program onto stderr
 */
void MainDisplayUsage( char *program, char *errorMsg )
{
    char *nullString = "<program name>";

    LogBanner();

    if( errorMsg != NULL )
    {
        fprintf( stderr, "\n%s\n\n", errorMsg );
    }

    if( program == NULL )
    {
        program = nullString;
    }

    fprintf( stderr, "\nUsage:\n\t%s [-H host] [-P port] [-u user] "
                     "[-p password] [-d database] [-D] [-v]\n\n", program );
    fprintf( stderr, 
               "Options:\n"
               "\t-H or --host\tMySQL host to connect to (default %s)\n"
               "\t-P or --port\tMySQL port to connect to (default %d)\n"
               "\t-u or --user\tMySQL user to connect as (default %s)\n"
               "\t-p or --password\tMySQL password to use (default %s)\n"
               "\t-d or --database\tMySQL database to use (default %s)\n"
               "\t-D or --daemon\tRun solely in daemon mode, detached\n"
               "\t-v or --verbose\tShow verbose information while running\n"
               "\t-g or --debug\tWrite a debugging logfile\n"
               "\t-s or --nospecial\tDisable Special functions\n"
               "\t-m or --mudport\tMUD port to use (default %d or database)\n"
               "\t-V or --version\tshow the version number and quit\n"
               "\t-h or --help\tshow this help text\n", DEF_MYSQL_HOST, 
               DEF_MYSQL_PORT, DEF_MYSQL_USER, DEF_MYSQL_PASSWD, DEF_MYSQL_DB,
               DEF_MUD_PORT );
    fprintf(stderr, 
               "\t-A or --nocolor\tDisable ALL color\n"
               "\t-N or --nodns\tDisable DNS lookups\n"
               "\t-R or --auth\tEnable newbie authorizing\n"
               "\t-L or --logall\tLog all users\n\n" );
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

    MainParseArgs(argc, argv);

    Uptime = time(0);
#if 0
    LogPrint(LOG_CRIT, "HavokMUD code version: %s", SRC_VERSION);
    LogPrint(LOG_CRIT, "Running game on port %d.", mud_port);
#endif

    srandom(time(0));
    REMOVE_BIT(SystemFlags, SYS_WIZLOCKED);

    versionTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );

#if 0
    LogPrintNoArg(LOG_CRIT, "Starting threads.");
#endif
    StartThreads();


#ifdef SITELOCK
    banHostTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
#endif

    /*
     * close stdin 
     */
    close(0);

#if 0
    db_initial_load();
#endif

    descNameTree = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    descNumTree  = BalancedBTreeCreate( NULL, BTREE_KEY_INT );

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
#if 0
        assign_mobiles();
#endif

#if 0
        boot_the_shops();
        assign_the_shopkeepers();
#endif

#if 0
        LogPrintNoArg( LOG_CRIT, "   Objects.");
        assign_objects();
#endif

#if 0
        LogPrintNoArg( LOG_CRIT, "   Room.");
        assign_rooms();
#endif
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
#if 0
            switch_light(MOON_SET);
#endif
            break;
        }
    case 5:
    case 6:
        {
            weather_info.sunlight = SUN_RISE;
#if 0
            switch_light(SUN_RISE);
#endif
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
            weather_info.sunlight = SUN_DARK;
#if 0
            switch_light(SUN_DARK);
#endif
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
 * New stuff
 */

void versionAdd( char *what, char *version )
{
    BalancedBTreeItem_t    *item;
    Version_t              *vItem;

    item = BalancedBTreeFind( versionTree, &what, UNLOCKED, FALSE );
    if( item ) {
        vItem = (Version_t *)item->item;
        vItem->count++;
        return;
    }

    vItem = CREATE(Version_t);
    item = CREATE(BalancedBTreeItem_t);

    vItem->what    = memstrlink( what );
    vItem->version = memstrlink( version );
    vItem->count   = 1;

    item->item     = (void *)vItem;
    item->key      = (void *)&vItem->what;

    BalancedBTreeAdd( versionTree, item, UNLOCKED, TRUE );
}

void versionRemove( char *what )
{
    BalancedBTreeItem_t    *item;
    Version_t              *vItem;
    
    item = BalancedBTreeFind( versionTree, &what, UNLOCKED, FALSE );
    if( !item ) {
        return;
    }

    vItem = (Version_t *)item->item;
    vItem->count--;

    if( !vItem->count ) {
        BalancedBTreeRemove( versionTree, item, UNLOCKED, TRUE );

        memfree( vItem->what );
        memfree( vItem->version );
        memfree( vItem );
        memfree( item );
    }
}

void MainDelayExit( void )
{
    int         i;

    LogPrintNoArg( LOG_CRIT, "Shutting down" );

    /* Signal to all that we are aborting */
    MudDone = FALSE;

    GlobalAbort = TRUE;

    /* Send out signals from all queues waking up anything waiting on them so
     * the listeners can unblock and die
     */
    QueueKillAll();

    /* Delay to allow all the other tasks to finish (esp. logging!) */
    LogPrintNoArg(LOG_INFO, "Waiting 15s for all threads to finish");
    for( i = 15; i && !MudDone; i-- ) {
        sleep(1);
    }

    LogPrintNoArg(LOG_DEBUG, "Shutdown complete!" );
    LogFlushOutput();

#if 0
    cursesAtExit();
#endif

    /* And finally... die */
    _exit( 0 );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
