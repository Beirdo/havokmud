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
 *
 * Comments :
 *
 * Login & character creation state machine
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>
#include "protected_data.h"

#include "protos.h"
#include "externs.h"
#include "interthread.h"

#define MAX_NAME_LENGTH 11

void EnterState(PlayerStruct_t *player, PlayerState_t newstate);
void show_race_choice(PlayerStruct_t *player);
void show_class_selection(PlayerStruct_t *player, int r);
void show_menu(PlayerStruct_t *player);

static char     swords[] = ">>>>>>>>";
#define STAT_SWORD(x) (((x)<18 && (x)>0) ? &(swords[5-(((x)-1)/3)]) : "ERR!")

unsigned char   echo_on[] = { IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0' };
unsigned char   echo_off[] = { IAC, WILL, TELOPT_ECHO, '\0' };
int             plr_tick_count = 0;
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
                                      "interpreter.c _parse_name");
            return( 1 );
        }
    }

    return( 0 );
}


void show_menu(PlayerStruct_t *player)
{

    int             bit;
    char            buf[100];
    char            bufx[1000];
    char            cls[50];
    char            mainclass[50];

    cls[0] = '\0';

    for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
        if (HasClass(player->charData, pc_num_class(bit))) {
            strcat(cls, classes[bit].abbrev);
        }
    }
    if (!(strcmp(cls, ""))) {
        sprintf(cls, "None Selected");
    }
    mainclass[0] = '\0';
    if (player->charData->specials.remortclass) {
       /*
        * remort == 0 means none picked
        */
        strcat(mainclass, 
               classes[(player->charData->specials.remortclass - 1)].abbrev);
    }
    if (!(strcmp(mainclass, ""))) {
        sprintf(mainclass, "None Selected");
    }
    sprintf(bufx, "$c0009-=$c0015Havok Character Creation Menu [%s]"
                  "$c0009=-\n\r\n\r", GET_NAME(player->charData));

    sprintf(buf, "$c00151) $c0012Gender.[$c0015%s$c0012]\n\r",
            Sex[((int) GET_SEX(player->charData))]);
    strcat(bufx, buf);

    sprintf(buf, "$c00152) $c0012Ansi Colors.\n\r");
    strcat(bufx, buf);

    if (GET_RACE(player->charData) != 0) {
        sprintf(buf, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                races[GET_RACE(player->charData)].racename);
        strcat(bufx, buf);
    } else {
        /*
         * make default race to Human rather than half-breed
         */
        GET_RACE(player->charData) = 1;
        sprintf(buf, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                races[GET_RACE(player->charData)].racename);
        strcat(bufx, buf);
    }

    sprintf(buf, "$c00154) $c0012Class.[$c0015%s$c0012]\n\r", cls);
    strcat(bufx, buf);

    sprintf(buf, "$c00155) $c0012Main Class.[$c0015%s$c0012]\n\r", mainclass);
    strcat(bufx, buf);

    if (!GET_CON(player->charData) || GET_CON(player->charData) == 0) {
        strcat(bufx, "$c00156) $c0012Character Stats.[$c0015None "
                     "Picked$c0012]\n\r");
    } else {
        strcat(bufx, "$c00156) $c0012Character Stats.[$c0015Done$c0012]\n\r");
    }
    sprintf(buf, "$c00157) $c0012Alignment.[$c000W%s$c000B]\n\r\n\r",
            (GET_ALIGNMENT(player->charData) ?
             AlignDesc(GET_ALIGNMENT(player->charData)) : "None"));
    strcat(bufx, buf);

    strcat(bufx, "$c0015D) $c0012Done!\n\r\n\r");
    strcat(bufx, "$c0011Please pick an option: \n\r");

    SendOutput(bufx, player);
}

