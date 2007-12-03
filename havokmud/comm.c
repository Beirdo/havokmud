/*
 ** DaleMUD     comm.c main communication routines. Based on DIKU and
 **             SillyMUD.
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"
#include "version.h"

void            identd_test(struct sockaddr_in in_addr);
void display_usage(char *progname);

/* Make OSX work */
#ifndef HAVE_SOCKLEN_T
#define socklen_t int
#endif 

/* 
 * max number of descriptors (connections) 
 * THIS IS SYSTEM DEPENDANT, use 64 is not sure! 
 */
#define MAX_CONNECTS 256        


#define DFLT_PORT 4000          /* default port */
#define MAX_HOSTNAME   256

/* 
 * time delay corresponding to 4
 * passes/sec 
 */
#define OPT_USEC 250000         

int             mud_port;

struct descriptor_data *descriptor_list,
               *next_to_process;
struct txt_block *bufpool = 0;  /* pool of large output buffers */
int             buf_largecount; /* # of large buffers which exist */
int             buf_overflows;  /* # of overflows of output */
int             buf_switches;   /* # of switches from small to large buf */

/*
 * int slow_nameserver = FALSE; 
 */

int             lawful = 0;     /* work like the game regulator */
int             slow_death = 0; /* Shut her down, Martha, she's sucking
                                 * mud */
long            TempDis = 0;

int             pulse;

#ifdef SITELOCK
char            hostlist[MAX_BAN_HOSTS][256];    /* list of sites to ban */
int             numberhosts;
#endif

int             maxdesc,
                avail_descs;
int             tics = 0;       /* for extern checkpointing */

void sigsetmaskset( int *set, int count );

#if 0
/*
 *********************************************************************
 *  main game loop and related stuff                                    *
 ********************************************************************* */

int __main()
{
    return (1);
}
#endif

/*
 * jdb code - added to try to handle all the different ways the
 * connections can die, and try to keep these 'invalid' sockets from
 * getting to select 
 */

void close_socket_fd(int desc)
{
    struct descriptor_data *d;

#ifdef LOG_DEBUG
    slog("begin close_socket_fd");
#endif

    for (d = descriptor_list; d; d = d->next) {
        if (d->descriptor == desc) {
            close_socket(d);
        }
    }

#ifdef LOG_DEBUG
    slog("end close_socket_fd");
#endif
}

#if 0
void display_usage(char *progname)
{
    fprintf(stderr, "Usage:\n"
                    "%s [-l] [-d libdir] [-s] [-D sqlDB] [-U sqlUser] "
                    "[-P sqlPass]\n"
                    "\t[-H sqlHost] [-p port] [-A] [-N] [-R] [-L] [-h] [-V] "
                    "[port]\n\n",
                    progname );
    fprintf(stderr, "\t-l\tRun in legal mode\n"
                    "\t=d\tDefine the library dir (default %s)\n"
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
                    "\t-N\tDiaable DNS lookups\n"
                    "\t-R\tEnable newbie authorizing\n"
                    "\t-L\tLog all users\n"
                    "\t-h\tShow this help page\n"
                    "\t-V\tShow version and exit\n\n" );
    fprintf(stderr, "\tThe MUD port can be defined either with -p PORT or with "
                    "PORT at the\n"
                    "\tend of the command line\n\n" );
}

