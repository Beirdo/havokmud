/*
 * DaleMUD,
 */

#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"

#define MANA_MU 1
#define MANA_CL 1

/*
 * 100 is the MAX_MANA for a character 
 */
#define USE_MANA(ch, sn) \
  ( spell_index[sn] == -1 ? 100 : \
    MAX((int)spell_info[spell_index[sn]].min_usesmana, \
        100 / MAX(2,(2+GET_LEVEL(ch, BestMagicClass(ch))-SPELL_LEVEL(ch,sn)))) )

/*
 * Global data 
 */

extern struct room_data *world;
extern struct char_data *character_list;
extern char    *spell_wear_off_msg[];
extern char    *spell_wear_off_soon_msg[];
extern char    *spell_wear_off_room_msg[];
extern char    *spell_wear_off_soon_room_msg[];
extern struct obj_data *object_list;
extern struct index_data *obj_index;
extern struct char_data *mem_list;
extern int      ArenaNoGroup,
                ArenaNoAssist,
                ArenaNoDispel,
                ArenaNoMagic,
                ArenaNoWSpells,
                ArenaNoSlay,
                ArenaNoFlee,
                ArenaNoHaste,
                ArenaNoPets,
                ArenaNoTravel,
                ArenaNoBash;
extern struct time_info_data time_info;
extern funcp    bweapons[];
extern struct chr_app_type chr_apply[];
extern struct int_app_type int_sf_modifier[];

/*
 * internal procedures 
 */
void            SpellWearOffSoon(int s, struct char_data *ch);
void            check_drowning(struct char_data *ch);
int             check_falling(struct char_data *ch);
void            check_decharm(struct char_data *ch);

/*
 * Extern procedures 
 */

char           *spells[] = {
    "armor",                    /* 1 */
    "teleport",
    "bless",
    "blindness",
    "burning hands",
    "call lightning",
    "charm person",
    "chill touch",
    "clone",
    "colour spray",
    "control weather",          /* 11 */
    "create food",
    "create water",
    "cure blind",
    "cure critic",
    "cure light",
    "curse",
    "detect evil",
    "detect invisibility",
    "detect magic",
    "detect poison",            /* 21 */
    "dispel evil",
    "earthquake",
    "enchant weapon",
    "energy drain",
    "fireball",
    "harm",
    "heal",
    "invisibility",
    "lightning bolt",
    "locate object",            /* 31 */
    "magic missile",
    "poison",
    "protection from evil",
    "remove curse",
    "sanctuary",
    "shocking grasp",
    "sleep",
    "strength",
    "summon",
    "ventriloquate",            /* 41 */
    "word of recall",
    "remove poison",
    "sense life",               /* 44 */

    /*
     * RESERVED SKILLS 
     */
    "sneak",                    /* 45 */
    "hide",
    "steal",
    "backstab",
    "pick",
    "kick",                     /* 50 */
    "bash",
    "rescue",
    /*
     * NON-CASTABLE SPELLS (Scrolls/potions/wands/staffs) 
     */

    "identify",                 /* 53 */
    "infravision",
    "cause light",
    "cause critical",
    "flamestrike",
    "dispel good",
    "weakness",
    "dispel magic",
    "knock",
    "know alignment",
    "animate dead",
    "paralyze",
    "remove paralysis",
    "fear",
    "acid blast",               /* 67 */
    "water breath",
    "fly",
    "cone of cold",             /* 70 */
    "meteor swarm",
    "ice storm",
    "shield",
    "monsum one",
    "monsum two",
    "monsum three",
    "monsum four",
    "monsum five",
    "monsum six",
    "monsum seven",             /* 80 */
    "fireshield",
    "charm monster",
    "cure serious",
    "cause serious",
    "refresh",
    "second wind",
    "turn",
    "succor",
    "create light",
    "continual light",          /* 90 */
    "calm",
    "stone skin",
    "conjure elemental",
    "true sight",
    "minor creation",
    "faerie fire",
    "faerie fog",
    "cacaodemon",
    "polymorph self",
    "mana",                     /* 100 */
    "astral walk",
    "resurrection",
    "heroes feast",             /* 103 */
    "group fly",
    "breath",
    "web",
    "minor track",
    "major track",
    "golem",
    "find familiar",            /* 110 */
    "changestaff",
    "holy word",
    "unholy word",
    "power word kill",
    "power word blind",
    "chain lightning",
    "scare",
    "aid",
    "command",
    "change form",              /* 120 */
    "feeblemind",
    "shillelagh",
    "goodberry",
    "flame blade",
    "animal growth",
    "insect growth",
    "creeping death",
    "commune",
    "animal summon one",
    "animal summon two",        /* 130 */
    "animal summon three",
    "fire servant",
    "earth servant",
    "water servant",
    "wind servant",
    "reincarnate",
    "charm vegetable",
    "vegetable growth",
    "tree",
    "animate rock",             /* 140 */
    "tree travel",
    "travelling",
    "animal friendship",
    "invis to animals",
    "slow poison",
    "entangle",
    "snare",
    "gust of wind",
    "barkskin",
    "sunray",                   /* 150 */
    "warp weapon",
    "heat stuff",
    "find traps",
    "firestorm",
    "haste",
    "slowness",
    "dust devil",
    "know monster",
    "transport via plant",
    "speak with plants",        /* 160 */
    "silence",
    "sending",
    "teleport without error",
    "portal",
    "dragon ride",
    "mount",
    "energy restore",
    "detect good",              /* 168 */
    "protection from good",
    "first aid",                /* 170 */
    "sign language",
    "riding",
    "switch opponents",
    "dodge",
    "remove trap",
    "retreat",
    "quivering palm",
    "safe fall",
    "feign death",
    "hunt",                     /* 180 */
    "find trap",
    "spring leap",
    "disarm",
    "read magic",
    "evaluate",
    "spy",
    "doorbash",
    "swim",
    "necromancy",
    "vegetable lore",           /* 190 */
    "demonology",
    "animal lore",
    "reptile lore",
    "people lore",
    "giant lore",
    "other lore",               /* 196 */
    "disguise",
    "climb",
    "disengage",
    "***",                      /* 200 */
    "flying with your wings",
    "your wings are tired",
    "Your wings are burned",
    "protection from good group",
    "giant growth",
    "***",
    "berserk",
    "tan",
    "avoid back attack",
    "find food",                /* 210 */
    "find water",               /* 211 */
    "pray",                     /* spell Prayer, 212 */
    "memorizing",
    "bellow",
    "darkness",
    "minor invulnerability",
    "major invulnerability",
    "protection from drain",
    "protection from breath",
    "anti magic shell",         /* anti magic shell */
    "doorway",
    "psi portal",
    "psi summon",
    "psi invisibility",
    "canibalize",
    "flame shroud",
    "aura sight",
    "great sight",
    "psionic blast",
    "hypnosis",                 /* 230 */
    "meditate",
    "scry",
    "adrenalize",
    "brew",
    "ration",
    "warcry",
    "blessing",
    "lay on hands",
    "heroic rescue",
    "dual wield",               /* 240 */
    "psi shield",
    "protection from evil group",
    "prismatic spray",
    "incendiary cloud",
    "disintegrate",
    "language common",
    "language elvish",
    "language halfling",
    "language dwarvish",
    "language orcish",          /* 250 */
    "language giantish",
    "language ogre",
    "language gnomish",
    "esp",
    "comprehend languages",
    "protection from fire",
    "protection from cold",
    "protection from energy",
    "protection from electricity",
    "enchant armor",            /* 260 */
    "messenger",
    "protection fire breath",
    "protection frost breath",
    "protection electric breath",
    "protection acid breath",
    "protection gas breath",
    "wizardeye",
    "mind burn",
    "clairvoyance",
    "psionic danger sense",     /* 270 */
    "psionic disintegrate",
    "telekinesis",
    "levitation",
    "cell adjustment",
    "chameleon",
    "psionic strength",
    "mind over body",
    "probability travel",
    "psionic teleport",
    "domination",               /* 280 */
    "mind wipe",
    "psychic crush",
    "tower of iron will",
    "mindblank",
    "psychic impersonation",
    "ultra blast",
    "intensify",
    "spot",
    "holy armor",               /* 289 */
    "holy strength",
    "enlightenment",
    "circle of protection",
    "wrath of god",
    "pacifism",
    "aura of power",            /* 295 */
    "legsweep",
    "charge",
    "standard",
    "berserked",
    "aggressive",
    "defensive",
    "evasive",
    "mend",                     /* 303 */
    "call steed",
    /*
     * necro spells 
     */
    "cold light",
    "disease",
    "invis to undead",
    "life tap",
    "suit of bone",
    "spectral shield",          /* 310 */
    "clinging darkness",
    "dominate undead",
    "unsummon",
    "siphon strength",
    "gather shadows",
    "mend bones",
    "trace corpse",
    "endure cold",
    "life draw",
    "numb the dead",            /* 320 */
    "binding",
    "decay",
    "shadow step",
    "cavorting bones",
    "mist of death",
    "nullify",
    "dark empathy",
    "eye of the dead",
    "soul steal",
    "life leech",               /* 330 */
    "dark pact",
    "darktravel",
    "vampiric embrace",
    "bind affinity",
    "scourge of the warlock",
    "finger of death",
    "flesh golem",
    "chillshield",              /* 338 */

    /*
     * psi stuff 
     */
    "wall of thought",
    "mind tap",

    "blade barrier",            /* 341 - blade barrier */
    "mana shield",
    "iron skins",
    "sense object",
    "kinolock",
    "flowerfist",
    "flurry of blows",
    "scribe",                   /* 348 */
    "group heal",

    "plant gate",               /* 350 */
    "\n"
};

