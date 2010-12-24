/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008, 2010 Gavin Hurlbut
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
 * Copyright 2008, 2010 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Login & character creation state machine
 */

#include "config.h"
#include "environment.h"
#include "memory.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>
#include "protected_data.h"
#include <time.h>
#include <sys/time.h>

#include "oldstructs.h"
#include "oldutils.h"
#include "oldexterns.h"
#include "interthread.h"
#include "protos.h"

#define MAX_NAME_LENGTH 20
#define MAX_EMAIL_LENGTH 300

void EnterState(PlayerStruct_t *player, PlayerState_t newstate);
void ShowCreationMenu(PlayerStruct_t *player);
void ShowAccountMenu(PlayerStruct_t *player);
void ShowPlayerListMenu(PlayerStruct_t *player);
void DoCreationMenu( PlayerStruct_t *player, char arg );
void DoAccountMenu( PlayerStruct_t *player, char arg );
void roll_abilities(PlayerStruct_t *player);
int SiteLock(char *site);
void CreateSendConfirmEmail( PlayerStruct_t *player );
void RollAbilities( PlayerStruct_t *player );

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

bool parseEmail(char *arg, char *name);
bool parseName(char *arg, char *name);
int checkAssName(char *name);

struct banned_user *bannedUsers = NULL; 
int             bannedUserCount; 


/**
 * @brief Trims the leading spaces and copies the name
 * @param arg input string
 * @param name output name
 * @return FALSE if the email is invalid, TRUE if valid
 *
 * An invalid email is indicated if the email is too long, or has non-alpha 
 * characters in it (other than @, _, - and .), or is empty
 */
bool parseEmail(char *arg, char *name)
{
    int             i;
    char           *begin;

    /*
     * skip whitespaces
     */
    arg = skip_spaces(arg);
    if( !arg ) {
        return( FALSE );
    }
    begin = arg;

    for (i = 0; (*name = *arg); arg++, i++, name++) {
        if ((*arg < 0) || !(isalpha((int)*arg) || *arg == '@' || *arg == '.' ||
                            *arg == '_' || *arg == '-') ||
            i > MAX_EMAIL_LENGTH) {
            return( FALSE );
        }
    }

    if (!i) {
        return ( FALSE );
    }

    if( !(arg = strchr(begin, '@')) ) {
        return( FALSE );
    }

    if( !strchr(arg, '.') ) {
        return( FALSE );
    }
    return( TRUE );
}

/**
 * @brief Trims the leading spaces and copies the name
 * @param arg input string
 * @param name output name
 * @return FALSE if the name is invalid, TRUE if valid
 *
 * An invalid name is indicated if the name is too long, or has non-alpha 
 * characters in it, or is empty
 */