int main(int argc, char **argv)
{
    char           *dir;
    extern FILE    *log_f;
    extern long     SystemFlags;
    extern int      spy_flag;
    int             opt;
    extern char    *optarg;
    extern int      optind;

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

    mud_port = -1;
    dir = NULL;

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

    while( (opt = getopt(argc, argv, "ld:sD:U:P:H:p:ANRLhV")) != -1 ) {
        switch (opt) {
        case 'l':
            lawful = 1;
            Log("Lawful mode selected.");
            break;

        case 'd':
            if( dir ) {
                free( dir );
            }
            dir = strdup(optarg);
            break;

        case 's':
            no_specials = 1;
            Log("Suppressing assignment of special routines.");
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
            /* Database hose */
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
            Log("Disabling ALL color");
            break;

        case 'N':
            SET_BIT(SystemFlags, SYS_SKIPDNS);
            Log("Disabling DNS");
            break;

        case 'R':
            SET_BIT(SystemFlags, SYS_REQAPPROVE);
            Log("Newbie authorizes enabled");
            break;

        case 'L':
            SET_BIT(SystemFlags, SYS_LOGALL);
            Log("Logging all users");
            break;

        case 'V':
            Log("HavokMUD code version: %s", VERSION);
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

    Log("HavokMUD code version: %s", VERSION);

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

    Uptime = time(0);

    Log("Running game on port %d.", mud_port);

    if( !dir ) {
        dir = strdup(DFLT_DIR);
    }

    Log("Using %s as data directory.", dir);

    if (chdir(dir) < 0) {
        perror("chdir");
        exit(1);
    }

    free( dir );

    srandom(time(0));
    REMOVE_BIT(SystemFlags, SYS_WIZLOCKED);

#ifdef SITELOCK
    Log("Blanking denied hosts.");
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

    run_the_game(mud_port);
    fclose(log_f);
    return (0);
}

/*
 * Init sockets, run game, and cleanup sockets 
 */
int run_the_game(int port)
{
    extern int      spy_flag;
    int             s;
    void            signal_setup(void);
    int             load(void);

    descriptor_list = NULL;

    Log("Signal trapping.");
    signal_setup();

    Log("Opening mother connection.");
    s = init_socket(port);

#ifdef USE_LAWFUL

    if (lawful && load() >= 6) {
        Log("System load too high at startup.");
        coma(1);
    }
#endif

    boot_db();

    Log("Entering game loop.");
    spy_flag = FALSE;
    game_loop(s);

    close_sockets(s);

    if (reboot_now) {
        Log("Rebooting.");
    }

    Log("Normal termination of game.");
    return( 0 );
}
#endif

/*
 * Accept new connects, relay commands, and call 'heartbeat-functs' 
 */
void game_loop(int s)
{
    fd_set          input_set,
                    output_set,
                    exc_set;
    struct timeval  last_time,
                    now,
                    timespent,
                    timeout,
                    null_time;
    char            comm[MAX_INPUT_LENGTH];
    char            promptbuf[180];
    struct descriptor_data *point,
                   *next_point;
    struct char_data *ch;
    int             update = 0;

    static struct timeval opt_time;
    static int      sigs[] = { SIGUSR1, SIGUSR2, SIGINT, SIGPIPE, SIGALRM, 
                               SIGTERM, SIGHUP };
    static int      sigcount = NELEMENTS(sigs);

    extern int      pulse;
    extern int      maxdesc;
    extern struct time_info_data time_info;     /* In db.c */

    null_time.tv_sec = 0;
    null_time.tv_usec = 0;

    opt_time.tv_usec = OPT_USEC;        /* Init time values */
    opt_time.tv_sec = 0;
#ifdef NETBSD
    gettimeofday(&last_time, (struct timezone *) NULL);
#else
    gettimeofday(&last_time, (struct timezone *) 0);
#endif

    maxdesc = s;

    /*
     * !! Change if more needed !! 
     */
    avail_descs = getdtablesize() - 2;  /* never used, pointless? */

    /*
     * Main loop 
     */
    while (!mudshutdown) {
        /*
         * Check what's happening out there 
         */

        FD_ZERO(&input_set);
        FD_ZERO(&output_set);
        FD_ZERO(&exc_set);

        FD_SET(s, &input_set);

#if 0
        dimd_loop();
#endif
        for (point = descriptor_list; point; point = point->next) {
            FD_SET(point->descriptor, &input_set);
            FD_SET(point->descriptor, &exc_set);
            FD_SET(point->descriptor, &output_set);

            if (maxdesc < point->descriptor) {
                maxdesc = point->descriptor;
            }
        }

        /*
         * check out the time 
         */
        gettimeofday(&now, NULL);

        timespent = timediff(&now, &last_time);
        timeout = timediff(&opt_time, &timespent);
        last_time.tv_sec = now.tv_sec + timeout.tv_sec;
        last_time.tv_usec = now.tv_usec + timeout.tv_usec;
        if (last_time.tv_usec >= 1000000) {
            last_time.tv_usec -= 1000000;
            last_time.tv_sec++;
        }

        sigsetmaskset(sigs, sigcount);

        if (select(maxdesc + 1, &input_set, &output_set, &exc_set, 
                   &null_time) < 0) {
            perror("Select poll");

            /*
             * one of the descriptors is broken... 
             */

            for (point = descriptor_list; point; point = next_point) {
                next_point = point->next;
                write_to_descriptor(point->descriptor, "\n\r");
            }

        }

        if (select(0, NULL, NULL, NULL, &timeout) < 0) {
            perror("Select sleep");
            /*
             * assert(0);
             */
        }

        sigsetmaskset(NULL, 0);

        /*
         * Respond to whatever might be happening 
         */

        /*
         * New connection? 
         */
        if (FD_ISSET(s, &input_set) && new_descriptor(s) < 0) {
            perror("New connection");
        }

        /*
         * kick out the freaky folks 
         */
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;
            if (FD_ISSET(point->descriptor, &exc_set)) {
                FD_CLR(point->descriptor, &input_set);
                FD_CLR(point->descriptor, &output_set);
                close_socket(point);
            }
        }

        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;
            if (FD_ISSET(point->descriptor, &input_set)) {
                if (process_input(point) < 0) {
                    close_socket(point);
                }
            }
        }

        /*
         * process_commands; 
         */
        for (point = descriptor_list; point; point = next_to_process) {
            next_to_process = point->next;

            if ((--(point->wait) <= 0) && get_from_q(&point->input, comm)) {
                if (point->character && point->connected == CON_PLYNG &&
                    point->character->specials.was_in_room != NOWHERE) {

                    point->character->specials.was_in_room = NOWHERE;
                    act("$n has returned.", TRUE, point->character, 0, 0,
                        TO_ROOM);
                }

                point->wait = 1;

                if (point->character) {
                    point->character->specials.timer = 0;
                }

                point->prompt_mode = 1;

                if (point->str) {
                    string_add(point, comm);
                } else if (!point->connected) {
                    if (point->showstr_count) 
                    {
                        show_string(point, comm);
                    } else {
                        command_interpreter(point->character, comm);
                    }
                } else if (point->connected == CON_EDITING) {
                    RoomEdit(point->character, comm);
                } else if (point->connected == CON_OBJ_EDITING) {
                    ObjEdit(point->character, comm);
                } else if (point->connected == CON_MOB_EDITING) {
                    MobEdit(point->character, comm);
                } else if (point->connected == CON_TFD_EDITING) {
                    TfdEdit(point->character, comm);
                } else {
#ifdef TODO
                    nanny(point, comm);
#endif
                }
            }
        }

        /*
         * either they are out of the game 
         * or they want a prompt.  
         */
        memory_check("end for 4, begin for 5");
        for (point = descriptor_list; point; point = next_point) {
            next_point = point->next;

#ifndef BLOCK_WRITE
            if (FD_ISSET(point->descriptor, &output_set) && point->output.head)
#else
            if (FD_ISSET(point->descriptor, &output_set) && *(point->output))
#endif
            {
                if (process_output(point) < 0) {
                    close_socket(point);
                } else {
                    point->prompt_mode = 1;
                }
            }
        }

        /*
         * give the people some prompts 
         */
        memory_check("end 5, begin 6");
        for (point = descriptor_list; point; point = point->next) {
            if (point->prompt_mode) {
                if (point->str) {
                    write_to_descriptor(point->descriptor, "-> ");
                } else if (!point->connected) {
                    if (point->showstr_count) 
#if 0
                if (point->showstr_point) 
#endif               
                    {
                        sprintf(promptbuf, "[ Enter to continue, (q)uit, "
                                           "(r)efresh, (b)ack, or page "
                                           "number (%d/%d) ]\n\r",
                                point->showstr_page, point->showstr_count);
                        write_to_descriptor(point->descriptor, promptbuf);
#if 0
                        write_to_descriptor(point->descriptor,"[Return "
                                "to continue/Q to quit]");
#endif
                    } else {
                        if (point->character->term == VT100) {
                            ch = point->character;

                            if (GET_MOVE(ch) != ch->last.move) {
                                SET_BIT(update, INFO_MOVE);
                                ch->last.move = GET_MOVE(ch);
                            }
                            if (GET_MAX_MOVE(ch) != ch->last.mmove) {
                                SET_BIT(update, INFO_MOVE);
                                ch->last.mmove = GET_MAX_MOVE(ch);
                            }
                            if (GET_HIT(ch) != ch->last.hit) {
                                SET_BIT(update, INFO_HP);
                                ch->last.hit = GET_HIT(ch);
                            }
                            if (GET_MAX_HIT(ch) != ch->last.mhit) {
                                SET_BIT(update, INFO_HP);
                                ch->last.mhit = GET_MAX_HIT(ch);
                            }
                            if (GET_MANA(ch) != ch->last.mana) {
                                SET_BIT(update, INFO_MANA);
                                ch->last.mana = GET_MANA(ch);
                            }
                            if (GET_MAX_MANA(ch) != ch->last.mmana) {
                                SET_BIT(update, INFO_MANA);
                                ch->last.mmana = GET_MAX_MANA(ch);
                            }
                            if (GET_GOLD(ch) != ch->last.gold) {
                                SET_BIT(update, INFO_GOLD);
                                ch->last.gold = GET_GOLD(ch);
                            }
                            if (GET_EXP(ch) != ch->last.exp) {
                                SET_BIT(update, INFO_EXP);
                                ch->last.exp = GET_EXP(ch);
                            }
                            if (update) {
                                UpdateScreen(ch, update);
                            }
                            sprintf(promptbuf, "> ");
                        } else {
                            construct_prompt(promptbuf, point->character);
                        }

                        if (point->character->player.has_mail) {
                            write_to_descriptor(point->descriptor, 
                                ParseAnsiColors( 
                                    IS_SET(point->character->player.user_flags, 
                                           USE_ANSI), "$c0003[MAIL] "));
                        }

                        if (IS_AFFECTED2(point->character, AFF2_AFK)) {
                            write_to_descriptor(point->descriptor, 
                                ParseAnsiColors(
                                    IS_SET(point->character->player.user_flags,
                                           USE_ANSI), "$c0006[AFK] "));
                        }

                        write_to_descriptor(point->descriptor, ParseAnsiColors( 
                                IS_SET(point->character->player.user_flags,
                                USE_ANSI), promptbuf));
                    }
                }
                point->prompt_mode = 0;
            }
        }
        

        /*
         * handle heartbeat stuff 
         */
        /*
         * Note: pulse now changes every 1/4 sec 
         */

        pulse++;

        if (!(pulse % PULSE_ZONE)) {
            dlog("Before zone update pulse");
            memory_check("BEfore Zone update");
            zone_update();
            if (lawful) {
                gr(s);
            }
        }

        if (!(pulse % PULSE_RIVER)) {
            dlog("Before river pulse ");
            memory_check("Before riverpulse");
            RiverPulseStuff(pulse);
        }

        if (!(pulse % PULSE_TELEPORT)) {
            dlog("Before Teleport pulse");
            memory_check("Tele pulse");
            TeleportPulseStuff(pulse);
        }

        if (!(pulse % PULSE_VIOLENCE)) {
            dlog("Before violence pulse...");
            memory_check("Pulse violence");
            check_mobile_activity(pulse);
            perform_violence(pulse);
            /* 
             * update memorizing spells 
             */
            update_mem(pulse);
        }

        /*
         * Newbie Advice 
         */
        if (!(pulse % PULSE_ADVICE)) {
            dlog("Before advice pulse");
            memory_check("advice pulse");
            AdvicePulseStuff(pulse);
        }

        if (!(pulse % PULSE_DARKNESS)) {
            dlog("Before darkness pulse");
            memory_check("darkness pulse");
            DarknessPulseStuff(pulse);
        }

        if (!(pulse % PULSE_ARENA)) {
            dlog("Before arena pulse");
            memory_check("arena pulse");
            ArenaPulseStuff(pulse);
        }
        if (!(pulse % PULSE_AUCTION)) {
            dlog("Before auction pulse");
            memory_check("auction pulse");
            AuctionPulseStuff(pulse);
        }

        if (!(pulse % PULSE_TROLLREGEN)) {
            dlog("Before troll regen pulse");
            memory_check("troll regen pulse");
            TrollRegenPulseStuff(pulse);
        }
        if (!(pulse % PULSE_TQP)) {
            dlog("Before travelqp pulse");
            memory_check("travelqp pulse");
            traveling_qp(pulse);
        }

        if (!(pulse % (SECS_PER_MUD_HOUR * 4))) {
            dlog("Before hourly tick pulse");
            memory_check(" before pulse weather");
            weather_and_time(1);
            affect_update(pulse);
            if (time_info.hours == 1)
                update_time();
        }

        if (!(pulse % PULSE_MAILCHECK)) {
            for (point = descriptor_list; point; point = point->next) {
                if (point->character) {
                    if (!IS_NPC(point->character)) {
                        point->character->player.has_mail =
                            db_has_mail(point->character->player.name);
                    }
                }
            }
        }

        memory_check("loop end");

        if (pulse >= 2400) {
            pulse = 0;
            if (lawful) {
                night_watchman();
            }
            check_reboot();
        }

        /* 
         * tics since last checkpoint signal 
         */
        tics++;
    }    
}

/*
 ******************************************************************
 *  general utility stuff (for local use)
 ****************************************************************** */

