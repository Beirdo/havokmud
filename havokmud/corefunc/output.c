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
#include "memory.h"
#include "oldstructs.h"
#include "utils.h"
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "linked_list.h"

static char ident[] _UNUSED_ =
    "$Id$";

#if 0
extern struct zone_data *zone_table;
#endif

void SendToAllCond( char *messg, int (*condfunc)(PlayerStruct_t *, void *),
                    void *arg );
int CondIsAwake( PlayerStruct_t *player, void *arg );
int CondIsOutdoors( PlayerStruct_t *player, void *arg );
int CondIsInDesert( PlayerStruct_t *player, void *arg );
int CondIsOutdoorsOther( PlayerStruct_t *player, void *arg );
int CondIsInArctic( PlayerStruct_t *player, void *arg );
int CondAllExcept( PlayerStruct_t *player, void *arg );
int CondInZone( PlayerStruct_t *player, void *arg );

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
    char            buf[MAX_STRING_LENGTH];     /* better safe than sorry */
    int             len;
    struct char_data *ch;
    va_list         args;

    if( !fmt ) {
        return;
    }

    outbuf = CREATE(OutputBuffer_t);
    if( !outbuf ) {
        return;
    }

    va_start(args, fmt);
    len = vsnprintf(buf, MAX_STRING_LENGTH, fmt, args);
    va_end(args);

    ch = player->charData;

    outbuf->buf = CREATEN(char, MAX_STRING_LENGTH);
    if( !outbuf->buf ) {
        memfree( outbuf );
        return;
    }

    if( ch ) {
        outbuf->len = ParseAnsiColors( IS_SET(ch->player.user_flags, USE_ANSI),
                                       buf, outbuf->buf );
    } else {
        outbuf->len = ParseAnsiColors( FALSE, buf, outbuf->buf );
    }

#ifdef DEBUG_OUTPUT
    LogPrint( LOG_INFO, "Sending output: %s", outbuf->buf );
#endif
    QueueEnqueueItem( player->outputQ, outbuf );
    connKickOutput( player->connection );
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

    buf = CREATE(OutputBuffer_t);
    if( !buf || !string || !len ) {
        return;
    }
    buf->buf = CREATEN(char, len);
    if( !buf->buf ) {
        memfree( buf );
        return;
    }
    memcpy(buf->buf, string, len);
    buf->len = len;

    QueueEnqueueItem( player->outputQ, buf );
}


void SendToAllCond( char *messg, int (*condfunc)(PlayerStruct_t *, void *),
                    void *arg )
{
    LinkedListItem_t       *item;
    ConnectionItem_t       *connection;
    PlayerStruct_t         *player;

    if( !messg ) {
        return;
    }

    LinkedListLock( ConnectionList );
    for (item = ConnectionList->head; item; item = item->next) {
        connection = (ConnectionItem_t *)item;
        player = connection->player;
        if( !player ) {
            continue;
        }

        if( player->state != STATE_PLAYING ) {
            continue;
        }

        if( condfunc && !condfunc(player, arg) ) {
            continue;
        }

        SendOutput( player, messg );
    }
    LinkedListUnlock( ConnectionList );
}


/**
 * @brief Sends a message to all playing players
 */
void SendToAll(char *messg)
{
    SendToAllCond(messg, NULL, NULL);
}

int CondIsAwake( PlayerStruct_t *player, void *arg )
{
    return( (AWAKE(player->charData)) ? 1 : 0 );
}

/**
 * @brief Sends a message to all awake players
 */
void SendToAllAwake(char *messg)
{
    SendToAllCond(messg, CondIsAwake, NULL);
}

int CondIsOutdoors( PlayerStruct_t *player, void *arg )
{
    return( (OUTSIDE(player->charData) && 
             !player->charData->specials.no_outdoor) ? 1 : 0 );
}

void send_to_outdoor(char *messg)
{
    SendToAllCond(messg, CondIsOutdoors, NULL);
}

int CondIsInDesert( PlayerStruct_t *player, void *arg )
{
#if 0
    struct room_data *rp;

    return( (OUTSIDE(player->charData) && 
             !IS_SET(player->charData->specials.act, PLR_NOOUTDOOR) && 
             (rp = roomFindNum(player->charData->in_room)) != NULL && 
             (IS_SET(zone_table[rp->zone].reset_mode, ZONE_DESERT) || 
              rp->sector_type == SECT_DESERT)) ? 1 : 0 );
#else
    return( FALSE );
#endif
}