bool parseName(char *arg, char *name)
{
    int             i;

    /*
     * skip whitespaces
     */
    arg = skip_spaces(arg);
    if( !arg ) {
        return( FALSE );
    }

    for (i = 0; (*name = *arg); arg++, i++, name++) {
        if ((*arg < 0) || !isalpha((int)*arg) || i > MAX_NAME_LENGTH) {
            return( FALSE );
        }
    }

    if (!i) {
        return ( FALSE );
    }
    return( TRUE );
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
int checkAssName(char *name)
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

void ShowPlayerListMenu(PlayerStruct_t *player)
{
    PlayerPC_t     *pcs;
    PlayerPC_t     *pc;

    if( player->pcs ) {
        for( pc = player->pcs; pc->id; pc++ ) {
            DestroyAttributes( pc );
            memfree( pc->name );
        }
        memfree( player->pcs );
    }
    player->pcs = pb_get_pc_list( player->account->id );

    pcs = player->pcs;
    if( !pcs ) {
        SendOutput(player, "Currently no players, create one.\n\r\n\r");
    } else {
        SendOutput(player, "PC Name\n\r=======\n\r");
        for( pc = pcs; pc->id; pc++ ) {
            SendOutput(player, "%s\n\r", pc->name);
        }
        SendOutput(player, "\n\r");
    }

    SendOutput(player, "\n\rHit enter to continue\n\r");
}

void ShowCreationMenu(PlayerStruct_t *player)
{
    PlayerPC_t     *pc;
    int             rerolls;

    pc = player->pc;

    SendOutput(player, "$c0009-=$c0015Havok Character Creation Menu"
                       "$c0009=-\n\r\n\r");

    rerolls = GetAttributeInt(pc, "rerolls", "core-pc");
    SendOutput(player, "Current Ability Roll: %d, %d, %d, %d, %d, %d\n\r"
                       "Available Rerolls: %d\n\r\n\r", 
                       GetAttributeInt(pc, "roll1",   "core-pc"),
                       GetAttributeInt(pc, "roll2",   "core-pc"),
                       GetAttributeInt(pc, "roll3",   "core-pc"),
                       GetAttributeInt(pc, "roll4",   "core-pc"),
                       GetAttributeInt(pc, "roll5",   "core-pc"),
                       GetAttributeInt(pc, "roll6",   "core-pc"),
                       rerolls);

    SendOutput(player, "$c00151) $c0012Name. [$c0015%s$c0012]\n\r",
                       (pc && pc->name ? pc->name : "not chosen"));
    SendOutput(player, "$c00152) $c0012Gender. [$c0015%s$c0012]\n\r",
                       Sex[GetAttributeInt(pc, "sex", "core-pc")]);
    SendOutput(player, "$c00153) $c0012Race. [$c0015%s$c0012]\n\r",
                           "TODO");
    SendOutput(player, "$c00154) $c0012Class. [$c0015%s$c0012]\n\r", "TODO" );

    SendOutput(player, "$c00155) $c0012Character Stats. [$c0015%s$c0012]\n\r",
                       "TODO");
    SendOutput(player, "$c00156) $c0012Alignment. [$c0015%s$c0012]\n\r\n\r",
                       "TODO");

    if( rerolls ) {
        SendOutput(player, "$c0015R) $c0012Reroll Ability Scores.\n\r\n\r" );
    }

    SendOutput(player, "$c0015D) $c0012Done!\n\r\n\r");
    SendOutput(player, "$c0011Please pick an option: \n\r");
}

void ShowAccountMenu(PlayerStruct_t *player)
{
    SendOutput(player, "\n\r\n\r$c0009-=$c0015Havok Account Menu [%s]"
                       "$c0009=-\n\r\n\r", player->account->email);
    SendOutput(player, "$c00151) $c0012ANSI Colors.\n\r");
    SendOutput(player, "$c00152) $c0012Change your password.\n\r");
    SendOutput(player, "$c00153) $c0012View the MOTD.\n\r");
    SendOutput(player, "$c00154) $c0012View the credits.\n\r");
    if( player->account->confirmed ) {
        SendOutput(player, "$c00155) $c0012List characters.\n\r");
        SendOutput(player, "$c00156) $c0012Create a new character.\n\r");
        SendOutput(player, "$c00157) $c0012Play an existing character.\n\r");
    }

    if( !player->account->confirmed ) {
        SendOutput(player, "$c0015E) $c0012Enter email confirmation code.\n\r");
        SendOutput(player, "$c0015R) $c0012Resend the confirmation email.\n\r");
    }
    SendOutput(player, "$c0015Q) $c0012Quit!\n\r\n\r");
    SendOutput(player, "$c0011Please pick an option: \n\r");
}


void EnterState(PlayerStruct_t *player, PlayerState_t newstate)
{
    struct char_data   *ch;

    if( !player ) {
        return;
    }

    ch = player->charData;

#ifdef DEBUG_STATE
    LogPrint( LOG_INFO, "Entering state %d from %d", newstate, player->state );
#endif

    switch( newstate ) {
    case STATE_INITIAL:
	break;
    case STATE_GET_EMAIL:
        SendOutput(player, "What is your account name (email address)? ");
        break;
    case STATE_CONFIRM_EMAIL:
        SendOutput(player, "Did I get that right, %s (Y/N)? ", 
                           player->account->email );
        break;
    case STATE_GET_NEW_USER_PASSWORD:
        SendOutput(player, "Give me a password for %s: ", 
                           player->account->email);
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_GET_PASSWORD:
        SendOutput(player, "Password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_CONFIRM_PASSWORD:
    case STATE_CONFIRM_NEW_PASSWORD:
        SendOutput(player, "Please retype password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_CHOOSE_ANSI:
        SendOutput(player, "Would you like ansi colors? (Yes or No)");
        break;
    case STATE_SHOW_MOTD:
#if 1
        if( !motd ) {
            motd = memstrlink("MOTD will be eventually read from the "
                              "database.\n\r\n\r");
        }
#endif
        SendOutput(player, motd);
        SendOutput(player, "\n\r\n[PRESS RETURN]");
        break;
    case STATE_SHOW_WMOTD:
#if 1
        if( !wmotd ) {
            wmotd = memstrlink("WMOTD will be eventually read from the "
                               "database.\n\r\n\r");
        }
#endif
        SendOutput(player, wmotd);
        SendOutput(player, "\n\r\n[PRESS RETURN]");
        break;
    case STATE_SHOW_CREDITS:
#if 1
        if( !credits ) {
            credits = memstrlink("Credits will be eventually read from the "
                                 "database.\n\r\n\r");
        }
#endif
        SendOutput(player, credits);
        SendOutput(player, "\n\r\n[PRESS RETURN]");
        break;
    case STATE_WIZLOCKED:
        SendOutput(player, "Goodbye.\n\r");
        break;
    case STATE_SHOW_ACCOUNT_MENU:
        ShowAccountMenu(player);
        break;
    case STATE_SHOW_PLAYER_LIST:
        ShowPlayerListMenu(player);
        break;
    case STATE_GET_NEW_PASSWORD:
        SendOutput(player, "Enter a new password: ");
        SendOutputRaw(player, echo_off, 4);
        break;
    case STATE_ENTER_CONFIRM_CODE:
        SendOutput(player, "Please enter the confirmation code you were "
                           "emailed:  ");
        break;
    case STATE_RESEND_CONFIRM_EMAIL:
        if( !player->account->confcode || !*player->account->confcode ) {
            SendOutput(player, "Sending your confirmation email...\n\r");
        } else {
            SendOutput(player, "Resending your confirmation email...\n\r");
        }
        CreateSendConfirmEmail(player);
        break;
    case STATE_REROLL_ABILITIES:
        RollAbilities(player);
        pb_save_pc( player->pc );
        newstate = STATE_SHOW_CREATION_MENU;
        /* Fall into the STATE_SHOW_CREATION_MENU state */
    case STATE_SHOW_CREATION_MENU:
        ShowCreationMenu(player);
        break;
    case STATE_CHOOSE_NAME:
        SendOutput(player, "Choose the name of your new PC: ");
        break;
    case STATE_CHOOSE_SEX:
        SendOutput(player, "What is your sex (M)ale/(F)emale/(N)eutral) ? ");
        break;

    /* sorted to here */
    case STATE_CHOOSE_RACE:
        SendOutput(player, "For help type '?'- will list level limits. \n\r"
                           " RACE:  ");
        ch->player.class = 0;
        ch->specials.remortclass = 0;
        break;
    case STATE_CHOOSE_CLASS:
        GET_ALIGNMENT(ch) = 0;
        GET_CON(ch) = 0;
        SendOutput(player, "\n\rSelect your class now.\n\r");
        SendOutput(player, "Enter ? for help.\n\r\n\rClass :");
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

        SendOutput(player, "Please select your alignment "
                           "($c000WGood$c000w/Neutral$c000w/"
                           "$c000REvil$c000w)");
        break;

    case STATE_PLAYING:
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
    case STATE_DELETE_USER:
        SendOutput(player, "Are you sure you want to delete yourself? "
                           "(yes/no) ");
        break;
    default:
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
    int             class,
                    race,
                    found = 0,
                    index = 0;
    char            tmp_name[20];
    char           *tmp;
    int             pick = 0;
    InputStateItem_t   *stateItem;
    PlayerPC_t     *pc;
    PlayerPC_t     *tempPc;
    char            attrib[256];
    int             i;
    int             roll[6];

    pc = player->pc;
    SendOutputRaw(player, echo_on, 6);

    switch (player->state) {
    case STATE_INITIAL:
#if 0
        if (!ch) {
            player->charData = CREATE(struct char_data);
            ch = player->charData;
            ch->playerDesc = player;
        }
#endif
        EnterState(player, STATE_GET_EMAIL);
        break;

    case STATE_GET_EMAIL:
        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection, UNLOCKED );
            return;
        } 
        
        if (!parseEmail(arg, tmp_name)) {
            SendOutput(player, "Illegal email address, please try again.\r\n");
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

        if( player->account ) {
            if( player->account->email ) {
                memfree( player->account->email );
            }
            if( player->account->pwd ) {
                memfree( player->account->pwd );
            }
            if( player->account->confcode ) {
                memfree( player->account->confcode );
            }
            memfree( player->account );
            player->account = NULL;
        }

        player->account = pb_load_account(tmp_name);
        if( player->account ) {
            /*
             * connecting an existing account ...
             */
            EnterState(player, STATE_GET_PASSWORD);
        } else {
            /*
             * player unknown gotta make a new
             */
            if (IS_SET(SystemFlags, SYS_WIZLOCKED)) {
                SendOutput(player, "Sorry, no new accounts at this time\n\r");
                EnterState(player, STATE_WIZLOCKED);
                return;
            }

            /*
             * move forward creating new character
             */
            player->account = CREATE(PlayerAccount_t);

            player->account->email = memstrlink(tmp_name);
            EnterState(player, STATE_CONFIRM_EMAIL);
        }
        break;

    case STATE_CONFIRM_EMAIL:
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
            SendOutput(player, "New account.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            break;
        case 'n':
            SendOutput(player, "Ok, what IS it, then? ");
            EnterState(player, STATE_GET_EMAIL);
            break;
        default:
            SendOutput(player, "Please type Yes or No? ");
            break;
        }
        break;

    case STATE_GET_NEW_USER_PASSWORD:
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            SendOutputRaw(player, echo_on, 6);
            SendOutput(player, "Illegal password.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        }

        if( player->account->pwd ) {
            memfree( player->account->pwd );
        }

        tmp = (char *)crypt(arg, player->account->email);
        tmp[10] = '\0';
        player->account->pwd = memstrlink(tmp);

        SendOutputRaw(player, echo_on, 6);
        EnterState(player, STATE_CONFIRM_PASSWORD);
        break;

    case STATE_CONFIRM_PASSWORD:
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *)crypt(arg, player->account->pwd), 
                            player->account->pwd, 10)) {
            SendOutputRaw(player, echo_on, 6);

            SendOutput(player, "Passwords don't match.\n\r");
            EnterState(player, STATE_GET_NEW_USER_PASSWORD);
            return;
        } 

        SendOutputRaw(player, echo_on, 6);
        EnterState(player, STATE_CHOOSE_ANSI);
        break;

    case STATE_GET_PASSWORD:
        arg = skip_spaces(arg);
        if (!arg) {
            connClose( player->connection, UNLOCKED );
            return;
        } 

        if (strncmp((char *)crypt(arg, player->account->pwd), 
                            player->account->pwd, 10)) {
            SendOutput(player, "Wrong password.\n\r");
            LogPrint(LOG_INFO, "%s entered a wrong password", player->account);
            connClose( player->connection, UNLOCKED );
            return;
        }

#if 0
        if (ch->specials.hostip == NULL) {
            if (!IS_IMMORTAL(ch) ||
                ch->invis_level <= 58) {
#endif
                ProtectedDataLock(player->connection->hostName);
                LogPrint(LOG_INFO, "%s[%s] has connected.", 
                         player->account->email,
                         (char *)player->connection->hostName->data);
                ProtectedDataUnlock(player->connection->hostName);
#if 0
            }
        } else if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            ProtectedDataLock(player->connection->hostName);
            LogPrint(LOG_INFO, "%s[%s] has connected - Last connected from[%s]",
                     player->account->email, 
                     (char *)player->connection->hostName->data,
                     ch->specials.hostip);
            ProtectedDataUnlock(player->connection->hostName);
        }

        if (ch->specials.hostip) {
            memfree(ch->specials.hostip);
        }
        ProtectedDataLock(player->connection->hostName);
        ch->specials.hostip = 
                        memstrlink((char *)player->connection->hostName->data);
        ProtectedDataUnlock(player->connection->hostName);
        ch->last_tell = NULL;
#endif

        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
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
            player->account->ansi = TRUE;

            SendOutput(player, "$c0012A$c0010N$c0011S$c0014I$c0007 colors "
                               "enabled.\n\r\n\r");
            break;

        case 'n':
            player->account->ansi = FALSE;
            break;

        default:
            SendOutput(player, "Please type Yes or No.\n\r"
                               "Would you like ANSI colors? :");
            return;
            break;
        }
        pb_save_account(player->account);
        if( !player->account->confcode || !*player->account->confcode ) {
            EnterState(player, STATE_RESEND_CONFIRM_EMAIL);
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        } else {
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        }
        break;

    case STATE_SHOW_MOTD:
        /*
         * read CR after printing motd
         */
#if 0
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
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        }
#else
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
#endif
        break;

    case STATE_SHOW_WMOTD:
        /*
         * read CR after printing motd
         */