int get_from_q(struct txt_q *queue, char *dest)
{
    struct txt_block *tmp;

    /*
     * Q empty? 
     */
    if (!queue->head) {
        return (0);
    }

    if (!dest) {
        log_sev("Sending message to null destination.", 5);
        return (0);
    }
    tmp = queue->head;

    if (dest && queue->head->text) {
        strcpy(dest, queue->head->text);
    }
    queue->head = queue->head->next;

    if (tmp) {
        if (tmp->text) {
            free(tmp->text);
        }
        free(tmp);
    }
    return (1);
}

#ifndef BLOCK_WRITE
void write_to_q(char *txt, struct txt_q *queue)
{
    struct txt_block *new;
    char            tbuf[256];
    int             strl;

    if (!queue) {
        Log("Output message to non-existant queue");
        return;
    }

    CREATE(new, struct txt_block, 1);
    strl = strlen(txt);

    if (strl < 0 || strl > 35000) {
        Log("strlen returned bogus length in write_to_q, string was: ");
        strncpy( tbuf, txt, 120 );
        tbuf[120] = '\0';
        Log(tbuf);
        if (new) {
            free(new);
        }
        return;
    }

    if (!(new->text = strdup(txt))) {
        Log("strdup returned null");
        assert(0);
    }
    new->next = NULL;

    /*
     * Q empty? 
     */
    if (!queue->head) {
        queue->head = queue->tail = new;
    } else {
        queue->tail->next = new;
        queue->tail = new;
    }
}

#else
void write_to_q(char *txt, struct txt_q *queue)
{
    struct txt_block *new;
    char            tbuf[256];
    int             strl;

    if (!queue) {
        Log("Output message to non-existant queue");
        return;
    }

    CREATE(new, struct txt_block, 1);
    strl = strlen(txt);
    if (strl < 0 || strl > 45000) {
        Log("strlen returned bogus length in write_to_q, string was:");
        strncpy( tbuf, txt, 120 );
        tbuf[120] = '\0';
        Log(tbuf);
        if (new) {
            free(new);
        }
        return;
    }
    new->text = strdup(txt);
    new->next = NULL;

    /*
     * Q empty? 
     */
    if (!queue->head) {
        queue->head = queue->tail = new;
    } else {
        queue->tail->next = new;
        queue->tail = new;
    }
}
#endif

#ifdef BLOCK_WRITE
void write_to_output(char *txt, struct descriptor_data *t)
{
    int             size;

    txt = ParseAnsiColors(IS_SET(t->character->player.user_flags, USE_ANSI),
                          txt);
    size = strlen(txt);

    /*
     * if we're in the overflow state already, ignore this 
     */
    if (t->bufptr < 0) {
        Log("over flow stat in write_to_output, comm.c");
        assert(0);
        return;
    }

    /*
     * if we have enough space, just write to buffer and that's it! 
     */
    if (t->bufspace >= size) {
        strcpy(t->output + t->bufptr, txt);
        t->bufspace -= size;
        t->bufptr += size;
    } else {
        /* 
         * otherwise, try to switch to a large buffer 
         */
        if (t->large_outbuf || ((size + strlen(t->output)) > LARGE_BUFSIZE)) {
            /*
             * we're already using large buffer, or even the large buffer
             * in't big enough -- switch to overflow state 
             */
            t->bufptr = -1;
            buf_overflows++;
            return;
        }

        buf_switches++;
        /*
         * if the pool has a buffer in it, grab it 
         */
        if (bufpool) {
            t->large_outbuf = bufpool;
            bufpool = bufpool->next;
        } else {
            /* 
             * else create one 
             */
            CREATE(t->large_outbuf, struct txt_block, 1);
            CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);
            buf_largecount++;
        }

        strcpy(t->large_outbuf->text, t->output);
        t->output = t->large_outbuf->text;
        strcat(t->output, txt);
        t->bufspace = LARGE_BUFSIZE - 1 - strlen(t->output);
        t->bufptr = strlen(t->output);
    }
}
#else
        /*
         * we use write_to_q instead of write_to_output 
         */
#endif

struct timeval timediff(struct timeval *a, struct timeval *b)
{
    struct timeval  rslt,
                    tmp;

    tmp = *a;
    rslt.tv_usec = 0;
    rslt.tv_sec = 0;
    if (tmp.tv_sec < b->tv_sec) {
        return (rslt);
    }

    if ((rslt.tv_usec = tmp.tv_usec - b->tv_usec) < 0) {
        rslt.tv_usec += 1000000;
        --(tmp.tv_sec);
    }
    if ((rslt.tv_sec = tmp.tv_sec - b->tv_sec) < 0) {
        rslt.tv_usec = 0;
        rslt.tv_sec = 0;
    }
    return (rslt);
}

#ifndef BLOCK_WRITE
/*
 * Empty the queues before closing connection 
 */
void flush_queues(struct descriptor_data *d)
{
    char            dummy[MAX_STRING_LENGTH];

    while (get_from_q(&d->output, dummy));
    while (get_from_q(&d->input, dummy));
}
#else
void flush_queues(struct descriptor_data *d)
{
    char            buf2[MAX_STRING_LENGTH];

    if (d->large_outbuf) {
        d->large_outbuf->next = bufpool;
        bufpool = d->large_outbuf;
    }

    while (get_from_q(&d->input, buf2));
}
#endif

/*
 ******************************************************************
 *  socket handling
 ****************************************************************** */

int init_socket(int port)
{
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int             x = 1;
    int             fd;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Init_socket: socket");
        exit(1);
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &x, sizeof(x)) < 0) {
        perror("Init_socket: SO_REUSEADDR");
        close(fd);
        exit(1);
    }

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        perror("Init_socket: bind");
        close(fd);
        exit(1);
    }

    if (listen(fd, 3) < 0) {
        perror("Init_socket: listen");
        close(fd);
        exit(1);
    }

    return fd;
}

#ifdef TODO
int new_connection(int s)
{
    struct sockaddr_in isa;
#ifdef __sun__
    struct sockaddr peer;
    char            buf[MAX_STRING_LENGTH];
#endif
    int             i;
    int             t;

    i = sizeof(isa);
#if 0
    getsockname(s, &isa, &i);
#endif

    if ((t = accept(s, (struct sockaddr *) &isa, (socklen_t *)&i)) < 0) {
        perror("Accept");
        return (-1);
    }
    nonblock(t);

#ifdef __sun__

    i = sizeof(peer);
    if (!getpeername(t, &peer, &i)) {
        *(peer.sa_data + 49) = '\0';
        Log("New connection from addr %s.", peer.sa_data);
    }
#endif

    return (t);
}

int new_descriptor(int s)
{

    int             desc,
                    size,
                    i;
    struct descriptor_data *newd;
    struct hostent *from;
    struct sockaddr_in sock;
    char            buf[200];

    if ((desc = new_connection(s)) < 0) {
        return (-1);
    }

    if ((desc + 1) >= MAX_CONNECTS) {
        struct descriptor_data *d;
        sprintf(buf, "Sorry.. The game is full. (max players %d) Try again "
                     "later.\n\r", desc);
        write_to_descriptor(desc, buf);
        close(desc);

        for (d = descriptor_list; d; d = d->next) {
            if (!d->character) {
                close_socket(d);
            }
        }
        return (0);
    } else if (desc > maxdesc) {
        maxdesc = desc;
    }

    CREATE(newd, struct descriptor_data, 1);

    /*
     * find info 
     */
    size = sizeof(sock);
    if (getpeername(desc, (struct sockaddr *) &sock, (socklen_t *)&size) < 0) {
        perror("getpeername");
        *newd->host = '\0';
    } else if (IS_SET(SystemFlags, SYS_SKIPDNS) || 
               !(from = gethostbyaddr((char *) &sock.sin_addr,
                                      sizeof(sock.sin_addr), AF_INET))) {

        if (!IS_SET(SystemFlags, SYS_SKIPDNS)){
            /*
             * perror("gethostbyaddr"); I hope this wasn't important,
             * cause we were getting it every disconnect or so -Pentak 
             */
        }

        i = sock.sin_addr.s_addr;
        sprintf(newd->host, "%d.%d.%d.%d", (i & 0x000000FF),
                (i & 0x0000FF00) >> 8, (i & 0x00FF0000) >> 16,
                (i & 0xFF000000) >> 24);
    } else {
        strncpy(newd->host, from->h_name, 255);
        newd->host[255] = '\0';
    }

#if 0
    if (isbanned(newd->host) == BAN_ALL) {
        close(desc);
        Log("Connection attempt denied from [%s]", newd->host);
        if (newd) {
            free(newd);
        }
        return (0);
    }
#endif

    if (strncmp("localhost", newd->host, 9) != 0) {
        Log("New connection from addr %s: %d: %d", newd->host, desc, maxdesc);
    }

    /*
     * init desc data 
     */
    newd->descriptor = desc;
    newd->connected = CON_NME;
    newd->wait = 1;
    newd->prompt_mode = 0;
    *newd->buf = '\0';
    newd->str = NULL;
    newd->showstr_head = NULL;
    newd->showstr_point = NULL;
    *newd->last_input = '\0';
#ifndef BLOCK_WRITE
    newd->output.head = NULL;
#else
    newd->output = newd->small_outbuf;
    *(newd->output) = '\0';
    newd->bufspace = SMALL_BUFSIZE - 1;
    newd->large_outbuf = NULL;
#endif
    newd->input.head = NULL;
    newd->next = descriptor_list;
    newd->character = NULL;
    newd->original = NULL;
    newd->snoop.snooping = NULL;
    newd->snoop.snoop_by = NULL;

    /*
     * prepend to list 
     */

    descriptor_list = newd;

    SEND_TO_Q(ParseAnsiColors(1, login), newd);
    SEND_TO_Q("If you're using Tintin or Lyntin, your client may not display "
              "the password\n\r", newd);
    SEND_TO_Q("sequence unless you change your settings. Please do not be "
              "discouraged.\n\r\n\r", newd);
    SEND_TO_Q("What is thy name? ", newd);

    return (0);
}
#endif

