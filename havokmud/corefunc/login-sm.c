/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005 Gavin Hurlbut
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
 * Copyright 2005 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Login & character creation state machine
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include "memory.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>
#include "protected_data.h"

#include "structs.h"
#include "utils.h"
#include "externs.h"
#include "interthread.h"

static char ident[] _UNUSED_ =
    "$Id$";


#define MAX_NAME_LENGTH 11

void EnterState(PlayerStruct_t *player, PlayerState_t newstate);
void show_race_choice(PlayerStruct_t *player);
void show_class_selection(PlayerStruct_t *player, int r);
void show_menu(PlayerStruct_t *player);
void DoCreationMenu( PlayerStruct_t *player, char arg );
void roll_abilities(PlayerStruct_t *player);
int SiteLock(char *site);

static char     swords[] = ">>>>>>>>";  /**< Used with STAT_SWORD to show 
                                             stats */

char           *login = NULL;
char           *credits = NULL;
char           *motd = NULL;
char           *wmotd = NULL;
char           *news = NULL;
char           *info = NULL;


/**
 * Shows from 1 to 8 ">" to represent stats from 1 through 18
 */
#define STAT_SWORD(x) (((x)<18 && (x)>0) ? &(swords[5-(((x)-1)/3)]) : "ERR!")

/**
 * Telnet sequence to turn on echo
 */