#if 0
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
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        }
#else
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
#endif
        break;

    case STATE_SHOW_CREDITS:
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        break;

    case STATE_WIZLOCKED:
        connClose( player->connection, UNLOCKED );
        break;

    case STATE_PRESS_ENTER:
        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    case STATE_SHOW_ACCOUNT_MENU:
        arg = skip_spaces(arg);
        if( !arg ) {
            ShowAccountMenu(player);
            SendOutput(player, "Invalid Choice.. Try again..\n\r");
            return;
        }

        DoAccountMenu(player, *arg);
        break;

    case STATE_SHOW_PLAYER_LIST:
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        break;

    case STATE_GET_NEW_PASSWORD:
        arg = skip_spaces(arg);
        if (!arg || strlen(arg) > 10) {
            SendOutputRaw(player, echo_on, 6);

            SendOutput(player, "Illegal password.\n\r");
            EnterState(player, STATE_GET_NEW_PASSWORD);
            return;
        }

        if( player->account->newpwd ) {
            memfree( player->account->newpwd );
        }

        tmp = (char *)crypt(arg, player->account->email);
        tmp[10] = '\0';
        player->account->newpwd = memstrlink(tmp);

        SendOutputRaw(player, echo_on, 6);
        EnterState(player, STATE_CONFIRM_NEW_PASSWORD);
        break;

    case STATE_CONFIRM_NEW_PASSWORD:
        arg = skip_spaces(arg);
        if (!arg || strncmp((char *)crypt(arg, player->account->newpwd), 
                            player->account->newpwd, 10)) {
            SendOutputRaw(player, echo_on, 6);

            SendOutput(player, "Passwords don't match.\n\r");
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
            return;
        } 

        SendOutputRaw(player, echo_on, 6);

        if( player->account->pwd ) {
            memfree( player->account->pwd );
        }

        player->account->pwd = player->account->newpwd;
        player->account->newpwd = NULL;

        SendOutput(player, "Password changed...\n\r");

        pb_save_account(player->account);
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        break;

    case STATE_ENTER_CONFIRM_CODE:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Entry aborted\n\r");
            EnterState( player, STATE_SHOW_ACCOUNT_MENU);
            return;
        }

        if( player->account->confcode && *player->account->confcode &&
            !strcasecmp(arg, player->account->confcode) ) {
            SendOutput(player, "\n\rYour email is now confirmed, you can now "
                               "play.  Thank you!\n\r");
            player->account->confirmed = TRUE;
            memfree( player->account->confcode );
            player->account->confcode = NULL;
            pb_save_account( player->account );
        } else {
            SendOutput(player, "\n\rConfirmation code does not match our "
                               "records.  Please try again,\n\r"
                               "or resend the confirmation email.\n\r" );
            player->account->confirmed = FALSE;
            pb_save_account( player->account );
        }
        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        break;

    /* Creation rework */

    case STATE_SHOW_CREATION_MENU:
        arg = skip_spaces(arg);
        if( !arg ) {
            ShowCreationMenu(player);
            SendOutput(player, "Invalid Choice.. Try again..\n\r");
            return;
        }

        DoCreationMenu(player, *arg);
        break;


    case STATE_CHOOSE_NAME:
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Never mind then.\n\r");
            EnterState(player, STATE_SHOW_CREATION_MENU);
            return;
        } 
        
        if ( (tempPc = pb_find_pc( arg ) ) ) {
            if( tempPc->account_id != player->account->id ) {
                SendOutput(player, "Name taken.\n\r");
                DestroyAttributes( tempPc );
                memfree( tempPc->name );
                memfree( tempPc );
                EnterState(player, STATE_CHOOSE_NAME);
                return;
            }
            
            player->pc = pb_load_pc(player->account->id, tempPc->id);
            DestroyAttributes( tempPc );
            memfree( tempPc->name );
            memfree( tempPc );

            if( GetAttributeBool(player->pc, "complete", "core-pc") ) {
                SendOutput(player, "That PC is completed!\n\r");

                DestroyAttributes( player->pc );
                memfree( player->pc->name );
                memfree( player->pc );
                player->pc = NULL;
                EnterState(player, STATE_SHOW_ACCOUNT_MENU);
                return;
            }
            EnterState(player, STATE_SHOW_CREATION_MENU);
            return;
        } 
        
        /* TODO: check for banned names */
        if( pc && pc->name ) {
            memfree( pc->name );
            pc->name = memstrlink( arg );
        } else if( !pc ) {
            pc = CREATE(PlayerPC_t);
            player->pc = pc;
            pc->account_id = player->account->id;
            pc->name = memstrlink( arg );
            SetAttributeBool( pc, "complete", "core-pc", FALSE );

            tmp = pb_get_setting("MaxReroll");
            if( !tmp ) {
                SetAttributeInt( pc, "rerolls", "core-pc", 1 );
            } else {
                SetAttributeInt( pc, "rerolls", "core-pc", atoi(tmp) + 1 );
                memfree( tmp );
            }

            RollAbilities( player );
        }
        pb_save_pc( pc );
        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    case STATE_CHOOSE_SEX:
        /*
         * query sex of new user
         */
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "That's not a sex..\n\r");
            EnterState(player, STATE_CHOOSE_SEX);
            return;
        }

        switch (tolower(*arg)) {
        case 'm':
            SetAttributeInt( pc, "sex", "core-pc", SEX_MALE );
            break;

        case 'f':
            SetAttributeInt( pc, "sex", "core-pc", SEX_FEMALE );
            break;

        case 'n':
            SetAttributeInt( pc, "sex", "core-pc", SEX_NEUTRAL );
            break;

        default:
            SendOutput(player, "That's not a sex..\n\r");
            EnterState(player, STATE_CHOOSE_SEX);
            return;
            break;
        }

        pb_save_pc( pc );
        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    case STATE_REROLL_ABILITIES:
        /* Shouldn't get here, but if we do, show the creation menu */
        EnterState(player, STATE_SHOW_CREATION_MENU);
        break;

    /* Sorted to here */

    case STATE_CHOOSE_ALIGNMENT:
#if 0
        arg = skip_spaces(arg);
        if( !arg ) {
            SendOutput(player, "Please select a alignment.\n\r");
            return;
        }

        switch (tolower(*arg)) {
        case 'n':
            GET_ALIGNMENT(ch) = 1;
            SendOutput(player, "You have chosen to be Neutral in "
                               "alignment.\n\r\n\r");
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;
        case 'g':
            GET_ALIGNMENT(ch) = 1000;
            SendOutput(player, "You have chosen to be a follower of "
                               "light.\n\r\n\r");
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;
        case 'e':
            GET_ALIGNMENT(ch) = -1000;
            SendOutput(player, "You have chosen the dark side.\n\r\n\r");
            EnterState(player, STATE_SHOW_CREATION_MENU);
            break;

        default:
            SendOutput(player, "Please select a alignment.\n\r");
            break;
        }
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;

    case STATE_CHOOSE_RACE:
#if 0
        ch->reroll = 20;
        arg = skip_spaces(arg);
        if (!arg) {
            EnterState(player, STATE_CHOOSE_RACE);
            return;
        } 
        
#if 0
        tmpi = atoi(arg);
        if (tmpi >= 1 && tmpi <= race_choice_count) {
            GET_RACE(ch) = race_choice[tmpi - 1].raceNum;
            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
#endif
            SendOutput(player, "\n\rThat's not a race.\n\rRACE?:");
            EnterState(player, STATE_CHOOSE_RACE);
#if 0
        }
