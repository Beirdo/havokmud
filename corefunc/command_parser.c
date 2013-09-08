/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
 * Copyright 2007 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Command parser for players
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

#include "oldstructs.h"
#include "oldutils.h"
#include "oldexterns.h"
#include "interthread.h"
#include "protos.h"

static char ident[] _UNUSED_ =
    "$Id$";

int             plr_tick_count = 0;

BalancedBTree_t    *commandName;
BalancedBTree_t    *commandNum;
BalancedBTree_t    *fillWords;

char *get_argument(char *line_in, char **arg_out);
char *get_argument_nofill(char *line_in, char **arg_out);
char *get_argument_delim(char *line_in, char **arg_out, char delim);
char *get_argument_common(char *line_in, char **arg_out, int do_fill,
                          char delim);
bool is_number(char *str);
int special(struct char_data *ch, int cmd, char *arg);
bool fill_word(char *argument);

/**
 * @brief Runs when somebody has just logged in (not from linkdead)
 * @todo Make this cleaner yet
 */

void JustLoggedIn( PlayerStruct_t *player )
{
    int             count_players;
    struct char_data *ch;
    int             homeroom;

    ch = player->charData;
#if 0
    reset_char(ch);
#endif
    total_connections++;
    if (!IS_IMMORTAL(ch) ||
        ch->invis_level <= 58) {
        LogPrint(LOG_INFO, "Loading %s's equipment", ch->player.name);
    }

    count_players = GetPlayerCount();

    if (total_max_players < count_players) {
        total_max_players = count_players;
    }

    load_char_objs(ch);
#if 0
    SendOutput(player, WELC_MESSG);
#endif

    /**
     * @todo check where this character_list crap was going
     */
#if 0
    ch->next = character_list;
    character_list = ch;
#endif

    homeroom = 3001;
    if (ch->in_room == NOWHERE || ch->in_room == AUTO_RENT) {
        if (!IS_IMMORTAL(ch)) {
            if (ch->specials.start_room <= 0) {
                if (GET_RACE(ch) == RACE_HALFLING) {
                    homeroom = 1103;
                }
            } else {
                homeroom = ch->specials.start_room;
            }
        } else {
            homeroom = 1000;
            if (ch->specials.start_room > NOWHERE && 
                roomFindNum(ch->specials.start_room)) {
                homeroom = ch->specials.start_room;
            }
        }
    } else if (roomFindNum(ch->in_room)) {
        homeroom = ch->in_room;
    }

#if 0
    char_to_room(ch, homeroom);
#endif
    ch->player.hometown = homeroom;

    ch->specials.tick = plr_tick_count++;
    if (plr_tick_count == PLR_TICK_WRAP) {
        plr_tick_count = 0;
    }
#if 0
    act("$n has entered the game.", TRUE, ch, 0, 0, TO_ROOM);
#endif

    if (!GetMaxLevel(ch)) {
#if 0
        /* create a new player */
#endif
    }

    player->prompt_mode = 1;
}

/**
 * @todo Rework this monstrosity a bit more, it's too unwieldly
 * @todo redo the "TempDis" portion
 * @todo redo the "Dismount" portion
 */
