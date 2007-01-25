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

/* CVS generated ID string (optional for h files) */
static char interthread_h_ident[] _UNUSED_ = 
    "$Id$";

typedef struct {
    int port;
    int timeout_sec;
    int timeout_usec;
} connectThreadArgs_t;

typedef enum {
    STATE_INITIAL,
    STATE_CHOOSE_SEX,
    STATE_CHOOSE_ANSI,
    STATE_CHOOSE_RACE,
    STATE_CHOOSE_CLASS,
    STATE_CHOOSE_MAIN_CLASS,
    STATE_CHOOSE_STATS,
    STATE_CHOOSE_ALIGNMENT,
    STATE_SHOW_MOTD,
    STATE_SHOW_CREATION_MENU,
    STATE_SHOW_LOGIN_MENU,
    STATE_GET_PASSWORD,
    STATE_CONFIRM_PASSWORD,
    STATE_CONFIRM_NAME,
    STATE_GET_NEW_USER_PASSWORD,
    STATE_GET_NEW_PASSWORD,
    STATE_CONFIRM_NEW_PASSWORD,
    STATE_GET_NAME,
    STATE_REROLL,
    STATE_CHECK_MAGE_TYPE,
    STATE_WAIT_FOR_AUTH,
    STATE_WIZLOCKED,
    STATE_SHOW_WMOTD,
    STATE_EDIT_EXTRA_DESCR,
    STATE_PRESS_ENTER,
    STATE_DELETE_USER,
    STATE_PLAYING
} PlayerState_t;

typedef struct
{
    char               *buf;
    int                 len;
} OutputBuffer_t;


struct _PlayerStruct_t;

typedef struct 
{
    LinkedListItem_t        link;
    int                     fd;
    BufferObject_t         *buffer;
    struct _PlayerStruct_t *player;
    OutputBuffer_t         *outBufDesc;
    ProtectedData_t        *hostName;
} ConnectionItem_t;

typedef struct _PlayerStruct_t
{
    LinkedListItem_t    link;
    ConnectionItem_t   *connection;
    int                 flush;

    PlayerState_t       state;
    QueueObject_t      *handlingQ;

    BufferObject_t     *in_buffer;
    char               *in_remain;
    int                 in_remain_len;

    QueueObject_t      *outputQ;

    struct char_data   *charData;
    struct char_data   *originalData;

    char              **editString;
    int                 editStringLen;
    QueueObject_t      *editOldHandlingQ;

    char                stat[MAX_STAT];
    int                 prompt_mode;
} PlayerStruct_t;

typedef enum
{
    CONN_NEW_CONNECT,
    CONN_INPUT_AVAIL,
    CONN_FLUSH_INPUT,
    CONN_DELETE_CONNECT
} ConnInputType_t;

typedef struct
{
    ConnInputType_t     type;
    PlayerStruct_t     *player;
} ConnInputItem_t;

typedef enum
{
    INPUT_INITIAL,
    INPUT_AVAIL
} InputStateType_t;

typedef struct
{
    InputStateType_t    type;
    PlayerStruct_t     *player;
    char               *line;
} InputStateItem_t;

typedef struct
{
    uint32              ipAddr;
    ConnectionItem_t   *connection;
} ConnDnsItem_t;

typedef struct
{
    LogLevel_t          level;
    pthread_t           threadId;
    char               *file;
    int                 line;
    char               *function;
    uint32              time_sec;
    uint32              time_usec;
    char               *message;
} LoggingItem_t;

typedef struct
{
    char               *name;
    QueueObject_t      *inputQ;
} PlayingThreadArgs_t;

typedef struct
{
    char               *name;
    void              (*func)(struct char_data *, char *, int);
#ifdef TODO
    void              (*func)(PlayerStruct_t *, char *, int);
#endif
    int                 number;
    int                 min_pos;
    int                 min_level;
} CommandDef_t;


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
char *ParseAnsiColors(int UsingAnsi, char *txt);
char *skip_spaces(char *string);

/*
 * char_data.c
 */
int GetMaxLevel( struct char_data *ch );
int HasClass(struct char_data *ch, int class);
int HowManyClasses( struct char_data *ch );
char *AlignDesc(int value);
int number(int from, int to);
int dice(int number, int size);


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