#endif
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;


    case STATE_CHOOSE_STATS:
#if 0
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
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;

    case STATE_CHOOSE_CLASS:
#if 0
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

        class = atoi(arg);
        race  = GET_RACE(ch);

        if( class <= 0 ) {
            SendOutput(player, "Invalid selection!\n\r");
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

#if 0
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
#endif
                    
        if (ch->player.class == 0) {
            SendOutput(player, "Invalid selection!\n\r");
            EnterState(player, STATE_CHOOSE_CLASS);
            return;
        }

        if( found ) {
            EnterState(player, STATE_SHOW_CREATION_MENU);
        } else {
            SendOutput(player, "Your race seems to be incorrect, please "
                               "reselect\n\r");
            LogPrintNoArg(LOG_NOTICE, "Couldn't find a race in creation, "
                                      "screwy!!");
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;

    case STATE_WAIT_FOR_AUTH:
#if 0
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
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;

    case STATE_DELETE_USER:
#if 0
        arg = skip_spaces(arg);
        if (arg && !strcasecmp(arg, "yes") && 
            strcasecmp("Guest", GET_NAME(ch))) {
            LogPrint(LOG_INFO, "%s just killed theirself!", GET_NAME(ch));
            connClose( player->connection, UNLOCKED );
        }

        EnterState(player, STATE_SHOW_LOGIN_MENU);
#else
        EnterState(player, STATE_SHOW_CREATION_MENU);
#endif
        break;

    case STATE_SHOW_LOGIN_MENU:
#if 0
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
            connClose( player->connection, UNLOCKED );
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
                connClose(player->connection, UNLOCKED);
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
#else
        EnterState(player, STATE_SHOW_LOGIN_MENU);
#endif
        break;

    case STATE_EDIT_EXTRA_DESCR:
        EnterState(player, STATE_SHOW_LOGIN_MENU);
        break;

    default:
        LogPrint(LOG_NOTICE, "Illegal state (%d)", player->state);
        SendOutput(player, "The mud has lost its brain on your connection, "
                           "please reconnect.\n\r");
        EnterState(player, STATE_WIZLOCKED);
        break;
    }
}


