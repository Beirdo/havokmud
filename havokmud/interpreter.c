#include "config.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"

#define STATE(d) ((d)->connected)

#define MAX_CMD_LIST 497
#define STAT_SWORD(x) ((x >= 1 && x <= 3) ? "-]>>>" : ((x >= 4 && x<= 6) ? \
                        "-]>>>>" : ((x >= 7 && x<= 9) ? "-]>>>>>" : \
                        ((x >= 10 && x<= 12) ? "-]>>>>>>" : \
                         ((x >= 13 && x<= 15) ? "-]>>>>>>>" : \
                          ((x >= 16 && x<18) ? "-]>>>>>>>>" : ((x >= 18) ? \
                           "-]>>>>>>>>>" : "ERROR! PLS REPORT!")))))))
#define ASSHOLE_FNAME "asshole.list"

char *get_argument_common(char *line_in, char **arg_out, int do_fill,
                          char delim);

char *newbie_note[] = {
    "\n\rWelcome to Havok, here are a few instructions to help you get\n\r"
    "along better at Havok.\n\r\n\r",
    " 1) The immortals of Havok have put alot of time into making Havok as\n\r"
    "    fun as possible, as well as ensuring that Havok is a stable and\n\r"
    "    reliable place to play.  They deserve your respect and courtesy\n\r"
    "    and by playing Havok you agree to show them both.\n\r",
    " 2) We try to get people to role play, but we do not force you. If\n\r" 
    "    you enjoy role playing please do so, if you do not please do not\n\r" 
    "    interfere with those that do.\n\r",
    " 3) Some commands for newbies are HELP, NEWS, and COMMANDS. Use help\n\r" 
    "    for solving most of your questions. If you are still confused feel\n\r"
    "    free to ask.\n\r",
    " 3) PLEASE do not curse over public channels including GOSSIP, SHOUT\n\r"
    "    AUCTION, etc.  Punishment for doing so begins with removing your\n\r"
    "    ability to use public channels.\n\r",
    " 4) Not all public channels work world wide. YELL and TELL for example\n\r"
    "    only work in the zone you're in. There are spells and skills to\n\r"
    "    communicate world wide if you need them. SHOUT and OOC are world\n\r"
    "    wide and cost you MANA and VITIALITY points.\n\r",
    " 5) Please do not use 'funny' or 'wacky' names. We try to encourage\n\r "
    "    role-playing and if you use those type of names it does not help\n\r" 
    "    matters. Do not use a name that is used in the Forgotten Realms\n\r"
    "    setting (such as Elminster, Drizzit etc..).  There are NPC's that\n\r"
    "    use these names and it will cause confusion. If you do not abide\n\r"
    "    by these rules an immortal might ask you to change your name.\n\r",
    " 6) Remember that we try to add a bit of realism and things such as\n\r"
    "    starving to death or dying of thirst CAN happen.\n\r",
    "\n\r\n\r",
    NULL
};

char *racehelp[] = {
    "\n\r"
    "Dwarves:   Shorter. Less Movement. Infravision. Higher con, Lower dex,\n\r"
    "           faster hit point gain.  less damage from poison, racial "
        "hatreds\n\r"
    "Moon Elf:  More Movement. Faster Mana gain, Higher dex, Lower con\n\r"
    "           Detect secret doors, racial hatreds, Infra vision\n\r"
    "Gold Elf:  Same as Moon, except more intellengent and less wisdom.\n\r"
    "Wild Elf:  Same as Moon except stronger, less intellengent.\n\r"
    "Sea Elf:   Same as moon except healthier and stronger, less "
        "dexterious.\n\r",
    "Dark Elf:  Same as elves, but higher dex, infravision, class limits "
        "are\n\r"
    "           different. Minus to hit in lighted rooms.\n\r"
    "Humans:    Average...Unlimited in all classes only race that is.\n\r"
    "           Least amount of racial hatreds than all classes.\n\r"
    "           Most class selections in the multi-class ranges.\n\r"
    "           Humans can be Barbarians.\n\r",
    "Halflings: Very short.  Higher dex, Lower str.  Less move, faster hit \n\r"
    "           point gain, less damage from poison. faster move gain\n\r"
    "Rock Gnomes:    Short. Higher intel, Lower wis.. Less move, infravision, "
        "faster\n\r"
    "           mana gain\n\r\n\r"
    "Forest Gnomes: Same as Rock Gnome, except High Wis, Dex, Low Int, Str\n\r"
    "Half-Elves:Infravision, detect secret doors, good move and mana gain, "
        "large\n\r"
    "           multi-class selection. Only race that can multi-class\n\r"
    "           the Druid class.\n\r",
    "Half-Orcs: Infravision, high con, low charisma Good move point gain.\n\r"
    "Half-Ogres:Infravision, high strength and con, low dex and intel. Good "
        "move\n\r"
    "           and hit point gain. Large size and weight. Cleric, warrior "
        "classes\n\r"
    "           only.\n\r"
    "Half-Giants:Infravision, highest strength bonus, high con, low intel, "
        "wis and\n\r"
    "            dex. Good hit point and move gain. Very large. Warrior\n\r"
    "            and Barbarian class ONLY. Giants get a hit point boost\n\r"
    "            at level 1.\n\r"
    "\n\r\n\r",
    NULL
};


char *class_help[] = {
    "Cleric:       Good defense.  Healing spells\n\r"
    "Druid:        Real outdoors types.  Spells, not many items\n\r"
    "Fighter:      Big, strong and stupid.  Nuff said.\n\r"
    "Magic-users:  Weak, puny, smart and very powerful at high levels.\n\r"
    "Thieves:      Quick, agile, sneaky.  Nobody trusts them.\n\r"
    "Monks:        Masters of the martial arts.  They can only be single "
    "classed\n\r"
    "Barbarians:   Strong fighters, trackers and survivers. More move, "
    "faster\n\r"
    "              hit point regeneration. Limited magic use. Single class ",
    "only and\n\r"
    "              only Humans and Half-Giants can be Barbarians.\n\r"
    "Paladins:     Holy warriors, defenders of good. Good fighters, some "
    "cleric\n\r"
    "              abilities.\n\r"
    "Rangers:      Woodland hunters. These guys are one with the ways of "
    "the\n\r"
    "              the forest life. Some druid type skills and warrior.\n\r"
    "Psionists:    Mind benders, they use the power of the mind to do\n\r"
    "              unthought-of things\n\r"
    "\n\rREMEMBER single class characters have a better hit point ratio than "
    "multi's.\n\r"
    "\n\r\n\r",
    NULL
};

char *ru_sorcerer[] = {
    "\n\r"
    "You have chosen the magic user class, there are two types of mages here "
    "at\n\r"
    "Havok. The Standard Mage uses mana for his power, the Sorcerer uses "
    "his\n\r"
    "memorization for his powers. Sorcerers will sit and memorize a spell of "
    "their\n\r"
    "choice and then after doing so can 'recall' this spell. Sorcerers are "
    "said\n\r",
    "to be weaker at lower levels than most classes, but after they attain "
    "higher\n\r"
    "experince they can become one of the most powerful.\n\r"
    "\n\r\n\r"
    "Enter 'yes' if you want to be a Sorcerer type of mage, hit return if "
    "you\n\r"
    "do not want to be a Sorcerer (yes/no) :",
    NULL
};

struct command_def commandList[] = {
    { "north", do_move, 1, POSITION_STANDING, 0 },
    { "east", do_move, 2, POSITION_STANDING, 0 },
    { "south", do_move, 3, POSITION_STANDING, 0 },
    { "west", do_move, 4, POSITION_STANDING, 0 },
    { "up", do_move, 5, POSITION_STANDING, 0 },
    { "down", do_move, 6, POSITION_STANDING, 0 },
    { "enter", do_enter, 7, POSITION_STANDING, 0 },
    { "exits", do_exits, 8, POSITION_RESTING, 0 },
    { "kiss", do_action, 9, POSITION_RESTING, 1 },
    { "get", do_get, 10, POSITION_RESTING, 1 },

    { "drink", do_drink, 11, POSITION_RESTING, 1 },
    { "eat", do_eat, 12, POSITION_RESTING, 1 },
    { "wear", do_wear, 13, POSITION_RESTING, 0 },
    { "wield", do_wield, 14, POSITION_RESTING, 1 },
    { "look", do_look, 15, POSITION_RESTING, 0 },
    { "score", do_score, 16, POSITION_SLEEPING, 0 },
    { "say", do_new_say, 17, POSITION_RESTING, 0 },
    { "shout", do_shout, 18, POSITION_RESTING, 2 },
    { "tell", do_tell, 19, POSITION_RESTING, 0 },
    { "inventory", do_inventory, 20, POSITION_STUNNED, 0 },
    { "qui", do_qui, 21, POSITION_DEAD, 0 },
    { "bounce", do_action, 22, POSITION_STANDING, 0 },
    { "smile", do_action, 23, POSITION_RESTING, 0 },
    { "dance", do_action, 24, POSITION_STANDING, 0 },

    { "kill", do_kill, 25, POSITION_FIGHTING, 1 },
    { "id", do_id, 26, POSITION_STANDING, 1 },
    { "laugh", do_action, 27, POSITION_RESTING, 0 },
    { "giggle", do_action, 28, POSITION_RESTING, 0 },
    { "shake", do_action, 29, POSITION_RESTING, 0 },
    { "puke", do_action, 30, POSITION_RESTING, 0 },
    { "growl", do_action, 31, POSITION_RESTING, 0 },
    { "scream", do_action, 32, POSITION_RESTING, 0 },
    { "insult", do_insult, 33, POSITION_RESTING, 0 },

    { "comfort", do_action, 34, POSITION_RESTING, 0 },
    { "nod", do_action, 35, POSITION_RESTING, 0 },
    { "sigh", do_action, 36, POSITION_RESTING, 0 },
    { "sulk", do_action, 37, POSITION_RESTING, 0 },
    { "help", do_help, 38, POSITION_DEAD, 0 },
    { "who", do_who, 39, POSITION_DEAD, 0 },
    { "emote", do_emote, 40, POSITION_SLEEPING, 0 },
    { "echo", do_echo, 41, POSITION_SLEEPING, 1 },
    { "stand", do_stand, 42, POSITION_RESTING, 0 },

    { "sit", do_sit, 43, POSITION_RESTING, 0 },
    { "rest", do_rest, 44, POSITION_RESTING, 0 },
    { "sleep", do_sleep, 45, POSITION_SLEEPING, 0 },
    { "wake", do_wake, 46, POSITION_SLEEPING, 0 },
    { "force", do_force, 47, POSITION_SLEEPING, 57 },
    { "transfer", do_trans, 48, POSITION_SLEEPING, 54 },
    { "hug", do_action, 49, POSITION_RESTING, 0 },
    { "snuggle", do_action, 50, POSITION_RESTING, 0 },
    { "cuddle", do_action, 51, POSITION_RESTING, 0 },

    { "nuzzle", do_action, 52, POSITION_RESTING, 0 },
    { "cry", do_action, 53, POSITION_RESTING, 0 },
    { "news", do_news, 54, POSITION_SLEEPING, 0 },
    { "equipment", do_equipment, 55, POSITION_STUNNED, 0 },
    { "buy", do_not_here, 56, POSITION_STANDING, 0 },
    { "sell", do_not_here, 57, POSITION_STANDING, 0 },
    { "value", do_value, 58, POSITION_RESTING, 0 },
    { "list", do_not_here, 59, POSITION_STANDING, 0 },
    { "drop", do_drop, 60, POSITION_RESTING, 1 },

    { "goto", do_goto, 61, POSITION_SLEEPING, 0 },
    { "weather", do_weather, 62, POSITION_RESTING, 0 },
    { "read", do_read, 63, POSITION_RESTING, 0 },
    { "pour", do_pour, 64, POSITION_STANDING, 0 },
    { "grab", do_grab, 65, POSITION_RESTING, 0 },
    { "remove", do_remove, 66, POSITION_RESTING, 0 },
    { "put", do_put, 67, POSITION_RESTING, 0 },
    { "shutdow", do_shutdow, 68, POSITION_DEAD, SILLYLORD },
    { "save", do_save, 69, POSITION_STUNNED, 0 },

    { "hit", do_hit, 70, POSITION_FIGHTING, 1 },
    { "string", do_string, 71, POSITION_SLEEPING, 53 },
    { "give", do_give, 72, POSITION_RESTING, 1 },
    { "quit", do_quit, 73, POSITION_DEAD, 0 },
    { "stat", do_stat, 74, POSITION_DEAD, 53 },
    { "guard", do_guard, 75, POSITION_STANDING, 1 },
    { "time", do_time, 76, POSITION_DEAD, 0 },
    { "load", do_load, 77, POSITION_DEAD, SAINT },
    { "purge", do_purge, 78, POSITION_DEAD, 53 },

    { "shutdown", do_shutdown, 79, POSITION_DEAD, SILLYLORD },
    { "idea", do_idea, 80, POSITION_DEAD, 0 },
    { "typo", do_typo, 81, POSITION_DEAD, 0 },
    { "bug", do_bug, 82, POSITION_DEAD, 0 },
    { "whisper", do_whisper, 83, POSITION_RESTING, 0 },
    { "cast", do_cast, 84, POSITION_SITTING, 1 },
    { "at", do_at, 85, POSITION_DEAD, 51 },
    { "ask", do_ask, 86, POSITION_RESTING, 0 },
    { "order", do_order, 87, POSITION_RESTING, 1 },

    { "sip", do_sip, 88, POSITION_RESTING, 0 },
    { "taste", do_taste, 89, POSITION_RESTING, 0 },
    { "snoop", do_snoop, 90, POSITION_DEAD, 55 },
    { "follow", do_follow, 91, POSITION_RESTING, 0 },
    { "rent", do_not_here, 92, POSITION_STANDING, 1 },
    { "offer", do_not_here, 93, POSITION_STANDING, 1 },
    { "poke", do_action, 94, POSITION_RESTING, 0 },
    { "advance", do_advance, 95, POSITION_DEAD, IMPLEMENTOR },
    { "accuse", do_action, 96, POSITION_SITTING, 0 },

    { "grin", do_action, 97, POSITION_RESTING, 0 },
    { "bow", do_action, 98, POSITION_STANDING, 0 },
    { "open", do_open, 99, POSITION_SITTING, 0 },
    { "close", do_close, 100, POSITION_SITTING, 0 },
    { "lock", do_lock, 101, POSITION_SITTING, 0 },
    { "unlock", do_unlock, 102, POSITION_SITTING, 0 },
    { "leave", do_leave, 103, POSITION_STANDING, 0 },
    { "applaud", do_action, 104, POSITION_RESTING, 0 },
    { "blush", do_action, 105, POSITION_RESTING, 0 },

    { "burp", do_action, 106, POSITION_RESTING, 0 },
    { "chuckle", do_action, 107, POSITION_RESTING, 0 },
    { "clap", do_action, 108, POSITION_RESTING, 0 },
    { "cough", do_action, 109, POSITION_RESTING, 0 },
    { "curtsey", do_action, 110, POSITION_STANDING, 0 },
    { "fart", do_action, 111, POSITION_RESTING, 0 },
    { "flip", do_action, 112, POSITION_STANDING, 0 },
    { "fondle", do_action, 113, POSITION_RESTING, 0 },
    { "frown", do_action, 114, POSITION_RESTING, 0 },

