#include "config.h"
#include <stdio.h>
#include <string.h>

#include "protos.h"

/*
 * extern variables
 */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;

/*
 * extern functions
 */

struct obj_data *create_money(int amount);
char            getall(char *name, char *newname);
int             getabunch(char *name, char *newname);

/*
 * protos.h?
 */

/*
 * procedures related to get
 */
void get(struct char_data *ch, struct obj_data *obj_object,
         struct obj_data *sub_object)
{
    char            buffer[256];
    char            buf[MAX_INPUT_LENGTH + 80];

    /*
     * check person to item ego
     */
    if (CheckEgo(ch, obj_object) && CheckGetBarbarianOK(ch, obj_object)) {
        if (sub_object) {
            if (!IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
                act("You get $p from $P.", 0, ch, obj_object, sub_object,
                    TO_CHAR);
                act("$n gets $p from $P.", 1, ch, obj_object, sub_object,
                    TO_ROOM);
                obj_from_obj(obj_object);
                obj_to_char(obj_object, ch);
            } else {
                act("$P must be opened first.", 1, ch, 0, sub_object,
                    TO_CHAR);
                return;
            }
        } else {
            if (obj_object->in_room == NOWHERE) {
                obj_object->in_room = ch->in_room;
            }
            act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
            act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
            obj_from_room(obj_object);
            obj_to_char(obj_object, ch);
        }
        if ((obj_object->obj_flags.type_flag == ITEM_MONEY) &&
            (obj_object->obj_flags.value[0] >= 1)) {
            obj_from_char(obj_object);
            /*
             * don't notify if it's 1 coin or less
             */
            if (obj_object->obj_flags.value[0] > 1) {
                sprintf(buffer, "There was %s coins.\n\r",
                        formatNum(obj_object->obj_flags.value[0]));
                send_to_char(buffer, ch);
            }

            /*
             * don't split less than 1 coins
             * and only try to split if you've got groupies
             */
            if (IS_SET(ch->specials.act, PLR_AUTOSPLIT) &&
                obj_object->obj_flags.value[0] > 1  &&
                (ch->followers || ch->master)) {
                sprintf(buffer, "%d", obj_object->obj_flags.value[0]);
                GET_GOLD(ch) += obj_object->obj_flags.value[0];
                do_split(ch, buffer, 0);
            } else {
                GET_GOLD(ch) += obj_object->obj_flags.value[0];
            }
            if (GET_GOLD(ch) > 500000 &&
                obj_object->obj_flags.value[0] > 100000) {
                sprintf(buf, "%s just got %s coins", GET_NAME(ch),
                        formatNum(obj_object->obj_flags.value[0]));
                Log(buf);
            }
            extract_obj(obj_object);
        }
    }
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
    char            arg1[MAX_STRING_LENGTH];
    char            arg2[MAX_STRING_LENGTH];
    char            buffer[MAX_STRING_LENGTH];
    struct obj_data *sub_object,
                   *obj_object,
                   *next_obj,
                   *blah;
    bool            found = FALSE;
    bool            fail = FALSE;
    bool            has = FALSE;
    int             type = 3;
    char            newarg[MAX_STRING_LENGTH];
    int             num,
                    p;

    dlog("in do_get");

    argument_interpreter(argument, arg1, arg2);

    /*
     * get type
     */
    if (!*arg1) {
        type = 0;
    }

    if (*arg1 && !*arg2) {
        if (!str_cmp(arg1, "all")) {
            /*
             * plain "get all"
             */
            type = 1;
        } else {
            /*
             * "get all.item"
             */
            type = 2;
        }
    }

    if (*arg1 && *arg2) {
        if (!str_cmp(arg1, "all")) {
            if (!str_cmp(arg2, "all")) {
                /*
                 * "get all all"
                 */
                type = 3;
            } else {
                /*
                 * get all object
                 */
                type = 4;
            }
        } else {
            if (!str_cmp(arg2, "all")) {
                /*
                 * "get object all"
                 */
                type = 5;
            } else {
                type = 6;
            }
        }
    }

