/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include "structs.h"
#include "trap.h"
#include "spells.h"
#include "utils.h"


const char     *ItemDamType[] = {
    "burned",
    "frozen",
    "electrified",
    "crushed",
    "corroded",
    "burned",
    "frozen",
    "shredded",
    "\n"
};

const char     *weekdays[7] = {
    "the Day of the Sword",
    "the Day of the Bow",
    "the Day of the Axe",
    "the Day of the Mace",
    "the Day of Battle",
    "the day of Grace",
    "the Day of Peace"
};

const char     *month_name[17] = {
    "Month of Winter",          /* 0 */
    "Month of the Winter Wolf",
    "Month of the Frost Giant",
    "Month of the Old Forces",
    "Month of the Grand Struggle",
    "Month of the Spring",
    "Month of Nature",
    "Month of Fertility",
    "Month of the Dragon",
    "Month of the Sun",
    "Month of the Heat",
    "Month of the Battle",
    "Month of the God Wars",
    "Month of the Shadows",
    "Month of the Long Shadows",
    "Month of the Ancient Darkness",
    "Month of the Great Evil"
};

const int       sharp[] = {
    0,
    0,
    0,
    1,                          /* Slashing */
    0,
    0,
    0,
    0,                          /* Bludgeon */
    0,
    0,
    0,
    0
};                              /* Pierce */

const char     *where[] = {
    "$c000B<$c000wused as light$c000B>$c000w      :",
    "$c000B<$c000wworn on finger$c000B>$c000w     :",
    "$c000B<$c000wworn on finger$c000B>$c000w     :",
    "$c000B<$c000wworn around neck$c000B>$c000w   :",
    "$c000B<$c000wworn around neck$c000B>$c000w   :",
    "$c000B<$c000wworn on body$c000B>$c000w       :",
    "$c000B<$c000wworn on head$c000B>$c000w       :",
    "$c000B<$c000wworn on legs$c000B>$c000w       :",
    "$c000B<$c000wworn on feet$c000B>$c000w       :",
    "$c000B<$c000wworn on hands$c000B>$c000w      :",
    "$c000B<$c000wworn on arms$c000B>$c000w       :",
    "$c000B<$c000wworn as shield$c000B>$c000w     :",
    "$c000B<$c000wworn about body$c000B>$c000w    :",
    "$c000B<$c000wworn about waist$c000B>$c000w   :",
    "$c000B<$c000wworn around wrist$c000B>$c000w  :",
    "$c000B<$c000wworn around wrist$c000B>$c000w  :",
    "$c000B<$c000wwielded$c000B>$c000w            :",
    "$c000B<$c000wheld$c000B>$c000w               :",
    "$c000B<$c000wworn on back$c000B>$c000w       :",
    "$c000B<$c000wworn in right ear$c000B>$c000w  :",
    "$c000B<$c000wworn in left ear$c000B>$c000w   :",
    "$c000B<$c000wworn on eyes$c000B>$c000w       :",
    "$c000B<$c000wnotched$c000B>$c000w            :"
};

const char     *drinks[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "dark ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local speciality",
    "slime mold juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt water",
    "coca cola",
    "\n"
};

const char     *drinknames[] = {
    "water",
    "beer",
    "wine",
    "ale",
    "ale",
    "whisky",
    "lemonade",
    "firebreather",
    "local",
    "juice",
    "milk",
    "tea",
    "coffee",
    "blood",
    "salt",
    "cola",
    "\n"
};


/*
 * fire cold elec blow acid
 */

int             ItemSaveThrows[22][5] = {
    {15, 2, 10, 10, 10},
    {19, 2, 16, 2, 7},
    {11, 2, 2, 13, 9},
    {7, 2, 2, 10, 8},
    {6, 2, 2, 7, 13},
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {6, 2, 2, 7, 13},           /* treasure */
    {6, 2, 2, 7, 13},           /* armor */
    {7, 6, 2, 20, 5},           /* potion */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {10, 10, 10, 10, 10},       /* not defined */
    {19, 2, 2, 16, 7},
    {7, 6, 2, 20, 5},           /* drinkcon */
    {6, 2, 2, 7, 13},
    {6, 3, 2, 3, 10},
    {6, 2, 2, 7, 13},           /* treasure */
    {11, 2, 2, 13, 9},
    {7, 2, 2, 10, 8}
};

const int       drink_aff[][3] = {
    {0, 1, 10},                 /* Water */
    {3, 2, 5},                  /* beer */
    {5, 2, 5},                  /* wine */
    {2, 2, 5},                  /* ale */
    {1, 2, 5},                  /* ale */
    {6, 1, 4},                  /* Whiskey */
    {0, 1, 8},                  /* lemonade */
    {10, 0, 0},                 /* firebr */
    {3, 3, 3},                  /* local */
    {0, 4, -8},                 /* juice */
    {0, 3, 6},
    {0, 1, 6},
    {0, 1, 6},
    {0, 2, -1},
    {0, 1, -2},
    {0, 1, 5},
    {0, 0, 0}
};

const char     *color_liquid[] = {
    "clear",
    "brown",
    "clear",
    "brown",
    "dark",
    "golden",
    "red",
    "green",
    "clear",
    "light green",
    "white",
    "brown",
    "black",
    "red",
    "clear",
    "black",
    "\n"
};

const char     *fullness[] = {
    "less than half ",
    "about half ",
    "more than half ",
    ""
};


const char     *item_types[] = {
    "UNDEFINED",                /* 0 */
    "LIGHT",
    "SCROLL",
    "WAND",
    "STAFF",
    "WEAPON",                   /* 5 */
    "FIRE WEAPON",
    "MISSILE",
    "TREASURE",
    "ARMOR",
    "POTION",                   /* 10 */
    "WORN",
    "OTHER",
    "TRASH",
    "TRAP",
    "CONTAINER",                /* 15 */
    "NOTE",
    "LIQUID CONTAINER",
    "KEY",
    "FOOD",
    "MONEY",                    /* 20 */
    "PEN",
    "BOAT",
    "AUDIO",
    "BOARD",
    "TREE",                     /* 25 */
    "ROCK",
    "PORTAL",                   /* 27 */
    "INSTRUMENT",
    "SHIPS HELM",
    "\n"
};

const char     *wear_bits[] = {
    "TAKE",
    "FINGER",
    "NECK",
    "BODY",
    "HEAD",
    "LEGS",
    "FEET",
    "HANDS",
    "ARMS",
    "SHIELD",
    "ABOUT",
    "WAIST",
    "WRIST",
    "BACK",
    "EARS",
    "EYES",
    "LIGHT-SOURCE",
    "HOLD",
    "WIELD",
    "THROW",
    "\n"
};

const char     *extra_bits[] = {
    "GLOW",
    "HUM",
    "METAL",
    "MINERAL",
    "ORGANIC",
    "INVISIBLE",
    "MAGIC",
    "NODROP",
    "BRITTLE",
    "RESISTANT",
    "ARTIFACT",
    "RARE",
    "UBERRARE",
    "QUEST",
    "\n"
};