const byte      saving_throws[MAX_CLASS][5][ABS_MAX_LVL] = {
    {
     /*
      * mage 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * cleric 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37, 36,
      35, 35, 34, 34, 33, 33, 32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 27,
      26, 25, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 36, 35,
      34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 26,
      25, 24, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * warrior 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 28,
      28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21,
      21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33,
      33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25,
      24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * thief 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23,
      22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33,
      32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 23,
      22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {47, 46, 46, 45, 45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38,
      37, 37, 36, 35, 35, 34, 34, 33, 33, 32, 31, 31, 30, 30, 29, 28, 28,
      27, 27, 26, 26, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * druid 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 42, 41, 41, 40, 39, 39, 38, 38, 37, 37, 36,
      35, 35, 34, 34, 33, 33, 32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 27,
      26, 25, 25, 24, 24, 23, 23, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 36, 35,
      34, 34, 33, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 26,
      25, 24, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * monk 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 28,
      28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21,
      21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33,
      33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25,
      24, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * barbarian 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 28, 28, 27,
      27, 26, 26, 25, 25, 24, 24, 23, 23, 23, 22, 22, 21, 21, 20, 20, 19,
      19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31,
      30, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 23, 22, 22,
      21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22, 21, 21,
      20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 36, 36, 35, 35, 34, 33, 33, 32, 32,
      31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 22, 22,
      21, 21, 20, 20, 19, 18, 18, 17, 17, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * sorcerer 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * paladin 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {30, 30, 29, 29, 28, 28, 28, 27, 27, 27, 26, 26, 25, 25, 25, 24, 24,
      24, 23, 23, 22, 22, 22, 21, 21, 21, 20, 20, 19, 19, 19, 18, 18, 17,
      17, 17, 16, 16, 16, 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28,
      27, 27, 26, 26, 25, 25, 24, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28, 27, 27, 26, 26,
      26, 25, 25, 24, 24, 23, 23, 22, 22, 22, 21, 21, 20, 20, 19, 19, 18,
      18, 18, 17, 17, 16, 16, 15, 15, 15, 14, 14, 13, 13, 12, 12, 11, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29,
      28, 28, 27, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 21,
      21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * ranger 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29, 29, 29, 28, 28,
      27, 27, 26, 26, 25, 25, 24, 24, 24, 23, 23, 22, 22, 21, 21, 20, 20,
      19, 19, 18, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13, 12, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31,
      31, 30, 30, 29, 29, 28, 28, 27, 27, 26, 26, 25, 25, 24, 24, 23, 23,
      22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 30, 30, 29,
      29, 28, 28, 27, 27, 26, 26, 25, 25, 25, 24, 24, 23, 23, 22, 22, 21,
      21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14, 13, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 36, 36, 35, 35, 34, 33, 33, 32, 32,
      31, 31, 30, 29, 29, 28, 28, 27, 27, 26, 25, 25, 24, 24, 23, 22, 22,
      21, 21, 20, 20, 19, 18, 18, 17, 17, 16, 15, 15, 14, 14, 13, 13, 12,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {41, 40, 40, 39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 34, 33, 33, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 24, 23,
      23, 22, 22, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * psi 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {33, 33, 32, 32, 32, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28, 28, 27,
      27, 27, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22, 22, 21, 21,
      21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17, 16, 16, 16, 15, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 33, 33, 32, 32, 31, 31, 31, 30, 30,
      29, 29, 28, 28, 27, 27, 26, 26, 26, 25, 25, 24, 24, 23, 23, 22, 22,
      21, 21, 20, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {27, 27, 26, 26, 26, 26, 25, 25, 25, 25, 24, 24, 24, 24, 23, 23, 23,
      23, 22, 22, 22, 22, 21, 21, 21, 21, 20, 20, 20, 19, 19, 19, 19, 18,
      18, 18, 18, 17, 17, 17, 17, 16, 16, 16, 16, 15, 15, 15, 15, 14, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {39, 39, 38, 38, 37, 37, 36, 36, 35, 35, 35, 34, 34, 33, 33, 32, 32,
      32, 31, 31, 30, 30, 29, 29, 28, 28, 28, 27, 27, 26, 26, 25, 25, 24,
      24, 24, 23, 23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {37, 37, 36, 36, 35, 35, 34, 34, 34, 33, 33, 32, 32, 32, 31, 31, 30,
      30, 29, 29, 29, 28, 28, 27, 27, 27, 26, 26, 25, 25, 24, 24, 24, 23,
      23, 22, 22, 21, 21, 21, 20, 20, 19, 19, 19, 18, 18, 17, 17, 16, 16,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     },
    {
     /*
      * necromancer 
      */
     /*
      * 1 2 3 4 5 6 7 8 9 10 15 20 25 30 35 40 45 50 
      */
     {45, 44, 44, 43, 42, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35,
      34, 33, 33, 32, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24,
      23, 23, 22, 21, 21, 20, 19, 19, 18, 17, 17, 16, 16, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {42, 41, 41, 40, 40, 39, 39, 38, 37, 37, 36, 36, 35, 34, 34, 33, 33,
      32, 32, 31, 30, 30, 29, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23,
      22, 22, 21, 21, 20, 19, 19, 18, 18, 17, 16, 16, 15, 15, 14, 14, 13,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {43, 42, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35, 35, 34, 34,
      33, 33, 32, 31, 31, 30, 30, 29, 29, 28, 27, 27, 26, 26, 25, 24, 24,
      23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15, 15, 14,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {45, 44, 44, 43, 43, 42, 41, 41, 40, 40, 39, 38, 38, 37, 37, 36, 35,
      35, 34, 34, 33, 32, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25,
      25, 24, 23, 23, 22, 22, 21, 20, 20, 19, 19, 18, 17, 17, 16, 16, 15,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6},
     {44, 43, 43, 42, 41, 41, 40, 39, 39, 38, 37, 37, 36, 35, 35, 34, 33,
      33, 32, 31, 31, 30, 29, 29, 28, 28, 27, 26, 26, 25, 24, 24, 23, 22,
      22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 15, 14, 14, 13, 12, 12, 11,
      6, 6, 6, 6, 6, 6, 6, 6, 6, 6}
     }
};


struct spell_info_type spell_info[] = {
    { 1, cast_armor, 5, 12, POSITION_STANDING,
      5, 2, IMMORTAL, 5, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 2, cast_teleport, 33, 12, POSITION_STANDING,
      17, IMMORTAL, IMMORTAL, 17, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 60, 1 },
    { 3, cast_bless, 5, 12, POSITION_STANDING, 
      IMMORTAL, 1, IMMORTAL, 17, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 4, cast_blindness, 15, 24, POSITION_FIGHTING, 
      12, 14, IMMORTAL, 12, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 60, 0 },
    { 5, cast_burning_hands, 15, 24, POSITION_FIGHTING, 
      6, IMMORTAL, IMMORTAL, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 5, 0 },
    { 6, cast_call_lightning, 20, 36, POSITION_FIGHTING,
      IMMORTAL, 45, 18, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 7, cast_charm_person, 10, 12, POSITION_STANDING, 
      4, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_FIGHT_VICT | TAR_VIOLENT, 0, 0 },
    { 8, cast_chill_touch, 15, 12, POSITION_FIGHTING, 
      4, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 10, cast_colour_spray, 20, 24, POSITION_FIGHTING, 
      14, IMMORTAL, IMMORTAL, 14, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 40, 0 },
    { 11, cast_control_weather, 25, 36, POSITION_STANDING, 
      IMMORTAL, 26, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 12, cast_create_food, 5, 12, POSITION_STANDING,
      IMMORTAL, 5, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 13, cast_create_water, 5, 12, POSITION_STANDING,
      IMMORTAL, 2, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_INV | TAR_OBJ_EQUIP, 0, 1 },
    { 14, cast_cure_blind, 5, 12, POSITION_STANDING, 
      IMMORTAL, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 15, cast_cure_critic, 11, 24, POSITION_FIGHTING, 
      IMMORTAL, 10, 13, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 30, 1 },
    { 16, cast_cure_light, 5, 12, POSITION_FIGHTING, 
      IMMORTAL, 1, 2, IMMORTAL, 9, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 10, 1 },
    { 17, cast_curse, 20, 24, POSITION_STANDING, 
      13, 12, IMMORTAL, 13, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | 
      TAR_FIGHT_VICT | TAR_VIOLENT, 0, 1 },
    { 18, cast_detect_evil, 5, 12, POSITION_STANDING, 
      IMMORTAL, 1, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 19, cast_detect_invisibility, 5, 12, POSITION_STANDING, 
      2, 5, 7, 2, IMMORTAL, IMMORTAL, IMMORTAL, 8,
      TAR_CHAR_ROOM, 0, 1 },
    { 20, cast_detect_magic, 5, 12, POSITION_STANDING, 
      1, 3, 5, 1, IMMORTAL, IMMORTAL, IMMORTAL, 3,
      TAR_CHAR_ROOM, 0, 1 },
    { 21, cast_detect_poison, 5, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP, 0, 1 },
    { 22, cast_dispel_evil, 15, 24, POSITION_FIGHTING, 
      IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 20, 0 },
    { 23, cast_earthquake, 15, 24, POSITION_FIGHTING, 
      IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 30, 0 },
    { 24, cast_enchant_weapon, 100, 48, POSITION_STANDING, 
      14, IMMORTAL, IMMORTAL, 14, IMMORTAL, IMMORTAL, IMMORTAL, 21,
      TAR_OBJ_INV | TAR_OBJ_EQUIP, 0, 0 },
    { 25, cast_energy_drain, 35, 36, POSITION_FIGHTING, 
      22, IMMORTAL, IMMORTAL, 22, IMMORTAL, IMMORTAL, IMMORTAL, 15,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 60, 1 },
    { 26, cast_fireball, 40, 36, POSITION_FIGHTING, 
      25, IMMORTAL, IMMORTAL, 25, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 35, 0 },
    { 27, cast_harm, 35, 36, POSITION_FIGHTING, 
      IMMORTAL, 25, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 110, 0 },
    { 28, cast_heal, 50, 12, POSITION_FIGHTING, 
      IMMORTAL, 25, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 90, 1 },
    { 29, cast_invisibility, 5, 12, POSITION_STANDING, 
      4, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP, 0, 1 },
    { 30, cast_lightning_bolt, 15, 24, POSITION_FIGHTING, 
      10, IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 25, 0 },
    { 31, cast_locate_object, 20, 12, POSITION_STANDING, 
      IMMORTAL, 11, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_WORLD, 0, 0 },
    { 32, cast_magic_missile, 15, 12, POSITION_FIGHTING, 
      1, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 33, cast_poison, 10, 24, POSITION_FIGHTING, 
      IMMORTAL, 13, 8, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 8,
      TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP |
      TAR_FIGHT_VICT | TAR_VIOLENT, 60, 0 },
    { 34, cast_protection_from_evil, 5, 12, POSITION_STANDING,
      IMMORTAL, 7, IMMORTAL, IMMORTAL, 20, 25, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 35, cast_remove_curse, 5, 12, POSITION_STANDING, 
      IMMORTAL, 8, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM, 0, 1 },
    { 36, cast_sanctuary, 50, 36, POSITION_STANDING, 
      IMMORTAL, 26, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 37, cast_shocking_grasp, 15, 12, POSITION_FIGHTING, 
      2, IMMORTAL, IMMORTAL, 2, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 38, cast_sleep, 15, 24, POSITION_FIGHTING, 
      3, IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 20, 0 },
    { 39, cast_strength, 10, 12, POSITION_STANDING, 
      6, IMMORTAL, IMMORTAL, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 40, cast_summon, 20, 36, POSITION_STANDING, 
      27, 19, IMMORTAL, 27, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_WORLD | TAR_VIOLENT, 0, 0 },
    { 41, cast_ventriloquate, 5, 12, POSITION_STANDING, 
      1, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO, 0, 0 },
    { 42, cast_word_of_recall, 5, 12, POSITION_STANDING, 
      IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 1 },
    { 43, cast_remove_poison, 5, 12, POSITION_STANDING, 
      IMMORTAL, 17, 8, IMMORTAL, 40, 20, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM, 0, 1 },
    { 44, cast_sense_life, 5, 12, POSITION_STANDING, 
      IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    /* sneak */
    { 45,  NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 4, NEVER_USE, 
      NEVER_USE, 
      TAR_IGNORE, 0, 0 },
    /* hide */
    { 46, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 2, NEVER_USE, 
      NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 47, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 48, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 49, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 50, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    /* bash */
    { 51, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 1, 1, NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 52, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 3, NEVER_USE, 
      NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 53, cast_identify, 15, 24, POSITION_STANDING, 
      10, 16, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_OBJ_INV, 0, 1 },
    { 54, cast_infravision, 7, 12, POSITION_STANDING, 
      8, IMMORTAL, 5, 8, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 55, cast_cause_light, 8, 12, POSITION_FIGHTING, 
      IMMORTAL, 1, 2, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 10, 0 },
    { 56, cast_cause_critic, 11, 24, POSITION_FIGHTING,
      IMMORTAL, 10, 13, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 57, cast_flamestrike, 15, 36, POSITION_FIGHTING, 
      IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 58, cast_dispel_good, 15, 36, POSITION_FIGHTING, 
      IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 20, 0 },
    { 59, cast_weakness, 10, 12, POSITION_FIGHTING, 
      6, IMMORTAL, IMMORTAL, 6, IMMORTAL, IMMORTAL, IMMORTAL, 9,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 60, cast_dispel_magic, 15, 12, POSITION_FIGHTING, 
      9, 10, 9, 9, IMMORTAL, IMMORTAL, IMMORTAL, 14, 
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_ROOM | TAR_OBJ_INV | 
      TAR_VIOLENT, 50, 1 },
    { 61, cast_knock, 10, 12, POSITION_STANDING, 
      3, IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 62, cast_know_alignment, 10, 12, POSITION_STANDING,
      7, 4, 2, 7, 5, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0, 1 },
    { 63, cast_animate_dead, 5, 12, POSITION_STANDING, 
      16, 8, IMMORTAL, 16, IMMORTAL, IMMORTAL, IMMORTAL, 5,
      TAR_OBJ_ROOM, 0, 0 },
    { 64, cast_paralyze, 15, 36, POSITION_FIGHTING, 
      20, IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 90, 0 },
    { 65, cast_remove_paralysis, 10, 12, POSITION_FIGHTING, 
      IMMORTAL, 6, 9, IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 30, 1 },
    { 66, cast_fear, 15, 12, POSITION_FIGHTING, 
      8, IMMORTAL, IMMORTAL, 8, IMMORTAL, IMMORTAL, IMMORTAL, 4,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 67, cast_acid_blast, 15, 24, POSITION_FIGHTING, 
      6, IMMORTAL, IMMORTAL, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 20, 0 },
    { 68, cast_water_breath, 15, 12, POSITION_STANDING, 
      9, IMMORTAL, 6, 9, IMMORTAL, 17, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 69, cast_flying, 15, 12, POSITION_STANDING, 
      11, 22, 14, 11, IMMORTAL, IMMORTAL, IMMORTAL, 20,
      TAR_CHAR_ROOM, 0, 1 },
    { 70, cast_cone_of_cold, 15, 24, POSITION_FIGHTING, 
      17, IMMORTAL, IMMORTAL, 17, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 40, 0 },
    { 71, cast_meteor_swarm, 35, 24, POSITION_FIGHTING, 
      36, IMMORTAL, IMMORTAL, 36, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 80, 0 },
    { 72, cast_ice_storm, 15, 12, POSITION_FIGHTING, 
      11, IMMORTAL, IMMORTAL, 11, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 30, 0 },
    { 73, cast_shield, 1, 24, POSITION_FIGHTING, 
      1, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 15, 1 },
    { 74, cast_mon_sum1, 10, 24, POSITION_STANDING, 
      4, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 75, cast_mon_sum2, 12, 24, POSITION_STANDING, 
      7, IMMORTAL, IMMORTAL, 7, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 76, cast_mon_sum3, 15, 24, POSITION_STANDING, 
      9, IMMORTAL, IMMORTAL, 9, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 77, cast_mon_sum4, 17, 24, POSITION_STANDING, 
      12, IMMORTAL, IMMORTAL, 12, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 78, cast_mon_sum5, 20, 24, POSITION_STANDING, 
      15, IMMORTAL, IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 79, cast_mon_sum6, 22, 24, POSITION_STANDING, 
      18, IMMORTAL, IMMORTAL, 18, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 80, cast_mon_sum7, 25, 24, POSITION_STANDING, 
      22, IMMORTAL, IMMORTAL, 22, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 81, cast_fireshield, 40, 24, POSITION_STANDING, 
      40, IMMORTAL, 48, 40, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0 },
    { 82, cast_charm_monster, 5, 12, POSITION_STANDING, 
      10, IMMORTAL, 12, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 0, 0 },
    { 83, cast_cure_serious, 9, 12, POSITION_FIGHTING, 
      IMMORTAL, 7, 8, 45, 45, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 20, 1 },
    { 84, cast_cause_serious, 9, 12, POSITION_FIGHTING, 
      IMMORTAL, 7, 8, 45, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 20, 0 },
    { 85, cast_refresh, 5, 12, POSITION_STANDING, 
      6, 3, 4, 6, IMMORTAL, IMMORTAL, IMMORTAL, 5,
      TAR_CHAR_ROOM, 0, 1 },
    { 86, cast_second_wind, 5, 12, POSITION_FIGHTING, 
      20, 9, 14, 20, 35, 30, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 25, 1 },
    { 87, cast_turn, 5, 12, POSITION_STANDING, 
      IMMORTAL, 1, 12, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 0 },
    { 88, cast_succor, 15, 24, POSITION_STANDING, 
      24, 23, IMMORTAL, 24, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 89, cast_light, 5, 12, POSITION_STANDING, 
      1, 2, 3, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 90, cast_cont_light, 10, 24, POSITION_STANDING, 
      10, 26, 16, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 91, cast_calm, 15, 24, POSITION_STANDING, 
      4, 2, IMMORTAL, 4, 18, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 0 },
    { 92, cast_stone_skin, 40, 24, POSITION_STANDING, 
      26, IMMORTAL, IMMORTAL, 26, IMMORTAL, IMMORTAL, IMMORTAL, 32,
      TAR_SELF_ONLY, 0, 0 },
    { 93, cast_conjure_elemental, 30, 24, POSITION_STANDING,
      16, 13, 11, 16, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 94, cast_true_seeing, 20, 24, POSITION_STANDING, 
      IMMORTAL, 19, 24, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 95, cast_minor_creation, 30, 24, POSITION_STANDING, 
      8, IMMORTAL, IMMORTAL, 8, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 96, cast_faerie_fire, 10, 12, POSITION_STANDING, 
      5, 4, 1, 5, IMMORTAL, 7, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_VIOLENT, 0, 0 },
    { 97, cast_faerie_fog, 20, 24, POSITION_STANDING,
      16, 11, 10, 16, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 98, cast_cacaodemon, 50, 24, POSITION_STANDING, 
      30, 29, IMMORTAL, 30, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 99, cast_poly_self, 10, 12, POSITION_STANDING, 
      9, IMMORTAL, IMMORTAL, 9, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 100, cast_mana, 200, 12, POSITION_FIGHTING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE,
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 101, cast_astral_walk, 30, 12, POSITION_STANDING, 
      IMMORTAL, 30, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_WORLD, 0, 1 },
    { 102, cast_resurrection, 33, 36, POSITION_STANDING, 
      IMMORTAL, 36, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_ROOM, 0, 0 },
    { 103, cast_heroes_feast, 40, 12, POSITION_STANDING, 
      IMMORTAL, 24, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 104, cast_fly_group, 30, 12, POSITION_STANDING, 
      24, IMMORTAL, 22, 24, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 105, cast_dragon_breath, 200, 127, POSITION_FIGHTING, 
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 
      NEVER_USE, NEVER_USE,
      TAR_IGNORE | TAR_VIOLENT, 0, 0 },
    { 106, cast_web, 3, 12, POSITION_FIGHTING, 
      11, IMMORTAL, IMMORTAL, 11, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 40, 0 },
    { 107, cast_minor_track, 10, 12, POSITION_STANDING, 
      12, IMMORTAL, 7, 12, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 108, cast_major_track, 20, 12, POSITION_STANDING, 
      20, IMMORTAL, 17, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 109, cast_golem, 30, 24, POSITION_STANDING, 
      IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 110, cast_familiar, 30, 24, POSITION_STANDING, 
      2, IMMORTAL, IMMORTAL, 2, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 111, cast_changestaff, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 30, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 112, cast_holyword, 30, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 
      IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 80, 0 },
    { 113, cast_unholyword, 30, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 80, 0 },
    { 114, cast_pword_kill, 10, 24, POSITION_FIGHTING, 
      23, IMMORTAL, IMMORTAL, 23, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 40, 0 },
    { 115, cast_pword_blind, 10, 24, POSITION_FIGHTING, 
      16, IMMORTAL, IMMORTAL, 16, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 20, 0 },
    { 116, cast_chain_lightn, 40, 24, POSITION_FIGHTING, 
      25, IMMORTAL, 25, 25, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 70, 0 },
    { 117, cast_scare, 3, 24, POSITION_FIGHTING, 
      4, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 20, 0 },
    { 118, cast_aid, 5, 24, POSITION_FIGHTING, 
      IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 10, 1 },
    { 119, cast_command, 3, 24, POSITION_FIGHTING, 
      IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 120, cast_change_form, 20, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 12, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 121, cast_feeblemind, 30, 24, POSITION_FIGHTING, 
      34, IMMORTAL, IMMORTAL, 34, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 40, 0 },
    { 122, cast_shillelagh, 10, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_INV, 0, 0 },
    { 123, cast_goodberry, 10, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 124, cast_flame_blade, 10, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 7, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 125, cast_animal_growth, 20, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, 35, IMMORTAL, IMMORTAL, 35, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO, 30, 0 },
    { 126, cast_insect_growth, 20, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, 33, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO, 30, 0 },
    { 127, cast_creeping_death, 50, 36, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 45, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 128, cast_commune, 10, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 129, cast_animal_summon_1, 15, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 15, IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 130, cast_animal_summon_2, 20, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, 25, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 131, cast_animal_summon_3, 25, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 25, IMMORTAL, IMMORTAL, 30, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 132, cast_fire_servant, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 35, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 133, cast_earth_servant, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 36, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 134, cast_water_servant, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 37, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 135, cast_wind_servant, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 38, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 136, cast_reincarnate, 50, 36, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 39, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_ROOM, 0, 0 },
    { 137, cast_charm_veggie, 5, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 17, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 0, 0 },
    { 138, cast_veggie_growth, 20, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO, 30, 0 },
    { 139, cast_tree, 30, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 140, cast_animate_rock, 15, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 31, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_INV | TAR_OBJ_ROOM, 0, 0 },
    { 141, cast_tree_travel, 2, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 8, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_SELF_ONLY, 0, 0 },
    { 142, cast_travelling, 2, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL, 15, IMMORTAL, IMMORTAL,
      TAR_SELF_ONLY, 0, 1 },
    { 143, cast_animal_friendship, 2, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 5, IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_NONO, 0, 0 },
    { 144, cast_invis_to_animals, 10, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 11, IMMORTAL, IMMORTAL, 8, IMMORTAL, IMMORTAL,
      TAR_SELF_ONLY, 0, 1 },
    { 145, cast_slow_poison, 20, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 6, IMMORTAL, 10, 10, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 146, cast_entangle, 20, 12, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, 16, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 0, 0 },
    { 147,  cast_snare, 10, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, 8, IMMORTAL, IMMORTAL, 5, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 0, 0 },
    { 148, cast_gust_of_wind, 2, 12, POSITION_FIGHTING,
      10, IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 30, 0 },
    { 149, cast_barkskin, 5, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 150, cast_sunray, 15, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, 27, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 151, cast_warp_weapon, 20, 12, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, 19, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
       TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT | TAR_OBJ_ROOM |
       TAR_OBJ_INV, 60, 0 },
    { 152, cast_heat_stuff, 30, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, 23, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT,  30, 0 },
    { 153, cast_find_traps, 10, 12, POSITION_STANDING,
      IMMORTAL, 16, 15, IMMORTAL, IMMORTAL, 13, IMMORTAL, IMMORTAL,
      TAR_SELF_ONLY, 0, 1 },
    { 154, cast_firestorm, 15, 12, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, 22, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 30, 0 },
    { 155, cast_haste, 20, 12, POSITION_STANDING, 
      23, IMMORTAL, IMMORTAL, 23, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 0, 1 },
    { 156, cast_slow, 20, 12, POSITION_STANDING,
      19, IMMORTAL, IMMORTAL, 19, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 0, 0 },
    { 157, cast_dust_devil, 10, 24, POSITION_STANDING,
      IMMORTAL, 3, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 158, cast_know_monster, 20, 12, POSITION_FIGHTING,
      9, IMMORTAL, IMMORTAL, 9, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_SELF_NONO | TAR_CHAR_ROOM | TAR_FIGHT_VICT, 30, 0 },
    { 159, cast_transport_via_plant, 20, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_WORLD, 0, 0 },
    { 160, cast_speak_with_plants, 5, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 7, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_ROOM, 0, 0 },
    { 161, cast_silence, 30, 12, POSITION_FIGHTING,
      21, 23, 25, 21, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 80, 0 },
    { 162, NULL, 5, 12, POSITION_STANDING,
      1, IMMORTAL, 1, 1, IMMORTAL, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    { 163, cast_teleport_wo_error, 20, 12, POSITION_STANDING, 
      30, BIG_GUY, BIG_GUY, 30, BIG_GUY, BIG_GUY, BIG_GUY, 35,
      TAR_CHAR_WORLD, 0, 1 },
    { 164, cast_portal, 50, 12, POSITION_STANDING, 
      43, BIG_GUY, BIG_GUY, 43, BIG_GUY, BIG_GUY, BIG_GUY, IMMORTAL,
      TAR_CHAR_WORLD, 0, 0 },
    { 165, cast_dragon_ride, 20, 12, POSITION_STANDING, 
      BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_IGNORE, 0, 0 },
    { 166, cast_mount, 20, 12, POSITION_STANDING, 
      47, BIG_GUY, 15, 47, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_IGNORE, 0, 0 },
    { 167, cast_energy_restore, 50, 1, POSITION_STANDING,
      IMMORTAL, 48, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 20, 1 },
    { 168, cast_detect_good, 5, 12, POSITION_STANDING,
      IMMORTAL, 1, 6, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 169, cast_protection_from_good, 5, 12, POSITION_STANDING, 
      IMMORTAL, 7, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1,
      TAR_CHAR_ROOM, 0, 1 },
    /* first aid */
    { 170, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, 1, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* switch opp */
    { 173, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, 1, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* dodge */
    { 174, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, 1, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* hunt */
    { 180, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      IMMORTAL,
      0, 0, 0 },
    /* disarm */
    { 183, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, 1, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* spy */
    { 186, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      IMMORTAL, 
      0, 0, 0 },
    { 187, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, 1, NEVER_USE, 
      NEVER_USE,
      TAR_IGNORE, 0, 0 },
    /* climb */
    { 198, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      IMMORTAL,
      0, 0, 0 },
    { 203, NULL, 200, 12, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE,
      NEVER_USE, NEVER_USE,
      TAR_IGNORE, 0, 0 },
    { 204, cast_protection_from_good_group, 45, 12, POSITION_STANDING,
      IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 205, cast_giant_growth, 50, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 0 },
    /* berserk */
    { 207, NULL, 200, 0, POSITION_STANDING, 
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      IMMORTAL,
      0, 0, 0 },
    /* tan */
    { 208, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      IMMORTAL,
      0, 0, 0 },
    /* avoid ba */
    { 209, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      IMMORTAL,
      0, 0, 0 },
    /* find food */
    { 210, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      IMMORTAL, 
      0, 0, 0 },
    /* find water */
    { 211, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, IMMORTAL, IMMORTAL, 1, IMMORTAL,
      IMMORTAL,
      0, 0, 0 },
    /* bellow */
    { 214, NULL, 200, 0, POSITION_STANDING,
      NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE, NEVER_USE,
      NEVER_USE, NEVER_USE,
      0, 0, 0 },
    { 215, cast_globe_darkness, 15, 12, POSITION_STANDING,
      5, 3, 3, 5, IMMORTAL, IMMORTAL, IMMORTAL, 5,
      TAR_CHAR_ROOM, 0, 1 },
    { 216, cast_globe_minor_inv, 25, 12, POSITION_STANDING,
      20, IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, 26,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 1 },
    { 217, cast_globe_major_inv, 50, 24, POSITION_STANDING,
      27, IMMORTAL, IMMORTAL, 27, IMMORTAL, IMMORTAL, IMMORTAL, 30,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 218, cast_prot_energy_drain, 40, 12, POSITION_STANDING,
      IMMORTAL, 48, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 1 },
    { 219, cast_prot_dragon_breath, 200, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      IMMORTAL,
      TAR_CHAR_ROOM, 0, 1 },
    { 220, cast_anti_magic_shell, 100, 24, POSITION_STANDING, 
      48, BIG_GUY, BIG_GUY, 48, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    /* doorway */
    { 221, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 3, IMMORTAL,
      0, 0, 0 },
    /* portal */
    { 222, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 32, IMMORTAL,
      0, 0, 0 },
    /* summon */
    { 223, NULL, 20, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 10, IMMORTAL,
      0, 0, 0 },
    /* invis */
    { 224, NULL, 20, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL,
      0, 0, 1},
    /* canibalize */
    { 225, NULL, 20, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 2, IMMORTAL,
      0, 0, 0 },
    /* flame shroud */
    { 226, NULL, 20, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 15, IMMORTAL,
      0, 0, 0 },
    /* aura sight */
    { 227, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 5, IMMORTAL,
      0, 0, 0 }, 
    /* great site */
    { 228, NULL, 20, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 11, IMMORTAL,
      0, 0, 0 },
    /* blast */
    { 229, NULL, 20,0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL,
      0, 0, 0 },
    /* hypnosis */
    { 230, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 2, IMMORTAL,
      0, 0, 0},
    /* meditate */
    { 231, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL,
      0, 0, 0 },
    /* scry */
    { 232, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 7, IMMORTAL,
      0, 0, 0 },
    /* adrenalize */
    { 233, NULL, 20, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 20, IMMORTAL,
      0, 0, 1 },
    /* brew potion */
    { 234, NULL, 200, POSITION_STANDING,
      IMMORTAL, IMMORTAL, 10, IMMORTAL, IMMORTAL, IMMORTAL, 10, IMMORTAL,
      0, 0, 0 },
    /* ration */
    { 235, NULL, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* paladin warcry */
    { 236, NULL, 200, 0, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 5, IMMORTAL, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* paladin blessing */
    { 237, NULL, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* paladin lay on hands */
    { 238, NULL, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* paladin heroic rescue */
    { 239, NULL, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    /* dual wield */
    { 240, NULL, 200, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, IMMORTAL,
      0, 0, 0},
    /* psi shield */
    { 241, NULL, 20, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL,
           TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 1 },
    { 242, cast_protection_from_evil_group, 45, 12, POSITION_STANDING, 
      IMMORTAL, 20, IMMORTAL, IMMORTAL, 35, 45, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 1 },
    { 243, cast_prismatic_spray, 40, 36, POSITION_FIGHTING, 
      17, IMMORTAL, IMMORTAL, 17, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 50, 0 },
    { 244, cast_incendiary_cloud, 60, 36, POSITION_FIGHTING,
      45, IMMORTAL, IMMORTAL, 45, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 80, 0 },
    { 245, cast_disintegrate, 45, 12, POSITION_FIGHTING,
      48, IMMORTAL, IMMORTAL, 48, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT, 60, 0 },
    /* psi esp */
    { 254, NULL, 10, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, IMMORTAL, 
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0 },
    { 255, cast_comp_languages, 10, 12, POSITION_STANDING,
      1, 3, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 0, 0 },
    { 256, cast_prot_fire, 40, 12, POSITION_STANDING,
      BIG_GUY, 21, 22, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 257, cast_prot_cold, 40, 12, POSITION_STANDING,
      BIG_GUY, 20, 21, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, 
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 258, cast_prot_energy, 40, 12, POSITION_STANDING,
      BIG_GUY, 19, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 259, cast_prot_elec, 40, 12, POSITION_STANDING,
      BIG_GUY, 18, 19, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY, BIG_GUY,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 260, cast_enchant_armor, 100, 48, POSITION_STANDING,
      16, IMMORTAL, IMMORTAL, 16, IMMORTAL, IMMORTAL, IMMORTAL, 17,
      TAR_OBJ_INV | TAR_OBJ_EQUIP, 0, 1 },
    { 261, NULL, 5, 12, POSITION_STANDING, 
      1, 1, IMMORTAL, 1, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    { 262, cast_prot_dragon_breath_fire, 50, 24, POSITION_STANDING,
      IMMORTAL, 40, 41, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 1 },
    { 263, cast_prot_dragon_breath_frost, 50, 24, POSITION_STANDING,
      IMMORTAL, 38, 39, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 264, cast_prot_dragon_breath_elec, 50, 24, POSITION_STANDING,
      IMMORTAL, 39, 40, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 265, cast_prot_dragon_breath_acid, 50, 24, POSITION_STANDING,
      IMMORTAL, 40, 41, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 266, cast_prot_dragon_breath_gas, 50, 24, POSITION_STANDING,
      IMMORTAL, 37, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 267, cast_wizard_eye, 40, 24, POSITION_STANDING, 
      35, IMMORTAL, IMMORTAL, 35, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 20, 1 },
    { 268, mind_use_burn, 15, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 5, IMMORTAL,
      TAR_IGNORE | TAR_VIOLENT, 5, 0 },
    { 269, mind_use_clairvoyance, 45, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 8, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 40, 0 },
    { 270, mind_use_danger_sense, 50, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 23, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 30, 0 },
    { 271, mind_use_disintegrate, 50, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 38, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 10, 0 },
    { 272, mind_use_telekinesis, 15, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 35, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 50, 0 },
    { 273, mind_use_levitation, 20, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 10, IMMORTAL,
      TAR_CHAR_ROOM, 40, 0 },
    { 274, mind_use_cell_adjustment, 100, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 14, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 80, 0 },
    { 275, mind_use_chameleon, 40, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 8, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 30, 0 },
    { 276, mind_use_psi_strength, 15, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 6, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 20, 1 },
    { 277, mind_use_mind_over_body, 60, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 14, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 20, 0 },
    { 278, mind_use_probability_travel, 40, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 21, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0 },
    { 279, mind_use_teleport, 15, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 12, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 30, 0 },
    { 281, mind_use_mind_wipe, 20, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 21, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 70, 0 }, 
    { 282, mind_use_psychic_crush, 45, 34, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 37, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT, 40, 0 },
    { 283, mind_use_tower_iron_will, 50, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 34, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 30, 0 },
    { 284, mind_use_mindblank, 50, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 17, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 20, 0 },
    { 285, mind_use_psychic_impersonation, 50, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 3, IMMORTAL,
      TAR_SELF_ONLY | TAR_CHAR_ROOM, 0, 0 },
    { 286, mind_use_ultra_blast, 30, 35, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 20, IMMORTAL,
      TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_IGNORE, 50, 0 },
    /* spot */
    { 288, NULL, 100, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1, 1, IMMORTAL, IMMORTAL,
      0, 0, 0 },
    { 289, cast_holy_armor, 25, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 5, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 290, cast_holy_strength, 25, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 3, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 291, cast_enlightenment, 10, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 25, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    { 293, cast_wrath_god, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 51, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 0, 0 },
    { 294, cast_pacifism, 15, 0, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 7, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM, 30, 0 },
    { 295, cast_aura_power, 50, 0, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 15, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_CHAR_ROOM | TAR_SELF_ONLY, 0, 0 },
    /* paladin charge */
    { 297, NULL, 200, 0, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 4, IMMORTAL, IMMORTAL, IMMORTAL,
     0, 0, 0 },
    { 305, cast_wall_of_thought, 20, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 35, IMMORTAL,
      TAR_SELF_ONLY, 0, 1 },
    { 306, mind_use_mind_tap, 35, 36, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 20, IMMORTAL,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 110, 0 },
    { 305, cast_cold_light, 5, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1,
      TAR_IGNORE, 0, 0 },
    { 306, cast_disease, 10, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 1,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 307, cast_invis_to_undead, 5, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 2, 
      TAR_IGNORE, 0, 1 },
    { 308, cast_life_tap, 5, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 2,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 309, cast_suit_of_bone, 5, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 3,
      TAR_IGNORE, 0, 1 },
    { 310, cast_spectral_shield, 10, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 4,
      TAR_IGNORE, 20, 1 },
    { 311, cast_clinging_darkness, 10, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 5,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 312, cast_dominate_undead, 20, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 6,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0, 0 },
    { 313, cast_unsummon, 5, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 7,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 40, 0 },
    { 314, cast_siphon_strength, 15, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 8,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 20, 0 },
    { 315, cast_gather_shadows, 5, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 8,
      TAR_IGNORE, 0, 0 },
    { 316, cast_mend_bones, 11, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 10,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 50, 0 },
    { 317, cast_trace_corpse, 20, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 11,
      TAR_OBJ_WORLD, 0, 0 },
    { 318, cast_endure_cold, 25, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 12,
      TAR_IGNORE, 0, 0 },
    { 319, cast_life_draw, 12, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 15,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 20, 0 },
    { 320, cast_numb_dead, 30, 36, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 15,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT, 0, 0 },
    { 321, cast_binding, 15, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 16,
      TAR_IGNORE, 0, 0 },
    { 322, cast_decay, 20, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 18,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 10, 0 },
    { 323, cast_shadow_step, 20, 24, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 20,
      TAR_IGNORE, 0, 0 },
    { 324, cast_cavorting_bones, 25, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 22,
      TAR_OBJ_ROOM, 0, 0 },
    { 325, cast_mist_of_death, 35, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 23,
      TAR_IGNORE | TAR_VIOLENT, 80, 0 },
    { 326, cast_nullify, 20, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 23,
      TAR_CHAR_ROOM, 0, 0 },
    { 327, cast_dark_empathy, 30, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 24,
      TAR_CHAR_ROOM, 70, 0 },
    { 328, cast_eye_of_the_dead, 25, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 26,
      TAR_IGNORE, 0, 0 },
    { 329, cast_soul_steal, 20, 12, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 27,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 330, cast_life_leech, 20, 12, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 31,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 30, 0 },
    { 331, cast_dark_pact, 33, 24, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 36,
      TAR_IGNORE, 0, 0 },
    { 332, cast_darktravel, 30, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 39,
      TAR_CHAR_WORLD, 0, 0 },
    { 333, cast_vampiric_embrace, 50, 36, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 41,
      TAR_IGNORE, 0, 0 },
    { 334, cast_bind_affinity, 50, 36, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 48,
      TAR_IGNORE, 0, 0 },
    { 335, cast_scourge_warlock, 35, 12, POSITION_FIGHTING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 29,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 50, 0 },
    { 336, cast_finger_of_death, 20, 24, POSITION_FIGHTING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 25,
      TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_VIOLENT, 60, 0 },
    { 337, cast_flesh_golem, 33, 36, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 37,
      TAR_OBJ_ROOM, 0, 0 },
    { 338, cast_chillshield, 40, 24, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 45,
      TAR_IGNORE, 0, 0 },
    { 341, cast_blade_barrier, 40, 24, POSITION_STANDING,
      IMMORTAL, 45, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 342, cast_mana_shield, 50, 36, POSITION_STANDING, 
      14, IMMORTAL, IMMORTAL, 14, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 343, cast_iron_skins, 65, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 20, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 344, mind_use_sense_object, 25, 12, POSITION_STANDING,
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 25, IMMORTAL,
      TAR_OBJ_WORLD, 0, 0 },
    { 345, mind_use_kinolock, 15, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, 9, IMMORTAL,
      TAR_IGNORE, 0, 0 },
    { 349, cast_group_heal, 75, 12, POSITION_FIGHTING,
      IMMORTAL, 29, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_IGNORE, 60, 0 },
    { 350, cast_plant_gate, 60, 12, POSITION_STANDING, 
      IMMORTAL, IMMORTAL, 40, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL, IMMORTAL,
      TAR_OBJ_WORLD, 0, 0 }
};
int spell_info_count = NELEMS(spell_info);
int spell_index[MAX_SPL_LIST];


int SPELL_LEVEL(struct char_data *ch, int sn)
{
    int             index;
    int             min;

    min = ABS_MAX_LVL;
    index = spell_index[sn];
    if( index == -1 ) {
        return( NEVER_USE );
    }

    if (HasClass(ch, CLASS_MAGIC_USER)) {
        min = MIN(min, spell_info[index].min_level_magic);
    }
    if (HasClass(ch, CLASS_SORCERER)) {
        min = MIN(min, spell_info[index].min_level_sorcerer);
    }
    if (HasClass(ch, CLASS_NECROMANCER)) {
        min = MIN(min, spell_info[index].min_level_necromancer);
    }
    if (HasClass(ch, CLASS_CLERIC)) {
        min = MIN(min, spell_info[index].min_level_cleric);
    }
    if (HasClass(ch, CLASS_PALADIN)) {
        min = MIN(min, spell_info[index].min_level_paladin);
    }
    if (HasClass(ch, CLASS_RANGER)) {
        min = MIN(min, spell_info[index].min_level_ranger);
    }
    if (HasClass(ch, CLASS_PSI)) {
        min = MIN(min, spell_info[index].min_level_psi);
    }
    if (HasClass(ch, CLASS_DRUID)) {
        min = MIN(min, spell_info[index].min_level_druid);
    }
    return (min);
}

#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

/*
 * This function returns the damage that a player will get when in a
 * certain sector type 
 */
int RoomElementalDamage(int flags, struct char_data *ch)
{
    int             damage = number(25, 50);

    if (IS_SET(ch->specials.act, PLR_NOOUTDOOR)) {
        return 0;
    }

    if (IS_SET(flags, FIRE_ROOM)) {
        if (IS_IMMUNE(ch, IMM_FIRE)) {
            return 0;
        }
        if (IsResist(ch, IMM_FIRE)) {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_FIRE)) {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin.. OUCH\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000RYou feel the heat from the area start to burn "
                         "your skin... OUCH!!\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, ICE_ROOM)) {
        if (IS_IMMUNE(ch, IMM_COLD)) {
            return 0;
        }
        if (IsResist(ch, IMM_COLD)) {   
            send_to_char("$c000CThe cold and frost seem to be taking "
                         "toll...\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_COLD)) {
            send_to_char("$c000CThe cold and frost seem to be taking toll... "
                         "OUCH\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000CThe cold and frost seem to be taking toll... "
                         "OUCH!!\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, EARTH_ROOM)) {
        if (IS_IMMUNE(ch, IMM_BLUNT)) {
            return 0;
        }
        if (IsResist(ch, IMM_BLUNT)) {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_BLUNT)) {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000yThe earch starts to shake, the ground crumbles "
                         "beneath you causing you great pain.\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, ELECTRIC_ROOM)) {
        if (IS_IMMUNE(ch, IMM_ELEC)) {
            return 0;
        }
        if (IsResist(ch, IMM_ELEC)) {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_ELEC)) {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000BElectricity surges up through the ground and "
                         "through your body causing you great pain.\n\r", ch);
            return damage;
        }
    }

    if (IS_SET(flags, WIND_ROOM)) {
        if (IS_IMMUNE(ch, IMM_PIERCE)) {
            return 0;
        }
        if (IsResist(ch, IMM_PIERCE)) {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage / 2;
        } else if (IsSusc(ch, IMM_PIERCE)) {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage * 2;
        } else {
            send_to_char("$c000bA sudden wind picks up and starts tossing "
                         "debris throughout the area.. The piercing sticks "
                         "and rocks cause you great pain.\n\r", ch);
            return damage;
        }
    }

    return 0;
}


int GetMoveRegen(struct char_data *i)
{
    int             damagex = 0;

    /*
     * Movement
     */
    if (ValidRoom(i) && IS_SET(real_roomp(i->in_room)->room_flags, MOVE_ROOM)) {
        send_to_char("Your feel your stamina decrease.\n\r", i);
        damagex = number(15, 30);
    }
    return GET_MOVE(i) + move_gain(i) - damagex;

}

int GetHitRegen(struct char_data *i)
{
    char            buf[256];
    int             damagex = 0,
#if 0
                    trollregen = 0,
#endif
                    darkpact = 0;
#if 0
    if(GET_RACE(i) == RACE_TROLL && GET_HIT(i)!=hit_limit(i)) {
        trollregen=hit_gain(i)*0.5; 
        send_to_char("Your wounds seem to close up and heal over some.\n\r",i);
        sprintf(buf,"%s's wounds seem to close up.\n\r",GET_NAME(i)); 
        send_to_room_except(buf,i->in_room,i);
    } 
#endif

    /*
     * darkpact regen penalty 
     */
    if (affected_by_spell(i, SPELL_DARK_PACT)) {
        send_to_char("$c0008Your pact with the Dark Lord grants your mental "
                     "power a boost, but drains some of your life.\n\r", i);
        darkpact = 15;
    }

    /*
     * Damage sector regen!!!!
     */
    damagex = RoomElementalDamage(real_roomp(i->in_room)->room_flags, i);
    if (damagex != 0) {
        sprintf(buf, "%s screams in pain!\n\r", GET_NAME(i));
        send_to_room_except(buf, i->in_room, i);
    }

    /*
     * Lets regen the character's Hitpoints
     */
    return hit_gain(i) + GET_HIT(i) - damagex - darkpact;       
#if 0
    +trollregen;
#endif
}

int ValidRoom(struct char_data *ch)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0        
        Log("/* no room? BLAH!!! least it never crashed */");
#endif        
        return (FALSE);
    }
    return (TRUE);
}

int GetManaRegen(struct char_data *i)
{
    int             damagex = 0,
                    darkpact = 0;

    if (ValidRoom(i) && IS_SET(real_roomp(i->in_room)->room_flags, MANA_ROOM)) {
        send_to_char("You feel your aura being drained by some unknown "
                     "force!\n\r", i);
        damagex = number(15, 30);
    }

    /*
     * darkpact regen bonus 
     */
    if (affected_by_spell(i, SPELL_DARK_PACT)) {
        darkpact = 30;
    }

    /*
     * Mana
     */
    return GET_MANA(i) + mana_gain(i) - damagex + darkpact;
}

void affect_update(int pulse)
{
    register struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    register struct obj_data *j;
    struct obj_data *next_thing;
    struct char_data *next_char;
    struct room_data *rp;
    int             dead = FALSE,
                    room;
    int             regenroom = 0;

    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        /*
         *  check the effects on the char
         */
        dead = FALSE;
        for (af = i->affected; af && !dead; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1 && af->type != SKILL_MEMORIZE) {
                af->duration--;
                if (af->duration == 1) {
                    SpellWearOffSoon(af->type, i);
                }
            } else if (af->type != SKILL_MEMORIZE) {
                /* dur < 1 */
                /*
                 * It must be a spell 
                 *
                 * /----- was FIRST_BREATH_WEAPON 
                 */
                if (af->type > 0 && af->type < MAX_EXIST_SPELL && 
                    af->type != SKILL_MEMORIZE) {
                    if (!af->next || af->next->type != af->type ||
                        af->next->duration > 0) {
                        SpellWearOff(af->type, i);
                        /*
                         * moved to it's own pulse update bcw
                         * check_memorize(i,af); 
                         */
#if 0
                        if (!affected_by_spell(i, SPELL_CHARM_PERSON))
                            /*
                             * added to fix bug 
                             */
                            /*
                             * ro does not remove it here! 
                             */
                            affect_remove(i, af);       /* msw 12/16/94 */
#endif
                    }
                } else if (af->type >= FIRST_BREATH_WEAPON && 
                           af->type <= LAST_BREATH_WEAPON) {
                    (bweapons[af->type - FIRST_BREATH_WEAPON])
                        (-af->modifier / 2, i, "", SPELL_TYPE_SPELL, i, 0);

                    if (!i->affected) {
                        /*
                         * oops, you're dead :) 
                         */
                        dead = TRUE;
                        break;
                    }

                    /*
                     * ro does not do it here either
                     *
                     *
                     * affect_remove(i, af); 
                     */
                }
                /*
                 * ro does it here! 
                 */
                affect_remove(i, af);
            }
        }

        if (!dead) {
            if (GET_POS(i) >= POSITION_STUNNED && (i->desc || !IS_PC(i))) {
                /*
                 * note - because of poison, this one has to be in the
                 * opposite order of the others.  The logic:
                 * 
                 * hit_gain() modifies the characters hps if they are
                 * poisoned. but if they were in the opposite order, the
                 * total would be: hps before poison + gain.  But of
                 * course, the hps after poison are lower, but No one
                 * cares! and that is why the gain is added to the hits,
                 * not vice versa 
                 */

                /*
                 * Regen mana/hitpoint/move
                 */
                regenroom = 0;

                if (ValidRoom(i) && 
                    IS_SET(real_roomp(i->in_room)->room_flags, REGEN_ROOM)) {
                    regenroom = 10;
                    if (GET_POS(i) > POSITION_SITTING) {
                        /* 
                         * Standing, fighting etc 
                         */
                        regenroom = 15;
                    } else if (GET_POS(i) > POSITION_SLEEPING) {
                        /* 
                         * Resting and sitting 
                         */
                        regenroom = 20;
                    } else if (GET_POS(i) > POSITION_STUNNED) {
                        /* 
                         * sleeping 
                         */
                        regenroom = 25;
                    } else {
                        regenroom = 20;
                    }
                    /*
                     * make it so imms can forego seeing this: 
                     */
                    if (!IS_SET(i->specials.act, PLR_NOOUTDOOR)) {
                        if (GET_HIT(i) != GET_MAX_HIT(i)) {
                            send_to_char("Your wounds seem to heal "
                                         "exceptionally quick.\n\r", i);
                        } else if (GET_MANA(i) != GET_MAX_MANA(i)) {
                            send_to_char("You feel your mystical abilities "
                                         "increase.\n\r", i);
                        } else if (GET_MOVE(i) != GET_MAX_MOVE(i)) {
                            send_to_char("Your stamina seems to increase "
                                         "rather quick.\n\r", i);
                        }
                    }
                }

                /*
                 * Lets regen the character's Hitpoints
                 */
                GET_HIT(i) = MIN(GetHitRegen(i) + regenroom, hit_limit(i));
                GET_MANA(i) = MIN(GetManaRegen(i) + regenroom, mana_limit(i));
                GET_MOVE(i) = MIN(GetMoveRegen(i) + regenroom, move_limit(i));
#if 0
                if (i->pc) {
                    GET_DIMD(i) += 2;
                }
#endif

                update_pos(i);
                if (GET_POS(i) == POSITION_DEAD) {
                    die(i, '\0');
                } else if (GET_POS(i) == POSITION_STUNNED) {
                    update_pos(i);
                }
            } else if (GET_POS(i) == POSITION_INCAP) {
                /*
                 * do nothing 
                 */ 
                damage(i, i, 0, TYPE_SUFFERING);
            } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
                damage(i, i, 1, TYPE_SUFFERING);
            }

            if (IS_PC(i)) {
                update_char_objects(i);
                if (GetMaxLevel(i) < DEMIGOD && i->in_room != 3 && 
                    i->in_room != 2) {
                    check_idling(i);
                }

                rp = real_roomp(i->in_room);
                if (rp) {
                    if (rp->sector_type == SECT_WATER_SWIM ||
                        rp->sector_type == SECT_WATER_NOSWIM) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -2);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -1);
                        }
                    } else if (rp->sector_type == SECT_DESERT) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -3);
                            gain_condition(i, THIRST, -3);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -3);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -2);
                        }
                    } else if (rp->sector_type == SECT_MOUNTAIN ||
                               rp->sector_type == SECT_HILLS) {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                        } else {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                        }
                    } else {
                        if (GET_RACE(i) == RACE_HALFLING) {
                            gain_condition(i, FULL, -2);
                            gain_condition(i, DRUNK, -2);
                            gain_condition(i, THIRST, -2);
                        } else if (GET_RACE(i) == RACE_HALF_OGRE) {
                            gain_condition(i, FULL, -3);
                            gain_condition(i, DRUNK, -3);
                            gain_condition(i, THIRST, -3);
                        } else {
                            gain_condition(i, FULL, -1);
                            gain_condition(i, DRUNK, -1);
                            gain_condition(i, THIRST, -1);
                        }
                    }
                }

                if (i->specials.tick == time_info.hours) { 
                    /* 
                     * works for 24, change for anything else 
                     */
                    if (!IS_IMMORTAL(i) && i->in_room != 3) {
                        /* 
                         * the special case for room 3 is a hack to keep link
                         * dead people who have no stuff from being saved
                         * without stuff... 
                         */
                        do_save(i, "", 0);
                    }
                }
            }
            check_nature(i);    
            /* 
             * check falling, check drowning, etc 
             */
        }
    }

    /*
     *  update the objects
     */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;   
        /* 
         * Next in object list 
         */

        /*
         * If this is a corpse 
         */
        if (GET_ITEM_TYPE(j) == ITEM_CONTAINER && j->obj_flags.value[3]) {
            /*
             * timer count down 
             */
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by) {
                    act("$p biodegrades in your hands. Everything in it falls "
                        "to the floor", FALSE, j->carried_by, j, 0, TO_CHAR);
                } else if (j->in_room != NOWHERE && 
                           real_roomp(j->in_room)->people) {
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                ObjFromCorpse(j);
            }
        } else if (obj_index[j->item_number].virtual == EMPTY_SCROLL) {
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by)  {
                    act("$p crumbles to dust.", FALSE, j->carried_by, j, 0,
                        TO_CHAR);
                } else if (j->in_room != NOWHERE && 
                           real_roomp(j->in_room)->people) {
                    act("$p crumbles to dust.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p crumbles to dust.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                extract_obj(j);
            }
        } else if (obj_index[j->item_number].virtual == EMPTY_POTION) {
            if (j->obj_flags.timer > 0) {
                j->obj_flags.timer--;
            }
            if (!j->obj_flags.timer) {
                if (j->carried_by) {
                    act("$p dissolves into nothingness.", FALSE,
                        j->carried_by, j, 0, TO_CHAR);
                } else if ((j->in_room != NOWHERE)
                           && (real_roomp(j->in_room)->people)) {
                    act("$p dissolves into nothingness.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_ROOM);
                    act("$p dissolves into nothingness.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                extract_obj(j);
            }
        } else {
            /*
             *  Sound objects
             */
            if (ITEM_TYPE(j) == ITEM_AUDIO) {
                if ((j->obj_flags.value[0] &&
                     pulse % j->obj_flags.value[0] == 0) || !number(0, 5)) {
                    if (j->carried_by) {
                        room = j->carried_by->in_room;
                    } else if (j->equipped_by) {
                        room = j->equipped_by->in_room;
                    } else if (j->in_room != NOWHERE) {
                        room = j->in_room;
                    } else {
                        room = RecGetObjRoom(j);
                    }
                    /*
                     *  broadcast to room
                     */

                    if (j->action_description) {
                        MakeNoise(room, j->action_description,
                                  j->action_description);
                    }
                }
            } else {
                if (obj_index[j->item_number].func && j->item_number >= 0) {
                    (*obj_index[j->item_number].func) (0, 0, 0, j, PULSE_TICK);
                }
            }
        }
    }
}

void update_mem(int pulse)
{
    register struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    struct char_data *next_char;

    /*
     * start looking here 
     */
    for (i = mem_list; i; i = next_char) {
        next_char = i->next_memorize;
        for (af = i->affected; af; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1 && af->type == SKILL_MEMORIZE) {
                af->duration--;
            } else if (af->type == SKILL_MEMORIZE) {
                SpellWearOff(af->type, i);
                check_memorize(i, af);
                stop_memorizing(i);
                affect_remove(i, af);
            }
        }
    }
}

void stop_memorizing(struct char_data *ch)
{
    struct char_data *tmp;

    if (mem_list == ch && !ch->next_memorize) {
        mem_list = 0;
    } else if (mem_list == ch) {
        mem_list = ch->next_memorize;
    } else {
        for (tmp = mem_list; tmp && (tmp->next_memorize != ch);
             tmp = tmp->next_memorize) {
            /*
             * Empty loop
             */
        }
        if (!tmp) {
            Log("Char memorize not found Error (spell_parser.c, "
                "stop_memorizing)");
            SpellWearOff(SKILL_MEMORIZE, ch);
            affect_from_char(ch, SKILL_MEMORIZE);
            ch->next_memorize = 0;
            return;
        }
        tmp->next_memorize = ch->next_memorize;
    }

    ch->next_memorize = 0;
}

#if 0
        /*
         * testing affect_update() 
         */
void affect_update(int pulse)
{
    struct descriptor_data *d;
    static struct affected_type *af,
                   *next_af_dude;
    register struct char_data *i;
    register struct obj_data *j;
    struct obj_data *next_thing;
    struct char_data *next_char;
    struct room_data *rp;
    int             dead = FALSE,
                    room,
                    y,
                    x,
                    bottom = 1,
                    top = 1,
                    time_until_backup = 1;
    long            time_until_reboot;
    char            buf[400];
    struct char_data *vict,
                   *next_v;
    struct obj_data *obj,
                   *next_o;

    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        /*
         *  check the effects on the char
         */
        dead = FALSE;
        for (af = i->affected; af && !dead; af = next_af_dude) {
            next_af_dude = af->next;
            if (af->duration >= 1)
                af->duration--;
            else {
                /*
                 * It must be a spell 
                 */
                if ((af->type > 0) && (af->type <= 120)) {      /* urg.. a 
                                                                 * constant 
                                                                 */
                    if (!af->next || (af->next->type != af->type) ||
                        (af->next->duration > 0)) {
                        if (*spell_wear_off_msg[af->type]) {
                            send_to_char(spell_wear_off_msg[af->type], i);
                            send_to_char("\n\r", i);
                        }

                    }
                } else if (af->type >= FIRST_BREATH_WEAPON &&
                           af->type <= LAST_BREATH_WEAPON) {
                    extern funcp    bweapons[];
                    bweapons[af->type -
                             FIRST_BREATH_WEAPON] (-af->modifier / 2, i,
                                                   "", SPELL_TYPE_SPELL, i,
                                                   0);
                    if (!i->affected) {
                        /*
                         * oops, you're dead :) 
                         */
                        dead = TRUE;
                        break;
                    }
                }

                affect_remove(i, af);
            }
        }
        if (!dead) {

            if (GET_POS(i) >= POSITION_STUNNED) {
                GET_HIT(i) = MIN(GET_HIT(i) + hit_gain(i), hit_limit(i));
                GET_MANA(i) =
                    MIN(GET_MANA(i) + mana_gain(i), mana_limit(i));
                GET_MOVE(i) =
                    MIN(GET_MOVE(i) + move_gain(i), move_limit(i));
                if (GET_POS(i) == POSITION_STUNNED)
                    update_pos(i);
            } else if (GET_POS(i) == POSITION_INCAP) {
                /*
                 * do nothing 
                 */ damage(i, i, 0, TYPE_SUFFERING);
            } else if (!IS_NPC(i) && (GET_POS(i) == POSITION_MORTALLYW)) {
                damage(i, i, 1, TYPE_SUFFERING);
            }
            if (IS_PC(i)) {
                update_char_objects(i);
                if (GetMaxLevel(i) < DEMIGOD)
                    check_idling(i);
                rp = real_roomp(i->in_room);
                if (rp) {
                    if (rp->sector_type == SECT_WATER_SWIM ||
                        rp->sector_type == SECT_WATER_NOSWIM) {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -1);
                    } else if (rp->sector_type == SECT_DESERT) {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -2);
                        gain_condition(i, THIRST, -2);
                    } else if (rp->sector_type == SECT_MOUNTAIN ||
                               rp->sector_type == SECT_HILLS) {
                        gain_condition(i, FULL, -2);
                        gain_condition(i, DRUNK, -2);
                    } else {
                        gain_condition(i, FULL, -1);
                        gain_condition(i, DRUNK, -1);
                        gain_condition(i, THIRST, -1);
                    }
                }

            }
        }
    }

    /*
     *  update the objects
     */

    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;   /* Next in object list */

        /*
         * If this is a corpse 
         */
        if ((GET_ITEM_TYPE(j) == ITEM_CONTAINER) &&
            (j->obj_flags.value[3])) {
            /*
             * timer count down 
             */
            if (j->obj_flags.timer > 0)
                j->obj_flags.timer--;

            if (!j->obj_flags.timer) {
                if (j->carried_by)
                    act("$p biodegrades in your hands.",
                        FALSE, j->carried_by, j, 0, TO_CHAR);
                else if ((j->in_room != NOWHERE) &&
                         (real_roomp(j->in_room)->people)) {
                    act("$p dissolves into a fertile soil.",
                        TRUE, real_roomp(j->in_room)->people, j, 0,
                        TO_ROOM);
                    act("$p dissolves into a fertile soil.", TRUE,
                        real_roomp(j->in_room)->people, j, 0, TO_CHAR);
                }
                ObjFromCorpse(j);
            }
        } else {

            /*
             *  Sound objects
             */
            if (ITEM_TYPE(j) == ITEM_AUDIO) {
                if (((j->obj_flags.value[0]) &&
                     (pulse % j->obj_flags.value[0]) == 0) ||
                    (!number(0, 5))) {
                    if (j->carried_by) {
                        room = j->carried_by->in_room;
                    } else if (j->equipped_by) {
                        room = j->equipped_by->in_room;
                    } else if (j->in_room != NOWHERE) {
                        room = j->in_room;
                    } else {
                        room = RecGetObjRoom(j);
                    }
                    /*
                     *  broadcast to room
                     */

                    if (j->action_description) {
                        MakeNoise(room, j->action_description,
                                  j->action_description);
                    }
                }
            }
        }
    }
}
#endif