    switch (type) {
    case 0:
        /*
         * get
         */
        send_to_char("Get what?\n\r", ch);
        break;
    case 1:
        /*
         * get all
         */
        sub_object = 0;
        found = FALSE;
        fail = FALSE;
        for (obj_object = real_roomp(ch->in_room)->contents;
             obj_object; obj_object = next_obj) {
            next_obj = obj_object->next_content;
            /*
             * check for a trap (traps fire often)
             */
            if (CheckForAnyTrap(ch, obj_object)) {
                return;
            }
            if (CAN_SEE_OBJ(ch, obj_object)) {
                if (IS_CARRYING_N(ch) + 1 <= CAN_CARRY_N(ch)) {
                    if (IS_CARRYING_W(ch) + obj_object->obj_flags.weight <=
                        CAN_CARRY_W(ch)) {
                        if (CAN_WEAR(obj_object, ITEM_TAKE)) {
                            get(ch, obj_object, sub_object);
                            found = TRUE;
                        } else {
                            send_to_char("You can't take that.\n\r", ch);
                            fail = TRUE;
                        }
                    } else {
                        sprintf(buffer, "%s : You can't carry that much "
                                        "weight.\n\r",
                                obj_object->short_description);
                        send_to_char(buffer, ch);
                        fail = TRUE;
                    }
                } else {
                    sprintf(buffer, "%s : You can't carry that many "
                                    "items.\n\r",
                            obj_object->short_description);
                    send_to_char(buffer, ch);
                    fail = TRUE;
                }
            }
        }

        if (found) {
            send_to_char("OK.\n\r", ch);
        } else if (!fail) {
            send_to_char("You see nothing here.\n\r", ch);
        }
        break;
    case 2:
        /*
         * get ??? (something)
         */
        sub_object = 0;
        found = FALSE;
        fail = FALSE;
        if (getall(arg1, newarg) == TRUE) {
            strcpy(arg1, newarg);
            num = -1;
        } else if ((p = getabunch(arg1, newarg)) != '\0') {
            strcpy(arg1, newarg);
            num = p;
        } else {
            num = 1;
        }

        while (num != 0) {
            obj_object =
                get_obj_in_list_vis(ch, arg1,
                                    real_roomp(ch->in_room)->contents);
            if (obj_object) {
                if (IS_CORPSE(obj_object) && num != 1) {
                    send_to_char("You can only get one corpse at a "
                                 "time.\n\r", ch);
                    fail = TRUE;
                    num = 0;
                    /* no need for num and fail above I guess */
                    return;
                }

                if (CheckForGetTrap(ch, obj_object)) {
                    return;
                }

                if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
                    if (IS_CARRYING_W(ch) + obj_object->obj_flags.weight <
                        CAN_CARRY_W(ch)) {
                        if (CAN_WEAR(obj_object, ITEM_TAKE)) {
                            get(ch, obj_object, sub_object);
                            found = TRUE;
                        } else {
                            send_to_char("You can't take that.\n\r", ch);
                            fail = TRUE;
                            num = 0;
                        }
                    } else {
                        sprintf(buffer, "%s : You can't carry that much "
                                        "weight.\n\r",
                                obj_object->short_description);
                        send_to_char(buffer, ch);
                        fail = TRUE;
                        num = 0;
                    }
                } else {
                    sprintf(buffer, "%s : You can't carry that many "
                                    "items.\n\r",
                            obj_object->short_description);
                    send_to_char(buffer, ch);
                    fail = TRUE;
                    num = 0;
                }
            } else {
                if (num > 0) {
                    sprintf(buffer, "You do not see a %s here.\n\r", arg1);
                    send_to_char(buffer, ch);
                }
                num = 0;
                fail = TRUE;
            }
            if (num > 0) {
                num--;
            }
        }
        break;
    case 3:
        /*
         * get all all
         */
        send_to_char("You must be joking?!\n\r", ch);
        break;
    case 4:
        /*
         * get all ???
         */
        found = FALSE;
        fail = FALSE;
        has = FALSE;
        sub_object = (struct obj_data *)get_obj_vis_accessible(ch, arg2);
        if (sub_object) {
            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
                if ((blah = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
                    has = TRUE;
                }

                for (obj_object = sub_object->contains;
                     obj_object; obj_object = next_obj) {
                    /*
                     * check for trap (jdb - 11/9)
                     */
                    if (CheckForGetTrap(ch, obj_object)) {
                        return;
                    }
                    next_obj = obj_object->next_content;
                    if (CAN_SEE_OBJ(ch, obj_object)) {
                        if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
                            if (has || IS_CARRYING_W(ch) + 
                                       obj_object->obj_flags.weight <
                                       CAN_CARRY_W(ch)) {
                                if (CAN_WEAR(obj_object, ITEM_TAKE)) {
                                    get(ch, obj_object, sub_object);
                                    found = TRUE;
                                } else {
                                    send_to_char("You can't take that\n\r", ch);
                                    fail = TRUE;
                                }
                            } else {
                                sprintf(buffer, "%s : You can't carry "
                                                "that much weight.\n\r",
                                        obj_object->short_description);
                                send_to_char(buffer, ch);
                                fail = TRUE;
                            }
                        } else {
                            sprintf(buffer, "%s : You can't carry that "
                                            "many items.\n\r",
                                    obj_object->short_description);
                            send_to_char(buffer, ch);
                            fail = TRUE;
                        }
                    }
                }

                if (!found && !fail) {
                    sprintf(buffer, "You do not see anything in %s.\n\r",
                            sub_object->short_description);
                    send_to_char(buffer, ch);
                    fail = TRUE;
                }
            } else {
                sprintf(buffer, "%s is not a container.\n\r",
                        sub_object->short_description);
                send_to_char(buffer, ch);
                fail = TRUE;
            }
        } else {
            sprintf(buffer, "You do not see or have the %s.\n\r", arg2);
            send_to_char(buffer, ch);
            fail = TRUE;
        }
        break;
    case 5:
        /*
         * get all all
         */
        send_to_char("You can't take a thing from more than one "
                     "container.\n\r", ch);
        break;
    case 6:
        /*
         * take ??? from ??? (is it??)
         */
        found = FALSE;
        fail = FALSE;
        has = FALSE;
        sub_object = (struct obj_data *)get_obj_vis_accessible(ch, arg2);
        if (sub_object) {
            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
                if ((blah = get_obj_in_list_vis(ch, arg2, ch->carrying)))
                    has = TRUE;
                if (getall(arg1, newarg) == TRUE) {
                    num = -1;
                    strcpy(arg1, newarg);
                } else if ((p = getabunch(arg1, newarg)) != '\0') {
                    num = p;
                    strcpy(arg1, newarg);
                } else {
                    num = 1;
                }

                while (num != 0) {
                    obj_object =
                        get_obj_in_list_vis(ch, arg1, sub_object->contains);
                    if (obj_object) {
                        if (CheckForInsideTrap(ch, sub_object)) {
                            return;
                        }

                        if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
                            if (has || IS_CARRYING_W(ch) + 
                                       obj_object->obj_flags.weight <
                                       CAN_CARRY_W(ch)) {
                                if (CAN_WEAR(obj_object, ITEM_TAKE)) {
                                    get(ch, obj_object, sub_object);
                                    found = TRUE;
                                } else {
                                    send_to_char("You can't take that\n\r", ch);
                                    fail = TRUE;
                                    num = 0;
                                }
                            } else {
                                sprintf(buffer, "%s : You can't carry that"
                                                " much weight.\n\r",
                                        obj_object->short_description);
                                send_to_char(buffer, ch);
                                fail = TRUE;
                                num = 0;
                            }
                        } else {
                            sprintf(buffer, "%s : You can't carry that "
                                            "many items.\n\r",
                                    obj_object->short_description);
                            send_to_char(buffer, ch);
                            fail = TRUE;
                            num = 0;
                        }
                    } else {
                        if (num > 0) {
                            sprintf(buffer, "%s does not contain the %s.\n\r",
                                    sub_object->short_description, arg1);
                            send_to_char(buffer, ch);
                        }
                        num = 0;
                        fail = TRUE;
                    }

                    if (num > 0) {
                        num--;
                    }
                }
            } else {
                sprintf(buffer, "%s is not a container.\n\r",
                        sub_object->short_description);
                send_to_char(buffer, ch);
                fail = TRUE;
            }
        } else {
            sprintf(buffer, "You do not see or have the %s.\n\r", arg2);
            send_to_char(buffer, ch);
            fail = TRUE;
        }
        break;
    }
}

