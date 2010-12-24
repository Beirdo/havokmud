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

#ifndef _structs_h_
#define _structs_h_

#include "environment.h"
#include <pthread.h>
#include <signal.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"
#include "protected_data.h"
#include "logging.h"
#include "balanced_btree.h"
#define MAX_OBJ_AFFECT 5
#include "oldstructs.h"
#include <mysql.h>

/* CVS generated ID string (optional for h files) */
static char structs_h_ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief New structure definitions
 * @todo rename to structs.h after removing the old one
 */

#define BV(x) (1 << (x))

typedef struct {
    int port;
    int timeout_sec;
    int timeout_usec;
} connectThreadArgs_t;

typedef enum {
    STATE_INITIAL,               /**< Initial state, show login banner */
    STATE_GET_EMAIL,             /**< Get the user's email address */
    STATE_CONFIRM_EMAIL,         /**< Confirm a new user's email address */
    STATE_GET_NEW_USER_PASSWORD, /**< Get a password for the new user */
    STATE_CONFIRM_PASSWORD,      /**< Confirm the password for the new user */
    STATE_GET_PASSWORD,          /**< Get the password for user logging in */
    STATE_CHOOSE_ANSI,           /**< Choose whether or not to use ANSI color */
    STATE_SHOW_MOTD,             /**< Show the MOTD text */
    STATE_SHOW_WMOTD,            /**< Show the WMOTD text */
    STATE_SHOW_CREDITS,          /**< Show the Credits text */
    STATE_WIZLOCKED,             /**< User is locked out, disconnect them */
    STATE_PRESS_ENTER,           /**< Await the user hitting enter */
    STATE_SHOW_ACCOUNT_MENU,     /**< Show the account menu */
    STATE_SHOW_PLAYER_LIST,      /**< Show the PC list */
    STATE_GET_NEW_PASSWORD,      /**< Get a new password */
    STATE_CONFIRM_NEW_PASSWORD,  /**< Confirm the new password */
    STATE_ENTER_CONFIRM_CODE,    /**< Prompt for emailed confirmation code */
    STATE_RESEND_CONFIRM_EMAIL,  /**< Resend the emailed confirmation code */
    STATE_SHOW_CREATION_MENU,    /**< Show the PC creation menu */
    STATE_CHOOSE_NAME,           /**< Choose the name of the new PC */
    STATE_CHOOSE_SEX,            /**< Choose the sex of the new PC */
    STATE_CHOOSE_RACE,           /**< Choose the race of the new PC */
    STATE_CHOOSE_CLASS,          /**< Choose the class of the new PC */
    STATE_CHOOSE_STATS,          /**< Choose the stats of the new PC */
    STATE_CHOOSE_ALIGNMENT,      /**< Choose the alignment of the new PC */
    STATE_REROLL_ABILITIES,      /**< Reroll the PC's ability scores */
/* sorted to here */
    STATE_SHOW_LOGIN_MENU,
    STATE_WAIT_FOR_AUTH,
    STATE_EDIT_EXTRA_DESCR,
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

typedef struct {
    int                 id;
    char               *email;
    char               *pwd;
    char               *newpwd;
    bool                ansi;
    bool                confirmed;
    char               *confcode;
} PlayerAccount_t;

typedef struct _PlayerPC_t
{
    int                 id;
    int                 account_id;
    char               *name;
    BalancedBTree_t    *attribs;
    BalancedBTree_t    *sources;
} PlayerPC_t;

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

    PlayerAccount_t    *account;
    PlayerPC_t         *pcs;        /* Should be a btree? */
    PlayerPC_t         *pc;         /* PC being played */

    /* sorted to here */

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
    char                ipAddr[16];
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


#define MAX_SCHEMA_QUERY 100
typedef QueryTable_t SchemaUpgrade_t[MAX_SCHEMA_QUERY];


typedef void (*QueryResFunc_t)( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                                long insertid );

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
    ITEM_TYPE_LIGHT = 1,
    ITEM_TYPE_SCROLL,
    ITEM_TYPE_WAND,
    ITEM_TYPE_STAFF,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_FIREWEAPON,
    ITEM_TYPE_MISSILE,
    ITEM_TYPE_TREASURE,
    ITEM_TYPE_ARMOR,
    ITEM_TYPE_POTION,
    ITEM_TYPE_WORN,
    ITEM_TYPE_OTHER,
    ITEM_TYPE_TRASH,
    ITEM_TYPE_TRAP,
    ITEM_TYPE_CONTAINER,
    ITEM_TYPE_NOTE,
    ITEM_TYPE_DRINKCON,
    ITEM_TYPE_KEY,
    ITEM_TYPE_FOOD,
    ITEM_TYPE_MONEY,
    ITEM_TYPE_PEN,
    ITEM_TYPE_BOAT,
    ITEM_TYPE_AUDIO,
    ITEM_TYPE_BOARD,
    ITEM_TYPE_TREE,
    ITEM_TYPE_ROCK,
    ITEM_TYPE_PORTAL,
    ITEM_TYPE_INSTRUMENT,
    ITEM_TYPE_SHIPS_HELM
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
#define ITEM_WEAR_WAIST         BV(11)
#define ITEM_WEAR_WRIST         BV(12)
#define ITEM_WEAR_BACK          BV(13)
#define ITEM_WEAR_EAR           BV(14)
#define ITEM_WEAR_EYE           BV(15)
#define ITEM_LIGHT_SOURCE       BV(16)
#define ITEM_HOLD               BV(17)
#define ITEM_WIELD              BV(18)
#define ITEM_THROW              BV(19)

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
#define ITEM_BRITTLE            BV(8)
#define ITEM_RESISTANT          BV(9)
#define ITEM_IMMUNE             BV(10)
#define ITEM_RARE               BV(11)
#define ITEM_UBERRARE           BV(12)
#define ITEM_QUEST              BV(13)

/*
 * Bitvector for 'anti_flags'
 */
#define ITEM_ANTI_SUN           BV(0)
#define ITEM_ANTI_GOOD          BV(1)
#define ITEM_ANTI_EVIL          BV(2)
#define ITEM_ANTI_NEUTRAL       BV(3)
#define ITEM_ANTI_MEN           BV(4)
#define ITEM_ANTI_WOMEN         BV(5)


/*
 * for containers - value[1]
 */

#define CONT_CLOSEABLE      BV(0)
#define CONT_PICKPROOF      BV(1)
#define CONT_CLOSED         BV(2)
#define CONT_LOCKED         BV(3)


#define KEYWORD_FULL_MATCH      BV(0)
#define KEYWORD_PARTIAL_MATCH   BV(1)

typedef struct {
    char                  **words;
    int                    *length;
    int                    *found;
    char                   *description;
    int                     count;
    bool                    exact;
    bool                    partial;
} Keywords_t;

struct index_data {
    int             vnum;       /* virtual number of this mob/obj */
    int             number;     /* number of existing units of this mob/obj */
    int_func        func;       /* special procedure for this mob/obj */
    void           *data;
    Keywords_t      keywords;
    char           *short_desc;
    char           *long_desc;
    int             MaxCount;
    LinkedList_t   *list;       /**< Linked List of all loaded objects/mobiles
                                 *   of this vnum */
};

/*
 * ======================== Structure for object ========================
 */
struct obj_data {
    LinkedListItem_t    globalLink; /**< Linked List linkage for 
                                     *   index_data::list -- the global
                                     *   list of items (one list per item
                                     *   number)
                                     */
    LinkedListItem_t    containLink; /**< Linked List linkage for 
                                      *   obj_data::containList.  This linkage 
                                      *   is all of the items contained in the 
                                      *   same object
                                      */
    LinkedListItem_t    contentLink; /**< Linked List linkage for
                                      *   room_data::contentList.  This linkage
                                      *   is all of the items contained in the
                                      *   same room
                                      */

    struct index_data *index;       /**< pointer back to the index entry to
                                     *   save calls to objectIndex
                                     */
    BalancedBTreeItem_t typeItem;   /**< The btree item in the objectTypeTree
                                     */
    BalancedBTreeItem_t *keywordItem; /**< The btree items in the 
                                       *   objectKeywordTree (one per keyword
                                       *   entry)
                                       */
    BalancedBTreeItem_t *containItem; /**< The btree items in the 
                                       *   obj_data::containKeywordTree (one per
                                       *   keyword entry) if this item is
                                       *   contained in another object
                                       */
    BalancedBTreeItem_t *contentItem; /**< The btree items in the 
                                       *   room_data::contentKeywordTree (one 
                                       *   per keyword entry) if this item is
                                       *   in a room
                                       */

    BalancedBTree_t    *containKeywordTree; /**< The double binary tree of all 
                                             *   items contained in this one 
                                             *   (same structure as the 
                                             *   objectKeywordTree)
                                             */
    LinkedList_t       *containList;        /**< Linked list of all items
                                             *   contained in this one.  The
                                             *   linkage is at 
                                             *   obj_data::containLink
                                             */

    int             item_number;    /**< Where in database */
    int             in_room;        /**< In what room, -1 when contained
                                     *   or carried */

    int             value[4];       /**< Values of the item (see list) */
    ItemType_t      type_flag;      /**< Type of item */

    long            wear_flags;     /**< Where you can wear it */
    long            extra_flags;    /**< If it hums, glows etc */
    long            anti_flags;     /**< Which things the item is anti */

    int             weight;         /**< Weight of the item */
    int             cost;           /**< Value when sold (gp.) */
    int             cost_per_day;   /**< Cost to keep per real day */
    int             timer;          /**< Timer for object */

    long            bitvector;      /**< To set chars bits */

    int             sector;             /**< for large rooms */
    int             char_vnum;          /**< for ressurection */
    long            char_f_pos;         /**< for ressurection */

    Keywords_t      keywords;           /**< Keywords used by get, etc. */
    char           *description;        /**< When in room */
    char           *short_description;  /**< when worn/carry/in cont.  */
    char           *action_description; /**< What to write when used */

    Keywords_t     *ex_description;     /**< extra descriptions */
    int             ex_description_count;   /**< count of extra descriptions */

    struct char_data *carried_by;       /**< Carried by whom?  NULL in 
                                         *   room/containers 
                                         */
    int             eq_pos;             /**< what is the equip. pos? */
    struct char_data *equipped_by;      /**< equipped by whom?  NULL in 
                                         *   room/containers
                                         */
    struct obj_data *in_obj;            /**< In what object NULL when none */

#if 1  /* These will be disappearing very shortly */
    struct obj_data *next_content;      /**< For carrying and room contents 
                                         *   lists 
                                         */
#endif

    int             level;              /**< Level ego of the item */
    int             max;                /**< max of the object */
    int             speed;              /**< Speed of the weapon */
    int             weapontype;         /**< Weapon type */
    int             tweak;              /**< Tweak value */

    char           *modBy;              /**< Last modified by */
    time_t          modified;           /**< Last modification time */
};

#define KEYWORD_ITEM_OFFSET (OFFSETOF(keywordItem, struct obj_data))
#define CONTAIN_ITEM_OFFSET (OFFSETOF(containItem, struct obj_data))
#define CONTENT_ITEM_OFFSET (OFFSETOF(contentItem, struct obj_data))

#define GLOBAL_LINK_OFFSET  (OFFSETOF(globalLink, struct obj_data))
#define GLOBAL_LINK_TO_OBJ(x)   \
    ((struct obj_data *)PTR_AT_OFFSET(-GLOBAL_LINK_OFFSET,(x)))

#define CONTAIN_LINK_OFFSET  (OFFSETOF(containLink, struct obj_data))
#define CONTAIN_LINK_TO_OBJ(x)  \
    ((struct obj_data *)PTR_AT_OFFSET(-CONTAIN_LINK_OFFSET,(x)))

#define CONTENT_LINK_OFFSET  (OFFSETOF(contentLink, struct obj_data))
#define CONTENT_LINK_TO_OBJ(x)  \
    ((struct obj_data *)PTR_AT_OFFSET(-CONTENT_LINK_OFFSET,(x)))

/*
 * The following defs are for room_data
 */

#define NOWHERE    -1           /* nil reference for room-database */
#define AUTO_RENT  -2           /* other special room, for auto-renting */

/*
 * Bitvector For 'room_flags'
 */

#define DARK                    BV(0)
#define DEATH                   BV(1)
#define NO_MOB                  BV(2)
#define INDOORS                 BV(3)
#define PEACEFUL                BV(4)
#define NOSTEAL                 BV(5)
#define NO_SUM                  BV(6)
#define NO_MAGIC                BV(7)
#define TUNNEL                  BV(8)
#define PRIVATE                 BV(9)
#define SILENCE                 BV(10)
#define LARGE                   BV(11)
#define NO_DEATH                BV(12)
#define SAVE_ROOM               BV(13)
#define NO_FLY                  BV(15)
#define REGEN_ROOM              BV(16)
#define FIRE_ROOM               BV(17)
#define ICE_ROOM                BV(18)
#define WIND_ROOM               BV(19)
#define EARTH_ROOM              BV(20)
#define ELECTRIC_ROOM           BV(21)
#define WATER_ROOM              BV(22)
#define MOVE_ROOM               BV(23)
#define MANA_ROOM               BV(24)
#define NO_FLEE                 BV(25)
#define NO_SPY                  BV(26)
#define EVER_LIGHT              BV(27)
#define ROOM_WILDERNESS         BV(28)

/*
 * For 'dir_option'
 */

typedef enum {
    NORTH = 0,
    EAST,
    SOUTH,
    WEST,
    UP,
    DOWN
} Directions_t;

#define EX_ISDOOR       BV(0)
#define EX_CLOSED       BV(1)
#define EX_LOCKED       BV(2)
#define EX_SECRET       BV(3)
#define EX_RSLOCKED     BV(4)
#define EX_PICKPROOF    BV(5)
#define EX_CLIMB        BV(6)


#define TELE_LOOK       BV(0)
#define TELE_COUNT      BV(1)
#define TELE_RANDOM     BV(2)
#define TELE_SPIN       BV(3)

struct room_direction_data {
    char           *general_description;        /* When look DIR.  */
    Keywords_t     *keywords;   /* for open/close */

    long            exit_info;  /* Exit info */
    long            key;        /* Key's number (-1 for no key) */
    long            to_room;    /* Where direction leeds (NOWHERE) */
    long            open_cmd;   /* cmd needed to OPEN/CLOSE door */
};

/*
 * ========================= Structure for room ==========================
 */
struct room_data {
    LinkedList_t       *contentList;
    BalancedBTree_t    *contentKeywordTree;

    int             number;     /* Rooms number */
    int             zone;       /* Room zone (for resetting) */
    int             continent;  /* Which continent/mega-zone */

    int             river_dir;  /* dir of flow on river */
    int             river_speed;        /* speed of flow on river */

    int             tele_time;  /* time to a teleport */
    int             tele_targ;  /* target room of a teleport */
    long            tele_mask;  /* flags for use with teleport */
    int             tele_cnt;   /* countdown teleports */

    int             moblim;     /* # of mobs allowed in room.  */

    char           *name;       /* Rooms name 'You are ...' */
    char           *description;        /* Shown when entered */
    Keywords_t     *ex_description;     /* for examine/look */
    int             ex_description_count;
    struct room_direction_data *dir_option[6];  /* Directions */
    long            room_flags; /* DEATH,DARK ... etc */
    int             light;      /* Number of lightsources in room */
    int             dark;
    int             (*func) ();        /* special procedure */

#if 0
    struct obj_data *contents;  /* List of items in room */
#endif
    struct char_data *people;   /* List of NPC / PC in room */

    int             special;
};

/*
 * for common mobile procedures: specials.proc
 */
typedef enum {
    PROC_NONE = 0,      /* no proc */
    PROC_SHOPKEEPER,    /* int shopkeeper() behaviour */
    PROC_GUILDMASTER,   /* GM, dependant on class in MobAct & level */
    PROC_SWALLOWER,     /* Swallows when not bashed */
    PROC_DRAIN,         /* Drains when not bashed */
    PROC_QUEST,         /* it's a give/receive mob */
    PROC_OLD_BREATH,
    PROC_FIRE_BREATH,
    PROC_GAS_BREATH,
    PROC_FROST_BREATH,
    PROC_ACID_BREATH,
    PROC_LIGHTNING_BREATH,
    PROC_DEHYDRATION_BREATH,
    PROC_VAPOR_BREATH,
    PROC_SOUND_BREATH,
    PROC_SHARD_BREATH,
    PROC_SLEEP_BREATH,
    PROC_LIGHT_BREATH,
    PROC_DARK_BREATH,
    PROC_RECEPTIONIST,
    PROC_REPAIRGUY
} CommonProcTypes_t;

typedef void (*SigFunc_t)( int, void * ); 
 
typedef struct { 
    SigFunc_t               sighupFunc; 
    void                   *sighupArg; 
    SigFunc_t               sigusr2Func;
    void                   *sigusr2Arg;
} ThreadCallback_t; 

typedef struct {
    PlayerStruct_t         *player;
    char                   *subject;
    char                   *body;
    int                     bodyind;
} MailItem_t;

typedef enum {
    LT_CONSOLE,
    LT_FILE,
    LT_SYSLOG,
    LT_NCURSES
} LogFileType_t;

typedef struct {
    LinkedListItem_t    linkage;
    int                 fd;
    LogFileType_t       type;
    bool                aborted;
    union {
        char           *filename;
    } identifier;
} LogFileChain_t;

typedef struct {
    char       *what;
    char       *version;
    int         count;
} Version_t;


typedef struct {
    char *source;
    char *json;
} JSONSource_t;

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
