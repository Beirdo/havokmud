/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD 
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

#include "protos.h"

/*
 * extern variables 
 */


int RecGetObjRoom(struct obj_data *obj)
{
    if (obj->in_room != NOWHERE) {
        return (obj->in_room);
    }
    if (obj->carried_by) {
        return (obj->carried_by->in_room);
    }
    if (obj->equipped_by) {
        return (obj->equipped_by->in_room);
    }
    if (obj->in_obj) {
        return (RecGetObjRoom(obj->in_obj));
    }
    return( -1 );
}

void MakeNoise(int room, char *local_snd, char *distant_snd)
{
    int             door;
    struct char_data *ch;
    struct room_data *rp,
                   *orp;

    rp = roomFindNum(room);

    if (!rp) {
        return;
    }

    for (ch = rp->people; ch; ch = ch->next_in_room) {
        if (!IS_NPC(ch) && !IS_AFFECTED(ch, AFF_SILENCE)) {
            send_to_char(local_snd, ch);
        }
    }

    for (door = 0; door <= 5; door++) {
        if (rp->dir_option[door] &&
            (orp = roomFindNum(rp->dir_option[door]->to_room))) {
            for (ch = orp->people; ch; ch = ch->next_in_room) {
                if (!IS_NPC(ch) && !IS_SET(ch->specials.act, PLR_DEAF) && 
                    !IS_AFFECTED(ch, AFF_SILENCE)) {
                    send_to_char(distant_snd, ch);
                }
            }
        }
    }
}

void MakeSound(int pulse)
{
    int             room;
    char            buffer[128];
    struct obj_data *obj;
    struct char_data *ch;

    /**
     *  objects
     */

    BalancedBTreeLock( objectTypeTree );
    for (obj = objectTypeFindFirst(ITEM_TYPE_AUDIO); obj; 
         obj = objectTypeFindNext(ITEM_TYPE_AUDIO, obj)) {
        if ((obj->value[0] && !(pulse % obj->value[0])) || !number(0, 5)) {
            if (obj->carried_by) {
                room = obj->carried_by->in_room;
            } else if (obj->equipped_by) {
                room = obj->equipped_by->in_room;
            } else if (obj->in_room != NOWHERE) {
                room = obj->in_room;
            } else {
                room = RecGetObjRoom(obj);
            }

            /*
             *  broadcast to room
             */
            if (obj->action_description) {
                MakeNoise(room, obj->action_description,
                                obj->action_description);
            }
        }
    }
    BalancedBTreeUnlock( objectTypeTree );

    /*
     *   mobiles
     */
    /**
     * @todo Convert to new LinkedList methodology
     */
    for (ch = character_list; ch; ch = ch->next) {
        if (IS_NPC(ch) && (ch->player.sounds) && !number(0, 5) && 
            strcmp(ch->player.sounds, "")) {
            /* 
             * don't make sound if empty sound string 
             */
            if (ch->specials.default_pos > POSITION_SLEEPING) {
                if (GET_POS(ch) > POSITION_SLEEPING) {
                    /*
                     * Make the sound 
                     */
                    MakeNoise(ch->in_room, ch->player.sounds,
                              ch->player.distant_snds);
                } else if (GET_POS(ch) == POSITION_SLEEPING) {
                    /*
                     * snore 
                     */
                    sprintf(buffer, "%s snores loudly.\n\r",
                            ch->player.short_descr);
                    MakeNoise(ch->in_room, buffer,
                              "You hear a loud snore nearby.\n\r");
                }
            } else if (GET_POS(ch) == ch->specials.default_pos) {
                /*
                 * Make the sound 
                 */
                MakeNoise(ch->in_room, ch->player.sounds,
                          ch->player.distant_snds);
            }
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
