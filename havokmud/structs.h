#ifndef _structs_h
#define _structs_h

/*
 *      DaleMUD structs, originated from SillyMUD variant of DIKU
 *
 *      vr 3.5
 */

#include <sys/types.h>
#include <assert.h>

typedef char    sbyte;
typedef unsigned char ubyte;
typedef short int sh_int;
typedef unsigned short int ush_int;
typedef char    bool;
typedef char    byte;

#define MAX_CLAN   10

#define ALL_DARK        1
#define FOREST_DARK     2
#define NO_DARK         3

#define cmd_look       15
#define cmd_write      149
#define cmd_read       63
#define cmd_remove     66
#define cmd_reply      515
#define cmd_reload     284
#define cmd_first_move 1

#define ONE_WORD                  24
#define ONE_LINE                  96
#define ONE_PARAGRAPH            576
#define ONE_PAGE                3072

#define PULSE_COMMAND   0
#define PULSE_TICK      1
#define EVENT_DEATH     2
#define EVENT_SUMMER    3
#define EVENT_SPRING    4
#define EVENT_FALL      5
#define EVENT_WINTER    6
#define EVENT_GATHER    7
#define EVENT_ATTACK    8
#define EVENT_FOLLOW    9
#define EVENT_MONTH    10
#define EVENT_BIRTH    11       /* birth event for the mob.  */
#define EVENT_FAMINE   12
#define EVENT_DWARVES_STRIKE 13 /* fitting number, eh? -DM */
#define EVENT_END_STRIKE  14
#define EVENT_END_FAMINE  15
#define EVENT_WEEK        16
#define EVENT_GOBLIN_RAID 17
#define EVENT_END_GOB_RAID 18

#define DWARVES_STRIKE 1
#define FAMINE         2

#define SEASON_WINTER  1
#define SEASON_SPRING  2
#define SEASON_SUMMER  4
#define SEASON_FALL    8

typedef struct alias_type {
    char           *com[10];    /* 10 aliases */
} Alias;

/*
 * Arena Stuff
 */
#define PREP_ROOM_EXITS 13
#define ARENA_ENTRANCE 39900

#define MAX_CLASS 12
#define OLD_MAX_CLASS 4

#define ABS_MAX_CLASS 20        /* USER FILE, DO NOT CHANGE! */
#define ABS_MAX_EXP   2000000000        /* used in polies/switching */

#define MAX_STAT 6              /* s i, w, d, co (ch) */

/*
 * the poofin and poofout shit.  Dm gave this to Parallax, and the other
 * gods are demanding it, so I'll install it :-)
 */

#define BIT_POOF_IN  1
#define BIT_POOF_OUT 2

/*
 * 32bit bitvector defines Should prevent some coder errors while adding
 * new bitvectors
 */
#define BV00            (1 <<  0)
#define BV01            (1 <<  1)
#define BV02            (1 <<  2)
#define BV03            (1 <<  3)
#define BV04            (1 <<  4)
#define BV05            (1 <<  5)
#define BV06            (1 <<  6)
#define BV07            (1 <<  7)
#define BV08            (1 <<  8)
#define BV09            (1 <<  9)
#define BV10            (1 << 10)
#define BV11            (1 << 11)
#define BV12            (1 << 12)
#define BV13            (1 << 13)
#define BV14            (1 << 14)
#define BV15            (1 << 15)
#define BV16            (1 << 16)
#define BV17            (1 << 17)
#define BV18            (1 << 18)
#define BV19            (1 << 19)
#define BV20            (1 << 20)
#define BV21            (1 << 21)
#define BV22            (1 << 22)
#define BV23            (1 << 23)
#define BV24            (1 << 24)
#define BV25            (1 << 25)
#define BV26            (1 << 26)
#define BV27            (1 << 27)
#define BV28            (1 << 28)
#define BV29            (1 << 29)
#define BV30            (1 << 30)
#define BV31            (1 << 31)

/*
 * Quest stuff
 */

struct QuestItem {
    int             item;
    char           *where;
};

/*
 * tailoring stuff
 */
                /*
                 * I define all these as TRUE, see makefile for more info
                 */

#define LIMITED_ITEMS 1
#define SITELOCK      1
#define NODUPLICATES  1
#define EGO           1
#define LEVEL_LOSS      1
#define NEWEXP          1
#define NEWGAIN         1
#define LOW_GOLD        1
#define ZONE_COMM_ONLY  1
#define PREVENT_PKILL 1
#define PERSONAL_PERM_LOCKOUTS  1
#define LAG_MOBILES     1

#define WEAPONSKLZ 1

/*
 * efficiency stuff
 */
#define MIN_GLOB_TRACK_LEV 31   /* mininum level for global track */
/*
 **  Site locking stuff.. written by Scot Gardner
 */
#define MAX_BAN_HOSTS 50

/*
 **  Newbie authorization stuff
 */

#define NEWBIE_REQUEST 1
#define NEWBIE_START   100
#define NEWBIE_AXE     0
#define NEWBIE_CHANCES 3

/*
 **  Limited item Stuff
 */

#define LIM_ITEM_COST_MIN  10000        /* mininum rent cost of a lim.
                                         * item */

#define MAX_LIM_ITEMS 25        /* max number of limited items you can
                                 * rent with */

/*
 **  distributed monster stuff
 */

#define TICK_WRAP_COUNT 3       /* PULSE_MOBILE / PULSE_TELEPORT */
                            /*
                             * Note: This stuff is all code dependent,
                             * Don't change it unless you know what you
                             * are doing.  comm.c and mobact.c hold the
                             * stuff that you will HAVE to rewrite if you
                             * change either of those constants.
                             */
#define PLR_TICK_WRAP   24      /* this should be a divisor of 24 (hours) */

/*
 **  multiclassing stuff
 */

#define MAGE_LEVEL_IND      0
#define CLERIC_LEVEL_IND    1
#define WARRIOR_LEVEL_IND   2
#define THIEF_LEVEL_IND     3
#define DRUID_LEVEL_IND     4
#define MONK_LEVEL_IND      5
#define BARBARIAN_LEVEL_IND 6
#define SORCERER_LEVEL_IND  7
#define PALADIN_LEVEL_IND   8
#define RANGER_LEVEL_IND    9
#define PSI_LEVEL_IND       10
#define NECROMANCER_LEVEL_IND 11

#define CLASS_COUNT  11

/*
 * user flags
 */
#define NO_DELETE       BV00    /* do not delete me, well until 6 months
                                 * pass! */
#define USE_ANSI        BV01    /* we use ANSI color, yeah! */
#define RACE_WAR        BV02    /* if enabled they can be hit by
                                 * opposing race */
#define UN_DEFINED_2    BV03
#define SHOW_EXITS      BV04    /* for auto display exits */

#define MURDER_1        BV05    /* actually killed someone! */
#define STOLE_1         BV06    /* caught stealing form someone! */
#define MURDER_2        BV07    /* rumored killed someone */
#define STOLE_2         BV08    /* rumored stole from someone */

#define STOLE_X         BV09    /* saved for later */
#define MURDER_X        BV10    /* saved for later */
#define USE_PAGING      BV11    /* pause screens? */
#define CAN_OBJ_EDIT    BV12    /* this user can use oedit to edit objects */
#define CAN_MOB_EDIT    BV13    /* this user can use medit to edit mobs */
#define FAST_AREA_EDIT  BV14    /* immort is using fast online mapping */
#define PKILLING        BV15
#define CLOAKED         BV16    /* Used for mortal cloaking of their eq */
#define ZONE_SOUNDS     BV17
#define CHAR_PRIVATE    BV18
#define CLAN_LEADER     BV19
#define NEW_USER        BV20
#define HERO_STATUS     BV21
#define GHOST           BV22
#define FAST_MAP_EDIT   BV23

#define OLD_COLORS      BV24

/*
 * system flags defined on the fly and by wizards for this boot
 */
#define SYS_NOPORTAL    BV00    /* no one can portal */
#define SYS_NOASTRAL    BV01    /* no one can astral */
#define SYS_NOSUMMON    BV02    /* no one can summon */
#define SYS_NOKILL      BV03    /* NO PC (good side or bad side) can fight */
#define SYS_LOGALL      BV04    /* log ALL users to the system log */
#define SYS_ECLIPS      BV05    /* the world is in constant darkness! */
#define SYS_SKIPDNS     BV06    /* skips DNS name searches on connects */
#define SYS_REQAPPROVE  BV07    /* force god approval for new char */
#define SYS_NOANSI      BV08    /* disable ansi colors world wide */
#define SYS_WIZLOCKED   BV09    /* System is Wizlocked for all */
#define SYS_NO_POLY     BV10    /* Spell Polymorph Self is disabled */
#define SYS_NOOOC       BV11    /* Provide from OOCing worldwird - Manwe */
#define SYS_LOCOBJ      BV12    /* Disable Locate Object - Manwe */
#define SYS_WLD_ARENA   BV13    /* Makes the entire world flagged arena */
#define SYS_NO_DEINIT   BV14    /* makes zone not deinit */
#define SYS_NO_TWEAK    BV15    /* makes items not tweak */
#define SYS_ZONELOCATE  BV16    /* makes locate in zone only */

#define SPEAK_COMMON            1
#define SPEAK_ELVISH            2
#define SPEAK_HALFLING          3
#define SPEAK_DWARVISH          4
#define SPEAK_ORCISH            5
#define SPEAK_GIANTISH          6
#define SPEAK_OGRE              7
#define SPEAK_GNOMISH           8
#define SPEAK_ALL               9
#define SPEAK_GODLIKE           10