/*
 * Check if making CH follow VICTIM will create an illegal 
 * Follow "Loop/circle" 
 * Problem: Appears to give us an infinite loop every once in a while 
 * Attempting to put in some guide to stop it...  Temp fix (GH)
 */
bool circle_follow(struct char_data *ch, struct char_data *victim)
{
    struct char_data *k;
    int             counter = 0;

    for (k = victim; k; k = k->master) {
        counter++;
        if (k == ch) {
            return (TRUE);
        }
        if (counter > 20) {
            Log("Possible infinite Loop in circle follower?");
            return (TRUE);
        }
    }

    return (FALSE);
}

/*
 * Called when stop following persons, or stopping charm 
 * This will NOT do if a character quits/dies!! 
 */
void stop_follower(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (!ch->master) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

        if (affected_by_spell(ch, SPELL_CHARM_PERSON)) {
            affect_from_char(ch, SPELL_CHARM_PERSON);
        }
    } else {
        act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
            act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
        }
    }

    if (ch->master->followers->follower == ch) {
        /* 
         * Head of follower-list? 
         */
        k = ch->master->followers;
        ch->master->followers = k->next;
        if (k)
            free(k);
    } else {
        /* 
         * locate follower who is not head of list 
         */
        for (k = ch->master->followers; k->next && k->next->follower != ch;
             k = k->next) {
            /*
             * Empty loop
             */
        }

        if (k->next) {
            j = k->next;
            k->next = j->next;
            if (j) {
                free(j);
            }
        } else {
            assert(FALSE);
        }
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

void stop_follower_quiet(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (!ch->master) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_CHARM)) {
        act("You realize that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_CHAR);
        act("$n realizes that $N is a jerk!", FALSE, ch, 0, ch->master,
            TO_NOTVICT);
        act("$n hates your guts!", FALSE, ch, 0, ch->master, TO_VICT);

        if (affected_by_spell(ch, SPELL_CHARM_PERSON)) {
            affect_from_char(ch, SPELL_CHARM_PERSON);
        }
    } else {
#if 0
        act("You stop following $N.", FALSE, ch, 0, ch->master, TO_CHAR);
#endif
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
#if 0
            act("$n stops following $N.", FALSE, ch, 0, ch->master, TO_NOTVICT);
            act("$n stops following you.", FALSE, ch, 0, ch->master, TO_VICT);
#endif
        }
    }

    if (ch->master->followers->follower == ch) {
        /* 
         * Head of follower-list? 
         */
        k = ch->master->followers;
        ch->master->followers = k->next;
        if (k) {
            free(k);
        }
    } else {
        /* 
         * locate follower who is not head of list 
         */
        for (k = ch->master->followers; k->next && k->next->follower != ch;
             k = k->next) {
            /*
             * Empty loop
             */
        }
        if (k->next) {
            j = k->next;
            k->next = j->next;
            if (j) {
                free(j);
            }
        } else {
            assert(FALSE);
        }
    }

    ch->master = 0;
    REMOVE_BIT(ch->specials.affected_by, AFF_CHARM | AFF_GROUP);
}