    { "gasp", do_action, 115, POSITION_RESTING, 0 },
    { "glare", do_action, 116, POSITION_RESTING, 0 },
    { "groan", do_action, 117, POSITION_RESTING, 0 },
    { "grope", do_action, 118, POSITION_RESTING, 0 },
    { "hiccup", do_action, 119, POSITION_RESTING, 0 },
    { "lick", do_action, 120, POSITION_RESTING, 0 },
    { "love", do_action, 121, POSITION_RESTING, 0 },
    { "moan", do_action, 122, POSITION_RESTING, 0 },
    { "nibble", do_action, 123, POSITION_RESTING, 0 },

    { "pout", do_action, 124, POSITION_RESTING, 0 },
    { "purr", do_action, 125, POSITION_RESTING, 0 },
    { "ruffle", do_action, 126, POSITION_STANDING, 0 },
    { "shiver", do_action, 127, POSITION_RESTING, 0 },
    { "shrug", do_action, 128, POSITION_RESTING, 0 },
    { "sing", do_action, 129, POSITION_RESTING, 0 },
    { "slap", do_action, 130, POSITION_RESTING, 0 },
    { "smirk", do_action, 131, POSITION_RESTING, 0 },
#if 0
    { "snap",do_action,132,POSITION_RESTING,0 },
#endif

    { "sneeze", do_action, 133, POSITION_RESTING, 0 },
    { "snicker", do_action, 134, POSITION_RESTING, 0 },
    { "sniff", do_action, 135, POSITION_RESTING, 0 },
    { "snore", do_action, 136, POSITION_SLEEPING, 0 },
    { "spit", do_action, 137, POSITION_STANDING, 0 },
    { "squeeze", do_action, 138, POSITION_RESTING, 0 },
    { "stare", do_action, 139, POSITION_RESTING, 0 },
#if 0
    { "strut",do_action,140,POSITION_STANDING,0 },
#endif

    { "thank", do_action, 141, POSITION_RESTING, 0 },
    { "twiddle", do_action, 142, POSITION_RESTING, 0 },
    { "wave", do_action, 143, POSITION_RESTING, 0 },
    { "whistle", do_action, 144, POSITION_RESTING, 0 },
    { "wiggle", do_action, 145, POSITION_STANDING, 0 },
    { "wink", do_action, 146, POSITION_RESTING, 0 },

    { "yawn", do_action, 147, POSITION_RESTING, 0 },
    { "snowball", do_action, 148, POSITION_STANDING, 51 },
    { "write", do_write, 149, POSITION_STANDING, 1 },
    { "hold", do_grab, 150, POSITION_RESTING, 1 },
    { "flee", do_flee, 151, POSITION_SITTING, 1 },
    { "sneak", do_sneak, 152, POSITION_STANDING, 1 },
    { "hide", do_hide, 153, POSITION_RESTING, 1 },
    { "backstab", do_backstab, 154, POSITION_STANDING, 1 },
    { "pick", do_pick, 155, POSITION_STANDING, 1 },

    { "steal", do_steal, 156, POSITION_STANDING, 1 },
    { "bash", do_bash, 157, POSITION_FIGHTING, 1 },
    { "rescue", do_rescue, 158, POSITION_FIGHTING, 1 },
    { "kick", do_kick, 159, POSITION_FIGHTING, 1 },
    { "french", do_action, 160, POSITION_RESTING, 0 },
    { "comb", do_action, 161, POSITION_RESTING, 0 },
    { "massage", do_action, 162, POSITION_RESTING, 0 },
    { "tickle", do_action, 163, POSITION_RESTING, 0 },
    { "practice", do_practice, 164, POSITION_RESTING, 1 },

    { "study", do_practice, 164, POSITION_RESTING, 1 },
    { "pat", do_action, 165, POSITION_RESTING, 0 },
    { "examine", do_examine, 166, POSITION_SITTING, 0 },
    { "take", do_get, 167, POSITION_RESTING, 1 },
    { "info", do_info, 168, POSITION_SLEEPING, 0 },
    { "'", do_new_say, 169, POSITION_RESTING, 0 },

    { "practise", do_practice, 170, POSITION_RESTING, 1 },
    { "curse", do_action, 171, POSITION_RESTING, 0 },
    { "use", do_use, 172, POSITION_SITTING, 1 },
    { "where", do_where, 173, POSITION_DEAD, 1 },
    { "levels", do_levels, 174, POSITION_DEAD, 0 },
#if 0
    { "reroll",do_reroll,175,POSITION_DEAD,SILLYLORD },
#endif

    { "pray", do_pray, 176, POSITION_SITTING, 1 },
    { ",", do_emote, 177, POSITION_SLEEPING, 0 },
    { "beg", do_action, 178, POSITION_RESTING, 0 },
#if 0
    { "bleed",do_not_here,179,POSITION_RESTING,0 },
#endif
    { "cringe", do_action, 180, POSITION_RESTING, 0 },
    { "cackle", do_action, 181, POSITION_RESTING, 0 },
    { "fume", do_action, 182, POSITION_RESTING, 0 },

    { "grovel", do_action, 183, POSITION_RESTING, 0 },
    { "hop", do_action, 184, POSITION_RESTING, 0 },
    { "nudge", do_action, 185, POSITION_RESTING, 0 },
    { "peer", do_action, 186, POSITION_RESTING, 0 },
    { "point", do_action, 187, POSITION_RESTING, 0 },
    { "ponder", do_action, 188, POSITION_RESTING, 0 },
    { "punch", do_action, 189, POSITION_RESTING, 0 },
    { "snarl", do_action, 190, POSITION_RESTING, 0 },
    { "spank", do_action, 191, POSITION_RESTING, 0 },

    { "steam", do_action, 192, POSITION_RESTING, 0 },
    { "tackle", do_action, 193, POSITION_RESTING, 0 },
    { "taunt", do_action, 194, POSITION_RESTING, 0 },
    { "think", do_commune, 195, POSITION_RESTING, IMMORTAL },
    { "whine", do_action, 196, POSITION_RESTING, 0 },
    { "worship", do_action, 197, POSITION_RESTING, 0 },
    { "yodel", do_action, 198, POSITION_RESTING, 0 },
    { "brief", do_brief, 199, POSITION_DEAD, 0 },
    { "wizlist", do_wizlist, 200, POSITION_DEAD, 0 },

    { "consider", do_consider, 201, POSITION_RESTING, 0 },
    { "group", do_group, 202, POSITION_RESTING, 1 },
    { "restore", do_restore, 203, POSITION_DEAD, 52 },
    { "return", do_return, 204, POSITION_RESTING, 0 },
    { "switch", do_switch, 205, POSITION_DEAD, 55 },
    { "quaff", do_quaff, 206, POSITION_RESTING, 0 },
    { "recite", do_recite, 207, POSITION_STANDING, 0 },
    { "users", do_users, 208, POSITION_DEAD, IMMORTAL },
    { "pose", do_pose, 209, POSITION_STANDING, 0 },

    { "noshout", do_noshout, 210, POSITION_SLEEPING, 54 },
    { "wizhelp", do_wizhelp, 211, POSITION_SLEEPING, IMMORTAL },
    { "credits", do_credits, 212, POSITION_DEAD, 0 },
    { "compact", do_compact, 213, POSITION_DEAD, 0 },
    { ":", do_emote, 214, POSITION_SLEEPING, 0 },
    { "deafen", do_plr_noshout, 215, POSITION_SLEEPING, 1 },
    { "slay", do_kill, 216, POSITION_STANDING, 57 },
    { "wimpy", do_wimp, 217, POSITION_DEAD, 0 },
    { "junk", do_junk, 218, POSITION_RESTING, 1 },

    { "deposit", do_not_here, 219, POSITION_RESTING, 1 },
    { "withdraw", do_not_here, 220, POSITION_RESTING, 1 },
    { "balance", do_not_here, 221, POSITION_RESTING, 1 },
    { "nohassle", do_nohassle, 222, POSITION_DEAD, IMMORTAL },
    { "system", do_system, 223, POSITION_DEAD, 55 },
    { "pull", do_open_exit, 224, POSITION_STANDING, 1 },
    { "stealth", do_stealth, 225, POSITION_DEAD, IMMORTAL },
    { "edit", do_edit, 226, POSITION_DEAD, 53 },
    { "@", do_set, 227, POSITION_DEAD, IMPLEMENTOR },

    { "rsave", do_rsave, 228, POSITION_DEAD, 53 },
    { "rload", do_rload, 229, POSITION_DEAD, 53 },
    { "track", do_track, 230, POSITION_STANDING, 1 },
    { "siteban", do_wizlock, 231, POSITION_DEAD, 54 },
    { "highfive", do_highfive, 232, POSITION_DEAD, 0 },
    { "title", do_title, 233, POSITION_DEAD, 20 },
    { "whozone", do_who, 234, POSITION_DEAD, 0 },
    { "assist", do_assist, 235, POSITION_FIGHTING, 1 },
    { "attribute", do_attribute, 236, POSITION_DEAD, 1 },

    { "world", do_world, 237, POSITION_DEAD, 0 },
    { "allspells", do_spells, 238, POSITION_DEAD, 0 },
    { "breath", do_breath, 239, POSITION_FIGHTING, 1 },
    { "show", do_show, 240, POSITION_DEAD, 52 },
    { "debug", do_debug, 241, POSITION_DEAD, 60 },
    { "invisible", do_invis, 242, POSITION_DEAD, IMMORTAL },
    { "gain", do_gain, 243, POSITION_DEAD, 1 },
#if 0
    { "rrload",do_rrload,244,POSITION_DEAD,CREATOR },
#endif

    { "daydream", do_action, 244, POSITION_SLEEPING, 0 },
    { "disarm", do_disarm, 245, POSITION_FIGHTING, 1 },
    { "bonk", do_action, 246, POSITION_SITTING, 1 },
    { "chpwd", do_passwd, 247, POSITION_SITTING, IMPLEMENTOR },
    { "fill", do_not_here, 248, POSITION_SITTING, 0 },
    { "imptest", do_imptest, 249, POSITION_SITTING, 60 },
    { "shoot", do_fire, 250, POSITION_STANDING, 1 },
    { "silence", do_silence, 251, POSITION_STANDING, 54 },
#if 0
    { "teams",do_not_here,252,POSITION_STANDING, MAX_IMMORT },
    { "player",do_not_here,253,POSITION_STANDING, MAX_IMMORT },
#endif

    { "create", do_create, 254, POSITION_STANDING, 53 },
    { "bamfin", do_bamfin, 255, POSITION_STANDING, IMMORTAL },
    { "bamfout", do_bamfout, 256, POSITION_STANDING, IMMORTAL },
    { "vis", do_invis, 257, POSITION_RESTING, 0 },
    { "doorbash", do_doorbash, 258, POSITION_STANDING, 1 },

    { "mosh", do_action, 259, POSITION_FIGHTING, 1 },
    /*
     * alias commands
     */
    { "alias", do_alias, 260, POSITION_SLEEPING, 1 },
    { "1", do_alias, 261, POSITION_DEAD, 1 },
    { "2", do_alias, 262, POSITION_DEAD, 1 },
    { "3", do_alias, 263, POSITION_DEAD, 1 },
    { "4", do_alias, 264, POSITION_DEAD, 1 },

    { "5", do_alias, 265, POSITION_DEAD, 1 },
    { "6", do_alias, 266, POSITION_DEAD, 1 },
    { "7", do_alias, 267, POSITION_DEAD, 1 },
    { "8", do_alias, 268, POSITION_DEAD, 1 },
    { "9", do_alias, 269, POSITION_DEAD, 1 },
    { "0", do_alias, 270, POSITION_DEAD, 1 },
    { "swim", do_swim, 271, POSITION_STANDING, 1 },
    { "spy", do_spy, 272, POSITION_STANDING, 1 },
    { "springleap", do_springleap, 273, POSITION_RESTING, 1 },

    { "quivering palm", do_quivering_palm, 274, POSITION_FIGHTING, 30 },
    { "feign death", do_feign_death, 275, POSITION_FIGHTING, 1 },
    { "mount", do_mount, 276, POSITION_STANDING, 1 },
    { "dismount", do_mount, 277, POSITION_MOUNTED, 1 },
    { "ride", do_mount, 278, POSITION_STANDING, 1 },
    { "sign", do_sign, 279, POSITION_RESTING, 1 },
    /*
     * had to put this here BEFORE setsev so it would get this and not
     * setsev
     */
    { "set", do_set_flags, 280, POSITION_DEAD, 0 },
    { "first aid", do_first_aid, 281, POSITION_RESTING, 1 },
    { "log", do_set_log, 282, POSITION_DEAD, 60 },
    { "recall", do_cast, 283, POSITION_SITTING, 1 },
    { "reload", reboot_text, 284, POSITION_DEAD, MAX_IMMORT },
    { "event", do_event, 285, POSITION_DEAD, 60 },
    { "disguise", do_disguise, 286, POSITION_STANDING, 1 },
    { "climb", do_climb, 287, POSITION_STANDING, 1 },

    { "beep", do_beep, 288, POSITION_DEAD, 51 },
    { "bite", do_action, 289, POSITION_RESTING, 1 },
    { "redit", do_redit, 290, POSITION_SLEEPING, 53 },
    { "display", do_display, 291, POSITION_SLEEPING, 1 },
    { "resize", do_resize, 292, POSITION_SLEEPING, 1 },
    { "\"", do_commune, 293, POSITION_SLEEPING, IMMORTAL },
    { "#", do_cset, 294, POSITION_DEAD, 59 },
    { "auth", do_auth, 299, POSITION_SLEEPING, IMMORTAL },
    { "noyell", do_plr_nogossip, 301, POSITION_RESTING, 0 },

    { "gossip", do_yell, 302, POSITION_RESTING, 0 },
    { "noauction", do_plr_noauction, 303, POSITION_RESTING, 0 },
    { "auction", do_auction, 304, POSITION_RESTING, 0 },
    { "discon", do_disconnect, 305, POSITION_RESTING, IMMORTAL },
    { "freeze", do_freeze, 306, POSITION_SLEEPING, 55 },
    { "drain", do_drainlevel, 307, POSITION_SLEEPING, IMPLEMENTOR },
    { "oedit", do_oedit, 308, POSITION_DEAD, 53 },
    { "report", do_report, 309, POSITION_RESTING, 1 },
    { "interven", do_god_interven, 310, POSITION_DEAD, 58 },

    { "gtell", do_gtell, 311, POSITION_SLEEPING, 1 },
    { "raise", do_action, 312, POSITION_RESTING, 1 },
    { "tap", do_action, 313, POSITION_STANDING, 1 },
    { "liege", do_action, 314, POSITION_RESTING, 1 },
    { "sneer", do_action, 315, POSITION_RESTING, 1 },
    { "howl", do_action, 316, POSITION_RESTING, 1 },
    { "kneel", do_action, 317, POSITION_STANDING, 1 },
    { "finger", do_action, 318, POSITION_RESTING, 1 },
    { "pace", do_action, 319, POSITION_STANDING, 1 },

