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
#include "structs.h"
#include "newstructs.h"

/* CVS generated ID string (optional for h files) */
static char interthread_h_ident[] _UNUSED_ = 
    "$Id$";

/*
 * Externals used for interthread communication
 */
extern QueueObject_t *ConnectInputQ;
extern QueueObject_t *ConnectDnsQ;
extern QueueObject_t *InputLoginQ;
extern QueueObject_t *InputEditorQ;
extern QueueObject_t *InputPlayerQ;
extern QueueObject_t *InputImmortQ;
extern QueueObject_t *LoggingQ;

extern BalancedBTree_t    *commandName;
extern BalancedBTree_t    *commandNum;

extern CommandDef_t coreCommands[];
extern int coreCommandCount;

extern bool GlobalAbort;
extern long SystemFlags;
extern long total_connections;
extern long total_max_players;
extern int no_specials;

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

extern char *mySQL_db;
extern char *mySQL_user;
extern char *mySQL_passwd;
extern char *mySQL_host;
extern int   mySQL_port;


/*
 * Prototypes of the thread entry points
 */
void *ConnectionThread( void *arg );
void *InputThread( void *arg );
void *LoginThread( void *arg );
void *EditorThread( void *arg );
void *DnsThread( void *arg );
void *LoggingThread( void *arg );
void *PlayingThread( void *arg );

/*
 * Prototypes of connections thread callbacks
 */
void connKickOutput( ConnectionItem_t *connItem );
void connClose( ConnectionItem_t *connItem );

/*
 * Other prototypes to move later
 */
void SendOutput( PlayerStruct_t *player, char *fmt, ... );
void SendOutputRaw( PlayerStruct_t *player, unsigned char *string, int len );
void LoginSendBanner( PlayerStruct_t *player );
void LoginStateMachine(PlayerStruct_t *player, char *arg);
void EditorStart( PlayerStruct_t *player, char **string, int maxlen );
void FlushQueue( QueueObject_t *queue, PlayerStruct_t *player );
PlayerStruct_t *FindCharacterNamed( char *name, PlayerStruct_t *oldPlayer );
int GetPlayerCount( void );
void JustLoggedIn( PlayerStruct_t *player );
void CommandParser( PlayerStruct_t *player, char *line );
void InitializeCommands( void );
void SetupCommands( CommandDef_t *commands, int count );
void AddCommand( CommandDef_t *cmd );

/*
 * TODO: move these!
 */
int_func procGetFuncByName( char *name, proc_type type );
char *procGetNameByFunc( int_func func, proc_type type );
int procIsRegistered( int_func func, proc_type type );


/* 
 * ansi_output.c 
 */
int ParseAnsiColors(bool UsingAnsi, char *txt, char *buf);
void ScreenOff( PlayerStruct_t *player );

/*
 * text_process.c
 */
char *skip_spaces(char *string);
int search_block(char *arg, char **list, bool exact);
void remove_cr(char *output, char *input);

/*
 * char_data.c
 */
int GetMaxLevel( struct char_data *ch );
int HasClass(struct char_data *ch, int clss);
int HowManyClasses( struct char_data *ch );
int pc_num_class(int clss);
char *AlignDesc(int value);
int number(int from, int to);
int dice(int number, int size);

/*
 * core_commands.c
 */
void            do_siteban(struct char_data *ch, char *argument, int cmd);

/*
 * mysql_handler.c
 */
void db_setup(void);
void db_initial_load(void);
void bind_numeric( MYSQL_BIND *data, long long int value, 
                   enum enum_field_types type );
void bind_string( MYSQL_BIND *data, char *value, enum enum_field_types type );
void bind_null_blob( MYSQL_BIND *data, void *value );
void db_queue_query( int queryId, QueryTable_t *queryTable,
                     MYSQL_BIND *queryData, int queryDataCount,
                     QueryResFunc_t queryCallback, void *queryCallbackArg,
                     pthread_mutex_t *queryMutex );

/* mysql_database.c */
void db_report_entry(int reportId, struct char_data *ch, char *report);
struct user_report *db_get_report(int reportId);
void db_clean_report(int reportId);

void db_load_textfiles(void);
int db_save_textfile(struct char_data *ch);
struct board_def *db_lookup_board(int vnum);
struct bulletin_board_message *db_get_board_message(int boardId, int msgNum);
void db_free_board_message(struct bulletin_board_message *msg);
int db_get_board_replies(struct board_def *board, int msgId, 
                         struct bulletin_board_message **msg);
void db_free_board_replies(struct bulletin_board_message *msg, int count);
void db_delete_board_message(struct board_def *board, short message_id);
void db_post_message(struct board_def *board, 
                     struct bulletin_board_message *msg);
void db_store_mail(char *to, char *from, char *message_pointer);
int             db_has_mail(char *recipient);
int   db_get_mail_ids(char *recipient, int *messageNum, int count);
char *db_get_mail_message(int messageId);
void db_delete_mail_message(int messageId);

char *db_lookup_help( int type, char *keywords );
char *db_lookup_help_similar( int type, char *keywords );
void db_save_object(struct obj_data *obj, int owner, int ownerItem );
struct obj_data *db_read_object(struct obj_data *obj, int vnum, int owner,
                                int ownerItem );
int db_find_object_named(char *string, int owner, int ownerItem);
struct index_data *db_generate_object_index(int *top, int *sort_top,
                                            int *alloc_top);


/*************************************************************************
 * Support for different platforms
 *************************************************************************/
#include "config.h"

#if defined( __CYGWIN__ )
/* Since stupid cygwin doesn't define this in the standard place */
char *crypt(const char *key, const char *salt);
#endif

#ifndef HAVE_STRNLEN 
/* FreeBSD and Solaris seem to be missing strnlen */
size_t strnlen(const char *s, size_t maxlen);
#endif

#ifndef HAVE_STRSEP
/* Solaris seems to be missing strsep */
char *strsep(char **stringp, const char *delim);
#endif

#ifndef HAVE_STRDUP
char           *strdup(const char *str)
#endif

#ifndef HAVE_STRSTR
char           *strstr(register const char *s, register const char *find)
#endif



#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
