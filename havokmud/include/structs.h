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
#if 0
typedef char    bool;
#endif
typedef char    byte;

#define MAX_MESSAGE_LENGTH 2048

#define ALL_DARK        1
#define FOREST_DARK     2
#define NO_DARK         3

#define ONE_WORD                  24
#define ONE_LINE                  96
#define ONE_PARAGRAPH            576
#define ONE_PAGE                3072

typedef enum {
    PULSE_COMMAND = 0,
    PULSE_TICK,
    EVENT_DEATH,
    EVENT_SUMMER,
    EVENT_SPRING,
    EVENT_FALL,
    EVENT_WINTER,
    EVENT_GATHER,
    EVENT_ATTACK,
    EVENT_FOLLOW,
    EVENT_MONTH,
    EVENT_BIRTH,
    EVENT_FAMINE,
    EVENT_DWARVES_STRIKE,
    EVENT_END_STRIKE,
    EVENT_END_FAMINE,
    EVENT_WEEK,
    EVENT_GOBLIN_RAID,
    EVENT_END_GOB_RAID
} Events_t;

#define DWARVES_STRIKE BV(0)
#define FAMINE         BV(1)

typedef enum {
    SEASON_WINTER = 0,
    SEASON_SPRING,
    SEASON_SUMMER,
    SEASON_FALL
} Seasons_t;

typedef struct alias_type {
    char           *com[10];    /* 10 aliases */
} Alias;


#define MAX_CLASS 12
#define OLD_MAX_CLASS 4

#define ABS_MAX_CLASS 20        /* USER FILE, DO NOT CHANGE! */
#define ABS_MAX_EXP   2000000000        /* used in polies/switching */

#define MAX_STAT 6              /* s i, w, d, co (ch) */

/*
 * the poofin and poofout mask bits
 */

#define BIT_POOF_IN     BV(0)
#define BIT_POOF_OUT    BV(1)


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
 * user flags
 */
#define NO_DELETE       BV(0)   /* do not delete me, well until 6 months
                                 * pass! */
#define USE_ANSI        BV(1)   /* we use ANSI color, yeah! */
#define RACE_WAR        BV(2)   /* if enabled they can be hit by
                                 * opposing race */
#define SHOW_EXITS      BV(3)   /* for auto display exits */

#define MURDER_1        BV(4)   /* actually killed someone! */
#define STOLE_1         BV(5)   /* caught stealing form someone! */
#define MURDER_2        BV(6)   /* rumored killed someone */
#define STOLE_2         BV(7)   /* rumored stole from someone */

#define USE_PAGING      BV(8)   /* pause screens? */
#define CAN_OBJ_EDIT    BV(9)   /* this user can use oedit to edit objects */
#define CAN_MOB_EDIT    BV(10)  /* this user can use medit to edit mobs */
#define FAST_AREA_EDIT  BV(11)  /* immort is using fast online mapping */
#define PKILLING        BV(12)
#define CLOAKED         BV(13)  /* Used for mortal cloaking of their eq */
#define ZONE_SOUNDS     BV(14)
#define CHAR_PRIVATE    BV(15)
#define NEW_USER        BV(17)
#define HERO_STATUS     BV(18)
#define GHOST           BV(19)
#define FAST_MAP_EDIT   BV(20)

#define OLD_COLORS      BV(21)

/*
 * system flags defined on the fly and by wizards for this boot
 */
#define SYS_NOPORTAL    BV(0)       /* no one can portal */
#define SYS_NOASTRAL    BV(1)       /* no one can astral */
#define SYS_NOSUMMON    BV(2)       /* no one can summon */
#define SYS_NOKILL      BV(3)       /* NO PC (good side or bad side) can 
                                     * fight */
#define SYS_LOGALL      BV(4)       /* log ALL users to the system log */
#define SYS_ECLIPS      BV(5)       /* the world is in constant darkness! */
#define SYS_SKIPDNS     BV(6)       /* skips DNS name searches on connects */
#define SYS_REQAPPROVE  BV(7)       /* force god approval for new char */
#define SYS_NOANSI      BV(8)       /* disable ansi colors world wide */
#define SYS_WIZLOCKED   BV(9)       /* System is Wizlocked for all */
#define SYS_NO_POLY     BV(10)      /* Spell Polymorph Self is disabled */
#define SYS_NOOOC       BV(11)      /* Provide from OOCing worldwird - Manwe */
#define SYS_LOCOBJ      BV(12)      /* Disable Locate Object - Manwe */
#define SYS_NO_DEINIT   BV(14)      /* makes zone not deinit */
#define SYS_NO_TWEAK    BV(15)      /* makes items not tweak */
#define SYS_ZONELOCATE  BV(16)      /* makes locate in zone only */

