#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"

#define MAIN_MENU           0
#define CHANGE_NAME         1
#define CHANGE_DESC         2
#define CHANGE_FLAGS        3
#define CHANGE_TYPE         4
#define CHANGE_TYPE2        5
#define CHANGE_TYPE3        6
#define CHANGE_EXIT         7
#define CHANGE_EXIT_NORTH   8
#define CHANGE_EXIT_EAST    9
#define CHANGE_EXIT_SOUTH   10
#define CHANGE_EXIT_WEST    11
#define CHANGE_EXIT_UP      12
#define CHANGE_EXIT_DOWN    13
#define CHANGE_EXIT_DELETE  14
#define CHANGE_NUMBER_NORTH 15
#define CHANGE_NUMBER_EAST  16
#define CHANGE_NUMBER_SOUTH 17
#define CHANGE_NUMBER_WEST  18
#define CHANGE_NUMBER_UP    19
#define CHANGE_NUMBER_DOWN  20
#define CHANGE_KEY_NORTH    21
#define CHANGE_KEY_EAST     22
#define CHANGE_KEY_SOUTH    23
#define CHANGE_KEY_WEST     24
#define CHANGE_KEY_UP       25
#define CHANGE_KEY_DOWN     26

#define ENTER_CHECK        1


char           *edit_menu =
    "    1) Name                       2) Description\n\r"
    "    3) Flags                      4) Sector Type\n\r"
    "    5) Exits\n\r\n\r";

char           *exit_menu = 
    "    1) North                      2) East\n\r"
    "    3) South                      4) West\n\r"
    "    5) Up                         6) Down\n\r\n\r";

void ChangeRoomFlags(struct room_data *rp, struct char_data *ch, char *arg,
                     int type)
{
    int             i,
                    row,
                    update;

    if (type != ENTER_CHECK) {
        if (!arg || !*arg || (*arg == '\n')) {
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
        }
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 28) {
            return;
        }
        i = 1 << update;

        if (IS_SET(rp->room_flags, i)) {
            REMOVE_BIT(rp->room_flags, i);
        } else {
            SET_BIT(rp->room_flags, i);
        }
    }

    ch_printf(ch, VT_HOMECLR);
    ch_printf(ch, "Room Flags:");

    row = 0;
    for (i = 0; i < 29; i++) {
        ch_printf(ch, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        ch_printf(ch, "%-2d [%s] %s", i + 1,
                  ((rp->room_flags & (1 << i)) ? "X" : " "), room_bits[i]);
    }

    ch_printf(ch, VT_CURSPOS, 23, 1);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void do_redit(struct char_data *ch, char *arg, int cmd)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);

    if (IS_NPC(ch)) {
        return;
    }
    if (!IS_IMMORTAL(ch)) {
        return;
    }
    /* 
     * someone is forced to do something. can be bad! 
     * the ch->desc->str field will cause problems... 
     */
    if (!ch->desc) {
        return;
    }

    if ((GetMaxLevel(ch) < 56) && (rp->zone != GET_ZONE(ch))) {
        send_to_char("Sorry, you are not authorized to edit this zone. Get "
                     "one assigned to you.\n\r", ch);
        return;
    }

    ch->specials.edit = MAIN_MENU;
    ch->desc->connected = CON_EDITING;

    act("$n has begun editing.", FALSE, ch, 0, 0, TO_ROOM);

    UpdateRoomMenu(ch);
}

void UpdateRoomMenu(struct char_data *ch)
{
    struct room_data *rp;

    rp = real_roomp(ch->in_room);

    send_to_char(VT_HOMECLR, ch);
    ch_printf(ch, VT_CURSPOS, 1, 1);
    ch_printf(ch, "Room Name: %s", rp->name);
    ch_printf(ch, VT_CURSPOS, 1, 40);
    ch_printf(ch, "Number: %ld", rp->number);
    ch_printf(ch, VT_CURSPOS, 1, 60);
    ch_printf(ch, "Sector Type: %s", sectors[rp->sector_type].type);
    ch_printf(ch, VT_CURSPOS, 3, 1);
    send_to_char("Menu:\n\r", ch);
    send_to_char(edit_menu, ch);
    send_to_char("--> ", ch);
}

