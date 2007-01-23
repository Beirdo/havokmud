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
 * @param player the player to send it to
 * @param fmt the printf format for the buffer to send
 * @param args the optional arguments for the printf
 *
 * Formats an output string using vsnprintf, ANSI expands it (if necessary),
 * then queues it.  Output strings are limited to MAX_STRING_LENGTH to protect 
 * against buffer overflows.
 */
void SendOutput( PlayerStruct_t *player, char *fmt, ... )
{
    OutputBuffer_t *outbuf;
    static char     buf[MAX_STRING_LENGTH];     /* better safe than sorry */
    int             len;
    va_list         args;

    if( !fmt ) {
        return;
    }

    outbuf = (OutputBuffer_t *)malloc(sizeof(OutputBuffer_t));
    if( !outbuf ) {
        return;
    }

    va_start(args, fmt);
    len = vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    outbuf->buf = strdup( 
            ParseAnsiColors(IS_SET(player->charData->player.user_flags, 
                                   USE_ANSI), buf) );
    if( !outbuf->buf ) {
        free( outbuf );
        return;
    }

    /**
     * @todo make sure that the buffer length here includes any ANSI expansion
     *       that may have been done
     */
    outbuf->len = len;

    QueueEnqueueItem( player->outputQ, outbuf );
}

/**
 * @brief Outputs a string to a player verbatim
 * @param player the player to send it to
 * @param string the string to send (can be binary)
 * @param len the length of the string
 *
 * Sends a preformatted raw character array to the player.  This is useful for
 * sending telnet control characters (turn off/on echo around password entry)
 * for example.
 */
void SendOutputRaw( PlayerStruct_t *player, unsigned char *string, int len )
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


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