#define FIRE_DAMAGE 1
#define COLD_DAMAGE 2
#define ELEC_DAMAGE 3
#define BLOW_DAMAGE 4
#define ACID_DAMAGE 5
#define FIRESHIELD  6
#define CHILLSHIELD 7
#define BLADE_BARRIER 8

#define HATE_SEX   1
#define HATE_RACE  2
#define HATE_CHAR  4
#define HATE_CLASS 8
#define HATE_EVIL  16
#define HATE_GOOD  32
#define HATE_VNUM  64

#define FEAR_SEX   1
#define FEAR_RACE  2
#define FEAR_CHAR  4
#define FEAR_CLASS 8
#define FEAR_EVIL  16
#define FEAR_GOOD  32
#define FEAR_VNUM  64

#define OP_SEX   1
#define OP_RACE  2
#define OP_CHAR  3
#define OP_CLASS 4
#define OP_EVIL  5
#define OP_GOOD  6
#define OP_VNUM  7

#define ABS_MAX_LVL  70
#define MAX_MORT     50
#define LOW_IMMORTAL 51
#define IMMORTAL     51
#define CREATOR      52
#define SAINT        53
#define DEMIGOD      54
#define LESSER_GOD   55
#define GOD          56
#define GREATER_GOD  57
#define SILLYLORD    58
#define IMPLEMENTOR  59
#define BIG_GUY      60
#define MAX_IMMORT   60
#define NEVER_USE    61

        /*
         * Immune flags...
         */

#define IMM_FIRE        1
#define IMM_COLD        2
#define IMM_ELEC        4
#define IMM_ENERGY      8
#define IMM_BLUNT      16
#define IMM_PIERCE     32
#define IMM_SLASH      64
#define IMM_ACID      128
#define IMM_POISON    256
#define IMM_DRAIN     512
#define IMM_SLEEP    1024
#define IMM_CHARM    2048
#define IMM_HOLD     4096
#define IMM_NONMAG   8192
#define IMM_PLUS1   16384
#define IMM_PLUS2   32768
#define IMM_PLUS3   65536
#define IMM_PLUS4  131072

#define PULSE_RIVER    15
#define PULSE_TELEPORT      10
#define PULSE_MAILCHECK         240
#define PULSE_ADVICE       200
#define PULSE_DARKNESS  250
#define PULSE_ARENA     100     /* see if there's a winner in arena */
#define PULSE_AUCTION 80        /* every 20 seconds */
#define PULSE_TROLLREGEN 14     /* every once in a while */
#define PULSE_TQP 40            /* do a travel check for the qp, every 10
                                 * secs */
#define MAX_ROOMS   5000

/*
 * FIghting styles!!!
 */
#define FIGHTING_STYLE_STANDARD   0
#define FIGHTING_STYLE_BERSERKED  1
#define FIGHTING_STYLE_AGGRESSIVE 2
#define FIGHTING_STYLE_DEFENSIVE  3
#define FIGHTING_STYLE_EVASIVE    4

/*
 * minimum # of zones needed to keep tqp alive
 */
#define MIN_INIT_TQP 22
#define TQP_AMOUNT    3         /* max number of tqps out and about */
#define TRAVELQP     26         /* vnum */
#define EMPTY_POTION 29
#define EMPTY_SCROLL 32
#define QUEST_POTION 27

struct nodes {
    int             visited;
    int             ancestor;
};

struct room_q {
#if 0
    int             room_nr;
#else
    long            room_nr;
#endif
    struct room_q  *next_q;
};

struct string_block {
    int             size;
    char           *data;
};

/*
 * memory stuff
 */

struct char_list {
    struct char_data *op_ch;
    char            name[50];
    struct char_list *next;
};

typedef struct {
    struct char_list *clist;
    int             sex;        /* number
                                 * 1=male,2=female,3=both,4=neut,5=m&n,6=f&n,
                                 * 7=all */
    int             race;       /* number */
    int             class;      /* 1=m,2=c,4=f,8=t */
    int             vnum;       /* # */
    int             evil;       /* align < evil = attack */
    int             good;       /* align > good = attack */
} Opinion;

/*
 * old stuff.
 */

#define PULSE_ZONE     240

#define PULSE_MOBILE    30
#define PULSE_VIOLENCE  16
#define WAIT_SEC       4
#define WAIT_ROUND     4

/*
 * We were getting purify hits on MAX_STRING_LENGTH
 */
#define MAX_STRING_LENGTH   40960
#define MAX_INPUT_LENGTH     160

#define SMALL_BUFSIZE   512
#define LARGE_BUFSIZE   MAX_STRING_LENGTH*2

#define MAX_MESSAGES          60
#define MAX_ITEMS            153

#define MESS_ATTACKER 1
#define MESS_VICTIM   2
#define MESS_ROOM     3

#define SECS_PER_REAL_MIN  60
#define SECS_PER_REAL_HOUR (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY  (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR (365*SECS_PER_REAL_DAY)

/*
 * Lowered to 65 from 75 as a test
 */
#define SECS_PER_MUD_HOUR  65
#define SECS_PER_MUD_DAY   (24*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH (35*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR  (17*SECS_PER_MUD_MONTH)

/*
 * The following defs are for obj_data
 */

/*
 * For 'type_flag'
 */

#define ITEM_LIGHT      1
#define ITEM_SCROLL     2
#define ITEM_WAND       3
#define ITEM_STAFF      4
#define ITEM_WEAPON     5
#define ITEM_FIREWEAPON 6
#define ITEM_MISSILE    7
#define ITEM_TREASURE   8
#define ITEM_ARMOR      9
#define ITEM_POTION    10
#define ITEM_WORN      11
#define ITEM_OTHER     12
#define ITEM_TRASH     13
#define ITEM_TRAP      14
#define ITEM_CONTAINER 15
#define ITEM_NOTE      16
#define ITEM_DRINKCON  17
#define ITEM_KEY       18
#define ITEM_FOOD      19
#define ITEM_MONEY     20
#define ITEM_PEN       21
#define ITEM_BOAT      22
#define ITEM_AUDIO     23
#define ITEM_BOARD     24
#define ITEM_TREE      25
#define ITEM_ROCK      26
#define ITEM_PORTAL             27
#define ITEM_INSTRUMENT 28
#define ITEM_SHIPS_HELM  29

/*
 * Bitvector For 'wear_flags'
 */

#define ITEM_TAKE              1
#define ITEM_WEAR_FINGER       2
#define ITEM_WEAR_NECK         4
#define ITEM_WEAR_BODY         8
#define ITEM_WEAR_HEAD        16
#define ITEM_WEAR_LEGS        32
#define ITEM_WEAR_FEET        64
#define ITEM_WEAR_HANDS      128
#define ITEM_WEAR_ARMS       256
#define ITEM_WEAR_SHIELD     512
#define ITEM_WEAR_ABOUT     1024
#define ITEM_WEAR_WAISTE    2048
#define ITEM_WEAR_WRIST     4096
#define ITEM_WIELD          8192
#define ITEM_HOLD          16384
#define ITEM_THROW         32768
#define ITEM_LIGHT_SOURCE 65536
#define ITEM_WEAR_BACK   131072
#define ITEM_WEAR_EAR    262144
#define ITEM_WEAR_EYE  524288

/*
 * Bitvector for 'extra_flags'
 */
#define ITEM_GLOW               BV00
#define ITEM_HUM                BV01
#define ITEM_METAL              BV02
#define ITEM_MINERAL            BV03
#define ITEM_ORGANIC            BV04
#define ITEM_INVISIBLE          BV05
#define ITEM_MAGIC              BV06
#define ITEM_NODROP             BV07
#define ITEM_ANTI_NECROMANCER   BV08
#define ITEM_ANTI_GOOD          BV09
#define ITEM_ANTI_EVIL          BV10
#define ITEM_ANTI_NEUTRAL       BV11
#define ITEM_ANTI_CLERIC        BV12
#define ITEM_ANTI_MAGE          BV13
#define ITEM_ANTI_THIEF         BV14
#define ITEM_ANTI_FIGHTER       BV15
#define ITEM_BRITTLE            BV16
#define ITEM_RESISTANT          BV17
#define ITEM_IMMUNE             BV18
#define ITEM_ANTI_MEN           BV19
#define ITEM_ANTI_WOMEN         BV20
#define ITEM_ANTI_SUN           BV21
#define ITEM_ANTI_BARBARIAN     BV22
#define ITEM_ANTI_RANGER        BV23
#define ITEM_ANTI_PALADIN       BV24
#define ITEM_ANTI_PSI           BV25
#define ITEM_ANTI_MONK          BV26
#define ITEM_ANTI_DRUID         BV27
#define ITEM_ONLY_CLASS         BV28
#define ITEM_UNUSED             BV29
#define ITEM_RARE               BV30
#define ITEM_QUEST              BV31

/*
 * UNIQUE AND PC CORPSE HERE
 */

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
#define DRINK_POISON  (1<<0)
#define DRINK_PERM    (1<<1)

/*
 * for containers - value[1]
 */

#define CONT_CLOSEABLE      1
#define CONT_PICKPROOF      2
#define CONT_CLOSED         4
#define CONT_LOCKED         8

struct extra_descr_data {
    char           *keyword;    /* Keyword in look/examine */
    char           *description;        /* What to see */
    struct extra_descr_data *next;      /* Next in list */
};

#define MAX_OBJ_AFFECT 5        /* Used in OBJ_FILE_ELEM *DO*NOT*CHANGE* */
#define OBJ_NOTIMER    -7000000