const char     *anti_bits[] = {
    "ANTI-SUN",
    "ANTI-GOOD",
    "ANTI-EVIL",
    "ANTI-NEUTRAL",
    "ANTI-MEN",
    "ANTI-WOMEN",
    "\n"
};

const char     *anti_class_bits[] = {
    "ONLY-CLASS",
    "ANTI-MAGE",
    "ANTI-CLERIC",
    "ANTI-WARRIOR",
    "ANTI-THIEF",
    "ANTI-DRUID",
    "ANTI-MONK",
    "ANTI-BARBARIAN",
    "ANTI-SORCERER",
    "ANTI-PALADIN",
    "ANTI-RANGER",
    "ANTI-PSIONIST",
    "ANTI-NECROMANCER",
    "\n"
};

const char     *room_bits[] = {
    "DARK",
    "DEATH",
    "NO_MOB",
    "INDOORS",
    "PEACEFUL",
    "NOSTEAL",
    "NO_SUM",
    "NO_MAGIC",
    "TUNNEL",
    "PRIVATE",
    "SILENCE",
    "LARGE",
    "NO_DEATH",
    "SAVE_ROOM",
    "ARENA_ROOM",
    "NO_FLY",
    "REGEN_ROOM",
    "FIRE_ROOM",
    "ICE_ROOM",
    "WIND_ROOM",
    "EARTH_ROOM",
    "ELECTRIC_ROOM",
    "WATER_ROOM",
    "MOVE_ROOM",
    "MANA_ROOM",
    "NO_FLEE",
    "NO_SPY",
    "EVER_LIGHT",
    "ROOM_MAP",
    "\n"
};

const char     *exit_bits[] = {
    "IS-DOOR",
    "CLOSED",
    "LOCKED",
    "SECRET",
    "RSLOCKED",
    "PICKPROOF",
    "CLIMB",
    "\n"
};


const char     *equipment_types[] = {
    "Special",
    "Worn on right finger",
    "Worn on left finger",
    "First worn around Neck",
    "Second worn around Neck",
    "Worn on body",
    "Worn on head",
    "Worn on legs",
    "Worn on feet",
    "Worn on hands",
    "Worn on arms",
    "Worn as shield",
    "Worn about body",
    "Worn around waist",
    "Worn around right wrist",
    "Worn around left wrist",
    "Wielded",
    "Held",
    "Worn on back",
    "Worn in right ear",
    "Worn in left ear",
    "Worn on face",
    "Load in weapon",
    "\n"
};

const char     *affected_bits[] = { "Blind",
    "Invisible",
    "Detect Evil",
    "Detect Invisible",
    "Detect Magic",
    "Sense Life",
    "Hold",
    "Sanctuary",
    "Dragon Ride",
    "Growth",
    "Curse",
    "Flying",
    "Poison",
    "Tree Travel",
    "Paralysis",
    "Infravision",
    "Water Breath",
    "Sleep",
    "Travelling",
    "Sneak",
    "Hide",
    "Silence",
    "Charm",
    "Darkness",
    "Protect from Evil",
    "True Sight",
    "Scrying",
    "Fireshield",
    "Group",
    "Telepathy",
    "Chillshield",
    "Blade barrier",
    "\n"
};

const char     *affected_bits2[] = {
    "Invisible to animals",
    "Heat Stuff",
    "Logging",
    "Berserk",
    "Group-Order",
    "AWAY-FROM-KEYBOARD",
    "Detect Good",
    "Protection from Good",
    "Follow",
    "Haste",
    "Slow",
    "Wings burned",
    "Style Berserked",
    "Quest",
    "No Outdoor Logs",
    "Wings tired",
    "Invis to Undead",
    "Guardian Angel",
    "Song of the Wanderer",
    "Sneak Skill",
    "\n"
};

const char     *special_user_flags[] = {
    "NO-DELETE",
    "USE-ANSI",
    "RACE-WARRIOR",
    "Undef-2",
    "SHOW-EXITS",
    "MURDER-1",
    "STOLE-1",
    "MURDER-2",
    "STOLE-2",
    "MURDER-X",
    "STOLE-X",
    "PAGE-PAUSE",
    "OBJECT-EDITOR-OK",
    "MOBILE-EDITOR-OK",
    "FAST-EDIT",
    "PKILLING",
    "CLOAKED",
    "ZONE-SOUNDS",
    "CHAR-PRIVATE",
    "CLAN-LEADER",
    "NEW-USER",
    "HERO-STATUS",
    "GHOST",
    "FAST-MAP",
    "\n"
};

const char     *immunity_names[] = { 
    "Fire",
    "Cold",
    "Electricity",
    "Energy",
    "Blunt",
    "Pierce",
    "Slash",
    "Acid",
    "Poison",
    "Drain",
    "Sleep",
    "Charm",
    "Hold",
    "Non-Magic",
    "+1",
    "+2",
    "+3",
    "+4",
    "\n"
};

const char     *apply_types[] = {
    "NONE",
    "STR",
    "DEX",
    "INT",
    "WIS",
    "CON",
    "CHR",
    "SEX",
    "LEVEL",
    "AGE",
    "CHAR_WEIGHT",
    "CHAR_HEIGHT",
    "MANA",
    "HIT",
    "MOVE",
    "GOLD",
    "SPELL AFF2",
    "ARMOR",
    "HITROLL",
    "DAMROLL",
    "SAVING_PARA",
    "SAVING_ROD",
    "SAVING_PETRI",
    "SAVING_BREATH",
    "SAVING_SPELL",
    "SAVING_ALL",
    "RESISTANCE",
    "SUSCEPTIBILITY",
    "IMMUNITY",
    "SPELL AFFECT",
    "WEAPON SPELL",
    "EAT SPELL",
    "BACKSTAB",
    "KICK",
    "SNEAK",
    "HIDE",
    "BASH",
    "PICK",
    "STEAL",
    "TRACK",
    "HIT-N-DAM",
    "SPELLFAIL",
    "ATTACKS",
    "HASTE",
    "SLOW",
    "OTHER",
    "FIND-TRAPS",
    "RIDE",
    "RACE-SLAYER",
    "ALIGN-SLAYER",
    "MANA-REGEN",
    "HIT-REGEN",
    "MOVE-REGEN",
    "MOD-THIRST",
    "MOD-HUNGER",
    "MOD-DRUNK",
    "T_STR",
    "T_INT",
    "T_DEX",
    "T_WIS",
    "T_CON",
    "T_CHR",
    "T_HPS",
    "T_MOVE",
    "T_MANA",
    "\n"
};

const char     *npc_class_types[] = {
    "Normal",
    "Undead",
    "\n"
};