void RoomEdit(struct char_data *ch, char *arg)
{
    if (ch->specials.edit == MAIN_MENU) {
        if (!arg || !*arg || *arg == '\n') {
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing.", FALSE, ch, 0, 0, TO_ROOM);
            return;
        }
        switch (atoi(arg)) {
        case 0:
            UpdateRoomMenu(ch);
            break;
        case CHANGE_NAME:
            ch->specials.edit = CHANGE_NAME;
            ChangeRoomName(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            break;
        case CHANGE_DESC:
            ch->specials.edit = CHANGE_DESC;
            ChangeRoomDesc(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            break;
        case CHANGE_FLAGS:
            ch->specials.edit = CHANGE_FLAGS;
            ChangeRoomFlags(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            break;
        case CHANGE_TYPE:
            ch->specials.edit = CHANGE_TYPE;
            ChangeRoomType(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.edit = CHANGE_EXIT;
            ChangeExitDir(real_roomp(ch->in_room), ch, "", ENTER_CHECK);
            break;
        default:
            UpdateRoomMenu(ch);
            break;
        }
        return;
    }

    switch (ch->specials.edit) {
    case CHANGE_NAME:
        ChangeRoomName(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_DESC:
        ChangeRoomDesc(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_FLAGS:
        ChangeRoomFlags(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_TYPE:
    case CHANGE_TYPE2:
    case CHANGE_TYPE3:
        ChangeRoomType(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_EXIT:
        ChangeExitDir(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_EXIT_NORTH:
    case CHANGE_EXIT_EAST:
    case CHANGE_EXIT_SOUTH:
    case CHANGE_EXIT_WEST:
    case CHANGE_EXIT_UP:
    case CHANGE_EXIT_DOWN:
        AddExitToRoom(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_KEY_NORTH:
    case CHANGE_KEY_EAST:
    case CHANGE_KEY_SOUTH:
    case CHANGE_KEY_WEST:
    case CHANGE_KEY_UP:
    case CHANGE_KEY_DOWN:
        ChangeKeyNumber(real_roomp(ch->in_room), ch, arg, 0);
        break;
    case CHANGE_NUMBER_NORTH:
    case CHANGE_NUMBER_EAST:
    case CHANGE_NUMBER_SOUTH:
    case CHANGE_NUMBER_WEST:
    case CHANGE_NUMBER_UP:
    case CHANGE_NUMBER_DOWN:
        ChangeExitNumber(real_roomp(ch->in_room), ch, arg, 0);
        break;
    default:
        Log("Got to bad spot in RoomEdit");
        break;
    }
}

void ChangeRoomName(struct room_data *rp, struct char_data *ch, char *arg,
                    int type)
{
    if (type != ENTER_CHECK) {
        if (!arg || !*arg || (*arg == '\n')) {
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
        }
    }
    if (type != ENTER_CHECK) {
        if (rp->name) {
            free(rp->name);
        }
        rp->name = (char *) strdup(arg);
        ch->specials.edit = MAIN_MENU;
        UpdateRoomMenu(ch);
        return;
    }

    ch_printf(ch, VT_HOMECLR);

    ch_printf(ch, "Current Room Name: %s", rp->name);
    send_to_char("\n\r\n\rNew Room Name: ", ch);

    return;
}

void ChangeRoomDesc(struct room_data *rp, struct char_data *ch, char *arg,
                    int type)
{
    if (type != ENTER_CHECK) {
        ch->specials.edit = MAIN_MENU;
        UpdateRoomMenu(ch);
        return;
    }

    ch_printf(ch, VT_HOMECLR);

    ch_printf(ch, "Current Room Description:\n\r");
    send_to_char(rp->description, ch);
    send_to_char("\n\r\n\rNew Room Description:\n\r", ch);
    send_to_char("(Use /? for help on editing strings. Press <C/R> again to "
                 "continue)\n\r", ch);
    if (rp->description) {
        free(rp->description);
    }
    rp->description = NULL;
    ch->desc->str = &rp->description;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeRoomType(struct room_data *rp, struct char_data *ch, char *arg,
                    int type)
{
    int             i,
                    row,
                    update;

    if (type != ENTER_CHECK) {
        if (!arg || !*arg || (*arg == '\n')) {
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
        }
    }

    update = atoi(arg);
    update--;

    if (type != ENTER_CHECK) {
        switch (ch->specials.edit) {
        case CHANGE_TYPE:
            if (update >= 0 && update <= 22) {
                rp->sector_type = update;
                if (rp->sector_type == SECT_WATER_NOSWIM) {
                    send_to_char("\n\rRiver Speed: ", ch);
                    ch->specials.edit = CHANGE_TYPE2;
                } else {
                    ch->specials.edit = MAIN_MENU;
                    UpdateRoomMenu(ch);
                }
            }
            return;
            break;
        case CHANGE_TYPE2:
            rp->river_speed = update;
            send_to_char("\n\rRiver Direction (0 - 5): ", ch);
            ch->specials.edit = CHANGE_TYPE3;
            return;
            break;
        case CHANGE_TYPE3:
            update++;
            if (update < 0 || update > 5) {
                send_to_char("Direction must be between 0 and 5.\n\r", ch);
                return;
            }
            rp->river_dir = update;
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
            break;
        }
    }

    ch_printf(ch, VT_HOMECLR);
    ch_printf(ch, "Sector Type: %s", sectors[rp->sector_type].type);

    row = 0;
    for (i = 0; i < 23; i++) {
        ch_printf(ch, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        ch_printf(ch, "%-2d %s", i + 1, sectors[i].type);
    }

    ch_printf(ch, VT_CURSPOS, 22, 1);
    send_to_char("Select the number to set to, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeExitDir(struct room_data *rp, struct char_data *ch, char *arg,
                   int type)
{
    int             update;

    if (type != ENTER_CHECK) {
        if (!arg || !*arg || (*arg == '\n')) {
            ch->specials.edit = MAIN_MENU;
            UpdateRoomMenu(ch);
            return;
        }

        update = atoi(arg) - 1;
        if (update == -1) {
            ChangeExitDir(rp, ch, "", ENTER_CHECK);
            return;
        }

        switch (update) {
        case 0:
            ch->specials.edit = CHANGE_EXIT_NORTH;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 1:
            ch->specials.edit = CHANGE_EXIT_EAST;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 2:
            ch->specials.edit = CHANGE_EXIT_SOUTH;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 3:
            ch->specials.edit = CHANGE_EXIT_WEST;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 4:
            ch->specials.edit = CHANGE_EXIT_UP;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 5:
            ch->specials.edit = CHANGE_EXIT_DOWN;
            AddExitToRoom(rp, ch, "", ENTER_CHECK);
            return;
            break;
        case 6:
            ch->specials.edit = CHANGE_EXIT_DELETE;
            DeleteExit(rp, ch, "", ENTER_CHECK);
            return;
            break;
        default:
            ChangeExitDir(rp, ch, "", ENTER_CHECK);
            return;
            break;
        }

    }

    send_to_char(VT_HOMECLR, ch);
    ch_printf(ch, "Room Name: %s", rp->name);
    ch_printf(ch, VT_CURSPOS, 1, 40);
    ch_printf(ch, "Room Number: %ld", rp->number);
    ch_printf(ch, VT_CURSPOS, 4, 1);
    send_to_char(exit_menu, ch);
    send_to_char("--> ", ch);
}

void AddExitToRoom(struct room_data *rp, struct char_data *ch, char *arg,
                   int type)
{
    int             update,
                    dir = 0,
                    row,
                    i = 0;

    switch (ch->specials.edit) {
    case CHANGE_EXIT_NORTH:
        dir = 0;
        break;
    case CHANGE_EXIT_EAST:
        dir = 1;
        break;
    case CHANGE_EXIT_SOUTH:
        dir = 2;
        break;
    case CHANGE_EXIT_WEST:
        dir = 3;
        break;
    case CHANGE_EXIT_UP:
        dir = 4;
        break;
    case CHANGE_EXIT_DOWN:
        dir = 5;
        break;
    }

    if (type != ENTER_CHECK) {
        if (!arg || !*arg || (*arg == '\n')) {
            switch (dir) {
            case 0:
                ch->specials.edit = CHANGE_NUMBER_NORTH;
                break;
            case 1:
                ch->specials.edit = CHANGE_NUMBER_EAST;
                break;
            case 2:
                ch->specials.edit = CHANGE_NUMBER_SOUTH;
                break;
            case 3:
                ch->specials.edit = CHANGE_NUMBER_WEST;
                break;
            case 4:
                ch->specials.edit = CHANGE_NUMBER_UP;
                break;
            case 5:
                ch->specials.edit = CHANGE_NUMBER_DOWN;
                break;
            }
            send_to_char("\n\r\n\rExit to Room: ", ch);
            ChangeExitNumber(rp, ch, "", ENTER_CHECK);
            return;
        }

        update = atoi(arg) - 1;

        if (update < 0 || update > 6) {
            return;
        }
        i = 1 << update;

        if (IS_SET(rp->dir_option[dir]->exit_info, i)) {
            REMOVE_BIT(rp->dir_option[dir]->exit_info, i);
        } else {
            SET_BIT(rp->dir_option[dir]->exit_info, i);
        }
    } else if (!rp->dir_option[dir]) {
        CREATE(rp->dir_option[dir], struct room_direction_data, 1);
        rp->dir_option[dir]->exit_info = 0;
    }

    ch_printf(ch, VT_HOMECLR);
    ch_printf(ch, "Exit Flags:");

    row = 0;
    for (i = 0; i < 7; i++) {
        ch_printf(ch, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        ch_printf(ch, "%-2d [%s] %s", i + 1,
                  ((rp->dir_option[dir]->exit_info & (1 << i)) ? "X" : " "),
                  exit_bits[i]);
    }

    ch_printf(ch, VT_CURSPOS, 20, 1);
    send_to_char("Select the number to toggle, <C/R> to return to "
                 "continue.\n\r--> ", ch);
}

void ChangeExitNumber(struct room_data *rp, struct char_data *ch,
                      char *arg, int type)
{
    int             dir = 0,
                    update;

    switch (ch->specials.edit) {
    case CHANGE_NUMBER_NORTH:
        dir = 0;
        break;
    case CHANGE_NUMBER_EAST:
        dir = 1;
        break;
    case CHANGE_NUMBER_SOUTH:
        dir = 2;
        break;
    case CHANGE_NUMBER_WEST:
        dir = 3;
        break;
    case CHANGE_NUMBER_UP:
        dir = 4;
        break;
    case CHANGE_NUMBER_DOWN:
        dir = 5;
        break;
    }

    if (type == ENTER_CHECK) {
        return;
    }
    update = atoi(arg);

    if (update < 0 || update > WORLD_SIZE) {
        ch_printf(ch, "\n\rRoom number must be between 0 and %d.\n\r",
                      WORLD_SIZE);
        send_to_char("\n\rExit to Room: ", ch);
        return;
    }

    rp->dir_option[dir]->to_room = update;

    switch (dir) {
    case 0:
        ch->specials.edit = CHANGE_KEY_NORTH;
        break;
    case 1:
        ch->specials.edit = CHANGE_KEY_EAST;
        break;
    case 2:
        ch->specials.edit = CHANGE_KEY_SOUTH;
        break;
    case 3:
        ch->specials.edit = CHANGE_KEY_WEST;
        break;
    case 4:
        ch->specials.edit = CHANGE_KEY_UP;
        break;
    case 5:
        ch->specials.edit = CHANGE_KEY_DOWN;
        break;
    }

    send_to_char("\n\rKey Number (0 for none): ", ch);

    ChangeKeyNumber(rp, ch, "", ENTER_CHECK);
}

void ChangeKeyNumber(struct room_data *rp, struct char_data *ch, char *arg,
                     int type)
{
    int             dir = 0,
                    update;

    switch (ch->specials.edit) {
    case CHANGE_KEY_NORTH:
        dir = 0;
        break;
    case CHANGE_KEY_EAST:
        dir = 1;
        break;
    case CHANGE_KEY_SOUTH:
        dir = 2;
        break;
    case CHANGE_KEY_WEST:
        dir = 3;
        break;
    case CHANGE_KEY_UP:
        dir = 4;
        break;
    case CHANGE_KEY_DOWN:
        dir = 5;
        break;
    }

    if (type == ENTER_CHECK) {
        return;
    }
    update = atoi(arg);

    if (!rp->dir_option[dir]->keyword) {
        rp->dir_option[dir]->keyword = (char *) strdup("door");
    }
    if (update < 0) {
        send_to_char("\n\rKey number must be greater than 0.\n\r", ch);
        send_to_char("\n\rKey Number (0 for none): ", ch);
        return;
    }

    rp->dir_option[dir]->key = update;

    ch->specials.edit = CHANGE_EXIT;
    ChangeExitDir(rp, ch, "", ENTER_CHECK);
}

void DeleteExit(struct room_data *rp, struct char_data *ch, char *arg,
                int type)
{
    ch->specials.edit = MAIN_MENU;
    UpdateRoomMenu(ch);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