struct obj_flag_data {
    int             value[4];   /* Values of the item (see list) */
    byte            type_flag;  /* Type of item */
    long            wear_flags; /* Where you can wear it */
    long            extra_flags;        /* If it hums,glows etc */
    int             weight;     /* Weigt what else */
    int             cost;       /* Value when sold (gp.) */
    int             cost_per_day;       /* Cost to keep pr. real day */
    int             timer;      /* Timer for object */
    long            bitvector;  /* To set chars bits */
};

/*
 * Used in OBJ_FILE_ELEM *DO*NOT*CHANGE*
 */
struct obj_affected_type {
    short           location;   /* Which ability to change (APPLY_XXX) */
    long            modifier;   /* How much it changes by */
};

/*
 * ======================== Structure for object ========================
 */
struct obj_data {
    sh_int          item_number;        /* Where in data-base */
    int             in_room;    /* In what room -1 when conta/carr */
    struct obj_flag_data obj_flags;     /* Object information */
    struct obj_affected_type
                    affected[MAX_OBJ_AFFECT];   /* Which abilities in PC
                                                 * to change */

    sh_int          sector;     /* for large rooms */
    sh_int          char_vnum;  /* for ressurection */
    long            char_f_pos; /* for ressurection */
    char           *name;       /* Title of object :get etc.  */
    char           *description;        /* When in room */
    char           *short_description;  /* when worn/carry/in cont.  */
    char           *action_description; /* What to write when used */
    struct extra_descr_data *ex_description;    /* extra descriptions */
    struct char_data *carried_by;       /* Carried by :NULL in room/conta */
    byte            eq_pos;     /* what is the equip. pos? */
    struct char_data *equipped_by;      /* equipped by :NULL in room/conta */
    struct obj_data *in_obj;    /* In what object NULL when none */
    struct obj_data *contains;  /* Contains objects */
    struct obj_data *next_content;      /* For 'contains' lists */
    struct obj_data *next;      /* For the object list */
    char           *old_name;   /* For Behead */
    int             is_corpse;  /* For Behead */
    int             beheaded_corpse;    /* For Behead */
    /*
     * New fields (GH)
     */
    int             level;      /* Level ego of the item */
    int             max;        /* max of the object */
    int             speed;      /* Speed of the weapon */
#if 0
    int fullness;
    /*
     * hrmm, can't quite get this to work smoothly
     * here, mebbe somewhere else -Lennya
     */
#endif
    int             weapontype;
    int             tweak;

    char           *modBy;
    long            modified;
};
/*
 * ======================================================================
 */

/*
 * The following defs are for room_data
 */

#define NOWHERE    -1           /* nil reference for room-database */
#define AUTO_RENT  -2           /* other special room, for auto-renting */

/*
 * Bitvector For 'room_flags'
 */

#define DARK                    BV00
#define DEATH                   BV01
#define NO_MOB                  BV02
#define INDOORS                 BV03
#define PEACEFUL                BV04
#define NOSTEAL                 BV05
#define NO_SUM                  BV06
#define NO_MAGIC                BV07
#define TUNNEL                  BV08
#define PRIVATE                 BV09
#define SILENCE                 BV10
#define LARGE                   BV11
#define NO_DEATH                BV12
#define SAVE_ROOM               BV13
#define ARENA_ROOM              BV14
#define NO_FLY                  BV15
#define REGEN_ROOM              BV16
#define FIRE_ROOM               BV17
#define ICE_ROOM                BV18
#define WIND_ROOM               BV19
#define EARTH_ROOM              BV20
#define ELECTRIC_ROOM           BV21
#define WATER_ROOM              BV22
#define MOVE_ROOM               BV23
#define MANA_ROOM               BV24
#define NO_FLEE                 BV25
#define NO_SPY                  BV26
#define EVER_LIGHT              BV27
#define ROOM_WILDERNESS         BV28

/*
 * For 'dir_option'
 */

#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

#define EX_ISDOOR       1
#define EX_CLOSED       2
#define EX_LOCKED       4
#define EX_SECRET       8
#define EX_RSLOCKED     16
#define EX_PICKPROOF    32
#define EX_CLIMB        64

/*
 * For 'Sector types'
 */

#define SECT_INSIDE          0
#define SECT_CITY            1
#define SECT_FIELD           2
#define SECT_FOREST          3
#define SECT_HILLS           4
#define SECT_MOUNTAIN        5
#define SECT_WATER_SWIM      6
#define SECT_WATER_NOSWIM    7
#define SECT_AIR             8
#define SECT_UNDERWATER      9
#define SECT_DESERT          10
#define SECT_TREE            11
#define SECT_SEA             12
#define SECT_BLANK           13
#define SECT_ROCK_MOUNTAIN   14
#define SECT_SNOW_MOUNTAIN   15
#define SECT_RUINS           16
#define SECT_JUNGLE          17
#define SECT_SWAMP           18
#define SECT_LAVA            19
#define SECT_ENTRANCE        20
#define SECT_FARM            21
#define SECT_EMPTY           22

#define SECT_MAX             22

#define TELE_LOOK            1
#define TELE_COUNT           2
#define TELE_RANDOM          4
#define TELE_SPIN            8

#define LARGE_NONE           0
#define LARGE_WATER          1
#define LARGE_AIR            2
#define LARGE_IMPASS         4

struct large_room_data {
#if 0
    unsigned int    flags[9];
#else
    long            flags[9];
#endif
};

struct auction_data {
    struct obj_data *obj;
    int             minbid;
    struct char_data *taker;

};

struct room_direction_data {
    char           *general_description;        /* When look DIR.  */
    char           *keyword;    /* for open/close */

#if 0
    sh_int          exit_info;  /* Exit info */
    int             key;        /* Key's number (-1 for no key) */
    int             to_room;    /* Where direction leeds (NOWHERE) */
    int             open_cmd;   /* cmd needed to OPEN/CLOSE door */
#else
    long            exit_info;  /* Exit info */
    long            key;        /* Key's number (-1 for no key) */
    long            to_room;    /* Where direction leeds (NOWHERE) */
    long            open_cmd;   /* cmd needed to OPEN/CLOSE door */
#endif

};

/*
 * ========================= Structure for room ==========================
 */
struct room_data {
    /*
     * sh_int
     */
#if 0
    sh_int          number;     /* Rooms number */
    sh_int          zone;       /* Room zone (for resetting) */
    sh_int          continent;  /* Which continent/mega-zone */
    sh_int          sector_type;        /* sector type (move/hide) */
#else
    long            number;     /* Rooms number */
    long            zone;       /* Room zone (for resetting) */
    long            continent;  /* Which continent/mega-zone */
    long            sector_type;        /* sector type (move/hide) */
#endif

    int             river_dir;  /* dir of flow on river */
    int             river_speed;        /* speed of flow on river */

    int             tele_time;  /* time to a teleport */
    int             tele_targ;  /* target room of a teleport */
    char            tele_mask;  /* flags for use with teleport */
    int             tele_cnt;   /* countdown teleports */

    unsigned char   moblim;     /* # of mobs allowed in room.  */

    char           *name;       /* Rooms name 'You are ...' */
    char           *description;        /* Shown when entered */
    struct extra_descr_data *ex_description;    /* for examine/look */
    struct room_direction_data *dir_option[6];  /* Directions */
    long            room_flags; /* DEATH,DARK ... etc */
    byte            light;      /* Number of lightsources in room */
    byte            dark;
    int             (*funct) ();        /* special procedure */

    struct obj_data *contents;  /* List of items in room */
    struct char_data *people;   /* List of NPC / PC in room */

    struct large_room_data *large;      /* special for large rooms */

    int             special;

};
/*
 * ========================================================================
 */

/*
 * The following defs and structures are related to char_data
 */

/*
 * For 'equipment'
 */

#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_NECK_1     3
#define WEAR_NECK_2     4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_LEGS       7
#define WEAR_FEET       8
#define WEAR_HANDS      9
#define WEAR_ARMS      10
#define WEAR_SHIELD    11
#define WEAR_ABOUT     12
#define WEAR_WAISTE    13
#define WEAR_WRIST_R   14
#define WEAR_WRIST_L   15
#define WIELD          16
#define HOLD           17
#define WEAR_BACK      18
#define WEAR_EAR_R     19
#define WEAR_EAR_L     20
#define WEAR_EYES      21
#define LOADED_WEAPON  22
#if 0
#define AUCTION_SLOT 23
#endif
#define MAX_WEAR_POS   23

/*
 * For 'char_payer_data'
 */

/*
 **  #2 has been used!!!!  Don't try using the last of the 3, because it is
 **  the keeper of active/inactive status for dead characters for ressurection!
 */
#define MAX_TOUNGE  3           /* Used in CHAR_FILE_U *DO*NOT*CHANGE* */

#define MAX_NEW_LANGUAGES 10    /* for new languages DO NOT CHANGE! */

#define MAX_SKILLS  400         /* Used in CHAR_FILE_U *DO*NOT*CHANGE*
                                 * 200->350 */
#define MAX_WEAR    (MAX_WEAR_POS+1)
#define MAX_AFFECT  40          /* Used in CHAR_FILE_U *DO*NOT*CHANGE*
                                 * 25->30 */
#define MAX_SAVES   8           /* number of saving throws types... DO NOT
                                 * CHANGE */

/*
 * Predifined conditions
 */
#define MAX_CONDITIONS 5        /* USER FILE, DO NOT CHANGE */