/*
 * Called when a character that follows/is followed dies 
 */
void die_follower(struct char_data *ch)
{
    struct follow_type *j,
                   *k;

    if (ch->master) {
        stop_follower(ch);
    }
    for (k = ch->followers; k; k = j) {
        j = k->next;
        stop_follower(k->follower);
    }
}

/*
 * Do NOT call this before having checked if a circle of followers 
 * will arise. CH will follow leader 
 */
void add_follower(struct char_data *ch, struct char_data *leader)
{
    struct follow_type *k;
    char            buf[200];

    /*
     * instead of crashing the mud we try this 
     */
    if (ch->master) {
        act("$n cannot follow you for some reason.", TRUE, ch, 0, leader,
            TO_VICT);
        act("You cannot follow $N for some reason.", TRUE, ch, 0, leader,
            TO_CHAR);
        sprintf(buf, "%s cannot follow %s for some reason", GET_NAME(ch),
                GET_NAME(leader));
        Log(buf);
        return;
    }

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->next = leader->followers;
    leader->followers = k;

    act("You now follow $N.", FALSE, ch, 0, leader, TO_CHAR);
    if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
        act("$n starts following you.", TRUE, ch, 0, leader, TO_VICT);
        act("$n now follows $N.", TRUE, ch, 0, leader, TO_NOTVICT);
    }
}