#ifndef BLOCK_WRITE
int process_output(struct descriptor_data *t)
{
    char            i[MAX_STRING_LENGTH + MAX_STRING_LENGTH];

    if (!t->prompt_mode && !t->connected && 
        write_to_descriptor(t->descriptor, "\n\r") < 0) {
        return (-1);
    }

    /*
     * Cycle thru output queue 
     */
    while (get_from_q(&t->output, i)) {
        if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
            SEND_TO_Q("% ", t->snoop.snoop_by->desc);
            SEND_TO_Q(i, t->snoop.snoop_by->desc);
        }
        if (write_to_descriptor(t->descriptor, i)) {
            return (-1);
        }
    }

    if (!t->connected && !(t->character && !IS_NPC(t->character) && 
        IS_SET(t->character->specials.act, PLR_COMPACT)) && 
        write_to_descriptor(t->descriptor, "\n\r") < 0) {
            return (-1);
    }

    return (1);
}
#else

/*
 * SEARCH HERE 
 */
int process_output(struct descriptor_data *t)
{
    char     buf[MAX_STRING_LENGTH];
    struct descriptor_data *point,
                   *next_point;

    /*
     * start writing at the 2nd space so we can prepend "% " for snoop 
     */
    if (!t->prompt_mode && !t->connected) {
        strcpy(&buf[2], "\n\r");
        strcat(&buf[2], t->output);
    } else {
        strcpy(&buf[2], t->output);
    }

    if (t->bufptr < 0) {
        Log("***** OVER FLOW **** in process_output, comm.c");
        strcat(&buf[2], "**OVERFLOW**");
    }

    if (!t->connected && !t->character && !IS_NPC(t->character) && 
        IS_SET(t->character->specials.act, PLR_COMPACT)) {
        strcat(&buf[2], "\n\r");
    }

    if (write_to_descriptor(t->descriptor, &buf[2]) < 0) {
        return -1;
    }

    if (t->snoop.snoop_by) {
        buf[0] = '%';
        buf[1] = ' ';
        SEND_TO_Q(buf, t->snoop.snoop_by->desc);
    }

    /*
     * if we were using a large buffer, put the large buffer on the buffer
     * pool and switch back to the small one 
     */
    if (t->large_outbuf) {
        t->large_outbuf->next = bufpool;
        bufpool = t->large_outbuf;
        t->large_outbuf = NULL;
        t->output = t->small_outbuf;
    }

    /*
     * reset total bufspace back to that of a small buffer 
     */
    t->bufspace = SMALL_BUFSIZE - 1;
    t->bufptr = 0;
    *(t->output) = '\0';

    return (1);
}

#endif

#if 1                           /* reset to use this code, lets see if it
                                 * helps */
int write_to_descriptor(int desc, char *txt)
{
    int             sofar,
                    thisround,
                    total;

    total = strlen(txt);
    sofar = 0;

    do {
        thisround = write(desc, txt + sofar, total - sofar);
        if (thisround < 0) {
            if (errno == EWOULDBLOCK) {
                break;
            }
            perror("Write to socket");
            /*
             * lets see if this stops it from crashing
             * close_socket_fd(desc);
             */
            return (-1);
        }
        sofar += thisround;
    } while (sofar < total);

    return (0);
}
#else
/*
 * merc code 
 */
#define UMIN(a, b)              ((a) < (b) ? (a) : (b))
int write_to_descriptor(int desc, char *txt)
{
    int             iStart;
    int             nWrite;
    int             nBlock,
                    length;
    char            buf[256];

    length = strlen(txt);

    for (iStart = 0; iStart < length; iStart += nWrite) {
        nBlock = UMIN(length - iStart, 4096);
        if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
            if (errno == EWOULDBLOCK) {
                break;
            }
            Log("<#=%d> had a error (%d) in write to descriptor (Broken Pipe?)",
                desc, errno);
            perror("Write_to_descriptor");
            /*
             * close_socket_fd(desc); 
             */
            return (-1);
        }
    }

    return (0);
}

#endif

/*
 * SEARCH HERE 
 */
#ifndef BLOCK_WRITE
/*
 * base silly process_input 
 */
int process_input(struct descriptor_data *t)
{
    int             sofar,
                    thisround,
                    begin,
                    squelch,
                    i,
                    k,
                    flag;
    char            tmp[MAX_INPUT_LENGTH + 2],
                    buffer[MAX_INPUT_LENGTH + 60];

    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);

    /*
     * Read in some stuff 
     */
    do {
        if ((thisround = 
                    read(t->descriptor, t->buf + begin + sofar,
                         (8 * MAX_INPUT_LENGTH) - (begin + sofar) - 1)) > 0) {
#if 0
            /* was */
                              MAX_STRING_LENGTH - (begin + sofar) - 1)) > 0) 
#endif
            sofar += thisround;
        } else {
            if (thisround < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("Read1 - ERROR");
                    return (-1);
                } else {
                    break;
                }
            } else {
                Log("EOF encountered on socket read.");
                return (-1);
            }
        }
    } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

    *(t->buf + begin + sofar) = 0;

    /*
     * if no newline is contained in input, return without proc'ing 
     */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++) {
        if (!*(t->buf + i)) {
            return (0);
        }
    }

    /*
     * input contains 1 or more newlines; process the stuff 
     */
    for (i = 0, k = 0; *(t->buf + i);) {
        if (!ISNEWL(*(t->buf + i)) && 
            !(flag = (k >= (MAX_INPUT_LENGTH - 2)))) {
            if (*(t->buf + i) == '\b') {
                /* 
                 * backspace 
                 */
                if (k) {
                    /* 
                     * more than one char ? 
                     */
                    if (*(tmp + --k) == '$') {
                        k--;
                    }
                    i++;
                } else {
                    /* 
                     * no or just one char.. Skip backsp 
                     */
                    i++;
                }
            } else {
                if (isascii((int)*(t->buf + i)) && 
                    isprint((int)*(t->buf + i))) {
                    /*
                     * trans char, double for '$' (printf) 
                     */
                    if ((*(tmp + k) = *(t->buf + i)) == '$') {
                        *(tmp + ++k) = '$';
                    }
                    k++;
                    i++;
                } else {
                    i++;
                }
            }
        } else {
            *(tmp + k) = 0;
            if (*tmp == '!') {
                strcpy(tmp, t->last_input);
            } else {
                strcpy(t->last_input, tmp);
            }

            write_to_q(tmp, &t->input);

            if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
                write_to_q("% ", &t->snoop.snoop_by->desc->output);
                write_to_q(tmp, &t->snoop.snoop_by->desc->output);
                write_to_q("\n\r", &t->snoop.snoop_by->desc->output);
            }

            if (flag) {
                sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", 
                        tmp);
                if (write_to_descriptor(t->descriptor, buffer) < 0) {
                    return (-1);
                }

                /*
                 * skip the rest of the line 
                 */
                while (!ISNEWL(t->buf[i])) {
                    i++;
                }
            }

            /*
             * find end of entry 
             */
            while (ISNEWL(t->buf[i])) {
                i++;
            }

            /*
             * squelch the entry from the buffer 
             */
            for (squelch = 0;; squelch++) {
                if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0') {
                    break;
                }
            }
            k = 0;
            i = 0;
        }
    }
    return (1);
}

#else