#define DRUNK        0
#define FULL         1
#define THIRST       2

/*
 * Bitvector for 'affected_by'
 */
#define AFF_BLIND             BV00
#define AFF_INVISIBLE         BV01
#define AFF_DETECT_EVIL       BV02
#define AFF_DETECT_INVISIBLE  BV03
#define AFF_DETECT_MAGIC      BV04
#define AFF_SENSE_LIFE        BV05
#define AFF_LIFE_PROT         BV06
#define AFF_SANCTUARY         BV07
#define AFF_DRAGON_RIDE       BV08
#define AFF_GROWTH            BV09
#define AFF_CURSE             BV10
#define AFF_FLYING            BV11
#define AFF_POISON            BV12
#define AFF_TREE_TRAVEL       BV13
#define AFF_PARALYSIS         BV14
#define AFF_INFRAVISION       BV15
#define AFF_WATERBREATH       BV16
#define AFF_SLEEP             BV17
#define AFF_TRAVELLING        BV18
#define AFF_SNEAK             BV19
#define AFF_HIDE              BV20
#define AFF_SILENCE           BV21
#define AFF_CHARM             BV22
#define AFF_DARKNESS          BV23
#define AFF_PROTECT_FROM_EVIL BV24
#define AFF_TRUE_SIGHT        BV25
#define AFF_SCRYING           BV26
#define AFF_FIRESHIELD        BV27
#define AFF_GROUP             BV28
#define AFF_TELEPATHY         BV29
#define AFF_CHILLSHIELD       BV30
#define AFF_BLADE_BARRIER     BV31

/*
 * affects 2
 */
#define AFF2_ANIMAL_INVIS      BV00
#define AFF2_HEAT_STUFF        BV01
#define AFF2_LOG_ME            BV02
#define AFF2_BERSERK           BV03
#define AFF2_CON_ORDER         BV04
#define AFF2_AFK               BV05
#define AFF2_DETECT_GOOD       BV06
#define AFF2_PROTECT_FROM_GOOD BV07
#define AFF2_FOLLOW            BV08
#define AFF2_HASTE             BV09
#define AFF2_SLOW              BV10
#define AFF2_WINGSBURNED       BV11
#define AFF2_STYLE_BERSERK     BV12
#define AFF2_QUEST             BV13
#define AFF2_NO_OUTDOOR        BV14
#define AFF2_WINGSTIRED        BV15
#define AFF2_INVIS_TO_UNDEAD   BV16
#define AFF2_SKILL_SNEAK       BV19

/*
 * Flag spells as brewable or single class
 */
#define BREWABLE_SPELL         BV00
#define SINGLE_CLASS_SPELL     BV01
#define DUAL_CLASS_SPELL       BV02
#define TRI_CLASS_SPELL        BV03

/*
 * modifiers to char's abilities
 */

#define APPLY_NONE              0
#define APPLY_STR               1
#define APPLY_DEX               2
#define APPLY_INT               3
#define APPLY_WIS               4
#define APPLY_CON               5
#define APPLY_CHR               6
#define APPLY_SEX               7
#define APPLY_LEVEL             8
#define APPLY_AGE               9
#define APPLY_CHAR_WEIGHT      10
#define APPLY_CHAR_HEIGHT      11
#define APPLY_MANA             12
#define APPLY_HIT              13
#define APPLY_MOVE             14
#define APPLY_GOLD             15
#define APPLY_SPELL2           16
#define APPLY_AC               17
#define APPLY_ARMOR            17
#define APPLY_HITROLL          18
#define APPLY_DAMROLL          19
#define APPLY_SAVING_PARA      20
#define APPLY_SAVING_ROD       21
#define APPLY_SAVING_PETRI     22
#define APPLY_SAVING_BREATH    23
#define APPLY_SAVING_SPELL     24
#define APPLY_SAVE_ALL         25
#define APPLY_IMMUNE           26
#define APPLY_SUSC             27
#define APPLY_M_IMMUNE         28
#define APPLY_SPELL            29
#define APPLY_WEAPON_SPELL     30
#define APPLY_EAT_SPELL        31
#define APPLY_BACKSTAB         32
#define APPLY_KICK             33
#define APPLY_SNEAK            34
#define APPLY_HIDE             35
#define APPLY_BASH             36
#define APPLY_PICK             37
#define APPLY_STEAL            38
#define APPLY_TRACK            39
#define APPLY_HITNDAM          40
#define APPLY_SPELLFAIL        41
#define APPLY_ATTACKS          42
#define APPLY_HASTE            43
#define APPLY_SLOW             44
#define APPLY_BV2              45
#define APPLY_FIND_TRAPS       46
#define APPLY_RIDE             47
#define APPLY_RACE_SLAYER      48
#define APPLY_ALIGN_SLAYER     49
#define APPLY_MANA_REGEN       50
#define APPLY_HIT_REGEN        51
#define APPLY_MOVE_REGEN       52
                /*
                 * Set thirst/hunger/drunk to MOD
                 */
#define APPLY_MOD_THIRST        53
#define APPLY_MOD_HUNGER        54
#define APPLY_MOD_DRUNK         55

                /*
                 * not implemented
                 */
#define APPLY_T_STR             56
#define APPLY_T_INT             57
#define APPLY_T_DEX             58
#define APPLY_T_WIS             59
#define APPLY_T_CON             60
#define APPLY_T_CHR             61
#define APPLY_T_HPS             62
#define APPLY_T_MOVE            63
#define APPLY_T_MANA            64

/*
 * 'class' for PC's
 */
#define CLASS_MAGIC_USER  BV00
#define CLASS_CLERIC      BV01
#define CLASS_WARRIOR     BV02
#define CLASS_THIEF       BV03
#define CLASS_DRUID       BV04
#define CLASS_MONK        BV05
#define CLASS_BARBARIAN   BV06
#define CLASS_SORCERER    BV07
#define CLASS_PALADIN     BV08
#define CLASS_RANGER      BV09
#define CLASS_PSI         BV10
#define CLASS_NECROMANCER BV11

/*
 * sex
 */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2

/*
 * positions
 */
#define POSITION_DEAD       0
#define POSITION_MORTALLYW  1
#define POSITION_INCAP      2
#define POSITION_STUNNED    3
#define POSITION_SLEEPING   4
#define POSITION_RESTING    5
#define POSITION_SITTING    6
#define POSITION_FIGHTING   7
#define POSITION_STANDING   8
#define POSITION_MOUNTED    9

/*
 * for mobile actions: specials.act
 */
#define ACT_SPEC       (1<<0)   /* special routine to be called if exist */
#define ACT_SENTINEL   (1<<1)   /* this mobile not to be moved */
#define ACT_SCAVENGER  (1<<2)   /* pick up stuff lying around */
#define ACT_ISNPC      (1<<3)   /* This bit is set for use with IS_NPC() */
#define ACT_NICE_THIEF (1<<4)   /* Set if a thief should NOT be killed */
#define ACT_AGGRESSIVE (1<<5)   /* Set if automatic attack on NPC's */
#define ACT_STAY_ZONE  (1<<6)   /* MOB Must stay inside its own zone */
#define ACT_WIMPY      (1<<7)   /* MOB Will flee when injured, and if
                                 * aggressive only attack sleeping players
                                 */
#define ACT_ANNOYING   (1<<8)   /* MOB is so utterly irritating that other
                                 * monsters will attack it...
                                 */
#define ACT_HATEFUL    (1<<9)   /* MOB will attack a PC or NPC matching a
                                 * specified name
                                 */
#define ACT_AFRAID    (1<<10)   /* MOB is afraid of a certain PC or NPC,
                                 * and will always run away ....
                                 */
#define ACT_IMMORTAL  (1<<11)   /* MOB is a natural event, can't be kiled */
#define ACT_HUNTING   (1<<12)   /* MOB is hunting someone */
#define ACT_DEADLY    (1<<13)   /* MOB has deadly poison */
#define ACT_POLYSELF  (1<<14)   /* MOB is a polymorphed person */
#define ACT_META_AGG  (1<<15)   /* MOB is _very_ aggressive */
#define ACT_GUARDIAN  (1<<16)   /* MOB will guard master */
#define ACT_NECROMANCER     (1<<17)
#define ACT_HUGE      (1<<18)   /* MOB is too large to go indoors */
#define ACT_SCRIPT    (1<<19)   /* MOB has a script assigned to it DO NOT SET */
#define ACT_GREET     (1<<20)   /* MOB greets people */

#define ACT_MAGIC_USER  (1<<21)
#define ACT_WARRIOR     (1<<22)
#define ACT_CLERIC      (1<<23)
#define ACT_THIEF       (1<<24)
#define ACT_DRUID       (1<<25)
#define ACT_MONK        (1<<26)
#define ACT_BARBARIAN   (1<<27)
#define ACT_PALADIN     (1<<28)
#define ACT_RANGER      (1<<29)
#define ACT_PSI         (1<<30)
/*
 * room for one more
 */
/*
 * for common mobile procedures: specials.proc
 */
#define PROC_NONE               0 /* no proc */
#define PROC_SHOPKEEPER         1 /* int shopkeeper() behaviour */
#define PROC_GUILDMASTER        2 /* GM, dependant on class in MobAct & level */
#define PROC_SWALLOWER          3 /* Swallows when not bashed */
#define PROC_DRAIN              4 /* Drains when not bashed */
#define PROC_QUEST              5 /* it's a give/receive mob */

