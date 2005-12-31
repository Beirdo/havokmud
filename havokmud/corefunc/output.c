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
 * Handles output to characters
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

/*
 * source: EOD, by John Booth <???> 
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

