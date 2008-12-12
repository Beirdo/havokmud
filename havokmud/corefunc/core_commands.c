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
 * @brief Core commands and the command add/remove code
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

#include "structs.h"
#include "utils.h"
#include "externs.h"
#include "interthread.h"
#include "balanced_btree.h"

static char ident[] _UNUSED_ =
    "$Id$";

void banHostDepthFirst( PlayerStruct_t *player, BalancedBTreeItem_t *item );
char *view_newhelp(int reportId);
char *view_report(int reportId);

CommandDef_t coreCommands[] = {
    { "siteban", do_siteban, 67, POSITION_DEAD, SILLYLORD },
    { "shutdow", do_shutdow, 68, POSITION_DEAD, SILLYLORD },
    { "shutdown", do_shutdown, 79, POSITION_DEAD, SILLYLORD },
    { "idea", do_idea, 80, POSITION_DEAD, 0 },
    { "typo", do_typo, 81, POSITION_DEAD, 0 },
    { "bug", do_bug, 82, POSITION_DEAD, 0 },
};
int coreCommandCount = NELEMENTS(coreCommands);



/**
 * @brief Allows to add or remove hosts from the ban list, or show the current
 *        banlist
 * @param ch The character performing the command
 * @param argument The command arguments
 * @param cmd The command number
 * @todo add database support for the banned host list
 */
void do_siteban(struct char_data *ch, char *argument, int cmd)
{
#ifdef SITELOCK
    char                   *arg1;
    char                   *arg2;
    int                     length;
    BalancedBTreeItem_t    *item;
    PlayerStruct_t         *player;
#endif

    player = (PlayerStruct_t *)ch->playerDesc;
    if( !player ) {
        return;
    }

    if ((GetMaxLevel(ch) < DEMIGOD) || (IS_NPC(ch))) {
        SendOutput( player, "You cannot Siteban.\n\r" );
        return;
    }

#ifdef SITELOCK
    /*
     * all, add (place), list, rem (place)
     * get first piece..
     */
    argument = get_argument(argument, &arg1);
    argument = get_argument(argument, &arg2);
    if (!arg1) {
        SendOutput( player, "Siteban {add <host> | rem <host> | list}\n\r" );
        return;
    }

    if (strcasecmp(arg1, "add") == 0) {
        if (!arg2) {
            SendOutput( player, "Siteban add <host_name>\n\r" );
            return;
        }

        length = strlen(arg2);
        if ((length <= 3) || (length >= 80)) {
            SendOutput( player, "Host is too long or short, please try "
                                "again\n\r" );
            return;
        }

        BalancedBTreeLock( banHostTree );

        item = BalancedBTreeFind( banHostTree, (void *)&arg2, LOCKED, TRUE );
        if( item ) {
            SendOutput( player, "Host is already in database\n\r" );
            BalancedBTreeUnlock( banHostTree );
            return;
        }

        item = CREATE(BalancedBTreeItem_t);
        if( !item ) {
            SendOutput( player, "Can't, out of memory!\n\r" );
            BalancedBTreeUnlock( banHostTree );
            return;
        }

        item->item = (void *)strdup(arg2);
        item->key  = &(item->item);

        BalancedBTreeAdd( banHostTree, item, LOCKED, TRUE );
        BalancedBTreeUnlock( banHostTree );

        LogPrint( LOG_CRIT, "%s has added host %s to the access denied list.", 
                            GET_NAME(ch), arg2 );
    } else if (strcasecmp(arg1, "rem") == 0) {
        if (!arg2) {
            SendOutput( player, "Siteban rem <host_name>\n\r" );
            return;
        }

        length = strlen(arg2);
        if ((length <= 3) || (length >= 80)) {
            SendOutput( player, "Host length is bad, please try again\n\r" );
            return;
        }

        BalancedBTreeLock( banHostTree );

        if( !banHostTree->root ) {
            SendOutput( player, "Banned Host list is empty\n\r" );
            BalancedBTreeUnlock( banHostTree );
            return;
        }

        item = BalancedBTreeFind( banHostTree, (void *)&arg2, LOCKED, TRUE );
        if( !item ) {
            SendOutput( player, "Host is not in database\n\r" );
            BalancedBTreeUnlock( banHostTree );
            return;
        }

        BalancedBTreeRemove( banHostTree, item, LOCKED, TRUE );
        BalancedBTreeUnlock( banHostTree );

        LogPrint( LOG_CRIT, "%s has removed host %s from the access denied "
                            "list.", GET_NAME(ch), arg2);

        memfree( item->item );
        memfree( item );
    } else if (strcasecmp(arg1, "list") == 0) {
        BalancedBTreeLock( banHostTree );

        if( !banHostTree->root ) {
            SendOutput( player, "Banned Host list is empty\n\r" );
            BalancedBTreeUnlock( banHostTree );
            return;
        }

        banHostDepthFirst( player, banHostTree->root );
        
        BalancedBTreeUnlock( banHostTree );
    } else {
        SendOutput( player, "Siteban {add <host> | rem <host> | list}\n\r" );
    }
#endif
}