#define PROC_OLD_BREATH         6
#define PROC_FIRE_BREATH        7
#define PROC_GAS_BREATH         8
#define PROC_FROST_BREATH       9
#define PROC_ACID_BREATH        10
#define PROC_LIGHTNING_BREATH   11
#define PROC_DEHYDRATION_BREATH 12
#define PROC_VAPOR_BREATH       13
#define PROC_SOUND_BREATH       14
#define PROC_SHARD_BREATH       15
#define PROC_SLEEP_BREATH       16
#define PROC_LIGHT_BREATH       17
#define PROC_DARK_BREATH        18
#define PROC_RECEPTIONIST       19
#define PROC_REPAIRGUY          20

/*
 * For players : specials.act
 */
#define PLR_BRIEF       (1<<0)
/*
 * (1<<1)
 */
#define PLR_COMPACT     (1<<2)
#define PLR_DONTSET     (1<<3)  /* Dont EVER set */
#define PLR_WIMPY       (1<<4)  /* character will flee when seriously
                                 * injured */
#define PLR_NOHASSLE    (1<<5)  /* char won't be attacked by aggressives. */
#define PLR_STEALTH     (1<<6)  /* char won't be announced in a variety of
                                 * situations */
#define PLR_HUNTING     (1<<7)  /* the player is hunting someone, do a
                                 * track each look */
#define PLR_DEAF        (1<<8)  /* The player does not hear shouts */
#define PLR_ECHO        (1<<9)  /* Messages (tells, shout,etc) echo back */
#define PLR_NOGOSSIP    (1<<10) /* New, gossip channel */
#define PLR_NOAUCTION   (1<<11) /* New AUCTION channel */
#define PLR_NODIMD      (1<<12) /* Cant use DIMD */
#define PLR_NOOOC       (1<<13) /* Can't hear the OOC channel - Manwe
                                 * Windmaster */
#define PLR_NOSHOUT     (1<<14) /* the player is not allowed to shout */
#define PLR_FREEZE      (1<<15) /* The player is frozen, must have pissed
                                 * an immo off */
#define PLR_NOTELL      (1<<16) /* The player does not hear tells */
#define PLR_NOFLY       (1<<17) /* After 'land' the player will not fly
                                 * until 'launch' */
#define PLR_WIZNOOOC    (1<<18) /* Been forbidden from OOCing by gods */
#define PLR_WIZREPORT   (1<<19)
#define PLR_NOBEEP      (1<<20) /* ignore all beeps */
#define PLR_HAVEROOM    (1<<21) /* Player is one of the first 100 heros
                                 * and have been assigned an homeroom */
#define PLR_MAILING     (1<<22) /* EMPTY */
#define PLR_NOOUTDOOR   (1<<23)
#define PLR_AUTOASSIST  (1<<24)
#define PLR_AUTOEXIT    (1<<25)
#define PLR_AUTOLOOT    (1<<26)
#define PLR_AUTOSAC     (1<<27)
#define PLR_AUTOGOLD    (1<<28)
#define PLR_AUTOSPLIT   (1<<29)
#define PLR_CLAN_LEADER (1<<30)
#define PLR_LEGEND      (1<<31)

/*
 * This structure is purely intended to be an easy way to transfer
 */
/*
 * and return information about time (real or mudwise).
 */
struct time_info_data {
    byte            hours,
                    day,
                    month;
    sh_int          year;
};

/*
 * Data used in the display code checks -DM
 */
struct last_checked {
    int             mana;
    int             mmana;
    int             hit;
    int             mhit;
    int             move;
    int             mmove;
    int             exp;
    int             gold;
};

/*
 * These data contain information about a players time data
 */
struct time_data {
    time_t          birth;      /* This represents the characters age */
    time_t          logon;      /* Time of the last logon (used to
                                 * calculate played) */
    int             played;     /* This is the total accumulated time
                                 * played in secs */
};

struct char_player_data {

    byte            sex;        /* PC / NPC s sex */
    short           weight;     /* PC / NPC s weight */
    short           height;     /* PC / NPC s height */
    bool            talks[MAX_TOUNGE];  /* PC s Tounges 0 for NPC/not used
                                         * for languagesn */
    long            user_flags; /* no delete, ansi etc... */
    int             speaks;     /* current language speaking */

    char           *name;       /* PC / NPC s name (kill ...  ) */
    char           *short_descr;        /* for 'actions' */
    char           *long_descr; /* for 'look'.. Only here for testing */
    char           *description;        /* Extra descriptions */
    char           *title;      /* PC / NPC s title */
    char           *sounds;     /* Sound that the monster makes (in room) */
    char           *distant_snds;       /* Sound that the monster makes
                                         * (adjacent to room) */

    long            class;      /* PC s class or NPC alignment */
    int             hometown;   /* PC s Hometown (zone) */

    long            extra_flags;        /* for ressurection in the future
                                         * , etc */

    struct time_data time;      /* PC s AGE in days */

    byte            level[ABS_MAX_CLASS];       /* PC / NPC s level */
    unsigned short int q_points;        /* Quest points of player */
    bool            has_mail;   /* Has the player mail waiting? */
};

/*
 * Used in CHAR_FILE_U *DO*NOT*CHANGE*
 */
struct char_ability_data {
    sbyte           str;
    sbyte           str_add;    /* 000 - 100 if strength 18 */
    sbyte           intel;
    sbyte           wis;
    sbyte           dex;
    sbyte           con;
    sbyte           chr;
    sbyte           extra;
    sbyte           extra2;
};

/*
 * Used in CHAR_FILE_U *DO*NOT*CHANGE*
 */
struct char_point_data {
    sh_int          mana;
    sh_int          max_mana;
    /*
     * ubyte mana_gain ;
     */
    char            mana_gain;

    sh_int          hit;
    sh_int          max_hit;    /* Max hit for NPC */
    /*
     * ubyte hit_gain;
     */
    char            hit_gain;

    sh_int          move;
    sh_int          max_move;   /* Max move for NPC */
    /*
     * ubyte move_gain;
     */
    char            move_gain;

    sh_int          extra1;     /* extra stuff */
    sh_int          extra2;
    ubyte           extra3;

    sh_int          armor;      /* Internal -100..100, external -10..10 AC
                                 */
    int             gold;       /* Money carried */
    int             bankgold;   /* gold in the bank.  */
    int             exp;        /* The experience of the player */
    int             clan;       /* clan number */
    int             leadership_exp;     /* leadership experience */

    sbyte           hitroll;    /* Any bonus or penalty to the hit roll */
    sbyte           damroll;    /* Any bonus or penalty to the damage roll
                                 */

    sbyte           extra_h;    /* extra */
};

struct pc_data {
    long            plr;        /* plr_* flags */
    int             dimd_credits;       /* credits used in remote comm */
    long            comm;       /* flags used for remote communications */
};
struct weaponskills {
    int             slot1;
    int             slot2;
    int             slot3;
    int             slot4;
    int             slot5;
    int             slot6;
    int             slot7;
    int             slot8;
    int             grade1;
    int             grade2;
    int             grade3;
    int             grade4;
    int             grade5;
    int             grade6;
    int             grade7;
    int             grade8;
};
struct char_special_data {
    int             spellfail;  /* max # for spell failure (101) */
    byte            tick;       /* the tick that the mob/player is on */
    byte            pmask;      /* poof mask */
    byte            position;   /* Standing or ...  */
    byte            default_pos;        /* Default position for NPC */
    byte            spells_to_learn;    /* How many can you learn yet this
                                         * level */
    byte            carry_items;        /* Number of items carried */
    byte            last_direction;     /* The last direction the monster
                                         * went */
    char            sev;        /* log severety level for gods */
    long            userpos;
    char           *userpwd;

    long            start_room; /* so people can be set to start certain
                                 * places */
    int             edit;       /* edit state */

    sbyte           mobtype;    /* mob type simple, A, L, B */
    unsigned long   exp_flag;   /* exp flag for this mob */
    sbyte           hp_num_dice;        /* number of HPS dice */
    unsigned int    hp_size_dice;       /* size of HPS dice */
    unsigned int    hp_bonus_hps;       /* bonus hps number */

    byte            damnodice;  /* The number of damage dice's */
    byte            damsizedice;        /* The size of the damage dice's */

    unsigned int    dam_bonus;  /* damage bonus */
    byte            medit;      /* mob edit menu at */
    struct char_data *mobedit;  /* mob editing */

    byte            oedit;      /* obj editing menu at */
    struct obj_data *objedit;   /* object editing */

    byte            hedit;
    struct help_file_u *help;   /* helpfile editing */
    byte            tfd;
    struct edit_txt_msg *txtedit;       /* motd/wmotd/news editing */

    int             tick_to_lag;

    sbyte           conditions[MAX_CONDITIONS]; /* Drunk full etc.  */
    int             permissions;
    int             zone;       /* zone that an NPC lives in */
    int             carry_weight;       /* Carried weight */
    int             timer;      /* Timer for update */
    int             was_in_room;        /* storage of location for
                                         * linkdead people */
    int             attack_type;        /* The Attack Type Bitvector for
                                         * NPC's */
    int             alignment;  /* +-1000 for alignments */

    char           *poofin;
    char           *poofout;
    char           *prompt;
    char           *bprompt;
    char           *email;      /* email address in aux */
    char           *immtitle;

    char           *clan;
    int            *clanNum;
    char           *rumor;
    char           *group_name; /* current group name if any... */
    char           *hostip;     /* (GH) keep track of IP */
    Alias          *A_list;
    struct char_data *misc;
    struct char_data *fighting; /* Opponent */

    struct char_data *hunting;  /* Hunting person..  */