const char     *system_flag_types[] = {
    "NO-PORTAL",
    "NO-ASTRAL",
    "NO-SUMMON",
    "NO-KILL",
    "LOG-ALL",
    "ECLIPSE",
    "NO-DNS",
    "REQ-APPROVAL",
    "NO-COLOR",
    "WIZLOCKED",
    "NO_POLY",
    "NO-OOC",
    "NO-LOCOBJ",
    "WLD-ARENA",
    "NO-DEINIT",
    "NO-TWEAK",
    "ZONE-LOCATE",
    "\n"
};

const char     *action_bits[] = {
    "SPEC",
    "SENTINEL",
    "SCAVENGER",
    "ISNPC",
    "NICE-THIEF",
    "AGGRESSIVE",
    "STAY-ZONE",
    "WIMPY",
    "ANNOYING",
    "HATEFUL",
    "AFRAID",
    "IMMORTAL",
    "HUNTING",
    "DEADLY",
    "POLYMORPHED",
    "META_AGGRESSIVE",
    "GUARDING",
    "NECROMANCER",
    "HUGE",
    "SCRIPT",
    "GREET",
    "MAGIC-USER",
    "WARRIOR",
    "CLERIC",
    "THIEF",
    "DRUID",
    "MONK",
    "BARBARIAN",
    "PALADIN",
    "RANGER",
    "PSIONIST",
    "\n"
};

const char     *procedure_bits[] = {
    "None",
    "Shopkeeper",
    "Guildmaster",
    "Swallower",
    "Drainer",
    "Quest",
    "Old Breath Weapon",
    "Fire Breather",
    "Gas Breather",
    "Frost Breather",
    "Acid Breather",
    "Electric Breather",
    "Dehydration Breather",
    "Vapor Breather",
    "Sound Breather",
    "Shard Breather",
    "Sleep Breather",
    "Light Breather",
    "Dark Breather",
    "Receptionist",
    "Repair Guy",

    "\n"
};

const char     *player_bits[] = {
    "Brief",
    "NoShout",
    "Compact",
    "DontSet",
    "Wimpy",
    "NoHassle",
    "Stealth",
    "Hunting",
    "Deaf",
    "Echo",
    "NoYell",
    "NoAuction",
    "Posting",                 /* Old NODIMD flag it seems */
    "NoOoc",
    "NoShout",
    "Freeze",
    "NoTell",
    "NoFly",
    "WizNoOoc",                 /* Doesn't seem to work TEB */
    "WizReport",
    "NoBeep",
    "HaveRoom",
    "Mailing",
    "NoOutdoor",
    "Autoassist",
    "Autoexit",
    "Autoloot",
    "Autosac",
    "Autogold",
    "Autosplit",
    "ClanLeader",
    "Legend",
    "Empty12",
    "\n"
};

const char     *position_types[] = {
    "Dead",
    "Mortally wounded",
    "Incapacitated",
    "Stunned",
    "Sleeping",
    "Resting",
    "Sitting",
    "Fighting",
    "Standing",
    "\n"
};

const char     *fight_styles[] = {
    "Standard",
    "Berserked",
    "Aggressive",
    "Defensive",
    "Evasive",
    "\n"
};

const char     *tfd_types[] = {
    "None Selected",
    "News",
    "Message of the Day",
    "Wizard's Mesasge of the Day",
    "\n"
};

const char     *connected_types[] = {
    "Playing",
    "Get name",
    "Confirm name",
    "Read Password",
    "Get new password",
    "Confirm new password",
    "Get sex",
    "Read messages of today",
    "Read Menu",
    "Get extra description",
    "Get class",
    "Link Dead",
    "New Password",
    "Password Confirm",
    "Wizlocked",
    "Get Race",
    "Racpar",
    "Auth",
    "City Choice",
    "Stat Order",
    "Delete",
    "Delete",
    "Stat Order",
    "Wizard MOTD",
    "Editing",
    "Nuking Theirself",
    "Mage/Sorcerer Question",
    "Object Editing",
    "Mob Editing",
    "Re-roll",
    "Press Enter",
    "Already Playing",
    "Getting Ansii",
    "Creation Menu",
    "Picking Alignment",
    "Help File Editing",
    "_e-mail reg",
    "Text File Editing",
    "\n"
};

/*
 * [ch] strength apply (all)
 */
const struct str_app_type str_app[31] = {
    {-5, -4, 0, 0},             /* 0 */
    {-5, -4, 1, 1},             /* 1 */
    {-3, -2, 1, 2},
    {-3, -1, 5, 3},             /* 3 */
    {-2, -1, 10, 4},
    {-2, -1, 20, 5},            /* 5 */
    {-1, 0, 25, 6},
    {-1, 0, 30, 7},
    {0, 0, 40, 8},
    {0, 0, 50, 9},
    {0, 0, 55, 10},             /* 10 */
    {0, 0, 70, 11},
    {0, 0, 80, 12},
    {0, 0, 90, 13},
    {0, 0, 100, 14},
    {0, 0, 110, 15},            /* 15 */
    {0, 1, 120, 16},
    {1, 1, 130, 18},
    {1, 2, 140, 20},            /* 18 */
    {3, 7, 485, 40},
    {3, 8, 535, 40},            /* 20 */
    {4, 9, 635, 40},
    {4, 10, 785, 40},
    {5, 11, 935, 40},
    {6, 12, 1235, 40},
    {7, 14, 1535, 40},          /* 25 */
    {1, 3, 155, 22},            /* 18/01-50 */
    {2, 3, 170, 24},            /* 18/51-75 */
    {2, 4, 185, 26},            /* 18/76-90 */
    {2, 5, 255, 28},            /* 18/91-99 */
    {3, 6, 355, 30}             /* 18/100 (30) */
};

/*
 * [dex] skillapply (thieves only)
 */
const struct dex_skill_type dex_app_skill[26] = {
    {-99, -99, -90, -99, -60},  /* 0 */
    {-90, -90, -60, -90, -50},  /* 1 */
    {-80, -80, -40, -80, -45},
    {-70, -70, -30, -70, -40},
    {-60, -60, -30, -60, -35},
    {-50, -50, -20, -50, -30},  /* 5 */
    {-40, -40, -20, -40, -25},
    {-30, -30, -15, -30, -20},
    {-20, -20, -15, -20, -15},
    {-15, -10, -10, -20, -10},
    {-10, -5, -10, -15, -5},    /* 10 */
    {-5, 0, -5, -10, 0},
    {0, 0, 0, -5, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},            /* 15 */
    {0, 5, 0, 0, 0},
    {5, 10, 0, 5, 5},
    {10, 15, 5, 10, 10},
    {15, 20, 10, 15, 15},
    {15, 20, 10, 15, 15},       /* 20 */
    {20, 25, 10, 15, 20},
    {20, 25, 15, 20, 20},
    {25, 25, 15, 20, 20},
    {25, 30, 15, 25, 25},
    {25, 30, 15, 25, 25}        /* 25 */
};

