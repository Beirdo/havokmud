/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
* Copyright 2007 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

#ifndef newstructs_h_
#define newstructs_h_

#include "environment.h"
#include <pthread.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"
#include "protected_data.h"
#include "logging.h"
#include "balanced_btree.h"
#include "structs.h"
#include <mysql.h>

/* CVS generated ID string (optional for h files) */
static char newstructs_h_ident[] _UNUSED_ = 
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

#if ( MYSQL_VERSION_ID < 40102 ) 

#define NO_PREPARED_STATEMENTS

/*
 * Adapted libmysqlclient14 (4.1.x) mysql_com.h
 */
#ifndef MYSQL_TYPE_DECIMAL

#define MYSQL_TYPE_DECIMAL     FIELD_TYPE_DECIMAL
#define MYSQL_TYPE_TINY        FIELD_TYPE_TINY
#define MYSQL_TYPE_SHORT       FIELD_TYPE_SHORT
#define MYSQL_TYPE_LONG        FIELD_TYPE_LONG
#define MYSQL_TYPE_FLOAT       FIELD_TYPE_FLOAT
#define MYSQL_TYPE_DOUBLE      FIELD_TYPE_DOUBLE
#define MYSQL_TYPE_NULL        FIELD_TYPE_NULL
#define MYSQL_TYPE_TIMESTAMP   FIELD_TYPE_TIMESTAMP
#define MYSQL_TYPE_LONGLONG    FIELD_TYPE_LONGLONG
#define MYSQL_TYPE_INT24       FIELD_TYPE_INT24
#define MYSQL_TYPE_DATE        FIELD_TYPE_DATE
#define MYSQL_TYPE_TIME        FIELD_TYPE_TIME
#define MYSQL_TYPE_DATETIME    FIELD_TYPE_DATETIME
#define MYSQL_TYPE_YEAR        FIELD_TYPE_YEAR
#define MYSQL_TYPE_NEWDATE     FIELD_TYPE_NEWDATE
#define MYSQL_TYPE_ENUM        FIELD_TYPE_ENUM
#define MYSQL_TYPE_SET         FIELD_TYPE_SET
#define MYSQL_TYPE_TINY_BLOB   FIELD_TYPE_TINY_BLOB
#define MYSQL_TYPE_MEDIUM_BLOB FIELD_TYPE_MEDIUM_BLOB
#define MYSQL_TYPE_LONG_BLOB   FIELD_TYPE_LONG_BLOB
#define MYSQL_TYPE_BLOB        FIELD_TYPE_BLOB
#define MYSQL_TYPE_VAR_STRING  FIELD_TYPE_VAR_STRING
#define MYSQL_TYPE_STRING      FIELD_TYPE_STRING
#define MYSQL_TYPE_CHAR        FIELD_TYPE_TINY
#define MYSQL_TYPE_INTERVAL    FIELD_TYPE_ENUM
#define MYSQL_TYPE_GEOMETRY    FIELD_TYPE_GEOMETRY

#endif

/*
 * From libmysqlclient14 (4.1.x) mysql.h
 */
typedef struct st_mysql_bind
{
  unsigned long *length;          /* output length pointer */
  my_bool       *is_null;         /* Pointer to null indicator */
  void          *buffer;          /* buffer to get/put data */
  enum enum_field_types buffer_type;    /* buffer type */
  unsigned long buffer_length;    /* buffer length, must be set for str/binary */  
} MYSQL_BIND;

#define MYSQL_STMT void

#endif

struct _QueryItem_t;
typedef void (*QueryChainFunc_t)( MYSQL_RES *res, struct _QueryItem_t *item ); 

typedef struct {
    const char         *queryPattern;
    QueryChainFunc_t    queryChainFunc;
    MYSQL_STMT         *queryStatement;
    bool                queryPrepared;
} QueryTable_t;


typedef void (*QueryResFunc_t)( MYSQL_RES *res, MYSQL_BIND *input, void *arg );

typedef struct _QueryItem_t {
    int                 queryId;
    QueryTable_t       *queryTable;
    MYSQL_BIND         *queryData;
    int                 queryDataCount;
    QueryResFunc_t      queryCallback;
    void               *queryCallbackArg;
    pthread_mutex_t    *queryMutex;
    unsigned int        querySequence;
} QueryItem_t;


typedef enum {
    PROC_UNDEF = 0,
    PROC_MOBILE,
    PROC_OBJECT,
    PROC_ROOM
} proc_type;

typedef int (*int_func)();

struct spec_proc {
    char           *name;
    int_func        func;
    proc_type       type;
};

struct special_proc_entry {
    int             vnum;
    int_func        func;
};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
