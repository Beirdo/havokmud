#ifndef _config_h
#define _config_h

#define _GNU_SOURCE
#define __EXTENSIONS__

#ifdef __CYGWIN__
#define __SAVE_ANSI__ __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

/**********************************************************
 *      COMPILE TIME DEFINES (GAMEFLAGS)
 */

/* For lower exp loss at lower levels */
#define NEWEXP

/* For level loss for really low exp */
#define LEVEL_LOSS

/* to use hash tables for rooms  (More cpu, less storage) */
#undef HASH    

/* To get rid of tracking (lower cpu) */
#undef NOTRACK

/* To limit items above a specific rent cost */
#define LIMITED_ITEMS

/* To force new players to be authorized */
#undef PLAYER_AUTH

/* To add some new debugging stuff (mostly limited to malloc_debug) */
#undef DEBUG

/* To add some commands for locking out specific hosts */
#define SITELOCK

/* To lock out oak.grove.iup.edu from boot time. */
#undef LOCKGROVE

/* To modify some commands to make item duplication near impossible: */
#define NODUPLICATES

/* To have each 58+ god restricted to one site, or set of sites */
#undef IMPL_SECURITY

/* To use BSD memory allocation/deallocation routines instead of the 
 * native memory routines:
 */
#undef KLUDGE_MEM

/* If your system doesn't have built in functions of strdup() and strstr() */
#undef KLUDGE_STRING

/* If you want to use a faster mana/hit/move regen system. */
#define NEWGAIN

/* If you want to save every room and mob items. */
#undef SAVEWORLD

/* If you want to use the Quest to Gain system. Not all classes */
#undef QUEST_GAIN

/* If you want to use a lower gold ration for your game */
#define LOW_GOLD

/* Check ego on give and gets */
#define USE_EGOS

/* logs all mob command sent to command_interpreter */
#undef LOG_MOB


/* This will add a bit more info to the logs */
#undef LOG_DEBUG

/* Forces the mud to remove outdated or inactive players and demote inactive 
 * gods a boot up. Slows boot process down a bit.
 */
#undef CLEAN_AT_BOOT

/* When used in combination with CLEAN_AT_BOOT it will clear out corrupted 
 * user files. Do this once or twice a month if needed, otherwise do not use 
 * it.
 */
#undef STRANGE_WACK

/* Enabling this will force all communications to be limited to the zone that
 * person is in. Making gossip/auction/tell NOT GLOBAL. Tell is limited to 
 * people in the zone you are currently standing in. This does NOT include 
 * SHOUT.
 */
#define ZONE_COMM_ONLY

/* This define should make mobiles lag with commands as pc's do. I.e. bashed 
 * pause, etc...
 */
#define LAG_MOBILES                

/* This define will add in a few checks in fight.c to keep pc's from killing 
 * each other, willing or Un-willing.
 */
#define PREVENT_PKILL

/* Check user inactivity against RENT_INACTIVE, if greater, wack the rent file.
 * RENT_INACTIVE located in db.c
 */
#undef CHECK_RENT_INACTIVE

/* My personal annoying people I lock out perminatly. You can add whomever is
 * annoying in there.
 */
#define PERSONAL_PERM_LOCKOUTS

/* Charge 100 coins per day of rent, regardless of item types. */
#undef NEW_RENT

/* Needs a desctription */
#define PREP_SPELLS

/* Log all actions */
#define LOG_ALL

/* Limit monks to 20 items */
#undef MONK_ITEM_LIMIT

#undef BYTE_COUNT
#define OLD_ZONE_STUFF          /* for temp testing of reset_zon */
#undef N_SAVE_WORLD
#undef TITAN

#else

#error you missed a spot, config.h included twice

#endif

