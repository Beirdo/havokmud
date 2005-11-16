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
 * Thread to handle network connections.
 */

#include "environment.h"
#include <pthread.h>
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

static char ident[] _UNUSED_ =
    "$Id$";

void SendOutput( char *string, PlayerStruct_t *player )
{
    OutputBuffer_t *buf;

    buf = (OutputBuffer_t *)malloc(sizeof(OutputBuffer_t));
    if( !buf ) {
        return;
    }
    buf->buf = strdup( string );
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
    if( !buf ) {
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
