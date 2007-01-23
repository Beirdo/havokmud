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
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>
#include "protected_data.h"

#include "protos.h"
#include "externs.h"
#include "interthread.h"

static char ident[] _UNUSED_ =
    "$Id$";

int             plr_tick_count = 0;

void JustLoggedIn( PlayerStruct_t *player )
{
    struct obj_data *obj;
    int             count_players;
    reset_char(player->charData);
    total_connections++;
    if (!IS_IMMORTAL(player->charData) ||
        player->charData->invis_level <= 58) {
        LogPrint(LOG_INFO, "Loading %s's equipment", 
                 player->charData->player.name);
    }

    count_players = GetPlayerCount();

    if (total_max_players < count_players) {
        total_max_players = count_players;
    }

    load_char_objs(player->charData);

    save_char(player->charData, AUTO_RENT);
    SendOutput(player, WELC_MESSG);
    player->charData->next = character_list;
    character_list = player->charData;
    if (player->charData->in_room == NOWHERE ||
        player->charData->in_room == AUTO_RENT) {
        if (!IS_IMMORTAL(player->charData)) {
            if (player->charData->specials.start_room <= 0) {
                if (GET_RACE(player->charData) == RACE_HALFLING) {
                    char_to_room(player->charData, 1103);
                    player->charData->player.hometown = 1103;
                } else {
                    char_to_room(player->charData, 3001);
                    player->charData->player.hometown = 3001;
                }
            } else {
                char_to_room(player->charData,
                             player->charData->specials.start_room);
                player->charData->player.hometown =
                    player->charData->specials.start_room;
            }
        } else {
            if (player->charData->specials.start_room <= NOWHERE) {
                char_to_room(player->charData, 1000);
                player->charData->player.hometown = 1000;
            } else {
                if (real_roomp(player->charData->specials.start_room)) {
                    char_to_room(player->charData,
                                 player->charData->specials.start_room);
                    player->charData->player.hometown =
                        player->charData->specials.start_room;
                } else {
                    char_to_room(player->charData, 1000);
                    player->charData->player.hometown = 1000;
                }
            }
        }
    } else if (real_roomp(player->charData->in_room)) {
        char_to_room(player->charData, player->charData->in_room);
        player->charData->player.hometown = player->charData->in_room;
    } else {
        char_to_room(player->charData, 3001);
        player->charData->player.hometown = 3001;
    }

    player->charData->specials.tick = plr_tick_count++;
    if (plr_tick_count == PLR_TICK_WRAP) {
        plr_tick_count = 0;
    }
    act("$n has entered the game.", TRUE, player->charData, 0, 0, TO_ROOM);

    if (!GetMaxLevel(player->charData)) {
        do_start(player->charData);
    }
    do_look(player->charData, NULL, 15);

    /*
     * do an auction check, grant reimbs as needed
     */
    if (player->charData->specials.auction) {
        obj = player->charData->specials.auction;
        player->charData->specials.auction = 0;
        obj->equipped_by = 0;
        obj->eq_pos = -1;

        obj_to_char(obj, player->charData);
        SendOutput(player, "Your item is returned to you.\n\r");
        do_save(player->charData, "", 0);
    }

    if (player->charData->specials.minbid) {
        GET_GOLD(player->charData) += player->charData->specials.minbid;
        player->charData->specials.minbid = 0;
        SendOutput(player, "You are returned your deposit for this "
                           "auction.\n\r");
        do_save(player->charData, "", 0);
    }
    player->prompt_mode = 1;
}

void CommandParser( PlayerStruct_t *player, char *line )
{
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