void CommandParser( PlayerStruct_t *player, char *line )
{
    struct char_data *ch;
    extern int      no_specials;
    CommandDef_t   *cmd;
    char           *arg,
                   *arg1,
                   *arg2;
    char           *tmparg;

    if( !player || !line ) {
        return;
    }

    if (!line || !*line || *line == '\n' || *line == '\r' ) {
        return;
    } 
    
    ch = player->charData;

#ifdef TODO
    if (HasClass(ch, TempDis) && GetMaxLevel(ch) < 58 && IS_PC(ch)) {
        SendOutput( player, "Sorry, we are tracking down a bug and this class "
                            "or one of your classes is disabled.\n\r");
        return;
    }
#endif

#ifdef TODO
    if (MOUNTED(ch) && ch->in_room != MOUNTED(ch)->in_room) {
        Dismount(ch, MOUNTED(ch), POSITION_STANDING);
    }
#endif

    if (!isalpha((int)*line)) {
        arg = memsnprintf(strlen(line) + 2, "%c %s", *line, &(line[1]));
        if( !arg ) {
            LogPrintNoArg( LOG_CRIT, "Nasty error in command_interpreter!!!" );
            return;
        }
    } else {
        arg = memstrlink( line );
        if( !arg ) {
            LogPrintNoArg( LOG_CRIT, "Nasty error in command_interpreter!!!" );
            return;
        }
    }
    
    tmparg = arg;
    arg = get_argument_nofill(arg, &arg1);
    arg2 = skip_spaces( arg );

    if (arg1) {
        cmd = FindCommand(arg1);
    } else {
        cmd = NULL;
    }

    if (!cmd || GetMaxLevel(ch) < cmd->min_level) {
        SendOutput( player, "Pardon?\n\r");
        if( tmparg ) {
            memfree(tmparg);
        }
        return;
    }

    if (!cmd->func) {
        SendOutput( player, "Sorry, but that command has yet to be "
                            "implemented...\n\r" );
        if( tmparg ) {
            memfree(tmparg);
        }
        return;
    }

    if (ch->specials.paralyzed && cmd->min_pos > POSITION_STUNNED) {
        SendOutput( player, "You are paralyzed, you can't do much!\n\r" );
        if( tmparg ) {
            memfree(tmparg);
        }
        return;
    }

    if (ch->specials.frozen && IS_PC(ch)) {
        /*
         * They can't move, must have pissed off an immo!
         * make sure polies can move, some mobs have this bit set
         */
        SendOutput( player, "You have been frozen in your steps, you cannot do "
                            "a thing!\n\r");
        if( tmparg ) {
            memfree(tmparg);
        }
        return;
    }

    if (GET_POS(ch) < cmd->min_pos) {
        switch (GET_POS(ch)) {
        case POSITION_DEAD:
            SendOutput( player, "Lie still; you are DEAD!!! :-( \n\r");
            break;
        case POSITION_INCAP:
        case POSITION_MORTALLYW:
            SendOutput( player, "You are in a pretty bad shape, unable to do "
                                "anything!\n\r");
            break;

        case POSITION_STUNNED:
            SendOutput( player, "All you can do right now, is think about the "
                                "stars!\n\r");
            break;
        case POSITION_SLEEPING:
            SendOutput( player, "In your dreams, or what?\n\r");
            break;
        case POSITION_RESTING:
            SendOutput( player, "Nah... You feel too relaxed to do that..\n\r");
            break;
        case POSITION_SITTING:
            SendOutput( player, "Maybe you should get on your feet first?\n\r");
            break;
        case POSITION_FIGHTING:
            SendOutput( player, "No way! You are fighting for your life!\n\r");
            break;
        case POSITION_STANDING:
            SendOutput( player, "Fraid you can't do that\n\r");
            break;
        case POSITION_MOUNTED:
            SendOutput( player, "Fraid you have no mounted steed.\n\r");
            break;

        }
        if( tmparg ) {
            memfree(tmparg);
        }
        return;
    } 
    
    /*
     * so you can log mobs if ya need to
     */
#ifdef LOG_MOB
    if (!IS_PC(ch) && !(ch->specials.polyself)) {
        LogPrint( LOG_INFO, "[%ld] <%s>:%s", ch->in_room, ch->player.name, 
                            line);
    }
#endif

    /*
     * to log all pc's
     */
    if (IS_SET(SystemFlags, SYS_LOGALL)) {
        if (IS_PC(ch)) {
            LogPrint( LOG_INFO, "[%ld] %s:%s", ch->in_room, ch->player.name, 
                                line);
        }
    } else if (ch->specials.log_me) {
        /*
         * user flagged as log person
         */
        LogPrint( LOG_INFO, "[%ld] %s:%s", ch->in_room, ch->player.name, line);
    } else if (IS_IMMORTAL(ch) && GetMaxLevel(ch) < MAX_IMMORT) {
        /*
         * we log ALL immortals
         */
        LogPrint( LOG_INFO, "[%ld] %s:%s", ch->in_room, ch->player.name, line);
    }

    if (GET_GOLD(ch) > 2000000) {
        LogPrint( LOG_INFO, "%s:%s", ch->player.name, line);
    }

    if (no_specials || !special(ch, cmd->number, arg2)) {
        (*cmd->func)(ch, arg2, cmd->number);
    }

    if( tmparg ) {
        memfree(tmparg);
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



bool is_number(char *str)
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


static char     *fill[] = { 
    "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
};
static int       fillCount = NELEMENTS(fill);

void initializeFillWords( void )
{
    int i;
    BalancedBTreeItem_t *item;

    fillWords = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );

    BalancedBTreeLock( fillWords );

    for( i = 0; i < fillCount; i++ ) {
        item = CREATE(BalancedBTreeItem_t);
        item->key  = &fill[i];
        item->item = fill[i];
        BalancedBTreeAdd( fillWords, item, LOCKED, FALSE );
    }
    BalancedBTreeAdd( fillWords, NULL, LOCKED, TRUE );

    BalancedBTreeUnlock( fillWords );
}

bool fill_word(char *argument)
{
    BalancedBTreeItem_t *item;

    BalancedBTreeLock( fillWords );
    item = BalancedBTreeFind( fillWords, &argument, LOCKED, FALSE );
    BalancedBTreeUnlock( fillWords );
    return( (item ? TRUE : FALSE) );
}

int special(struct char_data *ch, int cmd, char *arg)
{
    register struct obj_data   *i;
    register struct char_data  *k;
    struct index_data          *index;
    int                         j;
    struct room_data           *rm;
    LinkedListItem_t           *item;

    if (ch->in_room == NOWHERE) {
#if 0
        char_to_room(ch, 3001);
#endif
        return(FALSE);
    }

    /*
     * special in room?
     */
    rm = roomFindNum(ch->in_room);
    if( rm->func && (*rm->func)(ch, cmd, arg, roomFindNum(ch->in_room),
                                PULSE_COMMAND) ) {
        return(TRUE);
    }

    /*
     * special in equipment list?
     */
    for (j = 0; j <= (MAX_WEAR - 1); j++) {
        if (ch->equipment[j] && ch->equipment[j]->item_number >= 0) {
#ifdef TODO
            if (IS_OBJ_STAT(ch->equipment[j], anti_flags, ITEM_ANTI_SUN)) {
                AntiSunItem(ch, cmd, arg, ch->equipment[j], PULSE_COMMAND);
            }
#endif
            index = ch->equipment[j]->index;
            if (ch->equipment[j] && index && index->func &&
                (*index->func)(ch, cmd, arg, ch->equipment[j], PULSE_COMMAND)) {
                return (TRUE);
            }
        }
    }

    /*
     * item_number >=0
     * special in inventory?
     */
    for (i = ch->carrying; i; i = i->next_content) {
        if (i->item_number >= 0 && i->index->func && 
            (*i->index->func)(ch, cmd, arg, i, PULSE_COMMAND)) {
            /**
             * @bug Crashes here when saving item twice
             */
            return(TRUE);
        }
    }

    /*
     * special in mobile present?
     */
    for (k = rm->people; k; k = k->next_in_room) {
        if (IS_MOB(k) && k != ch && (index = mobileIndex(k->nr)) && 
            index->func && (*index->func) (ch, cmd, arg, k, PULSE_COMMAND)) {
            return(TRUE);
        }
    }

    /*
     * special in object present?
     */
    LinkedListLock( rm->contentList );
    for( item = rm->contentList->head; item; item = item->next ) {
        i = CONTENT_LINK_TO_OBJ(item);
        index = i->index;
        if (i->item_number >= 0 && index->func && 
            (*index->func)(ch, cmd, arg, i, PULSE_COMMAND)) {
            LinkedListUnlock( rm->contentList );
            return(TRUE);
        }
    }
    LinkedListUnlock( rm->contentList );

    return(FALSE);
}



void InitializeCommands( void )
{
    commandName = BalancedBTreeCreate( NULL, BTREE_KEY_STRING );
    commandNum  = BalancedBTreeCreate( NULL, BTREE_KEY_INT );

    SetupCommands( coreCommands, coreCommandCount );
}

void SetupCommands( CommandDef_t *commands, int count )
{
    int             i;
    CommandDef_t   *cmd;

    for( i = 0; i < count; i++ ) {
        cmd = &commands[i];
        AddCommand( cmd );
    }
}


/**
 * Adds a command to the Command BTrees
 */
void AddCommand( CommandDef_t *cmd )
{
    BalancedBTreeItem_t    *item;

    item = CREATE(BalancedBTreeItem_t);
    item->key = &cmd->name;
    item->item = cmd;
    BalancedBTreeAdd( commandName, item, UNLOCKED, TRUE );

    item = CREATE(BalancedBTreeItem_t);
    item->key = &cmd->number;
    item->item = cmd;
    BalancedBTreeAdd( commandNum, item, UNLOCKED, TRUE );
}

/**
 * Finds a command in the Command BTree (by name).  Partial matches should work
 * and return the first encountered match
 */
CommandDef_t *FindCommand( char *string )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *nextItem;
    CommandDef_t           *cmd;

    item = BalancedBTreeFind( commandName, (void *)&string, UNLOCKED, TRUE );
    if( item ) {
        nextItem = BalancedBTreeFindNext( commandName, item, UNLOCKED );
        if( nextItem && !commandName->keyCompareAlt((void *)&string,
                                                    nextItem->key) ) {
            /* Partial match with multiple matches */
            return( NULL );
        }
        cmd = (CommandDef_t *)item->item;
        return( cmd );
    }
    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