unsigned char   echo_on[] = { IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0' };
unsigned char   echo_off[] = { IAC, WILL, TELOPT_ECHO, '\0' };
char           *Sex[] = { "Neutral", "Male", "Female" };

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
    " 3) PLEASE do not curse over public channels including GOSSIP, SHOUT,\n\r"
    "    etc.  Punishment for doing so begins with removing your\n\r"
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

#if 1
const struct pc_race_choice race_choice[1];
int race_choice_count;
#endif


/**
 * @brief Trims the leading spaces and copies the name
 * @param arg input string
 * @param name output name
 * @return 1 if the name is invalid, 0 if valid
 *
 * An invalid name is indicated if the name is too long, or has non-alpha 
 * characters in it, or is empty
 */
int _parse_name(char *arg, char *name)
{
    int             i;

    /*
     * skip whitespaces
     */
    arg = skip_spaces(arg);
    if( !arg ) {
        return( 1 );
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

/**
 * @brief Checks the username against the banned user list
 * @param name the provided username
 * @return 1 if the name is banned or is too long, 0 if it's OK
 * @todo perhaps change to use strstr instead of many strcasecmp
 *
 * The banned user list contains profanities and other things that are not
 * desirable in a player name on a MUD.  If a full match or a partial match is
 * found (as dictated by the banned user list per banned user name), the name 
 * will be rejected
 */
int _check_ass_name(char *name)
{
    /*
     * 0 - full match
     * 1 - from start of string
     * 2 - from end of string
     * 3 - somewhere in string
     */
    int             i,
                    j;

    if (strlen(name) > MAX_NAME_LENGTH) {
        return( 1 );
    }

    if (!bannedUsers) {
        return( 0 );
    }

    for (i = 0; i < bannedUserCount; i++) {
        switch (bannedUsers[i].how) {
        case 0:
            /* Exact match */
            if (!strcasecmp(name, bannedUsers[i].name)) {
                return( 1 );
            }
            break;

        case 1:
            /* match to the beginning of the string */
            if (!strncasecmp(name, bannedUsers[i].name, bannedUsers[i].len) ) {
                return( 1 );
            }
            break;

        case 2:
            /* Match to the end of the string */
            if (strlen(name) < bannedUsers[i].len) {
                break;
            }

            if (!strcasecmp(&name[strlen(name) - bannedUsers[i].len],
                            bannedUsers[i].name)) {
                return( 1 );
            }
            break;

        case 3:
            /* Metch to the middle of the string */
            if (strlen(name) < bannedUsers[i].len) {
                break;
            }
            
            for (j = 0; j <= strlen(name) - bannedUsers[i].len; j++) {
                if (!strncasecmp(&name[j], bannedUsers[i].name,
                                 bannedUsers[i].len)) {
                    return( 1 );
                }
            }
            break;

        default:
            LogPrintNoArg(LOG_NOTICE, "Grr! invalid value in bannedUsers, "
                                      "interpreter.c _check_ass_name");
            return( 1 );
        }
    }

    return( 0 );
}


void show_menu(PlayerStruct_t *player)
{
    struct char_data   *ch;
    int                 bit;
    char                cls[50];

    ch = player->charData;


    SendOutput(player, "$c0009-=$c0015Havok Character Creation Menu [%s]"
                       "$c0009=-\n\r\n\r", GET_NAME(ch));
    SendOutput(player, "$c00151) $c0012Gender. [$c0015%s$c0012]\n\r",
                       Sex[((int) GET_SEX(ch))]);
    SendOutput(player, "$c00152) $c0012ANSI Colors.\n\r");

    if (GET_RACE(ch) == 0) {
        /*
         * make default race to Human rather than half-breed
         */
        GET_RACE(ch) = 1;
    }
#if 0
    SendOutput(player, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                           races[GET_RACE(ch)].racename);
#endif

    cls[0] = '\0';
    for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
        if (HasClass(ch, pc_num_class(bit))) {
            strcat(cls, classes[bit].abbrev);
        }
    }
    if (!(strcmp(cls, ""))) {
        sprintf(cls, "None Selected");
    }
    SendOutput(player, "$c00154) $c0012Class.[$c0015%s$c0012]\n\r", cls);

    cls[0] = '\0';
    if (ch->specials.remortclass) {
       /*
        * remort == 0 means none picked
        */
        strcat(cls, classes[(ch->specials.remortclass - 1)].abbrev);
    }
    if (!(strcmp(cls, ""))) {
        sprintf(cls, "None Selected");
    }
    SendOutput(player, "$c00155) $c0012Main Class.[$c0015%s$c0012]\n\r", cls );

    if (GET_CON(ch) == 0) {
        SendOutput(player, "$c00156) $c0012Character Stats.[$c0015None "
                           "Picked$c0012]\n\r");
    } else {
        SendOutput(player, "$c00156) $c0012Character Stats.[$c0015Done$c0012]"
                           "\n\r");
    }

    SendOutput(player, "$c00157) $c0012Alignment.[$c000W%s$c000B]\n\r\n\r",
                       (GET_ALIGNMENT(ch) ?
                       AlignDesc(GET_ALIGNMENT(ch)) : "None"));

    SendOutput(player, "$c0015D) $c0012Done!\n\r\n\r");
    SendOutput(player, "$c0011Please pick an option: \n\r");
}

void EnterState(PlayerStruct_t *player, PlayerState_t newstate)
{
    struct char_data   *ch;
    int                 chosen = 0;
    int                 i;

    if( !player ) {
        return;
    }

    ch = player->charData;

    LogPrint( LOG_INFO, "Entering state %d from %d", newstate, player->state );

    switch( newstate ) {
    case STATE_CHOOSE_SEX:
        SendOutput(player, "What is your sex (M/F) ? ");
        break;
    case STATE_CHOOSE_ANSI:
        SendOutput(player, "Would you like ansi colors? (Yes or No)");
        break;
    case STATE_CHOOSE_RACE:
        show_race_choice(player);
        SendOutput(player, "For help type '?'- will list level limits. \n\r"
                           " RACE:  ");
        ch->player.class = 0;
        ch->specials.remortclass = 0;
        break;
    case STATE_CHOOSE_CLASS:
        GET_ALIGNMENT(ch) = 0;
        GET_CON(ch) = 0;
        SendOutput(player, "\n\rSelect your class now.\n\r");
        show_class_selection(player, GET_RACE(ch));
        SendOutput(player, "Enter ? for help.\n\r\n\rClass :");
        break;
    case STATE_CHOOSE_MAIN_CLASS:
        SendOutput(player, "\n\rSelect your main class from the options "
                           "below.\n\r");

        for (chosen = 0; chosen <= NECROMANCER_LEVEL_IND; chosen++) {
            if (HasClass(ch, pc_num_class(chosen))) {
                SendOutput(player, "[%2d] %s\n\r", chosen + 1,
                                   classes[chosen].name);
            }
        }
        SendOutput(player, "\n\rMain Class :");
        break;
    case STATE_CHOOSE_STATS:
        SendOutput(player, "\n\rSelect your stat priority, by listing them from"
                           " highest to lowest\n\r"
                           "Separated by spaces.. don't duplicate\n\r"
                           "for example: 'S I W D Co Ch' would put the highest"
                           " roll in Strength, \n\r"
                           "next in intelligence, Wisdom, Dex, Con, and lastly"
                           " charisma\n\r"
                           "Your choices? ");
        break;
    case STATE_CHOOSE_ALIGNMENT:
        SendOutput(player, "Your alignment is an indication of how well or "
                           "badly you have morally\n\r"
                           "conducted yourself in the game. It ranges "
                           "numerically, from -1000\n\r"
                           "($c000RChaotic Evil$c000w) to 1000 ($c000WLawful "
                           "Good$c000w), 0 being neutral. Generally, if you "
                           "kill\n\r"
                           "'Good' mobs, you will gravitate towards Evil, and "
                           "vice-versa. Some spells\n\r"
                           "and skills also affect your alignment. ie Backstab "
                           "makes you evil, and\n\r"
                           "the spell heal makes you good\n\r");

        if (HasClass(ch, CLASS_PALADIN)) {
            SendOutput(player, "Please select your alignment "
                               "($c000WGood$c000w)");
        } else if (HasClass(ch, CLASS_DRUID)) {
            SendOutput(player, "Please select your alignment (Neutral)");
        } else if (HasClass(ch, CLASS_NECROMANCER)) {
            SendOutput(player, "Please select your alignment "
                               "($c000REvil$c000w)");
        } else if (HasClass(ch, CLASS_RANGER)) {
            SendOutput(player, "Please select your alignment "
                               "($c000WGood$c000w/Neutral$c000w)");
        } else {
            SendOutput(player, "Please select your alignment "
                               "($c000WGood$c000w/Neutral$c000w/"
                               "$c000REvil$c000w)");
        }
        break;

    case STATE_SHOW_MOTD:
        SendOutput(player, motd);
        SendOutput(player, "\n\r\n*** PRESS RETURN: ");
        break;

    case STATE_SHOW_CREATION_MENU:
        show_menu(player);
        break;

    case STATE_GET_PASSWORD:
        SendOutput(player, "Password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_CONFIRM_NAME:
        SendOutput(player, "Did I get that right, %s (Y/N)? ", GET_NAME(ch));
        break;
    case STATE_GET_NEW_USER_PASSWORD:
        SendOutput(player, "Give me a password for %s: ", GET_NAME(ch));
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_GET_NAME:
        if (GET_NAME(ch)) {
            memfree(GET_NAME(ch));
            GET_NAME(ch) = NULL;
        }
        SendOutput(player, "What is thy name? ");
        break;
    case STATE_PLAYING:
        break;
    case STATE_CONFIRM_PASSWORD:
    case STATE_CONFIRM_NEW_PASSWORD:
        SendOutput(player, "Please retype password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_REROLL:
        SendOutput(player, "Your current stats are:\n\r");
        SendOutput(player, "STR: -]%s\n\r", STAT_SWORD(GET_STR(ch)));
        SendOutput(player, "CON: -]%s\n\r", STAT_SWORD(GET_CON(ch)));
        SendOutput(player, "DEX: -]%s\n\r", STAT_SWORD(GET_DEX(ch)));
        SendOutput(player, "INT: -]%s\n\r", STAT_SWORD(GET_INT(ch)));
        SendOutput(player, "WIS: -]%s\n\r", STAT_SWORD(GET_WIS(ch)));
        SendOutput(player, "CHR: -]%s\n\r", STAT_SWORD(GET_CHR(ch)));
        SendOutput(player, "\n\rYou have %d rerolls left, press R to reroll, "
                           "any other key to keep.\n\r", ch->reroll);
        break;
    case STATE_CHECK_MAGE_TYPE:
        for( i = 0; ru_sorcerer[i]; i++ ) {
            SendOutput(player, ru_sorcerer[i]);
        }
        break;
    case STATE_WAIT_FOR_AUTH:
        SendOutput(player, "Please Wait for authorization.\n\r");
        break;
    case STATE_WIZLOCKED:
        SendOutput(player, "Goodbye.\n\r");
        break;
    case STATE_SHOW_WMOTD:
        SendOutput(player, wmotd);
        SendOutput(player, "\n\r\n[PRESS RETURN]");
        break;
    case STATE_SHOW_LOGIN_MENU:
#if 0
        SendOutput(player, MENU);
#endif
        break;
    case STATE_EDIT_EXTRA_DESCR:
        SendOutput(player, "<type /w to save.>\n\r");
        EditorStart(player, &ch->player.description, 240);
        break;
    case STATE_PRESS_ENTER:
        SendOutput(player, "\n\r<Press enter to continue>");
        break;
    case STATE_GET_NEW_PASSWORD:
        SendOutput(player, "Enter a new password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_DELETE_USER:
        SendOutput(player, "Are you sure you want to delete yourself? "
                           "(yes/no) ");
        break;
    case STATE_INITIAL:
        break;
    }

    player->state = newstate;
}


/**
 * @todo Fix character deletion
 * @todo Fix character load/save
 * @todo Fix reconnection
 */
void LoginStateMachine(PlayerStruct_t *player, char *arg)
{
#if 0
    int             player_i;
#endif
    int             class,
                    race,
                    found,
                    index = 0;
    char            tmp_name[20];
#if 0
    struct char_data *tmp_ch;
#endif
    struct char_data *ch;
    int             i = 0;
    int             tmpi = 0;
    int             pick = 0;
    PlayerStruct_t *oldPlayer;
    InputStateItem_t   *stateItem;

    ch = player->charData;
    SendOutputRaw(player, echo_on, 6);

    switch (player->state) {
    case STATE_INITIAL:
        EnterState(player, STATE_GET_NAME);
        break;

    case STATE_SHOW_CREATION_MENU:
        arg = skip_spaces(arg);
        if( !arg ) {
            show_menu(player);
            SendOutput(player, "Invalid Choice.. Try again..\n\r");
            return;
        }

        DoCreationMenu(player, *arg);
        break;

    case STATE_CHOOSE_ALIGNMENT:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Please select a alignment.\n\r");
            return;
        }

        switch (tolower(*arg)) {
        case 'n':
            if (!HasClass(ch, CLASS_PALADIN) &&
                !HasClass(ch, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(ch) = 1;
                SendOutput(player, "You have chosen to be Neutral in "
                                   "alignment.\n\r\n\r");
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;
        case 'g':
            if (!HasClass(ch, CLASS_DRUID) &&
                !HasClass(ch, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(ch) = 1000;
                SendOutput(player, "You have chosen to be a follower of "
                                   "light.\n\r\n\r");
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;
        case 'e':
            if (!HasClass(ch, CLASS_DRUID) &&
                !HasClass(ch, CLASS_PALADIN) &&
                !HasClass(ch, CLASS_RANGER)) {
                GET_ALIGNMENT(ch) = -1000;
                SendOutput(player, "You have chosen the dark side.\n\r\n\r");
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;

        default:
            SendOutput(player, "Please select a alignment.\n\r");
            break;
        }
        break;

    case STATE_CHOOSE_ANSI:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Please type Yes or No.\n\r"
                               "Would you like ANSI colors? :");
            return;
        }

        switch (tolower(*arg)) {
        case 'y':
            SET_BIT(ch->player.user_flags, USE_ANSI);

            SendOutput(player, "$c0012A$c0010N$c0011S$c0014I$c0007 colors "
                               "enabled.\n\r\n\r");
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;

        case 'n':
            REMOVE_BIT(ch->player.user_flags, USE_ANSI);
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;

        default:
            SendOutput(player, "Please type Yes or No.\n\r"
                               "Would you like ANSI colors? :");
            return;
            break;
        }
        break;

    case STATE_CHOOSE_RACE:
        ch->reroll = 20;
        arg = skip_spaces(arg);
        if (!arg) {
            EnterState(player, STATE_CHOOSE_RACE);
            return;
        } 
        
       if (*arg == '?') {
            for( i = 0; racehelp[i]; i++ ) {
                SendOutput(player, racehelp[i]);
            }
            EnterState(player, STATE_CHOOSE_RACE);
            return;
        } 
       
        tmpi = atoi(arg);
        if (tmpi >= 1 && tmpi <= race_choice_count) {
            GET_RACE(ch) = race_choice[tmpi - 1].raceNum;
            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
            SendOutput(player, "\n\rThat's not a race.\n\rRACE?:");
            EnterState(player, STATE_CHOOSE_RACE);
        }
        break;

    case STATE_GET_NAME:
    /*
     * wait for input of name
     */
        if (!ch) {
            player->charData = CREATE(struct char_data);
            ch = player->charData;
#if 0
            clear_char(ch);
#endif
            ch->playerDesc = player;
        }

        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection );
            return;
        } 
        
        if (_parse_name(arg, tmp_name)) {
            SendOutput(player, "Illegal name, please try another.\r\n");
            SendOutput(player, "Name: ");
            return;
        }

        ProtectedDataLock(player->connection->hostName);
        if (SiteLock((char *)player->connection->hostName->data)) {
            ProtectedDataUnlock(player->connection->hostName);
            SendOutput(player, "Sorry, this site is temporarily banned.\n\r");
            EnterState(player, STATE_WIZLOCKED);
            return;
        }
        ProtectedDataUnlock(player->connection->hostName);

#if 0
        if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
            /*
             * connecting an existing character ...
             */
#if 0
            store_to_char(&tmp_store, player->charData);
#endif
            strcpy(ch->pwd, tmp_store.pwd);
#ifdef TODO
            d->pos = player_table[player_i].nr;
#endif
            EnterState(player, STATE_GET_PASSWORD);
        } else {
#endif
            /*
             * player unknown gotta make a new
             */
            if (IS_SET(SystemFlags, SYS_WIZLOCKED)) {
                SendOutput(player, "Sorry, no new characters at this time\n\r");
                EnterState(player, STATE_WIZLOCKED);
                return;
            }

            if (_check_ass_name(tmp_name)) {
                SendOutput(player, "\n\rIllegal name, please try another.");
                SendOutput(player, "Name: ");
                return;
            }

            /*
             * move forward creating new character
             */
            GET_NAME(ch) = CREATEN(char, strlen(tmp_name) + 1);
            strcpy(GET_NAME(ch), CAP(tmp_name));
            EnterState(player, STATE_CONFIRM_NAME);
#if 0
        }
#endif
        break;

    case STATE_CONFIRM_NAME:
        /*
         * wait for conf. of new name
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if( !arg ) {
            /*
             * Please do Y or N
             */
            SendOutput(player, "Please type Yes or No? ");
            return;
        }

        switch(tolower(*arg)) {
        case 'y':
            SendOutputRaw(player, echo_on, 4);
            SendOutput(player, "New character.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            break;
        case 'n':
            SendOutput(player, "Ok, what IS it, then? ");
            EnterState(player, STATE_GET_NAME);
            break;
        default:
            SendOutput(player, "Please type Yes or No? ");
            break;
        }
        break;

    case STATE_GET_PASSWORD:
        /*
         * get pwd for known player
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection );
            return;
        } 

        if (strncmp((char *) crypt(arg, ch->pwd), ch->pwd, 10)) {
            SendOutput(player, "Wrong password.\n\r");
            LogPrint(LOG_INFO, "%s entered a wrong password", GET_NAME(ch));
            connClose( player->connection );
            return;
        }

#ifdef IMPL_SECURITY
        if (top_of_p_table > 0) {
            if (GetMaxLevel(ch) >= 58) {
                ProtectedDataLock(player->connection->hostName);
                switch (SecCheck(GET_NAME(ch), 
                                 (char *)player->connection->hostName->data)) {
                case -1:
                case 0:
                    SendOutput(player, "Security check reveals invalid site\n\r"
                               "Speak to an implementor to fix problem\n\r"
                               "If you are an implementor, add yourself to"
                               " the\n\r"
                               "Security directory (lib/security)\n\r");
                    ProtectedDataUnlock(player->connection->hostname);
                    connClose( player->connection );
                    break;
                default:
                    ProtectedDataUnlock(player->connection->hostname);
                    break;
                }
            }
        }
#endif

        /*
         * Check if already playing
         */
        oldPlayer = FindCharacterNamed( GET_NAME(ch), player );
        if( oldPlayer ) {
            connClose( oldPlayer->connection );
            break;
        }

        /*
         * Check if disconnected ...
         */
        /**
         * @todo Convert to new LinkedList methodology
         */
#if 0
        for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
            if ((!strcasecmp(GET_NAME(ch), GET_NAME(tmp_ch)) 
                 && !tmp_ch->playerDesc && !IS_NPC(tmp_ch)) ||
                (IS_NPC(tmp_ch) && tmp_ch->orig &&
                 !strcasecmp(GET_NAME(ch), GET_NAME(tmp_ch->orig)))) {

                SendOutputRaw(player, echo_on, 6);
                SendOutput(player, "Reconnecting.\n\r");

#if 0
                free_char(ch);
#endif
                tmp_ch->playerDesc = player;
                player->charData = tmp_ch;
                tmp_ch->specials.timer = 0;

                if (!IS_IMMORTAL(tmp_ch)) {
                    tmp_ch->invis_level = 0;
                }

#ifdef TODO
                if (tmp_ch->orig) {
                    tmp_ch->desc->original = tmp_ch->orig;
                    tmp_ch->orig = 0;
                }
#endif

                ch->persist = 0;

                if (!IS_IMMORTAL(tmp_ch) || tmp_ch->invis_level <= 58) {
#if 0
                    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
#endif
                    ProtectedDataLock(player->connection->hostName);
                    LogPrint(LOG_INFO, "%s[%s] has reconnected.", GET_NAME(ch),
                             (char *)player->connection->hostName->data);
                    ProtectedDataUnlock(player->connection->hostName);
                }
                
                if (ch->specials.hostip) {
                    memfree(ch->specials.hostip);
                }

                ProtectedDataLock(player->connection->hostName);
                ch->specials.hostip = 
                            strdup((char *)player->connection->hostName->data);
                ProtectedDataUnlock(player->connection->hostName);

                db_write_char_extra(ch);
                EnterState(player, STATE_PLAYING);
                if( IS_IMMORTAL(ch) ) {
                    player->handlingQ = InputImmortQ;
                } else {
                    player->handlingQ = InputPlayerQ;
                }
                return;
            }
        }
#endif

        db_load_char_extra(ch);
        if (ch->specials.hostip == NULL) {
            if (!IS_IMMORTAL(ch) ||
                ch->invis_level <= 58) {
                ProtectedDataLock(player->connection->hostName);
                LogPrint(LOG_INFO, "%s[%s] has connected.\n\r", GET_NAME(ch),
                         (char *)player->connection->hostName->data);
                ProtectedDataUnlock(player->connection->hostName);
            }
        } else if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            ProtectedDataLock(player->connection->hostName);
            LogPrint(LOG_INFO, "%s[%s] has connected - Last connected from[%s]",
                     GET_NAME(ch), 
                     (char *)player->connection->hostName->data,
                     ch->specials.hostip);
            ProtectedDataUnlock(player->connection->hostName);
        }

        if (ch->specials.hostip) {
            memfree(ch->specials.hostip);
        }
        ProtectedDataLock(player->connection->hostName);
        ch->specials.hostip = 
                        strdup((char *)player->connection->hostName->data);
        ProtectedDataUnlock(player->connection->hostName);
        ch->last_tell = NULL;

        db_write_char_extra(ch);
        EnterState(player, STATE_SHOW_MOTD);
        break;

    case STATE_GET_NEW_USER_PASSWORD:
        /*
         * get pwd for new player
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            SendOutputRaw(player, echo_on, 6);
            SendOutput(player, "Illegal password.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        }

        strncpy(ch->pwd, (char *)crypt(arg, ch->player.name), 10);
        ch->pwd[10] = '\0';

        SendOutputRaw(player, echo_on, 6);
        EnterState(player, STATE_CONFIRM_PASSWORD);
        break;

    case STATE_CONFIRM_PASSWORD:
        /*
         * get confirmation of new pwd
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *) crypt(arg, ch->pwd), ch->pwd, 10)) {
            SendOutputRaw(player, echo_on, 6);

            SendOutput(player, "Passwords don't match.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        } 

        SendOutputRaw(player, echo_on, 6);
        EnterState(player, STATE_CHOOSE_ANSI);
        break;

    case STATE_CHOOSE_SEX:
        /*
         * query sex of new user
         */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "That's not a sex..\n\r");
            EnterState(player, STATE_CHOOSE_SEX);
            return;
        }

        switch (tolower(*arg)) {
        case 'm':
            /*
             * sex MALE
             */
            ch->player.sex = SEX_MALE;
            break;

        case 'f':
            /*
             * sex FEMALE
             */
            ch->player.sex = SEX_FEMALE;
            break;

        default:
            SendOutput(player, "That's not a sex..\n\r");
            EnterState(player, STATE_CHOOSE_SEX);
            return;
            break;
        }

        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    case STATE_CHOOSE_STATS:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        index = 0;
        while (arg && *arg && index < MAX_STAT) {
            switch(tolower(*arg)) {
            case 's':
                player->stat[index++] = 's';
                break;
            case 'i':
                player->stat[index++] = 'i';
                break;
            case 'w':
                player->stat[index++] = 'w';
                break;
            case 'd':
                player->stat[index++] = 'd';
                break;
            case 'c':
                arg++;
                if(tolower(*arg) == 'o') {
                    player->stat[index++] = 'o';
                    break;
                } else if (tolower(*arg) == 'h') {
                    player->stat[index++] = 'h';
                    break;
                } 
                /* If neither Co or Ch, fall through to default */
            default:
                SendOutput(player, "That was an invalid choice.\n\r");
                EnterState(player, STATE_CHOOSE_STATS);
                return;
                break;
            }

            while(*arg != ' ') {
                arg++;
            }
            while(*arg == ' ') {
                arg++;
            }
        }

        if (index < MAX_STAT) {
            SendOutput(player, "You did not enter enough legal stats\n\r"
                               "That was an invalid choice.\n\r");
            EnterState(player, STATE_CHOOSE_STATS);
            return;
        } 
        
        roll_abilities(player);

        if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
            /*
             * set the AUTH flags
             * (3 chances)
             */
            ch->generic = NEWBIE_REQUEST + NEWBIE_CHANCES;
        }

        ch->reroll--;
        EnterState(player, STATE_REROLL);
        break;

    case STATE_REROLL:
        arg = skip_spaces(arg);
        ch->reroll--;

        if (!arg || tolower(*arg) != 'r') {
            SendOutput(player, "Stats chosen!\n\r");

            if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
                EnterState(player, STATE_WAIT_FOR_AUTH);
            } else {
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            return;
        } 
        
        roll_abilities(player);

        if (ch->reroll != 0) {
            EnterState(player, STATE_REROLL);
            return;
        } 
        
        SendOutput(player, "Your final stats are:\n\r");
        SendOutput(player, "STR: -]%s\n\r", STAT_SWORD(GET_STR(ch)));
        SendOutput(player, "CON: -]%s\n\r", STAT_SWORD(GET_CON(ch)));
        SendOutput(player, "DEX: -]%s\n\r", STAT_SWORD(GET_DEX(ch)));
        SendOutput(player, "INT: -]%s\n\r", STAT_SWORD(GET_INT(ch)));
        SendOutput(player, "WIS: -]%s\n\r", STAT_SWORD(GET_WIS(ch)));
        SendOutput(player, "CHR: -]%s\n\r", STAT_SWORD(GET_CHR(ch)));
        SendOutput(player, "Stats chosen!");

        if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
            EnterState(player, STATE_WAIT_FOR_AUTH);
        } else {
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
        break;

    case STATE_CHOOSE_MAIN_CLASS:
        arg = skip_spaces(arg);
        ch->specials.remortclass = 0;

        if (arg && (pick = atoi(arg)) &&
            HasClass(ch, pc_num_class(pick-1))) {
            ch->specials.remortclass = pick;
            EnterState(player, STATE_SHOW_CREATION_MENU);
            return;
        } 
        
        SendOutput(player, "\n\rInvalid class picked.\n\r");
        EnterState(player, STATE_CHOOSE_MAIN_CLASS);
        break;

    case STATE_CHOOSE_CLASS:
        /*
         * skip whitespaces
         */
        ch->player.class = 0;

        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Invalid selection!\n\r");
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        if( *arg == '?' ) {
            for( i = 0; class_help[i]; i++ ) {
                SendOutput(player, class_help[i]);
            }
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        class = atoi(arg);
        race  = GET_RACE(ch);

        if( class <= 0 ) {
            SendOutput(player, "Invalid selection!\n\r");
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        for( i = 0, found = FALSE; !found && i < race_choice_count; i++ ) {
            if( race_choice[i].raceNum == race ) {
                /*
                 * Found it.  Time to check the class
                 */
                if( class > race_choice[i].classCount ) {
                    SendOutput(player, "Invalid selection!\n\r");
                    EnterState(player, STATE_CHOOSE_CLASS);
                    return;
                }

                /* Class choice is valid */
                ch->player.class = race_choice[i].classesAvail[class - 1];
                found = TRUE;
            }
        }
                    
        if (ch->player.class == 0) {
            SendOutput(player, "Invalid selection!\n\r");
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        if( found ) {
            if (HasClass(ch, CLASS_MAGIC_USER)) {
                EnterState(player, STATE_CHECK_MAGE_TYPE);
                return;
            }

            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
            SendOutput(player, "Your race seems to be incorrect, please "
                               "reselect\n\r");
            LogPrintNoArg(LOG_NOTICE, "Couldn't find a race in creation, "
                                      "screwy!!");
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
        break;

    case STATE_WAIT_FOR_AUTH:
        if (ch->generic >= NEWBIE_START) {
            /*
             * now that classes are set, initialize
             */
#if 0
            init_char(ch);
#endif

            /*
             * create an entry in the file
             */
#ifdef TODO
            d->pos = create_entry(GET_NAME(ch));
#endif
#if 0
            save_char(ch, AUTO_RENT);
#endif
            EnterState(player, STATE_SHOW_MOTD);
            return;
        } 
        
        if (ch->generic >= NEWBIE_REQUEST) {
            ProtectedDataLock(player->connection->hostName);
            SysLogPrint(LOG_INFO, "%s [%s] new player.", GET_NAME(ch),
                                  (char *)player->connection->hostName->data);
            ProtectedDataUnlock(player->connection->hostName);

#ifdef TODO
            if (top_of_p_table > 0) {
                SysLogPrint(LOG_INFO, "Type Authorize %s [Yes | No | Message]",
                                      GET_NAME(ch));
                SysLogPrintNoArg(LOG_INFO, "type 'Wizhelp Authorize' for other "
                                           "commands");
            } else {
                LogPrintNoArg(LOG_NOTICE, "Initial character.  Authorized "
                                          "Automatically");
                ch->generic = NEWBIE_START + 5;
            }
#endif

            /*
             * enough for gods.  now player is told to shut up.
             * NEWBIE_START == 3 == 3 chances 
             */ 
            ch->generic--;
            SendOutput(player, "Please wait. You have %d requests "
                               "remaining.\n\r", ch->generic);
            if (ch->generic == 0) {
                EnterState(player, STATE_WIZLOCKED);
            } else {
                EnterState(player, STATE_WAIT_FOR_AUTH);
            }
        } else {
            EnterState(player, STATE_WIZLOCKED);
        }
        break;

    case STATE_CHECK_MAGE_TYPE:
        arg = skip_spaces(arg);
        if (arg && tolower(*arg) == 'y') {
            ch->player.class -= CLASS_MAGIC_USER;
            ch->player.class += CLASS_SORCERER;
        }
        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    case STATE_SHOW_MOTD:
        /*
         * read CR after printing motd
         */
        if (IS_IMMORTAL(ch)) {
            EnterState(player, STATE_SHOW_WMOTD);
            break;
        }

        if (ch->term != 0) {
            ScreenOff(player);
        }


        ProtectedDataLock(player->connection->hostName);
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || 
             SiteLock((char *)player->connection->hostName->data)) &&
            !IS_IMMORTAL(ch)) {
            ProtectedDataUnlock(player->connection->hostName);
            SendOutput(player, "Sorry, the game is locked up for repair or "
                               "your site is banned.\n\r");
            EnterState(player, STATE_WIZLOCKED);
        } else {
            ProtectedDataUnlock(player->connection->hostName);
            EnterState(player, STATE_SHOW_LOGIN_MENU);
        }
        break;

    case STATE_SHOW_WMOTD:
        /*
         * read CR after printing motd
         */
        ProtectedDataLock(player->connection->hostName);
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || 
             SiteLock((char *)player->connection->hostName->data)) &&
            !IS_IMMORTAL(ch)) {
            ProtectedDataUnlock(player->connection->hostName);
            SendOutput(player, "Sorry, the game is locked up for repair or "
                               "your site is banned.\n\r");
            EnterState(player, STATE_WIZLOCKED);
        } else {
            ProtectedDataUnlock(player->connection->hostName);
            EnterState(player, STATE_SHOW_LOGIN_MENU);
        }
        break;

    case STATE_WIZLOCKED:
        connClose( player->connection );
        break;

    case STATE_DELETE_USER:
        arg = skip_spaces(arg);
        if (arg && !strcasecmp(arg, "yes") && 
            strcasecmp("Guest", GET_NAME(ch))) {
            LogPrint(LOG_INFO, "%s just killed theirself!", GET_NAME(ch));
#ifdef TODO
            for (i = 0; i <= top_of_p_table; i++) {
                if (!strcasecmp(player_table[i].name, GET_NAME(ch))) {
                    if (player_table[i].name) {
                        memfree(player_table[i].name);
                    }
                    player_table[i].name = strdup("111111");
                    break;
                }
            }

            /*
             * get the structure from player_table[i].nr
             */
            if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
                perror("Opening player file for updating. (login-sm.c,"
                       " LoginStateMachine)");
                assert(0);
            }
            rewind(char_file);

            /*
             * read in the char, change the name, write back
             */
            sprintf(ch_st.name, "111111");
            rewind(char_file);
            fclose(char_file);
            sprintf(buf, "rent/%s", lower(GET_NAME(ch)));
            remove(buf);
            sprintf(buf, "rent/%s.aux", GET_NAME(ch));
            remove(buf);
#endif
            connClose( player->connection );
        }

        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    case STATE_PRESS_ENTER:
        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    case STATE_SHOW_LOGIN_MENU:
        /* 
         * get selection from main menu
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if(!arg) {
            SendOutput(player, "Wrong option.\n\r");
            EnterState(player, STATE_SHOW_LOGIN_MENU);
            break;
        }

        switch (tolower(*arg)) {
        case '0':
            connClose( player->connection );
            break;

        case '1':
            EnterState(player, STATE_PLAYING);
            if( IS_IMMORTAL(ch) ) {
                player->handlingQ = InputImmortQ;
            } else {
                player->handlingQ = InputPlayerQ;
            }

            stateItem = CREATE(InputStateItem_t);
            if( !stateItem ) {
                /*
                 * out of memory, doh!
                 */
                connClose(player->connection);
                return;
            }

            stateItem->player = player;
            stateItem->type   = INPUT_INITIAL;
            stateItem->line   = NULL;
            QueueEnqueueItem( player->handlingQ, stateItem );
            break;

        case '2':
            SendOutput(player, "Enter a text you'd like others to see when "
                               "they look at you.\n\r");
            if (ch->player.description) {
                SendOutput(player, "Old description :\n\r");
                if (ch->player.description) {
                    SendOutput(player, ch->player.description);
                    memfree(ch->player.description);
                } else {
                    SendOutput(player, "None");
                }
                ch->player.description = NULL;
            }
            EnterState(player, STATE_EDIT_EXTRA_DESCR);
            break;

        case '3':