/**
 * @brief Prints the the banned hosts lists in sorted order
 * @param player Player to send the output to
 * @param item Item at which to start the recursion of the tree
 *
 * This function recursively traverses the binary tree for the banned hosts
 * and outputs each entry in sorted order.  It should be called with item set
 * to banHostTree->root to print the whole tree.
 */
void banHostDepthFirst( PlayerStruct_t *player, BalancedBTreeItem_t *item )
{
    char            *host;

    if( !item ) {
        return;
    }

    banHostDepthFirst( player, item->left );

    host = (char *)item->item;
    SendOutput( player, "Host: %s\n\r", host );

    banHostDepthFirst( player, item->right );
}

void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
    PlayerStruct_t *player;

    if( !ch ) {
        return;
    }

    player = (PlayerStruct_t *)ch->playerDesc;
    if( !player ) {
        return;
    }

    SendOutput(player, "If you want to shut something down - say so!\n\r");
}

void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
    char            buf[100],
                   *arg;
    PlayerStruct_t *player;

    player = (PlayerStruct_t *)ch->playerDesc;
    if( !player ) {
        return;
    }

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &arg);

    if (!arg) {
        sprintf(buf, "Shutdown by %s.", GET_NAME(ch));
        SendToAll(buf);
        LogPrintNoArg( LOG_CRIT, buf );
        mudshutdown = 1;
    } else if (!strcasecmp(arg, "reboot")) {
        sprintf(buf, "Reboot by %s.", GET_NAME(ch));
        SendToAll(buf);
        LogPrintNoArg( LOG_CRIT, buf );
        mudshutdown = 1;
        reboot_now = 1;
    } else {
        SendOutput(player, "Go shut down someone your own size.\n\r");
    }
}

void do_idea(struct char_data *ch, char *argument, int cmd)
{
    PlayerStruct_t     *player; 

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

    if (IS_NPC(ch)) {
        SendOutput( player, "Monsters can't have ideas - Go away.\n\r" );
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        SendOutput( player, "That doesn't sound like a good idea to me.. "
                            "Sorry.\n\r" );
        return;
    }

    db_report_entry( REPORT_IDEA, ch, argument );

    SendOutput( player, "Ok. Thanks.\n\r" );
}

void do_typo(struct char_data *ch, char *argument, int cmd)
{
    PlayerStruct_t     *player; 

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

    if (IS_NPC(ch)) {
        SendOutput( player, "Monsters can't spell - leave me alone.\n\r" );
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        SendOutput( player, "I beg your pardon?\n\r" );
        return;
    }

    db_report_entry( REPORT_TYPO, ch, argument );

    SendOutput( player, "Ok. thanks.\n\r" );
}

void do_bug(struct char_data *ch, char *argument, int cmd)
{
    PlayerStruct_t     *player; 

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

    if (IS_NPC(ch)) {
        SendOutput( player, "You are a monster! Bug off!\n\r" );
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        SendOutput( player, "Pardon?\n\r" );
        return;
    }

    db_report_entry( REPORT_BUG, ch, argument );

    SysLogPrint( LOG_CRIT, "BUG Report by %s [%ld]: %s", GET_NAME(ch), 
                           ch->in_room, argument);

    SendOutput( player, "Ok.\n\r" );
}