void do_drop(struct char_data *ch, char *argument, int cmd)
{
    char            arg[MAX_INPUT_LENGTH + 80];
    int             amount;
    char            buffer[MAX_STRING_LENGTH];
    struct obj_data *tmp_object;
    struct obj_data *next_obj;
    bool            test = FALSE;
    char            newarg[1000];
    int             num,
                    p;

    dlog("in do_drop");

    argument = one_argument(argument, arg);
    if (is_number(arg)) {
        amount = advatoi(arg);
#if 0
        atoi(arg);
#endif
        argument = one_argument(argument, arg);

        if (str_cmp("coins", arg) != 0 && str_cmp("coin", arg) != 0) {
            send_to_char("Do you mean 'drop <number> coins' ?\n\r", ch);
            return;
        }
        if (amount < 0) {
            send_to_char("Sorry, you can't do that!\n\r", ch);
            return;
        }
        if (GET_GOLD(ch) < amount) {
            send_to_char("You haven't got that many coins!\n\r", ch);
            return;
        }
        sprintf(buffer, "You drop %s coins.\n\r", formatNum(amount));
        send_to_char(buffer, ch);
        if (amount == 0) {
            return;
        }

        act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
        tmp_object = create_money(amount);
        obj_to_room(tmp_object, ch->in_room);
        GET_GOLD(ch) -= amount;
        return;
    }
    /*
     * else { only_argument(argument, arg); }
     */
    if (*arg) {
        if (!str_cmp(arg, "all")) {
            for (tmp_object = ch->carrying;
                 tmp_object; tmp_object = next_obj) {
                next_obj = tmp_object->next_content;
                if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
                    obj_from_char(tmp_object);
                    obj_to_room(tmp_object, ch->in_room);
                    check_falling_obj(tmp_object, ch->in_room);
                    test = TRUE;
                } else if (CAN_SEE_OBJ(ch, tmp_object)) {
                    if (singular(tmp_object)) {
                        sprintf(buffer,
                                "You can't drop %s, it must be CURSED!\n\r",
                                tmp_object->short_description);
                    } else {
                        sprintf(buffer,
                                "You can't drop %s, they must be CURSED!\n\r",
                                tmp_object->short_description);
                    }
                    send_to_char(buffer, ch);
                    test = TRUE;
                }
            }
            if (!test) {
                send_to_char("You do not seem to have anything.\n\r", ch);
            } else {
                act("You drop everything you own.", 1, ch, 0, 0, TO_CHAR);
                act("$n drops everything $e owns.", 1, ch, 0, 0, TO_ROOM);
            }
#if   NODUPLICATES
            do_save(ch, "", 0);
#endif
        } else {
            if (getall(arg, newarg) == TRUE) {
                num = -1;
                strcpy(arg, newarg);
            } else if ((p = getabunch(arg, newarg)) != '\0') {
                num = p;
                strcpy(arg, newarg);
            } else {
                num = 1;
            }

            while (num != 0) {
                tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
                if (tmp_object) {
                    if (!IS_SET(tmp_object->obj_flags.extra_flags,
                                ITEM_NODROP)) {
                        sprintf(buffer, "You drop %s.\n\r",
                                tmp_object->short_description);
                        send_to_char(buffer, ch);
                        act("$n drops $p.", 1, ch, tmp_object, 0, TO_ROOM);
                        obj_from_char(tmp_object);
                        obj_to_room(tmp_object, ch->in_room);

                        check_falling_obj(tmp_object, ch->in_room);
                    } else {
                        if (singular(tmp_object)) {
                            send_to_char("You can't drop it, it must be "
                                         "CURSED!\n\r", ch);
                        } else {
                            send_to_char("You can't drop them, they must be "
                                         "CURSED!\n\r", ch);
                        }

                        num = 0;
                    }
                } else {
                    if (num > 0) {
                        send_to_char("You do not have that item.\n\r", ch);
                    }

                    num = 0;
                }
                if (num > 0) {
                    num--;
                }
            }
#if   NODUPLICATES
            do_save(ch, "", 0);
#endif
        }
    } else {
        send_to_char("Drop what?\n\r", ch);
    }
}

