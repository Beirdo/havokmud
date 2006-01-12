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
 * $Id: login.c 1513 2005-12-31 19:57:25Z gjhurlbu $
 *
 * Copyright 2005 Gavin Hurlbut
 * All rights reserved
 *
 * Comments :
 *
 * Thread to handle editing text for each user
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include "logging.h"

static char ident[] _UNUSED_ =
    "$Id: login.c 1513 2005-12-31 19:57:25Z gjhurlbu $";

void AddEditorInput( PlayerStruct_t *player, char *line);

void *EditorThread( void *arg )
{
    InputStateItem_t   *item;
    PlayerStruct_t     *player;
    char               *line;
    InputStateType_t    type;

    LogPrintNoArg( LOG_NOTICE, "Started EditorThread" );

    while( 1 ) {
        item = (InputStateItem_t *)QueueDequeueItem( InputEditorQ, -1 );
        player = item->player;
        line = item->line;
        type = item->type;

        free( item );

        switch( type ) {
        case INPUT_AVAIL:
            /*
             * User input, feed it to the editor
             */
            AddEditorInput(player, line);
            break;
        default:
            break;
        }
    }

    return( NULL );
}

void EditorStart( PlayerStruct_t *player, char **string, int maxlen )
{
    if( !player || !string ) {
        return;
    }

    player->editOldHandlingQ = player->handlingQ;
    player->handlingQ        = InputEditorQ;
    player->editString       = string;
    player->editStringLen    = maxlen;

    FlushQueue( player->editOldHandlingQ, player );
}

void AddEditorInput( PlayerStruct_t *player, char *line)
{
    int             terminator = 0;

    /*
     * First of all we're concerned with finding a / at the beginning of an
     * incoming string, since that's our command character.
     */
    if (line[0] == '/') {
        switch (line[1]) {
        case 'q':               /* character is bombing out of the editor */
            terminator = -1;
            *line = '\0';
            break;
        case 'w':               /* character is saving message */
            terminator = 1;
            *line = '\0';
            break;
        case 'c':
            if (*(player->editString)) {
                free(*(player->editString));
                *(player->editString) = NULL;
                SendOutput("Buffer cleared.\r\n", player);
            } else
                SendOutput("Buffer already empty.\r\n", player);
            *line = '\0';
            break;
        case 'p':               /* print the message */
            if (*player->editString && **player->editString)
                SendOutput(*player->editString, player);
            else
                SendOutput("The buffer is currently empty.\r\n", player);
            *line = '\0';
            break;
        case '?':               /* character wants help! */
            SendOutput("           HavokMUD 2 Editor Help Screen\r\n"
                       "----------------------------------------------------"
                       "-----------------\r\n\r\n"
                       "All commands issued within the editor must be issued at"
                       " the beginning\r\n"
                       "of a new line of text.  All commands are prefixed with "
                       "the character /\r\n\r\n"
                       "Commands:\r\n"
                       "------------------------------------------------------"
                       "----------------\r\n", player);
            SendOutput(" /w -- This will save your message and exit the "
                       "editor.\r\n"
                       " /q -- This aborts your message without saving and "
                       "exits the editor.\r\n"
                       " /c -- This will clear the current message without "
                       "exiting the editor.\r\n"
                       " /p -- This will print the current message to your "
                       "screen.\r\n"
                       " /? -- Take a guess =)\r\n", player);
            *line = '\0';
            break;

        default:
            /* just an innocent mistake, let the string through unharmed */
            break;
        }
    }

    if (!(*player->editString)) {
        if (strlen(line) > player->editStringLen) {
            SendOutput("String too long - Truncated.\r\n", player);
            line[player->editStringLen] = '\0';
        }

        CREATE(*player->editString, char, strlen(line) + 3);
        strcpy(*player->editString, line);
    } else {
        if (strlen(line) + strlen(*player->editString) > 
            player->editStringLen) {
            SendOutput("String too long. Last line ignored.\n\r", player );
        } else {
            if (!(*player->editString = (char *) realloc(*player->editString,
                         strlen(*player->editString) + strlen(line) + 3))) {
                LogPrintNoArg(LOG_CRIT, "Out of memory!");
            } else {
                strcat(*player->editString, line);
            }
        }
    }

    if (terminator) {
        /*
         * NULL empty messages 
         */
        if ((player->editString) && (*player->editString) && 
            (**player->editString == '\0')) {
            free(*player->editString);
            *player->editString = NULL;
        }

#ifdef TODO
        if (!d->connected && IS_SET(d->character->specials.act, PLR_MAILING)) {
            if ((terminator > 0) && *d->str) {
                db_store_mail(d->name, d->character->player.name, *d->str);
                SEND_TO_Q("Message sent.\r\n", d);
            } else {
                SEND_TO_Q("Message aborted.\r\n", d);
            }

            if (*d->str) {
                free(*d->str);
            }

            *d->str = NULL;
            if (d->str) {
                free(d->str);
            }
            if (d->name) {
                free(d->name);
            }
            d->name = 0;
            REMOVE_BIT(d->character->specials.act, PLR_MAILING);
        } else if (!d->connected && 
                   IS_SET(d->character->specials.act, PLR_POSTING)) {
            if (terminator > 0 && *d->str) {
                db_post_message(d->board, d->msg);
                free(d->board);

                SEND_TO_Q("Message posted.\r\n", d);
                act("$n has finished posting $s message.", TRUE,
                    d->character, NULL, NULL, TO_ROOM);
            } else {
                /* user has aborted the post */
                if (*d->str) {
                    free(*d->str);
                }
                *d->str = NULL;

                if (d->msg->author) {
                    free(d->msg->author);
                }
                if (d->msg->title) {
                    free(d->msg->title);
                }
                SEND_TO_Q("Post aborted.\r\n", d);
                act("$n decides not to finish $s message.", TRUE,
                    d->character, NULL, NULL, TO_ROOM);
            }

            d->board = NULL;
            d->msg = NULL;
            REMOVE_BIT(d->character->specials.act, PLR_POSTING);
        }
#endif
        player->editString = NULL;
        player->handlingQ = player->editOldHandlingQ;
        FlushQueue( InputEditorQ, player );
    } else if (*line) {
        strcat(*player->editString, "\r\n");
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