#if 0
            SendOutput(player, STORY);
#endif
            EnterState(player, STATE_PRESS_ENTER);
            break;

        case '4':
            SendOutput(player, credits);
            EnterState(player, STATE_PRESS_ENTER);
            break;

        case '5':
            EnterState(player, STATE_GET_NEW_PASSWORD);
            break;

        case 'k':
            EnterState(player, STATE_DELETE_USER);
            break;

        default:
            SendOutput(player, "Wrong option.\n\r");
            EnterState(player, STATE_SHOW_LOGIN_MENU);
            break;
        }
        break;

    case STATE_GET_NEW_PASSWORD:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            SendOutputRaw(player, echo_on, 6);

            SendOutput(player, "Illegal password.\n\r");
            EnterState(player, STATE_GET_NEW_PASSWORD);
            return;
        }

        strncpy(ch->pwd, (char *)crypt(arg, ch->player.name), 10);
        ch->pwd[10] = '\0';
        SendOutputRaw(player, echo_on, 6);

        EnterState(player, STATE_CONFIRM_NEW_PASSWORD);
        break;

    case STATE_EDIT_EXTRA_DESCR:
        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    case STATE_CONFIRM_NEW_PASSWORD:
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *)crypt(arg, ch->pwd), ch->pwd, 10)) {
            SendOutputRaw(player, echo_on, 6);
            SendOutput(player, "Passwords don't match.\n\r");
            EnterState(player, STATE_GET_NEW_PASSWORD);
            return;
        }

        SendOutputRaw(player, echo_on, 6);

        SendOutput(player, "\n\rDone. You must enter the game to make the "
                           "change final\n\r");

        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    default:
        LogPrint(LOG_NOTICE, "Nanny: illegal state of con'ness (%d)", 
                 player->state);
        SendOutput(player, "The mud has lost its brain on your connection, "
                           "please reconnect.\n\r");
        connClose( player->connection );
        break;
    }
}