void do_put(struct char_data *ch, char *argument, int cmd)
{
    char            buffer[256];
    char            arg1[128];
    char            arg2[128];
    struct obj_data *obj_object;
    struct obj_data *sub_object;
    struct obj_data *tmp_object;
    struct obj_data *next_object;
    struct char_data *tmp_char;
    int             bits;
    char            newarg[100];
    int             num,
                    p;

    dlog("in do_put");

    argument_interpreter(argument, arg1, arg2);

    if (*arg1) {
        if (*arg2) {
            if (getall(arg1, newarg) == TRUE) {
                num = -1;
                strcpy(arg1, newarg);
            } else if ((p = getabunch(arg1, newarg)) != '\0') {
                num = p;
                strcpy(arg1, newarg);
            } else {
                num = 1;
            }

            if (!strcmp(arg1, "all")) {
                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM, ch,
                                    &tmp_char, &sub_object);

                if (sub_object) {
                    if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
                        send_to_char("But its closed.\n\r", ch);
                        return;
                    }
                    /*
                     * put all bag
                     */
                    for (tmp_object = ch->carrying; tmp_object;
                         tmp_object = next_object) {
                        next_object = tmp_object->next_content;
                        if (IS_SET(tmp_object->obj_flags.extra_flags,
                                   ITEM_NODROP)) {
                            sprintf(buffer, "%s : CURSED!\n\r",
                                    tmp_object->short_description);
                            send_to_char(buffer, ch);
                        } else if (tmp_object != sub_object) {
                            if ((tmp_object->obj_flags.weight +
                                 sub_object->obj_flags.weight) >
                                sub_object->obj_flags.value[0] - 1) {
                                sprintf(buffer, "%s : It won't fit.\n\r",
                                        tmp_object->short_description);
                                send_to_char(buffer, ch);
                                /*
                                 * return;
                                 */
                            } else {
                                sprintf(buffer, "%s : OK\n\r",
                                        tmp_object->short_description);
                                send_to_char(buffer, ch);
                                obj_from_char(tmp_object);
                                if (sub_object->carried_by) {
                                    IS_CARRYING_W(ch) +=
                                        GET_OBJ_WEIGHT(tmp_object);
                                }
                                obj_to_obj(tmp_object, sub_object);
                            }
                        }
                    }
                    act("$n tries to fit as much stuff as $e can into $p.",
                        FALSE, ch, sub_object, 0, TO_ROOM);
                    return;
                } else {
                    sprintf(buffer, "You do not see or have the %s.\n\r.",
                            arg2);
                    send_to_char(buffer, ch);
                }
            } else {
                while (num != 0) {
#if 1
                    bits = generic_find(arg1, FIND_OBJ_INV, ch, &tmp_char,
                                        &obj_object);
#else
                    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
#endif

                    if (obj_object) {
                        if (IS_OBJ_STAT(obj_object, ITEM_NODROP)) {
                            if (singular(obj_object)) {
                                send_to_char("You can't let go of it, it must "
                                             "be CURSED!\n\r", ch);
                            } else {
                                send_to_char("You can't let go of them, they "
                                             "must be CURSED!\n\r", ch);
                            }
                            return;
                        }
                        bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM,
                                            ch, &tmp_char, &sub_object);
                        if (sub_object) {
                            if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
                                if (!IS_SET(sub_object->obj_flags.value[1],
                                            CONT_CLOSED)) {
                                    if (obj_object == sub_object) {
                                        if (singular(obj_object)) {
                                            send_to_char("You attempt to fold "
                                                         "it into itself, but "
                                                         "fail.\n\r", ch);
                                        } else {
                                            send_to_char("You attempt to fold "
                                                         "them inside out, but "
                                                         "fail.\n\r", ch);
                                        }
                                        return;
                                    }
                                    if ((sub_object->obj_flags.weight +
                                         obj_object->obj_flags.weight) <
                                        (sub_object->obj_flags.value[0])) {
                                        act("You put $p in $P", TRUE, ch,
                                            obj_object, sub_object, TO_CHAR);
                                        if (bits == FIND_OBJ_INV) {
                                            obj_from_char(obj_object);
                                            /*
                                             * make up for above line
                                             */
                                            if (sub_object->carried_by) {
                                                IS_CARRYING_W(ch) +=
                                                    GET_OBJ_WEIGHT(obj_object);
                                            }
                                            obj_to_obj(obj_object, sub_object);
                                        }
#if 0
                                        else {
                                            /*
                                             * This isn't used
                                             */
                                             obj_from_room(obj_object);
                                             obj_to_obj(obj_object,
                                             sub_object);

                                        }
#endif
                                        act("$n puts $p in $P", TRUE, ch,
                                            obj_object, sub_object, TO_ROOM);
                                        num--;
                                    } else {
                                        if (singular(sub_object)) {
                                            send_to_char("It won't fit.\n\r",
                                                         ch);
                                        } else {
                                            send_to_char("They won't fit.\n\r",
                                                         ch);
                                        }
                                        num = 0;
                                    }
                                } else {
                                    if (singular(obj_object)) {
                                        send_to_char("It seems to be "
                                                     "closed.\n\r", ch);
                                    } else {
                                        send_to_char("They seem to be "
                                                     "closed.\n\r", ch);
                                    }
                                    num = 0;
                                }
                            } else {
                                sprintf(buffer, "%s is not a container.\n\r",
                                        sub_object->short_description);
                                send_to_char(buffer, ch);
                                num = 0;
                            }
                        } else {
                            sprintf(buffer, "You don't have the %s.\n\r", arg2);
                            send_to_char(buffer, ch);
                            num = 0;
                        }
                    } else {
                        if (num > 0 || num == -1) {
                            sprintf(buffer, "You don't have the %s.\n\r", arg1);
                            send_to_char(buffer, ch);
                        }
                        num = 0;
                    }
                }