struct syllable {
    char            org[10];
    char            new[10];
};

struct syllable syls[] = {
    {" ", " "}, {"ar", "abra"}, {"au", "kada"}, {"bless", "fido"},
    {"blind", "nose"}, {"bur", "mosa"}, {"cu", "judi"}, {"ca", "jedi"},
    {"de", "oculo"}, {"en", "unso"}, {"light", "dies"}, {"lo", "hi"},
    {"mor", "zak"}, {"move", "sido"}, {"ness", "lacri"}, {"ning", "illa"},
    {"per", "duda"}, {"ra", "gru"}, {"re", "candus"}, {"son", "sabru"},
    {"se", "or"}, {"tect", "infra"}, {"tri", "cula"}, {"ven", "nofo"},
    {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"}, {"e", "z"}, {"f", "y"},
    {"g", "o"}, {"h", "p"}, {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
    {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"}, {"q", "d"}, {"r", "f"},
    {"s", "g"}, {"t", "h"}, {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
    {"y", "l"}, {"z", "k"}, {"", ""}
};

void say_spell(struct char_data *ch, int si)
{
    char            buf[MAX_STRING_LENGTH],
                    splwd[MAX_BUF_LENGTH];
    char            buf2[MAX_STRING_LENGTH];

    int             j,
                    offs;
    struct char_data *temp_char;
    extern struct syllable syls[];

    strcpy(buf, "");
    strcpy(splwd, spells[si - 1]);

    offs = 0;

    while (*(splwd + offs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, splwd + offs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                if (strlen(syls[j].org)) {
                    offs += strlen(syls[j].org);
                } else {
                    ++offs;
                }
            }
        }
    }

    sprintf(buf2, "$n utters the words, '%s'", buf);
    sprintf(buf, "$n utters the words, '%s'", spells[si - 1]);

    for (temp_char = real_roomp(ch->in_room)->people;
         temp_char; temp_char = temp_char->next_in_room) {
        if (temp_char != ch) {
            if (GET_RACE(ch) == GET_RACE(temp_char)) {
                act(buf, FALSE, ch, 0, temp_char, TO_VICT);
            } else {
                act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
            }
        }
    }
}

void weave_song(struct char_data *ch, int si)
{
    char            buf[MAX_STRING_LENGTH],
                    splwd[MAX_BUF_LENGTH];
    char            buf2[MAX_STRING_LENGTH];

    int             j,
                    offs;
    struct char_data *temp_char;
    extern struct syllable syls[];

    strcpy(buf, "");
    strcpy(splwd, spells[si - 1]);

    offs = 0;

    while (*(splwd + offs)) {
        for (j = 0; *(syls[j].org); j++) {
            if (!strncmp(syls[j].org, splwd + offs, strlen(syls[j].org))) {
                strcat(buf, syls[j].new);
                if (strlen(syls[j].org)) {
                    offs += strlen(syls[j].org);
                } else {
                    ++offs;
                }
            }
        }
    }

    sprintf(buf2, "$n utters the words, '%s'", buf);
    sprintf(buf, "$n magically weaves the song of %s.", spells[si - 1]);

    for (temp_char = real_roomp(ch->in_room)->people; temp_char;
         temp_char = temp_char->next_in_room) {
        if (temp_char != ch) {
            if (GET_RACE(ch) == GET_RACE(temp_char)) {
                act(buf, FALSE, ch, 0, temp_char, TO_VICT);
            } else {
                act(buf2, FALSE, ch, 0, temp_char, TO_VICT);
            }
        }
    }
}

bool saves_spell(struct char_data *ch, sh_int save_type)
{
    int             save;

    /*
     * Negative apply_saving_throw makes saving throw better! 
     */

    save = ch->specials.apply_saving_throw[save_type];

    if (!IS_NPC(ch)) {
        save += saving_throws[BestMagicClass(ch)][save_type]
                             [(int)GET_LEVEL(ch, BestMagicClass(ch))];
        if (GetMaxLevel(ch) > MAX_MORT) {
            return (TRUE);
        }
    }

    if (GET_RACE(ch) == RACE_GOD) {
        /* 
         * gods always save 
         */
        return (1);
    }

    return (MAX(5, save) < number(1, 50));
#if 0
    return(MAX(1,save) < number(1,20));
    char buf[200]; 
    int saveroll = number(1,50); 
    bool didsave = MAX(5,save) < saveroll; 
    sprintf(buf,"NaturalSave: %d SaveBonus: %d NumberRolled: %d "
                "DidSave?: %d\n\r", 
            saving_throws[BestMagicClass(ch)][save_type]
            [GET_LEVEL(ch,BestMagicClass(ch))], 
            ch->specials.apply_saving_throw[save_type], saveroll, didsave);
    Log(buf); return(didsave);
#endif
}

bool ImpSaveSpell(struct char_data * ch, sh_int save_type, int mod)
{
    int             save;

    /*
     * Positive mod is better for save 
     * Negative apply_saving_throw makes saving throw better! 
     */

    save = ch->specials.apply_saving_throw[save_type] - mod;

    if (!IS_NPC(ch)) {
        save += saving_throws[BestMagicClass(ch)][save_type]
                             [(int)GET_LEVEL(ch, BestMagicClass(ch))];
        if (GetMaxLevel(ch) >= IMMORTAL) {
            return (TRUE);
        }
    }

    return (MAX(1, save) < number(1, 20));
}

char           *skip_spaces(char *string)
{
    for (; *string && isspace(*string); string++) {
        /* 
         * Empty loop 
         */
    }

    return (string);
}

void do_id(struct char_data *ch, char *argument, int cmd)
{
    char            buf[256];

    sprintf(buf, "'id' %s", argument);
    do_cast(ch, buf, 0);
}

/*
 * Assumes that *argument does start with first letter of chopped string 
 */
void do_cast(struct char_data *ch, char *argument, int cmd)
{
    char            buf[254];
    struct obj_data *tar_obj;
    struct char_data *tar_char;
    char            name[MAX_INPUT_LENGTH];
    char            ori_argument[256];
    int             qend,
                    spl,
                    index,
                    i,
                    exp;
    bool            target_ok;
    int             max,
                    cost;

    if (affected_by_spell(ch, SPELL_FEEBLEMIND)) {
        send_to_char("Der, what is that?\n\r", ch);
        return;
    }

    if (!IsHumanoid(ch)) {
        send_to_char("Sorry, you don't have the right form for that.\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch)) {
        if (BestMagicClass(ch) == WARRIOR_LEVEL_IND) {
            send_to_char("Think you had better stick to fighting...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == THIEF_LEVEL_IND) {
            send_to_char("Think you should stick to stealing...\n\r", ch);
            return;
        } else if (BestMagicClass(ch) == MONK_LEVEL_IND) {
            send_to_char("Think you should stick to meditating...\n\r", ch);
            return;
        }
    }

    if (A_NOMAGIC(ch)) {
        send_to_char("The arena rules do not allow the use of magic!\n\r", ch);
        return;
    }

    if (cmd != 370 && apply_soundproof(ch)) {
        send_to_char("Too silent here to make a noise.\n\r", ch);
        return;
    }

    if (cmd == 370 && !HasClass(ch, CLASS_PSI)) {
        /* 
         * take away mind spells for non psi 
         */
        send_to_char("You, think, think harder.. and nearly bust a vein.\n\r",
                     ch);
        return;
    }

    if (!IS_IMMORTAL(ch) && HasClass(ch, CLASS_NECROMANCER) && 
        GET_ALIGNMENT(ch) >= -350) {
        /*
         * necro too GOOD to cast 
         */
        send_to_char("Alas, you have strayed too far from the Dark Lord's "
                     "guidance.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    for (i = 0; argument[i] && (i < 255); i++) {
        ori_argument[i] = argument[i];
    }
    ori_argument[i] = '\0';

    /*
     * If there is no chars in argument 
     */
    if (!(*argument)) {
        if (cmd != 600) {
            send_to_char("Cast which what where?\n\r", ch);
        } else {
            send_to_char("Sing which what where?\n\r", ch);
        }
        return;
    }

    if (*argument != '\'') {
        if (cmd != 600) {
            send_to_char("Magic must always be enclosed by the holy magic "
                         "symbols : '\n\r", ch);
        } else {
            send_to_char("Songs must always be enclosed by the vibrant symbols"
                         " : '\n\r", ch);
        }
        return;
    }

    /*
     * Locate the last quote && lowercase the magic words (if any) 
     */
    for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++) {
        *(argument + qend) = LOWER(*(argument + qend));
    }

    if (*(argument + qend) != '\'') {
        if (cmd != 600) {
            send_to_char("Magic must always be enclosed by the holy magic "
                         "symbols : '\n\r", ch);
        } else {
            send_to_char("Songs must always be enclosed by the vibrant symbols"
                         " : '\n\r", ch);
        }
        return;
    }

    spl = old_search_block(argument, 1, qend - 1, spells, 0);

    if (!spl) {
        send_to_char("Nothing seems to happen! Wow! \n\r", ch);
        return;
    }

    /*
     * mobs do not get skills so we just check it for PC's 
     */
    if (!ch->skills && (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF))) {
        send_to_char("You do not have skills!\n\r", ch);
        return;
    }

    if (cmd != 370 && OnlyClass(ch, CLASS_PSI)) {
        send_to_char("Use your mind!\n\r", ch);
        return;
    }

    if ((cmd == 84 || cmd == 370) && OnlyClass(ch, CLASS_SORCERER)) {
        send_to_char("You must use recall.\n\r", ch);
        return;
    }

    if ((cmd == 84 || cmd == 370) && HasClass(ch, CLASS_SORCERER) &&
        !IS_IMMORTAL(ch) && 
        IS_SET(ch->skills[spl].flags, SKILL_KNOWN_SORCERER)) {
        send_to_char("You must use recall for this spell.\n\r", ch);
        return;
    }

    if (ch->skills[spl].learned == 0) {
        send_to_char("You have no knowledge of this spell.\n\r", ch);
        return;
    }

    if (spl > 0 && spl < MAX_SKILLS && (index = spell_index[spl]) != -1 &&
        spell_info[index].spell_pointer) {
        if (GET_POS(ch) < spell_info[index].minimum_position) {
            switch (GET_POS(ch)) {
            case POSITION_SLEEPING:
                send_to_char("You dream about great magical powers.\n\r", ch);
                break;
            case POSITION_RESTING:
                send_to_char("You can't concentrate enough while resting.\n\r",
                             ch);
                break;
            case POSITION_SITTING:
                send_to_char("You can't do this sitting!\n\r", ch);
                break;
            case POSITION_FIGHTING:
                send_to_char("Impossible! You can't concentrate enough!.\n\r",
                             ch);
                break;
            default:
                send_to_char("It seems like you're in pretty bad shape!\n\r",
                             ch);
                break;
            }
        } else {
            if (!IS_IMMORTAL(ch) && 
                spell_info[index].min_level_magic > 
                    GET_LEVEL(ch, MAGE_LEVEL_IND) && 
                spell_info[index].min_level_sorcerer >
                    GET_LEVEL(ch, SORCERER_LEVEL_IND) && 
                spell_info[index].min_level_cleric >
                    GET_LEVEL(ch, CLERIC_LEVEL_IND) && 
                spell_info[index].min_level_paladin >
                    GET_LEVEL(ch, PALADIN_LEVEL_IND) && 
                spell_info[index].min_level_ranger >
                    GET_LEVEL(ch, RANGER_LEVEL_IND) && 
                spell_info[index].min_level_psi >
                    GET_LEVEL(ch, PSI_LEVEL_IND) && 
                spell_info[index].min_level_druid >
                    GET_LEVEL(ch, DRUID_LEVEL_IND) && 
                spell_info[index].min_level_necromancer >
                    GET_LEVEL(ch, NECROMANCER_LEVEL_IND)) {

                send_to_char("Sorry, you can't do that.\n\r", ch);
                return;
            }
            argument += qend + 1;       
            /* 
             * Point to the last ' 
             */
            for (; isspace(*argument); argument++) {
                /* 
                 * Empty loop 
                 */
            }

            /*
             **************** Locate targets **************** */
            target_ok = FALSE;
            tar_char = 0;
            tar_obj = 0;

            if (cmd != 600 && IS_SET(spell_info[index].targets, TAR_VIOLENT) && 
                check_peaceful(ch, 
                               "This seems to be an dead magic zone!\n\r")) {
                return;
            }

            if (cmd == 600 && IS_SET(spell_info[index].targets, TAR_VIOLENT) && 
                check_peaceful(ch, "Ah, no battle songs in here, matey!\n\r")) {
                return;
            }

            if (IS_IMMORTAL(ch) && IS_PC(ch) && GetMaxLevel(ch) < 59) {
                sprintf(buf, "%s cast %s", GET_NAME(ch), ori_argument);
                Log(buf);
            }

            if (!IS_SET(spell_info[index].targets, TAR_IGNORE)) {
                argument = one_argument(argument, name);

                if (str_cmp(name, "self") == 0) {
                    sprintf(name, "%s", GET_NAME(ch));
                }

                if (*name) {
                    /*
                     * room char spells 
                     */
                    if (IS_SET(spell_info[index].targets, TAR_CHAR_ROOM)) {
                        if ((tar_char = get_char_room_vis(ch, name)) || 
                            !str_cmp(GET_NAME(ch), name)) {
                            if (!str_cmp(GET_NAME(ch), name)) {
                                tar_char = ch;
                            }

                            if (tar_char == ch || 
                                tar_char == ch->specials.fighting || 
                                tar_char->attackers < 6 || 
                                tar_char->specials.fighting == ch) {
                                target_ok = TRUE;
                            } else if (cmd != 600) {
                                send_to_char("Too much fighting, you can't get"
                                             " a clear shot.\n\r", ch);
                                target_ok = FALSE;
                            }
                        } else {
                            target_ok = FALSE;
                        }
                    }

                    /*
                     * world char spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_CHAR_WORLD) && 
                        (tar_char = get_char_vis(ch, name))) {
                        target_ok = TRUE;
                    }

                    /*
                     * inv obj spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_OBJ_INV) &&
                        (tar_obj = 
                             get_obj_in_list_vis(ch, name, ch->carrying))) {
                        target_ok = TRUE;
                    }

                    /*
                     * room obj spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_OBJ_ROOM) &&
                        (tar_obj = get_obj_in_list_vis(ch, name,
                                        real_roomp(ch->in_room)->contents))) {
                        target_ok = TRUE;
                    }

                    /*
                     * world obj spells, locate object & transport via
                     * plant 
                     */
                    if (!target_ok && IS_SET(spell_info[index].targets,
                                             TAR_OBJ_WORLD)) {
                        target_ok = TRUE;
                        sprintf(argument, "%s", name);
                    }

                    /*
                     * eq obj spells 
                     */
                    if (!target_ok && IS_SET(spell_info[index].targets,
                                             TAR_OBJ_EQUIP)) {
                        for (i = 0; i < MAX_WEAR && !target_ok; i++) {
                            if (ch->equipment[i] && 
                                !str_cmp(name, ch->equipment[i]->name)) {
                                tar_obj = ch->equipment[i];
                                target_ok = TRUE;
                            }
                        }
                    }

                    /*
                     * self only spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_SELF_ONLY) &&
                        !str_cmp(GET_NAME(ch), name)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    /*
                     * group spells 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_GROUP) &&
                        (tar_char = get_char_vis(ch, name)) && 
                        IS_AFFECTED(tar_char, AFF_GROUP) && 
                        in_group(ch, tar_char)) {

                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    /*
                     * name spells (?) 
                     */
                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_NAME)) {
                        tar_obj = (void *) name;
                        target_ok = TRUE;
                    }

                    if (tar_char && IS_NPC(tar_char) &&
                        IS_SET(tar_char->specials.act, ACT_IMMORTAL)) {
                        send_to_char("You can't cast magic on that!", ch);
                        return;
                    }
                } else {
                    /* 
                     * No argument 
                     */
                    if (IS_SET(spell_info[index].targets, TAR_GROUP) &&
                        IS_AFFECTED(ch, AFF_GROUP)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    if (IS_SET(spell_info[index].targets, TAR_FIGHT_SELF) &&
                        ch->specials.fighting) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }

                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_FIGHT_VICT) &&
                        ch->specials.fighting) {
                        tar_char = ch->specials.fighting;
                        target_ok = TRUE;
                    }

                    if (!target_ok && 
                        IS_SET(spell_info[index].targets, TAR_SELF_ONLY)) {
                        tar_char = ch;
                        target_ok = TRUE;
                    }
                }
            } else {
                /* 
                 * No target, is a good target 
                 */
                target_ok = TRUE;
            }

            if (!target_ok) {
                if (*name) {
                    if (IS_SET(spell_info[index].targets, TAR_CHAR_WORLD)) {
                        send_to_char("Nobody playing by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, 
                                      TAR_CHAR_ROOM)) {
                        send_to_char("Nobody here by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, TAR_OBJ_INV)) {
                        send_to_char("You are not carrying anything like "
                                     "that.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, 
                                      TAR_OBJ_ROOM)) {
                        send_to_char("Nothing here by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, 
                                      TAR_OBJ_WORLD)) {
                        send_to_char("Nothing at all by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, 
                                      TAR_OBJ_EQUIP)) {
                        send_to_char("You are not wearing anything like "
                                     "that.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, 
                                      TAR_OBJ_WORLD)) { 
                        send_to_char("Nothing at all by that name.\n\r", ch);
                    } else if (IS_SET(spell_info[index].targets, TAR_GROUP)) {
                        send_to_char("You can only cast this spell when "
                                     "grouped.\n\r", ch);
                    }
                } else {
                    /* 
                     * No argument 
                     */
                    if (IS_SET(spell_info[index].targets, TAR_GROUP)) {
                        send_to_char("You can only cast this spell when "
                                     "grouped.\n\r", ch);
                    } else if (spell_info[index].targets < TAR_OBJ_INV) {
                        if (cmd != 600) {
                            send_to_char("Who should the spell be cast "
                                         "upon?\n\r", ch);
                        } else {
                            send_to_char("Who should the song be aimed "
                                         "at?\n\r", ch);
                        }
                    } else {
                        if (cmd != 600) {
                            send_to_char("What should the spell be cast "
                                         "upon?\n\r", ch);
                        } else {
                            send_to_char("What should the song be aimed "
                                         "at?\n\r", ch);
                        }
                    }
                }
                return;
            } 
            
            /* 
             * TARGET IS OK 
             */
            if (tar_char == ch && 
                IS_SET(spell_info[index].targets, TAR_SELF_NONO)) {
                if (cmd != 600) {
                    send_to_char("You can not cast this spell upon "
                                 "yourself.\n\r", ch);
                } else {
                    send_to_char("You can not aim this song at yourself.\n\r",
                                 ch);
                }
                return;
            } else if (tar_char != ch && 
                       IS_SET(spell_info[index].targets, TAR_SELF_ONLY)) {
                if (cmd != 600) {
                    send_to_char("You can only cast this spell upon "
                                 "yourself.\n\r", ch);
                } else {
                    send_to_char("You can only aim this song at yourself.\n\r",
                                 ch);
                }
                return;
            } else if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == tar_char) {
                send_to_char("You are afraid that it could harm your "
                             "master.\n\r", ch);
                return;
            } else if (!IS_SET(spell_info[index].targets, TAR_IGNORE) &&
                       A_NOASSIST(ch, tar_char)) {
                act("$N is engaged with someone else, no can do.",
                    FALSE, ch, 0, tar_char, TO_CHAR);
                return;
            }

            if (cmd == 283 && !MEMORIZED(ch, spl)) {
                /* 
                 * recall 
                 */
                send_to_char("You don't have that spell memorized!\n\r", ch);
                return;
            } 
            
            if (GetMaxLevel(ch) < IMMORTAL &&
                (GET_MANA(ch) < USE_MANA(ch, spl) || GET_MANA(ch) <= 0)) {
                send_to_char("You can't summon enough energy!\n\r", ch);
                return;
            }

            if (spl != SPELL_VENTRILOQUATE && cmd != 370) {
                /* 
                 * mind 
                 */
                say_spell(ch, spl);
            }

            WAIT_STATE(ch, spell_info[index].beats);

            if (!spell_info[index].spell_pointer && spl > 0) {
                send_to_char("Sorry, this magic has not yet been implemented "
                             ":(\n\r", ch);
            } else {
                max = ch->specials.spellfail;
                /* 
                 * 0 - 240 
                 */
                max += GET_COND(ch, DRUNK) * 10;

                switch (BestMagicClass(ch)) {
                case MAGE_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case SORCERER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10; 
                    }
                    break;
                case CLERIC_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_CLERIC)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case DRUID_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_DRUID)) {
                        /* 
                         * 20% harder to cast spells 
                         */ 
                        max += 10;
                    }
                    break;
                case PALADIN_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_PALADIN)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case PSI_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_PSI)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case RANGER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_RANGER)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                case NECROMANCER_LEVEL_IND:
                    if (EqWBits(ch, ITEM_ANTI_NECROMANCER)) {
                        max += 10;
                    }
                    break;

                default:
                    if (EqWBits(ch, ITEM_ANTI_MAGE)) {
                        /* 
                         * 20% harder to cast spells 
                         */
                        max += 10;
                    }
                    break;
                }

                max += int_sf_modifier[(int)GET_INT(ch)].learn;

                if (ch->attackers > 0) {
                    max += spell_info[index].spellfail;
                } else if (ch->specials.fighting) {
                    max += spell_info[index].spellfail / 2;
                }
                if (number(1, max) > ch->skills[spl].learned && 
                    !IsSpecialized(ch->skills[spl].special) && cmd != 283) {
                    send_to_char("You lost your concentration!\n\r", ch);
                    cost = (int) USE_MANA(ch, (int) spl);
                    GET_MANA(ch) -= (cost >> 1);
                    LearnFromMistake(ch, spl, 0, 95);
                    return;
                }

                if (!IS_IMMORTAL(ch)) {
                    if (tar_char == ch &&
                        affected_by_spell(tar_char, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against your own anti-magic "
                            "shell!", FALSE, ch, 0, 0, TO_CHAR);
                        act("$n wastes a spell on $s own anti-magic shell!", 
                            FALSE, ch, 0, 0, TO_ROOM);
                        return;
                    }

                    if (tar_char && 
                        affected_by_spell(tar_char, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against $N's anti-magic shell!",
                            FALSE, ch, 0, tar_char, TO_CHAR);
                        act("$n wastes a spell on $N's anti-magic shell!",
                            FALSE, ch, 0, tar_char, TO_NOTVICT);
                        act("$n casts a spell and growls as it fizzles against"
                            " your anti-magic shell!", FALSE, ch, 0, tar_char,
                            TO_VICT);
                        return;
                    }
                    
                    if (tar_char && GET_POS(tar_char) == POSITION_DEAD) {
                        send_to_char("The magic fizzles against the dead "
                                     "body.\n", ch);
                        return;
                    }

                    if (affected_by_spell(ch, SPELL_ANTI_MAGIC_SHELL)) {
                        act("Your magic fizzles against your anti-magic shell!",
                            FALSE, ch, 0, 0, TO_CHAR);
                        act("$n tries to cast a spell within a anti-magic "
                            "shell, muhahaha!", FALSE, ch, 0, 0, TO_ROOM);
                        return;
                    }

                    if (check_nomagic(ch, "Your skill appears useless in this"
                                          " magic dead zone.\n\r",
                                      "$n's spell dissolves like so much wet "
                                      "toilet paper.\n\r")) {
                        return;
                    }
                }

                send_to_char("Ok.\n\r", ch);
                (*spell_info[index].spell_pointer)
                    (GET_LEVEL(ch, BestMagicClass(ch)), ch, argument, 
                     SPELL_TYPE_SPELL, tar_char, tar_obj);

                cost = USE_MANA(ch, spl);
                exp = NewExpCap(ch, cost * 50);

                if (cmd == 283) {
                    /* 
                     * recall 
                     */
                    FORGET(ch, spl);
                }

                GET_MANA(ch) -= cost;
                if( exp ) {
                    sprintf(buf, "$c000BYou receive $c000W%d $c000Bexperience "
                                 "from your expert casting abilities."
                                 "$c000w\n\r", exp);
                    send_to_char(buf, ch);
                    gain_exp(ch, exp);
                }
            }
        }
        return;
    }

    switch (number(1, 5)) {
    case 1:
        send_to_char("Bylle Grylle Grop Gryf???\n\r", ch);
        break;
    case 2:
        send_to_char("Olle Bolle Snop Snyf?\n\r", ch);
        break;
    case 3:
        send_to_char("Olle Grylle Bolle Bylle?!?\n\r", ch);
        break;
    case 4:
        send_to_char("Gryffe Olle Gnyffe Snop???\n\r", ch);
        break;
    default:
        send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r", ch);
        break;
    }
}