void EnterState(PlayerStruct_t *player, PlayerState_t newstate)
{
    char            buf[MAX_STRING_LENGTH];
    int             chosen = 0;
    int             i;

    if( !player ) {
        return;
    }

    switch( newstate ) {
    case STATE_CHOOSE_SEX:
        SendOutput("What is your sex (M/F) ? ", player);
        break;
    case STATE_CHOOSE_ANSI:
        SendOutput("Would you like ansi colors? (Yes or No)", player);
        break;
    case STATE_CHOOSE_RACE:
        show_race_choice(player);
        SendOutput("For help type '?'- will list level limits. \n\r RACE:  ", 
                   player);
        player->charData->player.class = 0;
        player->charData->specials.remortclass = 0;
        break;
    case STATE_CHOOSE_CLASS:
        GET_ALIGNMENT(player->charData) = 0;
        GET_CON(player->charData) = 0;
        SendOutput("\n\rSelect your class now.\n\r", player);
        show_class_selection(player, GET_RACE(player->charData));
        SendOutput("Enter ? for help.\n\r\n\rClass :", player);
        break;
    case STATE_CHOOSE_MAIN_CLASS:
        SendOutput("\n\rSelect your main class from the options below.\n\r",
                   player);

        for (chosen = 0; chosen <= NECROMANCER_LEVEL_IND; chosen++) {
            if (HasClass(player->charData, pc_num_class(chosen))) {
                ch_printf(player, "[%2d] %s\n\r", chosen + 1,
                          classes[chosen].name);
            }
        }
        SendOutput("\n\rMain Class :", player);
        break;
    case STATE_CHOOSE_STATS:
        SendOutput("\n\rSelect your stat priority, by listing them from"
                   " highest to lowest\n\r", player);
        SendOutput("Seperated by spaces.. don't duplicate\n\r", player);
        SendOutput("for example: 'S I W D Co Ch' would put the highest"
                   " roll in Strength, \n\r", player);
        SendOutput("next in intelligence, Wisdom, Dex, Con, and lastly"
                   " charisma\n\r", player);
        SendOutput("Your choices? ", player);
        break;
    case STATE_CHOOSE_ALIGNMENT:
        ch_printf(player,
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

        if (HasClass(player->charData, CLASS_PALADIN)) {
            ch_printf(player, "Please select your alignment "
                              "($c000WGood$c000w)");
        } else if (HasClass(player->charData, CLASS_DRUID)) {
            ch_printf(player, "Please select your alignment "
                              "(Neutral)");
        } else if (HasClass(player->charData, CLASS_NECROMANCER)) {
            ch_printf(player, "Please select your alignment "
                              "($c000REvil$c000w)");
        } else if (HasClass(player->charData, CLASS_RANGER)) {
            ch_printf(player, "Please select your alignment "
                              "($c000WGood$c000w/Neutral$c000w)");
        } else {
            ch_printf(player, "Please select your alignment "
                              "($c000WGood$c000w/Neutral$c000w/"
                              "$c000REvil$c000w)");
        }
        break;

    case STATE_SHOW_MOTD:
        SendOutput(motd, player);
        SendOutput("\n\r\n*** PRESS RETURN: ", player);
        break;

    case STATE_SHOW_CREATION_MENU:
        show_menu(player);
        break;

    case STATE_GET_PASSWORD:
        SendOutput("Password: ", player);
        SendOutputRaw(echo_off, 4, player);
        break;
    case STATE_CONFIRM_NAME:
        sprintf(buf, "Did I get that right, %s (Y/N)? ",
                GET_NAME(player->charData));
        SendOutput(buf, player);
        break;
    case STATE_GET_NEW_USER_PASSWORD:
        sprintf(buf, "Give me a password for %s: ", GET_NAME(player->charData));
        SendOutput(buf, player);
        SendOutputRaw(echo_off, 4, player);
        break;
    case STATE_GET_NAME:
        if (GET_NAME(player->charData)) {
            free(GET_NAME(player->charData));
            GET_NAME(player->charData) = NULL;
        }
        SendOutput("What is thy name? ", player);
        break;
    case STATE_PLAYING:
        break;
    case STATE_CONFIRM_PASSWORD:
    case STATE_CONFIRM_NEW_PASSWORD:
        SendOutput("Please retype password: ", player);
        SendOutputRaw(echo_off, 4, player);
        break;
    case STATE_REROLL:
        SendOutput("Your current stats are:\n\r", player);
        sprintf(buf, "STR: -]%s\n\r", STAT_SWORD(GET_STR(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "CON: -]%s\n\r", STAT_SWORD(GET_CON(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "DEX: -]%s\n\r", STAT_SWORD(GET_DEX(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "INT: -]%s\n\r", STAT_SWORD(GET_INT(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "WIS: -]%s\n\r", STAT_SWORD(GET_WIS(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "CHR: -]%s\n\r", STAT_SWORD(GET_CHR(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "\n\rYou have %d rerolls left, press R to reroll, any"
                     " other key to keep.\n\r", player->charData->reroll);
        SendOutput(buf, player);
        break;
    case STATE_CHECK_MAGE_TYPE:
        for( i = 0; ru_sorcerer[i]; i++ ) {
            SendOutput(ru_sorcerer[i], player);
        }
        break;
    case STATE_WAIT_FOR_AUTH:
        SendOutput("Please Wait for authorization.\n\r", player);
        break;
    case STATE_WIZLOCKED:
        SendOutput("Goodbye.\n\r", player);
        break;
    case STATE_SHOW_WMOTD:
        SendOutput(wmotd, player);
        SendOutput("\n\r\n[PRESS RETURN]", player);
        break;
    case STATE_SHOW_LOGIN_MENU:
        SendOutput(MENU, player);
        break;
    case STATE_EDIT_EXTRA_DESCR:
        SendOutput("<type /w to save.>\n\r", player);
        break;
    case STATE_PRESS_ENTER:
        SendOutput("\n\r<Press enter to continue>", player);
        break;
    case STATE_GET_NEW_PASSWORD:
        SendOutput("Enter a new password: ", player);
        SendOutputRaw(echo_off, 4, player);
        break;
    case STATE_DELETE_USER:
        SendOutput("Are you sure you want to delete yourself? (yes/no) ", 
                   player);
        break;
    case STATE_INITIAL:
        break;
    }

    player->state = newstate;
}

void LoginStateMachine(PlayerStruct_t *player, char *arg)
{
    struct descriptor_data *desc;
    char            buf[1024];

    int             player_i;
    int             class,
                    race,
                    found,
                    index = 0;
    char            tmp_name[20];
    struct char_file_u tmp_store;
    struct char_data *tmp_ch;
    struct descriptor_data *k;
    int             count_players = 0;
    int             bit = 0;
    int             i = 0;
    int             tmpi = 0;
    int             already_p = 0;
    int             pick = 0;
    struct char_file_u ch_st;
    FILE           *char_file;
    struct obj_data *obj;

    SendOutputRaw(echo_on, 6, player);

    switch (player->state) {
    case STATE_SHOW_CREATION_MENU:
#if 0
        show_menu(d);
#endif
        arg = skip_spaces(arg);
        if( !arg ) {
            show_menu(player);
            SendOutput("Invalid Choice.. Try again..\n\r", player);
            return;
        }

        switch (*arg) {
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
            if (player->charData->player.class != 0) {
                EnterState(player, STATE_CHOOSE_MAIN_CLASS);
            } else {
                SendOutput("\nPlease select a class first.\n\r", player);
            }
            break;
        case '6':
            player->charData->reroll = 20;
            if (player->charData->player.class != 0) {
                EnterState(player, STATE_CHOOSE_STATS);
            } else {
                SendOutput("\nPlease select a class first.\n\r", player);
            }
            break;
        case '7':
            EnterState(player, STATE_CHOOSE_ALIGNMENT);
            break;

        case 'd':
        case 'D':
            count_players = 0;
            for (bit = 0; bit <= NECROMANCER_LEVEL_IND; bit++) {
                if (HasClass(player->charData, pc_num_class(bit))) {
                    count_players++;
                }
            }
            if (count_players <= 0) {
                SendOutput("Please enter a valid class.", player);
                return;
            }
            if (player->charData->specials.remortclass <= 0) {
                SendOutput("Please enter a valid main class.", player);
                return;
            }

            if (GET_SEX(player->charData) == 0) {
                SendOutput("Please enter a proper sex.", player);
                return;
            }

            if (!GET_ALIGNMENT(player->charData)) {
                SendOutput("Please choose an alignment.", player);
                return;
            }
            if (!GET_CON(player->charData) || GET_CON(player->charData) == 0) {
                SendOutput("Please pick your stats.", player);
                return;
            }

#ifdef TODO
            ProtectedDataLock(player->connection->hostName);
            LogPrint(LOG_INFO, "%s [%s] new player.", 
                     GET_NAME(player->charData), 
                     (char *)player->connection->hostName->data);
            ProtectedDataUnlock(player->connection->hostName);
#endif

            /*
             * now that classes are set, initialize
             */
            init_char(player->charData);

            /*
             * create an entry in the file
             */
#ifdef TODO
            d->pos = create_entry(GET_NAME(player->charData));
#endif
            save_char(player->charData, AUTO_RENT);

            for( i = 0; newbie_note[i]; i++ ) {
                SendOutput(newbie_note[i], player);
            }

            EnterState(player, STATE_SHOW_MOTD);
            break;
        default:
            show_menu(player);
            SendOutput("Invalid Choice.. Try again..\n\r", player);
            break;
        }
        break;
    case STATE_CHOOSE_ALIGNMENT:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput("Please select a alignment.\n\r", player);
            return;
        }

        switch (tolower(*arg)) {
        case 'n':
            if (!HasClass(player->charData, CLASS_PALADIN) &&
                !HasClass(player->charData, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(player->charData) = 1;
                SendOutput("You have chosen to be Neutral in "
                           "alignment.\n\r\n\r", player);
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;
        case 'g':
            if (!HasClass(player->charData, CLASS_DRUID) &&
                !HasClass(player->charData, CLASS_NECROMANCER)) {
                GET_ALIGNMENT(player->charData) = 1000;
                SendOutput("You have chosen to be a follower of "
                           "light.\n\r\n\r", player);
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;
        case 'e':
            if (!HasClass(player->charData, CLASS_DRUID) &&
                !HasClass(player->charData, CLASS_PALADIN) &&
                !HasClass(player->charData, CLASS_RANGER)) {
                GET_ALIGNMENT(player->charData) = -1000;
                SendOutput("You have chosen the dark side.\n\r\n\r", player);
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            break;

        default:
            SendOutput("Please select a alignment.\n\r", player);
            break;
        }
        break;

    case STATE_CHOOSE_ANSI:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput("Please type Yes or No.\n\r"
                       "Would you like ansi colors? :", player);
            return;
        }

        switch (tolower(*arg)) {
        case 'y':
            /*
             * Set ansi
             */
            SET_BIT(player->charData->player.user_flags, USE_ANSI);

            SendOutput("$c0012A$c0010n$c0011s$c0014i$c0007 colors "
                       "enabled.\n\r\n\r", player);
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;

        case 'n':
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;

        default:
            SendOutput("Please type Yes or No.\n\r"
                       "Would you like ansi colors? :", player);
            return;
            break;
        }
        break;

    case STATE_CHOOSE_RACE:
        player->charData->reroll = 20;
        arg = skip_spaces(arg);
        if (!arg) {
            EnterState(player, STATE_CHOOSE_RACE);
            return;
        } 
        
       if (*arg == '?') {
            for( i = 0; racehelp[i]; i++ ) {
                SendOutput(racehelp[i], player);
            }
            EnterState(player, STATE_CHOOSE_RACE);
            return;
        } 
       
        tmpi = atoi(arg);
        if (tmpi >= 1 && tmpi <= race_choice_count) {
            GET_RACE(player->charData) = race_choice[tmpi - 1].raceNum;
            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
            SendOutput("\n\rThat's not a race.\n\rRACE?:", player);
            EnterState(player, STATE_CHOOSE_RACE);
        }
        break;

    case STATE_GET_NAME:
    /*
     * wait for input of name
     */
        if (!player->charData) {
            CREATE(player->charData, struct char_data, 1);
            clear_char(player->charData);
#ifdef TODO
            player->charData->desc = d;
#endif
        }

        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection );
            return;
        } 
        
        if (_parse_name(arg, tmp_name)) {
            SendOutput("Illegal name, please try another.\r\n", player);
            SendOutput("Name: ", player);
            return;
        }

        ProtectedDataLock(player->connection->hostName);
        if (SiteLock((char *)player->connection->hostName->data)) {
            ProtectedDataUnlock(player->connection->hostName);
            SendOutput("Sorry, this site is temporarily banned.\n\r", player);
            EnterState(player, STATE_WIZLOCKED);
            return;
        }
        ProtectedDataUnlock(player->connection->hostName);

        if ((player_i = load_char(tmp_name, &tmp_store)) > -1) {
            /*
             * connecting an existing character ...
             */
            store_to_char(&tmp_store, player->charData);
#ifdef TODO
            strcpy(d->pwd, tmp_store.pwd);
            d->pos = player_table[player_i].nr;
#endif
            EnterState(player, STATE_GET_PASSWORD);
        } else {
            /*
             * player unknown gotta make a new
             */
            if (_check_ass_name(tmp_name)) {
                SendOutput("\n\rIllegal name, please try another.", player);
                SendOutput("Name: ", player);
                return;
            }

            if (IS_SET(SystemFlags, SYS_WIZLOCKED)) {
                sprintf(buf, "Sorry, no new characters at this time\n\r");
                EnterState(player, STATE_WIZLOCKED);
                return;
            }

            /*
             * move forward creating new character
             */
            CREATE(GET_NAME(player->charData), char, strlen(tmp_name) + 1);
            strcpy(GET_NAME(player->charData), CAP(tmp_name));
            EnterState(player, STATE_CONFIRM_NAME);
        }
        break;

    case STATE_CONFIRM_NAME:
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
            SendOutput("Please type Yes or No? ", player);
            return;
        }

        switch(tolower(*arg)) {
        case 'y':
            SendOutputRaw(echo_on, 4, player);
            SendOutput("New character.\n\r", player);
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            break;
        case 'n':
            SendOutput("Ok, what IS it, then? ", player);
            EnterState(player, STATE_GET_NAME);
            break;
        default:
            SendOutput("Please type Yes or No? ", player);
            break;
        }
        break;

    case STATE_GET_PASSWORD:
        /*
         * get pwd for known player
         */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection );
            return;
        } 

#ifdef TODO
        if (strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            SendOutput("Wrong password.\n\r", player);
            LogPrint(LOG_INFO, "%s entered a wrong password", 
                     GET_NAME(player->charData));
            connClose( player->connection );
            return;
        }
#endif

#ifdef IMPL_SECURITY
        if (top_of_p_table > 0) {
            if (GetMaxLevel(player->charData) >= 58) {
#ifdef TODO
                ProtectedDataLock(player->connection->hostName);
                switch (SecCheck(GET_NAME(player->charData), 
                                 (char *)player->connection->hostName->data)) {
                case -1:
                case 0:
                    SendOutput("Security check reveals invalid site\n\r"
                               "Speak to an implementor to fix problem\n\r"
                               "If you are an implementor, add yourself to"
                               " the\n\r"
                               "Security directory (lib/security)\n\r",
                               player);
                    ProtectedDataUnlock(player->connection->hostname);
                    connClose( player->connection );
                    break;
                default:
                    ProtectedDataUnlock(player->connection->hostname);
                    break;
                }
#endif
            }
        }
#endif
        /*
         * Check if already playing
         */
        for (k = descriptor_list; k; k = k->next) {
            if ((k->character != player->charData) && k->character) {
                /*
                 * check to see if 'character' was switched to by the
                 * one trying to connect
                 */
                if (k->original) {
                    if (GET_NAME(k->original) &&
                        strcasecmp(GET_NAME(k->original),
                                   GET_NAME(player->charData)) == 0) {
                        already_p = 1;
                    }
                } else {
                    /*
                     * No switch has been made
                     */
                    if (GET_NAME(k->character) &&
                        strcasecmp(GET_NAME(k->character),
                                   GET_NAME(player->charData)) == 0) {
                        already_p = 1;
                    }
                }
            }

            if (already_p) {
#ifdef TODO
                /* use k */
                connClose( player->connection );
#endif
                break;
            }
        }

        /*
         * Check if disconnected ...
         */
        for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
            if ((!strcasecmp(GET_NAME(player->charData), GET_NAME(tmp_ch)) 
                 && !tmp_ch->desc && !IS_NPC(tmp_ch)) ||
                (IS_NPC(tmp_ch) && tmp_ch->orig &&
                 !strcasecmp(GET_NAME(player->charData),
                             GET_NAME(tmp_ch->orig)))) {

                SendOutputRaw(echo_on, 6, player);
                SendOutput("Reconnecting.\n\r", player);

                free_char(player->charData);
#ifdef TODO
                tmp_ch->desc = d;
#endif
                player->charData = tmp_ch;
                tmp_ch->specials.timer = 0;

                if (!IS_IMMORTAL(tmp_ch)) {
                    tmp_ch->invis_level = 0;
                }

                if (tmp_ch->orig) {
                    tmp_ch->desc->original = tmp_ch->orig;
                    tmp_ch->orig = 0;
                }

                player->charData->persist = 0;

                if (!IS_IMMORTAL(tmp_ch) || tmp_ch->invis_level <= 58) {
                    act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
#ifdef TODO
                    ProtectedDataLock(player->connection->hostName);
                    LogPrint(LOG_INFO, "%s[%s] has reconnected.", 
                             GET_NAME(player->charData),
                             (char *)player->connection->hostName->data);
                    ProtectedDataUnlock(player->connection->hostName);
#endif
                }
                
                if (player->charData->specials.hostip) {
                    free(player->charData->specials.hostip);
                }

                ProtectedDataLock(player->connection->hostName);
                player->charData->specials.hostip = 
                            strdup((char *)player->connection->hostName->data);
                ProtectedDataUnlock(player->connection->hostName);

                write_char_extra(player->charData);
                EnterState(player, STATE_PLAYING);
                return;
            }
        }

        load_char_extra(player->charData);
        if (player->charData->specials.hostip == NULL) {
            if (!IS_IMMORTAL(player->charData) ||
                player->charData->invis_level <= 58) {
#ifdef TODO
                ProtectedDataLock(player->connection->hostName);
                LogPrint(LOG_INFO, "%s[%s] has connected.\n\r", 
                         GET_NAME(player->charData),
                         (char *)player->connection->hostName->data);
                ProtectedDataUnlock(player->connection->hostName);
#endif
            }
        } else if (!IS_IMMORTAL(player->charData) ||
                   player->charData->invis_level <= 58) {
#ifdef TODO
            ProtectedDataLock(player->connection->hostName);
            LogPrint(LOG_INFO, "%s[%s] has connected - Last connected from[%s]",
                     GET_NAME(player->charData), 
                (char *)player->connection->hostName->data,
                player->charData->specials.hostip);
            ProtectedDataUnlock(player->connection->hostName);
#endif
        }

        if (player->charData->specials.hostip) {
            free(player->charData->specials.hostip);
        }
        ProtectedDataLock(player->connection->hostName);
        player->charData->specials.hostip = 
                        strdup((char *)player->connection->hostName->data);
        ProtectedDataUnlock(player->connection->hostName);
        player->charData->last_tell = NULL;

        write_char_extra(player->charData);
        EnterState(player, STATE_SHOW_MOTD);
        break;

    case STATE_GET_NEW_USER_PASSWORD:
        /*
         * get pwd for new player
         */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            SendOutputRaw(echo_on, 6, player);
            SendOutput("Illegal password.\n\r", player);
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        }

#ifdef TODO
        strncpy(d->pwd, (char *) crypt(arg, player->charData->player.name), 10);
        d->pwd[10] = '\0';
#endif
        SendOutputRaw(echo_on, 6, player);
        EnterState(player, STATE_CONFIRM_PASSWORD);
        break;

    case STATE_CONFIRM_PASSWORD:
        /*
         * get confirmation of new pwd
         */
        /*
         * skip whitespaces
         */
        arg = skip_spaces(arg);
#ifdef TODO
        if (!arg || strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            SendOutputRaw(echo_on, 6, player);

            SendOutput("Passwords don't match.\n\r", player);
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        } 
#endif

        SendOutputRaw(echo_on, 6, player);
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
            SendOutput("That's not a sex..\n\r", player);
            EnterState(player, STATE_CHOOSE_SEX);
            return;
        }

        switch (tolower(*arg)) {
        case 'm':
            /*
             * sex MALE
             */
            player->charData->player.sex = SEX_MALE;
            break;

        case 'f':
            /*
             * sex FEMALE
             */
            player->charData->player.sex = SEX_FEMALE;
            break;

        default:
            SendOutput("That's not a sex..\n\r", player);
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
#ifdef TODO
        while (arg && *arg && index < MAX_STAT) {
            switch(tolower(*arg)) {
            case 's':
                d->stat[index++] = 's';
                break;
            case 'i':
                d->stat[index++] = 'i';
                break;
            case 'w':
                d->stat[index++] = 'w';
                break;
            case 'd':
                d->stat[index++] = 'd';
                break;
            case 'c':
                arg++;
                if(tolower(*arg) == 'o') {
                    d->stat[index++] = 'o';
                    break;
                } else if (tolower(*arg) == 'h') {
                    d->stat[index++] = 'h';
                    break;
                } 
                /* If neither Co or Ch, fall through to default */
            default:
                SendOutput("That was an invalid choice.\n\r", player);
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
#endif

        if (index < MAX_STAT) {
            SendOutput("You did not enter enough legal stats\n\r", player);
            SendOutput("That was an invalid choice.\n\r", player);
            EnterState(player, STATE_CHOOSE_STATS);
            return;
        } 
        
        roll_abilities(player->charData);
        if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
            /*
             * set the AUTH flags
             * (3 chances)
             */
            player->charData->generic = NEWBIE_REQUEST + NEWBIE_CHANCES;
        }

        player->charData->reroll--;
        EnterState(player, STATE_REROLL);
        break;

    case STATE_REROLL:
        arg = skip_spaces(arg);
        player->charData->reroll--;

        if (!arg || tolower(*arg) != 'r') {
            SendOutput("Stats chosen!\n\r", player);

            if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
                EnterState(player, STATE_WAIT_FOR_AUTH);
            } else {
                EnterState(player, STATE_SHOW_CREATION_MENU);
            }
            return;
        } 
        
        roll_abilities(player->charData);
        if (player->charData->reroll != 0) {
            EnterState(player, STATE_REROLL);
            return;
        } 
        
        SendOutput("Your final stats are:\n\r", player);
        sprintf(buf, "STR: -]%s\n\r", STAT_SWORD(GET_STR(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "CON: -]%s\n\r", STAT_SWORD(GET_CON(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "DEX: -]%s\n\r", STAT_SWORD(GET_DEX(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "INT: -]%s\n\r", STAT_SWORD(GET_INT(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "WIS: -]%s\n\r", STAT_SWORD(GET_WIS(player->charData)));
        SendOutput(buf, player);
        sprintf(buf, "CHR: -]%s\n\r", STAT_SWORD(GET_CHR(player->charData)));
        SendOutput(buf, player);
        SendOutput("Stats chosen!", player);

        if (IS_SET(SystemFlags, SYS_REQAPPROVE)) {
            EnterState(player, STATE_WAIT_FOR_AUTH);
        } else {
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
        break;

    case STATE_CHOOSE_MAIN_CLASS:
        arg = skip_spaces(arg);
        player->charData->specials.remortclass = 0;

        if (arg && (pick = atoi(arg)) &&
            HasClass(player->charData, pc_num_class(pick-1))) {
            player->charData->specials.remortclass = pick;
            EnterState(player, STATE_SHOW_CREATION_MENU);
            return;
        } 
        
        SendOutput("\n\rInvalid class picked.\n\r", player);
        EnterState(player, STATE_CHOOSE_MAIN_CLASS);
        break;

    case STATE_CHOOSE_CLASS:
        /*
         * skip whitespaces
         */
        player->charData->player.class = 0;

        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput("Invalid selection!\n\r", player);
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        if( *arg == '?' ) {
            for( i = 0; class_help[i]; i++ ) {
                SendOutput(class_help[i], player);
            }
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        class = atoi(arg);
        race  = GET_RACE(player->charData);
        race = 0;

        if( class <= 0 ) {
            SendOutput("Invalid selection!\n\r", player);
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        for( i = 0, found = FALSE; !found && i < race_choice_count; i++ ) {
            if( race_choice[i].raceNum == race ) {
                /*
                 * Found it.  Time to check the class
                 */
                if( class > race_choice[i].classCount ) {
                    SendOutput("Invalid selection!\n\r", player);
                    EnterState(player, STATE_CHOOSE_CLASS);
                    return;
                }

                /* Class choice is valid */
                player->charData->player.class = 
                    race_choice[i].classesAvail[class - 1];
                found = TRUE;
            }
        }
                    
        if (player->charData->player.class == 0) {
            SendOutput("Invalid selection!\n\r", player);
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        if( found ) {
            if (HasClass(player->charData, CLASS_MAGIC_USER)) {
                EnterState(player, STATE_CHECK_MAGE_TYPE);
                return;
            }

            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
            SendOutput("Your race seems to be incorrect, please reselect\n\r", 
                       player);
            LogPrintNoArg(LOG_NOTICE, "Couldn't find a race in creation, "
                                      "screwy!!");
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
        break;

    case STATE_WAIT_FOR_AUTH:
        if (player->charData->generic >= NEWBIE_START) {
            /*
             ** now that classes are set, initialize
             */
            init_char(player->charData);
            /*
             * create an entry in the file
             */
#ifdef TODO
            d->pos = create_entry(GET_NAME(player->charData));
#endif
            save_char(player->charData, AUTO_RENT);
            EnterState(player, STATE_SHOW_MOTD);
            return;
        } 
        
        if (player->charData->generic >= NEWBIE_REQUEST) {
#ifdef TODO
            ProtectedDataLock(player->connection->hostName);
            sprintf(buf, "%s [%s] new player.", GET_NAME(player->charData),
                    (char *)player->connection->hostName->data);
            ProtectedDataUnlock(player->connection->hostName);
#endif
            log_sev(buf, 1);
            /*
             * I decided to give them another chance.  -Steppenwolf
             * They blew it. -DM
             */
            if (top_of_p_table > 0) {
                sprintf(buf, "Type Authorize %s [Yes | No | Message]",
                        GET_NAME(player->charData));
                log_sev(buf, 1);
                log_sev("type 'Wizhelp Authorize' for other commands", 1);
            } else {
                LogPrintNoArg(LOG_NOTICE, "Initial character.  Authorized "
                                          "Automatically");
                player->charData->generic = NEWBIE_START + 5;
            }

            /*
             **  enough for gods.  now player is told to shut up.
             */
            /* NEWBIE_START == 3 == 3 chances */
            player->charData->generic--;
            sprintf(buf, "Please wait. You have %d requests remaining.\n\r",
                    player->charData->generic);
            SendOutput(buf, player);
            if (player->charData->generic == 0) {
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
            player->charData->player.class -= CLASS_MAGIC_USER;
            player->charData->player.class += CLASS_SORCERER;
        }
        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    case STATE_SHOW_MOTD:
        /*
         * read CR after printing motd
         */
        if (IS_IMMORTAL(player->charData)) {
            EnterState(player, STATE_SHOW_WMOTD);
            break;
        }

        if (player->charData->term != 0) {
            ScreenOff(player->charData);
        }


#ifdef TODO
        ProtectedDataLock(player->connection->hostName);
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || 
             SiteLock((char *)player->connection->hostName->data)) &&
            !IS_IMMORTAL(player->charData)) {
            ProtectedDataUnlock(player->connection->hostName);
            sprintf(buf, "Sorry, the game is locked up for repair or your "
                         "site is banned.\n\r");
            SendOutput(buf, player);
            EnterState(player, STATE_WIZLOCKED);
        } else {
            ProtectedDataUnlock(player->connection->hostName);
            EnterState(player, STATE_SHOW_LOGIN_MENU);
        }
#endif
        break;

    case STATE_SHOW_WMOTD:
        /*
         * read CR after printing motd
         */
#ifdef TODO
        ProtectedDataLock(player->connection->hostName);
        if ((IS_SET(SystemFlags, SYS_WIZLOCKED) || 
             SiteLock((char *)player->connection->hostName->data)) &&
            !IS_IMMORTAL(player->charData)) {
            ProtectedDataUnlock(player->connection->hostName);
            sprintf(buf, "Sorry, the game is locked up for repair or your "
                         "site is banned.\n\r");
            SendOutput(buf, player);
            EnterState(player, STATE_WIZLOCKED);
        } else {
            ProtectedDataUnlock(player->connection->hostName);
            EnterState(player, STATE_SHOW_LOGIN_MENU);
        }
#endif
        break;

    case STATE_WIZLOCKED:
        connClose( player->connection );
        break;

    case STATE_DELETE_USER:
        arg = skip_spaces(arg);
        if (arg && !strcasecmp(arg, "yes") && 
            strcasecmp("Guest", GET_NAME(player->charData))) {
            LogPrint(LOG_INFO, "%s just killed theirself!", 
                     GET_NAME(player->charData));
            for (i = 0; i <= top_of_p_table; i++) {
                if (!strcasecmp(player_table[i].name, 
                                GET_NAME(player->charData))) {
                    if (player_table[i].name) {
                        free(player_table[i].name);
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
            sprintf(buf, "rent/%s", lower(GET_NAME(player->charData)));
            remove(buf);
            sprintf(buf, "rent/%s.aux", GET_NAME(player->charData));
            remove(buf);
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
            SendOutput("Wrong option.\n\r", player);
            EnterState(player, STATE_SHOW_LOGIN_MENU);
            break;
        }

        switch (tolower(*arg)) {
        case '0':
            connClose( player->connection );
            break;

        case '1':
            reset_char(player->charData);
            total_connections++;
            if (!IS_IMMORTAL(player->charData) ||
                player->charData->invis_level <= 58) {
                LogPrint(LOG_INFO, "Loading %s's equipment", 
                         player->charData->player.name);
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
            load_char_objs(player->charData);

            save_char(player->charData, AUTO_RENT);
            SendOutput(WELC_MESSG, player);
            player->charData->next = character_list;
            character_list = player->charData;
            if (player->charData->in_room == NOWHERE ||
                player->charData->in_room == AUTO_RENT) {
                if (!IS_IMMORTAL(player->charData)) {
                    if (player->charData->specials.start_room <= 0) {
                        if (GET_RACE(player->charData) == RACE_HALFLING) {
                            char_to_room(player->charData, 1103);
                            player->charData->player.hometown = 1103;
                        } else {
                            char_to_room(player->charData, 3001);
                            player->charData->player.hometown = 3001;
                        }
                    } else {
                        char_to_room(player->charData,
                                     player->charData->specials.start_room);
                        player->charData->player.hometown =
                            player->charData->specials.start_room;
                    }
                } else {
                    if (player->charData->specials.start_room <= NOWHERE) {
                        char_to_room(player->charData, 1000);
                        player->charData->player.hometown = 1000;
                    } else {
                        if (real_roomp(player->charData->specials.start_room)) {
                            char_to_room(player->charData,
                                         player->charData->specials.start_room);
                            player->charData->player.hometown =
                                player->charData->specials.start_room;
                        } else {
                            char_to_room(player->charData, 1000);
                            player->charData->player.hometown = 1000;
                        }
                    }
                }
            } else if (real_roomp(player->charData->in_room)) {
                char_to_room(player->charData, player->charData->in_room);
                player->charData->player.hometown = player->charData->in_room;
            } else {
                char_to_room(player->charData, 3001);
                player->charData->player.hometown = 3001;
            }

            player->charData->specials.tick = plr_tick_count++;
            if (plr_tick_count == PLR_TICK_WRAP) {
                plr_tick_count = 0;
            }
            act("$n has entered the game.", TRUE, player->charData, 0, 0, 
                TO_ROOM);
            EnterState(player, STATE_PLAYING);

            if (!GetMaxLevel(player->charData)) {
                do_start(player->charData);
            }
            do_look(player->charData, NULL, 15);
            /*
             * do an auction check, grant reimbs as needed
             */
            if (player->charData->specials.auction) {
                obj = player->charData->specials.auction;
                player->charData->specials.auction = 0;
                obj->equipped_by = 0;
                obj->eq_pos = -1;

                obj_to_char(obj, player->charData);
                SendOutput("Your item is returned to you.\n\r", player);
                do_save(player->charData, "", 0);
            }
            if (player->charData->specials.minbid) {
                GET_GOLD(player->charData) += player->charData->specials.minbid;
                player->charData->specials.minbid = 0;
                SendOutput("You are returned your deposit for this "
                             "auction.\n\r", player);
                do_save(player->charData, "", 0);
            }
#ifdef TODO
            d->prompt_mode = 1;
#endif
            break;

        case '2':
            SendOutput("Enter a text you'd like others to see when they look "
                       "at you.\n\r", player);
            if (player->charData->player.description) {
                SendOutput("Old description :\n\r", player);
                SendOutput(player->charData->player.description, player);
                if (player->charData->player.description) {
                    free(player->charData->player.description);
                }
                player->charData->player.description = 0;
            }
#ifdef TODO
            d->str = &player->charData->player.description;
            d->max_str = 240;
#endif
            EnterState(player, STATE_EDIT_EXTRA_DESCR);
            break;

        case '3':
            SendOutput(STORY, player);
            EnterState(player, STATE_PRESS_ENTER);
            break;

        case '4':
            SendOutput(credits, player);
            EnterState(player, STATE_PRESS_ENTER);
            break;

        case '5':
            EnterState(player, STATE_GET_NEW_PASSWORD);
            break;

        case 'k':
            EnterState(player, STATE_DELETE_USER);
            break;

        default:
            SendOutput("Wrong option.\n\r", player);
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
            SendOutputRaw(echo_on, 6, player);

            SendOutput("Illegal password.\n\r", player);
            EnterState(player, STATE_GET_NEW_PASSWORD);
            return;
        }

#ifdef TODO
        strncpy(d->pwd, (char *) crypt(arg, player->charData->player.name), 10);
        *(d->pwd + 10) = '\0';
#endif
        SendOutputRaw(echo_on, 6, player);

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
#ifdef TODO
        if (!arg || strncmp((char *) crypt(arg, d->pwd), d->pwd, 10)) {
            SendOutputRaw(echo_on, 6, player);
            SendOutput("Passwords don't match.\n\r", player);
            EnterState(player, STATE_GET_NEW_PASSWORD);
            return;
        }
#endif

        SendOutputRaw(echo_on, 6, player);

        SendOutput("\n\rDone. You must enter the game to make the change "
                   "final\n\r", player);

        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    default:
        LogPrint(LOG_NOTICE, "Nanny: illegal state of con'ness (%d)", 
                 player->state);
        SendOutput("The mud has lost its brain on your connection, please "
                   "reconnect.\n\r", player);
        connClose( player->connection );
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
                sprintclasses((unsigned)race_choice[i].classesAvail[j],
                              buf2);
                strcat(buf, buf2);
                strcat(buf, "\n\r");
                SendOutput(buf, player);
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
    char            buf[255],
                    buf2[254];

    SendOutput("                                  Level Limits\n\r", player);
    sprintf(buf, "%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s "
                 "%-3s %-3s %-3s\n\r",
            "#", "Race", "ma", "cl", "wa", "th", "dr", "mk", "ba", "so",
            "pa", "ra", "ps", "ne");
    SendOutput(buf, player);

    for (i = 0; i < race_choice_count; i++) {
        sprintf(buf, "%s$c000W%-3d)$c0007 %-15s", 
                (race_choice[i].raceNum == RACE_DROW ?
                 "$c000WThe Races Listed below may have some racials hatreds."
                 "  Advanced players only.\n\r" : ""),
                i + 1, races[race_choice[i].raceNum].racename);
        /*
         * show level limits 
         */
        for (j = 0; j < MAX_CLASS; j++) {
            sprintf(buf2, " %-3d", races[race_choice[i].raceNum].racialMax[j]);
            strcat(buf, buf2);
        }

        strcat(buf, "\n\r");
        SendOutput(buf, player);
    }

    SendOutput("$c000gma=magic user, cl=cleric, wa=warrior,th=thief,"
               "dr=druid,mk=monk\n\r", player);
    SendOutput("$c000gba=barbarian,so=sorcerer,pa=paladin,ra=ranger,ps=psi,"
               "ne=necromancer\n\r\n\r", player);
}

void LoginSendBanner( PlayerStruct_t *player )
{
    SendOutput(login, player);
    SendOutput("If you're using Tintin or Lyntin, your client may not display "
               "the password\n\r", player);
    SendOutput("sequence unless you change your settings. Please do not be "
               "discouraged.\n\r\n\r", player);
    EnterState(player, STATE_GET_NAME);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