    { "tongue", do_action, 320, POSITION_RESTING, 1 },
    { "flex", do_action, 321, POSITION_STANDING, 1 },
    { "ack", do_action, 322, POSITION_RESTING, 1 },
    { "eh", do_action, 323, POSITION_RESTING, 1 },
    { "caress", do_action, 324, POSITION_RESTING, 1 },
    { "cheer", do_action, 325, POSITION_RESTING, 1 },
    { "jump", do_action, 326, POSITION_STANDING, 1 },
#if 0
    { "roll",do_action,327,POSITION_RESTING,1 },
#endif

    { "split", do_split, 328, POSITION_RESTING, 1 },
    { "berserk", do_berserk, 329, POSITION_FIGHTING, 1 },
    { "tan", do_tan, 330, POSITION_STANDING, 0 },
    { "memorize", do_memorize, 331, POSITION_RESTING, 1 },
    { "find", do_find, 332, POSITION_STANDING, 1 },
    { "bellow", do_bellow, 333, POSITION_FIGHTING, 1 },
    { "camouflage", do_hide, 334, POSITION_STANDING, 1 },
    { "carve", do_carve, 335, POSITION_STANDING, 1 },

    { "nuke", do_nuke, 336, POSITION_DEAD, IMPLEMENTOR },
    { "skills", do_show_skill, 337, POSITION_SLEEPING, 0 },
    { "doorway", do_doorway, 338, POSITION_STANDING, 1 },
    { "portal", do_psi_portal, 339, POSITION_STANDING, 1 },
    { "summon", do_mindsummon, 340, POSITION_STANDING, 1 },
    { "canibalize", do_canibalize, 341, POSITION_STANDING, 1 },
    { "flame", do_flame_shroud, 342, POSITION_STANDING, 1 },
    { "aura", do_aura_sight, 343, POSITION_RESTING, 1 },
    { "great", do_great_sight, 344, POSITION_RESTING, 1 },

    { "psionic invisibility", do_invisibililty, 345,
                POSITION_STANDING, 1 },
    { "blast", do_blast, 346, POSITION_FIGHTING, 1 },
#if 0
    { "psionic blast",do_blast,347,POSITION_FIGHTING,0 },
#endif
    { "medit", do_medit, 347, POSITION_DEAD, 53 },
    { "hypnotize", do_hypnosis, 348, POSITION_STANDING, 1 },
    { "scry", do_scry, 349, POSITION_RESTING, 1 },

    { "adrenalize", do_adrenalize, 350, POSITION_STANDING, 1 },
    { "brew", do_brew, 351, POSITION_STANDING, 1 },
    { "meditate", do_meditate, 352, POSITION_RESTING, 0 },
    { "forcerent", do_force_rent, 353, POSITION_DEAD, 58 },
    { "warcry", do_holy_warcry, 354, POSITION_FIGHTING, 0 },
    { "lay on hands", do_lay_on_hands, 355, POSITION_RESTING, 0 },
    { "blessing", do_blessing, 356, POSITION_STANDING, 0 },
    { "heroic", do_heroic_rescue, 357, POSITION_FIGHTING, 0 },
    { "scan", do_scan, 358, POSITION_STANDING, IMMORTAL },

    { "shield", do_psi_shield, 359, POSITION_STANDING, 0 },
    { "notell", do_plr_notell, 360, POSITION_DEAD, 0 },
    { "commands", do_command_list, 361, POSITION_DEAD, 0 },
    { "ghost", do_ghost, 362, POSITION_DEAD, 58 },
    { "speak", do_speak, 363, POSITION_DEAD, 0 },
    { "setsev", do_setsev, 364, POSITION_DEAD, 51 },
    { "esp", do_esp, 365, POSITION_STANDING, 0 },
    { "mail", do_not_here, 366, POSITION_STANDING, 0 },
    { "check", do_not_here, 367, POSITION_STANDING, 0 },

    { "receive", do_not_here, 368, POSITION_STANDING, 0 },
    { "telepathy", do_telepathy, 369, POSITION_RESTING, 0 },
    { "mind", do_cast, 370, POSITION_SITTING, 0 },
    { "twist", do_open_exit, 371, POSITION_STANDING, 0 },
    { "turn", do_open_exit, 372, POSITION_STANDING, 0 },
    { "lift", do_open_exit, 373, POSITION_STANDING, 0 },
    { "push", do_open_exit, 374, POSITION_STANDING, 0 },
    { "zload", do_zload, 375, POSITION_STANDING, 53 },
    { "zsave", do_zsave, 376, POSITION_STANDING, 53 },

    { "zclean", do_zclean, 377, POSITION_STANDING, 53 },
    { "wrebuild", do_WorldSave, 378, POSITION_STANDING, 60 },
    { "gwho", list_groups, 379, POSITION_DEAD, 0 },
    { "mforce", do_mforce, 380, POSITION_DEAD, 53 },
    { "clone", do_clone, 381, POSITION_DEAD, 53 },
    { "fire", do_fire, 382, POSITION_STANDING, 0 },
    { "throw", do_throw, 383, POSITION_SITTING, 0 },
    { "run", do_run, 384, POSITION_STANDING, 0 },
    { "notch", do_weapon_load, 385, POSITION_RESTING, 0 },

    { "spot", do_spot, 387, POSITION_STANDING, 0 },
    { "view", do_viewfile, 388, POSITION_DEAD, 51 },
    { "afk", do_set_afk, 389, POSITION_DEAD, 1 },


    /*
     * lots of Socials
     */
    { "adore", do_action, 400, POSITION_RESTING, 0 },
    { "agree", do_action, 401, POSITION_RESTING, 0 },
    { "bleed", do_action, 402, POSITION_RESTING, 0 },
    { "blink", do_action, 403, POSITION_RESTING, 0 },
    { "blow", do_action, 404, POSITION_RESTING, 0 },
    { "blame", do_action, 405, POSITION_RESTING, 0 },
    { "bark", do_action, 406, POSITION_RESTING, 0 },
    { "bhug", do_action, 407, POSITION_RESTING, 0 },
    { "bcheck", do_action, 408, POSITION_RESTING, 0 },
    { "boast", do_action, 409, POSITION_RESTING, 0 },

    { "chide", do_action, 410, POSITION_RESTING, 0 },
    { "compliment", do_action, 411, POSITION_RESTING, 0 },
    { "ceyes", do_action, 412, POSITION_RESTING, 0 },
    { "cears", do_action, 413, POSITION_RESTING, 0 },
    { "cross", do_action, 414, POSITION_RESTING, 0 },
    { "console", do_action, 415, POSITION_RESTING, 0 },
    { "calm", do_action, 416, POSITION_RESTING, 0 },
    { "cower", do_action, 417, POSITION_RESTING, 0 },
    { "confess", do_action, 418, POSITION_RESTING, 0 },

    { "drool", do_action, 419, POSITION_RESTING, 0 },
    { "grit", do_action, 420, POSITION_RESTING, 0 },
    { "greet", do_action, 421, POSITION_RESTING, 0 },
    { "gulp", do_action, 422, POSITION_RESTING, 0 },
    { "gloat", do_action, 423, POSITION_RESTING, 0 },
    { "gaze", do_action, 424, POSITION_RESTING, 0 },
    { "hum", do_action, 425, POSITION_RESTING, 0 },
    { "hkiss", do_action, 426, POSITION_RESTING, 0 },
    { "ignore", do_action, 427, POSITION_RESTING, 0 },

    { "interrupt", do_action, 428, POSITION_RESTING, 0 },
    { "knock", do_action, 429, POSITION_RESTING, 0 },
    { "listen", do_action, 430, POSITION_RESTING, 0 },
    { "muse", do_action, 431, POSITION_RESTING, 0 },
    { "pinch", do_action, 432, POSITION_RESTING, 0 },
    { "praise", do_action, 433, POSITION_RESTING, 0 },
    { "plot", do_action, 434, POSITION_RESTING, 0 },
    { "pie", do_action, 435, POSITION_RESTING, 0 },
    { "pleade", do_action, 436, POSITION_RESTING, 0 },

    { "pant", do_action, 437, POSITION_RESTING, 0 },
    { "rub", do_action, 438, POSITION_RESTING, 0 },
    { "roll", do_action, 439, POSITION_RESTING, 0 },
    { "recoil", do_action, 440, POSITION_RESTING, 0 },
    { "roar", do_action, 441, POSITION_RESTING, 0 },
    { "relax", do_action, 442, POSITION_RESTING, 0 },
    { "snap", do_action, 443, POSITION_RESTING, 0 },
    { "strut", do_action, 444, POSITION_RESTING, 0 },
    { "stroke", do_action, 445, POSITION_RESTING, 0 },

    { "stretch", do_action, 446, POSITION_RESTING, 0 },
    { "swave", do_action, 447, POSITION_RESTING, 0 },
    { "sob", do_action, 448, POSITION_RESTING, 0 },
    { "scratch", do_action, 449, POSITION_RESTING, 0 },
    { "squirm", do_action, 450, POSITION_RESTING, 0 },
    { "strangle", do_action, 451, POSITION_RESTING, 0 },
    { "scowl", do_action, 452, POSITION_RESTING, 0 },
    { "shudder", do_action, 453, POSITION_RESTING, 0 },
    { "strip", do_action, 454, POSITION_RESTING, 0 },

    { "scoff", do_action, 455, POSITION_RESTING, 0 },
    { "salute", do_action, 456, POSITION_RESTING, 0 },
    { "scold", do_action, 457, POSITION_RESTING, 0 },
    { "stagger", do_action, 458, POSITION_RESTING, 0 },
    { "toss", do_action, 459, POSITION_RESTING, 0 },
    { "twirl", do_action, 460, POSITION_RESTING, 0 },
    { "toast", do_action, 461, POSITION_RESTING, 0 },
    { "tug", do_action, 462, POSITION_RESTING, 0 },
    { "touch", do_action, 463, POSITION_RESTING, 0 },

    { "tremble", do_action, 464, POSITION_RESTING, 0 },
    { "twitch", do_action, 465, POSITION_RESTING, 0 },
    { "whimper", do_action, 466, POSITION_RESTING, 0 },
    { "whap", do_action, 467, POSITION_RESTING, 0 },
    { "wedge", do_action, 468, POSITION_RESTING, 0 },
    { "apologize", do_action, 469, POSITION_RESTING, 0 },

#if 0
    { "dmanage", do_dmanage, 480, POSITION_RESTING, IMPLEMENTOR },
    { "drestrict", do_drestrict, 481, POSITION_RESTING, 55 },
    { "dlink", do_dlink, 482, POSITION_RESTING, 55 },
    { "dunlink", do_dunlink, 483, POSITION_RESTING, 55 },
    { "dlist", do_dlist, 484, POSITION_RESTING, 51 },
    { "dwho", do_dwho, 485, POSITION_RESTING, 51 },
    { "dgossip", do_dgossip, 486, POSITION_RESTING, 51 },
    { "dtell", do_dtell, 487, POSITION_RESTING, 51 },
    { "dthink", do_dthink, 488, POSITION_RESTING, 51 },
#endif

    { "sending", do_sending, 489, POSITION_STANDING, 1 },
    { "messenger", do_sending, 490, POSITION_STANDING, 1 },
    { "promote", do_promote, 491, POSITION_RESTING, 1 },
    { "ooedit", do_ooedit, 492, POSITION_DEAD, 53 },
    { "whois", do_finger, 493, POSITION_DEAD, 1 },
    { "osave", do_osave, 494, POSITION_DEAD, 53 },
    { "msave", do_msave, 494, POSITION_DEAD, 53 },
    { "?", do_help, 495, POSITION_DEAD, 0 },
    { "home", do_home, 496, POSITION_DEAD, 51 },
    { "wizset", do_wizset, 497, POSITION_DEAD, 51 },

    { "ooc", do_ooc, 497, POSITION_RESTING, 2 },
    { "noooc", do_plr_noooc, 498, POSITION_SLEEPING, 2 },
    { "wiznoooc", do_wiznoooc, 499, POSITION_SLEEPING, 54 },

    /*
     * New social commands that Ugha added... - Manwe Windmaster 280697
     */
    { "homer", do_action, 500, POSITION_RESTING, 0 },
    { "grumble", do_action, 501, POSITION_RESTING, 0 },
    { "typoking", do_action, 502, POSITION_RESTING, 0 },
    { "evilgrin", do_action, 503, POSITION_RESTING, 0 },
    { "faint", do_action, 504, POSITION_RESTING, 0 },
    { "ckiss", do_action, 505, POSITION_STANDING, 0 },
    { "brb", do_action, 506, POSITION_RESTING, 0 },
    { "ready", do_action, 507, POSITION_STANDING, 0 },
    { "wolfwhistle", do_action, 508, POSITION_RESTING, 0 },
    { "wizreport", do_wizreport, 509, POSITION_RESTING, 55 },
    { "lgos", do_lgos, 510, POSITION_RESTING, 51 },
    { "groove", do_action, 511, POSITION_RESTING, 1 },

    { "wdisplay", do_action, 512, POSITION_STANDING, 1 },
    { "jam", do_action, 513, POSITION_STANDING, 1 },
    { "donate", do_donate, 514, POSITION_RESTING, 1 },
    { "reply", do_reply, 515, POSITION_RESTING, 1 },
    { "set_spy", do_set_spy, 516, POSITION_DEAD, 60 },
    { "reward", do_reward, 517, POSITION_RESTING, 51 },
    { "punish", do_punish, 518, POSITION_RESTING, 51 },
    { "spend", do_spend, 519, POSITION_RESTING, 51 },
    { "seepoints", do_see_points, 520, POSITION_RESTING, 51 },

    { "bugmail", bugmail, 521, POSITION_STANDING, 1 },
    { "setobjmax", do_setobjmax, 522, POSITION_RESTING, 53 },
    { "setobjspeed", do_setobjspeed, 523, POSITION_RESTING, 53 },
    { "wclean", do_wclean, 524, POSITION_RESTING, 58 },
    { "glance", do_glance, 525, POSITION_SITTING, 1 },
    { "arena", do_arena, 525, POSITION_SITTING, 1 },
    { "startarena", do_startarena, 525, POSITION_SITTING, 55 },
    { "whoarena", do_whoarena, 525, POSITION_RESTING, 0 },
    { "frolic", do_action, 526, POSITION_STANDING, 0 },

    { "land", do_land, 527, POSITION_STANDING, 0 },
    { "launch", do_launch, 528, POSITION_STANDING, 0 },
    { "disengage", do_disengage, 529, POSITION_FIGHTING, 1 },
    { "prompt", do_set_prompt, 530, POSITION_RESTING, 0 },
    { "talk", do_talk, 531, POSITION_RESTING, 1 },
    { "disagree", do_action, 532, POSITION_RESTING, 0 },
    { "beckon", do_action, 533, POSITION_RESTING, 0 },
    { "pounce", do_action, 534, POSITION_STANDING, 0 },
    { "amaze", do_action, 535, POSITION_RESTING, 0 },

    { "tank", do_action, 536, POSITION_STANDING, 0 },
    { "hshake", do_action, 537, POSITION_STANDING, 0 },
    { "backhand", do_action, 538, POSITION_STANDING, 0 },
    { "surrender", do_action, 539, POSITION_RESTING, 0 },
    { "collapse", do_action, 540, POSITION_STANDING, 0 },
    { "wince", do_action, 541, POSITION_RESTING, 0 },
    { "tag", do_action, 542, POSITION_RESTING, 0 },
    { "trip", do_action, 543, POSITION_RESTING, 0 },
    { "grunt", do_action, 544, POSITION_RESTING, 0 },