/*
 * [level] backstab multiplyer (thieves only)
 */
const byte      backstab_mult[ABS_MAX_LVL] = {
    1,                          /* 0 */
    2,                          /* 1 */
    2,
    2,
    2,
    2,                          /* 5 */
    2,
    2,
    3,                          /* 8 */
    3,
    3,                          /* 10 */
    3,
    3,
    3,
    3,
    3,                          /* 15 */
    4,                          /* 16 */
    4,
    4,
    4,
    4,                          /* 20 */
    4,
    4,
    4,
    5,                          /* 25 */
    5,
    5,
    5,
    5,
    5,                          /* 30 */
    5,
    5,
    6,
    6,
    6,                          /* 35 */
    6,
    6,
    6,
    7,
    7,                          /* 40 */
    7,
    7,
    7,
    8,
    8,                          /* 45 */
    8,
    8,
    8,
    9,
    10,                         /* 50? */
    11,
    11,
    12,
    12,
    13,
    14,
    15,
    16,
    17,
    18,                         /* 60 */
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26,
    27,
    28                          /* 70 */
};
/*
 * [level]backstab multiplier for non primary thieves (Gordon 11JAN04)
 */
const byte      lesser_backstab_mult[ABS_MAX_LVL] = {
    1,                          /* 0 */
    2,                          /* 1 */
    2,
    2,
    2,
    2,                          /* 5 */
    2,
    2,
    2,                          /* 8 */
    3,
    3,                          /* 10 */
    3,
    3,
    3,
    3,
    3,                          /* 15 */
    3,                          /* 16 */
    4,
    4,
    4,
    4,                          /* 20 */
    4,
    4,
    4,
    4,                          /* 25 */
    4,
    4,
    4,
    4,
    4,                          /* 30 */
    5,
    5,
    5,
    5,
    5,                          /* 35 */
    5,
    5,
    5,
    5,
    6,                          /* 40 */
    6,
    6,
    6,
    6,
    6,                          /* 45 */
    6,
    6,
    7,
    7,
    7,                          /* 50? */
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,                         /* 60 */
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
    25,
    26                          /* 70 */
};

/*
 * [dex] apply (all)
 */
struct dex_app_type dex_app[41] = {
    {-7, -7, 60},               /* 0 */
    {-6, -6, 50},               /* 1 */
    {-4, -4, 50},
    {-3, -3, 40},
    {-2, -2, 30},
    {-1, -1, 20},               /* 5 */
    {0, 0, 10},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},                  /* 10 */
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, -10},                /* 15 */
    {1, 1, -20},
    {2, 2, -30},
    {2, 2, -40},
    {3, 3, -40},
    {3, 3, -40},                /* 20 */
    {4, 4, -50},
    {4, 4, -50},
    {4, 4, -50},
    {5, 5, -60},
    {5, 5, -60}                 /* 25 */

};

/*
 * [con] apply (all)
 */
struct con_app_type con_app[26] = {
    {-4, 20},                   /* 0 */
    {-3, 25},                   /* 1 */
    {-2, 30},
    {-2, 35},
    {-1, 40},
    {-1, 45},                   /* 5 */
    {-1, 50},
    {0, 55},
    {0, 60},
    {0, 65},
    {0, 70},                    /* 10 */
    {0, 75},
    {0, 80},
    {0, 85},
    {0, 88},
    {1, 90},                    /* 15 */
    {2, 95},                    /* 16 */
    {3, 97},                    /* 17 */
    {4, 99},                    /* 18 */
    {4, 99},
    {5, 99},                    /* 20 */
    {6, 99},
    {6, 99},
    {7, 99},
    {8, 99},
    {9, 100}                    /* 25 */
};

/*
 * [int] apply (all)
 */
struct int_app_type int_app[26] = {
    {0},
    {1},                          /* 1 */
    {2},
    {3},
    {4},
    {5},                          /* 5 */
    {6},
    {8},
    {10},
    {12},
    {14},                         /* 10 */
    {16},
    {18},
    {20},
    {22},
    {25},                         /* 15 */
    {28},
    {32},
    {35},
    {40},
    {45},                         /* 20 */
    {50},
    {60},
    {70},
    {80},
    {99}                          /* 25 */
};

struct int_app_type int_sf_modifier[26] = {
    {60},
    {50},                         /* 1 */
    {42},
    {35},
    {29},
    {24},                         /* 5 */
    {19},
    {15},
    {11},
    {8},
    {5},                          /* 10 */
    {3},
    {2},
    {1},
    {0},
    {-1},                         /* 15 */
    {-2},
    {-4},
    {-8},
    {-16},
    {-25},                        /* 20 */
    {-50},
    {-50},
    {-50},
    {-50},
    {-50}                         /* 25 */
};

/*
 * [wis] apply (all)
 */
struct wis_app_type wis_app[26] = {
    {0},
    {0},
    {0},
    {1},
    {1},
    {1},
    {1},
    {1},
    {2},
    {2},
    {3},                          /* 10 */
    {3},                          /* 11 */
    {3},                          /* 12 */
    {4},                          /* 13 */
    {4},                          /* 14 */
    {5},                          /* 15 */
    {5},                          /* 16 */
    {6},                          /* 17 */
    {7},                          /* 18 */
    {7},
    {7},
    {7},
    {7},
    {7},
    {7},
    {8}
};

struct chr_app_type chr_apply[26] = {
    {0, -70},                   /* 0 */
    {0, -70},
    {1, -60},
    {1, -50},
    {1, -40},                   /* 4 */
    {2, -30},
    {2, -20},
    {3, -10},
    {4, 0},
    {5, 0},                     /* 9 */
    {6, 0},
    {7, 0},
    {8, 0},
    {9, +5},                    /* 13 */
    {10, +10},
    {12, +15},
    {14, +20},
    {17, +25},
    {20, +30},                  /* 18 */
    {20, +40},
    {25, +50},
    {25, +60},
    {25, +70},
    {25, +80},
    {25, +90},
    {25, +95},
};