int process_input(struct descriptor_data *t)
{
    int             sofar,
                    thisround,
                    begin,
                    squelch,
                    i,
                    k,
                    flag;
    char            tmp[MAX_INPUT_LENGTH + 2],
                    buffer[MAX_INPUT_LENGTH + 60];

    sofar = 0;
    flag = 0;
    begin = strlen(t->buf);

    /*
     * Read in some stuff 
     */
    do {
        if ((thisround = 
                    read(t->descriptor, t->buf + begin + sofar,
                         (8 * MAX_INPUT_LENGTH) - (begin + sofar) - 1)) > 0) {
            sofar += thisround;
        } else {
            if (thisround < 0) {
                if (errno != EWOULDBLOCK) {
                    perror("Read1 - ERROR");
                    return (-1);
                } else {
                    break;
                }
            } else {
                Log("EOF encountered on socket read.");
                return (-1);
            }
        }
    } while (!ISNEWL(*(t->buf + begin + sofar - 1)));

    *(t->buf + begin + sofar) = 0;

    /*
     * if no newline is contained in input, return without proc'ing 
     */
    for (i = begin; !ISNEWL(*(t->buf + i)); i++) {
        if (!*(t->buf + i)) {
            return (0);
        }
    }

    /*
     * input contains 1 or more newlines; process the stuff 
     */
    for (i = 0, k = 0; *(t->buf + i);) {
        if (!ISNEWL(*(t->buf + i)) &&
            !(flag = (k >= (MAX_INPUT_LENGTH - 2)))) {
            if (*(t->buf + i) == '\b') {
                /* 
                 * backspace 
                 */
                if (k) {
                    /* 
                     * more than one char ? 
                     */
                    if (*(tmp + --k) == '$') {
                        k--;
                    }
                    i++;
                } else {
                    /* 
                     * no or just one char.. Skip backsp 
                     */
                    i++;
                }
            } else {
                if (isascii(*(t->buf + i)) && isprint(*(t->buf + i))) {
                    /*
                     * trans char, double for '$' (printf) 
                     */
                    if ((*(tmp + k) = *(t->buf + i)) == '$') {
                        *(tmp + ++k) = '$';
                    }
                    k++;
                    i++;
                } else {
                    i++;
                }
        } else {
            *(tmp + k) = 0;
            if (*tmp == '!') {
                strcpy(tmp, t->last_input);
            } else {
                strcpy(t->last_input, tmp);
            }

            write_to_q(tmp, &t->input);

            if ((t->snoop.snoop_by) && (t->snoop.snoop_by->desc)) {
                SEND_TO_Q("% ", t->snoop.snoop_by->desc);
                SEND_TO_Q(tmp, t->snoop.snoop_by->desc);
                SEND_TO_Q("\n\r", t->snoop.snoop_by->desc);
            }

            if (flag) {
                sprintf(buffer, "Line too long. Truncated to:\n\r%s\n\r", tmp);
                if (write_to_descriptor(t->descriptor, buffer) < 0) {
                    return (-1);
                }

                /*
                 * skip the rest of the line 
                 */
                while (!ISNEWL(t->buf[i])) {
                    i++;
                }
            }

            /*
             * find end of entry 
             */
            while (ISNEWL(t->buf[i])) {
                i++;
            }

            /*
             * squelch the entry from the buffer 
             */
            for (squelch = 0;; squelch++) {
                if ((*(t->buf + squelch) = *(t->buf + i + squelch)) == '\0') {
                    break;
                }
            }
            k = 0;
            i = 0;
        }
    }
    return (1);
}
#endif

void close_sockets(int s)
{
    Log("Closing all sockets.");

    while (descriptor_list) {
        close_socket(descriptor_list);
    }

    close(s);
}

void close_socket(struct descriptor_data *d)
{
    struct descriptor_data *tmp;

    if (!d) {
        Log("!d in close_socket");
        return;
    }
#ifdef LOG_DEBUG
    slog("begin close_socket");
#endif

    close(d->descriptor);
    flush_queues(d);
    if (d->descriptor == maxdesc) {
        --maxdesc;
    }

    /*
     * Forget snooping 
     */
    if (d->snoop.snooping) {
        d->snoop.snooping->desc->snoop.snoop_by = 0;
    }

    if (d->snoop.snoop_by) {
        send_to_char("Your victim is no longer among us.\n\r",
                     d->snoop.snoop_by);
        d->snoop.snoop_by->desc->snoop.snooping = 0;
    }

    if (d->character) {
        if (d->connected == CON_PLYNG) {
            /* 
             * Disallow energy restore after link close - MW
             */
            d->character->old_exp = 0;
            do_save(d->character, "", 0);
            act("$n has lost $s touch with reality.", TRUE, d->character,
                0, 0, TO_ROOM);

            if (!IS_IMMORTAL(d->character) || d->character->invis_level <= 58) {
                if (!GET_NAME(d->character)) {
                    Log("Closing link to: %s.", GET_NAME(d->character));
                } else {
                    Log("Closing link to: NULL.");
                }
            }   

            /* 
             * poly, or switched god 
             */
            if (IS_NPC(d->character) && d->character->desc) {
                d->character->orig = d->character->desc->original;
            }
            d->character->desc = 0;

            /*
             * d->character->invis_level = IMMORTAL; msw 8/9/94 
             */

            if (!IS_AFFECTED(d->character, AFF_CHARM) && d->character->master) {
                stop_follower(d->character);
            }
        } else {
            if (GET_NAME(d->character) && 
                (!IS_IMMORTAL(d->character) || 
                 d->character->invis_level <= 58)) {
                Log("Losing player: %s.", GET_NAME(d->character));
            }
            free_char(d->character);
        }
    } else {
        Log("Losing descriptor without char.");
    }

    if (next_to_process == d) {
        /* 
         * to avoid crashing the process loop 
         */
        next_to_process = next_to_process->next;
    }

    if (d == descriptor_list) {
        /* 
         * this is the head of the list 
         */
        descriptor_list = descriptor_list->next;
    } else {
        /* 
         * This is somewhere inside the list
         * Locate the previous element 
         */
        for (tmp = descriptor_list; tmp && tmp->next != d; tmp = tmp->next) {
            /* 
             * Empty loop 
             */
        }

        if (tmp != NULL) {
            tmp->next = d->next;
        } else {
            Log(" ERROR< ERROR< ERROR< ERROR< corrupted list...");
            /*
             * not sure where this gets fried, but it keeps popping up now
             * and then, let me know if you figure it out. msw 
             * DENIS see if you can figure this out 
             *
             * I know that when a new user cuts link in the login process
             * the first time it will get here, how to stop it I do not know 
             */
        }
    }

    if (d->showstr_head) {
        /* 
         * this piece of code causes core dumps on ardent titans 
         */
        free(d->showstr_head);  
    }

    if (d) {
        free(d);
    }

#ifdef LOG_DEBUG
    slog("end close_socket");
#endif
}


void nonblock(int s)
{
    if (fcntl(s, F_SETFL, O_NDELAY) == -1) {
        perror("Noblock");
        assert(0);
    }
}

#define COMA_SIGN \
"\n\r\
DikuMUD is currently inactive due to excessive load on the host machine.\n\r\
Please try again later.\n\r\n\
\n\r\
   Sadly,\n\r\
\n\r\
    the DikuMUD system operators\n\r\n\r"

/*
 * sleep while the load is too high 
 */
void coma(int s)
{
#ifdef USE_LAWFUL

    fd_set          input_set;
    static struct timeval timeout = {
        60,
        0
    };
    int             conn;

    int             workhours(void);
    int             load(void);

    static int      sigs[] = { SIGUSR1, SIGUSR2, SIGINT, SIGPIPE, SIGALRM,
                               SIGTERM, SIGURG, SIGXCPU, SIGHUP };
    static int      sigcount = NELEMS(sigs);


    Log("Entering comatose state.");

    sigsetmaskset(sigs, sigcount);

    while (descriptor_list) {
        close_socket(descriptor_list);
    }

    FD_ZERO(&input_set);
    do {
        FD_SET(s, &input_set);
        if (select(64, &input_set, 0, 0, &timeout) < 0) {
            perror("coma select");
            assert(0);
        }
        if (FD_ISSET(s, &input_set)) {
            if (load() < 6) {
                Log("Leaving coma with visitor.");
                sigsetmask(NULL, 0);
                return;
            }
            if ((conn = new_connection(s)) >= 0) {
                write_to_descriptor(conn, COMA_SIGN);
                sleep(2);
                close(conn);
            }
        }

        tics = 1;
        if (workhours()) {
            Log("Working hours collision during coma. Exit.");
            assert(0);
        }
    } while (load() >= 6);

    Log("Leaving coma.");
    sigsetmask(NULL, 0);
#endif
}

/*
 ****************************************************************
 *       Public routines for system-to-player-communication
 **************************************************************** 
 */
#ifdef TODO
/*
 * source: EOD, by John Booth <???> 
 */
int oldSendOutput(struct char_data *ch, char *fmt, ...)
{
    char            buf[MAX_STRING_LENGTH];     /* better safe than sorry */
    int             len;
    va_list         args;

    va_start(args, fmt);
    len = vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    send_to_char(buf, ch);

    return(len);
}

void send_to_char(char *messg, struct char_data *ch)
{
    if (ch && ch->desc && messg) {
        SEND_TO_Q(ParseAnsiColors(IS_SET(ch->player.user_flags, USE_ANSI),
                  messg), ch->desc);
    }
}
#endif

void save_all()
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next) {
        if (i->character) {
            save_char(i->character, AUTO_RENT);
        }
    }
}

/*
 * higher-level communication 
 */

/**
 * @todo use the common ANSI stuff rather than recreating the wheel!
 */