#if   NODUPLICATES
                do_save(ch, "", 0);
#endif
            }
        } else {
            sprintf(buffer, "Put %s in what?\n\r", arg1);
            send_to_char(buffer, ch);
        }
    } else {
        send_to_char("Put what in what?\n\r", ch);
    }
}

#if 0
int newstrlen(char *p)
{
    int             i;

    for (i = 0; i < 10 && *p; i++, p++);
    return (i);
}
#endif

void do_give(struct char_data *ch, char *argument, int cmd)
{
    char            obj_name[200],
                    vict_name[80],
                    buf[132];
    char            arg[80],
                    newarg[100];
    int             amount,
                    num,
                    p,
                    count;
    struct char_data *vict = NULL;
    struct obj_data *obj;

    dlog("in do_give");

    argument = one_argument(argument, obj_name);
    obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);
    /*
     * sprintf(buf,"obj_name: %s",obj_name);
     */
    if (!obj && is_number(obj_name)) {
        if (strnlen(obj_name, 10) == 10) {
            obj_name[10] = '\0';
        }
        amount = advatoi(obj_name);
        /*
         * atoi(obj_name);
         */
        argument = one_argument(argument, arg);

        if (str_cmp("coins", arg) != 0 && str_cmp("coin", arg) != 0 &&
            str_cmp("gold", arg) != 0)
        {
            send_to_char("Do you mean, 'give <number> coins <person>' ?\n\r",
                         ch);
            return;
        }

        if (amount < 0) {
            send_to_char("Sorry, you can't do that!\n\r", ch);
            return;
        }
        if (GET_GOLD(ch) < amount &&
            (IS_NPC(ch) || GetMaxLevel(ch) < DEMIGOD)) {
            send_to_char("You haven't got that many coins!\n\r", ch);
            return;
        }

        argument = one_argument(argument, vict_name);

        if (!*vict_name) {
            send_to_char("To who?\n\r", ch);
            return;
        }

        if (!(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return;
        }
        sprintf(buf, "You give %s gold coins to %s.\n\r", formatNum(amount),
                PERS(vict, ch));
        send_to_char(buf, ch);
        sprintf(buf, "%s gives you %s gold coins.\n\r", PERS(ch, vict),
                formatNum(amount));
        send_to_char(buf, vict);
        act("$n gives some gold coins to $N.", 1, ch, 0, vict, TO_NOTVICT);
        if (IS_NPC(ch) || GetMaxLevel(ch) < DEMIGOD) {
            GET_GOLD(ch) -= amount;
        }
        GET_GOLD(vict) += amount;
        save_char(ch, AUTO_RENT);
        if (GET_GOLD(vict) > 500000 && amount > 100000) {
            sprintf(buf, "%s gave %d coins to %s", GET_NAME(ch), amount,
                    GET_NAME(vict));
            Log(buf);
        }
    } else {
        argument = one_argument(argument, vict_name);

        if (!*obj_name || !*vict_name) {
            send_to_char("Give what to who?\n\r", ch);
            return;
        }

        if (getall(obj_name, newarg) == TRUE) {
            num = -1;
            strcpy(obj_name, newarg);
        } else if ((p = getabunch(obj_name, newarg)) != '\0') {
            num = p;
            strcpy(obj_name, newarg);
        } else {
            num = 1;
        }

        count = 0;
        while (num != 0) {
            if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)) &&
                num >= -1) {
                send_to_char("You do not seem to have anything like that.\n\r",
                             ch);
                return;
            }

            if (!(vict = get_char_room_vis(ch, vict_name))) {
                send_to_char("No one by that name around here.\n\r", ch);
                return;
            }

            if (!CheckGiveBarbarianOK(ch, vict, obj)) {
                return;
            }

            if (!CheckEgoGive(ch, vict, obj)) {
                return;
            }
            if (vict == ch) {
                send_to_char("Why are you trying to give something to "
                             "yourself?\n\r", ch);
                return;
            }

            /*
             * I hate it when I want to give stuff to mobiles/players and
             * they cannot carry it -Lennya
             */
            if (!IS_IMMORTAL(ch)) {
                if ((IS_CARRYING_N(vict)) + 1 > CAN_CARRY_N(vict)) {
                    act("$N seems to have $S hands full.", 0, ch, 0, vict,
                        TO_CHAR);
                    return;
                }
                if (obj->obj_flags.weight + IS_CARRYING_W(vict) >
                    CAN_CARRY_W(vict)) {
                    act("$E can't carry that much weight.", 0, ch, 0, vict,
                        TO_CHAR);
                    return;
                }
            }

            if ((IS_OBJ_STAT(obj, ITEM_NODROP)) && (!IS_IMMORTAL(ch))) {
                if (singular(obj)) {
                    sprintf(buf, "You can't let go of %s, it must be CURSED!\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                } else {
                    sprintf(buf, "You can't let go of %s, they must be "
                                 "CURSED!\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                }
                return;
            } else {
                act("$n gives $p to $N.", 1, ch, obj, vict, TO_NOTVICT);
                act("$n gives you $p.", 0, ch, obj, vict, TO_VICT);
                act("You give $p to $N", 0, ch, obj, vict, TO_CHAR);
                obj_from_char(obj);
                obj_to_char(obj, vict);
                if (num > 0)
                    num--;
                count++;
            }

        }
#if   NODUPLICATES
        do_save(ch, "", 0);
        do_save(vict, "", 0);
#endif
    }
}

