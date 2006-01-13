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
 * @brief Handles output to characters
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static char ident[] _UNUSED_ =
    "$Id$";

/**
 * @brief Outputs a string to a player with ANSI color expansion
 * @param string the string to send
 * @param player the player to send it to
 * @todo The parameters should be reordered to put player first.  This will
 *       allow the functionality of ch_printf to be rolled into this function
 *       in a sane way.  This would approximate the parameters of a sprintf
 *       call
 */
void SendOutput( char *string, PlayerStruct_t *player )
{
    OutputBuffer_t *buf;

    buf = (OutputBuffer_t *)malloc(sizeof(OutputBuffer_t));
    if( !buf || !string ) {
        return;
    }
    buf->buf = strdup( 
            ParseAnsiColors(IS_SET(player->charData->player.user_flags, 
                                   USE_ANSI), string) );
    if( !buf->buf ) {
        free( buf );
        return;
    }
    buf->len = strlen( string );

    QueueEnqueueItem( player->outputQ, buf );
}

/**
 * @brief Outputs a string to a player verbatim
 * @param string the string to send (can be binary)
 * @param len the length of the string
 * @param player the player to send it to
 * @todo The parameters should be reordered to have the player first as this
 *       will make it consistant with SendOutput which also needs to be
 *       reordered
 *
 * Sends a preformatted raw character array to the player.  This is useful for
 * sending telnet control characters (turn off/on echo around password entry)
 * for example.
 */
void SendOutputRaw( unsigned char *string, int len, PlayerStruct_t *player )
{
    OutputBuffer_t *buf;

    buf = (OutputBuffer_t *)malloc(sizeof(OutputBuffer_t));
    if( !buf || !string || !len ) {
        return;
    }
    buf->buf = malloc(len);
    if( !buf->buf ) {
        free( buf );
        return;
    }
    memcpy(buf->buf, string, len);
    buf->len = len;

    QueueEnqueueItem( player->outputQ, buf );
}

/**
 * @brief Outputs a string to a player with printf functionality
 * @param player player who will get the output
 * @param fmt printf format
 * @return length of the outputted string (max of MAX_STRING_LENT)
 * @deprecated this functionality will be rolled into SendOutput
 *
 * Formats an output string using vsnprintf, then uses SendOutput to queue it.
 * This functionality will be added to SendOutput soon.  Output strings are
 * limited to MAX_STRING_LENGTH to protect against buffer overflows.
 */
int ch_printf(PlayerStruct_t *player, char *fmt, ...)
{
    char            buf[MAX_STRING_LENGTH];     /* better safe than sorry */
    int             len;
    va_list         args;

    va_start(args, fmt);
    len = vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    SendOutput(buf, player);

    return(len);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