void DoCreationMenu( PlayerStruct_t *player, char arg )
{
    int             bitcount;
    struct char_data *ch;

    ch = player->charData;

    switch (arg) 
    {
    case '1':
        EnterState(player, STATE_CHOOSE_NAME);
        break;
    case 'r':
    case 'R':
        EnterState(player, STATE_REROLL_ABILITIES);
        break;
    case '2':
        EnterState(player, STATE_CHOOSE_SEX);
        break;
    case '3':
        EnterState(player, STATE_CHOOSE_RACE);
        break;
    case '4':
        EnterState(player, STATE_CHOOSE_CLASS);
        break;
    case '5':
        ch->reroll = 20;
        if (ch->player.class != 0) {
            EnterState(player, STATE_CHOOSE_STATS);
        } else {
            SendOutput(player, "\nPlease select a class first.\n\r");
        }
        break;
    case '6':
        EnterState(player, STATE_CHOOSE_ALIGNMENT);
        break;

    case 'd':
    case 'D':
#if 0
        bitcount = 0;
        if (bitcount <= 0) {
            SendOutput(player, "Please enter a valid class.");
            return;
        }
#endif

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

        EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        break;
    default:
        ShowCreationMenu(player);
        SendOutput(player, "Invalid Choice.. Try again..\n\r");
        break;
    }
}

