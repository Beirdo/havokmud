#ifndef _race_h
#define _race_h

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

/*
 * a few pc races 
 */
#define RACE_HALF_ELF   74
#define RACE_HALF_OGRE    75
#define RACE_HALF_ORC     76
#define RACE_HALF_GIANT   77
/*
 * end pc 
 */

#define RACE_LIZARDMAN 78

#define RACE_DARK_DWARF 79
#define RACE_DEEP_GNOME 80

#define RACE_GNOLL      81

#define RACE_GOLD_ELF   82
#define RACE_WILD_ELF   83
#define RACE_SEA_ELF    84
#define RACE_FOREST_GNOME 85
#define RACE_AVARIEL      86

/*
 * New Races by Greg Hovey 
 */
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

#define MAX_RACE      131

struct race_type {
    int             race;
    char           *racename;

    int             start;
    int             young;
    int             mature;
    int             middle;
    int             old;
    int             ancient;
    int             venerable;
    int             size;

};

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