    struct char_data *ridden_by;
    struct char_data *mounted_on;

    struct char_data *charging; /* we are charging this person */
    int             charge_dir; /* direction charging */

    long            affected_by;        /* Bitvector for spells/skills
                                         * affected by */
    long            affected_by2;       /* Other special things */

    long            act;        /* flags for NPC behavior */

    sh_int          apply_saving_throw[MAX_SAVES];      /* Saving throw
                                                         * (Bonuses) */

    int             questwon;
    struct auction_data *auctionx;      /* not used */
    struct obj_data *auction;
    long            minbid;

    int             a_deaths;
    int             a_kills;
    int             m_deaths;
    long            m_kills;

    int             proc;       /* flags for more common NPC behaviour
                                 * (shopkeeper, GM, swallower, drainer,
                                 * etc) */

    char           *talks;      /* what mob says when talked to */
    char           *quest_yes;  /* what mob says if returning correct item
                                 * and dishing out prize */
    char           *quest_no;   /* what mob says when returning wrong item
                                 */

    int             is_playing;
    int             is_hearing;

    int             remortclass;
};

/*
 * skill_data flags
 */
#define SKILL_KNOWN                 BV00
#define SKILL_KNOWN_CLERIC          BV01
#define SKILL_KNOWN_MAGE            BV02
#define SKILL_KNOWN_SORCERER        BV03
#define SKILL_KNOWN_THIEF           BV04
#define SKILL_KNOWN_MONK            BV05
#define SKILL_KNOWN_DRUID           BV06
#define SKILL_KNOWN_WARRIOR         BV07
/*
 * need to make byte flags, to long or int flags in char_skill_data
 */
#define SKILL_KNOWN_BARBARIAN       BV08
#define SKILL_KNOWN_PALADIN         BV09
#define SKILL_KNOWN_RANGER          BV10
#define SKILL_KNOWN_PSI             BV11
#define SKILL_KNOWN_NECROMANCER     BV12

/*
 * skill_data special
 */
#define SKILL_SPECIALIZED       1
#define SKILL_UNDEFINED         2
#define SKILL_UNDEFINED2        4

/*
 * Used in CHAR_FILE_U *DO*NOT*CHANGE*
 */
struct char_skill_data {
    byte            learned;    /* % chance for success 0 = not learned */

    /*
     * change to int or long
     */
    byte            flags;      /* SKILL KNOWN? bit settings */
    byte            special;    /* spell/skill specializations */
    byte            nummem;     /* number of times this spell is memorized */
};

/*
 * Used in CHAR_FILE_U *DO*NOT*CHANGE*
 */
struct affected_type {
    short           type;       /* The type of spell that caused this */
    sh_int          duration;   /* For how long its effects will last */
    long            modifier;   /* This is added to apropriate ability */
    long            location;   /* Tells which ability to
                                 * change(APPLY_XXX) */
    long            bitvector;  /* Tells which bits to set (AFF_XXX) */
    struct affected_type *next;
};

struct follow_type {
    struct char_data *follower;
    struct follow_type *next;
};

/*
 * ================== Structure for player/non-player
 * =====================
 */
struct char_data {
    long            nr;         /* monster nr */
    long            in_room;    /* Location */
    int             reroll;     /* Number of rerolls still availiable */
    int             term;
    int             size;
    struct last_checked last;   /* For displays */
    unsigned        immune;     /* Immunities */
    unsigned        M_immune;   /* Meta Immunities */
    unsigned        susc;       /* susceptibilities */
    float           mult_att;   /* the number of attacks */
    byte            attackers;
    byte            sector;     /* which part of a large room am i in? */
    int             generic;    /* generic int */
    int             commandp;   /* command poitner for scripts */
    int             waitp;      /* waitp for scripts */
    int             commandp2;  /* place-holder for gosubs, etc. */
    int             script;

    sh_int          race;
    sh_int          hunt_dist;  /* max dist the player can hunt */

    struct pc_data *pc;         /* pcdata */

    unsigned short  hatefield;
    unsigned short  fearfield;

    Opinion         hates;
    Opinion         fears;

    sh_int          persist;
    int             old_room;

    void           *act_ptr;    /* numeric argument for the mobile actions
                                 */

    struct char_player_data player;     /* Normal data */
    struct char_ability_data abilities; /* Abilities */
    struct char_ability_data tmpabilities;      /* The abilities we use */
    struct affected_type *affected;     /* affected by what spells */
    struct char_point_data points;      /* Points */
    struct char_special_data specials;  /* Special plaing constant */

    /*
     * monitor these three
     */
    struct char_skill_data *skills;     /* Skills */
    struct obj_data *equipment[MAX_WEAR];       /* Equipment array */
    struct obj_data *carrying;  /* Head of list */
    /*
     * end monitor
     */

    struct descriptor_data *desc;       /* NULL for mobiles */
    struct char_data *orig;     /* Special for polymorph */

    struct char_data *next_in_room;     /* For room->people - list */
    struct char_data *next;     /* all in game list */
    struct char_data *next_fighting;    /* For fighting list */
    struct char_data *next_memorize;    /* For list of people memorizing */

    struct follow_type *followers;      /* List of chars followers */
    struct char_data *master;   /* Who is char following? */
    int             invis_level;        /* visibility of gods */
    char            last_tell[80];      /* Who last telled something to
                                         * the char */
    int             old_exp;    /* For energe restore -MW */
    int             style;      /* Fighting Style!!! */
    struct weaponskills weaponskills;   /* the weaponry shiznit -Lennya */
};

/*
 * ========================================================================
 */

/*
 * How much light is in the land ?
 */

#define SUN_DARK        0
#define SUN_RISE        1
#define SUN_LIGHT       2
#define SUN_SET         3
#define MOON_SET        4
#define MOON_RISE       5

/*
 * And how is the sky ?
 */

#define SKY_CLOUDLESS   0
#define SKY_CLOUDY      1
#define SKY_RAINING     2
#define SKY_LIGHTNING   3

struct weather_data {
    int             pressure;   /* How is the pressure ( Mb ) */
    int             change;     /* How fast and what way does it change. */
    int             sky;        /* How is the sky. */
    int             sunlight;   /* And how much sun. */
};

/*
 ***********************************************************************
 *  file element for player file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */

struct char_file_u {
    int             class;
    byte            sex;
    byte            level[ABS_MAX_CLASS];
    unsigned short int q_points;        /* Quest points of player */
    time_t          birth;      /* Time of birth of character */
    int             played;     /* Number of secs played in total */
    int             race;
    unsigned int    weight;
    unsigned int    height;
    char            title[80];
    char            extra_str[255];
    sh_int          hometown;
    char            description[240];
    bool            talks[MAX_TOUNGE];
    long            extra_flags;
    sh_int          load_room;  /* Which room to place char in */
    struct char_ability_data abilities;
    struct char_point_data points;
    struct char_skill_data skills[MAX_SKILLS];
    struct affected_type affected[MAX_AFFECT];
    /*
     * specials
     */
    byte            spells_to_learn;
    int             alignment;
    long            affected_by;
    long            affected_by2;
    time_t          last_logon; /* Time (in secs) of last logon */
    long            act;        /* ACT Flags */

    /*
     * char data
     */
    char            name[20];
    char            alias[30];  /* for later use.... */
    char            pwd[11];
    sh_int          apply_saving_throw[MAX_SAVES];
    int             conditions[MAX_CONDITIONS];
    int             startroom;  /* which room the player should start in */
    long            user_flags; /* no-delete,use ansi,etc... */
    int             speaks;     /* language currently speakin in */

    int             a_deaths;
    int             a_kills;
    int             m_deaths;
    long            m_kills;

    int             remortclass;
    int             slot1;
    int             slot2;
    int             slot3;
    int             slot4;
    int             slot5;
    int             slot6;
    int             slot7;
    int             slot8;
    int             grade1;
    int             grade2;
    int             grade3;
    int             grade4;
    int             grade5;
    int             grade6;
    int             grade7;
    int             grade8;
};

/*
 ***********************************************************************
 *  file element for help   file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */

struct help_file_u {
    char           *name;
    char           *usage;      /* for later use.... */
    char           *accumulative;
    char           *duration;
    char           *level;
    char           *damagetype;
    char           *saves;
    char           *description;
    char           *references;
    int             wizard;
    char           *modBy;
    long            modified;
    int             newfile;
    long            index;
};

/*
 ***********************************************************************
 *  file element for object file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */

struct obj_cost {
    /* used in act.other.c:do_save as well as in reception2.c */
    int             total_cost;
    int             no_carried;
    bool            ok;
};

#define MAX_OBJ_SAVE 200        /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct obj_file_elem {
    int             item_number;

    int             value[4];
    int             extra_flags;
    int             weight;
    int             timer;
    long            bitvector;
    char            name[128];  /* big, but not horrendously so */
    char            sd[128];
    char            desc[256];
    byte            wearpos;
    byte            depth;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u {
    char            owner[20];  /* Name of player */
    int             gold_left;  /* Number of goldcoins left at owner */
    int             total_cost; /* The cost for all items, per day */
    long            last_update;        /* Time in seconds, when last
                                         * updated */
    long            minimum_stay;       /* For stasis */
    int             number;     /* number of objects */
    struct obj_file_elem objects[MAX_OBJ_SAVE];
};

#if 0

#define MAX_OBJ_SAVE 200        /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */

struct rental_header {
    char            inuse;
    int             length;
    char            owner[20];  /* Name of player */
};

struct obj_file_elem {
    int             item_number;

