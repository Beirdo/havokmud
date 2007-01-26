#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "protos.h"

/*
 * external vars 
 */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int      top_of_world;
extern struct int_app_type int_app[26];
extern char    *pc_class_types[];
extern char    *dirs[];
extern struct QuestItem QuestList[4][IMMORTAL];
extern int      gSeason;        /* what season is it ? */

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
extern char    *spells[];
extern int      rev_dir[];


/*
 * chess_game() stuff starts here 
 * Inspiration and original idea by Feith 
 * Implementation by Gecko 
 */

#define WHITE 0
#define BLACK 1

int             side = WHITE;   /* to avoid having to pass side with each
                                 * function call */

#define IS_BLACK(piece) (((piece) >= 1400) && ((piece) <= 1415))
#define IS_WHITE(piece) (((piece) >= 1448) && ((piece) <= 1463))
#define IS_PIECE(piece) ((IS_WHITE(piece)) || (IS_BLACK(piece)))
#define IS_ENEMY(piece) (side?IS_WHITE(piece):IS_BLACK(piece))
#define IS_FRIEND(piece) (side?IS_BLACK(piece):IS_WHITE(piece))
#define ON_BOARD(room) (((room) >= 1400) && ((room) <= 1463))
#define FORWARD (side?2:0)
#define BACK    (side?0:2)
#define LEFT    (side?1:3)
#define RIGHT   (side?3:1)

#define EXIT_ROOM(roomp,dir) ((roomp)?((roomp)->dir_option[dir]):NULL)
#define CAN_GO_ROOM(roomp,dir) (EXIT_ROOM(roomp,dir) && \
                               real_roomp(EXIT_ROOM(roomp,dir)->to_room))

/*
 * get pointer to room in the given direction 
 */
#define ROOMP(roomp,dir) ((CAN_GO_ROOM(roomp,dir)) ? \
                          real_roomp(EXIT_ROOM(roomp,dir)->to_room) : NULL)

struct room_data *forward_square(struct room_data *room)
{
    return ROOMP(room, FORWARD);
}

struct room_data *back_square(struct room_data *room)
{
    return ROOMP(room, BACK);
}

struct room_data *left_square(struct room_data *room)
{
    return ROOMP(room, LEFT);
}

struct room_data *right_square(struct room_data *room)
{
    return ROOMP(room, RIGHT);
}

struct room_data *forward_left_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, FORWARD), LEFT);
}

struct room_data *forward_right_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, FORWARD), RIGHT);
}

struct room_data *back_right_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, BACK), RIGHT);
}

struct room_data *back_left_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, BACK), LEFT);
}

struct char_data *square_contains_enemy(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_ENEMY(mob_index[i->nr].virtual)) {
            return i;
        }
    }

    return NULL;
}

int square_contains_friend(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_FRIEND(mob_index[i->nr].virtual)) {
            return TRUE;
        }
    }

    return FALSE;
}

int square_empty(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_PIECE(mob_index[i->nr].virtual)) {
            return FALSE;
        }
    }

    return TRUE;
}

int chess_game(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct room_data *rp = NULL,
                   *crp = real_roomp(ch->in_room);
    struct char_data *ep = NULL;
    int             move_dir = 0,
                    move_amount = 0,
                    move_found = FALSE;
    int             c = 0;

    if (cmd || !AWAKE(ch)) {
        return FALSE;
    }
    /*  
     * keep original fighter() spec_proc for kings and knights 
     */
    if (ch->specials.fighting) {
        switch (mob_index[ch->nr].virtual) {
        case 1401:
        case 1404:
        case 1406:
        case 1457:
        case 1460:
        case 1462:
            return fighter(ch, cmd, arg, mob, type);
        default:
            return FALSE;
        }
    }
    if (!crp || !ON_BOARD(crp->number)) {
        return FALSE;
    }
    if (side == WHITE && IS_BLACK(mob_index[ch->nr].virtual)) {
        return FALSE;
    }
    if (side == BLACK && IS_WHITE(mob_index[ch->nr].virtual)) {
        return FALSE;
    }
    if (number(0, 15)) {
        return FALSE;
    }
    switch (mob_index[ch->nr].virtual) {
    case 1408:                  /* black pawns */
    case 1409:
    case 1410:
    case 1411:
    case 1412:
    case 1413:
    case 1414:
    case 1415:
    case 1448:
    case 1449:
    case 1450:
    case 1451:                  /* white pawns */
    case 1452:
    case 1453:
    case 1454:
    case 1455:
        move_dir = number(0, 3);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(crp);
            break;
        case 1:
            rp = forward_right_square(crp);
            break;
        case 2:
            rp = forward_square(crp);
            break;
        case 3:
            if (real_roomp(ch->in_room) &&
                real_roomp(ch->in_room)->number == mob_index[ch->nr].virtual) {
                rp = forward_square(crp);
                if (rp && square_empty(rp) && ON_BOARD(rp->number)) {
                    crp = rp;
                    rp = forward_square(crp);
                }
            }
        }
        if (rp && (!square_contains_friend(rp)) && ON_BOARD(rp->number)) {
            ep = square_contains_enemy(rp);
            if ((move_dir <= 1 && ep) || (move_dir > 1 && !ep)) {
                move_found = TRUE;
            }
        }
        break;

    case 1400:                  /* black rooks */
    case 1407:
    case 1456:                  /* white rooks */
    case 1463:
        move_dir = number(0, 3);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_square(crp);
                break;
            case 1:
                rp = back_square(crp);
                break;
            case 2:
                rp = right_square(crp);
                break;
            case 3:
                rp = left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1401:                  /* black knights */
    case 1406:
    case 1457:                  /* white knights */
    case 1462:
        move_dir = number(0, 7);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(forward_square(crp));
            break;
        case 1:
            rp = forward_right_square(forward_square(crp));
            break;
        case 2:
            rp = forward_right_square(right_square(crp));
            break;
        case 3:
            rp = back_right_square(right_square(crp));
            break;
        case 4:
            rp = back_right_square(back_square(crp));
            break;
        case 5:
            rp = back_left_square(back_square(crp));
            break;
        case 6:
            rp = back_left_square(left_square(crp));
            break;
        case 7:
            rp = forward_left_square(left_square(crp));
        }

        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;

    case 1402:                  /* black bishops */
    case 1405:
    case 1458:                  /* white bishops */
    case 1461:
        move_dir = number(0, 3);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_left_square(crp);
                break;
            case 1:
                rp = forward_right_square(crp);
                break;
            case 2:
                rp = back_right_square(crp);
                break;
            case 3:
                rp = back_left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1403:                  /* black queen */
    case 1459:                  /* white queen */
        move_dir = number(0, 7);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_left_square(crp);
                break;
            case 1:
                rp = forward_square(crp);
                break;
            case 2:
                rp = forward_right_square(crp);
                break;
            case 3:
                rp = right_square(crp);
                break;
            case 4:
                rp = back_right_square(crp);
                break;
            case 5:
                rp = back_square(crp);
                break;
            case 6:
                rp = back_left_square(crp);
                break;
            case 7:
                rp = left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1404:                  /* black king */
    case 1460:                  /* white king */
        move_dir = number(0, 7);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(crp);
            break;
        case 1:
            rp = forward_square(crp);
            break;
        case 2:
            rp = forward_right_square(crp);
            break;
        case 3:
            rp = right_square(crp);
            break;
        case 4:
            rp = back_right_square(crp);
            break;
        case 5:
            rp = back_square(crp);
            break;
        case 6:
            rp = back_left_square(crp);
            break;
        case 7:
            rp = left_square(crp);
        }

        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;
    }

    if (move_found && rp) {
        command_interpreter(ch, "emote leaves the room.");
        char_from_room(ch);
        char_to_room(ch, rp->number);
        command_interpreter(ch, "emote has arrived.");

        if (ep) {
            if (side) {
                switch (number(0, 3)) {
                case 0:
                    command_interpreter(ch, "emote grins evilly and says, "
                                            "'ONLY EVIL shall rule!'");
                    break;
                case 1:
                    command_interpreter(ch, "emote leers cruelly and says, "
                                            "'You will die now!'");
                    break;
                case 2:
                    command_interpreter(ch, "emote issues a bloodcurdling "
                                            "scream.");
                    break;
                case 3:
                    command_interpreter(ch, "emote glares with black anger.");
                    break;
                }
            } else {
                switch (number(0, 3)) {
                case 0:
                    command_interpreter(ch, "emote glows an even brighter "
                                            "pristine white.");
                    break;
                case 1:
                    command_interpreter(ch, "emote chants a prayer and begins "
                                            "battle.");
                    break;
                case 2:
                    command_interpreter(ch, "emote says, 'Black shall lose!");
                    break;
                case 3:
                    command_interpreter(ch, "emote shouts, 'For the Flame! The"
                                            " Flame!'");
                    break;
                }
            }
            hit(ch, ep, TYPE_UNDEFINED);
        }
        side = (side + 1) % 2;
        return TRUE;
    }
    return FALSE;
}