const struct QuestItem QuestList[4][IMMORTAL] = {
    {
        {2402, "An unusual way of looking at things.\n\r"},
        {1, "It can be found in the donation room or on your head.\n\r"},
        {1410, "It's a heavy bag of white powder.\n\r"},
        {6010, "You can make pies out of them, deer seem to like them "
               "too.\n\r"},
        {3013, "It's a yummy breakfast food, which goes great with eggs and "
               "cheese.\n\r"},
        {20, "If you twiddle your thumbs enough you'll find one.\n\r"},
        {24764, "Dead people might wear them on their hands.\n\r"},
        {112, "If you found one of these, it would be mighty strange!\n\r"},
        {106, "Eye of Toad and Toe of Frog, bring me one or the other.\n\r"},
        {109, "A child's favorite place holds the object of my desire, on the "
              "dark river.\n\r"},
        /*
         * 10 
         */
        {3628, "The latest in New padded footwear.\n\r"},
        {113, "A child might play with one when the skexies aren't "
              "around.\n\r"},
        {19204, "A precious moon in a misty castle.\n\r"},
        {20006, "Are you a fly? You might run into one of these, "
                "beware...\n\r"},
        {1109, "Little people have tiny weapons...bring me one.\n\r"},
        {6203, "IReallyReallyWantACurvedBlade.\n\r"},
        {21007, "I want to be taller and younger, find the nasty "
                "children.\n\r"},
        {5228, "Don't you find screaming women so disarming?\n\r"},
        {7204, "Vaulted lightning.\n\r"},
        {16043, "Precious elements can come in dull, dark mines.\n\r"},
        /*
         * 20 
         */
        {20007, "You'll catch him napping, no guardian of passing time.\n\r"},
        {16903, "Nature's mistake, carried by a man on a new moon, fish on "
                "full.\n\r"},
        {5226, "Sealed in the hands of a city's failed guardian.\n\r"},
        {10900, "Anachronistic rectangular receptacle holds circular "
                "plane.\n\r"},
        {13840, "What kind of conditioner does one use for asps?\n\r"},
        {7406, "If you don't bring a scroll of recall, you might die a fiery "
               "death.\n\r"},
        {120, "Dock down anchor.\n\r"},
        {21008, "Very useful, behind a hearth.\n\r"},
        {10002, "He didn't put them to sleep with these, the results were "
                "quite deadly.\n\r"},
        {3648, "Unsummoned they pose large problems. What you want is on "
               "their queen.\n\r"},
        /*
         * 30
         */
        {15805, "A single sample of fine plumage held by a guard and a "
                "ghost.\n\r"},
        {21141, "In the land of the troglodytes there is a headpiece with "
                "unpleasant powers.\n\r"},
        {1532, "Three witches have the flighty component you need.\n\r"},
        {5304, "A spectral force holds the key to advancement in a geometric "
               "dead end.\n\r"},
        {9496, "A great golden sword was once taken by the giants of the great"
               " north. Return it to me.\n\r"},
        {5105, "What you need is as dark as the heart of the elf who wields "
               "it.\n\r"},
        {21011, "The key to your current problem is in Orshingal on a haughty"
                " strutter.\n\r"},
        {11011, "Brave Sir Robin seems to have experienced a fright!\n\r"},
        {6616, "You might smile if drinking a can of this. Look in "
               "Prydain.\n\r"},
        {21125, "With enough of this strong amber drink you'd forget about the"
                " nightly ghosts.\n\r"},
        /*
         * 40 
         */
        {5309, "Powerful items of magic follow... first bring me a medallion "
               "of mana.\n\r"},
        {1585, "Bubble bubble toil and trouble, bring me a staff on the "
               "double.\n\r"},
        {21003, "I need some good boots, you know how strange it is to find "
                "them.\n\r"},
        {13704, "Watch for a dragon, find his ring.\n\r"},
        {252, "I don't want a normal cloak, maybe one that's a little "
              "distorted.\n\r"},
        {3670, "Bottled mana is what I need.\n\r"},
        {1104, "The master of fireworks, take his silver.\n\r"},
        {5020, "You're not a real mage until you get platinum from "
               "purple.\n\r"},
        {1599, "Grand Major.\n\r"},
        {20002, "She's hiding in her web, but she has my hat!\n\r"}
    },
    /*
     * 50
     */
    {
        {1401, "I was thinking about going fishing but don't have any gear, "
               "can you help?\n\r"},
        {41257, "Some new guys told me that a Dwarven Warrior has a pair of "
                "these.\n\r"},
        {1110, "White and young, with no corners or sides, a golden treasure "
               "can be found inside.\n\r"},
        {3070, "The armorer might have a pair, but they're only his third best "
               "on hand.\n\r"},
        {3057, "Judicandus dies.\n\r"},
        {6001, "I want clothes I can play chess on.\n\r"},
        {16033, "A goblin's favorite food, around the eastern path.\n\r"},
        {107, "Every righteous cleric should have one, but few of them "
              "do.\n\r"},
        {4000, "I have a weakness for cheap rings.\n\r"},
        {3025, "Cleaver for hire, just outside of Karsinya.\n\r"},
        /*
         * 60 
         */
        {3649, "My wife needs something New, to help keep her girlish "
               "figure.\n\r"},
        {7202, "Mindflayers have small ones, especially in the sewers.\n\r"},
        {19203, "The weapon of a traitor, lost in a fog.\n\r"},
        {15814, "Beautiful as can be, take what this gelfling friend "
                "offers.\n\r"},
        {119, "Play with a gypsy child when he asks you to or else!\n\r"},
        {5012, "You might use these to kill a vampire, they are in the "
               "desert.\n\r"},
        {6809, "Really cool sunglasses on a really cool guy in a really cool "
               "place.\n\r"},
        {17021, "J-E-L-L-O!\n\r"},
        {3648, "Giant women have great fashion sense.\n\r"},
        {27001, "A hideously ugly woman making a hideously disgusting "
                "brew.\n\r"},
        /*
         * 70
         */
        {105, "A venomed bite will end your life - you need the jaws that do "
              "it.\n\r"},
        {3668, "Buy some wine from a fortuneteller's brother.\n\r"},
        {1703, "On a cat, but not in the petting zoo.\n\r"},
        {13758, "Held by a doggie with who will bite you and bite you and "
                "bite you.\n\r"},
        {5240, "In the old city you'll find the accursed vibrant stone you "
               "require.\n\r"},
        {5013, "Where can you go for directions in the desert? Try the wet "
               "spot.\n\r"},
        {17011, "An unholy symbol on an unholy creature under a fuming "
                "mountain.\n\r"},
        {1708, "The color of the bird that carries it.\n\r"},
        {9203, "What would you use to swat a very large mosquito? Get it from "
               "the giants.\n\r"},
        {21109, "A bow made of dark wood, carried by a troglodyte.\n\r"},
        /*
         * 80 
         */
        {15817, "In an secret cold place, a dark flower is carried by a "
                "midnight one.\n\r"},
        {9430, "Argent Ursa, Crevasse of the Arctic Enlargements.\n\r"},
        {6112, "If you would be king, you'd need one. With a wooded "
               "wyrm.\n\r"},
        {1758, "Carried by a hag in the dark lake under the sewers.\n\r"},
        {27411, "This Roo's better than you and she has the stick to prove "
                "it.\n\r"},
        {5317, "The dead don't lie still when properly prepared.\n\r"},
        {5033, "You can get it off a drider, but he won't give it to you.\n\r"},
        {16615, "South of a bay, past a portal, into a tower, be "
                "prepared.\n\r"},
        {121, "To the far southeast, in the lair of a pair of arachnids.\n\r"},
        {13901, "On the shore, down the river from the troll-bridge.\n\r"},
        /*
         * 90 
         */
        {5104, "Four heads are better than one.\n\r"},
        {15806, "You don't stand a ghost of a chance against a glow of white "
                "and a cloak of fire.\n\r"},
        {16022, "A powerful, blunt, and extremely fragile weapon.\n\r"},
        {122, "The sole possession of a devil down-under.\n\r"},
        {7220, "The highest thing at the top of a chain.\n\r"},
        {13785, "From the fairest.\n\r"},
        {1597, "Mana in a green ring.\n\r"},
        {1563, "Enchanting, blue and very rare.\n\r"},
        {5001, "Search for a banded male.\n\r"},
        {20003, "Ensnared for power, she holds the helmet of the wise.\n\r"}
    },
    /*
     * 100
     */
    {
        {2402, "An unusual way of looking at things.\n\r"},
        {11, "Something you might find in the donation room, or on your "
             "body.\n\r"},
        {16034, "Goblins have been known to play with these, especially in "
                "dark caves.\n\r"},
        {6000, "A decent weapon, just the right size for a tree.\n\r"},
        {24760, "Dead men's feet look like this.\n\r"},
        {1413, "You were SUPPOSED to bell the CAT!\n\r"},
        {18256, "In the city of Mordilnia, a shield of Roy G. Biv.\n\r"},
        {8121, "A bag that opens with a ripping sound.\n\r"},
        {108, "Floating for safety on the dark.\n\r"},
        {123, "A mule stole my hat, now he fights in front of an "
              "audience.\n\r"},
        /*
         * 110 
         */
        {3621, "Thank goodness when I broke my arm I still had my New "
               "shield.\n\r"},
        {117, "If you get this, someone will be quite howling angry.\n\r"},
        {7405, "Sewer secret light sources.\n\r"},
        {6205, "My eyes just aren't as fast to focus as they used to be.\n\r"},
        {4051, "These warriors seem scarred, but its all in their head."},
        {5219, "Fresh deer...yum!\n\r"},
        {16015, "An ugly bird in the mountains once told me this: 'A despotic "
                "ruler rules with one of these.'\n\r"},
        {1718, "Hey, that's not a painting at all, but boy is it ugly! In the "
               "new city.\n\r"},
        {5032, "Guarded by a brass dragon, bring me this precious stone.\n\r"},
        {3685, "Mightier than a sword, wielded by a four man.\n\r"},
        /*
         * 120 
         */
        {5100, "Learn humility: I want a common sword.\n\r"},
        {16902, "They'd all be normal in a moonless world. You need to steal a "
                "silver stole.\n\r"},
        {17022, "A lion with a woman's torso holds the book you need.\n\r"},
        {5206, "To hold the girth of a corpulent man it must be ferrous. In "
               "the old city hall.\n\r"},
        {1737, "In the hands of an elf with a green thumb, trying to learn as "
               "much New material as possible.\n\r"},
        {5306, "My mommy gave me a knife, but i lost it in the pyramid.\n\r"},
        {21006, "Childlike, maybe, but they're not children. You need the "
                "locked up cloth.\n\r"},
        {9204, "The largest in the hands of the largest of the large.\n\r"},
        {1721, "Get the toolbook of the trade from the royal cook in the new "
               "city.\n\r"},
        {16901, "Only an elephant's might be as big as this boar's "
                "mouthpiece.\n\r"},
        /*
         * 130 
         */
        {6511, "A bearded woman might be so engaged, but a guard's got this "
               "one.\n\r"},
        {5101, "Dark elves can be noble too, but they won't let you take their"
               " arms.\n\r"},
        {1761, "In a suspended polygon, in a chest which is not.\n\r"},
        {15812, "You think that water can't be sharp? Look under "
                "birdland.\n\r"},
        {16046, "A miner's tool in the dwarven mines.\n\r"},
        {21114, "These skeletal beasts will reel you in, you want the "
                "crowbar.\n\r"},
        {13762, "Once in Hades, the key to getting out lies with a long dead "
                "warrior.\n\r"},
        {20005, "This usurper will think you very tasty, defeat him for the "
                "thing you need.\n\r"},
        {5019, "A nasty potion in the hands of an even nastier desert "
               "worm.\n\r"},
        {10002, "This item can be found on 'Al'\n\r"},
        /*
         * 140 
         */
        {5221, "Weapons are the keys to many quests. First, bring me a Stone "
               "golem's sword.\n\r"},
        {9442, "The weapon of the largest giant in the rift.\n\r"},
        {15808, "Weapon of champions.\n\r"},
        {13775, "By the light of this sharp moon.\n\r"},
        {21004, "By name, you can assume it's the largest weapon in the "
                "game.\n\r"},
        {3092, "He's always pissed, and so are his guards. Take his weapon "
               "and make it yours.\n\r"},
        {5002, "The weapon of the oldest wyrm.\n\r"},
        {5107, "One Two Three Four Five Six.\n\r"},
        {1430, "It rises from the ashes, and guards a tower.\n\r"},
        {5021, "You're not a REAL fighter until you've had one of these "
               "enchanted.\n\r"}
    },
    /*
     * 150 
     */
    {
        {2402, "An unusual way of looking at things.\n\r"},
        {4, "You might find one of these in the donation room, or in your "
            "hand.\n\r"},
        {3071, "They're the best on hand for 5 coins.\n\r"},
        {30, "At the wrong end of a nasty spell, or a heavy hitter.\n\r"},
        {3012, "I'm hungry, get me something to eat, and make it a foreign "
               "food!\n\r"},
        {24767, "I've heard that skeletons love bleach.\n\r"},
        {6006, "Nearly useless in a hearth.\n\r"},
        {4104, "It's what makes kobolds green.\n\r"},
        {42, "Do she-devils steal, as they flap their bat wings?\n\r"},
        {19202, "Animal light, lost in a fog.\n\r"},
        /*
         * 160 
         */
        {3647, "These New boots were made for walking.\n\r"},
        {4101, "Hands only a warrior could love.\n\r"},
        {116, "Near the eastern road, it only grows on the north side of "
              "rocks.\n\r"},
        {111, "Only a fool would look at the end of the river.\n\r"},
        {15812, "I'd love a really cool backstabbing weapon...Make sure it "
                "doesn't melt.\n\r"},
        {17023, "Being charming can be offensive, especially in a plumed "
                "white cap.\n\r"},
        {9205, "You could hide a giant in this stuff.\n\r"},
        {10002, "feeling tired and fuzzy?  Exhibit some stealth, or you just "
                "might get eaten.\n\r"},
        {3690, "I am an old man, but I will crush you at chess.\n\r"},
        {5000, "Find the dark elves. Pick something silver.\n\r"},
        /*
         * 170 
         */
        {15802, "It's easy work to work a rejected bird for the means to his "
                "former home.\n\r"},
        {1750, "In the twisted forest of the Graecians a man in a black cloak "
               "has it.\n\r"},
        {5012, "Vampire's bane in a wicker basket near a desert pool.\n\r"},
        {20008, "The toothless dragon eats the means to your advancement.\n\r"},
        {6810, "You are everywhere you look in this frozen northern maze of "
               "ice.\n\r"},
        {255, "Get the happy stick from this hick.\n\r"},
        {7190, "In a secret sewer place a squeaking rodent wears a "
               "trinket.\n\r"},
        {7205, "The master flayer under the city has it on him, but not in "
               "use. Steal it!\n\r"},
        {7230, "You could be stoned for skinning this subterranean reptilian "
               "monster.\n\r"},
        {3690, "An old man at the park might have one, but these old men are "
               "in the new city.\n\r"},
        /*
         * 180 
         */
        {1729, "In the forest north of the new city a traveller lost his way. "
               "It's on him.\n\r"},
        {1708, "It's growing on a cliff face, on the way to the lost "
               "kingdom.\n\r"},
        {1759, "The moon's phase can change a man. Find the badger in a "
               "tavern.\n\r"},
        {1718, "Getting a tan doesn't mean you haven't been studying at all. "
               "Check out the history of the New city.\n\r"},
        {5243, "I hope it is clear which stone you will need.\n\r"},
        {5302, "In a hanging desert artifact, the softest golem has the key to"
               " your success.\n\r"},
        {21008, "If your dog were this ugly, you'd lock him in a fireplace "
                "too! Bring me what he's guarding.\n\r"},
        {9206, "It can be electrifying scaling a dragon; a big guy must have "
               "done it.\n\r"},
        {6524, "The dwarven mazekeeper has the only pair, if you can find "
               "him.\n\r"},
        {1533, "Three witches in the mage's tower have the orb you need.\n\r"},
        /*
         * 190 
         */
        {9425, "A huge gemstone, guarded by ice toads, beware their "
               "poison.\n\r"},
        {5113, "The weapon of a maiden, shaped like a goddess.\n\r"},
        {21014, "The dagger of a yellow-belly.\n\r"},
        {5037, "A thief of great reknown, at least he doesn't use a "
               "spoon.\n\r"},
        {1101, "Elven blade of ancient lore, matches insects blow for "
               "blow.\n\r"},
        {27000, "Grithas can be annoying, even jagged sometimes.\n\r"},
        {27409, "The weapon of a primitive man, just right for killing his "
                "mortal foe.\n\r"},
        {1594, "White wielded by white, glowing white.\n\r"},
        {20001, "He judges your soul, he wields a weapon that can pierce it "
                "too.\n\r"},
        {13703, "Watch for a dragon, he wears that which you seek.\n\r"}
    }
    /*
     * 200 
     */
};

