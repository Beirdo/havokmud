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
 * @brief Thread to handle editing text for each user
 */

#include "environment.h"
#include <pthread.h>
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include "logging.h"
#include "memory.h"

static char ident[] _UNUSED_ =
    "$Id$";

void AddEditorInput( PlayerStruct_t *player, char *line);

/**
 * @brief Handles players editing text
 * @param arg unused
 * @return never returns until shutdown
 *
 * To allow the users to edit their descriptions, send mail, post messages, etc
 * it is necessary to have an in-game editor.  While we don't technically need
 * a special thread for this, it makes the Input thread logic simpler to 
 * implement it this way.
 *
 * This thread will receive lines of text from the input thread and append
 * them to the string being edited.  A vi-like command-set is also provided,
 * using "/w", "/q", and so on.
 */
void *EditorThread( void *arg )
{
    InputStateItem_t   *item;
    PlayerStruct_t     *player;
    char               *line;
    InputStateType_t    type;

    while( 1 ) {
        item = (InputStateItem_t *)QueueDequeueItem( InputEditorQ, -1 );
        player = item->player;
        line = item->line;
        type = item->type;

        memfree( item );

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

/**
 * @brief Sets up a string to be edited by a player
 * @param player the player doing the editing
 * @param string a pointer to the location that stores the string pointer
 * @param maxlen the maximum string length to allow
 * 
 * This sets up the handling queue for the player and stores the previous
 * handling queue so we can go back when the editing's done.  Once the handling
 * queue is switched, all input is flushed for the player on the old queue.
 * A double-pointer is used for the string as the editor actually will 
 * be allocating the string storage internally, so we need the location that
 * stores the string pointer.
 */
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

/**
 * @brief Adds a line of input to the edited string, handles editing commands
 * @param player the player doing the editing
 * @param line the line to add
 * @todo Look into *NOT* reallocating so much as it is rather slow.
 * @todo Finish the conversion of the mail and bulletin-board specific handling.
 *
 * This will deal with the editor commands.  If no command has been entered,
 * the line is appended to the string if there is enough space.  This function
 * will dynamically allocate and reallocate the string to fit the contents,
 * up to the maximum length.  When the editing is finished, the handling queue
 * is set back to the previous one, and the editing queue is flushed for this
 * player.
 */
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
                SendOutput(player, "Buffer cleared.\r\n");
            } else
                SendOutput(player, "Buffer already empty.\r\n");
            *line = '\0';
            break;
        case 'p':               /* print the message */
            if (*player->editString && **player->editString)
                SendOutput(player, *player->editString);
            else
                SendOutput(player, "The buffer is currently empty.\r\n");
            *line = '\0';
            break;
        case '?':               /* character wants help! */
            SendOutput(player,
                       "           HavokMUD 2 Editor Help Screen\r\n"
                       "----------------------------------------------------"
                       "-----------------\r\n\r\n"
                       "All commands issued within the editor must be issued at"
                       " the beginning\r\n"
                       "of a new line of text.  All commands are prefixed with "
                       "the character /\r\n\r\n"
                       "Commands:\r\n"
                       "------------------------------------------------------"
                       "----------------\r\n");
            SendOutput(player,
                       " /w -- This will save your message and exit the "
                       "editor.\r\n"
                       " /q -- This aborts your message without saving and "
                       "exits the editor.\r\n"
                       " /c -- This will clear the current message without "
                       "exiting the editor.\r\n"
                       " /p -- This will print the current message to your "
                       "screen.\r\n"
                       " /? -- Take a guess =)\r\n");
            *line = '\0';
            break;

        default:
            /* just an innocent mistake, let the string through unharmed */
            break;
        }
    }

    if (!(*player->editString)) {
        if (strlen(line) > player->editStringLen) {
            SendOutput(player, "String too long - Truncated.\r\n");
            line[player->editStringLen] = '\0';
        }

        *player->editString = CREATEN( char, strlen(line) + 3);
        strcpy(*player->editString, line);
    } else {
        if (strlen(line) + strlen(*player->editString) > 
            player->editStringLen) {
            SendOutput(player, "String too long. Last line ignored.\n\r");
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