    { "imitate", do_action, 545, POSITION_RESTING, 0 },
    { "hickey", do_action, 546, POSITION_RESTING, 0 },
    { "torture", do_action, 547, POSITION_RESTING, 0 },
    { "addict", do_action, 548, POSITION_RESTING, 0 },
    { "adjust", do_action, 549, POSITION_RESTING, 0 },
    { "anti", do_action, 550, POSITION_RESTING, 0 },
    { "bbl", do_action, 551, POSITION_RESTING, 0 },
    { "beam", do_action, 552, POSITION_RESTING, 0 },
    { "challenge", do_action, 553, POSITION_RESTING, 0 },

    { "mutter", do_action, 554, POSITION_RESTING, 0 },
    { "beat", do_action, 555, POSITION_RESTING, 0 },
    { "moon", do_action, 556, POSITION_RESTING, 0 },
    { "dream", do_action, 557, POSITION_RESTING, 0 },
    { "shove", do_action, 558, POSITION_RESTING, 0 },
    { "behead", do_behead, 559, POSITION_STANDING, 0 },
    { "pinfo", do_flag_status, 560, POSITION_RESTING, 59 },

    /*
     * New command for editing gossiping scribe's messages -bwise
     */
    { "gosmsg", do_not_here, 562, POSITION_RESTING, 58 },
    { "yell", do_yell, 563, POSITION_RESTING, 0 },
    { "legsweep", do_leg_sweep, 564, POSITION_FIGHTING, 0 },
    { "charge", do_charge, 565, POSITION_STANDING, 0 },
    { "orebuild", do_orebuild, 566, POSITION_STANDING, 59 },
    { "draw", do_draw, 567, POSITION_FIGHTING, 0 },
    { "zconv", do_zconv, 568, POSITION_STANDING, 60 },
    { "bprompt", do_set_bprompt, 569, POSITION_RESTING, 0 },
    { "bid", do_bid, 570, POSITION_RESTING, 0 },
    { "resist", do_resistances, 571, POSITION_RESTING, 0 },

    { "style", do_style, 572, POSITION_STANDING, 0 },
    { "fight", do_style, 572, POSITION_STANDING, 0 },
    { "iwizlist", do_iwizlist, 573, POSITION_DEAD, 0 },
    { "flux", do_flux, 574, POSITION_STANDING, 53 },
    { "bs", do_backstab, 575, POSITION_STANDING, 1 },
    { "autoassist", do_auto, 576, POSITION_RESTING, 1 },
    { "autoloot", do_auto, 577, POSITION_RESTING, 1 },
    { "autogold", do_auto, 578, POSITION_RESTING, 1 },
    { "autosplit", do_auto, 579, POSITION_RESTING, 1 },
#if 0
    { "autosac",do_auto, 580, POSITION_RESTING,1 },
#endif
    { "autoexits", do_auto, 581, POSITION_RESTING, 1 },
    { "train", do_not_here, 582, POSITION_STANDING, 1 },
    { "mend", do_mend, 583, POSITION_STANDING, 1 },
    { "quest", do_set_quest, 585, POSITION_RESTING, 1 },
    { "qtrans", do_qtrans, 586, POSITION_STANDING, 51 },
    { "nooutdoor", do_set_nooutdoor, 587, POSITION_RESTING, 51 },
    { "dismiss", do_dismiss, 588, POSITION_RESTING, 1 },

    { "setsound", do_setsound, 589, POSITION_STANDING, 53 },
    { "induct", do_induct, 590, POSITION_STANDING, 1 },
    { "expel", do_expel, 591, POSITION_STANDING, 1 },
    { "chat", do_chat, 592, POSITION_RESTING, 1 },
    { "qchat", do_qchat, 594, POSITION_RESTING, 1 },
    { "clanlist", do_clanlist, 595, POSITION_RESTING, 1 },
    { "call steed", do_steed, 596, POSITION_STANDING, 1 },
    { "top10", do_top10, 597, POSITION_RESTING, 59 },
    { "tweak", do_tweak, 598, POSITION_STANDING, 55 },

    { "mrebuild", do_mrebuild, 599, POSITION_STANDING, 59 },
    { "flurry", do_flurry, 600, POSITION_FIGHTING, 1 },
    { "flowerfist", do_flowerfist, 601, POSITION_FIGHTING, 1 },
    { "sharpen", do_sharpen, 602, POSITION_RESTING, 1 },
    { "eval", do_eval, 603, POSITION_STANDING, 53 },
    { "reimburse", do_reimb, 604, POSITION_STANDING, 53 },
    { "remort", do_not_here, 605, POSITION_STANDING, 50 },
    { "affects", do_attribute, 606, POSITION_DEAD, 1 },
/*
 * 607
 * 608
 * 609
 */
    { "weapons", do_weapons, 610, POSITION_RESTING, 1 },
    { "allweapons", do_allweapons, 611, POSITION_SITTING, 1 },
    { "setwtype", do_setwtype, 612, POSITION_STANDING, 53 },
    { "init", do_zload, 613, POSITION_STANDING, 53 },
#ifdef HEDIT_WORKS
    { "hedit", do_hedit, 614, POSITION_RESTING, 51 },
#endif
    { "chtextfile", do_chtextfile, 615, POSITION_RESTING, 53 },

    { "retreat", do_flee, 616, POSITION_SITTING, 1 },
    { "zones", do_list_zones, 617, POSITION_SITTING, 1 },
    { "areas", do_list_zones, 617, POSITION_SITTING, 1 },
    { "recallhome", do_recallhome, 618, POSITION_SITTING, 1 },
    { "scribe", do_scribe, 619, POSITION_STANDING, 1 },
    /*
     * New command for sailing/ships
     */
    { "board", do_sea_commands, 620, POSITION_RESTING, 1 },
    { "disembark", do_sea_commands, 621, POSITION_SITTING, 1 },
    { "embark", do_sea_commands, 622, POSITION_STANDING, 1 },
    { "plank", do_sea_commands, 623, POSITION_STANDING, 1 },
    { "sail", do_sea_commands, 624, POSITION_STANDING, 1 },
    { "steer", do_sea_commands, 625, POSITION_STANDING, 1 }
};
int commandCount = NELEMS(commandList);

extern long     total_connections;
extern long     total_max_players;
extern const char *RaceName[];
extern const int RacialMax[MAX_RACE + 1][MAX_CLASS];
extern char     *motd;
extern char     *wmotd;
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int      top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern char    *pc_class_types[];
#ifdef HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern long     SystemFlags;
extern long     TempDis;
extern struct descriptor_data *descriptor_list;
extern const struct class_def classes[MAX_CLASS];

unsigned char   echo_on[] = { IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0' };
unsigned char   echo_off[] = { IAC, WILL, TELOPT_ECHO, '\0' };
int             Silence = 0;
int             plr_tick_count = 0;
int             MAX_NAME_LENGTH = 11;
char           *Sex[] = { "Neutral", "Male", "Female" };


int             pc_num_class(int clss);
void show_class_selection(struct descriptor_data *d, int r);

/*
 * this is how we tell which race gets which class !
 * to add a new class seletion add the CLASS_NAME above the
 * message 'NEW CLASS SELETIONS HERE'
 */

/*
 * these are the allowable PC races
 */
const int       race_choice[] = {
    RACE_DWARF,
    RACE_MOON_ELF,
    RACE_GOLD_ELF,
    RACE_WILD_ELF,
    RACE_AVARIEL,
    RACE_HUMAN,
    RACE_ROCK_GNOME,
    RACE_FOREST_GNOME,
    RACE_HALFLING,
    RACE_HALF_ELF,
    RACE_HALF_ORC,
    RACE_HALF_OGRE,
    RACE_HALF_GIANT,
    RACE_DROW,                  /* bad guys here */

    RACE_ORC,
    RACE_GOBLIN,
    RACE_TROLL,
    RACE_DARK_DWARF,

    -1
};

/*
 * WARNING: do not remove the 0 at the end of the const, will cause your
 * mud to die hard!
 */

const int       avariel_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_CLERIC + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_CLERIC,
    CLASS_MAGIC_USER + CLASS_CLERIC + CLASS_WARRIOR,
    0
};

const int       moon_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       gold_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */

    0
};

const int       sea_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */

    0
};

const int       wild_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */

    0
};

const int       dwarf_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       halfling_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_THIEF + CLASS_WARRIOR,
    CLASS_DRUID,
    CLASS_MONK,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       rock_gnome_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_CLERIC,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       forest_gnome_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_RANGER,
    CLASS_DRUID,
    CLASS_RANGER + CLASS_DRUID,
    CLASS_CLERIC + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       human_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_BARBARIAN,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_NECROMANCER,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_CLERIC,
    CLASS_NECROMANCER + CLASS_WARRIOR,
    CLASS_WARRIOR + CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_THIEF + CLASS_CLERIC + CLASS_MAGIC_USER,
#if 0
    CLASS_THIEF+CLASS_WARRIOR+CLASS_CLERIC,
#endif
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       half_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_DRUID,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_CLERIC,
    CLASS_WARRIOR + CLASS_CLERIC,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       half_orc_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC + CLASS_THIEF,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       half_ogre_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_CLERIC + CLASS_WARRIOR,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       half_giant_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_BARBARIAN,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       orc_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_THIEF + CLASS_CLERIC,
    /*
     * new class below here
     */

    0
};

const int       goblin_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_WARRIOR + CLASS_THIEF,
    /*
     * new class below here
     */

    0
};

const int       troll_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_BARBARIAN,
    CLASS_WARRIOR + CLASS_CLERIC,
    /*
     * new clases below here
     */

    0
};

const int       default_class_choice[] = {
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_CLERIC,
    CLASS_MAGIC_USER,
    CLASS_BARBARIAN,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_PSI,
    /*
     * new clases below here
     */

    0
};

const int       dark_dwarf_class_choice[] = {
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_WARRIOR + CLASS_CLERIC,
    CLASS_NECROMANCER + CLASS_WARRIOR,
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};

const int       dark_elf_class_choice[] = {
    CLASS_MAGIC_USER,
    CLASS_CLERIC,
    CLASS_WARRIOR,
    CLASS_THIEF,
    CLASS_DRUID,
    CLASS_MONK,
    CLASS_PSI,
    CLASS_WARRIOR + CLASS_MAGIC_USER,
    CLASS_WARRIOR + CLASS_THIEF,
    CLASS_MAGIC_USER + CLASS_THIEF,
    CLASS_CLERIC + CLASS_MAGIC_USER,
    CLASS_NECROMANCER + CLASS_WARRIOR,
    CLASS_MAGIC_USER + CLASS_WARRIOR + CLASS_THIEF,
#if 0
    CLASS_NECROMANCER+CLASS_WARRIOR+CLASS_THIEF,
#endif
    /*
     * NEW CLASS SELECTIONS HERE
     */
    0
};


char           *fill[] = { "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
};

/*
 * this is to determine the relative length of strings with color codes in
 * them by determining the number of $ signs used -Lennya
 */
int color_strlen(struct char_data *ch, char *arg, int cmd)
{
    int             num = 0,
                    abs = 0,
                    rel = 0,
                    i = 0;

    if (!arg || !*arg) {
        return (0);
    }
    abs = strlen(arg);
    for (i = 0; i <= abs; i++) {
        if (arg[i] == '$') {
            num++;
        }
    }
    rel = abs - (6 * num);
    if (rel < 0) {
        Log("erps, oddness in color_strlen");
        return (0);
    }
    return (rel);
}

int search_block(char *arg, char **list, bool exact)
{
    int length;
    int i;

    if (exact) {
        for( i = 0; *list[i] != '\n'; i++ ) {
            if( !strcasecmp( arg, list[i] ) ) {
                return( i );
            }
        }
    } else {
        length = strlen( arg );
        if( length == 0 ) {
            return( -1 );
        }

        for( i = 0; *list[i] != '\n'; i++ ) {
            if( !strncasecmp( arg, list[i], length ) ) {
                return( i );
            }
        }
    }

    return( -1 );
}

int old_search_block(char *argument, int begin, int length, char **list,
                     int mode)
{
    int             guess,
                    found,
                    search;

    /*
     * If the word contain 0 letters, then a match is already found
     */
    found = (length < 1);

    guess = 0;

    /*
     * Search for a match
     */

    if (mode) {
        while (!found && *(list[guess]) != '\n') {
            found = (length == strlen(list[guess]));
            for (search = 0; (search < length && found); search++) {
                found = (*(argument + begin + search) ==
                        *(list[guess] + search));
            }
            guess++;
        }
    } else {
        while (!found && *(list[guess]) != '\n') {
            found = 1;
            for (search = 0; (search < length && found); search++) {
                found = (*(argument + begin + search) ==
                        *(list[guess] + search));
            }
            guess++;
        }
    }

    return (found ? guess : -1);
}

void command_interpreter(struct char_data *ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    extern int      no_specials;
    NODE           *n;
    char           *arg,
                   *arg1,
                   *arg2;
    char           *tmparg;
    int             i,
                    found;

    if (HasClass(ch, TempDis) && GetMaxLevel(ch) < 58 && IS_PC(ch)) {
        send_to_char("Sorry, we are tracking down a bug and this class or one"
                     " of your classes is disabled.\n\r", ch);
        return;
    }

    REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);

#if 0
    /*
     * Removed by Greg Hovey (FALSE)
     */
    if (IS_AFFECTED2(ch, AFF2_AFK) && FALSE) {
        act("$c0006$n has returned to $s keyboard", TRUE, ch, 0, 0,
            TO_ROOM);
        act("$c0006You return to the keyboard.", TRUE, ch, 0, 0, TO_CHAR);
        REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
        if (ch->pc)
            REMOVE_BIT(ch->pc->comm, COMM_AFK);
    }