void do_donate(struct char_data *ch, char *argument, int cmd)
{

    const int       donations1 = 13510;
    const int       donations2 = 99;

    char            arg[MAX_INPUT_LENGTH + 80];
    int             value;
    char            buffer[MAX_STRING_LENGTH];
    struct obj_data *tmp_object;
    struct obj_data *next_obj;
    bool            test = FALSE;
    char            newarg[1000];
    char           *s;
    int             num,
                    p;

    dlog("in do_donate");

    s = one_argument(argument, arg);

    if (*arg) {
        if (!str_cmp(arg, "all")) {
            value = 0;
            for (tmp_object = ch->carrying;
                 tmp_object; tmp_object = next_obj) {
                next_obj = tmp_object->next_content;
                if (!IS_SET(tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
                    obj_from_char(tmp_object);
                    obj_to_room(tmp_object, ((number(0, 1) == 1)) ?
                                donations1 :
                                donations2);
                    value += ((tmp_object->obj_flags.cost) * 10 / 100);
                    test = TRUE;
                } else {
                    if (CAN_SEE_OBJ(ch, tmp_object)) {
                        if (singular(tmp_object)) {
                            sprintf(buffer, "You can't donate %s, it must be "
                                            "CURSED!\n\r",
                                    tmp_object->short_description);
                        } else {
                            sprintf(buffer, "You can't donate %s, they must "
                                            "be CURSED!\n\r",
                                    tmp_object->short_description);
                        }
                        send_to_char(buffer, ch);
                        test = TRUE;
                    }
                }
            }
            if (!test) {
                send_to_char("You do not seem to have anything.\n\r", ch);
            } else {
                act("You donate everything you have! How generous!", 1, ch,
                    0, 0, TO_CHAR);
                act("$n donates everything he carries!.", 1, ch, 0, 0, TO_ROOM);
            }
#if   NODUPLICATES
            do_save(ch, "", 0);
#endif
        } else {
            if (getall(arg, newarg) == TRUE) {
                num = -1;
                strcpy(arg, newarg);
            } else if ((p = getabunch(arg, newarg)) != '\0') {
                num = p;
                strcpy(arg, newarg);
            } else {
                num = 1;
            }
            value = 0;
            while (num != 0) {
                tmp_object = get_obj_in_list_vis(ch, arg, ch->carrying);
                if (tmp_object) {
                    if (!IS_SET
                        (tmp_object->obj_flags.extra_flags, ITEM_NODROP)) {
                        sprintf(buffer, "%s disappears from your hands! A "
                                        "small voice says: 'Thank you'.\n\r",
                                tmp_object->short_description);
                        send_to_char(buffer, ch);
                        sprintf(buffer, "$c0004The gods thank you for your "
                                        "donation!\n\r");
                        send_to_char(buffer, ch);
                        act("$p disappears from $n's hands!.", 1, ch,
                            tmp_object, 0, TO_ROOM);
                        obj_from_char(tmp_object);
                        obj_to_room(tmp_object, ((number(0, 1) == 1)) ?
                                    donations1 :
                                    donations2);
                        value += ((tmp_object->obj_flags.cost) * 10 / 100);
                    } else {
                        if (singular(tmp_object)) {
                            send_to_char("You can't donate it, it must be "
                                         "CURSED!\n\r", ch);
                        } else {
                            send_to_char("You can't donate them, they must be"
                                         " CURSED!\n\r", ch);
                        }
                        num = 0;
                    }
                } else {
                    if (num > 0) {
                        send_to_char("Try donating something you have, "
                                     "silly!\n\r", ch);
                    }

                    num = 0;
                }
                if (num > 0) {
                    num--;
                }
            }
#if   NODUPLICATES
            do_save(ch, "", 0);
#endif
            /*
             * GET_GOLD(ch) += value;
             */
        }
    } else {
        send_to_char("Normally, you gotta donate something!?\n\r", ch);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