void assign_spell_pointers(void)
{
    int             i;

    for( i = 0; i < MAX_SPL_LIST; i++ ) {
        spell_index[i] = -1;
    }

    for( i = 0; i < spell_info_count; i++ ) {
        spell_index[spell_info[i].nr] = i;
    }
}


void SpellWearOffSoon(int s, struct char_data *ch)
{
    if (s > MAX_SKILLS + 10) {
        return;
    }
    if (spell_wear_off_soon_msg[s] && *spell_wear_off_soon_msg[s]) {
        send_to_char(spell_wear_off_soon_msg[s], ch);
        send_to_char("\n\r", ch);
    }

    if (spell_wear_off_soon_room_msg[s] && *spell_wear_off_soon_room_msg[s]) {
        act(spell_wear_off_soon_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
    }
}

void SpellWearOff(int s, struct char_data *ch)
{
    struct affected_type af;

    if (s > MAX_SKILLS + 10) {
        return;
    }
    if (spell_wear_off_msg[s] && *spell_wear_off_msg[s]) {
        send_to_char(spell_wear_off_msg[s], ch);
        send_to_char("\n\r", ch);
    }

    if (spell_wear_off_room_msg[s] && *spell_wear_off_room_msg[s]) {
        act(spell_wear_off_room_msg[s], FALSE, ch, 0, 0, TO_ROOM);
    }

    if (s == SPELL_CHARM_PERSON || s == SPELL_CHARM_MONSTER) {
        check_decharm(ch);
    }

    if (s == SPELL_FLY) {
        check_falling(ch);
    }
    if (s == SPELL_WATER_BREATH) {
        check_drowning(ch);
    }

    if (s == COND_WINGS_FLY) {
        af.type = COND_WINGS_TIRED;
        af.location = APPLY_BV2;
        af.duration = 25 - GET_CON(ch);
        af.modifier = 0;
        af.bitvector = AFF2_WINGSTIRED;
        affect_to_char(ch, &af);
        check_falling(ch);
    }
}

void check_decharm(struct char_data *ch)
{
    struct char_data *m;

    if (!ch->master) {
        return;
    }
    m = ch->master;
    stop_follower(ch);
    REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
    AddFeared(ch, m);
    do_flee(ch, "", 0);
}

int check_falling(struct char_data *ch)
{
    struct room_data *rp,
                   *targ;
    int             done,
                    count,
                    saved;
    char            buf[256];

    if (IS_IMMORTAL(ch)) {
        /* 
         * so if this guy is using redit the mud does not crash when he 
         * falls... 
         */
        return (FALSE);         
    }

    if (GET_POS(ch) <= POSITION_DEAD) {
        /* 
         * ch will be purged, check corpse instead, bug fix msw 
         */
        return (FALSE);
    }

    if (IS_AFFECTED(ch, AFF_FLYING)) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);
    if (!rp) {
        return (FALSE);
    }
    if (rp->sector_type != SECT_AIR) {
        return (FALSE);
    }
    if (ch->skills && number(1, 101) < ch->skills[SKILL_SAFE_FALL].learned) {
        act("You manage to slow your fall, enough to stay alive..",
            TRUE, ch, 0, 0, TO_CHAR);
        saved = TRUE;
    } else {
        act("The world spins, and you sky-dive out of control",
            TRUE, ch, 0, 0, TO_CHAR);
        saved = FALSE;
    }

    done = FALSE;
    count = 0;

    while (!done && count < 100) {
        /*
         * check for an exit down. if there is one, go through it. 
         */
        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        } else {
            /*
             * pretend that this is the smash room. 
             */
            if (IS_SET(rp->room_flags, ARENA_ROOM)) {
                /* 
                 * Volcano part of the Arena
                 */
                send_to_char("You are enveloped in a sea of burning flames and"
                             " molten rock.\n\r", ch);
                send_to_char("Soon, you are nothing but ashes.\n\r", ch);
                sprintf(buf, "%s disappears beneath a sea of flame with a loud"
                             " hiss and lot of smoke.\n\r", GET_NAME(ch));
                send_to_room_except(buf, ch->in_room, ch);

                sprintf(buf, "%s killed by burning in ARENA!\n\r",
                        GET_NAME(ch));
                send_to_all(buf);
                raw_kill_arena(ch);
                return (TRUE);
            }

            if (count > 1) {
                send_to_char("You are smashed into tiny pieces.\n\r", ch);
                act("$n smashes against the ground at high speed",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("You are drenched with blood and gore",
                    FALSE, ch, 0, 0, TO_ROOM);

                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                if (!IS_IMMORTAL(ch)) {
                    GET_HIT(ch) = 0;
                    sprintf(buf, "%s has fallen to death", GET_NAME(ch));
                    Log(buf);

                    if (!ch->desc) {
                        GET_GOLD(ch) = 0;
                    }
                    die(ch, '\0');      /* change to the smashed type */
                }
                return (TRUE);
            }
            
            send_to_char("You land with a resounding THUMP!\n\r", ch);
            GET_HIT(ch) = 0;
            GET_POS(ch) = POSITION_STUNNED;
            act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

            /*
             * should damage all their stuff 
             */
            DamageAllStuff(ch, BLOW_DAMAGE);

            return (TRUE);
        }

        act("$n plunges towards oblivion", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You plunge from the sky\n\r", ch);
        char_from_room(ch);
        char_to_room(ch, rp->dir_option[DOWN]->to_room);
        act("$n falls from the sky", FALSE, ch, 0, 0, TO_ROOM);
        count++;

        do_look(ch, "", 0);

        if (IS_SET(targ->room_flags, DEATH) && !IS_IMMORTAL(ch)) {
            NailThisSucker(ch);
            return (TRUE);
        }

        if (targ->sector_type != SECT_AIR) {
            /*
             * do damage, or kill 
             */
            if (count == 1) {
                send_to_char("You land with a resounding THUMP!\n\r", ch);
                GET_HIT(ch) = 0;
                GET_POS(ch) = POSITION_STUNNED;
                act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0,
                    TO_ROOM);
                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                return (TRUE);
            }
            
            if (!saved) {
                send_to_char("You are smashed into tiny pieces.\n\r", ch);
                if (targ->sector_type >= SECT_WATER_SWIM) {
                    act("$n is smashed to a pulp by $s impact with the water",
                        FALSE, ch, 0, 0, TO_ROOM);
                } else {
                    act("$n is smashed to a bloody pulp by $s impact with the"
                        " ground", FALSE, ch, 0, 0, TO_ROOM);
                }
                act("You are drenched with blood and gore", FALSE, ch, 0,
                    0, TO_ROOM);

                /*
                 * should damage all their stuff 
                 */
                DamageAllStuff(ch, BLOW_DAMAGE);

                if (!IS_IMMORTAL(ch)) {
                    GET_HIT(ch) = 0;
                    sprintf(buf, "%s has fallen to death", GET_NAME(ch));
                    Log(buf);
                    
                    if (!ch->desc) {
                        GET_GOLD(ch) = 0;
                    }    
                    die(ch, '\0');
                }
                return (TRUE);
            }
            
            send_to_char("You land with a resounding THUMP!\n\r", ch);
            GET_HIT(ch) = 0;
            GET_POS(ch) = POSITION_STUNNED;
            act("$n lands with a resounding THUMP!", FALSE, ch, 0, 0, TO_ROOM);

            /*
             * should damage all their stuff 
             */
            DamageAllStuff(ch, BLOW_DAMAGE);
            return (TRUE);
        }
        
        /*
         * time to try the next room 
         */
        rp = targ;
        targ = 0;
    }

    if (count >= 100) {
        Log("Someone messed up an air room.");
        char_from_room(ch);
        char_to_room(ch, 2);
        do_look(ch, "", 0);
    }
    return (FALSE);
}