#endif

    if (MOUNTED(ch) && ch->in_room != MOUNTED(ch)->in_room) {
        Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }

    /*
     *  a bug check.
     */
    if (!IS_NPC(ch)) {
        found = FALSE;
        if ((!ch->player.name[0]) || (ch->player.name[0] < ' ')) {
            Log("Error in character name.  Changed to 'Error'");
            if (ch->player.name) {
                free(ch->player.name);
            }
            ch->player.name = (char *) malloc(6);
            strcpy(ch->player.name, "Error");
            return;
        }
        strcpy(buf, ch->player.name);
        for (i = 0; i < strlen(buf) && !found; i++) {
            if (buf[i] < 65) {
                found = TRUE;
            }
        }
        if (found) {
            Log("Error in character name.  Changed to 'Error'");
            if (ch->player.name) {
                free(ch->player.name);
            }
            ch->player.name = (char *) malloc(6);
            strcpy(ch->player.name, "Error");
            return;
        }
    }

    if (!argument || !*argument || *argument == '\n' || *argument == '\r' ) {
        return;
    } 
    
    if (!isalpha((int)*argument)) {
        arg = (char *)malloc(strlen(argument) + 2);
        if( !arg ) {
            Log( "Nasty error in command_interpreter!!!" );
            return;
        }

        sprintf(arg, "%c %s", *argument, &(argument[1]));
    } else {
        arg = strdup( argument );
        if( !arg ) {
            Log( "Nasty error in command_interpreter!!!" );
            return;
        }
    }
    
    tmparg = arg;
    arg = get_argument_nofill(arg, &arg1);
    arg2 = skip_spaces( arg );

    /*
     * New parser by DM
     */
    if (arg1) {
        n = FindValidCommand(arg1);
    } else {
        n = NULL;
    }

    /*
     * cmd = old_search_block(argument,begin,look_at,command,0);
     */
    if (!n || GetMaxLevel(ch) < n->min_level) {
        send_to_char("Pardon?\n\r", ch);
        if( tmparg ) {
            free(tmparg);
        }
        return;
    }

    if (!n->func) {
        send_to_char("Sorry, but that command has yet to be implemented...\n\r",
                     ch);
        if( tmparg ) {
            free(tmparg);
        }
        return;
    }

    if (IS_AFFECTED(ch, AFF_PARALYSIS) && n->min_pos > POSITION_STUNNED) {
        send_to_char(" You are paralyzed, you can't do much!\n\r", ch);
        if( tmparg ) {
            free(tmparg);
        }
        return;
    }

    if (IS_SET(ch->specials.act, PLR_FREEZE) && IS_PC(ch)) {
        /*
         * They can't move, must have pissed off an immo!
         * make sure polies can move, some mobs have this bit set
         */
        send_to_char("You have been frozen in your steps, you cannot do a "
                     "thing!\n\r", ch);
        if( tmparg ) {
            free(tmparg);
        }
        return;
    }

    if (GET_POS(ch) < n->min_pos) {
        switch (GET_POS(ch)) {
        case POSITION_DEAD:
            send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
            break;
        case POSITION_INCAP:
        case POSITION_MORTALLYW:
            send_to_char("You are in a pretty bad shape, unable to do "
                         "anything!\n\r", ch);
            break;

        case POSITION_STUNNED:
            send_to_char("All you can do right now, is think about the "
                         "stars!\n\r", ch);
            break;
        case POSITION_SLEEPING:
            send_to_char("In your dreams, or what?\n\r", ch);
            break;
        case POSITION_RESTING:
            send_to_char("Nah... You feel too relaxed to do that..\n\r", ch);
            break;
        case POSITION_SITTING:
            send_to_char("Maybe you should get on your feet first?\n\r", ch);
            break;
        case POSITION_FIGHTING:
            send_to_char("No way! You are fighting for your life!\n\r", ch);
            break;
        case POSITION_STANDING:
            send_to_char("Fraid you can't do that\n\r", ch);
            break;

        }
        if( tmparg ) {
            free(tmparg);
        }
        return;
    } 
    
    if (n->log) {
        sprintf(buf, "%s:%s", ch->player.name, argument);
        slog(buf);
    }

    /*
     * so you can log mobs if ya need to
     */
#ifdef LOG_MOB
    if (!IS_PC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        sprintf(buf, "[%ld] <%s>:%s", ch->in_room, ch->player.name, argument);
        slog(buf);
    }
#endif

    /*
     * to log all pc's
     */
    if (IS_SET(SystemFlags, SYS_LOGALL)) {
        if (IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) {
            sprintf(buf, "[%ld] %s:%s", ch->in_room, ch->player.name, argument);
            slog(buf);
        }
    } else if (IS_AFFECTED2(ch, AFF2_LOG_ME)) {
        /*
         * user flagged as log person
         */
        sprintf(buf, "[%ld] %s:%s", ch->in_room, ch->player.name, argument);
        slog(buf);
    } else if (IS_IMMORTAL(ch) && GetMaxLevel(ch) < MAX_IMMORT) {
        /*
         * we log ALL immortals
         */
        sprintf(buf, "[%ld] %s:%s", ch->in_room, ch->player.name, argument);
        slog(buf);
    }

    if (GET_GOLD(ch) > 2000000) {
        sprintf(buf, "%s:%s", fname(ch->player.name), argument);
        slog(buf);
    }

    if (no_specials || !special(ch, n->number, arg2)) {
        (*n->func)(ch, arg2, n->number);
    }
    if( tmparg ) {
        free(tmparg);
    }
}

char *get_argument(char *line_in, char **arg_out)
{
    return( get_argument_common(line_in, arg_out, TRUE, '\0') );
}

char *get_argument_nofill(char *line_in, char **arg_out)
{
    return( get_argument_common(line_in, arg_out, FALSE, '\0') );
}

char *get_argument_delim(char *line_in, char **arg_out, char delim)
{
    return( get_argument_common(line_in, arg_out, TRUE, delim) );
}

char *get_argument_common(char *line_in, char **arg_out, int do_fill,
                          char delim)
{
    char           *arg;
    char           *line;
    int             i;
    int             length;
    char            delimstr[2];

    line = line_in;
    if( !line || !*line ) {
        *arg_out = NULL;
        return( NULL );
    }

    if( delim ) {
        delimstr[0] = delim;
        delimstr[1] = '\0';
    }

    /* Split out the first argument into arg_out */
    do {
        if( !line ) {
            *arg_out = NULL;
            return( NULL );
        }

        line = skip_spaces( line );
        if( !line ) {
            *arg_out = NULL;
            return( NULL );
        }
        
        if( delim && *line == delim ) {
            /* Found a delimiter, skip past it, then match the other one rather
             * than a space
             */
            line++;
            arg = strsep( &line, delimstr );
        } else {
            arg = strsep( &line, " " );
        }

        /* Now arg points to the first argument, and *line points at the rest
         */
        *arg_out = arg;

        /* Skip the "fill" words */
    } while (do_fill && fill_word(*arg_out));

    /* Convert the argument to lower case */
    length = strlen( *arg_out );
    for( i = 0; i < length; i++ ) {
        (*arg_out)[i] = tolower((*arg_out)[i]);
    }

    return( line );
}



int is_number(char *str)
{
    if (*str == '\0') {
        return (FALSE);
    } else if (strnlen(str, 10) > 8) {
        return (FALSE);
    } else if (atoi(str) == 0 && str[0] != '0') {
        return (FALSE);
    } else {
        return (TRUE);
    }
}


int fill_word(char *argument)
{
    return (search_block(argument, fill, TRUE) >= 0);
}


int special(struct char_data *ch, int cmd, char *arg)
{
    register struct obj_data *i;
    register struct char_data *k;
    int             j;

    if (ch->in_room == NOWHERE) {
        char_to_room(ch, 3001);
        return (0);
    }

    /*
     * special in room?
     */
    if (real_roomp(ch->in_room)->funct &&
        (*real_roomp(ch->in_room)->funct) (ch, cmd, arg,
                                           real_roomp(ch->in_room),
                                           PULSE_COMMAND)) {
        return (1);
    }

    /*
     * special in equipment list?
     */
    for (j = 0; j <= (MAX_WEAR - 1); j++) {
        if (ch->equipment[j] && ch->equipment[j]->item_number >= 0) {
            if (IS_SET(ch->equipment[j]->obj_flags.extra_flags,
                ITEM_ANTI_SUN)) {
                AntiSunItem(ch, cmd, arg, ch->equipment[j], PULSE_COMMAND);
            }
            if (ch->equipment[j] &&
                obj_index[ch->equipment[j]->item_number].func &&
                (*obj_index[ch->equipment[j]->item_number].func)
                      (ch, cmd, arg, ch->equipment[j], PULSE_COMMAND)) {
                return (TRUE);
            }
        }
    }

    /*
     * item_number >=0
     */
    /*
     * special in inventory?
     */
    for (i = ch->carrying; i; i = i->next_content) {
        if (i->item_number >= 0 && obj_index[i->item_number].func &&
            (*obj_index[i->item_number].func) (ch, cmd, arg, i,
                                               PULSE_COMMAND)) {
            /*
             * Crashes here when saving item twice
             */
            return (1);
        }
    }

    /*
     * special in mobile present?
     */
    for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room) {
        if (IS_MOB(k) && k != ch && mob_index[k->nr].func &&
            (*mob_index[k->nr].func) (ch, cmd, arg, k, PULSE_COMMAND)) {
            return (1);
        }
    }

    /*
     * special in object present?
     */
    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
        if (i->item_number >= 0 && obj_index[i->item_number].func &&
            (*obj_index[i->item_number].func) (ch, cmd, arg, i,
                                               PULSE_COMMAND)) {
            /*
             * Crash here maybe?? FROZE HERE!! loop?
             */
            return (1);
        }
    }

    return (0);
}



void assign_command_pointers(void)
{
    int i;
    struct command_def *cmd;

    InitRadix();

    for( i = 0; i < commandCount; i++ ) {
        cmd = &commandList[i];
        AddCommand( cmd->name, cmd->func, cmd->number, cmd->min_pos,
                    cmd->min_lev );
    }
}

/*
 *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */

/*
 * locate entry in p_table with entry->name == name. -1 mrks failed search
 */
int find_name(char *name)
{
    int             i;

    for (i = 0; i <= top_of_p_table; i++) {
        if (!strcasecmp((player_table + i)->name, name)) {
            return (i);
        }
    }

    return (-1);
}

int _parse_name(char *arg, char *name)
{
    int             i;

    /*
     * skip whitespaces
     */
    arg = skip_spaces(arg);
    if( !arg ) {
        return( 0 );
    }

    for (i = 0; (*name = *arg); arg++, i++, name++) {
        if ((*arg < 0) || !isalpha((int)*arg) || i > MAX_NAME_LENGTH) {
            return (1);
        }

    }

    if (!i) {
        return (1);
    }
    return (0);
}


int _check_ass_name(char *name)
{
    /*
     * 0 - full match
     * 1 - from start of string
     * 2 - from end of string
     * 3 - somewhere in string
     */
    static struct shitlist {
        int             how;
        char            name[80];
    }              *shitlist = NULL;
    FILE           *f;
    char            buf[512];
    int             i,
                    j,
                    k;

    if (strlen(name) > MAX_NAME_LENGTH) {
        return 1;
    }
    if (!shitlist) {
        if ((f = fopen(ASSHOLE_FNAME, "rt")) == NULL) {
            Log("can't open asshole names list");
            shitlist = (struct shitlist *) calloc(1, sizeof(struct shitlist));
            *shitlist[0].name = 0;
            return 0;
        }
        for (i = 0; fgets(buf, 180, f) != NULL; i++) {
            /*
             * Empty loop
             */
        }
        shitlist = (struct shitlist *) calloc((i + 3), sizeof(struct shitlist));
        rewind(f);
        for (i = 0; fgets(buf, 180, f) != NULL; i++) {
            if (buf[strlen(buf) - 1] == '\n' || buf[strlen(buf) - 1] == '\r') {
                buf[strlen(buf) - 1] = 0;
            }
            if (buf[strlen(buf) - 1] == '\n' || buf[strlen(buf) - 1] == '\r') {
                buf[strlen(buf) - 1] = 0;
            }

            if (*buf == '*') {
                if (buf[strlen(buf) - 1] == '*') {
                    shitlist[i].how = 3;
                    buf[strlen(buf) - 1] = 0;
                    strcpy(shitlist[i].name, buf + 1);
                } else {
                    shitlist[i].how = 2;
                    strcpy(shitlist[i].name, buf + 1);
                }
            } else {
                if (buf[strlen(buf) - 1] == '*') {
                    shitlist[i].how = 1;
                    buf[strlen(buf) - 1] = 0;
                    strcpy(shitlist[i].name, buf);
                } else {
                    shitlist[i].how = 0;
                    strcpy(shitlist[i].name, buf);
                }
            }
        }
        *shitlist[i].name = 0;

        for (i = 0; *shitlist[i].name; i++) {
            sprintf(buf, "mode: %d, name: %s", shitlist[i].how,
                    shitlist[i].name);
#if 0
            Log(buf);
#endif
        }
    }

    for (j = 0; *shitlist[j].name; j++) {
        switch (shitlist[j].how) {
        case 0:
            if (!strcasecmp(name, shitlist[j].name)) {
                return 1;
            }
            break;
        case 1:
            if (!strncasecmp(name, shitlist[j].name, strlen(shitlist[j].name))) {
                return 1;
            }
            break;
        case 2:
            if (strlen(name) < strlen(shitlist[j].name)) {
                break;
            }
            if (!strcasecmp(name + (strlen(name) - strlen(shitlist[j].name)),
                         shitlist[j].name)) {
                return 1;
            }
            break;
        case 3:
            if (strlen(name) < strlen(shitlist[j].name)) {
                break;
            }
            for (k = 0; k <= strlen(name) - strlen(shitlist[j].name); k++) {
                if (!strncasecmp(name + k, shitlist[j].name,
                              strlen(shitlist[j].name))) {
                    return 1;
                }
            }
            break;
        default:
            Log("Grr! invalid value in shitlist, interpreter.c _parse_name");
            return 1;
        }
    }
    return (0);
}


void show_menu(struct descriptor_data *d)
{

    int             bit;
    char            buf[100];
    char            bufx[1000];
    char            cls[50];
    char            mainclass[50];

    cls[0] = '\0';

    for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
        if (HasClass(d->character, pc_num_class(bit))) {
            strcat(cls, classes[bit].abbrev);
        }
    }
    if (!(strcmp(cls, ""))) {
        sprintf(cls, "None Selected");
    }
    sprintf(mainclass, "%s", "");
    if (d->character->specials.remortclass) {
       /*
        * remort == 0 means none picked
        */
        strcat(mainclass, 
               classes[(d->character->specials.remortclass - 1)].abbrev);
    }
    if (!(strcmp(mainclass, ""))) {
        sprintf(mainclass, "None Selected");
    }
    sprintf(bufx, "$c0009-=$c0015Havok Character Creation Menu [%s]"
                  "$c0009=-\n\r\n\r", GET_NAME(d->character));

    sprintf(buf, "$c00151) $c0012Gender.[$c0015%s$c0012]\n\r",
            Sex[((int) GET_SEX(d->character))]);
    strcat(bufx, buf);

    sprintf(buf, "$c00152) $c0012Ansi Colors.\n\r");
    strcat(bufx, buf);

    if (GET_RACE(d->character) != 0) {
        sprintf(buf, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                RaceName[GET_RACE(d->character)]);
        strcat(bufx, buf);
    } else {
        /*
         * make default race to Human rather than half-breed
         */
        GET_RACE(d->character) = 1;
        sprintf(buf, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                RaceName[GET_RACE(d->character)]);
        strcat(bufx, buf);
    }

    sprintf(buf, "$c00154) $c0012Class.[$c0015%s$c0012]\n\r", cls);
    strcat(bufx, buf);

    sprintf(buf, "$c00155) $c0012Main Class.[$c0015%s$c0012]\n\r", mainclass);
    strcat(bufx, buf);

    if (!GET_CON(d->character) || GET_CON(d->character) == 0) {
        strcat(bufx, "$c00156) $c0012Character Stats.[$c0015None "
                     "Picked$c0012]\n\r");
    } else {
        strcat(bufx, "$c00156) $c0012Character Stats.[$c0015Done$c0012]\n\r");
    }
    sprintf(buf, "$c00157) $c0012Alignment.[$c000W%s$c000B]\n\r\n\r",
            (GET_ALIGNMENT(d->character) ?
             AlignDesc(GET_ALIGNMENT(d->character)) : "None"));
    strcat(bufx, buf);

    strcat(bufx, "$c0015D) $c0012Done!\n\r\n\r");
    strcat(bufx, "$c0011Please pick an option: \n\r");

    send_to_char(bufx, d->character);
}