void DoAccountMenu( PlayerStruct_t *player, char arg )
{
    switch (arg) 
    {
    case '1':
        EnterState(player, STATE_CHOOSE_ANSI);
        break;
    case '2':
        EnterState(player, STATE_GET_NEW_PASSWORD);
        break;
    case '3':
        EnterState(player, STATE_SHOW_MOTD);
        break;
    case '4':
        EnterState(player, STATE_SHOW_CREDITS);
        break;
    case '5':
        if( player->account->confirmed ) {
            EnterState(player, STATE_SHOW_PLAYER_LIST);
        }
        break;
    case '6':
        if( player->account->confirmed ) {
            EnterState(player, STATE_SHOW_CREATION_MENU);
        }
        break;
    case '7':
        if( player->account->confirmed ) {
            EnterState(player, STATE_PLAYING);
        }
        break;
    case 'e':
    case 'E':
        if( !player->account->confirmed ) {
	    EnterState(player, STATE_ENTER_CONFIRM_CODE);
        }
        break;
    case 'r':
    case 'R':
        if( !player->account->confirmed ) {
            EnterState(player, STATE_RESEND_CONFIRM_EMAIL);
            EnterState(player, STATE_SHOW_ACCOUNT_MENU);
        }
        break;

    case 'q':
    case 'Q':
        SendOutput(player, "Thanks for dropping by, seeya later!\n\r");
        EnterState(player, STATE_WIZLOCKED);
        return;
    default:
        ShowAccountMenu(player);
        SendOutput(player, "Invalid Choice.. Try again..\n\r");
        break;
    }
}