typedef enum {
    SPEAK_COMMON = 1,
    SPEAK_ELVISH,
    SPEAK_HALFLING,
    SPEAK_DWARVISH,
    SPEAK_ORCISH,
    SPEAK_GIANTISH,
    SPEAK_OGRE,
    SPEAK_GNOMISH,
    SPEAK_ALL,
    SPEAK_GODLIKE
} SpeakLanguages_t;

#define HATE_SEX   BV(0)
#define HATE_RACE  BV(1)
#define HATE_CHAR  BV(2)
#define HATE_CLASS BV(3)
#define HATE_EVIL  BV(4)
#define HATE_GOOD  BV(5)
#define HATE_VNUM  BV(6)

#define FEAR_SEX   BV(0)
#define FEAR_RACE  BV(1)
#define FEAR_CHAR  BV(2)
#define FEAR_CLASS BV(3)
#define FEAR_EVIL  BV(4)
#define FEAR_GOOD  BV(5)
#define FEAR_VNUM  BV(6)

typedef enum {
    OP_SEX = 1,
    OP_RACE,
    OP_CHAR,
    OP_CLASS,
    OP_EVIL,
    OP_GOOD,
    OP_VNUM
} OpinionType_t;

#define ABS_MAX_LVL  70
#define MAX_MORT     50
#define IMMORTAL     51
#define CREATOR      52
#define SAINT        53
#define DEMIGOD      54
#define LESSER_GOD   55
#define GOD          56
#define GREATER_GOD  57
#define SILLYLORD    58
#define IMPLEMENTOR  59
#define MAX_IMMORT   60
#define NEVER_USE    61

/*
 * Immune flags...
 */

#define IMM_FIRE    BV(0)
#define IMM_COLD    BV(1)
#define IMM_ELEC    BV(2)
#define IMM_ENERGY  BV(3)
#define IMM_BLUNT   BV(4)
#define IMM_PIERCE  BV(5)
#define IMM_SLASH   BV(6)
#define IMM_ACID    BV(7)
#define IMM_POISON  BV(8)
#define IMM_DRAIN   BV(9)
#define IMM_SLEEP   BV(10)
#define IMM_CHARM   BV(11)
#define IMM_HOLD    BV(12)
#define IMM_NONMAG  BV(13)
#define IMM_PLUS1   BV(14)
#define IMM_PLUS2   BV(15)
#define IMM_PLUS3   BV(16)
#define IMM_PLUS4   BV(17)

#define PULSE_RIVER         15
#define PULSE_TELEPORT      10
#define PULSE_MAILCHECK     240
#define PULSE_ADVICE        200
#define PULSE_DARKNESS      250
#define PULSE_TROLLREGEN    14  /* every once in a while */
#define PULSE_TQP           40  /* do a travel check for qp, every 10 sec */
#define PULSE_ZONE          240
#define PULSE_MOBILE        30
#define PULSE_VIOLENCE      16

#define WAIT_SEC       4
#define WAIT_ROUND     4

#define MAX_ROOMS           5000

/*
 * minimum # of zones needed to keep tqp alive
 */
#define MIN_INIT_TQP 22
#define TQP_AMOUNT    3         /* max number of tqps out and about */
#define TRAVELQP     26         /* vnum */
#define EMPTY_POTION 29
#define EMPTY_SCROLL 32
#define QUEST_POTION 27

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


#define OBJ_NOTIMER    -7000000
/*
 * ======================================================================
 */

/*
 * ========================================================================
 */

/*
 * The following defs and structures are related to char_data
 */

/*
 * For 'equipment'
 */