void nanny(struct descriptor_data *d, char *arg)
{
    struct descriptor_data *desc;
    char            buf[1024];
    char            bufx[1000];

    int             player_i,
                    count = 0,
                    oops = FALSE,
                    index = 0,
                    chosen = 0;
    char            tmp_name[20];
    struct char_file_u tmp_store;
    struct char_data *tmp_ch;
    struct descriptor_data *k;
    int             count_players = 0,
                    bit = 0;
    int             i = 0,
                    tmpi = 0;
    int             already_p = 0;
    int             pick = 0;
    int             ii = 0;
    struct char_file_u ch_st;
    FILE           *char_file;
    struct obj_data *obj;

    write(d->descriptor, echo_on, 6);

    switch (STATE(d)) {
    case CON_CREATION_MENU:
#if 0
        show_menu(d);
#endif
        arg = skip_spaces(arg);
        if( !arg ) {
            show_menu(d);
            send_to_char("Invalid Choice.. Try again..", d->character);
            return;
        }

        switch (*arg) {
        case '1':
            SEND_TO_Q("What is your sex (M/F) ? ", d);
            STATE(d) = CON_QSEX;
            break;
        case '2':
            SEND_TO_Q("Would you like ansi colors? (Yes or No)", d);
            STATE(d) = CON_ANSI;
            break;
        case '3':
            show_race_choice(d);
            SEND_TO_Q("For help type '?'- will list level limits. \n\r RACE:  ",
                      d);
            STATE(d) = CON_QRACE;
            d->character->player.class = 0;
            break;
        case '4':
            GET_ALIGNMENT(d->character) = 0;
            GET_CON(d->character) = 0;
            send_to_char("class", d->character);
            SEND_TO_Q("\n\rSelect your class now.\n\r", d);
            show_class_selection(d, GET_RACE(d->character));
            SEND_TO_Q("Enter ? for help.\n\r", d);
            SEND_TO_Q("\n\rClass :", d);
            STATE(d) = CON_QCLASS;
            break;
        case '5':
            if (d->character->player.class != 0) {
                SEND_TO_Q("\n\rSelect your main class from the options "
                          "below.\n\r", d);

                for (chosen = 0; chosen <= NECROMANCER_LEVEL_IND; chosen++) {
                    if (HasClass(d->character, pc_num_class(chosen))) {
                        sprintf(bufx, "[%2d] %s\n\r", chosen,
                                classes[chosen].name);
                        send_to_char(bufx, d->character);
                    }
                }
                SEND_TO_Q("\n\rMain Class :", d);
                STATE(d) = CON_MCLASS;
            } else {
                SEND_TO_Q("\nPlease select a class first.\n\r", d);
            }
            break;
        case '6':
            d->character->reroll = 20;
            if (d->character->player.class != 0) {
                SEND_TO_Q("\n\rSelect your stat priority, by listing them from"
                          " highest to lowest\n\r", d);
                SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest"
                          " roll in Strength, \n\r", d);
                SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly"
                          " charisma\n\r", d);
                SEND_TO_Q("Your choices? ", d);
                STATE(d) = CON_STAT_LIST;
            } else {
                SEND_TO_Q("\nPlease select a class first.\n\r", d);
            }
            break;
        case '7':
            sprintf(bufx,
                    "Your alignment is an indication of how well or badly you"
                    " have morally\n\r"
                    "conducted yourself in the game. It ranges numerically, "
                    "from -1000\n\r"
                    "($c000RChaotic Evil$c000w) to 1000 ($c000WLawful Good"
                    "$c000w), 0 being neutral. Generally, if you kill\n\r"
                    "'Good' mobs, you will gravitate towards Evil, and "
                    "vice-versa. Some spells\n\r"
                    "and skills also affect your alignment. ie Backstab makes"
                    " you evil, and\n\r"
                    "the spell heal makes you good\n\r");
            send_to_char(bufx, d->character);

            if (HasClass(d->character, CLASS_PALADIN)) {
                sprintf(bufx, "Please select your alignment "
                              "($c000WGood$c000w)");
            } else if (HasClass(d->character, CLASS_DRUID)) {
                sprintf(bufx, "Please select your alignment (Neutral)");
            } else if (HasClass(d->character, CLASS_NECROMANCER)) {
                sprintf(bufx, "Please select your alignment "
                              "($c000REvil$c000w)");
            } else if (HasClass(d->character, CLASS_RANGER)) {
                sprintf(bufx, "Please select your alignment "
                              "($c000WGood$c000w/Neutral$c000w)");
            } else {
                sprintf(bufx, "Please select your alignment "
                              "($c000WGood$c000w/Neutral$c000w/"
                              "$c000REvil$c000w)");
            }
            send_to_char(bufx, d->character);
            STATE(d) = CON_ALIGNMENT;
            break;

        case 'd':
        case 'D':
            count_players = 0;
            for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
                if (HasClass(d->character, pc_num_class(bit))) {
                    count_players++;
                }
            }
            if (count_players <= 0) {
                SEND_TO_Q("Please enter a valid class.", d);
                return;
            }
            if (d->character->specials.remortclass <= 0) {
                SEND_TO_Q("Please enter a valid main class.", d);
                return;
            }

            if (GET_SEX(d->character) == 0) {
                SEND_TO_Q("Please enter a proper sex.", d);
                return;
            }

            if (!GET_ALIGNMENT(d->character)) {
                SEND_TO_Q("Please choose an alignment.", d);
                return;
            }
            if (!GET_CON(d->character) || GET_CON(d->character) == 0) {
                SEND_TO_Q("Please pick your stats.", d);
                return;
            }

            sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),
                    d->host);
            Log(buf);

            /*
             * now that classes are set, initialize
             */
            init_char(d->character);

            /*
             * create an entry in the file
             */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, AUTO_RENT);

            for( i = 0; newbie_note[i]; i++ ) {
                SEND_TO_Q(newbie_note[i], d);
            }
#if 0
            SEND_TO_Q(motd, d);
#endif
            send_to_char(motd, d->character);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
            break;
        default:
            show_menu(d);
            send_to_char("Invalid Choice.. Try again..", d->character);
            break;
        }
        break;
    case CON_ALIGNMENT:
        arg = skip_spaces(arg);
        if( !arg ) {
            SEND_TO_Q("Please select a alignment.\n\r", d);
            return;
        }

        switch (*arg) {
        case 'n':
        case 'N':
            if (!HasClass(d->character, CLASS_PALADIN) &&
                !HasClass(d->character, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(d->character) = 1;
                send_to_char("You have chosen to be Neutral in "
                             "alignment.\n\r\n\r", d->character);
                STATE(d) = CON_CREATION_MENU;
                show_menu(d);
            }
            break;
        case 'g':
        case 'G':
            if (!HasClass(d->character, CLASS_DRUID) &&
                !HasClass(d->character, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(d->character) = 1000;
                send_to_char("You have chosen to be a follower of "
                             "light.\n\r\n\r", d->character);
                show_menu(d);
                STATE(d) = CON_CREATION_MENU;
            }
            break;
        case 'e':
        case 'E':
            if (!HasClass(d->character, CLASS_DRUID) &&
                !HasClass(d->character, CLASS_PALADIN) &&
                !HasClass(d->character, CLASS_RANGER)) {
                GET_ALIGNMENT(d->character) = -1000;
                send_to_char("You have chosen the dark side.\n\r\n\r",
                             d->character);
                STATE(d) = CON_CREATION_MENU;
                show_menu(d);
            }
            break;

        default:
            SEND_TO_Q("Please select a alignment.\n\r", d);
            break;
        }
        break;

    case CON_ANSI:
        arg = skip_spaces(arg);
        if( !arg ) {
            SEND_TO_Q("Please type Yes or No.\n\r", d);
            SEND_TO_Q("Would you like ansi colors? :", d);
#if 0
            STATE(d) = CON_ANSI;
#endif
            return;
        }

        switch (*arg) {
        case 'y':
        case 'Y':
            /*
             * Set ansi
             */
            SET_BIT(d->character->player.user_flags, USE_ANSI);

            send_to_char("$c0012A$c0010n$c0011s$c0014i$c0007 colors "
                    "enabled.\n\r\n\r", d->character);
            show_menu(d);
            STATE(d) = CON_CREATION_MENU;
            break;

        case 'n':
        case 'N':
            STATE(d) = CON_CREATION_MENU;
            show_menu(d);
            break;

        default:
            SEND_TO_Q("Please type Yes or No.\n\r", d);
            SEND_TO_Q("Would you like ansi colors? :", d);
#if 0
            STATE(d) = CON_ANSI;
#endif
            return;
            break;
        }
        break;

    case CON_QRACE:
        d->character->reroll = 20;
        arg = skip_spaces(arg);
        if (!arg) {
            show_race_choice(d);
            SEND_TO_Q("For help, and level limits type '?'. \n\r RACE?:  ", d);
            STATE(d) = CON_QRACE;
        } else if (*arg == '?') {
            for( i = 0; racehelp[i]; i++ ) {
                SEND_TO_Q(racehelp[i], d);
            }
            show_race_choice(d);
            SEND_TO_Q("For help type '?' - will also list level limits. \n\r"
                      " RACE?:  ", d);
            STATE(d) = CON_QRACE;
        } else {
            while (race_choice[i] != -1)
                i++;
            tmpi = atoi(arg);
            if (tmpi >= 0 && tmpi <= i - 1) {
                /*
                 * set the chars race to this
                 */
                GET_RACE(d->character) = race_choice[tmpi];
                show_menu(d);
                STATE(d) = CON_CREATION_MENU;
            } else {
                SEND_TO_Q("\n\rThat's not a race.\n\rRACE?:", d);
                show_race_choice(d);
                STATE(d) = CON_QRACE;
                /*
                 * bogus race selection!
                 */
            }

        }
        break;

    case CON_NME:
    /*
     * wait for input of name
     */
        if (!d->character) {
            CREATE(d->character, struct char_data, 1);
            clear_char(d->character);
            d->character->desc = d;
        }

        arg = skip_spaces(arg);
        if (!arg) {
            close_socket(d);
        } else {
            if (_parse_name(arg, tmp_name)) {
                SEND_TO_Q("Illegal name, please try another.\r\n", d);
                SEND_TO_Q("Name: ", d);
                return;
            }
            if (SiteLock(d->host)) {
                SEND_TO_Q("Sorry, this site is temporarily banned.\n\r", d);
                STATE(d) = CON_WIZLOCK;
                return;
            }

            if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
                /*
                 * connecting an existing character ...
                 */
                store_to_char(&tmp_store, d->character);
                strcpy(d->pwd, tmp_store.pwd);
                d->pos = player_table[player_i].nr;
                SEND_TO_Q("Password: ", d);
                write(d->descriptor, echo_off, 4);
                STATE(d) = CON_PWDNRM;
            } else {
                /*
                 * player unknown gotta make a new
                 */
                if (_check_ass_name(tmp_name)) {
                    SEND_TO_Q("\n\rIllegal name, please try another.", d);
                    SEND_TO_Q("Name: ", d);
                    return;
                }
                /*
                 * move forward creating new character
                 */
                if (!IS_SET(SystemFlags, SYS_WIZLOCKED)) {
                    CREATE(GET_NAME(d->character), char, strlen(tmp_name) + 1);
                    strcpy(GET_NAME(d->character), CAP(tmp_name));
                    sprintf(buf, "Did I get that right, %s (Y/N)? ", tmp_name);
                    SEND_TO_Q(buf, d);
                    STATE(d) = CON_NMECNF;
                    /*
                     * We are wiz locked ...
                     */
                } else {
                    sprintf(buf, "Sorry, no new characters at this time\n\r");
                    SEND_TO_Q(buf, d);
                    STATE(d) = CON_WIZLOCK;
                }
            }
        }
        break;

    case CON_NMECNF:
    /*
     * wait for conf. of new name
     */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if( !arg ) {
            /*
             * Please do Y or N
             */
            SEND_TO_Q("Please type Yes or No? ", d);
            return;
        }

        if (*arg == 'y' || *arg == 'Y') {
            write(d->descriptor, echo_on, 4);
            SEND_TO_Q("New character.\n\r", d);

            sprintf(buf, "Give me a password for %s: ", GET_NAME(d->character));

            SEND_TO_Q(buf, d);
            write(d->descriptor, echo_off, 4);
            STATE(d) = CON_PWDGET;
        } else if (*arg == 'n' || *arg == 'N') {
            SEND_TO_Q("Ok, what IS it, then? ", d);
            if (GET_NAME(d->character)) {
                free(GET_NAME(d->character));
            }
            STATE(d) = CON_NME;
        } else {
            /*
             * Please do Y or N
             */
            SEND_TO_Q("Please type Yes or No? ", d);
        }
        break;

    case CON_PWDNRM:
    /*
     * get pwd for known player
     */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg) {
            close_socket(d);
        } else {
            if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
                SEND_TO_Q("Wrong password.\n\r", d);
                sprintf(buf, "%s entered a wrong password",
                        GET_NAME(d->character));
                Log(buf);
                close_socket(d);
                return;
            }
#ifdef IMPL_SECURITY
            if (top_of_p_table > 0) {
                if (GetMaxLevel(d->character) >= 58) {
                    switch (SecCheck(GET_NAME(d->character), d->host)) {
                    case -1:
                    case 0:
                        SEND_TO_Q("Security check reveals invalid site\n\r", d);
                        SEND_TO_Q("Speak to an implementor to fix problem\n\r",
                                  d);
                        SEND_TO_Q("If you are an implementor, add yourself to"
                                  " the\n\r", d);
                        SEND_TO_Q("Security directory (lib/security)\n\r", d);
                        close_socket(d);
                        break;
                    }
                }
            }