void send_to_desert(char *messg)
{
    SendToAllCond(messg, CondIsInDesert, NULL);
}

int CondIsOutdoorsOther( PlayerStruct_t *player, void *arg ) 
{
#if 0
    struct room_data *rp;

    return( (OUTSIDE(player->charData) && 
             !IS_SET(player->charData->specials.act, PLR_NOOUTDOOR) && 
             (rp = roomFindNum(player->charData->in_room)) != NULL && 
             !IS_SET(zone_table[rp->zone].reset_mode, ZONE_DESERT) && 
             !IS_SET(zone_table[rp->zone].reset_mode, ZONE_ARCTIC) && 
             rp->sector_type != SECT_DESERT) ? 1 : 0 );
#else
    return( FALSE );
#endif
}

void send_to_out_other(char *messg)
{
    SendToAllCond(messg, CondIsOutdoorsOther, NULL);
}

int CondIsInArctic( PlayerStruct_t *player, void *arg )
{
#if 0
    struct room_data *rp;

    return( (OUTSIDE(player->charData) && 
             !IS_SET(player->charData->specials.act, PLR_NOOUTDOOR) && 
             (rp = roomFindNum(player->charData->in_room)) != NULL && 
             IS_SET(zone_table[rp->zone].reset_mode, ZONE_ARCTIC)) ? 1 : 0 );
#else
    return( FALSE );
#endif
}

void send_to_arctic(char *messg)
{
    SendToAllCond(messg, CondIsInArctic, NULL);
}

int CondAllExcept( PlayerStruct_t *player, void *arg )
{
    struct char_data *ch;

    ch = (struct char_data *)arg;

    return( (player->charData != ch) ? 1 : 0 );
}

void send_to_except(char *messg, struct char_data *ch)
{
    SendToAllCond(messg, CondAllExcept, (void *)ch);
}

typedef struct {
    long        zone;
    struct char_data *ch;
} SendToZoneArgs_t;

int CondInZone( PlayerStruct_t *player, void *arg )
{
    SendToZoneArgs_t   *args;
    struct room_data   *rp;

    args = (SendToZoneArgs_t *)arg;
    if( !args ) {
        return( 0 );
    }

    rp = roomFindNum(player->charData->in_room);

    return( (player->charData != args->ch && rp && 
             rp->zone == args->zone) ? 1 : 0);
}

void send_to_zone(char *messg, struct char_data *ch)
{
    SendToZoneArgs_t    args;

    args.zone = roomFindNum(ch->in_room)->zone;
    args.ch   = ch;

    SendToAllCond(messg, CondInZone, (void *)&args);
}

/**
 * @todo make char_data::playerDesc point at the PlayerStruct_t
 */
void send_to_room(char *messg, int room)
{
    struct char_data   *ch;
    PlayerStruct_t     *player;

    if( !messg ) {
        return;
    }

    for (ch = roomFindNum(room)->people; ch; ch = ch->next_in_room) {
        player = (PlayerStruct_t *)ch->playerDesc;
        if (player && GET_POS(ch) > POSITION_SLEEPING) {
            SendOutput(player, messg);
        }
    }
}

void send_to_room_except(char *messg, int room, struct char_data *ch)
{
    struct char_data   *ch2;
    PlayerStruct_t     *player;

    if( !messg ) {
        return;
    }

    for (ch2 = roomFindNum(room)->people; ch2; ch2 = ch2->next_in_room) {
        player = (PlayerStruct_t *)ch2->playerDesc;
        if (ch2 != ch && player && GET_POS(ch2) > POSITION_SLEEPING) {
            SendOutput( player, messg );
        }
    }
}

void send_to_room_except_two(char *messg, int room, struct char_data *ch1, 
                             struct char_data *ch2) 
{
    struct char_data   *ch3;
    PlayerStruct_t     *player;

    if( !messg ) {
        return;
    }

    for (ch3 = roomFindNum(room)->people; ch3; ch3 = ch3->next_in_room) {
        player = (PlayerStruct_t *)ch3->playerDesc;
        if (ch3 != ch1 && ch3 != ch2 && player && 
            GET_POS(ch3) > POSITION_SLEEPING) {
            SendOutput( player, messg );
        }
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