void DoCreationMenu( PlayerStruct_t *player, char arg )
{
    int             bit;
    int             bitcount;
    int             i;
    struct char_data *ch;

    ch = player->charData;

    switch (arg) 
    {
    case '1':
        EnterState(player, STATE_CHOOSE_SEX);
        break;
    case '2':
        EnterState(player, STATE_CHOOSE_ANSI);
        break;
    case '3':
        EnterState(player, STATE_CHOOSE_RACE);
        break;
    case '4':
        EnterState(player, STATE_CHOOSE_CLASS);
        break;
    case '5':
        if (ch->player.class != 0) {
            EnterState(player, STATE_CHOOSE_MAIN_CLASS);
        } else {
            SendOutput(player, "\nPlease select a class first.\n\r");
        }
        break;
    case '6':
        ch->reroll = 20;
        if (ch->player.class != 0) {
            EnterState(player, STATE_CHOOSE_STATS);
        } else {
            SendOutput(player, "\nPlease select a class first.\n\r");
        }
        break;
    case '7':
        EnterState(player, STATE_CHOOSE_ALIGNMENT);
        break;

    case 'd':
    case 'D':
        bitcount = 0;
        for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
            if (HasClass(ch, pc_num_class(bit))) {
                bitcount++;
            }
        }
        if (bitcount <= 0) {
            SendOutput(player, "Please enter a valid class.");
            return;
        }
        if (ch->specials.remortclass <= 0) {
            SendOutput(player, "Please enter a valid main class.");
            return;
        }

        if (GET_SEX(ch) == 0) {
            SendOutput(player, "Please enter a proper sex.");
            return;
        }

        if (!GET_ALIGNMENT(ch)) {
            SendOutput(player, "Please choose an alignment.");
            return;
        }
        if (GET_CON(ch) == 0) {
            SendOutput(player, "Please pick your stats.");
            return;
        }

        ProtectedDataLock(player->connection->hostName);
        LogPrint(LOG_INFO, "%s [%s] new player.", GET_NAME(ch), 
                 (char *)player->connection->hostName->data);
        ProtectedDataUnlock(player->connection->hostName);

        /*
         * now that classes are set, initialize
         */
#if 0
        init_char(ch);
#endif

        /*
         * create an entry in the file
         */
#ifdef TODO
        d->pos = create_entry(GET_NAME(ch));
#endif
#if 0
        save_char(ch, AUTO_RENT);
#endif

        for( i = 0; newbie_note[i]; i++ ) {
            SendOutput(player, newbie_note[i]);
        }

        EnterState(player, STATE_SHOW_MOTD);
        break;
    default:
        show_menu(player);
        SendOutput(player, "Invalid Choice.. Try again..\n\r");
        break;
    }
}

