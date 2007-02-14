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


#define BV(x) (1 << (x))

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

/**
 * @todo clean up these definitions
 */

/*
 * The following defs are for obj_data
 */

/*
 * For 'type_flag'
 */
typedef enum {
    ITEM_LIGHT = 1,
    ITEM_SCROLL,
    ITEM_WAND,
    ITEM_STAFF,
    ITEM_WEAPON,
    ITEM_FIREWEAPON,
    ITEM_MISSILE,
    ITEM_TREASURE,
    ITEM_ARMOR,
    ITEM_POTION,
    ITEM_WORN,
    ITEM_OTHER,
    ITEM_TRASH,
    ITEM_TRAP,
    ITEM_CONTAINER,
    ITEM_NOTE,
    ITEM_DRINKCON,
    ITEM_KEY,
    ITEM_FOOD,
    ITEM_MONEY,
    ITEM_PEN,
    ITEM_BOAT,
    ITEM_AUDIO,
    ITEM_BOARD,
    ITEM_TREE,
    ITEM_ROCK,
    ITEM_PORTAL,
    ITEM_INSTRUMENT,
    ITEM_SHIPS_HELM
} ItemType_t;

/*
 * Bitvector For 'wear_flags'
 */

#define ITEM_TAKE               BV(0)
#define ITEM_WEAR_FINGER        BV(1)
#define ITEM_WEAR_NECK          BV(2)
#define ITEM_WEAR_BODY          BV(3)
#define ITEM_WEAR_HEAD          BV(4)
#define ITEM_WEAR_LEGS          BV(5)
#define ITEM_WEAR_FEET          BV(6)
#define ITEM_WEAR_HANDS         BV(7)
#define ITEM_WEAR_ARMS          BV(8)
#define ITEM_WEAR_SHIELD        BV(9)
#define ITEM_WEAR_ABOUT         BV(10)
#define ITEM_WEAR_WAISTE        BV(11)
#define ITEM_WEAR_WRIST         BV(12)
#define ITEM_WIELD              BV(13)
#define ITEM_HOLD               BV(14)
#define ITEM_THROW              BV(15)
#define ITEM_LIGHT_SOURCE       BV(16)
#define ITEM_WEAR_BACK          BV(17)
#define ITEM_WEAR_EAR           BV(18)
#define ITEM_WEAR_EYE           BV(19)

/*
 * Bitvector for 'extra_flags'
 */
#define ITEM_GLOW               BV(0)
#define ITEM_HUM                BV(1)
#define ITEM_METAL              BV(2)
#define ITEM_MINERAL            BV(3)
#define ITEM_ORGANIC            BV(4)
#define ITEM_INVISIBLE          BV(5)
#define ITEM_MAGIC              BV(6)
#define ITEM_NODROP             BV(7)
#define ITEM_ANTI_NECROMANCER   BV(8)
#define ITEM_ANTI_GOOD          BV(9)
#define ITEM_ANTI_EVIL          BV(10)
#define ITEM_ANTI_NEUTRAL       BV(11)
#define ITEM_ANTI_CLERIC        BV(12)
#define ITEM_ANTI_MAGE          BV(13)
#define ITEM_ANTI_THIEF         BV(14)
#define ITEM_ANTI_FIGHTER       BV(15)
#define ITEM_BRITTLE            BV(16)
#define ITEM_RESISTANT          BV(17)
#define ITEM_IMMUNE             BV(18)
#define ITEM_ANTI_MEN           BV(19)
#define ITEM_ANTI_WOMEN         BV(20)
#define ITEM_ANTI_SUN           BV(21)
#define ITEM_ANTI_BARBARIAN     BV(22)
#define ITEM_ANTI_RANGER        BV(23)
#define ITEM_ANTI_PALADIN       BV(24)
#define ITEM_ANTI_PSI           BV(25)
#define ITEM_ANTI_MONK          BV(26)
#define ITEM_ANTI_DRUID         BV(27)
#define ITEM_ONLY_CLASS         BV(28)
#define ITEM_UNUSED             BV(29)
#define ITEM_RARE               BV(30)
#define ITEM_QUEST              BV(31)

/*
 * Some different kind of liquids
 */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_COKE       15

/*
 * special addition for drinks
 */
#define DRINK_POISON  BV(0)
#define DRINK_PERM    BV(1)

/*
 * for containers - value[1]
 */

#define CONT_CLOSEABLE      BV(0)
#define CONT_PICKPROOF      BV(1)
#define CONT_CLOSED         BV(2)
#define CONT_LOCKED         BV(3)

struct extra_descr_data {
    char           *keyword;    /* Keyword in look/examine */
    char           *description;        /* What to see */
    struct extra_descr_data *next;      /* Next in list */
};

#define MAX_OBJ_AFFECT 5        /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */

struct obj_flag_data {
    int             value[4];       /* Values of the item (see list) */
    ItemType_t      type_flag;      /* Type of item */
    long            wear_flags;     /* Where you can wear it */
    long            extra_flags;    /* If it hums,glows etc */
    int             weight;         /* Weight what else */
    int             cost;           /* Value when sold (gp.) */
    int             cost_per_day;   /* Cost to keep pr. real day */
    int             timer;          /* Timer for object */
    long            bitvector;      /* To set chars bits */
};

struct obj_affected_type {
    short           location;   /* Which ability to change (APPLY_XXX) */
    long            modifier;   /* How much it changes by */
};

/*
 * ======================== Structure for object ========================
 */
struct obj_data {
    int             item_number;        /* Where in data-base */
    int             in_room;            /* In what room -1 when conta/carr */
    struct obj_flag_data obj_flags;     /* Object information */
    struct obj_affected_type
                    affected[MAX_OBJ_AFFECT];   /* Which abilities in PC
                                                 * to change */

    sh_int          sector;             /* for large rooms */
    int             char_vnum;          /* for ressurection */
    long            char_f_pos;         /* for ressurection */
    char           *name;               /* Title of object :get etc.  */
    char           *description;        /* When in room */
    char           *short_description;  /* when worn/carry/in cont.  */
    char           *action_description; /* What to write when used */
    struct extra_descr_data *ex_description;    /* extra descriptions */
    struct char_data *carried_by;       /* Carried by :NULL in room/conta */
    byte            eq_pos;             /* what is the equip. pos? */
    struct char_data *equipped_by;      /* equipped by :NULL in room/conta */
    struct obj_data *in_obj;            /* In what object NULL when none */
    struct obj_data *contains;          /* Contains objects */
    struct obj_data *next_content;      /* For 'contains' lists */
    struct obj_data *next;              /* For the object list */
    char           *old_name;           /* For Behead */
    int             is_corpse;          /* For Behead */
    int             beheaded_corpse;    /* For Behead */
    int             level;              /* Level ego of the item */
    int             max;                /* max of the object */
    int             speed;              /* Speed of the weapon */
    int             weapontype;
    int             tweak;

    char           *modBy;
    time_t          modified;
};


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