void RollAbilities( PlayerStruct_t *player )
{
    int             rerolls;
    PlayerPC_t     *pc;
    int             i;
    int             roll[6];
    char            attrib[6];

    pc = player->pc;

    rerolls = GetAttributeInt( pc, "rerolls", "core-pc" );
    if( !rerolls ) {
        SendOutput( player, "No rerolls remain.  Sorry.\n\r\n\r" );
        return;
    }

    SetAttributeInt( pc, "rerolls", "core-pc", --rerolls );
    
    SendOutput( player, "Rolling Ability scores - %d rerolls left.\n\r\n\r",
                rerolls );

    LogPrint( LOG_INFO, "Player %s, PC %s abilities roll.  %d remain.", 
                        player->account->email, pc->name, rerolls );

    for(i = 0; i < 6; i++ ) {
        roll[i] = dice( player, 4, 6, 3 );
        sprintf( attrib, "roll%d", i+1 );
        SetAttributeInt( pc, attrib, "core-pc", roll[i] );
    }
}


#if 0
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
#endif

void LoginSendBanner( PlayerStruct_t *player )
{
    SendOutput(player, login);
    SendOutput(player, "If you're using Tintin or Lyntin, your client may not "
                       "display the password\n\r"
                       "sequence unless you change your settings. Please do not"
                       " be discouraged.\n\r\n\r");
    EnterState(player, STATE_GET_EMAIL);
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


#define MAX_EMAIL_LEN 4096

void CreateSendConfirmEmail( PlayerStruct_t *player )
{
    char            buffer[MAX_EMAIL_LEN];
    struct timeval  now;
    void           *ctx;
    char            digest[16];

    if( !player->account->confcode || !*player->account->confcode ) {
        /* New email, create new confcode */
        gettimeofday( &now, NULL );
        snprintf( buffer, MAX_EMAIL_LEN, "==!%s!==!%ld!==", 
                  player->account->email, now.tv_sec );
        /* MD5 it */
        ctx = opiemd5init();
        opiemd5update(ctx, (unsigned char *)buffer, strlen(buffer));
        opiemd5final((unsigned char *)digest, ctx);
        /* Convert the MD5 digest into English words */
        opiebtoe( buffer, digest );
        if( player->account->confcode ) {
            memfree( player->account->confcode );
        }
        player->account->confcode = memstrlink( buffer );
        player->account->confirmed = FALSE;
        pb_save_account(player->account);
    }

    snprintf( buffer, MAX_EMAIL_LEN, 
              "\r\n\r\nThank you for joining Havokmud.\r\n\r\n"
              "To confirm that this email is active, please login to your "
              "account, and enter\r\n"
              "the following confirmation code in the account menu:\r\n\r\n"
              "        %s\r\n\r\n"
              "Please note that the order of the words is important, but not "
              "the upper/lower\r\n"
              "case of the letters.\r\n\r\n"
              "Thanks.\r\n\r\n", player->account->confcode );

    send_email( player, "Havokmud confirmation email", buffer );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