void show_class_selection(PlayerStruct_t *player, int r)
{
    int             found;
    int             i,
                    j;
    char            buf[254],
                    buf2[254];

    for( i = 0, found = FALSE; !found && race_choice_count; i++ ) {
        if( race_choice[i].raceNum == r ) {
            /* Found it, show the classes */
            for( j = 0; j < race_choice[i].classCount; j++ ) {
                sprintf( buf, "%d) ", j + 1 );
#if 0
                sprintclasses((unsigned)race_choice[i].classesAvail[j],
                              buf2);
#endif
                strcat(buf, buf2);
                strcat(buf, "\n\r");
                SendOutput(player, buf);
            }

            found = TRUE;
        }
    }

    if( !found ) {
        LogPrintNoArg(LOG_INFO, "Screwup in show_class_selection()");
    }
}


void show_race_choice(PlayerStruct_t *player)
{
    int             i,
                    j;

    SendOutput(player, "                                  Level Limits\n\r");
    SendOutput(player, 
                 "%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s "
                 "%-3s %-3s %-3s\n\r",
                 "#", "Race", "ma", "cl", "wa", "th", "dr", "mk", "ba", "so",
                 "pa", "ra", "ps", "ne");

    for (i = 0; i < race_choice_count; i++) {
        SendOutput(player, "%s$c000W%-3d)$c0007 %-15s", 
                           (race_choice[i].raceNum == RACE_DROW ?
                            "$c000WThe Races Listed below may have some "
                            "racial hatreds.  Advanced players only.\n\r" : ""),
                            i + 1, races[race_choice[i].raceNum].racename);

        /*
         * show level limits 
         */
        for (j = 0; j < MAX_CLASS; j++) {
            SendOutput(player, " %-3d", 
                               races[race_choice[i].raceNum].racialMax[j]);
        }

        SendOutput(player, "\n\r");
    }

    SendOutput(player, "$c000gma=magic user, cl=cleric, wa=warrior,th=thief,"
                       "dr=druid,mk=monk\n\r"
                       "$c000gba=barbarian,so=sorcerer,pa=paladin,ra=ranger,"
                       "ps=psi,ne=necromancer\n\r\n\r");
}