typedef enum {
    WEAR_LIGHT = 0,
    WEAR_FINGER_R,
    WEAR_FINGER_L,
    WEAR_NECK_1,
    WEAR_NECK_2,
    WEAR_BODY,
    WEAR_HEAD,
    WEAR_LEGS,
    WEAR_FEET,
    WEAR_HANDS,
    WEAR_ARMS,
    WEAR_SHIELD,
    WEAR_ABOUT,
    WEAR_WAIST,
    WEAR_WRIST_R,
    WEAR_WRIST_L,
    WIELD,
    HOLD,
    WEAR_BACK,
    WEAR_EAR_R,
    WEAR_EAR_L,
    WEAR_EYES,
    LOADED_WEAPON
} WearPos_t;
#define MAX_WEAR_POS   LOADED_WEAPON

/*
 * For 'char_player_data'
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
 * Predefined conditions
 */
#define MAX_CONDITIONS 5

typedef enum {
    DRUNK,
    FULL,
    THIRST
} Conditions_t;


/*
 * sex
 */
typedef enum {
    SEX_NEUTRAL = 0,
    SEX_MALE,
    SEX_FEMALE
} Sexes_t;

/*
 * positions
 */
typedef enum {
    POSITION_DEAD = 0,
    POSITION_MORTALLYW,
    POSITION_INCAP,
    POSITION_STUNNED,
    POSITION_SLEEPING,
    POSITION_RESTING,
    POSITION_SITTING,
    POSITION_FIGHTING,
    POSITION_STANDING,
    POSITION_MOUNTED
} Positions_t;


/*
 * For players : specials.act
 */
#define PLR_BRIEF       BV(0)
#define PLR_COMPACT     BV(1)
#define PLR_WIMPY       BV(2)   /* character will flee when seriously
                                 * injured */
#define PLR_NOHASSLE    BV(3)   /* char won't be attacked by aggressives. */
#define PLR_STEALTH     BV(4)   /* char won't be announced in a variety of
                                 * situations */
#define PLR_HUNTING     BV(5)   /* the player is hunting someone, do a
                                 * track each look */
#define PLR_DEAF        BV(6)   /* The player does not hear shouts */
#define PLR_ECHO        BV(7)   /* Messages (tells, shout,etc) echo back */
#define PLR_NOGOSSIP    BV(8)   /* New, gossip channel */
#define PLR_NOOOC       BV(10)  /* Can't hear the OOC channel */
#define PLR_NOSHOUT     BV(11)  /* the player is not allowed to shout */
#define PLR_NOTELL      BV(12)  /* The player does not hear tells */
#define PLR_POSTING     BV(13)  /* Posting to a board */
#define PLR_MAILING     BV(14)  /* Writing a message */
#define PLR_NOFLY       BV(15)  /* After 'land' the player will not fly
                                 * until 'launch' */
#define PLR_FREEZE      BV(16)  /* The player is frozen, must have pissed
                                 * an immo off */
#define PLR_WIZNOOOC    BV(17)  /* Been forbidden from OOCing by gods */
#define PLR_WIZREPORT   BV(18)
#define PLR_POLYSELF    BV(19)  /* matches ACT_POLYSELF */
#define PLR_NOBEEP      BV(20)  /* ignore all beeps */
#define PLR_HAVEROOM    BV(21)  /* Player is one of the first 100 heros
                                 * and have been assigned an homeroom */
#define PLR_NOOUTDOOR   BV(22)
#define PLR_AUTOASSIST  BV(23)
#define PLR_AUTOEXIT    BV(24)
#define PLR_AUTOLOOT    BV(25)
#define PLR_AUTOSAC     BV(26)
#define PLR_AUTOGOLD    BV(27)
#define PLR_AUTOSPLIT   BV(28)
#define PLR_LEGEND      BV(30)

/*
 * This structure is purely intended to be an easy way to transfer
 * and return information about time (real or mudwise).
 */
struct time_info_data {
    int             hours;
    int             day;
    int             month;
    int             year;
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

/** 
 * @todo implement the new max_level handling in gaining and loading 
 * @todo implement the new class_count handling in creation and loading
 */
struct char_player_data {
    Sexes_t         sex;        /* PC / NPC s sex */
    int             weight;     /* PC / NPC s weight */
    int             height;     /* PC / NPC s height */
    bool            talks[MAX_TOUNGE];  /* PC s Tounges 0 for NPC/not used
                                         * for languagesn */
    SpeakLanguages_t speaks;    /* current language speaking */