const int       preproomexitsquad1[] = {
    39901,
    39941,
    39935,
    39949,
    -1
};

const int       preproomexitsquad2[] = {
    40096,
    39991,
    40009,
    39982,
    -1
};

const int       preproomexitsquad3[] = {
    40011,
    40047,
    40018,
    40026,
    -1
};

const int       preproomexitsquad4[] = {
    40048,
    40075,
    40061,
    40081,
    -1
};

/*
 * Lennya: Let's make these for all classes, preferably in alphabetical
 * order for skills command.  Perhaps make a similar listing called
 * pracset warriorprac[] for GM purposes? would be easy to get spells at
 * GM in spell order (new spells at bottom).
 */
const struct skillset loreskills[] = {
    /*
     * lores
     */
    {"animal lore", SKILL_CONS_ANIMAL, 1, 94},
    {"demonology", SKILL_CONS_DEMON, 1, 94},
    {"giant lore", SKILL_CONS_GIANT, 1, 94},
    {"necromancy", SKILL_CONS_UNDEAD, 1, 94},
    {"other lore", SKILL_CONS_OTHER, 1, 94},
    {"people lore", SKILL_CONS_PEOPLE, 1, 94},
    {"reptile lore", SKILL_CONS_REPTILE, 1, 94},
    {"vegetable lore", SKILL_CONS_VEGGIE, 1, 94},
    {"read magic", SKILL_READ_MAGIC, 1, 94},
    {"sign language", SKILL_SIGN, 1, 94},
    /*
     * languages
     */
    {"common", LANG_COMMON, 1, 94},
    {"dwarvish", LANG_DWARVISH, 1, 94},
    {"elvish", LANG_ELVISH, 1, 94},
    {"giantish", LANG_GIANTISH, 1, 94},
    {"gnomish", LANG_GNOMISH, 1, 94},
    {"halfling", LANG_HALFLING, 1, 94},
    {"ogre", LANG_OGRE, 1, 94},
    {"orcish", LANG_ORCISH, 1, 94},
    {"None", -1, -1, -1}
};
const int loreskillCount = NELEMENTS(loreskills) - 1;