int AcidBlob(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
        if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && 
            !strncmp(i->name, "corpse", 6)) {
            act("$n destroys some trash.", FALSE, ch, 0, 0, TO_ROOM);

            obj_from_room(i);
            extract_obj(i);
            return (TRUE);
        }
    }
    return (FALSE);
}


int avatar_celestian(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (number(0, 1)) {
        return (fighter(mob, cmd, arg, mob, type));
    } else {
        return (magic_user(mob, cmd, arg, mob, type));
    }
}







int EvilBlade(struct char_data *ch, int cmd, char *arg,
              struct obj_data *tobj, int type)
{
#ifdef USE_EGOS
    extern struct str_app_type str_app[];
    struct obj_data *obj,
                   *blade;
    struct char_data *joe,
                   *holder;
    struct char_data *lowjoe = 0;
    char           *arg1,
                    buf[250];

    if ((type != PULSE_COMMAND) || (IS_IMMORTAL(ch)) || 
        (!real_roomp(ch->in_room))) {
        return (FALSE);
    }
    for (obj = real_roomp(ch->in_room)->contents;
         obj; obj = obj->next_content) {
        if (obj_index[obj->item_number].func == EvilBlade) {
            /*
             * I am on the floor 
             */
            for (joe = real_roomp(ch->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if ((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe) {
                if (CAN_GET_OBJ(lowjoe, obj)) {
                    obj_from_room(obj);
                    obj_to_char(obj, lowjoe);

                    sprintf(buf, "%s leaps into your hands!\n\r",
                            obj->short_description);
                    send_to_char(buf, lowjoe);

                    sprintf(buf, "%s jumps from the floor and leaps into %s's"
                                 " hands!\n\r",
                            obj->short_description, GET_NAME(lowjoe));
                    act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                    if (!EgoBladeSave(lowjoe)) {
                        if (!lowjoe->equipment[WIELD]) {
                            sprintf(buf, "%s forces you to wield it!\n\r",
                                    obj->short_description);
                            send_to_char(buf, lowjoe);
                            wear(lowjoe, obj, 12);
                            return (FALSE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to make you "
                                         "wield it.\n\r", 
                                    obj->short_description);
                            send_to_char(buf, lowjoe);
                            return (FALSE);
                        }
                    }
                }
            }
        }
    }

    for (holder = real_roomp(ch->in_room)->people; holder;
         holder = holder->next_in_room) {
        for (obj = holder->carrying; obj; obj = obj->next_content) {
            if (obj_index[obj->item_number].func &&
                obj_index[obj->item_number].func != board) {
                /*
                 * held
                 */
                if (holder->equipment[WIELD] && !EgoBladeSave(holder) && 
                    !EgoBladeSave(holder)) {
                    sprintf(buf, "%s gets pissed off that you are wielding "
                                 "another weapon!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                    sprintf(buf, "%s knocks %s out of your hands!!\n\r",
                            obj->short_description,
                            holder->equipment[WIELD]->short_description);
                    send_to_char(buf, holder);

                    blade = unequip_char(holder, WIELD);
                    if (blade) {
                        obj_to_room(blade, holder->in_room);
                    }
                    if (!holder->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);
                        wear(holder, obj, 12);
                        return (FALSE);
                    }
                }

                if (!EgoBladeSave(holder) && !EgoBladeSave(holder) &&
                    !holder->equipment[WIELD]) {
                    sprintf(buf, "%s forces you to wield it!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                    wear(holder, obj, 12);
                    return (FALSE);
                }

                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the %s, you feel less enthused about"
                                 " your master.\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }
            }
        }

        if (holder->equipment[WIELD] && 
            obj_index[holder->equipment[WIELD]->item_number].func && 
            obj_index[holder->equipment[WIELD]->item_number].func != board) {
            /*
             * YES! I am being held!
             */
            obj = holder->equipment[WIELD];
            if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                affect_from_char(holder, SPELL_CHARM_PERSON);
                sprintf(buf, "Due to the %s, you feel less enthused about "
                             "your master.\n\r", obj->short_description);
                send_to_char(buf, holder);
            }

            if (holder->specials.fighting) {
                sprintf(buf, "%s almost sings in your hand!\n\r",
                        obj->short_description);
                send_to_char(buf, holder);
                sprintf(buf, "You can hear $n's %s almost sing with joy!",
                        obj->short_description);
                act(buf, FALSE, holder, 0, 0, TO_ROOM);

                if (holder == ch && cmd == 151) {
                    if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
                        sprintf(buf, "You can feel %s attempt to stay in the "
                                     "fight!\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        return (FALSE);
                    } else {
                        sprintf(buf, "%s laughs at your attempt to flee from "
                                     "a fight!\n\r", obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s gives you a little warning...\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s twists around and smacks you!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "Wow! $n's %s just whipped around and "
                                     "smacked $m one!", obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);

                        GET_HIT(ch) -= 25;
                        if (GET_HIT(ch) < 0) {
                            GET_HIT(ch) = 0;
                            GET_POS(ch) = POSITION_STUNNED;
                        }
                        return (TRUE);
                    }
                }
            }
            
            if (cmd == 66 && holder == ch) {
                arg = get_argument(arg, &arg1);
                if( !arg1 ) {
                    send_to_char( "Remove what?\n\r", ch );
                    return( FALSE );
                }

                if (strcmp(arg1, "all") == 0) {
                    if (!EgoBladeSave(ch)) {
                        sprintf(buf, "%s laughs at your attempt remove it!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s gives you a little warning...\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s twists around and smacks you!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "Wow! $n's %s just whipped around and "
                                     "smacked $m one!", obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);

                        GET_HIT(ch) -= 25;
                        if (GET_HIT(ch) < 0) {
                            GET_HIT(ch) = 0;
                            GET_POS(ch) = POSITION_STUNNED;
                        }
                        return (TRUE);
                    } else {
                        sprintf(buf, "You can feel %s attempt to stay "
                                     "wielded!\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        return (FALSE);
                    }
                } else {
                    if (isname(arg1, obj->name)) {
                        if (!EgoBladeSave(ch)) {
                            sprintf(buf, "%s laughs at your attempt to remove"
                                         " it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!", 
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 25;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to stay "
                                         "wielded!\n\r", 
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        }
                    }
                }
            }

            for (joe = real_roomp(holder->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if (GET_ALIGNMENT(joe) >= 500 && IS_MOB(joe) && 
                    CAN_SEE(holder, joe) && holder != joe) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe) {
                if (!EgoBladeSave(holder)) {
                    if (GET_POS(holder) != POSITION_STANDING) {
                        sprintf(buf, "%s yanks you to your feet!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);
                        GET_POS(holder) = POSITION_STANDING;
                    }
                    sprintf(buf, "%s leaps out of control!!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                    sprintf(buf, "%s jumps for $n's neck!",
                            obj->short_description);
                    act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);
                    sprintf(buf, "hit %s", GET_NAME(lowjoe));
                    command_interpreter(holder, buf);
                    return (TRUE);
                } else {
                    return (FALSE);
                }
            }

            if (cmd == 70 && holder == ch) {
                sprintf(buf, "%s almost sings in your hands!!\n\r",
                        obj->short_description);
                send_to_char(buf, ch);
                sprintf(buf, "You can hear $n's %s almost sing with joy!",
                        obj->short_description);

                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                return (FALSE);
            }
        }
    }
#endif
    return (FALSE);
}

int GoodBlade(struct char_data *ch, int cmd, char *arg,
              struct obj_data *tobj, int type)
{
#ifdef USE_EGOS
    extern struct str_app_type str_app[];
    struct obj_data *obj,
                   *blade;
    struct char_data *joe,
                   *holder;
    struct char_data *lowjoe = 0;
    char           *arg1,
                    buf[250];

    if ((type != PULSE_COMMAND) || (IS_IMMORTAL(ch)) || 
        (!real_roomp(ch->in_room))) {
        return (FALSE);
    }
    return (FALSE);
    /*
     * disabled
     */
    for (obj = real_roomp(ch->in_room)->contents;
         obj; obj = obj->next_content) {
        if (obj_index[obj->item_number].func == GoodBlade) {
            /*
             * I am on the floor 
             */
            for (joe = real_roomp(ch->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if (GET_ALIGNMENT(joe) >= 350 && !IS_IMMORTAL(joe)) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe && CAN_GET_OBJ(lowjoe, obj)) {
                obj_from_room(obj);
                obj_to_char(obj, lowjoe);

                sprintf(buf, "%s leaps into your hands!\n\r",
                        obj->short_description);
                send_to_char(buf, lowjoe);

                sprintf(buf, "%s jumps from the floor and leaps into %s's "
                             "hands!\n\r", obj->short_description,
                        GET_NAME(lowjoe));
                act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                if (!EgoBladeSave(lowjoe)) {
                    if (!lowjoe->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, lowjoe);
                        wear(lowjoe, obj, 12);
                        return (FALSE);
                    } else {
                        sprintf(buf, "You can feel %s attept to make you wield"
                                     " it.\n\r", obj->short_description);
                        send_to_char(buf, lowjoe);
                        return (FALSE);
                    }
                }
            }
        }
    }

    for (holder = real_roomp(ch->in_room)->people; holder;
         holder = holder->next_in_room) {
        for (obj = holder->carrying; obj; obj = obj->next_content) {
            if (obj_index[obj->item_number].func &&
                obj_index[obj->item_number].func != board) {
                /*
                 * held
                 */
                if (holder->equipment[WIELD] && !EgoBladeSave(holder) && 
                    !EgoBladeSave(holder)) {
                    sprintf(buf, "%s gets pissed off that you are wielding "
                                 "another weapon!\n\r", obj->short_description);
                    send_to_char(buf, holder);

                    sprintf(buf, "%s knocks %s out of your hands!!\n\r",
                            obj->short_description,
                            holder->equipment[WIELD]->short_description);
                    send_to_char(buf, holder);

                    blade = unequip_char(holder, WIELD);
                    if (blade) {
                        obj_to_room(blade, holder->in_room);
                    }
                    if (!holder->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);
                        wear(holder, obj, 12);
                        return (FALSE);
                    }
                }

                if (!EgoBladeSave(holder) && !EgoBladeSave(holder) &&
                    !holder->equipment[WIELD]) {
                    sprintf(buf, "%s forces you yto wield it!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                    wear(holder, obj, 12);
                    return (FALSE);
                }

                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the effects of %s, you feel less "
                                 "enthused about your master.\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }
            }
        }

        if (holder->equipment[WIELD]) {
            if (obj_index[holder->equipment[WIELD]->item_number].func && 
                obj_index[holder->equipment[WIELD]->item_number].func != 
                   board) {
                /*
                 * YES! I am being held!
                 */
                obj = holder->equipment[WIELD];

                /*
                 * remove charm 
                 */
                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the effects of %s, you feel less "
                                 "enthused about your master.\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove blindness 
                 */
                if (affected_by_spell(holder, SPELL_BLINDNESS)) {
                    affect_from_char(holder, SPELL_BLINDNESS);
                    sprintf(buf, "%s hums in your hands, you can see!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove chill touch 
                 */
                if (affected_by_spell(holder, SPELL_CHILL_TOUCH)) {
                    affect_from_char(holder, SPELL_CHILL_TOUCH);
                    sprintf(buf, "%s hums in your hands, you feel warm "
                                 "again!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove slow 
                 */
                if (affected_by_spell(holder, SPELL_SLOW)) {
                    affect_from_char(holder, SPELL_SLOW);
                    sprintf(buf, "%s hums in your hands, you feel yourself "
                                 "speed back up!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove poison 
                 */
                if (affected_by_spell(holder, SPELL_POISON)) {
                    affect_from_char(holder, SPELL_POISON);
                    sprintf(buf, "%s hums in your hands, the sick feeling "
                                 "fades!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * wielder is hurt, heal them 
                 */
                if (number(1, 101) > 90 && 
                    GET_HIT(holder) < GET_MAX_HIT(holder) / 2) {
                    act("You get a gentle warm pulse from $p, you feel MUCH "
                        "better!", FALSE, holder, obj, 0, TO_CHAR);
                    act("$n smiles as $p pulses in $s hands!", FALSE,
                        holder, obj, 0, TO_ROOM);
                    GET_HIT(holder) = GET_MAX_HIT(holder) - number(1, 10);
                    return (FALSE);
                }

                if (holder->specials.fighting) {
                    sprintf(buf, "%s almost sings in your hand!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);

                    sprintf(buf, "You can hear %s almost sing with joy in $n's"
                                 " hands!", obj->short_description);
                    act(buf, FALSE, holder, 0, 0, TO_ROOM);

                    if (holder == ch && cmd == 151) {
                        if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
                            sprintf(buf, "You can feel %s attempt to stay in "
                                         "the fight!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        } else {
                            sprintf(buf, "%s laughs at your attempt to flee "
                                         "from a fight!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        }
                    }
                }

                if (cmd == 66 && holder == ch) {
                    arg = get_argument(arg, &arg1);
                    if( !arg1 ) {
                        send_to_char("Remove what?\n\r", ch);
                        return( FALSE );
                    }

                    if (strcmp(arg1, "all") == 0) {
                        if (!EgoBladeSave(ch)) {
                            sprintf(buf, "%s laughs at your attempt remove "
                                         "it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to stay "
                                         "wielded!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        }
                    } else if (isname(arg1, obj->name)) {
                        if (!EgoBladeSave(ch)) {
                            sprintf(buf, "%s laughs at your attempt to remove"
                                         " it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to stay "
                                         "wielded!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        }
                    }
                }

                for (joe = real_roomp(holder->in_room)->people; joe;
                     joe = joe->next_in_room) {
                    if (GET_ALIGNMENT(joe) <= -350 && IS_MOB(joe) && 
                        CAN_SEE(holder, joe) && holder != joe) {
                        if (lowjoe) {
                            if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
                                lowjoe = joe;
                            }
                        } else
                            lowjoe = joe;
                    }
                }

                if (lowjoe) {
                    if (!EgoBladeSave(holder)) {
                        if (GET_POS(holder) != POSITION_STANDING) {
                            sprintf(buf, "%s yanks you yo your feet!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            GET_POS(holder) = POSITION_STANDING;
                        }

                        sprintf(buf, "%s leaps out of control!!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);

                        sprintf(buf, "%s howls out for $n's neck!",
                                obj->short_description);
                        act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                        sprintf(buf, "hit %s", GET_NAME(lowjoe));
                        command_interpreter(holder, buf);
                        return (TRUE);
                    } else {
                        return (FALSE);
                    }
                }

                if (cmd == 70 && holder == ch) {
                    sprintf(buf, "%s almost sings in your hands!!\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);

                    sprintf(buf, "You can hear $n's %s almost sing with joy!",
                            obj->short_description);
                    act(buf, FALSE, ch, 0, 0, TO_ROOM);
                    return (FALSE);
                }
            }
        }
    }

#endif
    return (FALSE);
}

int NeutralBlade(struct char_data *ch, int cmd, char *arg,
                 struct obj_data *tobj, int type)
{
#ifdef USE_EGOS
    return (FALSE);
#endif
}




int magic_user_imp(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell;
    char            buf[254];

    if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    /*
     * might move this somewhere else later... 
     */

    SET_BIT(ch->player.class, CLASS_MAGIC_USER);
    ch->player.level[MAGE_LEVEL_IND] = GetMaxLevel(ch);

    if (!ch->specials.fighting && !IS_PC(ch)) {
        if (GetMaxLevel(ch) < 25) {
            return FALSE;
        } else {
            if (!ch->desc) {
                if (Summoner(ch, cmd, arg, mob, type)) {
                    return (TRUE);
                } else if (NumCharmedFollowersInRoom(ch) < 5 && 
                         IS_SET(ch->hatefield, HATE_CHAR)) {
                    act("$n utters the words 'Here boy!'.", 1, ch, 0,
                        0, TO_ROOM);
                    cast_mon_sum7(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                    command_interpreter(ch, "order followers guard on");
                    return (TRUE);
                }
            }
            return FALSE;
        }
    }

    if (!ch->specials.fighting) {
        return FALSE;
    }
    if (!IS_PC(ch) && GET_POS(ch) > POSITION_STUNNED &&
        GET_POS(ch) < POSITION_FIGHTING) {
        StandUp(ch);
        if (GET_HIT(ch) <= GET_HIT(ch->specials.fighting) / 2) {
            do_flee(ch, NULL, 0);
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return (TRUE);
    }

    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (number(0, 1) && UseViolentHeldItem(ch)) {
        return (TRUE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    /*
     * Find a dude to to evil things upon ! 
     */

    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    lspell = number(0, GetMaxLevel(ch));
    if (!IS_PC(ch)) {
        /* weight it towards the upper levels of the mages range */
        lspell += GetMaxLevel(ch) / 5;
    }
    lspell = MIN(GetMaxLevel(ch), lspell);

    /*
     **  check your own problems:
     */

    if (lspell < 1) {
        lspell = 1;
    }
    /*
     ** only problem I can see with this new spell casting is if the mobs
     ** name is the same as the victim....
     */

    if (IS_AFFECTED(ch, AFF_BLIND) && lspell > 15) {
        sprintf(buf, "cast 'remove blind' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 10 &&
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        sprintf(buf, "cast 'dispel magic' %s", GET_NAME(vict));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 10 &&
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        sprintf(buf, "cast 'dispel magic' %s", GET_NAME(vict));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 28 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {

#if 0
        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
            return (TRUE);
        }
#endif
        sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 15 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (GET_HIT(ch) > GET_MAX_HIT(ch) / 2 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
        GetMaxLevel(vict) < GetMaxLevel(ch) && number(0, 1)) {

        /*
         **  Non-damaging case:
         */

        if (lspell > 8 && lspell < 50 && !number(0, 6)) {
            sprintf(buf, "cast 'web' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 6)) {
            sprintf(buf, "cast 'weakness' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 7)) {
            sprintf(buf, "cast 'armor' %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 12 && lspell < 20 && !number(0, 7)) {
            sprintf(buf, "cast 'curse' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 10 && lspell < 20 && !number(0, 5)) {
            sprintf(buf, "cast 'blind' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 8 && lspell < 40 && !number(0, 5) &&
            vict->specials.fighting != ch) {
            sprintf(buf, "cast 'charm monster' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            if (IS_AFFECTED(vict, AFF_CHARM)) {
                if (!vict->specials.fighting) {
                    sprintf(buf, "order %s kill %s",
                            GET_NAME(vict), GET_NAME(ch->specials.fighting));
                } else {
                    sprintf(buf, "order %s remove all", GET_NAME(vict));
                }
                command_interpreter(ch, buf);
            }
        }

        /*
         **  The really nifty case:
         */
        switch (lspell) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            command_interpreter(ch, "cast 'monsum one'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 11:
        case 12:
        case 13:
            command_interpreter(ch, "cast 'monsum two'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 14:
        case 15:
            command_interpreter(ch, "cast 'monsum three'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 16:
        case 17:
        case 18:
            command_interpreter(ch, "cast 'monsum four'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 19:
        case 20:
        case 21:
        case 22:
            command_interpreter(ch, "cast 'monsum five'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 23:
        case 24:
        case 25:
            command_interpreter(ch, "cast 'monsum six'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 26:
        default:
            command_interpreter(ch, "cast 'monsum seven'");
            command_interpreter(ch, "order followers guard on");
            break;
        }
        return (TRUE);
    } else {
        buf[0] = '\0';
        switch (lspell) {
        case 1:
        case 2:
            sprintf(buf, "cast 'magic missle' %s", GET_NAME(vict));
            break;
        case 3:
        case 4:
        case 5:
            sprintf(buf, "cast 'shocking grasp' %s", GET_NAME(vict));
            break;
        case 6:
        case 7:
        case 8:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'web' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'burning hands' %s", GET_NAME(vict));
            }
            break;
        case 9:
        case 10:
            sprintf(buf, "cast 'acid blast' %s", GET_NAME(vict));
            break;
        case 11:
        case 12:
        case 13:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'lightning bolt' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'ice storm' %s", GET_NAME(vict));
            }
            break;
        case 14:
        case 15:
            sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'colour spray' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'cone of cold' %s", GET_NAME(vict));
            }
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            sprintf(buf, "cast 'fireball' %s", GET_NAME(vict));
            break;
        case 38:
        case 39:
        case 40:
        case 41:
            if (IS_EVIL(ch)) {
                sprintf(buf, "cast 'energy drain' %s", GET_NAME(vict));
            }
            break;
        default:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'meteor swarm' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'fireball' %s", GET_NAME(vict));
            }
            break;
        }
        if( *buf ) {
            command_interpreter(ch, buf);
        }
    }
    return TRUE;
}

int cleric_imp(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell,
                    healperc = 0;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) != POSITION_FIGHTING) {
        if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }
        return FALSE;
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (!ch->specials.fighting && GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
        if ((lspell = GetMaxLevel(ch)) >= 20) {
            act("$n utters the words 'What a Rush!'.", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else if (lspell > 12) {
            act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1,
                ch, 0, 0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else if (lspell > 8) {
            act("$n utters the words 'I feel much better now!'.", 1,
                ch, 0, 0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else {
            act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        }
    }

    /*
     * Find a dude to to evil things upon ! 
     */

    if ((vict = FindAHatee(ch)) == NULL) {
        vict = FindVictim(ch);
    }
    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    /*
     * gen number from 0 to level 
     */

    lspell = number(0, GetMaxLevel(ch));
    lspell += GetMaxLevel(ch) / 5;
    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 31 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n utters the words 'Woah! I'm outta here!'",
            1, ch, 0, 0, TO_ROOM);

        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            return (TRUE);
        }

        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    /*
     * first -- hit a foe, or help yourself? 
     */

    if (ch->points.hit < (ch->points.max_hit / 2)) {
        healperc = 7;
    } else if (ch->points.hit < (ch->points.max_hit / 4)) {
        healperc = 5;
    } else if (ch->points.hit < (ch->points.max_hit / 8)) {
        healperc = 3;
    }
    if (number(1, healperc + 2) > 3) {
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 &&
            !number(0, 5)) {
            act("$n whistles.", 1, ch, 0, 0, TO_ROOM);
            act("$n utters the words 'Here Lightning!'.", 1, ch, 0, 0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
        case 3:
            act("$n utters the words 'Moo ha ha!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            act("$n utters the words 'Hocus Pocus!'.", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 7:
            act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 8:
            act("$n utters the words 'Urgle Blurg'.", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 11:
            act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0, 0, TO_ROOM);
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!IS_SET(vict->M_immune, IMM_FIRE)) {
                act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0,
                    0, TO_ROOM);
                cast_flamestrike(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_SANCTUARY) && 
                       GetMaxLevel(ch) > GetMaxLevel(vict)) {
                act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
                cast_cause_critic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 17:
        case 18:
        case 19:
        default:
            act("$n utters the words 'Hurts, doesn't it?'.", 1, ch, 0, 0,
                TO_ROOM);
            cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        }

        return (TRUE);
    } else {
        if (IS_AFFECTED(ch, AFF_BLIND) && lspell >= 4 && !number(0, 3)) {
            act("$n utters the words 'Praise <Deity Name>, I can SEE!'.",
                1, ch, 0, 0, TO_ROOM);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_CURSE) && lspell >= 6 && !number(0, 6)) {
            act("$n utters the words 'I'm rubber, you're glue.", 1, ch, 0,
                0, TO_ROOM);
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_POISON) && lspell >= 5 && !number(0, 6)) {
            act("$n utters the words 'Praise <Deity Name> I don't feel sick "
                "no more!'.", 1, ch, 0, 0, TO_ROOM);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'Abrazak'.", 1, ch, 0, 0, TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            act("$n utters the words 'I feel much better now!'.", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
        case 18:
            act("$n utters the words 'What a Rush!'.", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            act("$n utters the words 'Oooh, pretty!'.", 1, ch, 0, 0, TO_ROOM);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        }

        return (TRUE);
    }
}





/*
 * well, this paladin will give a tough battle.. imho paladins just can't
 * be stupid and do _random_ things on yourself and on opponents.. grin
 * 
 */

int Paladin(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *vict,
                   *tch;
    char            buf[255];

    if (!ch->skills || GET_LEVEL(ch, PALADIN_LEVEL_IND) <= 0) { 
        /* init skills */
        SET_BIT(ch->player.class, CLASS_PALADIN);
        if (!ch->skills) {
            SpaceForSkills(ch);
        }
        GET_LEVEL(ch, PALADIN_LEVEL_IND) = GetMaxLevel(ch);
        /*
         * set skill levels 
         */
        ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_HOLY_WARCRY].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_LAY_ON_HANDS].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_BASH].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_KICK].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_BLESSING].learned = GetMaxLevel(ch) + 40;
    }

    if (cmd) {
        /*
         * we will not give free blesses or lay on hands.. we will just
         * ignore all commands.. yah. 
         */
        return (FALSE);
    }

    if (affected_by_spell(ch, SPELL_PARALYSIS)) {
        /* poor guy.. */
        return (FALSE);
    }

    if (!AWAKE(mob) && !affected_by_spell(mob, SPELL_SLEEP)) {
        /*
         * hey, why you sleeping guy? STAND AND FIGHT! 
         */
        command_interpreter(mob, "wake");
        command_interpreter(mob, "stand");
        return (TRUE);
    }

    if (ch->specials.fighting && ch->specials.fighting != ch) {
        if (GET_POS(ch) == POSITION_SITTING || 
            GET_POS(ch) == POSITION_RESTING) {
            do_stand(ch, NULL, 0);
            return (TRUE);
        }

        vict = ch->specials.fighting;
        if (!vict) {
            Log("!vict in paladin");
            return (FALSE);
        }

        /*
         * well, if we in battle, do some nice things on ourself..  
         */
        if (!affected_by_spell(ch, SKILL_BLESSING)) {
            /* bless myself */
            sprintf(buf, "blessing %s", GET_NAME(ch)); 
            command_interpreter(ch, buf);
            return (TRUE);
        }

        if (!affected_by_spell(ch, SKILL_LAY_ON_HANDS) && 
            GET_HIT(ch) < GET_MAX_HIT(ch) / 2) {
            sprintf(buf, "lay %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return (TRUE);
        }

        switch (number(1, 6)) {
        case 1:
            /* intellegent kick/bash.. hmm */
            if (HasClass(vict, CLASS_SORCERER | CLASS_MAGIC_USER) || 
                HasClass(vict, CLASS_CLERIC | CLASS_PSI)) {
                do_bash(ch, NULL, 0);
            } else {
                do_kick(ch, NULL, 0);
            }
            return (TRUE);
            break;
        case 2:
            do_bash(ch, NULL, 0);
            return (TRUE);
            break;
        case 3:
            do_kick(ch, NULL, 0);
            return (TRUE);
            break;
        case 4:
            do_holy_warcry(ch, NULL, 0);
            return (TRUE);
            break;
        default:
            return (fighter(ch, cmd, arg, mob, type));
            break;
        }
    } else {
        /*
         * check our hps 
         */
        if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && 
            !affected_by_spell(ch, SKILL_LAY_ON_HANDS)) {
            sprintf(buf, "lay %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return (TRUE);
        }

        if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && 
            GET_MANA(ch) > GET_MANA(ch) / 2 && number(1, 6) > 4) {
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        /*
         * lets check some spells on us.. 
         */
        if (IS_AFFECTED(ch, AFF_POISON) && !number(0, 6) &&
            GET_LEVEL(ch, PALADIN_LEVEL_IND) > 10) {
            act("$n asks $s deity to remove poison from $s blood!", 1, ch,
                0, 0, TO_ROOM);
            if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 40) {
                cast_slow_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            } else {
                cast_remove_poison(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
            }
            return (TRUE);
        }

        if (!IS_AFFECTED(ch, AFF_PROTECT_FROM_EVIL) && !number(0, 6)) {
            act("$n prays to $s deity to protect $m from evil.", 1, ch, 0,
                0, TO_ROOM);
            cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (!IS_NPC(tch) && !number(0, 4)) {
                if (IS_SET(ch->specials.act, ACT_GREET) && 
                    GetMaxLevel(tch) > 5 && CAN_SEE(ch, tch)) {
                    if (GET_ALIGNMENT(tch) >= 900) {
                        sprintf(buf, "bow %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        if (tch->player.sex == SEX_FEMALE) {
                            command_interpreter(ch, "say Greetings, noble "
                                                    "lady!");
                        } else {
                            command_interpreter(ch, "say Greetings, noble "
                                                    "sir!");
                        }
                    } else if (GET_ALIGNMENT(tch) >= 350) {
                        sprintf(buf, "smile %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say Greetings, adventurer.");
                    } else if (GET_ALIGNMENT(tch) >= -350) {
                        sprintf(buf, "wink %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say You're doing well on "
                                                "your path of Neutrality");
                    } else if (GET_ALIGNMENT(tch) >= -750) {
                        sprintf(buf, "nod %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say May the prophet smile "
                                                "upon you");
                    } else if (GET_ALIGNMENT(tch) >= -900) {
                        sprintf(buf, "frown %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say You're falling in hands "
                                                "of evil, beware!");
                    } else {
                        /*
                         * hmm, not nice guy.. 
                         */
                        sprintf(buf, "glare %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say I sense great evil here!");
                    }
                    SET_BIT(ch->specials.act, ACT_GREET);
                    break;
                }
            } else if (IsUndead(tch) || IsDiabolic(tch)) {
                command_interpreter(ch, "say Praise the Light, meet thine "
                                        "maker!");
                sprintf(buf, "warcry %s", GET_NAME(tch));
                command_interpreter(ch, buf);
            }
        }
    }
    return (FALSE);
}

/*
 * PSI_CAN is a shorthand for me, use ONLY for PSIs
 */
#define PSI_CAN(skill,level) (spell_index[(skill)] == -1 ? FALSE : \
                     spell_info[spell_index[(skill)]].min_level_psi <= (level))

/*
 * NOTAFF_N_LEARNED more shorthand, this one is general
 */
#define NOTAFF_N_LEARNED(ch,spell,skill) (!affected_by_spell((ch),(spell)) && \
                                          ((ch)->skills[(skill)].learned))

/*
 * SP(el)L_(and target's)N(a)ME :: puts spell name and target's name
 * together
 */
#define SPL_NME(tname,spell) sprintf( buf, "'%s' %s", spells[(spell)], (tname))

/*
 * (Psychic)C(rush)_OR_B(last) if psi can do a crush he does, if not
 * blast!
 */
#define C_OR_B(ch,vict) ((ch)->skills[SKILL_PSYCHIC_CRUSH].learned ? \
        (mind_psychic_crush(PML,(ch),(vict),NULL)) : \
        (do_blast((ch),(vict)->player.name,1)) )

/*
 * CAST_OR_BLAST checks to see if psi can cast special attack spell, if
 * not it will either do a psyhic crush or it will do a psionic blast 
 */
#define CAST_OR_BLAST(ch,vict,spell) \
    SPL_NME((vict)->player.name,(spell-1)); \
    ((ch)->skills[(spell)].learned ? do_cast((ch),buf,370) : \
     C_OR_B((ch),(vict)))

/*
 * Bugs: if 2 mobs of same name in room, may cause problems. fix add macro 
 * to add those "-" between parts and check for number of mobs with same
 * name in room and figure out which one you are!!! * add teleport, prob
 * travel, and disguise 
 */

int Psionist(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    int             PML;        /* psi mob's level */
    int             Qmana;      /* psi mob's 1/4 mana */
    int             cmana;      /* psi mob's current mana level */
    int             hpcan;      /* safe to canibalize/summon */
    char            buf[MAX_STRING_LENGTH];   /* all purpose buffer */
    struct char_data *targ;     /* who the psi_mob is fighting */
    int             group;      /* does targ have followers or is grouped? 
                                 */

    if (cmd) {
        return (FALSE);
    }
    if (!AWAKE(mob) && !affected_by_spell(mob, SKILL_MEDITATE)) {
        /*
         * no sleeping on the job!!!!
         * NOTE: this also prevents mobs who are incap from casting
         */
        command_interpreter(mob, "wake");
        command_interpreter(mob, "stand");
        return (TRUE);
    }

    if (!MobCastCheck(ch, 1)) {
        return (TRUE);
    }

    if (!IS_SET(mob->player.class, CLASS_PSI)) {
        SET_BIT(mob->player.class, CLASS_PSI);
        PML = GET_LEVEL(mob, PSI_LEVEL_IND) = GetMaxLevel(mob);
        SpaceForSkills(mob);

        /*
         * SetSkillLevels first defensive, general spells. 
         * I set all of these so that gods running quests can use the psi
         * mobs skills, unlike most other special procedure mobs
         */
        if (PSI_CAN(SKILL_CANIBALIZE, PML)) {
            mob->skills[SKILL_CANIBALIZE].learned =
                MIN(95, 10 + dice(10, (int) PML / 2));
        }

        if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML)) {
            mob->skills[SKILL_CELL_ADJUSTMENT].learned =
                MIN(95, 10 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_CHAMELEON, PML)) {
            mob->skills[SKILL_CHAMELEON].learned = MIN(95, 50 + number(2, PML));
        }

        if (PSI_CAN(SKILL_FLAME_SHROUD, PML)) {
            mob->skills[SKILL_FLAME_SHROUD].learned =
                MIN(95, 50 + dice(5, PML));
        }

        if (PSI_CAN(SKILL_GREAT_SIGHT, PML)) {
            mob->skills[SKILL_GREAT_SIGHT].learned = MIN(95, 33 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_INVIS, PML)) {
            mob->skills[SKILL_INVIS].learned = MIN(95, 50 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_MEDITATE, PML)) {
            mob->skills[SKILL_MEDITATE].learned =
                MIN(95, 33 + dice((int) PML / 5, 20));
        }

        if (PSI_CAN(SKILL_MINDBLANK, PML)) {
            mob->skills[SKILL_MINDBLANK].learned = MIN(95, 50 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_PORTAL, PML)) {
            mob->skills[SKILL_PORTAL].learned = MIN(95, 40 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_PROBABILITY_TRAVEL, PML)) {
            mob->skills[SKILL_PROBABILITY_TRAVEL].learned =
                MIN(95, 35 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_PSI_SHIELD, PML)) {
            mob->skills[SKILL_PSI_SHIELD].learned =
                MIN(95, 66 + dice(10, (int) PML / 3));
        }

        if (PSI_CAN(SKILL_PSI_STRENGTH, PML)) {
            mob->skills[SKILL_PSI_STRENGTH].learned =
                MIN(95, dice((int) PML / 5, 20));
        }

        if (PSI_CAN(SKILL_PSYCHIC_IMPERSONATION, PML)) {
            mob->skills[SKILL_PSYCHIC_IMPERSONATION].learned =
                MIN(95, 66 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_SUMMON, PML)) {
            mob->skills[SKILL_SUMMON].learned = MIN(95, 66 + PML);
        }

        if (PSI_CAN(SKILL_TOWER_IRON_WILL, PML)) {
            mob->skills[SKILL_TOWER_IRON_WILL].learned =
                MIN(95, 50 + dice(4, PML));
        }

        /*
         * ATTACK -- TYPE SPELLS BEGIN HERE
         */
        if (PSI_CAN(SKILL_MIND_BURN, PML)) {
            mob->skills[SKILL_MIND_BURN].learned = MIN(95, 33 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_DISINTEGRATE, PML)) {
            mob->skills[SKILL_DISINTEGRATE].learned =
                MIN(95, 33 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_MIND_WIPE, PML)) {
            mob->skills[SKILL_MIND_WIPE].learned = MIN(95, 45 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_PSIONIC_BLAST, PML)) {
            mob->skills[SKILL_PSIONIC_BLAST].learned =
                MIN(99, 33 + dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_PSYCHIC_CRUSH, PML)) {
            mob->skills[SKILL_PSYCHIC_CRUSH].learned =
                MIN(99, 66 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_TELEKINESIS, PML)) {
            mob->skills[SKILL_TELEKINESIS].learned =
                MIN(95, dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_PSI_TELEPORT, PML)) {
            mob->skills[SKILL_PSI_TELEPORT].learned =
                MIN(95, dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_ULTRA_BLAST, PML)) {
            mob->skills[SKILL_ULTRA_BLAST].learned =
                MIN(99, 66 + dice(3, PML));
        }

        mob->points.mana = 100;
    }

    /*
     * Aarcerak's little bug fix.. sitting mobs can't cast/mind/etc.. 
     */
    if (mob->specials.fighting && GET_POS(mob) < POSITION_FIGHTING && 
        GET_POS(mob) > POSITION_STUNNED) {
        StandUp(mob);
        return (TRUE);
    }

    if (affected_by_spell(mob, SPELL_FEEBLEMIND)) {
        if (!IS_AFFECTED(mob, AFF_HIDE)) {
            act("$n waits for $s death blow impatiently.", FALSE, mob, 0,
                0, TO_ROOM);
        }
        return (TRUE);
    }

    if (GET_POS(mob) == POSITION_SITTING ||
        GET_POS(mob) == POSITION_RESTING) {
        do_stand(mob, NULL, 0);
        return (TRUE);
    }

    PML = GET_LEVEL(mob, PSI_LEVEL_IND);

    if (NOTAFF_N_LEARNED(mob, SPELL_FIRESHIELD, SKILL_FLAME_SHROUD)) {
        command_interpreter(mob, "flame");
        return (TRUE);
    }

    if (affected_by_spell(mob, SKILL_MEDITATE)) {
        if (mob->points.mana <= .75 * mob->points.max_mana) {
            /* regaining mana */
            return (TRUE);
        }  else {
            command_interpreter(mob, "stand");
            return (TRUE);
        }
    }

    if (IS_AFFECTED(mob, AFF_HIDE)) {
        /* hiding, break fer flame shrd */
        return (FALSE);
    }

    Qmana = 51;
    hpcan = (int) (.75 * mob->points.max_hit);
    cmana = mob->points.mana;

    if (!mob->specials.fighting) {
        if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML) && cmana > Qmana &&
            mob->points.hit < hpcan) {
            command_interpreter(mob, "say That was too close for comfort.");
            mind_teleport(PML, mob, mob, NULL);
            mind_cell_adjustment(PML, mob, mob, NULL);
            return (TRUE);
        }

        if (cmana <= Qmana) {
            if (mob->points.hit > hpcan + 1 && 
                mob->skills[SKILL_CANIBALIZE].learned) {
                if (cmana + 2 * (mob->points.hit - hpcan) >=
                    mob->points.max_mana) {
                    /* Qmana=51>=cm, cm+(2*24) <= 99 */
                    sprintf(buf, "canibalize 24"); 
                } else {
                    sprintf(buf, "canibalize %d", 
                            (mob->points.hit - hpcan - 1));
                }
                command_interpreter(mob, buf);
            } else if (mob->skills[SKILL_MEDITATE].learned) {
                command_interpreter(mob, "meditate");
            }
            return (TRUE);
        }

        if (NOTAFF_N_LEARNED(mob, SKILL_PSI_SHIELD, SKILL_PSI_SHIELD)) {
            command_interpreter(mob, "shield");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_MINDBLANK, SKILL_MINDBLANK)) {
            command_interpreter(mob, "mind 'mindblank'");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_TOWER_IRON_WILL,
                                    SKILL_TOWER_IRON_WILL)) {
            command_interpreter(mob, "mind 'tower of iron will'");
        } else if (NOTAFF_N_LEARNED(mob, SPELL_SENSE_LIFE, SKILL_GREAT_SIGHT)) {
            command_interpreter(mob, "sight");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_PSI_STRENGTH, 
                                    SKILL_PSI_STRENGTH)) {
            command_interpreter(mob, "mind 'psionic strength' self");
        } else if (IS_SET(mob->hatefield, HATE_CHAR) && 
                   mob->points.hit > hpcan) {
            command_interpreter(mob, "say It's payback time!");
            mob->points.mana = 100;
            if (PSI_CAN(SKILL_PORTAL, PML) || PSI_CAN(SKILL_SUMMON, PML)) {
                return (Summoner(mob, 0, NULL, mob, 0));
            }
        } else if (NOTAFF_N_LEARNED(mob, SPELL_INVISIBLE, SKILL_INVIS)) {
            command_interpreter(mob, "psionic");
        } else if (mob->skills[SKILL_CHAMELEON].learned) {
            command_interpreter(mob, "mind 'chameleon'");
        }
        return (TRUE);
    } else {
        /*
         * some psi combat spells still cost mana, set to max mana start of 
         * every round of combat 
         */
        mob->points.mana = 100; 
        targ = mob->specials.fighting;
        if (mob->points.max_hit - hpcan > 1.5 * mob->points.hit) {
            if (!mob->skills[SKILL_PSI_TELEPORT].learned || 
                !IsOnPmp(mob->in_room)) {
                act("$n looks around frantically.", 0, mob, 0, 0, TO_ROOM);
                command_interpreter(mob, "flee");
                return (TRUE);
            }

            act("$n screams defiantly, 'I'll get you yet, $N!'", 0, mob, 0,
                targ, TO_ROOM);
            command_interpreter(mob, "mind 'psionic teleport' self");
            return (TRUE);
        }

        group = (targ->followers || targ->master ? TRUE : FALSE);
        if (group && dice(1, 2) - 1) {
            group = FALSE;
        }

        if (!group) {
            /*
             * not fighting a group, or has selected person fighting, for 
             * spec 
             */
            if (dice(0, 1)) {
                /* do special attack 50% of time */
                if (IS_SET(targ->player.class, CLASS_MAGIC_USER | 
                                               CLASS_CLERIC)) {
                    if (dice(0, 1)) {
                        CAST_OR_BLAST(mob, targ, SKILL_TELEKINESIS);
                    } else {
                        CAST_OR_BLAST(mob, targ, SKILL_MIND_WIPE);
                    }
                } else if (GetMaxLevel(targ) < 20 && dice(0, 1)) {
                    CAST_OR_BLAST(mob, targ, SKILL_PSI_TELEPORT);
                } else {
                    CAST_OR_BLAST(mob, targ, SKILL_DISINTEGRATE);
                }
            } else {
                /* norm attack, psychic crush or psionic blast */
                C_OR_B(mob, targ);
            }
        } else if (mob->skills[SKILL_ULTRA_BLAST].learned) {
            mind_ultra_blast(PML, mob, targ, NULL);
        } else if (mob->skills[SKILL_MIND_BURN].learned) {
            mind_burn(PML, mob, targ, NULL);
        } else {
            sprintf(buf, "blast %s", GET_NAME(targ));
            command_interpreter(mob, buf);
        }
    }
    return (TRUE);
}




int AntiSunItem(struct char_data *ch, int cmd, char *arg,
                struct obj_data *obj, int type)
{
    if (type != PULSE_COMMAND) {
        return (FALSE);
    }
    if (OUTSIDE(ch) && weather_info.sunlight == SUN_LIGHT && 
        weather_info.sky <= SKY_CLOUDY && !IS_AFFECTED(ch, AFF_DARKNESS)) {
        /*
         * frag the item! 
         */
        act("The sun strikes $p, causing it to fall apart!", FALSE, ch,
            obj, 0, TO_CHAR);
        act("The sun strikes $p worn by $n, causing it to fall apart!",
            FALSE, ch, obj, 0, TO_ROOM);
        MakeScrap(ch, 0, obj);
        return (TRUE);
    }
    return (FALSE);
}



int fighter_cleric(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    if (number(1, 100) > 49) {
        return (fighter(ch, cmd, arg, mob, type));
    } else {
        return (cleric(ch, cmd, arg, mob, type));
    }
}

int cleric_mage(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (number(1, 100) > 49) {
        return (cleric(ch, cmd, arg, mob, type));
    } else {
        return (magic_user(ch, cmd, arg, mob, type));
    }
}

int Ranger(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct follow_type *fol;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if (check_soundproof(ch) || check_nomagic(ch, 0, 0)) {
        return (fighter(ch, cmd, arg, mob, type));
    }
    if (!ch->specials.fighting) {
        if (MobCastCheck(ch, 0)) {
            if (GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
                act("$n utters the words 'I feel good!'.", 1, ch, 0, 0,
                    TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
                return (TRUE);
            }

#ifdef PREP_SPELLS
            if (!ch->desc) {
                /* is it a mob? */
                if (!affected_by_spell(ch, SPELL_BARKSKIN)) {
                    act("$n utters the words 'oakey dokey'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_barkskin(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL) && 
                    !IS_EVIL(ch)) {
                    act("$n utters the words 'anti evil'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                              SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 19 &&
                    !affected_by_spell(ch, SPELL_GIANT_GROWTH)) {
                    act("$n utters the words 'The Blessings of Kane'.",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_giant_growth(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 29 &&
                    !affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                    OUTSIDE(ch) && !ch->followers && 
                    !IS_SET(real_roomp((ch)->in_room)->room_flags, TUNNEL)) {
                    /*
                     * let's give ranger some pets 
                     */
                    act("$n whistles loudly.", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_3(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);

                    if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                        ch->followers) {
                        command_interpreter(ch, "order followers guard on");
                        command_interpreter(ch, "group all");
                        act("$n utters the words 'instant growth'.",
                            FALSE, ch, 0, 0, TO_ROOM);

                        for (fol = ch->followers; fol; fol = fol->next) {
                            if (!affected_by_spell(fol->follower, 
                                                   SPELL_ANIMAL_GROWTH)) {
                                cast_animal_growth(GetMaxLevel(ch), ch, "",
                                                   SPELL_TYPE_SPELL,
                                                   fol->follower, 0);
                                WAIT_STATE(ch, PULSE_VIOLENCE);
                            }
                        }
                    }
                    return (TRUE);
                }

                if (affected_by_spell(ch, SPELL_POISON)) {
                    act("$n utters the words 'remove poison'.", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_remove_poison(GetMaxLevel(ch), ch, "",
                                       SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 24 && 
                    affected_by_spell(ch, SPELL_CURSE)) {
                    act("$n utters the words 'neutralize'.", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_remove_curse(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GET_MOVE(ch) < GET_MAX_MOVE(ch) / 2) {
                    act("$n utters the words 'lemon aid'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
                    return (TRUE);
                }
            }
#endif
        }
    } else {
        return (fighter(ch, cmd, arg, mob, type));
    }
    return( TRUE );
}


/*
 *  From: sund_procs.c                          Part of Exile MUD
 *
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.
 *
 */



int marbles(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct obj_data *tobj = NULL;

    if (tobj->in_room == NOWHERE) {
        return FALSE;
    }
    if (cmd >= 1 && cmd <= 6 && GET_POS(ch) == POSITION_STANDING && 
        !IS_NPC(ch)) {
        if (number(1, 100) + GET_DEX(ch) > 50) {
            act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
            GET_POS(ch) = POSITION_SITTING;
            return TRUE;
        } else {
            act("You slip on $p, but manage to retain your balance.",
                FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p, but manages to retain $s balance.", FALSE,
                ch, tobj, 0, TO_ROOM);
        }
    }
    return FALSE;
}





int bahamut_prayer(struct char_data *ch, struct char_data *vict)
{
    struct char_data *i;

    if (IS_PC(ch)) {
        return (0);
    }
    if (GET_HIT(ch) <= 2000) {
        if (ch->mult_att < 4) {
            ch->mult_att = 4;
        }

        for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
            GET_HIT(i) -= 25;
            send_to_char("$c0011A blinding holy light engulfs the room and" 
                         "sears your life away!\n\r", i);
            if (mob_index[i->nr].virtual == BAHAMUT) {
                GET_HIT(i) += 25;
            }
        }
        return (FALSE);
    }

    if (GET_HIT(ch) <= 3000) {
        if (ch->mult_att >= 4) {
            ch->mult_att = 1;
        } else if (ch->mult_att == 1) {
            act("$c0011$n bows down and prays to the dragon lord.", FALSE,
                ch, 0, 0, TO_ROOM);
        } else {
            ch->mult_att = 4;
        }

        return (FALSE);
    }

    return (FALSE);
}

int bahamut_armor(struct char_data *ch, struct char_data *vict)
{
    if (!ch || !vict) {
        return (FALSE);
    }

    switch (number(0, 30)) {
    case 1:
        act("$c0011A blinding beam of light bursts from you and sears your "
            "enemy's life away.", FALSE, ch, 0, 0, TO_CHAR);
        act("$c0011A blinding beam of light bursts from $n.", FALSE, ch, 0,
            0, TO_ROOM);
        GET_HIT(ch) += 15;
        GET_HIT(vict) -= 15;
        break;
    case 3:
    case 4:
    case 5:
        act("$c0011A bright light flickers around you briefly.", FALSE, ch,
            0, 0, TO_CHAR);
        act("$c0011A bright light flickers around $n briefly.", FALSE, ch,
            0, 0, TO_ROOM);
        break;
    default:
        break;
    }
    return (TRUE);
}





/*
 * Toy for Banon.. a new say for him (GH) April 2002 
 */
int godsay(struct char_data *ch, int cmd, char *argument,
           struct obj_data *obj, int type)
{
    char            buf[MAX_INPUT_LENGTH + 80];

    if (cmd != 17) {
        return (FALSE);
    }
    dlog("in godsay");

    if (apply_soundproof(ch)) {
        return ( FALSE );
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
        return( TRUE );
    }

    sprintf(buf, "$c0012-=$c0015$n$c0012=-$c0011 says '$c0014%s$c0011'",
            argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        sprintf(buf, "$c0015You say '$c0014%s$c0015'\n\r", argument);
        send_to_char(buf, ch);
    }
    return( TRUE );
}




int guardian_sin(struct char_data *ch, struct char_data *vict)
{
    struct affected_type af;
    struct obj_data *obj;
    char            buf[240];
    int             gold;

    obj = ch->equipment[WIELD];

    /*
     * I'm thinking 1% chance for each spec to happen Try 0-8 to test
     * specs. may have to tweak the rate 
     */
    switch (number(0, 100)) {
    case 1:
        if (!SET_BIT(ch->specials.affected_by2, AFF2_BERSERK)) {
            /* berserk ch */
            /*
             * cool! let's berserk the wielder of the mace, regardless of
             * class. 
             */
            act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
                FALSE, ch, 0, 0, TO_ROOM);
            act("$c0008Roaring at $n, he invokes the Wrath of Raiva.$c0007", 1,
                vict, 0, 0, TO_ROOM);
            act("$c0008Roaring at you, he invokes the Wrath of Raiva.$c0007", 
                FALSE, vict, 0, 0, TO_VICT);
            SET_BIT(ch->specials.affected_by2, AFF2_BERSERK);
            act("$c1012$n growls at $mself, and whirls into a killing frenzy!",
                FALSE, ch, 0, vict, TO_ROOM);
            act("$c1012The madness overtakes you quickly!", FALSE, ch, 0,
                0, TO_CHAR);

            if (obj->short_description) {
                free(obj->short_description);
                sprintf(buf, "%s", "Guardian of Wrath");
                obj->short_description = strdup(buf);
            }
        }
        break;
    case 2:
#if 0        
        if (IS_NPC(ch))  {
            /* 
             * since slow doesn't really affect mobs, check 
             * here if wielder is PC 
             */
            return(FALSE);
        }
#endif        
        if (IsImmune(ch, IMM_HOLD)) {
            /* 
             * immune PCs shouldn't be affected. 
             */
            return (FALSE);
        }

        if (!affected_by_spell(ch, SPELL_SLOW)) {
            act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
                FALSE, ch, 0, 0, TO_ROOM);
            act("$c0008He giggles at $n while invoking the Leviathon of "
                "Laethargio.$c0007", 1, ch, 0, 0, TO_ROOM);
            act("$c0008He giggles at you while invoking the Leviathon of "
                "Laethargio.$c0007", FALSE, ch, 0, 0, TO_CHAR);

            af.type = SPELL_SLOW;
            af.duration = 2;
            af.modifier = 1;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_SLOW;
            affect_to_char(ch, &af);

            act("$c0008$n seems very slow.$c0007", 1, ch, 0, 0, TO_ROOM);
            send_to_char("$c0008You feel very slow!$c0007\r\n", ch);
            
            if (obj->short_description) {
                free(obj->short_description);
                sprintf(buf, "%s", "Guardian of Sloth");
                obj->short_description = strdup(buf);
            }
        }
        break;
    case 3:
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Pointing, he sends the Essence of Neid towards $n.$c0007",
            1, vict, 0, 0, TO_ROOM);
        act("$c0008Pointing, he sends the Essence of Neid towards you.$c0007",
            FALSE, vict, 0, 0, TO_VICT);
        SET_BIT(vict->specials.affected_by, AFF_BLIND);
        act("$c0008$n's eyes glaze over.$c0007", 1, vict, 0, 0, TO_ROOM);
        send_to_char("$c0008You blink, and the world has turned "
                     "dark.$c0007\r\n", vict);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Envy");
            obj->short_description = strdup(buf);
        }
        break;
    case 4:
        /*
         * This one is funky: dispel magic, dependant on wielder's level.
         * Good stuff, in most cases. However, dispelling fsd mobs when
         * you got your own fs running, this may prove scrappish. Cool! 
         */
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Looking disdainfully at $n, he chants to Lord Orgulho."
            "$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008Looking disdainfully at you, he chants to Lord Orgulho."
            "$c0007", FALSE, vict, 0, 0, TO_VICT);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Pride");
            obj->short_description = strdup(buf);
        }

        break;
    case 5:
        if (IS_PC(vict)) {
            /* 
             * don't steal from other players, not even in arena. 
             */
            return (FALSE);
        }
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Cackling gleefully, he sends Ginayro through $n's "
            "pockets.$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008Your pockets seem a little lighter as Ginayro's Spirit "
            "visits them.$c0007", FALSE, vict, 0, 0, TO_VICT);

        /*
         * steal monies from vict, spoils go to ch 
         */
        gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
        gold = MIN(number(10000, 30000), gold);
        if (gold > 0) {
            GET_GOLD(ch) += gold;
            GET_GOLD(vict) -= gold;
            sprintf(buf, "$c0008Ginayro rewarded you with %d gold coins."
                         "$c0007\r\n", gold);
            send_to_char(buf, ch);
        } else {
            send_to_char("$c0008Ginayro couldn't grab you any gold.$c0007\r\n",
                         ch);
        }

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Greed");
            obj->short_description = strdup(buf);
        }
        break;
    case 6:
        if (affected_by_spell(ch, SKILL_ADRENALIZE)) {
            /* 
             * don't get multiple adrenalizes 
             */
            return (FALSE);
        }
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008He bestows the Spirit of Luhuria upon $n, who gets an "
            "excited look in his eye.$c0007", 1, ch, 0, 0, TO_ROOM);
        act("$c0008He bestows the Spirit of Luhuria upon you, setting your "
            "loins afire.$c0007", FALSE, ch, 0, 0, TO_CHAR);
        /*
         * set adrenalize 
         */
        af.type = SKILL_ADRENALIZE;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.duration = 7;
        af.bitvector = 0;
        affect_to_char(ch, &af);

        af.location = APPLY_DAMROLL;
        af.modifier = 4;
        affect_to_char(ch, &af);

        af.location = APPLY_AC;
        af.modifier = 20;
        affect_to_char(ch, &af);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Lust");
            obj->short_description = strdup(buf);
        }
        break;
    case 7:
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008An enormous maw takes a big bite out of $n, sating Hambre's"
            " appetite.$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008You've just become life bait for Hambre, who sinks his "
            "teeth deep into your flesh.$c0007", FALSE, vict, 0, 0, TO_VICT);
        /*
         * take big bite outta vict: 20% of current hp damage, that's
         * gotta hurt! 
         */
        GET_HIT(vict) -= (GET_HIT(vict) / 5);
        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Gluttony");
            obj->short_description = strdup(buf);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