void LoginSendBanner( PlayerStruct_t *player )
{
    SendOutput(player, login);
    SendOutput(player, "If you're using Tintin or Lyntin, your client may not "
                       "display the password\n\r"
                       "sequence unless you change your settings. Please do not"
                       " be discouraged.\n\r\n\r");
    EnterState(player, STATE_GET_NAME);
}

/**
 * @todo clean this up
 *
 * Give pointers to the five abilities
 */
void roll_abilities(PlayerStruct_t *player)
{
    struct char_data *ch;
    int             i,
                    j,
                    k,
                    temp;
    float           avg;
    ubyte           table[MAX_STAT];
    ubyte           rools[4];

    ch = player->charData;

    for (i = 0; i < MAX_STAT; i++) {
        table[i] = 8;
    }

    do {
        for (i = 0; i < MAX_STAT; i++) {
            for (j = 0; j < 4; j++) {
                rools[j] = number(1, 6);
            }
            temp = (unsigned int) rools[0] + (unsigned int) rools[1] +
                   (unsigned int) rools[2] + (unsigned int) rools[3] -
                   MIN((int) rools[0],
                       MIN((int) rools[1],
                           MIN((int) rools[2], (int) rools[3])));

            for (k = 0; k < MAX_STAT; k++) {
                if (table[k] < temp) {
                    SWITCH(temp, table[k]);
                }
            }
        }
        for (j = 0, avg = 0; j < MAX_STAT; j++) {
            avg += table[j];
        }
        avg /= j;
    }
    while (avg < 9.0);

    for (i = 0; i < MAX_STAT; i++) {
        switch (player->stat[i]) {
        case 's':
            ch->abilities.str = table[i];
            break;
        case 'i':
            ch->abilities.intel = table[i];
            break;
        case 'd':
            ch->abilities.dex = table[i];
            break;
        case 'w':
            ch->abilities.wis = table[i];
            break;
        case 'o':
            ch->abilities.con = table[i];
            break;
        case 'h':
            ch->abilities.chr = table[i];
            break;
        default:
            break;
        }
    }

    if (ch->abilities.str == 0) {
        ch->abilities.str = 8;
    }
    if (ch->abilities.intel == 0) {
        ch->abilities.intel = 8;
    }
    if (ch->abilities.dex == 0) {
        ch->abilities.dex = 8;
    }
    if (ch->abilities.wis == 0) {
        ch->abilities.wis = 8;
    }
    if (ch->abilities.con == 0) {
        ch->abilities.con = 8;
    }
    if (ch->abilities.chr == 0) {
        ch->abilities.chr = 8;
    }
    ch->abilities.str_add = 0;

    switch (GET_RACE(ch)) {
        case RACE_MOON_ELF:
            ch->abilities.dex++;
            ch->abilities.con--;
            break;
        case RACE_AVARIEL:
            ch->abilities.dex++;
            ch->abilities.con--;
            break;
        case RACE_SEA_ELF:
            ch->abilities.dex += 2;
            ch->abilities.con -= 2;
        case RACE_WILD_ELF:
            ch->abilities.str++;
            ch->abilities.intel--;
            ch->abilities.dex++;
            ch->abilities.wis--;
            break;
        case RACE_GOLD_ELF:
            ch->abilities.intel++;
            ch->abilities.wis--;
            ch->abilities.dex++;
            ch->abilities.con--;
            break;
        case RACE_DWARF:
        case RACE_DARK_DWARF:
            ch->abilities.con++;
            ch->abilities.dex--;
            break;
        case RACE_ROCK_GNOME:
            ch->abilities.intel++;
            ch->abilities.wis--;
        case RACE_DEEP_GNOME:
            ch->abilities.intel--;
            ch->abilities.wis++;
            ch->abilities.dex++;
            ch->abilities.chr -= 2;
            break;
        case RACE_FOREST_GNOME:
            ch->abilities.intel--;
            ch->abilities.wis++;
            ch->abilities.dex++;
            ch->abilities.str--;
            break;
        case RACE_HALFLING:
        case RACE_GOBLIN:
            ch->abilities.dex++;
            ch->abilities.str--;
            break;
        case RACE_DROW:
            ch->abilities.dex += 2;
            ch->abilities.con--;
            ch->abilities.chr--;
            break;
        case RACE_HALF_OGRE:
            ch->abilities.str++;
            ch->abilities.con++;
            ch->abilities.dex--;
            ch->abilities.intel--;
            break;
        case RACE_ORC:
            ch->abilities.str++;
            ch->abilities.con++;
            ch->abilities.chr -= 2;
            ch->abilities.intel -= 2;
            break;
        case RACE_HALF_ORC:
            ch->abilities.con++;
            ch->abilities.chr--;
        case RACE_HALF_GIANT:
        case RACE_TROLL:
            ch->abilities.str += 2;
            ch->abilities.con++;
            ch->abilities.dex--;
            ch->abilities.wis--;
            ch->abilities.intel--;
            break;
        default:
            break;
    }
    ch->points.max_hit = HowManyClasses(ch) * 10;

    /*
     * race specific hps stuff
     */
    if (GET_RACE(ch) == RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL) {
        ch->points.max_hit += 15;
    }


    /*
     * class specific hps stuff
     */
    if (HasClass(ch, CLASS_MAGIC_USER) ||
        HasClass(ch, CLASS_SORCERER) ||
        HasClass(ch, CLASS_NECROMANCER)) {
        ch->points.max_hit += number(1, 4);
    }
    if (HasClass(ch, CLASS_THIEF) ||
        HasClass(ch, CLASS_PSI) ||
        HasClass(ch, CLASS_MONK)) {
        ch->points.max_hit += number(1, 6);
    }
    if (HasClass(ch, CLASS_CLERIC) ||
        HasClass(ch, CLASS_DRUID)) {
        ch->points.max_hit += number(1, 8);
    }
    if (HasClass(ch, CLASS_WARRIOR) ||
        HasClass(ch, CLASS_BARBARIAN) ||
        HasClass(ch, CLASS_PALADIN) ||
        HasClass(ch, CLASS_RANGER)) {
        ch->points.max_hit += number(1, 10);

        if (ch->abilities.str == 18) {
            ch->abilities.str_add = number(0, 100);
        }
        if (ch->abilities.str > 18 &&
            (GET_RACE(ch) != RACE_HALF_GIANT || GET_RACE(ch) == RACE_TROLL)) {
            ch->abilities.str_add =
                number(((ch->abilities.str - 18) * 10), 100);
        }
        else if (ch->abilities.str > 18) {
            /*
             * was a half-giant so
             * just make 100
             */
            ch->abilities.str_add = 100;
        }
    }

    ch->points.max_hit /= HowManyClasses(ch);
    ch->tmpabilities = ch->abilities;
}

int SiteLock(char *site)
{
#ifdef SITELOCK
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( banHostTree, (void *)&site, UNLOCKED, TRUE );
    if( item ) {
        return( TRUE );
    }
#endif
    return (FALSE);
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