const struct skillset archerskills[] = {
    {"spot", SKILL_SPOT, 1, 94},
    {"None", -1, -1, -1}
};
const int archerskillCount = NELEMENTS(archerskills) - 1;

/*
 * divided ninjaskills into 3 parts, seems easier
 */
const struct skillset thfninjaskills[] = {
    {"climb", SKILL_CLIMB, 1, 94},
    {"disguise", SKILL_HIDE, 1, 94},
    {"None", -1, -1, -1}
};
const int thfninjaskillCount = NELEMENTS(thfninjaskills) - 1;

const struct skillset warninjaskills[] = {
    {"disarm", SKILL_DISARM, 1, 94},
    {"doorbash", SKILL_DOORBASH, 1, 94},
    {"None", -1, -1, -1}
};
const int warninjaskillCount = NELEMENTS(warninjaskills) - 1;

const struct skillset allninjaskills[] = {
    {"riding", SKILL_RIDE, 1, 94},
    {"disengage", SKILL_DISENGAGE, 1, 94},
    {"None", -1, -1, -1}
};
const int allninjaskillCount = NELEMENTS(allninjaskills) - 1;

const struct skillset warmonkskills[] = {
    {"dodge", SKILL_DODGE, 1, 94},
    {"retreat", SKILL_RETREAT, 1, 45},
    {"switch opponents", SKILL_SWITCH_OPP, 1, 45},
    {"None", -1, -1, -1}
};
const int warmonkskillCount = NELEMENTS(warmonkskills);


const char     *AttackType[] = {
    "Smite",
    "Stab",                     /* 1 */
    "Whip",
    "Slash",
    "Smash",
    "Cleave",
    "Crush",
    "Pound",
    "Claw",
    "Bite",
    "Sting",                    /* 10 */
    "Pierce",
    "Blast",
    "Impale",
    "Range Weapon",
    "Jab",
    "Punch",
    "Strike",
    "Undefined",                /* test for 0 */
    "\n"
};