    int             value[4];
    int             extra_flags;
    int             weight;
    int             timer;
    long            bitvector;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};

struct obj_file_u {
    int             gold_left;  /* Number of goldcoins left at owner */
    int             total_cost; /* The cost for all items, per day */
    long            last_update;        /* Time in seconds, when last
                                         * updated */
    long            minimum_stay;       /* For stasis */
    int             nobjects;   /* how many objects below */
    struct obj_file_elem objects[MAX_OBJ_SAVE];
    /*
     * We don't always allocate this much space but it is handy for the
     * times when you need a fast one lying around.
     */
};

#endif

/*
 ***********************************************************
 *  The following structures are related to descriptor_data   *
 *********************************************************** */

struct txt_block {
    char           *text;
    struct txt_block *next;
};

struct txt_q {
    struct txt_block *head;
    struct txt_block *tail;
};

/*
 * modes of connectedness
 */

#define CON_PLYNG           0
#define CON_NME             1
#define CON_NMECNF          2
#define CON_PWDNRM          3
#define CON_PWDGET          4
#define CON_PWDCNF          5
#define CON_QSEX            6
#define CON_RMOTD           7
#define CON_SLCT            8
#define CON_EXDSCR          9
#define CON_QCLASS          10
#define CON_LDEAD           11
#define CON_PWDNEW          12
#define CON_PWDNCNF         13
#define CON_WIZLOCK         14
#define CON_QRACE           15
#define CON_RACPAR          16
#define CON_AUTH            17
#define CON_CITY_CHOICE     18
#define CON_STAT_LIST       19
#define CON_QDELETE         20
#define CON_QDELETE2        21
#define CON_STAT_LISTV      22
#define CON_WMOTD           23
#define CON_EDITING         24
#define CON_DELETE_ME       25
#define CON_CHECK_MAGE_TYPE 26
#define CON_OBJ_EDITING     27
#define CON_MOB_EDITING     28
#define CON_REROLL          29
#define CON_PRESS_ENTER     30
#define CON_ALREADY_PLAYING 31
#define CON_ANSI            32
#define CON_CREATION_MENU   33
#define CON_ALIGNMENT       34
#define CON_HELP_EDITING    35
#define CON_EMAILREG        36
#define CON_TFD_EDITING     37
#define CON_MCLASS          38

struct snoop_data {
    struct char_data *snooping;
    /*
     * Who is this char snooping
     */
    struct char_data *snoop_by;
    /*
     * And who is snooping on this char
     */
};

struct descriptor_data {
    int             descriptor; /* file descriptor for socket */

    char           *name;       /* ptr to name for mail system */

    char            host[256];  /* hostname */
    char            pwd[12];    /* password */
    int             pos;        /* position in player-file */
    int             connected;  /* mode of 'connectedness' */
    int             wait;       /* wait for how many loops */

    char           *showstr_point;      /* - */
    char           *showstr_head;       /* for paging through texts */
    char          **showstr_vector;     /* for paging through texts */
    int             showstr_count;      /* number of pages to page through
                                         */
    int             showstr_page;       /* which page are we currently
                                         * showing? */
    char          **str;        /* for the modify-str system */
    int             max_str;    /* - */

    int             prompt_mode;        /* control of prompt-printing */
    char            buf[MAX_STRING_LENGTH];     /* buffer for raw input */
    char            last_input[MAX_INPUT_LENGTH];       /* the last input */

    char            stat[MAX_STAT];     /* stat priorities */

    /*
     * for the new write_to_out
     */
    char            small_outbuf[SMALL_BUFSIZE];
    int             bufptr;
    int             bufspace;
    struct txt_block *large_outbuf;
#if BLOCK_WRITE
    char           *output;
#else
    struct txt_q    output;     /* q of strings to send */
#endif
    struct txt_q    input;      /* q of unprocessed input */
    struct char_data *character;        /* linked to char */
    struct char_data *original; /* original char */
    struct snoop_data snoop;    /* to snoop people.  */
    struct descriptor_data *next;       /* link to next descriptor */

    char           *list_string;        /* for building very large buffers
                                         * for page_string */
    int             list_size;  /* keeps track of strlen of list_string */

    struct bulletin_board_message *msg; /* For posting discussion messages
                                         */
    struct bulletin_board *board;       /* To keep track of the board
                                         * we're posting to */

};

struct msg_type {
    char           *attacker_msg;       /* message to attacker */
    char           *victim_msg; /* message to victim */
    char           *room_msg;   /* message to room */
};

struct bulletin_board_message {
    char           *author;
    char           *title;
    char           *text;
    time_t          date;
    int             char_id;
    short           message_id;
    short           reply_to;
    short           language;

    struct bulletin_board_message *next;
};

struct bulletin_board {
    short           num_posts;
    struct bulletin_board_message *messages;
    int             board_num;

    struct bulletin_board *next;
};

struct message_type {
    struct msg_type die_msg;    /* messages when death */
    struct msg_type miss_msg;   /* messages when miss */
    struct msg_type hit_msg;    /* messages when hit */
    struct msg_type sanctuary_msg;      /* messages when hit on sanctuary */
    struct msg_type god_msg;    /* messages when hit on god */
    struct message_type *next;  /* to next messages of this kind. */
};

struct message_list {
    int             a_type;     /* Attack type */
    int             number_of_attacks;  /* How many attack messages to
                                         * chose from. */
    struct message_type *msg;   /* List of messages.  */
};

struct dex_skill_type {
    sh_int          p_pocket;
    sh_int          p_locks;
    sh_int          traps;
    sh_int          sneak;
    sh_int          hide;
};

struct dex_app_type {
    sh_int          reaction;
    sh_int          miss_att;
    sh_int          defensive;
};

struct str_app_type {
    sh_int          tohit;      /* To Hit (THAC0) Bonus/Penalty */
    sh_int          todam;      /* Damage Bonus/Penalty */
    sh_int          carry_w;    /* Maximum weight that can be carrried */
    sh_int          wield_w;    /* Maximum weight that can be wielded */
};

struct wis_app_type {
    byte            bonus;      /* how many bonus skills a player can */
    /*
     * practice pr. level
     */
};

struct int_app_type {
    byte            learn;      /* how many % a player learns a
                                 * spell/skill */
};

struct con_app_type {
    sh_int          hitp;
    sh_int          shock;
};

struct chr_app_type {
    sh_int          num_fol;
    sh_int          reaction;
};

/************************************************************/

typedef void    (*funcp) ();

struct breather {
    int             vnum;
    int             cost;
    funcp          *breaths;
};

struct skillset {
    char           *name;
    int             skillnum;
    int             level;
    int             maxlearn;
};

struct affect_list {
    char           *affectname;
    int             affectnumber;
    int             affecttype;
};

struct clan {
    int             number;
    char           *name;
    char           *shortname;
    char           *desc;
    int             home;

};

struct edit_txt_msg {
    int             file;       /* 0 = none, 1 = news, 2 = motd, 3 = wmotd */
    char           *date;
    char           *author;
    char           *body;
};

struct class_titles {
    char           *male;
    char           *female;
    char           *neutral;

};

/*
 * Ok.. this list probaby isn't finished.. They have to match up for # for
 * #.. So when someone has time.. Just make sure that its numbered right
 * NOTE: I had to take a few out.. (which screws up the numbering.. b/c
 * they were already defined.. Once this is finished.. This should make
 * the code a prettier place
 *
 * Instead of every proc going if(cmd==322) It can now be
 * if(cmd==CMD_blah)
 */
