#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"
#include "externs.h"

#define MAX_CMD_LIST 497
char *get_argument_common(char *line_in, char **arg_out, int do_fill,
                          char delim);


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
#if 0
    { "imptest", do_imptest, 249, POSITION_SITTING, 60 },
#endif
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
#if 0
    { "orebuild", do_orebuild, 566, POSITION_STANDING, 59 },
#endif
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
int commandCount = NELEMENTS(commandList);


unsigned char   echo_on[] = { IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0' };
unsigned char   echo_off[] = { IAC, WILL, TELOPT_ECHO, '\0' };
int             Silence = 0;
int             plr_tick_count = 0;
int             MAX_NAME_LENGTH = 11;
char           *Sex[] = { "Neutral", "Male", "Female" };


int             pc_num_class(int clss);
void show_class_selection(struct descriptor_data *d, int r);


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

int find_direction(char *arg)
{
    int length;
    int i;

    length = strlen( arg );
    if( length == 0 ) {
        return( -1 );
    }

    for( i = 0; i < directionCount; i++ ) {
        if( !strncasecmp( arg, direction[i].dir, length ) ) {
            return( i );
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
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