    char           *name;       /* PC / NPC s name (kill ...  ) */
    char           *short_descr;    /* for 'actions' */
    char           *long_descr; /* for 'look'.. Only here for testing */
    char           *description;    /* Extra descriptions */
    char           *title;      /* PC / NPC s title */
    char           *sounds;     /* Sound that the monster makes (in room) */
    char           *distant_snds;   /* Sound that the monster makes
                                     * (adjacent to room) */

    long            class;      /* PC s class or NPC alignment */
    long            user_flags; /* no delete, ansi etc... */
    long            extra_flags;    /* for resurrection in the future, etc */

    int             hometown;   /* PC s Hometown (zone) */

    struct time_data time;      /* PC s AGE in days */

    int             level[ABS_MAX_CLASS];   /* PC / NPC s level */
#ifndef TODO
    int             max_level;      /* Maximum level (no need to recalc all the 
                                     * time */
    int             class_count;    /* Number of classes had */
#endif
    int             q_points;       /* Quest points of player */
    bool            has_mail;   /* Has the player mail waiting? */
};

struct char_ability_data {
    int             str;
    int             str_add;    /* 000 - 100 if strength 18 */
    int             intel;
    int             wis;
    int             dex;
    int             con;
    int             chr;
};

struct char_point_data {
    int             mana;
    int             max_mana;
    int             mana_gain;

    int             hit;
    int             max_hit;    /* Max hit for NPC */
    int             hit_gain;

    int             move;
    int             max_move;   /* Max move for NPC */
    int             move_gain;

    int             armor;      /* Internal -100..100, external -10..10 AC */
    int             gold;       /* Money carried */
    int             bankgold;   /* gold in the bank.  */
    int             exp;        /* The experience of the player */
    int             leadership_exp;     /* leadership experience */

    int             hitroll;    /* Any bonus or penalty to the hit roll */
    int             damroll;    /* Any bonus or penalty to the damage roll */
};

struct char_special_data {
    int             spellfail;  /* max # for spell failure (101) */
    int             tick;       /* the tick that the mob/player is on */
    int             pmask;      /* poof mask */
    Positions_t     position;   /* Standing or ...  */
    Positions_t     default_pos;        /* Default position for NPC */
    int             spells_to_learn;    /* How many can you learn yet this
                                         * level */
    int             carry_items;        /* Number of items carried */
    int             last_direction;     /* The last direction the monster
                                         * went */
    int             sev;        /* log severity level for gods */

    int             start_room; /* so people can be set to start certain
                                 * places */
    int             edit;       /* edit state */

    int             mobtype;    /* mob type simple, A, L, B */
    int             exp_flag;   /* exp flag for this mob */
    int             hp_num_dice;        /* number of HPS dice */
    int             hp_size_dice;       /* size of HPS dice */
    int             hp_bonus_hps;       /* bonus hps number */

    int             damnodice;  /* The number of damage dice's */
    int             damsizedice;        /* The size of the damage dice's */

    int             dam_bonus;  /* damage bonus */
    int             medit;      /* mob edit menu at */
    struct char_data *mobedit;  /* mob editing */

    int             oedit;      /* obj editing menu at */
    struct obj_data *objedit;   /* object editing */

    int             tfd;
    struct edit_txt_msg *txtedit;       /* motd/wmotd/news editing */

    int             tick_to_lag;

    int             conditions[MAX_CONDITIONS]; /* Drunk full etc.  */
    int             permissions;
    int             zone;       /* zone that an NPC lives in */
    int             carry_weight;   /* Carried weight */
    int             timer;      /* Timer for update */
    int             was_in_room;    /* storage of location for
                                     * linkdead people */
    int             attack_type;    /* The Attack Type for NPC's */
    int             alignment;  /* +-1000 for alignments */

    char           *poofin;
    char           *poofout;
    char           *prompt;
    char           *bprompt;
    char           *email;      /* email address in aux */
    char           *immtitle;

    char           *rumor;
    char           *group_name; /* current group name if any... */
    char           *hostip;     /* keep track of IP */
    Alias          *A_list;
    struct char_data *misc;
    struct char_data *fighting; /* Opponent */