const struct affect_list oedit_list[] = {
    {"Blindness", AFF_BLIND, 1},
    {"Curse", AFF_CURSE, 1},
    {"Detect Invisible", AFF_DETECT_INVISIBLE, 1},
    {"Detect Magic", AFF_DETECT_MAGIC, 1},
    {"Detect Evil", AFF_DETECT_EVIL, 1},
    {"Detect Good", AFF_DETECT_GOOD, 1},
    {"Sense Life", AFF_SENSE_LIFE, 1},
    {"Infravision", AFF_INFRAVISION, 1},
    {"True Sight", AFF_TRUE_SIGHT, 1},
    {"Invisibility", AFF_INVISIBLE, 1},
    {"Sneak", AFF_SNEAK, 1},
    {"Hide", AFF_HIDE, 1},
    {"Prot. from Evil", AFF2_PROTECT_FROM_EVIL, 2},
    {"Prot. from Good", AFF2_PROTECT_FROM_GOOD, 2},
    {"Growth", AFF_GROWTH, 1},
    {"Sanctuary", AFF_SANCTUARY, 1},
    {"Fireshield", AFF_FIRESHIELD, 1},
    {"Flying", AFF_FLYING, 1},
    {"Travelling", AFF_TRAVELLING, 1},
    {"Tree Travel", AFF_TREE_TRAVEL, 1},
    {"Water Breath", AFF_WATERBREATH, 1},
    {"Poison", AFF_POISON, 1},
    {"Sleep", AFF_SLEEP, 1},
    {"Silence", AFF_SILENCE, 1},
    {"Paralysis", AFF_PARALYSIS, 1},
    {"Scrying", AFF_SCRYING, 1},
    {"Telepathy", AFF_TELEPATHY, 1},
    {"Berserk", AFF2_BERSERK, 2},
    {"Invis to Animals", AFF2_ANIMAL_INVIS, 2}
};

const struct skillset styleskillset[] = {
    {"standard", STYLE_STANDARD, 1, 94},
    {"berserked", STYLE_BERSERKED, 10, 94},
    {"aggressive", STYLE_AGGRESSIVE, 10, 94},
    {"defensive", STYLE_DEFENSIVE, 10, 94},
    {"evasive", STYLE_EVASIVE, 10, 94},
    {"None", -1, -1, -1}
};
const int styleskillCount = NELEMENTS(styleskillset);

const struct skillset weaponskills[] = {
    {"short sword", WEAPON_SHORT_SWORD, 1, 45},
    {"long sword", WEAPON_LONG_SWORD, 1, 45},
    {"broadsword", WEAPON_BROADSWORD, 1, 45},
    {"bastard sword", WEAPON_BASTARD_SWORD, 1, 45},
    {"two-handed sword", WEAPON_TWO_HANDED_SWORD, 1, 45},
    {"sickle", WEAPON_SICKLE, 1, 45},
    {"scythe", WEAPON_SCYTHE, 1, 45},
    {"scimitar", WEAPON_SCIMITAR, 1, 45},
    {"rapier", WEAPON_RAPIER, 1, 45},
    {"sabre", WEAPON_SABRE, 1, 45},
    {"katana", WEAPON_KATANA, 1, 45},
    {"wakizashi", WEAPON_WAKIZASHI, 1, 45},
    {"dagger", WEAPON_DAGGER, 1, 45},
    {"knife", WEAPON_KNIFE, 1, 45},
    {"stiletto", WEAPON_STILETTO, 1, 45},
    {"spear", WEAPON_SPEAR, 1, 45},
    {"pike", WEAPON_PIKE, 1, 45},
    {"ranseur", WEAPON_RANSEUR, 1, 45},
    {"naginata", WEAPON_NAGINATA, 1, 45},
    {"halberd", WEAPON_HALBERD, 1, 45},
    {"lucern hammer", WEAPON_LUCERN_HAMMER, 1, 45},
    {"trident", WEAPON_TRIDENT, 1, 45},
    {"fork", WEAPON_FORK, 1, 45},
    {"axe", WEAPON_AXE, 1, 45},
    {"hand axe", WEAPON_HAND_AXE, 1, 45},
    {"two-handed axe", WEAPON_TWO_HANDED_AXE, 1, 45},
    {"pick", WEAPON_PICK, 1, 45},
    {"hammer", WEAPON_HAMMER, 1, 45},
    {"club", WEAPON_CLUB, 1, 45},
    {"great club", WEAPON_GREAT_CLUB, 1, 45},
    {"mace", WEAPON_MACE, 1, 45},
    {"maul", WEAPON_MAUL, 1, 45},
    {"morningstar", WEAPON_MORNINGSTAR, 1, 45},
    {"flail", WEAPON_FLAIL, 1, 45},
    {"nunchaku", WEAPON_NUNCHAKU, 1, 45},
    {"chain", WEAPON_CHAIN, 1, 45},
    {"bostick", WEAPON_BOSTICK, 1, 45},
    {"staff", WEAPON_STAFF, 1, 45},
    {"sai", WEAPON_SAI, 1, 45},
    {"sap", WEAPON_SAP, 1, 45},
    {"bolas", WEAPON_BOLAS, 1, 45},
    {"cestus", WEAPON_CESTUS, 1, 45},
    {"scourge", WEAPON_SCOURGE, 1, 45},
    {"whip", WEAPON_WHIP, 1, 45},
    {"boomerang", WEAPON_BOOMERANG, 1, 45},
    {"shuriken", WEAPON_SHURIKEN, 1, 45},
    {"throwing knife", WEAPON_THROWING_KNIFE, 1, 45},
    {"dart", WEAPON_DART, 1, 45},
    {"javelin", WEAPON_JAVELIN, 1, 45},
    {"harpoon", WEAPON_HARPOON, 1, 45},
    {"sling", WEAPON_SLING, 1, 45},
    {"staff sling", WEAPON_STAFF_SLING, 1, 45},
    {"short bow", WEAPON_SHORT_BOW, 1, 45},
    {"long bow", WEAPON_LONG_BOW, 1, 45},
    {"hand crossbow", WEAPON_HAND_CROSSBOW, 1, 45},
    {"light crossbow", WEAPON_LIGHT_CROSSBOW, 1, 45},
    {"heavy crossbow", WEAPON_HEAVY_CROSSBOW, 1, 45},
    {"blowgun", WEAPON_BLOWGUN, 1, 45},
    {"tetsubo", WEAPON_TETSUBO, 1, 45},
    {"generic", WEAPON_GENERIC, 1, 45},
    {"none", -1, -1, -1}
};
const int weaponskillCount = NELEMENTS(weaponskills) - 1;


const int       zoneloc[171] = { 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1636,
    883, 960, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    922, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 807, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 180, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,

    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792,
    1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792, 1792
};


/*
 * xcoord, y coord, Disembark room, embark room
 */
const struct map_coord map_coords[] = {
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {33, 46, 3005, 10000},
    {-1, -1, -1, -1}
};

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
