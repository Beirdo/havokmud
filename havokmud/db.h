#ifndef _db_h
#define _db_h

/*
 * data files used by the game system 
 */

#ifndef RLIMIT_OFILE
#define RLIMIT_OFILE      99999
#endif
#define DFLT_DIR          "../lib"      /* default data directory */

#define WORLD_FILE        "tinyworld.wld"       /* room definitions */

#define MOB_FILE          "tinyworld.mob.new"   /* monster prototypes */

#define OBJ_FILE          "tinyworld.obj"       /* object prototypes */
#define ZONE_FILE         "tinyworld.zon"       /* zone defs & command
                                                 * tables */
#define CREDITS_FILE      "credits"     /* for the 'credits' command */
#define NEWS_FILE         "news"        /* for the 'news' command */
#define MOTD_FILE         "motd"        /* messages of today */
#define WMOTD_FILE        "wizmotd"     /* wizard's motd */
#define PLAYER_FILE       "players"     /* the player database */
#define TIME_FILE         "time"        /* game calendar information */
#define IDEA_FILE         "ideas"       /* for the 'idea'-command */
#define TYPO_FILE         "typos"       /* 'typo' */
#define BUG_FILE          "bugs"        /* 'bug' */
#define WIZBUG_FILE       "wizbug"      /* 'bug' for wizreport - MW */
#define WIZIDEA_FILE      "wizidea"     /* 'idea' for wizreport - MW */
#define WIZTYPO_FILE      "wiztypo"     /* 'typo' for wizreport - MW */
#define MESS_FILE         "messages"    /* damage message */
#define SOCMESS_FILE      "actions"     /* messgs for social acts */
#define HELP_KWRD_FILE    "help_table"  /* for HELP <keywrd> */
#define HELP_PAGE_FILE    "help"        /* for HELP <CR> */
#define WIZ_HELP_FILE     "wizhelp_table"       /* For wizhelp <keyword> */
#define INFO_FILE         "info"        /* for INFO */
#define WIZLIST_FILE      "wizlist"     /* for WIZLIST */

#define IWIZLIST_FILE      "iwizlist"   /* for WIZLIST */

#define POSEMESS_FILE     "poses"       /* for 'pose'-command */
#define MAIL_FILE         "mud_mail"    /* */

#define REAL 0
#define VIRTUAL 1

/*
 * structure for the reset commands 
 */
struct reset_com {
    char            command;    /* current command */
    bool            if_flag;    /* if TRUE: exe only if preceding exe'd */
    int             arg1;       /* */
    int             arg2;       /* Arguments to the command */
    int             arg3;       /* */

    /*
     *  Commands:              *
     *  'M': Read a mobile     *
     *  'O': Read an object    *
     *  'G': Give obj to mob   *
     *  'P': Put obj in obj    *
     *  'G': Obj to char       *
     *  'E': Obj to char equip *
     *  'D': Set state of door *
     */
};

/*
 * zone definition structure. for the 'zone-table' 
 */
struct zone_data {
    char           *name;       /* name of this zone */
    int             num;        /* number of this zone */
    int             lifespan;   /* how long between resets (minutes) */
    int             age;        /* current age of this zone (minutes) */
    long            top;        /* upper limit for rooms in this zone */
    short           start;      /* has this zone been reset yet? */

    int             reset_mode; /* conditions for reset (see below) */
    struct reset_com *cmd;      /* command table for reset */
    char            races[84];  /* races that are tolerated here */
    float           CurrTotGold,
                    LastTotGold;
    float           CurrToPCGold,
                    LastToPCGold;

    /*
     *  Reset mode:                              *
     *  0: Don't reset, and don't update age.    *
     *  1: Reset if no PC's are located in zone. *
     *  2: Just reset.                           *
     */
};

/*
 * element in monster and object index-tables 
 */
struct index_data {
    long            virtual;    /* virtual number of this mob/obj */
    long            pos;        /* file position of this field */
    int             number;     /* number of existing units of this
                                 * mob/obj */
    int             (*func) (); /* special procedure for this mob/obj */
    void           *data;
    char           *name;
    char           *short_desc;
    char           *long_desc;
    int             MaxObjCount;
};

/*
 * for queueing zones for update 
 */
struct reset_q_element {
    int             zone_to_reset;      /* ref to zone_data */
    struct reset_q_element *next;
};

/*
 * structure for the update queue 
 */
struct reset_q_type {
    struct reset_q_element *head;
    struct reset_q_element *tail;
} reset_q;

struct player_index_element {
    char           *name;
    long            nr;
};

struct help_index_element {
    char           *keyword;
    long            pos;
};

#define ZONE_NEVER       0      /* never deinit, never reset Asgard,
                                 * Scratch Zone For Static Game */
#define ZONE_EMPTY       1      /* deinit ifempty, reset ifempty TMK,
                                 * Ravenloft, mostly proc zones */
#define ZONE_ALWAYS      2      /* deinit ifempty, reset always The new
                                 * deinit, reinit cycle */
#define ZONE_ASTRAL      4
#define ZONE_DESERT      8
#define ZONE_ARCTIC      16
#define ZONE_UNDER_GROUND 32
#define ZONE_CLOSED       64
#define ZONE_NODEINIT   128     /* deinit never, reset always For the old
                                 * farts that don't like change */

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