enum {
    CMD_north = 1, CMD_east, CMD_south, CMD_west, CMD_up, CMD_down, CMD_enter,
    CMD_exits, CMD_kiss, CMD_get, CMD_drink, CMD_eat, CMD_wear, CMD_wield,
    CMD_score, CMD_say, CMD_shout, CMD_tell, CMD_inventory, CMD_qui,
    CMD_bounce, CMD_smile, CMD_dance, CMD_kill, CMD_id, CMD_laugh,
    CMD_giggle, CMD_shake, CMD_puke, CMD_growl, CMD_scream, CMD_insult,
    CMD_comfort, CMD_nod, CMD_sigh, CMD_sulk, CMD_help, CMD_who,
    CMD_emote, CMD_echo, CMD_stand, CMD_sit, CMD_rest, CMD_sleep,
    CMD_wake, CMD_force, CMD_transfer, CMD_hug, CMD_snuggle,
    CMD_cuddle, CMD_nuzzle, CMD_cry, CMD_news, CMD_equipment, CMD_buy,
    CMD_sell, CMD_value, CMD_list, CMD_drop, CMD_goto, CMD_weather, CMD_pour,
    CMD_grab, CMD_put, CMD_shutdow, CMD_save, CMD_hit, CMD_string, CMD_give,
    CMD_quit, CMD_stat, CMD_guard, CMD_time, CMD_purge, CMD_shutdown, CMD_idea,
    CMD_typo, CMD_bug, CMD_whisper, CMD_cast, CMD_at, CMD_ask, CMD_order,
    CMD_sip, CMD_taste, CMD_snoop, CMD_follow, CMD_rent, CMD_offer, CMD_poke,
    CMD_advance, CMD_accuse, CMD_grin, CMD_bow, CMD_open, CMD_close, CMD_lock,
    CMD_unlock, CMD_leave, CMD_applaud, CMD_blush, CMD_burp, CMD_chuckle,
    CMD_clap, CMD_cough, CMD_curtsey, CMD_fart, CMD_flip, CMD_fondle, CMD_frown,
    CMD_gasp, CMD_glare, CMD_groan, CMD_grope, CMD_hiccup, CMD_lick,
    CMD_love, CMD_moan, CMD_nibble, CMD_pout, CMD_purr, CMD_ruffle,
    CMD_shiver, CMD_shrug, CMD_sing, CMD_slap, CMD_smirk, CMD_snap,
    CMD_sneeze, CMD_snicker, CMD_sniff, CMD_snore, CMD_spit,
    CMD_squeeze, CMD_stare, CMD_strut, CMD_thank, CMD_twiddle,
    CMD_wave, CMD_whistle, CMD_wiggle, CMD_wink, CMD_yawn, CMD_snowball,
    CMD_hold, CMD_flee, CMD_sneak, CMD_hide, CMD_backstab, CMD_pick, CMD_steal,
    CMD_bash, CMD_rescue, CMD_kick, CMD_french, CMD_comb, CMD_massage,
    CMD_tickle, CMD_practice, CMD_study, CMD_pat, CMD_examine, CMD_take,
    CMD_info, CMD_TEMP, CMD_practise, CMD_curse, CMD_use, CMD_where,
    CMD_levels, CMD_reroll, CMD_pray, CMD_TEMP2, CMD_beg, CMD_cringe,
    CMD_cackle, CMD_fume, CMD_grovel, CMD_hop, CMD_nudge, CMD_peer, CMD_point,
    CMD_ponder, CMD_punch, CMD_snarl, CMD_spank, CMD_steam, CMD_tackle,
    CMD_taunt, CMD_think, CMD_whine, CMD_worship, CMD_yodel, CMD_brief,
    CMD_wizlist, CMD_consider, CMD_group, CMD_restore, CMD_return, CMD_switch,
    CMD_quaff, CMD_recite, CMD_users, CMD_pose, CMD_noshout, CMD_wizhelp,
    CMD_credits, CMD_compact, CMD_temp3, CMD_deafen, CMD_slay,
    CMD_wimpy, CMD_junk, CMD_deposit, CMD_withdraw, CMD_balance,
    CMD_nohassle, CMD_system, CMD_pull, CMD_stealth, CMD_edit,
    CMD_temp5, CMD_rsave, CMD_rload, CMD_track, CMD_siteban,
    CMD_highfive, CMD_title, CMD_whozone, CMD_assist, CMD_attribute,
    CMD_world, CMD_allspells, CMD_breath, CMD_show, CMD_debug,
    CMD_invisible, CMD_gain, CMD_rrload, CMD_daydream, CMD_disarm,
    CMD_bonk, CMD_chpwd, CMD_fill, CMD_imptest, CMD_shoot, CMD_silence,
    CMD_teams, CMD_player, CMD_create, CMD_bamfin, CMD_bamfout,
    CMD_vis, CMD_doorbash, CMD_mosh, CMD_alias, CMD_1, CMD_2, CMD_3,
    CMD_4, CMD_5, CMD_6, CMD_7, CMD_8, CMD_9, CMD_0, CMD_swim, CMD_spy,
    CMD_springleap, CMD_quiveringpalm, CMD_feigndeath, CMD_mount,
    CMD_dismount, CMD_ride, CMD_sign, CMD_set, CMD_firstaid, CMD_log,
    CMD_recall, CMD_event, CMD_disguise, CMD_climb, CMD_beep, CMD_bite,
    CMD_redit, CMD_display, CMD_resize, CMD_TEMP6, CMD_TEMP7, CMD_auth,
    CMD_noyell, CMD_gossip, CMD_noauction, CMD_auction, CMD_discon,
    CMD_freeze, CMD_drain, CMD_oedit, CMD_report, CMD_interven,
    CMD_gtell, CMD_raise, CMD_tap, CMD_liege, CMD_sneer, CMD_howl,
    CMD_kneel, CMD_finge, CMD_pace, CMD_tongue, CMD_flex, CMD_ack,
    CMD_eh, CMD_caress, CMD_cheer, CMD_jump, CMD_split, CMD_berserk, CMD_tan,
    CMD_memorize, CMD_find, CMD_bellow, CMD_camouflage, CMD_carve, CMD_nuke,
    CMD_skills, CMD_doorway, CMD_portal, CMD_summon, CMD_canibalize, CMD_flame,
    CMD_aura, CMD_great, CMD_psionicinvisibility, CMD_blast,
    CMD_psionicblast, CMD_medit, CMD_hypnotize, CMD_scry,
    CMD_adrenalize, CMD_brew, CMD_meditate, CMD_forcerent, CMD_warcry,
    CMD_layonhands, CMD_blessin, CMD_heroic, CMD_scan, CMD_shield,
    CMD_notell, CMD_commands, CMD_ghost, CMD_speak, CMD_setsev,
    CMD_esp, CMD_mail, CMD_check, CMD_receive, CMD_telepathy, CMD_mind,
    CMD_twist, CMD_turn, CMD_lift, CMD_push, CMD_zload, CMD_zsave,
    CMD_zclean, CMD_wrebuild, CMD_gwho, CMD_mforce, CMD_clone,
    CMD_fire, CMD_throw, CMD_run, CMD_notch, CMD_load, CMD_spot,
    CMD_view, CMD_afk, CMD_adore, CMD_agree, CMD_bleed, CMD_blink,
    CMD_blow, CMD_blame, CMD_bark, CMD_bhug, CMD_bcheck, CMD_boast,
    CMD_chide, CMD_compliment, CMD_ceyes, CMD_cears, CMD_cross,
    CMD_console, CMD_calm, CMD_cower, CMD_confess, CMD_drool, CMD_grit,
    CMD_greet, CMD_gulp, CMD_gloat, CMD_gaze, CMD_hum, CMD_hkiss,
    CMD_ignore, CMD_interrupt, CMD_knock, CMD_listen, CMD_muse,
    CMD_pinch, CMD_praise, CMD_plot, CMD_pie, CMD_pleade, CMD_pant,
    CMD_rub, CMD_roll, CMD_recoil, CMD_roar, CMD_relax, CMD_stroke,
    CMD_stretch, CMD_swave, CMD_sob, CMD_scratch, CMD_squirm, CMD_strangle,
    CMD_scowl, CMD_shudder, CMD_strip, CMD_scoff, CMD_salute, CMD_scold,
    CMD_stagger, CMD_toss, CMD_twirl, CMD_toast, CMD_tug, CMD_touch,
    CMD_tremble, CMD_twitch, CMD_whimper, CMD_whap, CMD_wedge, CMD_apologize,
    CMD_dmanage, CMD_drestrict, CMD_dlink, CMD_dunlink, CMD_dlist, CMD_dwho,
    CMD_dgossip, CMD_dtell, CMD_dthink, CMD_sending, CMD_messenger,
    CMD_promote, CMD_ooedit, CMD_whois, CMD_osave, CMD_msave, CMD_home,
    CMD_wizset, CMD_ooc, CMD_noooc, CMD_wiznoooc, CMD_homer, CMD_grumble,
    CMD_typoking, CMD_evilgrin, CMD_faint, CMD_ckiss, CMD_brb, CMD_ready,
    CMD_wolfwhistle, CMD_wizreport, CMD_lgos, CMD_groove, CMD_wdisplay,
    CMD_jam, CMD_donate, CMD_set_spy, CMD_reward, CMD_punish, CMD_spend,
    CMD_seepoints, CMD_bugmail, CMD_setobjmax, CMD_setobjspeed, CMD_wclean,
    CMD_glance, CMD_arena, CMD_startarena, CMD_whoarena, CMD_frolic,
    CMD_land, CMD_launch, CMD_disengage, CMD_prompt, CMD_talk,
    CMD_disagree, CMD_beckon, CMD_pounce, CMD_amaze, CMD_tank,
    CMD_hshake, CMD_backhand, CMD_surrender, CMD_collapse, CMD_wince,
    CMD_tag, CMD_trip, CMD_grunt, CMD_imitate, CMD_hickey, CMD_torture,
    CMD_addict, CMD_adjust, CMD_anti, CMD_bbl, CMD_beam, CMD_challenge,
    CMD_mutter, CMD_beat, CMD_moon, CMD_dream, CMD_shove, CMD_behead,
    CMD_pinfo , CMD_gosmsg, CMD_yell, CMD_legsweep, CMD_charge, CMD_orebuild,
    CMD_draw, CMD_zconv, CMD_bprompt, CMD_bid, CMD_resist,
    CMD_style, CMD_fight, CMD_iwizlist, CMD_flux, CMD_bs,
    CMD_autoassist, CMD_autoloot, CMD_autogold, CMD_autosplit,
    CMD_autosac, CMD_autoexits, CMD_train, CMD_mend, CMD_quest,
    CMD_qtrans, CMD_nooutdoor, CMD_dismiss, CMD_setsound,
    CMD_induct, CMD_expel, CMD_chat, CMD_qchat, CMD_clanlist,
    CMD_callsteed, CMD_top10, CMD_tweak, CMD_mrebuild, CMD_flurry,
    CMD_flowerfist, CMD_sharpen, CMD_eval, CMD_reimburse,
    CMD_remort, CMD_affects, CMD_board, CMD_disembark, CMD_embark,
    CMD_plank, CMD_weapons, CMD_allweapons, CMD_setwtype, CMD_init,
    CMD_hedit, CMD_chtextfile, CMD_retreat, CMD_zones, CMD_areas,
    CMD_recallhome, CMD_scribe
} CommandList;

struct map_coord {
    int             x;
    int             y;
    int             room;
    int             home;

};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