void act(char *str, int hide_invisible, struct char_data *ch,
         struct obj_data *obj, void *vict_obj, int type)
{
    register char  *strp,
                   *point,
                   *i = NULL;
    int             KLUDGE = TRUE;
    struct char_data *to = NULL;
    char            buf[MAX_STRING_LENGTH],
                    tmp[MAX_INPUT_LENGTH];

    if (!str || !*str || ch->in_room <= -1) {
        return;
    }

    if (type == TO_VICT) {
        to = (struct char_data *) vict_obj;
    } else if (type == TO_CHAR) {
        to = ch;
    } else if (real_roomp(ch->in_room)) {
        to = real_roomp(ch->in_room)->people;
    } else {
        Log("Crash in ACT");
    }

    for (; to; to = to->next_in_room) {
        if (to->desc && (to != ch || type == TO_CHAR) && 
            (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) && 
            !((type == TO_NOTVICT) && (to == (struct char_data *) vict_obj))) {
            for (strp = str, point = buf;;) {
                if (*strp == '$') { 
                    switch (*(++strp)) {
                       /* 
                        * Let's forget about direct color parsing for
                        * now... we a proc to
                        * do that after all...  - Manwe
                        */

                        /*
                         * parse ansi colors here $CMBFG, where M is
                         * modier, B is back ground color and FG is fore
                         * $C0001 would be normal, black back, red fore.
                         * $C1411 would be bold, blue back, light yellow
                         * fore 
                         */
                    case '$':
                        if (*strp != '$' || toupper(*(strp + 1)) != 'C' ||
                            !isdigit((int)*(strp + 2)) || 
                            !isdigit((int)*(strp + 3))) {
                            i = "$";
                            break;
                        }
                        strp++;
                    case 'C':
                    case 'c':
                        if (IS_SET(to->player.user_flags, USE_ANSI)) {
                            KLUDGE = TRUE;
                            strncpy( tmp, &strp[1], 4 );
                            tmp[4] = '\0';
                            i = ansi_parse(tmp);
                        } else
                            i = "";
                        ++strp;
                        ++strp;
                        ++strp;
                        ++strp; /* delete nums */
                        break;
                        /*
                         * case 'C': case 'c': break; 
                         * Let's just forget about the $c for now... -
                         * Manwe
                         */
                    case 'n':
                        i = PERS(ch, to);
                        break;
                    case 'N':
                        i = PERS((struct char_data *) vict_obj, to);
                        break;
                    case 'm':
                        i = HMHR(ch);
                        break;
                    case 'M':
                        i = HMHR((struct char_data *) vict_obj);
                        break;
                    case 's':
                        i = HSHR(ch);
                        break;
                    case 'S':
                        i = HSHR((struct char_data *) vict_obj);
                        break;
                    case 'e':
                        i = HSSH(ch);
                        break;
                    case 'E':
                        i = HSSH((struct char_data *) vict_obj);
                        break;
                    case 'o':
                        i = OBJN(obj, to);
                        break;
                    case 'O':
                        i = OBJN((struct obj_data *) vict_obj, to);
                        break;
                    case 'p':
                        i = OBJS(obj, to);
                        break;
                    case 'P':
                        i = OBJS((struct obj_data *) vict_obj, to);
                        break;
                    case 'a':
                        i = SANA(obj);
                        break;
                    case 'A':
                        i = SANA((struct obj_data *) vict_obj);
                        break;
                    case 'T':
                        i = (char *) vict_obj;
                        break;
                    case 'F':
                        i = fname((char *) vict_obj);
                        break;
                    default:
                        Log("Illegal $-code to act(): %s", str);
                        break;
                    }

                    while ((*point = *(i++))) {
                        ++point;
                    }

                    ++strp;

                } else if (!(*(point++) = *(strp++))) {
                    break;
                }
            }

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';
            /*
             * technically at that point, buf should be the string parsed, 
             * but with remaining color codes -Manwe 
             * I hope this will work... -Manwe
             */
            SEND_TO_Q(ParseAnsiColors(IS_SET(to->player.user_flags, USE_ANSI),
                                      CAP(buf)), to->desc);

            /*
             * kludge to turn the color back grey, try to move to prompt 
             */
            if (KLUDGE && IS_SET(to->player.user_flags, USE_ANSI)) {
                SEND_TO_Q(ansi_parse("0007"), to->desc);
            }
        }

        if ((type == TO_VICT) || (type == TO_CHAR)) {
            return;
        }
    }
}

/**
 * @todo how is this different from act()
 * @todo use common ansi handling
 */
void act2(char *str, int hide_invisible, struct char_data *ch,
          struct obj_data *obj, void *vict_obj, struct char_data *vict,
          int type)
{
    register char  *strp,
                   *point,
                   *i = NULL;
    int             KLUDGE = TRUE;
    struct char_data *to = NULL;
    char            buf[MAX_STRING_LENGTH],
                    tmp[MAX_INPUT_LENGTH];

    if (!str || !*str || ch->in_room <= -1) {
        return;
    }

    if (type == TO_VICT) {
        to = (struct char_data *) vict_obj;
    } else if (type == TO_CHAR) {
        to = ch;
    } else if (real_roomp(ch->in_room)) {
        to = real_roomp(ch->in_room)->people;
    } else {
        Log("Crash in ACT");
    }

    for (; to; to = to->next_in_room) {
        if (to->desc && (to != ch || type == TO_CHAR) && 
            (CAN_SEE(to, ch) || !hide_invisible) && AWAKE(to) && 
            !(type == TO_NOTVICT && to == (struct char_data *) vict_obj)) {
            for (strp = str, point = buf;;) {
                if (*strp == '$') {
                    switch (*(++strp)) {

                        /* Let's forget about direct color parsing for
                         * now... we a proc to
                         * do that after all...  - Manwe
                         *   
                         * parse ansi colors here $CMBFG, where M is
                         * modier, B is back ground color and FG is fore
                         * $C0001 would be normal, black back, red fore.
                         * $C1411 would be bold, blue back, light yellow
                         * fore 
                         */
                    case '$':
                        if (*strp != '$' || toupper(*(strp + 1)) != 'C' ||
                            !isdigit((int)*(strp + 2)) || 
                            !isdigit((int)*(strp + 3))) {
                            i = "$";
                            break;
                        }
                        strp++;
                    case 'C':
                    case 'c':
                        if (IS_SET(to->player.user_flags, USE_ANSI)) {
                            KLUDGE = TRUE;
                            strncpy( tmp, &strp[1], 4 );
                            tmp[4] = '\0';
                            i = ansi_parse(tmp);
                        } else
                            i = "";
                        ++strp;
                        ++strp;
                        ++strp;
                        ++strp; /* delete nums */
                        break;
                        /*
                         * case 'C': case 'c': break; 
                         *
                         * Let's just forget about the $c for now... -
                         * Manwe
                         */
                    case 'n':
                        i = PERS(ch, to);
                        break;
                    case 'N':
                        i = PERS((struct char_data *) vict_obj, to);
                        break;
                    case 'm':
                        i = HMHR(ch);
                        break;
                    case 'M':
                        i = HMHR((struct char_data *) vict_obj);
                        break;
                    case 's':
                        i = HSHR(ch);
                        break;
                    case 'S':
                        i = HSHR((struct char_data *) vict_obj);
                        break;
                    case 'e':
                        i = HSSH(ch);
                        break;
                    case 'E':
                        i = HSSH((struct char_data *) vict_obj);
                        break;
                    case 'o':
                        i = OBJN(obj, to);
                        break;
                    case 'O':
                        i = OBJN((struct obj_data *) vict_obj, to);
                        break;
                    case 'p':
                        i = OBJS(obj, to);
                        break;
                    case 'P':
                        i = OBJS((struct obj_data *) vict_obj, to);
                        break;
                    case 'a':
                        i = SANA(obj);
                        break;
                    case 'A':
                        i = SANA((struct obj_data *) vict_obj);
                        break;
                    case 'T':
                        i = (char *) vict_obj;
                        break;
                    case 'F':
                        i = fname((char *) vict_obj);
                        break;
                    default:
                        Log("Illegal $-code to act(): %s", str);
                        break;
                    }

                    while ((*point = *(i++))) {
                        ++point;
                    }

                    ++strp;

                } else if (!(*(point++) = *(strp++))) {
                    break;
                }
            }

            *(--point) = '\n';
            *(++point) = '\r';
            *(++point) = '\0';

            /*
             * technically at that point, buf should be the string parsed, 
             * but with remaining color codes -Manwe 
             *
             * I hope this will work... -Manwe
             */

            if (vict) {
                SEND_TO_Q(ParseAnsiColors(IS_SET(to->player.user_flags, 
                                          USE_ANSI), CAP(buf)), vict->desc);

                /*
                 * kludge to turn the color back grey, try to move to
                 * prompt 
                 */
                if (KLUDGE && IS_SET(to->player.user_flags, USE_ANSI)) {
                    SEND_TO_Q(ansi_parse("0007"), vict->desc);
                }
            }
        }
        if ((type == TO_VICT) || (type == TO_CHAR)) {
            return;
        }
    }
}