#endif
            /*
             * Check if already playing
             */
            for (k = descriptor_list; k; k = k->next) {
                if ((k->character != d->character) && k->character) {
                    /*
                     * check to see if 'character' was switched to by the
                     * one trying to connect
                     */
                    if (k->original) {
                        if (GET_NAME(k->original) &&
                            strcasecmp(GET_NAME(k->original),
                                    GET_NAME(d->character)) == 0) {
                            already_p = 1;
                        }
                    } else {
                        /*
                         * No switch has been made
                         */
                        if (GET_NAME(k->character) &&
                            strcasecmp(GET_NAME(k->character),
                                    GET_NAME(d->character)) == 0) {
                            already_p = 1;
                        }
                    }
                }

                if (already_p) {
                    close_socket(k);
                    break;
                }
            }

            /*
             * Check if disconnected ...
             */
            for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
                if ((!strcasecmp(GET_NAME(d->character), GET_NAME(tmp_ch)) &&
                     !tmp_ch->desc && !IS_NPC(tmp_ch)) ||
                    (IS_NPC(tmp_ch) && tmp_ch->orig &&
                     !strcasecmp(GET_NAME(d->character),
                              GET_NAME(tmp_ch->orig)))) {

                    write(d->descriptor, echo_on, 6);
                    SEND_TO_Q("Reconnecting.\n\r", d);

                    free_char(d->character);
                    tmp_ch->desc = d;
                    d->character = tmp_ch;
                    tmp_ch->specials.timer = 0;
                    if (!IS_IMMORTAL(tmp_ch)) {
                        tmp_ch->invis_level = 0;
                    }
                    if (tmp_ch->orig) {
                        tmp_ch->desc->original = tmp_ch->orig;
                        tmp_ch->orig = 0;
                    }
                    d->character->persist = 0;
                    STATE(d) = CON_PLYNG;

                    if (!IS_IMMORTAL(tmp_ch) || tmp_ch->invis_level <= 58) {
                        act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
                        sprintf(buf, "%s[%s] has reconnected.",
                                GET_NAME(d->character), d->host);
                        Log(buf);
                    }
                    
                    if (d->character->specials.hostip) {
                        free(d->character->specials.hostip);
                    }
                    d->character->specials.hostip = strdup(d->host);
                    write_char_extra(d->character);
                    return;
                }
            }

            load_char_extra(d->character);
            if (d->character->specials.hostip == NULL) {
                if (!IS_IMMORTAL(d->character) ||
                    d->character->invis_level <= 58) {
                    sprintf(buf, "%s[%s] has connected.\n\r",
                            GET_NAME(d->character), d->host);
                    Log(buf);
                }
            } else {
                if (!IS_IMMORTAL(d->character) ||
                    d->character->invis_level <= 58) {
                    sprintf(buf, "%s[%s] has connected - Last connected"
                            " from[%s]",
                            GET_NAME(d->character), d->host,
                            d->character->specials.hostip);
                    Log(buf);
                }
                SEND_TO_Q(buf, d);
            }

            if (d->character->specials.hostip) {
                free(d->character->specials.hostip);
            }
            d->character->specials.hostip = strdup(d->host);
            write_char_extra(d->character);

            send_to_char(motd, d->character);
#if 0
            SEND_TO_Q(motd, d);