void check_drowning(struct char_data *ch)
{
    struct room_data *rp;
    char            buf[256];

    if (IS_AFFECTED(ch, AFF_WATERBREATH)) {
        return;
    }
    rp = real_roomp(ch->in_room);

    if (!rp) {
        return;
    }
    if (rp->sector_type == SECT_UNDERWATER) {
        send_to_char("PANIC!  You're drowning!!!!!!", ch);
        GET_HIT(ch) -= number(1, 30);
        GET_MOVE(ch) -= number(10, 50);
        update_pos(ch);

        if (GET_HIT(ch) < -10) {
            sprintf(buf, "%s killed by drowning", GET_NAME(ch));
            Log(buf);

            if (!ch->desc) {
                GET_GOLD(ch) = 0;
            }
            die(ch, '\0');
        }
    }
}

void check_falling_obj(struct obj_data *obj, int room)
{
    struct room_data *rp,
                   *targ = NULL;
    int             done,
                    count;

    if (obj->in_room != room) {
        Log("unusual object information in check_falling_obj");
        return;
    }

    rp = real_roomp(room);
    if (!rp || rp->sector_type != SECT_AIR) {
        return;
    }
    done = FALSE;
    count = 0;

    while (!done && count < 100) {
        if (rp->dir_option[DOWN] && rp->dir_option[DOWN]->to_room > -1) {
            targ = real_roomp(rp->dir_option[DOWN]->to_room);
        } else if (count > 1) {
            if (rp->people) {
                /*
                 * pretend that this is the smash room. 
                 */
                act("$p smashes against the ground at high speed",
                    FALSE, rp->people, obj, 0, TO_ROOM);
                act("$p smashes against the ground at high speed",
                    FALSE, rp->people, obj, 0, TO_CHAR);
            }
            return;
        } else if (rp->people) {
            act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0, 
                TO_ROOM);
            act("$p lands with a loud THUMP!", FALSE, rp->people, obj, 0,
                TO_CHAR);
            return;
        }

        if (rp->people) {
            /* 
             * have to reference a person 
             */
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_ROOM);
            act("$p falls out of sight", FALSE, rp->people, obj, 0, TO_CHAR);
        }

        obj_from_room(obj);
        obj_to_room(obj, rp->dir_option[DOWN]->to_room);

        if (targ->people) {
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_ROOM);
            act("$p falls from the sky", FALSE, targ->people, obj, 0, TO_CHAR);
        }

        count++;

        if (targ->sector_type != SECT_AIR) {
            if (count == 1) {
                if (targ->people) {
                    act("$p lands with a loud THUMP!", FALSE, targ->people,
                        obj, 0, TO_ROOM);
                    act("$p lands with a loud THUMP!", FALSE, targ->people,
                        obj, 0, TO_CHAR);
                }
                return;
            } 

            if (targ->people) {
                if (targ->sector_type >= SECT_WATER_SWIM) {
                    act("$p smashes against the water at high speed",
                        FALSE, targ->people, obj, 0, TO_ROOM);
                    act("$p smashes against the water at high speed",
                        FALSE, targ->people, obj, 0, TO_CHAR);
                } else {
                    act("$p smashes against the ground at high speed",
                        FALSE, targ->people, obj, 0, TO_ROOM);
                    act("$p smashes against the ground at high speed",
                        FALSE, targ->people, obj, 0, TO_CHAR);
                }
            }
            return;
        } else {
            /*
             * time to try the next room 
             */
            rp = targ;
            targ = 0;
        }
    }

    if (count >= 100) {
        Log("Someone screwed up an air room.");
        obj_from_room(obj);
        obj_to_room(obj, 4);
        return;
    }
}

int check_nature(struct char_data *i)
{

    if (check_falling(i)) {
        return (TRUE);
    }
    check_drowning(i);
    return( FALSE );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