void raw_force_all(char *to_force)
{
    struct descriptor_data *i;

    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            oldSendOutput(i->character, "The game has forced you to '%s'.\n\r", 
                                    to_force);
            command_interpreter(i->character, to_force);
        }
    }
}

int _affected_by_s(struct char_data *ch, int skill)
{
    struct affected_type *hjp;
    int             fs = 0,
                    fa = 0;

    switch (skill) {
    case SPELL_BLADE_BARRIER:
        if (IS_AFFECTED(ch, AFF_BLADE_BARRIER)) {
            fa = 1;
        }
        break;
    case SPELL_CHILLSHIELD:
        if (IS_AFFECTED(ch, AFF_CHILLSHIELD)) {
            fa = 1;
        }
        break;
    case SPELL_FIRESHIELD:
        if (IS_AFFECTED(ch, AFF_FIRESHIELD)) {
            fa = 1;
        }
        break;
    case SPELL_SANCTUARY:
        if (IS_AFFECTED(ch, AFF_SANCTUARY)) {
            fa = 1;
        }
        break;
    case SPELL_GATHER_SHADOWS:
    case SPELL_INVISIBLE:
        if (IS_AFFECTED(ch, AFF_INVISIBLE)) {
            fa = 1;
        }
        break;
    case SPELL_TRUE_SIGHT:
        if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
            fa = 1;
        }
        break;
    case SPELL_PROT_ENERGY_DRAIN:
        if (IS_SET(ch->M_immune, IMM_DRAIN)) {
            fa = 1;
        }
        break;
    case IMM_DRAIN:
        if (IS_SET(ch->immune, IMM_DRAIN)) {
            fa = 1;
        }
        break;
    case SPELL_FLY:
        if (IS_AFFECTED(ch, AFF_FLYING)) {
            fa = 1;
        }
        break;
    }

    if (ch->affected) {
        for (hjp = ch->affected; hjp; hjp = hjp->next) {
            if (hjp->type == skill) {
                /* 
                 * in case it's 0 
                 */
                fs = (hjp->duration + 1);
            }
        }
    }

    if (!fa && !fs) {
        return -1;
    } else if (fa && !fs) {
        return 999;
    } else {
        return fs - 1;
    }
}

void construct_prompt(char *outbuf, struct char_data *ch)
{
    struct room_data *rm = 0;
    char            tbuf[255],
                   *pr_scan,
                   *mask;
    long            l,
                    exp,
                    texp;
    int             i,
                    s_flag = 0;

    *outbuf = 0;

    if (ch->specials.prompt == NULL) {
        /* 
         * use default prompts 
         */
        if (IS_IMMORTAL(ch)) {
            mask = "Havok: (type help prompt) H:%h R:%R i%iI+> ";
        } else {
            mask = "Havok: (type help prompt) H:%h M:%m V:%v> ";
        }
    } else if (ch->specials.fighting) {
        if (ch->specials.bprompt == NULL) {
            mask = ch->specials.prompt;
        } else {
            mask = ch->specials.bprompt;
        }
    } else {
        mask = ch->specials.prompt;
    }

    for (pr_scan = mask; *pr_scan; pr_scan++) {
        if (*pr_scan == '%') {
            if (*(++pr_scan) == '%') {
                tbuf[0] = '%';
                tbuf[1] = 0;
            } else {
                switch (*pr_scan) {
                    /*
                     * stats for character 
                     */
                case 'H':
                    sprintf(tbuf, "%d", GET_MAX_HIT(ch));
                    break;
                case 'h':
                    sprintf(tbuf, "%d", GET_HIT(ch));
                    break;
                case 'M':
                    sprintf(tbuf, "%d", GET_MAX_MANA(ch));
                    break;
                case 'm':
                    sprintf(tbuf, "%d", GET_MANA(ch));
                    break;
                case 'V':
                    sprintf(tbuf, "%d", GET_MAX_MOVE(ch));
                    break;
                case 'v':
                    sprintf(tbuf, "%d", GET_MOVE(ch));
                    break;
                case 'G':
                    sprintf(tbuf, "%d", GET_BANK(ch));
                    break;
                case 'g':
                    sprintf(tbuf, "%d", GET_GOLD(ch));
                    break;
                case 'X':       /* xp stuff */
                    sprintf(tbuf, "%d", GET_EXP(ch));
                    break;
                case 'x':
                    /* 
                     * xp left to level (any level, btw..) 
                     */
                    for (l = 1, i = 0, exp = 999999999;
                         i <= NECROMANCER_LEVEL_IND; i++, l <<= 1) {
                        if (HasClass(ch, l)) {
                            texp = classes[i].levels[GET_LEVEL(ch, i) + 1].exp -
                                   GET_EXP(ch);
                            if (texp < exp) {
                                exp = texp;
                            }
                        }
                    }
                    sprintf(tbuf, "%ld", exp);
                    break;
                case 'C':
                    /* 
                     * mob condition 
                     */
                    if (ch->specials.fighting) {
                        if (GET_MAX_HIT(ch->specials.fighting) == 0) {
                            strcpy(tbuf, "unknown");
                        } else {
                            i = (100 * GET_HIT(ch->specials.fighting)) /
                                GET_MAX_HIT(ch->specials.fighting);
                            if (i >= 100) {
                                strcpy(tbuf, "excellent");
                            } else if (i >= 90) {
                                strcpy(tbuf, "few scratches");
                            } else if (i >= 75) {
                                strcpy(tbuf, "small wounds");
                            } else if (i >= 50) {
                                strcpy(tbuf, "wounded");
                            } else if (i >= 30) {
                                strcpy(tbuf, "big nasty");
                            } else if (i >= 15) {
                                strcpy(tbuf, "badly wounded");
                            } else if (i >= 0) {
                                strcpy(tbuf, "awful");
                            } else {
                                strcpy(tbuf, "bleeding");
                            }
                        }
                    } else {
                        strcpy(tbuf, "*");
                    }
                    break;
                case 'c':
                    /* 
                     * tank condition 
                     */
                    if (ch->specials.fighting && 
                        ch->specials.fighting->specials.fighting) {
                        if (GET_MAX_HIT(ch->specials.fighting->
                                        specials.fighting) == 0) {
                            strcpy(tbuf, "Unknown");
                        } else {
                            i = (100 *
                                 GET_HIT(ch->specials.fighting->specials.
                                         fighting)) /
                                GET_MAX_HIT(ch->specials.fighting->
                                            specials.fighting);
                            if (i >= 100) {
                                strcpy(tbuf, "excellent");
                            } else if (i >= 90) {
                                strcpy(tbuf, "few scratches");
                            } else if (i >= 75) {
                                strcpy(tbuf, "small wounds");
                            } else if (i >= 50) {
                                strcpy(tbuf, "wounded");
                            } else if (i >= 30) {
                                strcpy(tbuf, "big nasty");
                            } else if (i >= 15) {
                                strcpy(tbuf, "badly wounded");
                            } else if (i >= 0) {
                                strcpy(tbuf, "awful");
                            } else {
                                strcpy(tbuf, "bleeding");
                            }
                        }
                    } else {
                        strcpy(tbuf, "*");
                    }

                    break;
                case 's':
                    s_flag = 1;
                case 'S':       /* affected spells */
                    *tbuf = 0;
                    if ((i = _affected_by_s(ch, SPELL_FIRESHIELD)) != -1) {
                        strcat(tbuf, (i > 1) ? "F" : "f");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_CHILLSHIELD)) != -1) {
                        strcat(tbuf, (i > 1) ? "C" : "c");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_BLADE_BARRIER)) != -1) {
                        strcat(tbuf, (i > 1) ? "B" : "b");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_SANCTUARY)) != -1) {
                        strcat(tbuf, (i > 1) ? "S" : "s");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_INVISIBLE)) != -1 || 
                        (i = _affected_by_s(ch, SPELL_GATHER_SHADOWS)) != -1) {
                        strcat(tbuf, (i > 1) ? "I" : "i");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_TRUE_SIGHT)) != -1) {
                        strcat(tbuf, (i > 1) ? "T" : "t");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_PROT_ENERGY_DRAIN)) != 
                            -1) {
                        strcat(tbuf, (i > 1) ? "D" : "d");
                    } else if ((i = _affected_by_s(ch, IMM_DRAIN)) != -1) {
                        strcat(tbuf, (i > 1) ? "R" : "r");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_ANTI_MAGIC_SHELL)) != 
                            -1) {
                        strcat(tbuf, (i > 1) ? "A" : "a");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    if ((i = _affected_by_s(ch, SPELL_FLY)) != -1) {
                        strcat(tbuf, (i > 1) ? "L" : "l");
                    } else if (s_flag) {
                        strcat(tbuf, "-");
                    }
                    break;
                case 't':
                    /* 
                     * Mud time 
                     */
                    sprintf(tbuf, "%d%s", 
                            ((time_info.hours % 12 == 0) ? 12 : 
                             ((time_info.hours) % 12)),
                            ((time_info.hours >= 12) ? "pm" : "am"));
                    break;
                case 'T':
                    /* 
                     * System Time 
                     */
#if 0
                    curr_time = time(NULL);
                    sprintf(tbuf,"%02d:%02d",
                    localtime(&curr_time)->tm_hour,
                            localtime(&curr_time)->tm_min);
#endif
                    break;
                case 'R':
                    /* 
                     * room number for immortals 
                     */
                    if (IS_IMMORTAL(ch)) {
                        rm = real_roomp(ch->in_room);
                        if (!rm) {
                            char_to_room(ch, 0);
                            rm = real_roomp(ch->in_room);
                        }
                        sprintf(tbuf, "%ld", rm->number);
                    } else {
                        *tbuf = 0;
                    }
                    break;
                case 'z':
                    /* 
                     * zone number for immortals 
                     */
                    if (IS_IMMORTAL(ch)) {
                        rm = real_roomp(ch->in_room);
                        if (!rm) {
                            char_to_room(ch, 0);
                            rm = real_roomp(ch->in_room);
                        }
                        sprintf(tbuf, "%ld", rm->zone);
                    } else {
                        *tbuf = 0;
                    }
                    break;
                case 'i':
                    /* 
                     * immortal stuff going 
                     */
                    pr_scan++;
                    if (!IS_IMMORTAL(ch)) {
                        *tbuf = 0;
                        break;
                    }
                    switch (*pr_scan) {
                    case 'I':
                        /* 
                         * invisible status 
                         */
                        sprintf(tbuf, "%d", ch->invis_level);
                        break;
                    case 'S':
                        /* 
                         * stealth mode 
                         */
                        strcpy(tbuf, IS_SET(ch->specials.act, PLR_STEALTH) ? 
                                     "On" : "Off");
                        break;
                    case 's':
                        rm = real_roomp(ch->in_room);
                        if( !rm ) {
                            strcpy( tbuf, "UNKNOWN" );
                        } else {
                            strcpy( tbuf, sectors[rm->sector_type].type );
                        }
                        break;
                    case 'F':
                        rm = real_roomp(ch->in_room);
                        if(!rm) {
                            strcpy(tbuf, "UNKNOWN");
                        } else {
                            sprintbit((unsigned long) rm->room_flags,
                                      room_bits, tbuf);
                        }
                        break;
                    case 'N':
                        /* 
                         * snoop name 
                         */
                        if (ch->desc->snoop.snooping) {
                            strcpy(tbuf, ch->desc->snoop.snooping->player.name);
                        } else {
                            *tbuf = 0;
                        }
                        break;
                    default:
#if 0
                        Log("Invalid Immmortal Prompt code '%c'", *pr_scan); 
#endif
                        *tbuf = 0;
                        break;
                    }
                    break;
                default:
#if 0
                     Log("Invalid Prompt code '%c'", *pr_scan);
#endif
                    *tbuf = 0;
                    break;
                }
            }
        } else {
            tbuf[0] = *pr_scan;
            tbuf[1] = 0;
        }
        strcat(outbuf, tbuf);
    }
}