char *view_newhelp(int reportId)
{
    struct user_report *report;
    struct user_report *thisReport;
    char               *buf;
    char                buf2[MAX_STRING_LENGTH];
    int                 length;
    int                 lengthAvail;
    int                 i;
    int                 count;

    buf = NULL;
    length = 0;
    lengthAvail = 0;
    report = NULL;

    report = db_get_report( REPORT_HELP );
    count = report->resCount;
    if( !count ) {
        count++;
    }

    for( i = 0; i < count; i++ ) {
        thisReport = &report[i];
        sprintf( buf2, "**%s: help %s\n\r", thisReport->character, 
                                            thisReport->report );
        while( length + strlen(buf2) >= lengthAvail ) {
            /* Need to grow the buffer */
            lengthAvail += MAX_STRING_LENGTH;
            buf = (char *)realloc(buf, lengthAvail);
            if( !buf ) {
                LogPrintNoArg( LOG_CRIT, "Out of memory in view_newhelp!");
                return(NULL);
            }
        }

        buf[length] = '\0';
        strcat(buf, buf2);
        length += strlen(buf2);
    }
    memfree(report);

    return( buf );
}

char *view_report(int reportId)
{
    struct user_report *report;
    struct user_report *thisReport;
    char               *buf;
    char                buf2[MAX_STRING_LENGTH];
    int                 length;
    int                 lengthAvail;
    int                 i;
    int                 count;

    buf = NULL;
    length = 0;
    lengthAvail = 0;
    report = NULL;

    if( reportId == REPORT_MOTD ) {
        return( motd ? strdup( motd ) : NULL );
    } else if( reportId == REPORT_WMOTD ) {
        return( wmotd ? strdup( wmotd ) : NULL );
    }

    report = db_get_report( reportId );
    count = report->resCount;
    if( !count ) {
        count++;
    }

    for( i = 0; i < count; i++ ) {
        thisReport = &report[i];
        sprintf( buf2, "**%s[%d]: %s\n\r", thisReport->character, 
                                           thisReport->roomNum,
                                           thisReport->report );

        while( length + strlen(buf2) >= lengthAvail ) {
            /* Need to grow the buffer */
            lengthAvail += MAX_STRING_LENGTH;
            buf = (char *)realloc(buf, lengthAvail);
            if( !buf ) {
                LogPrintNoArg( LOG_CRIT, "Out of memory in view_report!");
                return(NULL);
            }
        }

        buf[length] = '\0';
        strcat(buf, buf2);
        length += strlen(buf2);
    }
    memfree( report );

    return( buf );
}

typedef struct {
    char       *file;
    int         reportId;
    char     *(*func)(int);
    long        bit;
} Report_t;

Report_t reports[] = {
    { "help", REPORT_HELP, view_newhelp, 0 },
    { "quest", REPORT_QUEST, view_report, 0 },
    { "bug", REPORT_WIZBUG, view_report, 0 },
    { "idea", REPORT_WIZIDEA, view_report, 0 },
    { "typo", REPORT_WIZTYPO, view_report, 0 },
    { "morttypo", REPORT_TYPO, view_report, PLR_WIZREPORT },
    { "mortbug", REPORT_BUG, view_report, PLR_WIZREPORT },
    { "mortidea", REPORT_IDEA, view_report, PLR_WIZREPORT },
    { "motd", REPORT_MOTD, view_report, 0 },
    { "wmotd", REPORT_WMOTD, view_report, 0 }
};
int report_count = NELEMENTS( reports );

/**
 * @todo this should once again use paged output, but I need to rewrite that
 *       support
 */
void do_viewfile(struct char_data *ch, char *argument, int cmd)
{
    char           *namefile;
    char           *buf = NULL;
    int             i;
    PlayerStruct_t *player;

    if( !ch || !(player = (PlayerStruct_t *)ch->playerDesc) ) {
        return;
    }

    argument = get_argument(argument, &namefile);
    if( !namefile ) {
        SendOutput( player, "Commands: view <bug|typo|idea|mortbug|morttypo|"
                            "mortidea|motd|wmotd|help>.\n\r");
        return;
    }

    for( i = 0; i < report_count && !buf; i++ ) {
        if (!strcmp(namefile, reports[i].file)) {
#if 0
            if( reports[i].bit && !IS_SET(ch->specials.act, reports[i].bit) ) {
                SendOutput( player, "You do not have the power to do this");
                return;
            }
#endif
            buf = (reports[i].func)(reports[i].reportId);
        }
    }

    if( i >= report_count ) {
        SendOutput( player, "Commands: view <bug|typo|idea|mortbug|morttypo|"
                            "mortidea|motd|wmotd|help>.\n\r");
        return;
    }

    if( buf ) {
        SendOutput( player, buf );
        memfree( buf );
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