    struct char_data *hunting;  /* Hunting person..  */

    struct char_data *ridden_by;
    struct char_data *mounted_on;

    struct char_data *charging; /* we are charging this person */
    int             charge_dir; /* direction charging */

    bool            npc;
    bool            polyself;
    bool            script;
    bool            no_outdoor;
    bool            frozen;

    bool	    paralyzed;
    bool            blind;
    bool            true_sight;
    bool            detect_invis;

    bool            log_me;

    int             apply_saving_throw[MAX_SAVES];      /* Saving throw
                                                         * (Bonuses) */

    int             questwon;

    int             a_deaths;
    int             a_kills;
    int             m_deaths;
    long long       m_kills;

#if 1
    int proc;       /* flags for more common NPC behaviour */
#else
    CommonProcTypes_t proc;       /* flags for more common NPC behaviour
                                 * (shopkeeper, GM, swallower, drainer,
                                 * etc) */
#endif

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
#define SKILL_KNOWN                 BV(0)
#define SKILL_KNOWN_CLERIC          BV(1)
#define SKILL_KNOWN_MAGE            BV(2)
#define SKILL_KNOWN_SORCERER        BV(3)
#define SKILL_KNOWN_THIEF           BV(4)
#define SKILL_KNOWN_MONK            BV(5)
#define SKILL_KNOWN_DRUID           BV(6)
#define SKILL_KNOWN_WARRIOR         BV(7)
#define SKILL_KNOWN_BARBARIAN       BV(8)
#define SKILL_KNOWN_PALADIN         BV(9)
#define SKILL_KNOWN_RANGER          BV(10)
#define SKILL_KNOWN_PSI             BV(11)
#define SKILL_KNOWN_NECROMANCER     BV(12)

/*
 * skill_data special
 */
#define SKILL_SPECIALIZED       BV(0)
#define SKILL_UNDEFINED         BV(1)
#define SKILL_UNDEFINED2        BV(2)

struct char_skill_data {
    int             learned;    /* % chance for success 0 = not learned */
    long            flags;      /* SKILL KNOWN? bit settings */
    long            special;    /* spell/skill specializations */
    int             nummem;     /* number of times this spell is memorized */
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
    int             nr;         /* monster nr */
    int             playerId;   /* ID number of the player */
    int             in_room;    /* Location */
    int             reroll;     /* Number of rerolls still availiable */
    int             term;
    int             size;
    struct last_checked last;   /* For displays */
    long            immune;     /* Immunities */
    long            M_immune;   /* Meta Immunities */
    long            susc;       /* susceptibilities */
    float           mult_att;   /* the number of attacks */
    int             attackers;
    int             sector;     /* which part of a large room am i in? */
    int             generic;    /* generic int */
    int             commandp;   /* command poitner for scripts */
    int             waitp;      /* waitp for scripts */
    int             commandp2;  /* place-holder for gosubs, etc. */
    int             script;

    int             race;
    int             hunt_dist;  /* max dist the player can hunt */

    long            hatefield;
    long            fearfield;

    Opinion         hates;
    Opinion         fears;

    int             persist;
    int             old_room;

    void           *act_ptr;    /* numeric argument for the mobile actions */

    struct char_player_data player;     /* Normal data */
    struct char_ability_data abilities; /* Abilities */
    struct char_ability_data tmpabilities;      /* The abilities we use */
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

#if TODO
    struct descriptor_data *desc;       /* NULL for mobiles */
#endif
    void *playerDesc;
    struct char_data *orig;     /* Special for polymorph */

    struct char_data *next_in_room;     /* For room->people - list */
    struct char_data *next;     /* all in game list */
    struct char_data *next_fighting;    /* For fighting list */
    struct char_data *next_memorize;    /* For list of people memorizing */