#endif
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        }
        break;

    case CON_PWDGET:
    /*
     * get pwd for new player
     */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            write(d->descriptor, echo_on, 6);
            SEND_TO_Q("Illegal password.\n\r", d);
            SEND_TO_Q("Password: ", d);

            write(d->descriptor, echo_off, 4);
            return;
        }

        strncpy(d->pwd, (char *) crypt(arg, d->character->player.name), 10);
        *(d->pwd + 10) = '\0';
        write(d->descriptor, echo_on, 6);
        SEND_TO_Q("Please retype password: ", d);

        write(d->descriptor, echo_off, 4);
        STATE(d) = CON_PWDCNF;
        break;

    case CON_PWDCNF:
    /*
     * get confirmation of new pwd
     */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            write(d->descriptor, echo_on, 6);

            SEND_TO_Q("Passwords don't match.\n\r", d);
            SEND_TO_Q("Retype password: ", d);
            STATE(d) = CON_PWDGET;
            write(d->descriptor, echo_off, 4);
            return;
        } else {
            write(d->descriptor, echo_on, 6);

            SEND_TO_Q("Would you like to have ansi colors? ", d);
#if 0
            show_menu(d);
#endif
            STATE(d) = CON_ANSI;
#if 0
            CON_CREATION_MENU;
#endif
        }
        break;

    case CON_QSEX:
    /*
     * query sex of new user
     */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if( !arg ) {
            SEND_TO_Q("That's not a sex..\n\r", d);
            SEND_TO_Q("What IS your sex? :", d);
            return;
        }

        switch (*arg) {
        case 'm':
        case 'M':
            /*
             * sex MALE
             */
            d->character->player.sex = SEX_MALE;
            break;

        case 'f':
        case 'F':
            /*
             * sex FEMALE
             */
            d->character->player.sex = SEX_FEMALE;
            break;

        default:
            SEND_TO_Q("That's not a sex..\n\r", d);
            SEND_TO_Q("What IS your sex? :", d);
            return;
            break;
        }

        show_menu(d);
        STATE(d) = CON_CREATION_MENU;
        break;

    case CON_STAT_LIST:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        index = 0;
        while (arg && *arg && index < MAX_STAT) {
            if (*arg == 'S' || *arg == 's') {
                d->stat[index++] = 's';
            }
            if (*arg == 'I' || *arg == 'i') {
                d->stat[index++] = 'i';
            }
            if (*arg == 'W' || *arg == 'w') {
                d->stat[index++] = 'w';
            }
            if (*arg == 'D' || *arg == 'd') {
                d->stat[index++] = 'd';
            }
            if (*arg == 'C' || *arg == 'c') {
                arg++;
                if (*arg == 'O' || *arg == 'o') {
                    d->stat[index++] = 'o';
                } else if (*arg == 'H' || *arg == 'h') {
                    d->stat[index++] = 'h';
                } else {
                    SEND_TO_Q("That was an invalid choice.\n\r", d);
                    SEND_TO_Q("\n\rSelect your stat priority, by listing them"
                              " from highest to lowest\n\r", d);
                    SEND_TO_Q("Seperated by spaces.  don't duplicate letters"
                              "\n\r", d);
                    SEND_TO_Q("for example: 'S I W D Co Ch' would put the "
                              "highest roll in Strength, \n\r", d);
                    SEND_TO_Q("next in intelligence, Wisdom, Dex, Con and "
                              "lastly Charisma\n\r", d);
                    SEND_TO_Q("Your choice? ", d);
                    STATE(d) = CON_STAT_LIST;
                    break;
                }
            }
            arg++;
        }

        if (index < MAX_STAT) {
            SEND_TO_Q("You did not enter enough legal stats\n\r", d);
            SEND_TO_Q("That was an invalid choice.\n\r", d);
            SEND_TO_Q("\n\rSelect your stat priority, by listing them from "
                      "highest to lowest\n\r", d);
            SEND_TO_Q("Seperated by spaces, don't duplicate letters \n\r", d);
            SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll"
                      " in Strength, \n\r", d);
            SEND_TO_Q("next in intelligence, Wisdom, Dex, Con and lastly "
                      "Charisma\n\r", d);
            SEND_TO_Q("Your choice? ", d);
            STATE(d) = CON_STAT_LIST;
            break;
        } else {
            roll_abilities(d->character);
            SEND_TO_Q("Your current stats are:\n\r", d);
            sprintf(buf, "STR: %s\n\r", STAT_SWORD(GET_STR(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CON: %s\n\r", STAT_SWORD(GET_CON(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "DEX: %s\n\r", STAT_SWORD(GET_DEX(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "INT: %s\n\r", STAT_SWORD(GET_INT(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "WIS: %s\n\r", STAT_SWORD(GET_WIS(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CHR: %s\n\r", STAT_SWORD(GET_CHR(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "\n\rYou have 9 rerolls left, press R to reroll, any"
                         " other key to keep.\n\r");
            SEND_TO_Q(buf, d);
            if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
                /*
                 * set the AUTH flags
                 * (3 chances)
                 */
                d->character->generic = NEWBIE_REQUEST + NEWBIE_CHANCES;
            }
        }

        if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
            /*
             * set the AUTH flags
             * (3 chances)
             */
            d->character->generic = NEWBIE_REQUEST + NEWBIE_CHANCES;
        }

        STATE(d) = CON_REROLL;
        break;

    case CON_REROLL:
        arg = skip_spaces(arg);
        d->character->reroll--;

        if (!arg || (*arg != 'r' && *arg != 'R')) {
            SEND_TO_Q("Stats chosen!\n\r", d);

            STATE(d) = CON_CREATION_MENU;

            if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
                show_menu(d);
                STATE(d) = CON_CREATION_MENU;
            } else {
                show_menu(d);
                STATE(d) = CON_CREATION_MENU;
            }
        } else if (d->character->reroll != 0) {
            roll_abilities(d->character);
            SEND_TO_Q("Your current stats are:\n\r", d);
            sprintf(buf, "STR: %s\n\r",
                    STAT_SWORD(GET_STR(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CON: %s\n\r",
                    STAT_SWORD(GET_CON(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "DEX: %s\n\r",
                    STAT_SWORD(GET_DEX(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "INT: %s\n\r",
                    STAT_SWORD(GET_INT(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "WIS: %s\n\r",
                    STAT_SWORD(GET_WIS(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CHR: %s\n\r",
                    STAT_SWORD(GET_CHR(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "\n\rYou have %d rerolls left, press R to reroll,"
                         " any other key to keep.\n\r",
                         d->character->reroll);
            SEND_TO_Q(buf, d);
            STATE(d) = CON_REROLL;
        } else {
            roll_abilities(d->character);
            SEND_TO_Q("Your final stats are:\n\r", d);
            sprintf(buf, "STR: %s\n\r",
                    STAT_SWORD(GET_STR(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CON: %s\n\r",
                    STAT_SWORD(GET_CON(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "DEX: %s\n\r",
                    STAT_SWORD(GET_DEX(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "INT: %s\n\r",
                    STAT_SWORD(GET_INT(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "WIS: %s\n\r",
                    STAT_SWORD(GET_WIS(d->character)));
            SEND_TO_Q(buf, d);
            sprintf(buf, "CHR: %s\n\r",
                    STAT_SWORD(GET_CHR(d->character)));
            SEND_TO_Q(buf, d);
            SEND_TO_Q("Stats chosen!", d);
            STATE(d) = CON_CREATION_MENU;
            break;
        }
        break;

    case CON_PRESS_ENTER:
        /*
         * page_string(d,NEWBIE_NOTE,1);
         */
        for( i = 0; newbie_note[i]; i++ ) {
            SEND_TO_Q(newbie_note[i], d);
        }
        STATE(d) = CON_RMOTD;
        send_to_char(motd, d->character);
        SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
        STATE(d) = CON_RMOTD;
        break;

    case CON_MCLASS:
        arg = skip_spaces(arg);
        d->character->specials.remortclass = 0;

        if (arg && (pick = atoi(arg)) &&
            HasClass(d->character, pc_num_class(pick))) {
            d->character->specials.remortclass = pick + 1;
            STATE(d) = CON_CREATION_MENU;
            show_menu(d);
        } else {
            SEND_TO_Q("\n\rInvalid class picked.\n\r", d);
            d->character->specials.remortclass = 0;
            SEND_TO_Q("\n\rSelect your main class from the options below.\n\r",
                      d);
            for (chosen = 0; chosen <= NECROMANCER_LEVEL_IND; chosen++) {
                if (HasClass(d->character, pc_num_class(chosen))) {
                    sprintf(bufx, "[%2d] %s\n\r", chosen, classes[chosen].name);
                    send_to_char(bufx, d->character);
                }
            }
            SEND_TO_Q("\n\rMain Class :", d);
            STATE(d) = CON_MCLASS;
        }
        break;

    case CON_QCLASS:
        /*
         * skip whitespaces
         */

        arg = skip_spaces(arg);
        if( !arg ) {
            SEND_TO_Q("Invalid selection!\n\r", d);
            show_class_selection(d, GET_RACE(d->character));
            SEND_TO_Q("Enter ? for help.\n\r", d);
            SEND_TO_Q("\n\rClass :", d);
            return;
        }

        d->character->player.class = 0;
        count = 0;
        oops = FALSE;

        switch (*arg) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            switch (GET_RACE(d->character)) {
            case RACE_AVARIEL:
                ii = 0;
                while (d->character->player.class == 0 &&
                       avariel_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = avariel_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_GOLD_ELF:
                ii = 0;
                while (d->character->player.class == 0 &&
                       gold_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = gold_elf_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_WILD_ELF:
                ii = 0;
                while (d->character->player.class == 0 &&
                       wild_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = wild_elf_class_choice[ii];
                    }
                    ii++;
                }
                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else
                    show_class_selection(d, GET_RACE(d->character));
                break;

            case RACE_SEA_ELF:
                ii = 0;
                while (d->character->player.class == 0 &&
                       sea_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = sea_elf_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_MOON_ELF:
                ii = 0;
                while (d->character->player.class == 0 &&
                       moon_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = moon_elf_class_choice[ii];
                    }
                    ii++;
                }
                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HUMAN:
                ii = 0;
                while (d->character->player.class == 0
                       && human_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = human_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HALFLING:
                ii = 0;
                while (d->character->player.class == 0
                       && halfling_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = halfling_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_FOREST_GNOME:
                ii = 0;
                while (d->character->player.class == 0 &&
                       forest_gnome_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class =
                            forest_gnome_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_DEEP_GNOME:
            case RACE_ROCK_GNOME:
                ii = 0;
                while (d->character->player.class == 0 &&
                       rock_gnome_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class =
                            rock_gnome_class_choice[ii];
                    }
                    ii++;
                }

                if ((d->character->player.class != 0)) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_DWARF:
                ii = 0;
                while (d->character->player.class == 0
                       && dwarf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = dwarf_class_choice[ii];
                    }
                    ii++;
                }
                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HALF_ELF:
                ii = 0;
                while (d->character->player.class == 0
                       && half_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = half_elf_class_choice[ii];
                    }
                    ii++;
                }
                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HALF_OGRE:
                ii = 0;
                while (d->character->player.class == 0
                       && half_ogre_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = half_ogre_class_choice[ii];
                    }
                    ii++;
                }
                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HALF_GIANT:
                ii = 0;
                while (d->character->player.class == 0
                       && half_giant_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class =
                            half_giant_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_HALF_ORC:
                ii = 0;
                while (d->character->player.class == 0
                       && half_orc_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = half_orc_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_ORC:
                ii = 0;
                while (d->character->player.class == 0
                       && orc_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = orc_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_GOBLIN:
                ii = 0;
                while (d->character->player.class == 0
                       && goblin_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = goblin_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_DROW:
                ii = 0;
                while (d->character->player.class == 0
                       && dark_elf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = dark_elf_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_DARK_DWARF:
                ii = 0;
                while (d->character->player.class == 0
                       && dark_dwarf_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class =
                            dark_dwarf_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            case RACE_TROLL:
                ii = 0;
                while (d->character->player.class == 0
                       && troll_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = troll_class_choice[ii];
                    }
                    ii++;
                }

                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;

            default:
                ii = 0;
                while (d->character->player.class == 0
                       && default_class_choice[ii] != 0) {
                    if (atoi(arg) == ii) {
                        d->character->player.class = default_class_choice[ii];
                    }
                    ii++;
                }
                if (d->character->player.class != 0) {
                    if (!HasClass(d->character, CLASS_MAGIC_USER)) {
                        STATE(d) = CON_CREATION_MENU;
                        show_menu(d);
                    }
                } else {
                    show_class_selection(d, GET_RACE(d->character));
                }
                break;
            }
            break;

        case '?':
            for( i = 0; class_help[i]; i++ ) {
                SEND_TO_Q(class_help[i], d);
            }
            SEND_TO_Q("\n\rSelect your class now.\n\r", d);
            show_class_selection(d, GET_RACE(d->character));
            SEND_TO_Q("Enter ? for help.\n\r", d);
            SEND_TO_Q("\n\rClass :", d);
            break;

        default:
            SEND_TO_Q("Invalid selection!\n\r", d);
            show_class_selection(d, GET_RACE(d->character));
            SEND_TO_Q("Enter ? for help.\n\r", d);
            SEND_TO_Q("\n\rClass :", d);
            break;
        }

        if (HasClass(d->character, CLASS_MAGIC_USER)) {
            for( i = 0; ru_sorcerer[i]; i++ ) {
                SEND_TO_Q(ru_sorcerer[i], d);
            }
            STATE(d) = CON_CHECK_MAGE_TYPE;
            break;
        }

        break;

    case CON_AUTH:
        if (d->character->generic >= NEWBIE_START) {
            /*
             ** now that classes are set, initialize
             */
            init_char(d->character);
            /*
             * create an entry in the file
             */
            d->pos = create_entry(GET_NAME(d->character));
            save_char(d->character, AUTO_RENT);
            send_to_char(motd, d->character);
            SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
            STATE(d) = CON_RMOTD;
        } else if (d->character->generic >= NEWBIE_REQUEST) {
            sprintf(buf, "%s [%s] new player.", GET_NAME(d->character),
                    d->host);
            log_sev(buf, 1);
            if (!strncmp(d->host, "128.197.152", 11)) {
                d->character->generic = 1;
            }
            /*
             * I decided to give them another chance.  -Steppenwolf
             * They blew it. -DM
             */
            if (!strncmp(d->host, "oak.grove", 9) ||
                !strncmp(d->host, "143.195.1.20", 12)) {
                d->character->generic = 1;
            } else {
                if (top_of_p_table > 0) {
                    sprintf(buf, "Type Authorize %s [Yes | No | Message]",
                            GET_NAME(d->character));
                    log_sev(buf, 1);
                    log_sev("type 'Wizhelp Authorize' for other commands", 1);
                } else {
                    Log("Initial character.  Authorized Automatically");
                    d->character->generic = NEWBIE_START + 5;
                }
            }
            /*
             **  enough for gods.  now player is told to shut up.
             */
            /* NEWBIE_START == 3 == 3 chances */
            d->character->generic--;
            sprintf(buf, "Please wait. You have %d requests remaining.\n\r",
                    d->character->generic);
            SEND_TO_Q(buf, d);
            if (d->character->generic == 0) {
                SEND_TO_Q("Goodbye.", d);
                STATE(d) = CON_WIZLOCK;
                break;
            } else {
                SEND_TO_Q("Please Wait.\n\r", d);
                STATE(d) = CON_AUTH;
            }
        } else {
            STATE(d) = CON_WIZLOCK;
        }
        break;

    case CON_CHECK_MAGE_TYPE:
        arg = skip_spaces(arg);
        if (arg && !strcmp(arg, "yes")) {
            d->character->player.class -= CLASS_MAGIC_USER;
            d->character->player.class += CLASS_SORCERER;
        }
        STATE(d) = CON_CREATION_MENU;
        show_menu(d);
        break;

    case CON_RMOTD:
        /*
         * read CR after printing motd
         */
        if (IS_IMMORTAL(d->character)) {
            send_to_char(wmotd, d->character);
#if 0
            SEND_TO_Q(wmotd, d);
#endif
            SEND_TO_Q("\n\r\n[PRESS RETURN]", d);
            STATE(d) = CON_WMOTD;
            break;
        }
        if (d->character->term != 0) {
            ScreenOff(d->character);
        }
        send_to_char(MENU, d->character);

        STATE(d) = CON_SLCT;
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || SiteLock(d->host)) &&
            !IS_IMMORTAL(d->character)) {
            sprintf(buf, "Sorry, the game is locked up for repair or your "
                         "site is banned.\n\r");
            SEND_TO_Q(buf, d);
            STATE(d) = CON_WIZLOCK;
        }
        break;

    case CON_WMOTD:
        /*
         * read CR after printing motd
         */
        send_to_char(MENU, d->character);

        STATE(d) = CON_SLCT;
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || SiteLock(d->host)) &&
            !IS_IMMORTAL(d->character)) {
            sprintf(buf, "Sorry, the game is locked up for repair or your site"
                         " is banned.\n\r");
            SEND_TO_Q(buf, d);
            STATE(d) = CON_WIZLOCK;
        }
        break;

    case CON_WIZLOCK:
        close_socket(d);
        break;

    case CON_DELETE_ME:
        arg = skip_spaces(arg);
        if (arg && !strcmp(arg, "yes") && 
            strcmp("Guest", GET_NAME(d->character))) {
            sprintf(buf, "%s just killed theirself!", GET_NAME(d->character));
            Log(buf);
            for (i = 0; i <= top_of_p_table; i++) {
                if (!strcasecmp((player_table + i)->name,
                             GET_NAME(d->character))) {
                    if ((player_table + i)->name) {
                        free((player_table + i)->name);
                    }
                    (player_table + i)->name =
                        (char *) malloc(strlen("111111"));
                    strcpy((player_table + i)->name, "111111");
                    break;
                }
            }

            /*
             * get the structure from player_table[i].nr
             */
            if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
                perror("Opening player file for updating. (interpreter.c,"
                       " nanny)");
                assert(0);
            }
            rewind(char_file);
            fseek(char_file, (long) (player_table[i].nr *
                                     sizeof(struct char_file_u)), 0);

            /*
             * read in the char, change the name, write back
             */
            fread(&ch_st, sizeof(struct char_file_u), 1, char_file);
            sprintf(ch_st.name, "111111");
            rewind(char_file);
            fseek(char_file, (long) (player_table[i].nr *
                                     sizeof(struct char_file_u)), 0);
            fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
            fclose(char_file);
            sprintf(buf, "rent/%s", lower(GET_NAME(d->character)));
            remove(buf);
            sprintf(buf, "rent/%s.aux", GET_NAME(d->character));
            remove(buf);
            close_socket(d);
        }
        send_to_char(MENU, d->character);

        STATE(d) = CON_SLCT;
        break;

    case CON_SLCT:
        /* get selection from main menu */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if(!arg) {
            SEND_TO_Q("Wrong option.\n\r", d);
            send_to_char(MENU, d->character);
            break;
        }
        switch (*arg) {
        case '0':
            close_socket(d);
            break;

        case '1':
            reset_char(d->character);
            total_connections++;
            if (!IS_IMMORTAL(d->character) || d->character->invis_level <= 58) {
                sprintf(buf, "Loading %s's equipment",
                        d->character->player.name);
                Log(buf);
            }

            count_players = 1;
            for (desc = descriptor_list; desc; desc = desc->next) {
                if (!desc->connected) {
                    count_players++;
                }
            }

            if (total_max_players < count_players) {
                total_max_players = count_players;
            }
            load_char_objs(d->character);

            save_char(d->character, AUTO_RENT);
            send_to_char(WELC_MESSG, d->character);
            d->character->next = character_list;
            character_list = d->character;
            if (d->character->in_room == NOWHERE ||
                d->character->in_room == AUTO_RENT) {
                if (!IS_IMMORTAL(d->character)) {
                    if (d->character->specials.start_room <= 0) {
                        if (GET_RACE(d->character) == RACE_HALFLING) {
                            char_to_room(d->character, 1103);
                            d->character->player.hometown = 1103;
                        } else {
                            char_to_room(d->character, 3001);
                            d->character->player.hometown = 3001;
                        }
                    } else {
                        char_to_room(d->character,
                                     d->character->specials.start_room);
                        d->character->player.hometown =
                            d->character->specials.start_room;
                    }
                } else {
                    if (d->character->specials.start_room <= NOWHERE) {
                        char_to_room(d->character, 1000);
                        d->character->player.hometown = 1000;
                    } else {
                        if (real_roomp(d->character->specials.start_room)) {
                            char_to_room(d->character,
                                         d->character->specials.start_room);
                            d->character->player.hometown =
                                d->character->specials.start_room;
                        } else {
                            char_to_room(d->character, 1000);
                            d->character->player.hometown = 1000;
                        }
                    }
                }
            } else if (real_roomp(d->character->in_room)) {
                char_to_room(d->character, d->character->in_room);
                d->character->player.hometown = d->character->in_room;
            } else {
                char_to_room(d->character, 3001);
                d->character->player.hometown = 3001;
            }

            d->character->specials.tick = plr_tick_count++;
            if (plr_tick_count == PLR_TICK_WRAP) {
                plr_tick_count = 0;
            }
            act("$n has entered the game.", TRUE, d->character, 0, 0, TO_ROOM);
            STATE(d) = CON_PLYNG;
            if (!GetMaxLevel(d->character)) {
                do_start(d->character);
            }
            do_look(d->character, NULL, 15);
            /*
             * do an auction check, grant reimbs as needed
             */
            if (d->character->specials.auction) {
                obj = d->character->specials.auction;
                d->character->specials.auction = 0;
                obj->equipped_by = 0;
                obj->eq_pos = -1;

                obj_to_char(obj, d->character);
                send_to_char("Your item is returned to you.\n\r", d->character);
                do_save(d->character, "", 0);
            }
            if (d->character->specials.minbid) {
                GET_GOLD(d->character) += d->character->specials.minbid;
                d->character->specials.minbid = 0;
                send_to_char("You are returned your deposit for this "
                             "auction.\n\r", d->character);
                do_save(d->character, "", 0);
            }
            d->prompt_mode = 1;
            break;

        case '2':
            SEND_TO_Q("Enter a text you'd like others to see when they look "
                      "at you.\n\r", d);
            SEND_TO_Q("Terminate with a '~'.\n\r", d);
            if (d->character->player.description) {
                SEND_TO_Q("Old description :\n\r", d);
                SEND_TO_Q(d->character->player.description, d);
                if (d->character->player.description) {
                    free(d->character->player.description);
                }
                d->character->player.description = 0;
            }
            d->str = &d->character->player.description;
            d->max_str = 240;
            STATE(d) = CON_EXDSCR;
            SEND_TO_Q("<type /w to save.>\n\r", d);
            break;

        case '3':
            SEND_TO_Q(STORY, d);
            STATE(d) = CON_RMOTD;
            break;

        case '4':
            SEND_TO_Q("Enter a new password: ", d);
            write(d->descriptor, echo_off, 4);
            STATE(d) = CON_PWDNEW;
            break;

        case 'K':
        case 'k':
            SEND_TO_Q("Are you sure you want to delete yourself? (yes/no) ", d);
            STATE(d) = CON_DELETE_ME;
            break;

        default:
            SEND_TO_Q("Wrong option.\n\r", d);
            send_to_char(MENU, d->character);
            break;
        }
        break;

    case CON_PWDNEW:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            write(d->descriptor, echo_on, 6);

            SEND_TO_Q("Illegal password.\n\r", d);
            SEND_TO_Q("Password: ", d);

            write(d->descriptor, echo_off, 4);

            return;
        }

        strncpy(d->pwd, (char *) crypt(arg, d->character->player.name), 10);
        *(d->pwd + 10) = '\0';
        write(d->descriptor, echo_on, 6);

        SEND_TO_Q("Please retype password: ", d);

        STATE(d) = CON_PWDNCNF;
        write(d->descriptor, echo_off, 4);
        break;

    case CON_EXDSCR:
        send_to_char(MENU, d->character);
        STATE(d) = CON_SLCT;
        break;

    case CON_PWDNCNF:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            write(d->descriptor, echo_on, 6);
            SEND_TO_Q("Passwords don't match.\n\r", d);
            SEND_TO_Q("Retype password: ", d);
            write(d->descriptor, echo_off, 4);

            STATE(d) = CON_PWDNEW;
            return;
        }
        write(d->descriptor, echo_on, 6);

        SEND_TO_Q("\n\rDone. You must enter the game to make the change "
                  "final\n\r", d);
        send_to_char(MENU, d->character);

        STATE(d) = CON_SLCT;
        break;

    default:
        sprintf(buf, "Nanny: illegal state of con'ness (%d)", STATE(d));
        Log(buf);
        abort();
        break;
    }
}

void show_class_selection(struct descriptor_data *d, int r)
{
    int             i = 0;
    char            buf[254],
                    buf2[254];
    extern char    *pc_class_types[];

    switch (r) {
    case RACE_AVARIEL:
        for (i = 0; avariel_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) avariel_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_GOLD_ELF:
        for (i = 0; gold_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) gold_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_WILD_ELF:
        for (i = 0; wild_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) wild_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_SEA_ELF:
        for (i = 0; sea_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) sea_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_MOON_ELF:
        for (i = 0; moon_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) moon_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_FOREST_GNOME:
        for (i = 0; forest_gnome_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);

            sprintbit((unsigned) forest_gnome_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_DEEP_GNOME:
    case RACE_ROCK_GNOME:
        for (i = 0; rock_gnome_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) rock_gnome_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_DWARF:
        for (i = 0; dwarf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) dwarf_class_choice[i], pc_class_types,
                      buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HALFLING:
        for (i = 0; halfling_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) halfling_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HUMAN:
        for (i = 0; human_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) human_class_choice[i], pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HALF_ELF:
        for (i = 0; half_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) half_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HALF_ORC:
        for (i = 0; half_orc_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) half_orc_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HALF_OGRE:
        for (i = 0; half_ogre_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) half_ogre_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_HALF_GIANT:
        for (i = 0; half_giant_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) half_giant_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_ORC:
        for (i = 0; orc_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) orc_class_choice[i], pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_GOBLIN:
        for (i = 0; goblin_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) goblin_class_choice[i], pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_DROW:
        for (i = 0; dark_elf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) dark_elf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_DARK_DWARF:
        for (i = 0; dark_dwarf_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) dark_dwarf_class_choice[i],
                      pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    case RACE_TROLL:
        for (i = 0; troll_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) troll_class_choice[i], pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;

    default:
        for (i = 0; default_class_choice[i] != 0; i++) {
            sprintf(buf, "%d) ", i);
            sprintbit((unsigned) default_class_choice[i], pc_class_types, buf2);
            strcat(buf, buf2);
            strcat(buf, "\n\r");
            SEND_TO_Q(buf, d);
        }
        break;
    }
}

void show_race_choice(struct descriptor_data *d)
{
    int             ii,
                    i = 0;
    char            buf[255],
                    buf2[254];

    SEND_TO_Q("                                  Level Limits\n\r", d);
    sprintf(buf, "%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s "
                 "%-3s %-3s %-3s\n\r",
            "#", "Race", "ma", "cl", "wa", "th", "dr", "mk", "ba", "so",
            "pa", "ra", "ps", "ne");
    SEND_TO_Q(buf, d);

    while (race_choice[i] != -1) {
        sprintf(buf, "$c000W%-3d)$c0007 %-15s", i, RaceName[race_choice[i]]);

        /*
         * show level limits
         */
        for (ii = 0; ii < MAX_CLASS; ii++) {
            sprintf(buf2, " %-3d", RacialMax[race_choice[i]][ii]);
            strcat(buf, buf2);
        }

        strcat(buf, "\n\r");
        if (i == 13) {
            strcat(buf, "$c000WThe Races Listed below may have some racials"
                        " hatreds. Advanced players only.\n\r");
        }
        send_to_char(buf, d->character);
        i++;
    }

    send_to_char("$c000gma=magic user, cl=cleric, wa=warrior,th=thief,"
                 "dr=druid,mk=monk\n\r", d->character);
    send_to_char("$c000gba=barbarian,so=sorcerer,pa=paladin,ra=ranger,ps=psi,"
                 "ne=necromancer\n\r\n\r", d->character);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
