
#include "config.h"
#include <signal.h>
#include <stdio.h>
#include <sys/time.h>

#include "protos.h"

void            checkpointing(int signal);
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

    /*
     * set up the deadlock-protection 
     */

    /*
     * disabled for testing 
     */
#if 0
    interval.tv_sec = 900; 
    interval.tv_usec = 0; 
    itime.it_interval = interval; 
    itime.it_value = interval; 
    setitimer(ITIMER_VIRTUAL, &itime, 0); 
    signal(SIGVTALRM, checkpointing); 
#endif    
}

void checkpointing(int signal)
{
    extern int      tics;

#if 1
    if (!tics) {
        Log("CHECKPOINT shutdown: tics not updated");
        /*
         * abort(); temp removal for debug 
         */
    } else {
        tics = 0;
    }
#else
    if (!tics) {
        Log("CHECKPOINT tics not updated, setting to 1, be very wary");
        tics = 1;
    } else {
        tics = 0;
    }
#endif

}

void shutdown_request(int signal)
{
    extern int      mudshutdown;

    Log("Received USR2 - shutdown request");
    mudshutdown = 1;
}

/*
 * kick out players etc 
 */
void hupsig(int signal)
{
    int             i;
    extern int      mudshutdown,
                    reboot;

    Log("Received SIGHUP, SIGINT, or SIGTERM. Shutting down");

    raw_force_all("return");
    raw_force_all("save");

    for (i = 0; i < 30; i++) {
        SaveTheWorld();
    }

    mudshutdown = reboot = 1;
}

void logsig(int signal)
{
    Log("Signal received. Ignoring.");
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