    struct follow_type *followers;      /* List of chars followers */
    struct char_data *master;   /* Who is char following? */
    int             invis_level;        /* visibility of gods */
    char           *last_tell;          /* Who last telled something to
                                         * the char */
    int             old_exp;    /* For energe restore -MW */
    char            pwd[12];    /* password */
};

/*
 * ========================================================================
 */

/*
 * How much light is in the land ?
 */

typedef enum {
    SUN_DARK = 0,
    SUN_RISE,
    SUN_LIGHT,
    SUN_SET,
    MOON_SET,
    MOON_RISE
} WeatherLight_t;

/*
 * And how is the sky ?
 */

typedef enum {
    SKY_CLOUDLESS = 0,
    SKY_CLOUDY,
    SKY_RAINING,
    SKY_LIGHTNING
} WeatherSky_t;

struct weather_data {
    int             pressure;   /* How is the pressure ( Mb ) */
    int             change;     /* How fast and what way does it change. */
    WeatherSky_t    sky;        /* How is the sky. */
    WeatherLight_t  sunlight;   /* And how much sun. */
};


/*
 ***********************************************************************
 *  file element for object file. BEWARE: Changing it will ruin the file  *
 *********************************************************************** */


#define MAX_OBJ_SAVE 200        /* Used in OBJ_FILE_U *DO*NOT*CHANGE* */
#ifndef MAX_OBJ_AFFECT
#include "newstructs.h"
#endif

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


/*
 ***********************************************************
 *  The following structures are related to descriptor_data   *
 *********************************************************** */


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

struct bulletin_board_message {
    char           *author;
    char           *title;
    char           *text;
    time_t          date;
    int             message_id;
    int             reply_to;
};

struct board_def {
    int                 boardId;
    int                 vnum;
    int                 minLevel;
    int                 messageCount;
    struct obj_data    *obj;
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
    char           *output;
    struct char_data *character;        /* linked to char */
    struct char_data *original; /* original char */
    struct snoop_data snoop;    /* to snoop people.  */
    struct descriptor_data *next;       /* link to next descriptor */

    char           *list_string;        /* for building very large buffers
                                         * for page_string */
    int             list_size;  /* keeps track of strlen of list_string */

    struct bulletin_board_message *msg; /* For posting discussion messages
                                         */
    struct board_def *board;       /* To keep track of the board
                                    * we're posting to */

};


/************************************************************/

typedef void    (*funcp) ();


struct edit_txt_msg {
    int             file;       /* 0 = none, 1 = news, 2 = motd, 3 = wmotd */
    char           *date;
    char           *author;
    char           *body;
};


struct command_def {
    char           *name;
    void          (*func)();
    int             number;
    int             min_pos;
    int             min_lev;
};

struct class_level_t {
    int             thaco;
    char           *title_m;
    char           *title_f;
    long            exp;
};

struct class_def {
    char           *abbrev;
    char           *name;
    struct class_level_t *levels;
    int             levelCount;
};

#define MSG_SKILL_WEAR_OFF              0
#define MSG_SKILL_WEAR_OFF_ROOM         1
#define MSG_SKILL_WEAR_OFF_SOON         2
#define MSG_SKILL_WEAR_OFF_SOON_ROOM    3
#define MSG_SKILL_DESCRIPTIVE           4
#define SKILL_MSG_COUNT                 5

#define MSG_FIGHT_DIE_ATTACKER          0
#define MSG_FIGHT_DIE_VICTIM            1
#define MSG_FIGHT_DIE_ROOM              2
#define MSG_FIGHT_MISS_ATTACKER         3
#define MSG_FIGHT_MISS_VICTIM           4
#define MSG_FIGHT_MISS_ROOM             5
#define MSG_FIGHT_HIT_ATTACKER          6
#define MSG_FIGHT_HIT_VICTIM            7
#define MSG_FIGHT_HIT_ROOM              8
#define MSG_FIGHT_GOD_ATTACKER          9
#define MSG_FIGHT_GOD_VICTIM            10
#define MSG_FIGHT_GOD_ROOM              11
#define FIGHT_MSG_COUNT                 12

struct skill_def {
    int            skillId;
    int            skillType;
    char          *name;
    char          *message[SKILL_MSG_COUNT];
};


struct message_list {
    int             a_type;     /* Attack type */
    char           *msg[FIGHT_MSG_COUNT];
};


struct pc_race_choice {
    int             raceNum;
    const int      *classesAvail;
    int             classCount;
};

struct dir_data {
    int             forward;
    int             rev;
    int             trap;
    char           *exit;
    char           *listexit;
    char           *dir;
    char           *desc;
};

struct sector_data {
    char           *type;
    char           *mapChar;
    int             moveLoss;
};

struct banned_user {
    int             how;
    char           *name;
    int             len;
};

#define MSG_SOCIAL_NO_ARG_SELF      0
#define MSG_SOCIAL_NO_ARG_ROOM      1
#define MSG_SOCIAL_ARG_SELF         2
#define MSG_SOCIAL_ARG_ROOM         3
#define MSG_SOCIAL_ARG_VICTIM       4
#define MSG_SOCIAL_ARG_NO_VICTIM    5
#define MSG_SOCIAL_ARG_AUTO_SELF    6
#define MSG_SOCIAL_ARG_AUTO_ROOM    7
#define MSG_SOCIAL_ARG_OBJ_SELF     8
#define MSG_SOCIAL_ARG_OBJ_ROOM     9
#define SOCIAL_MSG_COUNT            10

struct social_messg {
    int             act_nr;
    int             hide;
    int             min_victim_position;
    char           *msg[SOCIAL_MSG_COUNT];
};


struct pose_type {
    /*
     * minimum level for poser
     */
    int             level;

