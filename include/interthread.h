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
*--------------------------------------------------------*/

/**
 * @file
 * @brief Prototypes and definitions for inter-thread communication
 */


#ifndef interthread_h_
#define interthread_h_

#include "environment.h"
#include <pthread.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"
#include "protected_data.h"
#include "logging.h"
#include "balanced_btree.h"
#include "oldstructs.h"
#include "structs.h"

/* CVS generated ID string (optional for h files) */
static char interthread_h_ident[] _UNUSED_ = 
    "$Id$";

/*
 * Externals used for interthread communication
 */

extern LinkedList_t *ConnectionList;
extern QueueObject_t *ConnectInputQ;
extern QueueObject_t *ConnectDnsQ;
extern QueueObject_t *InputLoginQ;
extern QueueObject_t *InputEditorQ;
extern QueueObject_t *InputPlayerQ;
extern QueueObject_t *InputImmortQ;
extern QueueObject_t *LoggingQ;
extern QueueObject_t *MailQ;
extern QueueObject_t *QueryQ;
extern QueueObject_t *ProtobufQ;

extern BalancedBTree_t    *commandName;
extern BalancedBTree_t    *commandNum;

extern CommandDef_t coreCommands[];
extern int coreCommandCount;

extern bool MudDone;
extern bool GlobalAbort;
extern long SystemFlags;
extern long total_connections;
extern long total_max_players;
extern bool no_specials;
extern bool mudshutdown;
extern bool reboot_now;
extern bool Daemon;
extern bool Debug;
extern bool verbose;
extern int  mud_port;


extern char *login;
extern char *credits;
extern char *motd;
extern char *wmotd;
extern char *news;
extern char *info;

extern BalancedBTree_t    *banHostTree;
extern BalancedBTree_t    *descNameTree;
extern BalancedBTree_t    *descNumTree;

extern struct sector_data *sectors;
extern int             sectorCount;

extern struct message_list *fightMessages;
extern int fightMessageCount;

extern struct banned_user *bannedUsers;
extern int             bannedUserCount;

extern struct social_messg *socialMessages;
extern int                 socialMessageCount;

extern struct message_list  *kickMessages;
extern int                 kickMessageCount;

extern struct pose_type   *poseMessages;
extern int                 poseMessageCount;

extern struct spec_proc    specProcs[];
extern int                 specProcCount;

extern struct race_type   *races;
extern int                 raceCount;

#define DEF_MYSQL_DB     "havokdevel"
#define DEF_MYSQL_USER   "havokmud"
#define DEF_MYSQL_PASSWD "havokmud"
#define DEF_MYSQL_HOST   "localhost"
#define DEF_MYSQL_PORT   3306

#define DEF_MUD_PORT     4000          /**< default MUD TCP port */

extern char *mySQL_db;
extern char *mySQL_user;
extern char *mySQL_passwd;
extern char *mySQL_host;
extern int   mySQL_port;

extern struct time_info_data time_info;
extern struct weather_data weather_info;
extern struct room_data *room_db[];

extern BalancedBTree_t *objectKeywordTree;
extern BalancedBTree_t *objectTypeTree;

/*
 * Prototypes of the thread entry points
 */
extern pthread_mutex_t *startupMutex;

void *ConnectionThread( void *arg );
void *InputThread( void *arg );
void *LoginThread( void *arg );
void *EditorThread( void *arg );
void *DnsThread( void *arg );
void *LoggingThread( void *arg );
void *PlayingThread( void *arg );
void *MysqlThread( void *arg );
void *SmtpThread( void *arg );
void *ProtobufThread( void *arg );
void *WebServiceThread( void *arg );

/*
 * Prototypes of connections thread callbacks
 */
void connKickOutput( ConnectionItem_t *connItem );
void connClose( ConnectionItem_t *connItem, Locked_t locked );


/*
 * From mysql_upgrade.c
 */
void db_check_schema_main( void );

/*
 * From thread_api.c
 */
int thread_mutex_init( pthread_mutex_t *mutex );
void thread_create( pthread_t *pthreadId, void * (*routine)(void *),  
                    void *arg, char *name, ThreadCallback_t *callbacks ); 
void thread_register( pthread_t *pthreadId, char *name,  
                      ThreadCallback_t *callbacks ); 
char *thread_name( pthread_t pthreadId ); 
void thread_colors( pthread_t pthreadId, char **bg, char **fg );
void thread_deregister( pthread_t pthreadId ); 
void ThreadAllKill( int signum ); 
SigFunc_t ThreadGetHandler( pthread_t threadId, int signum, void **parg ); 


/*
 * From smtp.c
 */
void send_email( PlayerStruct_t *player, char *subject, char *body );

/*
 * From master.c
 */
void LogBanner( void );

/*
 * From logging.c
 */
void LogFlushOutput( void );

/*
 * From signals.c
 */
void do_backtrace( int signum, void *ip );

/*
 * From memory.c
 */
void *MemoryCoalesceThread( void *arg );
void memoryStats( int signum, void *ip );


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