void UpdateScreen(struct char_data *ch, int update)
{
    char            buf[255];
    int             size;

    size = ch->size;

    if (size <= 0) {
        return;
    }

    if (IS_SET(update, INFO_MANA)) {
        sprintf(buf, VT_CURSAVE);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 2, 7);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "          ");
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 2, 7);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURREST);
        write_to_descriptor(ch->desc->descriptor, buf);
    }

    if (IS_SET(update, INFO_MOVE)) {
        sprintf(buf, VT_CURSAVE);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 3, 58);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "          ");
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 3, 58);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURREST);
        write_to_descriptor(ch->desc->descriptor, buf);
    }

    if (IS_SET(update, INFO_HP)) {
        sprintf(buf, VT_CURSAVE);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 3, 13);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "          ");
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 3, 13);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURREST);
        write_to_descriptor(ch->desc->descriptor, buf);
    }

    if (IS_SET(update, INFO_GOLD)) {
        sprintf(buf, VT_CURSAVE);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 2, 47);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "                ");
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 2, 47);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "%d", GET_GOLD(ch));
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURREST);
        write_to_descriptor(ch->desc->descriptor, buf);
    }

    if (IS_SET(update, INFO_EXP)) {
        sprintf(buf, VT_CURSAVE);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 1, 20);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "                ");
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURSPOS, size - 1, 20);
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, "%d", GET_EXP(ch));
        write_to_descriptor(ch->desc->descriptor, buf);
        sprintf(buf, VT_CURREST);
        write_to_descriptor(ch->desc->descriptor, buf);
    }
}

void InitScreen(struct char_data *ch)
{
    int             size;

    size = ch->size;
    oldSendOutput(ch, VT_HOMECLR);
    oldSendOutput(ch, VT_MARGSET, 0, size - 5);
    oldSendOutput(ch, VT_CURSPOS, size - 4, 1);
    oldSendOutput(ch, "-========================================================="
                  "==================-");
    oldSendOutput(ch, VT_CURSPOS, size - 3, 1);
    oldSendOutput(ch, "Hit Points: ");
    oldSendOutput(ch, VT_CURSPOS, size - 3, 40);
    oldSendOutput(ch, "Movement Points: ");
    oldSendOutput(ch, VT_CURSPOS, size - 2, 1);
    oldSendOutput(ch, "Mana: ");
    oldSendOutput(ch, VT_CURSPOS, size - 2, 40);
    oldSendOutput(ch, "Gold: ");
    oldSendOutput(ch, VT_CURSPOS, size - 1, 1);
    oldSendOutput(ch, "Experience Points: ");

    ch->last.mana = GET_MANA(ch);
    ch->last.mmana = GET_MAX_MANA(ch);
    ch->last.hit = GET_HIT(ch);
    ch->last.mhit = GET_MAX_HIT(ch);
    ch->last.move = GET_MOVE(ch);
    ch->last.mmove = GET_MAX_MOVE(ch);
    ch->last.exp = GET_EXP(ch);
    ch->last.gold = GET_GOLD(ch);

    /*
     * Update all of the info parts 
     */
    oldSendOutput(ch, VT_CURSPOS, size - 3, 13);
    oldSendOutput(ch, "%d(%d)", GET_HIT(ch), GET_MAX_HIT(ch));
    oldSendOutput(ch, VT_CURSPOS, size - 3, 58);
    oldSendOutput(ch, "%d(%d)", GET_MOVE(ch), GET_MAX_MOVE(ch));
    oldSendOutput(ch, VT_CURSPOS, size - 2, 7);
    oldSendOutput(ch, "%d(%d)", GET_MANA(ch), GET_MAX_MANA(ch));
    oldSendOutput(ch, VT_CURSPOS, size - 2, 47);
    oldSendOutput(ch, "%d", GET_GOLD(ch));
    oldSendOutput(ch, VT_CURSPOS, size - 1, 20);
    oldSendOutput(ch, "%d", GET_EXP(ch));

    oldSendOutput(ch, VT_CURSPOS, 0, 0);
}

void identd_test(struct sockaddr_in in_addr)
{
    struct sockaddr_in addr;
    int             fd,
                    addrlen,
                    lport,
                    fport;
    FILE           *fp_in,
                   *fp_out;
    char            buf[8192],
                    reply_type[81],
                    opsys[81],
                    ident[1024];
    int             i;

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("identd socket");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = in_addr.sin_addr.s_addr;
    addr.sin_port = htons(113);
    addrlen = sizeof(addr);

    if (connect(fd, (struct sockaddr *) &addr, addrlen) == -1) {
        Log("identd server not responding, errno: %d\n", errno);
        return;
    }

    addrlen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr *) &addr, 
                    (socklen_t *)&addrlen) == -1) {
        perror("getsockname");
    }

    fp_in = fdopen(fd, "r");
    fp_out = fdopen(fd, "w");
    if (!fp_in || !fp_out) {
        perror("fdopen");
    }

    fprintf(fp_out, "%d , %d\n", ntohs(in_addr.sin_port), mud_port);
    fflush(fp_out);

    if (fgets(buf, sizeof(buf) - 1, fp_in) == NULL) {
        perror("fgets");
    }

    shutdown(fd, 1);

    i = sscanf(buf, "%d , %d : %[^ \t\n\r:] : %[^\t\n\r:] : %[^\n\r]",
               &lport, &fport, reply_type, opsys, ident);
    if (i < 3) {
        fprintf(stderr, "fscanf: too few arguments (%d)\n", i);
        return;
    }
    if (strcmp(reply_type, "ERROR") == 0) {
        Log("Ident error: error code: %s\n", opsys);
    } else if (strcmp(reply_type, "USERID") != 0) {
        Log("Ident error: illegal reply type: %s\n", reply_type);
    } else {
        Log("ident data -- system:%s user:%s\n", opsys, ident);
    }

    fclose(fp_out);
    fclose(fp_in);

    return;
}

#if 0
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
#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