    char          **poser_msg;
    char          **room_msg;
};

/*
 * Race -- Npc, otherwise 
 */
#define RACE_HALFBREED 0
#define RACE_HUMAN     1
#define RACE_MOON_ELF  2
#define RACE_DWARF     3
#define RACE_HALFLING  4
#define RACE_ROCK_GNOME 5
#define RACE_REPTILE  6
#define RACE_SPECIAL  7
#define RACE_LYCANTH  8
#define RACE_DRAGON   9
#define RACE_UNDEAD   10
#define RACE_ORC      11
#define RACE_INSECT   12
#define RACE_ARACHNID 13
#define RACE_DINOSAUR 14
#define RACE_FISH     15
#define RACE_BIRD     16
#define RACE_GIANT    17        /* generic giant more specials down ---V */
#define RACE_PREDATOR 18
#define RACE_PARASITE 19
#define RACE_SLIME    20
#define RACE_DEMON    21
#define RACE_SNAKE    22
#define RACE_HERBIV   23
#define RACE_TREE     24
#define RACE_VEGGIE   25
#define RACE_ELEMENT  26
#define RACE_PLANAR   27
#define RACE_DEVIL    28
#define RACE_GHOST    29
#define RACE_GOBLIN   30
#define RACE_TROLL    31
#define RACE_VEGMAN   32
#define RACE_MFLAYER  33
#define RACE_PRIMATE  34
#define RACE_ENFAN    35
#define RACE_DROW     36
#define RACE_GOLEM    37
#define RACE_SKEXIE   38
#define RACE_TROGMAN  39
#define RACE_PATRYN   40
#define RACE_LABRAT   41
#define RACE_SARTAN   42
#define RACE_TYTAN    43
#define RACE_SMURF    44
#define RACE_ROO      45
#define RACE_HORSE    46
#define RACE_DRAAGDIM 47
#define RACE_ASTRAL   48
#define RACE_GOD      49

#define RACE_GIANT_HILL   50
#define RACE_GIANT_FROST  51
#define RACE_GIANT_FIRE   52
#define RACE_GIANT_CLOUD  53
#define RACE_GIANT_STORM  54
#define RACE_GIANT_STONE  55

#define RACE_DRAGON_RED    56
#define RACE_DRAGON_BLACK  57
#define RACE_DRAGON_GREEN  58
#define RACE_DRAGON_WHITE  59
#define RACE_DRAGON_BLUE   60
#define RACE_DRAGON_SILVER 61
#define RACE_DRAGON_GOLD   62
#define RACE_DRAGON_BRONZE 63
#define RACE_DRAGON_COPPER 64
#define RACE_DRAGON_BRASS  65

#define RACE_UNDEAD_VAMPIRE 66
#define RACE_UNDEAD_LICH    67
#define RACE_UNDEAD_WIGHT   68
#define RACE_UNDEAD_GHAST   69
#define RACE_UNDEAD_SPECTRE 70
#define RACE_UNDEAD_ZOMBIE  71
#define RACE_UNDEAD_SKELETON 72
#define RACE_UNDEAD_GHOUL    73
#define RACE_HALF_ELF   74
#define RACE_HALF_OGRE    75
#define RACE_HALF_ORC     76
#define RACE_HALF_GIANT   77
#define RACE_LIZARDMAN 78
#define RACE_DARK_DWARF 79
#define RACE_DEEP_GNOME 80
#define RACE_GNOLL      81
#define RACE_GOLD_ELF   82
#define RACE_WILD_ELF   83
#define RACE_SEA_ELF    84
#define RACE_FOREST_GNOME 85
#define RACE_AVARIEL      86
#define RACE_ETTIN        87
#define RACE_COCKATRICE   88
#define RACE_COCODILE     89
#define RACE_BASILISK     90
#define RACE_GARGOYLE     91
#define RACE_DRIDER       92
#define RACE_DISPLACER_BEAST 93
#define RACE_GRIFFON      94
#define RACE_HELL_HOUND   95
#define RACE_HYDRA        96
#define RACE_MARGOYLE     97
#define RACE_MASTODON     98
#define RACE_MEDUSA       99
#define RACE_MINOTAUR     100
#define RACE_MOBAT        101
#define RACE_OTYUGH       102
#define RACE_NEO_OTYUGH   103
#define RACE_REMORHAZ     104
#define RACE_PURPLE_WORM  105
#define RACE_PHASE_SPIDER 106
#define RACE_SPHINX       107
#define RACE_WARG         108
#define RACE_WYVERN       109
#define RACE_UMBER_HULK   110
#define RACE_SLUG         111
#define RACE_BULETTE      112
#define RACE_CARRION_CRAWLER 113
#define RACE_DRACOLISK    114
#define RACE_BANSHEE      115
#define RACE_BEHOLDER     116
#define RACE_DEATH_TYRANT 117
#define RACE_DRACOLICH    118
#define RACE_RAKSHASA     119
#define RACE_BANE_MINION  120
#define RACE_ANKHEGS      121
#define RACE_BUGBEARS     122
#define RACE_EFREET       123
#define RACE_HARPIES      124
#define RACE_MANTICORES   125
#define RACE_SCRAGS       126
#define RACE_OWLBEARS     127
#define RACE_SQUID        128
#define RACE_TIGER        129
#define RACE_WIGHTS       130
#define RACE_YETI         131
#define RACE_DRAGON_AMETHYST    132
#define RACE_DRAGON_CRYSTAL     133
#define RACE_DRAGON_EMERALD     134
#define RACE_DRAGON_SAPPHIRE    135
#define RACE_DRAGON_TOPAZ       136
#define RACE_DRAGON_BROWN       137
#define RACE_DRAGON_CLOUD       138
#define RACE_DRAGON_DEEP        139
#define RACE_DRAGON_MERCURY     140
#define RACE_DRAGON_MIST        141
#define RACE_DRAGON_SHADOW      142
#define RACE_DRAGON_STEEL       143
#define RACE_DRAGON_YELLOW      144
#define RACE_DRAGON_TURTLE      145

struct race_type {
    int             race;
    char           *racename;
    char           *desc;

    int            *racialMax;

    int             size;

    int             start;
    int             young;
    int             mature;
    int             middle;
    int             old;
    int             ancient;
    int             venerable;

    int             nativeLanguage;
};


struct lang_def {
    int             langSpeaks;
    int             langSkill;
    char           *name;
};

#define REPORT_HELP         1
#define REPORT_BUG          2
#define REPORT_IDEA         3
#define REPORT_TYPO         4
#define REPORT_WIZBUG       5
#define REPORT_WIZIDEA      6
#define REPORT_WIZTYPO      7
#define REPORT_QUEST        8
#define REPORT_WIZHELP      9
#define REPORT_MOTD         10
#define REPORT_WMOTD        11

struct user_report {
    void               *res;
    int                 resCount;
    char               *timestamp;
    char               *character;
    int                 roomNum;
    char               *report;
};

/*
 * minimum level a player must be to send mail 
 */
#define MIN_MAIL_LEVEL 2

/*
 * # of gold coins required to send mail 
 */
#define STAMP_PRICE 150

/*
 * Maximum size of mail in bytes (arbitrary) 
 */
#define MAX_MAIL_SIZE 4000


#define HELP_MORTAL   1
#define HELP_IMMORTAL 2


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
