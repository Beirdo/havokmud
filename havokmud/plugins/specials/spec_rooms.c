#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"

/*
 * external vars 
 */

int             monkpreproom = 550;
int             druidpreproom = 500;


/*
 * extern procedures 
 */

#define MONK_CHALLENGE_ROOM 551
#define DRUID_CHALLENGE_ROOM 501

#define Fountain_Level 20

#define NOD  35
#define DRUID_MOB 600
#define MONK_MOB  650
#define FLEE 151

extern char    *dirs[];

#define BAHAMUT_SKIN 45503

extern int      cog_sequence;
int             chest_pointer = 0;


/*
 ********************************************************************
 *  Special procedures for rooms                                    *
 ******************************************************************** */

int bank(struct char_data *ch, int cmd, char *arg, struct room_data *rp,
         int type)
{

    char            buf[256];
    int             money;

    money = advatoi(arg);

    if (IS_NPC(ch)) {
        return (FALSE);
    }
    save_char(ch, ch->in_room);

    if (GET_BANK(ch) > GetMaxLevel(ch) * 40000 && GetMaxLevel(ch) < 40) {
        send_to_char("I'm sorry, but we can no longer hold more than 40000 "
                     "coins per level.\n\r", ch);
        GET_GOLD(ch) += GET_BANK(ch) - GetMaxLevel(ch) * 40000;
        GET_BANK(ch) = GetMaxLevel(ch) * 40000;
    }

    /*
     * deposit
     */
    if (cmd == 219) {
        if (HasClass(ch, CLASS_MONK) && (GetMaxLevel(ch) < 40)) {
            send_to_char("Your vows forbid you to retain personal wealth\n\r",
                         ch);
            return (TRUE);
        }

        if (!arg || money <= 0) {
            send_to_char("Go away, you bother me.\n\r", ch);
            return (TRUE);
        } 

        if (money > GET_GOLD(ch)) {
            send_to_char("You don't have enough for that!\n\r", ch);
            return (TRUE);
        } 

        if (money + GET_BANK(ch) > GetMaxLevel(ch) * 40000 &&
            GetMaxLevel(ch) < 40) {
            send_to_char("I'm sorry, Regulations only allow us to ensure 40000"
                         " coins per level.\n\r", ch);
            return (TRUE);
        }
        
        send_to_char("Thank you.\n\r", ch);
        GET_GOLD(ch) = GET_GOLD(ch) - money;
        GET_BANK(ch) = GET_BANK(ch) + money;
        sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
        send_to_char(buf, ch);
        return (TRUE);
    }

    if (cmd == 220) {
        /*
         * withdraw
         */
        if (HasClass(ch, CLASS_MONK) && GetMaxLevel(ch) < 40) {
            send_to_char("Your vows forbid you to retain personal wealth\n\r",
                         ch);
            return (TRUE);
        }

        if (!arg || money <= 0) {
            send_to_char("Go away, you bother me.\n\r", ch);
            return (TRUE);
        } 

        if (money > GET_BANK(ch)) {
            send_to_char("You don't have enough in the bank for that!\n\r", ch);
            return (TRUE);
        } 

        send_to_char("Thank you.\n\r", ch);
        GET_GOLD(ch) = GET_GOLD(ch) + money;
        GET_BANK(ch) = GET_BANK(ch) - money;
        sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
        send_to_char(buf, ch);
        return (TRUE);
    }

    if (cmd == 221) {
        sprintf(buf, "Your balance is %d.\n\r", GET_BANK(ch));
        send_to_char(buf, ch);
        return (TRUE);
    }
    return (FALSE);
}

/**
 * @bug Should allow an immort to get all
 * donation room
 */
int Donation(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{
    char           *arg1;
    char           *arg2;
    struct obj_data *sub_object;
    
    if ((cmd != 10) && (cmd != 167)) {
        return (FALSE);
    }
    
    arg = get_argument(arg, &arg1);
    arg = get_argument(arg, &arg2);

    if (arg1 && !strncmp(arg1, "all", 3) && !arg2) {
        /* removed check for immortal to test */
        send_to_char("Now now, that would be greedy!\n\r", ch);
        return (TRUE);
    }

    if (arg2 && (sub_object = objectGetInCharOrRoom(ch, arg2))) {
        if (ITEM_TYPE(sub_object) == ITEM_TYPE_CONTAINER) {
            if ((sub_object = objectGetOnChar(ch, arg2, ch))) {
                return (FALSE);
            } else {
                send_to_char("Now now, that would be greedy!\n\r", ch);
                return (TRUE);
            }
        } else {
            send_to_char("That isnt a container.\n\r", ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

int dump(struct char_data *ch, int cmd, char *arg, struct room_data *rp,
         int type)
{
    struct obj_data *k;
    char            buf[100];
    struct char_data *tmp_char;
    int             value = 0;
    struct room_data   *rp;
    LinkedListItem_t   *item,
                       *nextItem;

    rp = roomFindNum(ch->in_room);
    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = nextItem ) {
        nextItem = item->next;
        k = CONTENT_LINK_TO_OBJ(item);

        sprintf(buf, "The %s vanish in a puff of smoke.\n\r",
                fname(k->short_description));
        for (tmp_char = rp->people; tmp_char;
             tmp_char = tmp_char->next_in_room) {
            if (objectIsVisible(tmp_char, k)) {
                send_to_char(buf, tmp_char);
            }
        }
        objectExtract(k);
    }
    LinkedListUnlock( rp->contentList );

    if (cmd != 60) {
        return (FALSE);
    }
    do_drop(ch, arg, cmd);

    value = 0;

    rp = roomFindNum(ch->in_room);
    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = nextItem ) {
        nextItem = item->next;
        k = CONTENT_LINK_TO_OBJ(item);

        sprintf(buf, "The %s vanish in a puff of smoke.\n\r", 
                fname(k->short_description));
        for (tmp_char = rp->people; tmp_char;
             tmp_char = tmp_char->next_in_room) {
            if (objectIsVisible(tmp_char, k)) {
                send_to_char(buf, tmp_char);
            }
        }
        value += (MIN(1000, MAX(k->cost / 4, 1)));
        objectExtract(k);
    }
    LinkedListUnlock( rp->contentList );

    if (value) {
        act("You are awarded for outstanding performance.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n has been awarded for being a good citizen.",
            TRUE, ch, 0, 0, TO_ROOM);

        if (GetMaxLevel(ch) < 3) {
            gain_exp(ch, MIN(100, value));
        } else {
            GET_GOLD(ch) += value;
        }
    }
    return( TRUE );
}

int Fountain(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{

    int             bits,
                    water;
    char            buf[MAX_INPUT_LENGTH];
    char           *arg1;
    struct char_data *tmp_char;
    char            container[20];
    struct obj_data *obj;

    if (cmd == 248) {
        /*
         * fill
         */

        arg = get_argument(arg, &arg1);
        if( !arg1 ) {
            send_to_char( "Fill what?", ch );
            return( FALSE );
        }

        bits = generic_find(arg1, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP,
                            ch, &tmp_char, &obj);

        if (!bits) {
            return (FALSE);
        }

        if (ITEM_TYPE(obj) != ITEM_TYPE_DRINKCON) {
            send_to_char("Thats not a drink container!\n\r", ch);
            return (TRUE);
        }

        if (obj->value[2] != LIQ_WATER && obj->value[1] != 0) {
            name_from_drinkcon(obj);
            obj->value[2] = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);
        } else {
            /*
             * Calculate water it can contain
             */
            water = obj->value[0] - obj->value[1];

            if (water > 0) {
                obj->value[2] = LIQ_WATER;
                obj->value[1] += water;
                weight_change_object(obj, water);
                name_from_drinkcon(obj);
                name_to_drinkcon(obj, LIQ_WATER);
                act("$p is filled.", FALSE, ch, obj, 0, TO_CHAR);
                act("$n fills $p with water.", FALSE, ch, obj, 0, TO_ROOM);
            }
        }
        return (TRUE);
    }

    if (cmd == 11) {
        /*
         * drink
         */
        switch (ch->in_room) {
        case 13518:
        case 11014:
        case 5234:
        case 3141:
        case 13406:
            strncpy(container, "fountain", 20);
            break;
        case 22642:
        case 22644:
        case 22646:
        case 22648:
            strncpy(container, "brook", 20);
            break;
        case 53025:
            strncpy(container, "well", 20);
            break;
        default:
            strncpy(container, "fountain", 20);
        };

        arg1 = skip_spaces(arg);

        if (arg1 && strcasecmp(arg1, container) && strcasecmp(arg1, "water")) {
            return (FALSE);
        }

        sprintf(buf, "You drink from the %s.\n\r", container);
        send_to_char(buf, ch);

        sprintf(buf, "$n drinks from the %s.", container);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);

        if (!IS_IMMORTAL(ch)) {
            GET_COND(ch, THIRST) = 24;
        }
        if (GET_COND(ch, THIRST) > 20) {
            act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);
        }
        return (TRUE);
    }

    /*
     * All commands except fill and drink
     */
    return (FALSE);
}

int pet_shops(struct char_data *ch, int cmd, char *arg,
              struct room_data *rp, int type)
{
    char            buf[MAX_STRING_LENGTH],
                   *arg1,
                   *pet_name;
    int             pet_room;
    struct char_data *pet;

    pet_room = ch->in_room + 1;

    if (cmd == 59) {
        /*
         * List
         */
        send_to_char("Available pets are:\n\r", ch);
        for (pet = roomFindNum(pet_room)->people; pet;
             pet = pet->next_in_room) {
            sprintf(buf, "%8d - %s\n\r", 24 * GET_EXP(pet),
                    pet->player.short_descr);
            send_to_char(buf, ch);
        }
        return (TRUE);
    }

    if (cmd == 56) {
        /*
         * Buy
         */
        arg = get_argument(arg, &arg1);
        pet_name = skip_spaces(arg);

        /*
         * Pet_Name is for later use when I feel like it
         */

        if (!arg1 || !pet_name || !(pet = get_char_room(arg1, pet_room))) {
            send_to_char("There is no such pet!\n\r", ch);
            return (TRUE);
        }

        if (GET_GOLD(ch) < (GET_EXP(pet) * 10)) {
            send_to_char("You don't have enough gold!\n\r", ch);
            return (TRUE);
        }

        GET_GOLD(ch) -= GET_EXP(pet) * 10;

        /** @todo Fix this to use VIRTUAL.  pet->nr is a REAL */
        pet = read_mobile(pet->nr);
        GET_EXP(pet) = 0;
        SET_BIT(pet->specials.affected_by, AFF_CHARM);

        if (pet_name) {
            sprintf(buf, "%s %s", pet->player.name, pet_name);
            free(pet->player.name);
            pet->player.name = strdup(buf);

            sprintf(buf, "%sA small sign on a chain around the neck says 'My "
                         "Name is %s'\n\r", pet->player.description, pet_name);
            free(pet->player.description);
            pet->player.description = strdup(buf);
        }

        char_to_room(pet, ch->in_room);
        add_follower(pet, ch);

        IS_CARRYING_W(pet) = 0;
        IS_CARRYING_N(pet) = 0;

        send_to_char("May you enjoy your pet.\n\r", ch);
        act("$n bought $N as a pet.", FALSE, ch, 0, pet, TO_ROOM);

        return (TRUE);
    }

    /*
     * All commands except list and buy
     */
    return (FALSE);
}

int pray_for_items(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{
    char            buf[256];
    int             key_room,
                    gold;
    bool            found;
    struct obj_data *tmp_obj,
                   *obj;
    int             i;
    char           *descr;
    struct room_data   *rp;
    LinkedListItem_t   *item,
                       *nextItem;

    if (cmd != 176) {
        /*
         * You must pray to get the stuff
         */
        return FALSE;
    }

    key_room = 1 + ch->in_room;

    strcpy(buf, "item_for_");
    strcat(buf, GET_NAME(ch));

    gold = 0;
    found = FALSE;

    rp = roomFindNum(key_room);
    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = nextItem ) {
        nextItem = item->next;
        tmp_obj = CONTENT_LINK_TO_OBJ(item);
        descr = find_ex_description(buf, tmp_obj->ex_description,
                                    tmp_obj->ex_description_count);

        if (descr) {
            if (gold == 0) {
                gold = 1;
                act("$n kneels and at the altar and chants a prayer to "
                    "Odin.", FALSE, ch, 0, 0, TO_ROOM);
                act("You notice a faint light in Odin's eye.",
                    FALSE, ch, 0, 0, TO_CHAR);
            }

            obj = objectRead(tmp_obj->item_number);
            objectPutInRoom(obj, ch->in_room, UNLOCKED);
            act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_ROOM);
            act("$p slowly fades into existence.", FALSE, ch, obj, 0, TO_CHAR);
            gold += obj->cost;
            found = TRUE;
        }
    }
    LinkedListUnlock( rp->contentList );

    if (found) {
        GET_GOLD(ch) -= gold;
        GET_GOLD(ch) = MAX(0, GET_GOLD(ch));
        return TRUE;
    }

    return FALSE;
}

int arena_prep_room(struct char_data *ch, int cmd, char *arg,
                    struct room_data *rp, int type)
{

    char            buf[MAX_STRING_LENGTH + 30];
    extern int      preproomexitsquad1[],
                    preproomexitsquad2[],
                    preproomexitsquad3[],
                    preproomexitsquad4[],
                    Quadrant;

    if (cmd == 1) {
        send_to_char("You enter the arena!\n\r", ch);
        sprintf(buf, "%s leaves the preparation room\n\r", GET_NAME(ch));
        send_to_room_except(buf, ch->in_room, ch);
        char_from_room(ch);

        switch (Quadrant) {
        case 1:
            char_to_room(ch, preproomexitsquad1[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 2:
            char_to_room(ch, preproomexitsquad2[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 3:
            char_to_room(ch, preproomexitsquad3[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 4:
            char_to_room(ch, preproomexitsquad4[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        default:
            Log("Major Screw Up In Arena Prep. Room!!");
            return (TRUE);
        }
        return (TRUE);
    }
    return (FALSE);
}

int arena_arrow_dispel_trap(struct char_data *ch, int cmd, char *arg,
                            struct room_data *rp, int type)
{

    char            buf[MAX_STRING_LENGTH + 30];

    if (cmd >= 1 &&  cmd <= 6) {
        if (dice(1, 100) < 65) {
            send_to_char("A magic arrow emerges from the wall and hits "
                         "you!\n\r", ch);
            sprintf(buf, "A magic arrow emerges from the wall hits %s\n\r",
                    GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            spell_dispel_magic(60, ch, ch, 0);
            do_move(ch, arg, cmd);
            return (TRUE);
        } else {
            send_to_char("A magic arrow emerges from the wall and almost hits"
                         " you!\n\r", ch);
            sprintf(buf, "A magic arrow emerges from the wall nearly hits "
                         "%s\n\r", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            return (FALSE);
        }
    }
    return (FALSE);
}

int arena_fireball_trap(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    char            buf[MAX_STRING_LENGTH + 30];
    int             dam;

    if (cmd >= 1 && cmd <= 6 && dice(1, 100) < 70) {
        dam = dice(30, 6);
        if (saves_spell(ch, SAVING_SPELL)) {
            send_to_char("You barely avoid a fireball!", ch);
            sprintf(buf, "%s barely avoids a huge fireball!", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            return (FALSE);
        } else {
            send_to_char("You are envelopped by burning flames!", ch);
            sprintf(buf, "A fireball strikes %s!", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            MissileDamage(ch, ch, dam, SPELL_FIREBALL);
            return (FALSE);
        }
        return (FALSE);
    }
    return (FALSE);
}

int arena_dispel_trap(struct char_data *ch, int cmd, char *arg,
                      struct room_data *rp, int type)
{
    if (cmd >= 1 && cmd <= 6) {
        spell_dispel_magic(50, ch, ch, 0);
    }
    return (FALSE);
}

int dispel_room(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    if (cmd == 5) {
        /* 
         * Up 
         */
        spell_dispel_magic(50, ch, ch, 0);
    }
    return (FALSE);
}

int fiery_alley(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    if (ch->in_room == 40287 && cmd == 4) {
        /* 
         * West 
         */
        spell_fireball(40, ch, ch, 0);
        return (FALSE);
    }

    if (ch->in_room == 40285 && cmd == 2) {
        /* 
         * East 
         */
        spell_fireball(40, ch, ch, 0);
        return (FALSE);
    }
    return (FALSE);
}

int Magic_Fountain(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{

    char           *buf;

    if (cmd == 11) {
        /* 
         * drink 
         */

        arg = get_argument(arg, &buf);

        if (!buf || (strcasecmp(buf, "fountain") && strcasecmp(buf, "water"))) {
            return (FALSE);
        }

        send_to_char("You drink from the fountain\n\r", ch);
        act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);

        if (GET_COND(ch, THIRST) > 20) {
            act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        if (GET_COND(ch, FULL) > 20) {
            act("You do are full.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) += 1;

        switch (number(0, 40)) {

            /*
             * Lets try and make 1-10 Good, 11-26 Bad, 27-40 Nothing 
             */
        case 1:
            cast_refresh(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 2:
            cast_stone_skin(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
            cast_cure_serious(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 4:
            cast_cure_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 5:
            cast_armor(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
            cast_bless(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 7:
            cast_invisibility(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 8:
            cast_strength(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 9:
            cast_remove_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 10:
            cast_true_seeing(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

            /*
             * Time for the nasty Spells 
             */

        case 11:
            cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 12:
            cast_teleport(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 13:
            cast_web(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 14:
            cast_curse(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 15:
            cast_blindness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 16:
            cast_weakness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
            cast_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 18:
            cast_cause_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 19:
            cast_cause_critic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 20:
            cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 21:
            cast_magic_missile(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 22:
            cast_faerie_fire(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 23:
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 24:
            cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            break;
        case 25:
            cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 26:
            cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

        default:
            send_to_char("The fluid tastes like dry sand in your mouth.\n\r",
                         ch);
            break;
        }
        return (TRUE);
    }

    return (FALSE);
}

int monk_challenge_room(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    struct char_data *i;
    struct room_data *me;
    int             rm;
    LinkedListItem_t   *item,
                       *nextItem;

    rm = ch->in_room;

    me = roomFindNum(ch->in_room);
    if ((!me) || (!me->river_speed)) {
        return (FALSE);
    }
    if (IS_PC(ch)) {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    }

    if (cmd == FLEE) {
        /*
         * this person just lost 
         */
        send_to_char("You lose.\n\r", ch);
        if (IS_PC(ch)) {
            if (IS_NPC(ch)) {
                command_interpreter(ch, "return");
            }
            GET_EXP(ch) = 
                MIN(classes[MONK_LEVEL_IND].
                       levels[(int)GET_LEVEL(ch, MONK_LEVEL_IND)].exp,
                    GET_EXP(ch));
            send_to_char("Go home.\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, rm - 1);
            me->river_speed = 0;
            return (TRUE);
        } else if (ch->nr >= MONK_MOB && ch->nr <= MONK_MOB + 40) {
            extract_char(ch);
            /*
             * find pc in room; 
             */
            for (i = me->people; i; i = i->next_in_room) {
                if (IS_PC(i)) {
                    if (IS_NPC(i)) {
                        command_interpreter(i, "return");
                    }
                    GET_EXP(i) = 
                        MAX(classes[MONK_LEVEL_IND].
                               levels[GET_LEVEL(i, MONK_LEVEL_IND) + 
                                      1].exp + 1, GET_EXP(i));
                    GainLevel(i, MONK_LEVEL_IND);
                    char_from_room(i);
                    char_to_room(i, monkpreproom);

                    while (me->people) {
                        extract_char(me->people);
                    }

                    LinkedListLock( me->contentList );
                    for( item = me->contentList->head; item; item = nextItem ) {
                        nextItem = item->next;
                        objectExtract(CONTENT_LINK_TO_OBJ(item));
                    }
                    LinkedListUnlock( me->contentList );

                    me->river_speed = 0;
                    return (TRUE);
                }
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

int monk_challenge_prep_room(struct char_data *ch, int cmd, char *arg,
                             struct room_data *rp, int type)
{
    struct room_data *me,
                   *chal;
    int             i,
                    newr;
    struct obj_data *o,
                   *tmp_obj,
                   *next_obj;
    struct char_data *mob;

    me = roomFindNum(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    chal = roomFindNum(MONK_CHALLENGE_ROOM);
    if (!chal) {
        send_to_char("The challenge room is gone.. please contact a god\n\r",
                     ch);
        return (TRUE);
    }

    if (cmd == NOD) {
        if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("You're no monk\n\r", ch);
            return (FALSE);
        }

        if (GET_LEVEL(ch, MONK_LEVEL_IND) < 10) {
            send_to_char("You have no business here, kid.\n\r", ch);
            return (FALSE);
        }

        if (GET_EXP(ch) <= 
            classes[MONK_LEVEL_IND].
                levels[GET_LEVEL(ch, MONK_LEVEL_IND) + 1].exp - 100) {
            send_to_char("You cannot advance now\n\r", ch);
            return (TRUE);
        }

        if (chal->river_speed != 0) {
            send_to_char("The challenge room is busy.. please wait\n\r", ch);
            return (TRUE);
        }

        for (i = 0; i < MAX_WEAR; i++) {
            if (ch->equipment[i]) {
                o = unequip_char(ch, i);
                objectGiveToChar(o, ch);
            }
        }

        for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = next_obj) {
            next_obj = tmp_obj->next_content;
            objectTakeFromChar(tmp_obj);
            objectPutInRoom(tmp_obj, ch->in_room, UNLOCKED);
        }

        monkpreproom = ch->in_room;

        send_to_char("You are taken into the combat room.\n\r", ch);
        act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
        newr = MONK_CHALLENGE_ROOM;
        char_from_room(ch);
        char_to_room(ch, newr);

        /*
         * load the mob at the same lev as char 
         */
        mob = read_mobile(MONK_MOB + GET_LEVEL(ch, MONK_LEVEL_IND) - 10);

        if (!mob) {
            send_to_char("The fight is called off.. Go home.\n\r", ch);
            return (TRUE);
        }
        char_to_room(mob, ch->in_room);
        chal->river_speed = 1;
        do_look(ch, NULL, 0);
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        return (TRUE);
    }

    return (FALSE);
}

int druid_challenge_prep_room(struct char_data *ch, int cmd, char *arg,
                              struct room_data *rp, int type)
{
    struct room_data *me,
                   *chal;
    int             i,
                    newr;
    struct obj_data *o,
                   *tmp_obj,
                   *next_obj;
    struct char_data *mob;

    me = roomFindNum(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    chal = roomFindNum(DRUID_CHALLENGE_ROOM);
    if (!chal) {
        send_to_char("The challenge room is gone.. please contact a god\n\r",
                     ch);
        return (TRUE);
    }

    if (cmd == NOD) {
        if (!HasClass(ch, CLASS_DRUID)) {
            send_to_char("You're no druid.\n\r", ch);
            return (FALSE);
        }

        if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 10) {
            send_to_char("You have no business here, kid.\n\r", ch);
            return (FALSE);
        }

        if (GET_EXP(ch) <= 
            classes[DRUID_LEVEL_IND].
                levels[GET_LEVEL(ch, DRUID_LEVEL_IND) + 1].exp - 100) {
            send_to_char("You cannot advance now.\n\r", ch);
            return (TRUE);
        } else if (GET_LEVEL(ch, DRUID_LEVEL_IND) == 50) {
            send_to_char("You are far too powerful to be trained here... Seek "
                         "an implementor to help you.", ch);
            return (FALSE);
        }

        if (chal->river_speed != 0) {
            send_to_char("The challenge room is busy.. please wait.\n\r", ch);
            return (TRUE);
        }

        for (i = 0; i < MAX_WEAR; i++) {
            if (ch->equipment[i]) {
                o = unequip_char(ch, i);
                objectGiveToChar(o, ch);
            }
        }

        for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = next_obj) {
            next_obj = tmp_obj->next_content;
            objectTakeFromChar(tmp_obj);
            objectPutInRoom(tmp_obj, ch->in_room, UNLOCKED);
        }

        druidpreproom = ch->in_room;

        send_to_char("You are taken into the combat room.\n\r", ch);
        act("$n is ushered into the combat room.", FALSE, ch, 0, 0, TO_ROOM);
        newr = DRUID_CHALLENGE_ROOM;
        char_from_room(ch);
        char_to_room(ch, newr);

        /*
         * load the mob at the same lev as char 
         */
        mob = read_mobile(DRUID_MOB + GET_LEVEL(ch, DRUID_LEVEL_IND) - 10);

        if (!mob) {
            send_to_char("The fight is called off. Go home.\n\r", ch);
            return (TRUE);
        }
        char_to_room(mob, ch->in_room);
        chal->river_speed = 1;
        do_look(ch, NULL, 0);
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        return (TRUE);
    }

    return (FALSE);
}

int druid_challenge_room(struct char_data *ch, int cmd, char *arg,
                         struct room_data *rp, int type)
{
    struct char_data *i;
    struct room_data *me;
    int             rm;
    LinkedListItem_t   *item,
                       *nextItem;

    me = roomFindNum(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    rm = ch->in_room;

    if (!me->river_speed) {
        return (FALSE);
    }
    if (IS_PC(ch)) {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    }

    if (cmd == FLEE) {
        /*
         * this person just lost 
         */
        send_to_char("You lose\n\r", ch);
        if (IS_PC(ch)) {
            if (IS_NPC(ch)) {
                command_interpreter(ch, "return");
            }
            GET_EXP(ch) = MIN(classes[DRUID_LEVEL_IND].
                               levels[(int)GET_LEVEL(ch, DRUID_LEVEL_IND)].exp,
                              GET_EXP(ch));
            send_to_char("Go home\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, rm - 1);
            me->river_speed = 0;
            while (me->people) {
                extract_char(me->people);
            }
            return (TRUE);
        } 
        
        if (ch->nr >= DRUID_MOB && ch->nr <= DRUID_MOB + 40) {
            extract_char(ch);
            /*
             * find pc in room; 
             */
            for (i = me->people; i; i = i->next_in_room) {
                if (IS_PC(i)) {
                    if (IS_NPC(i)) {
                        command_interpreter(i, "return");
                    }
                    GET_EXP(i) = MAX(classes[DRUID_LEVEL_IND].
                                      levels[GET_LEVEL(i, DRUID_LEVEL_IND) + 
                                             1].exp + 1, GET_EXP(i));

                    GainLevel(i, DRUID_LEVEL_IND);
                    char_from_room(i);
                    char_to_room(i, druidpreproom);

                    if (affected_by_spell(i, SPELL_POISON)) {
                        affect_from_char(ch, SPELL_POISON);
                    }

                    if (affected_by_spell(i, SPELL_HEAT_STUFF)) {
                        affect_from_char(ch, SPELL_HEAT_STUFF);
                    }

                    while (me->people) {
                        extract_char(me->people);
                    }

                    LinkedListLock( me->contentList );
                    for( item = me->contentList->head; item; item = nextItem ) {
                        nextItem = item->next;
                        objectExtract(CONTENT_LINK_TO_OBJ(item));
                    }
                    LinkedListUnlock( me->contentList );

                    me->river_speed = 0;

                    return (TRUE);
                }
            }
            return (TRUE);
        } 
        
        return (FALSE);
    }

    return (FALSE);
}

int bahamut_home(struct char_data *ch, int cmd, char *arg,
                 struct room_data *rp, int type)
{
    int             r_num = 0,
                    percent = 0;
    struct obj_data *object,
                   *bahamut;
    static const char  *corpse = "corpse corpse of Bahamut, The Ancient "
                                 "Platinum Dragon is");
    char           *itemname,
                   *itemtype,
                    buf[256];

    if (cmd == 330) {
        if (MOUNTED(ch)) {
            send_to_char("Not from this mount you cannot!\n\r", ch);
            return (TRUE);
        }

        if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
            !HasClass(ch, CLASS_BARBARIAN | CLASS_WARRIOR | CLASS_RANGER)) {
            send_to_char("What do you think you are, A tanner?\n\r", ch);
            return (TRUE);
        }

        bahamut = objectGetInCharOrRoom(ch, corpse);
        if( !bahamut ) {
            bahamut = objectGetGlobal( ch, corpse, NULL );
        }

        arg = get_argument(arg, &itemname);
        arg = get_argument(arg, &itemtype);

        if (!itemname) {
            send_to_char("Tan what?\n\r", ch);
            return (TRUE);
        }

        if (!itemtype) {
            send_to_char("I see that, but what do you wanna make?\n\r", ch);
            return (TRUE);
        }

        if (!(object = objectGetInRoom(ch, itemname, 
                                       roomFindNum(ch->in_room)))) {
            send_to_char("Where did that carcass go?\n\r", ch);
            return (TRUE);
        }

        if (bahamut != object) {
            return (FALSE);
        }
        
        if (strcmp(itemtype, "jacket")) {
            return (FALSE);
        }

        if (object->affected[0].modifier != 0 && 
            object->affected[1].modifier != 0) {

            /* 101% is a complete failure */
            percent = number(1, 101);

            if (ch->skills && ch->skills[SKILL_TAN].learned && 
                GET_POS(ch) > POSITION_SLEEPING) {
                if (percent > ch->skills[SKILL_TAN].learned) {
                    /* make corpse unusable for another tan */
                    object->affected[1].modifier = 0;
                    sprintf(buf, "You hack at %s but manage to only destroy "
                            "the hide.\n\r", object->short_description);
                    send_to_char(buf, ch);

                    sprintf(buf, "%s tries to skin %s for it's hide, but "
                                 "destroys it.", GET_NAME(ch), 
                                 object->short_description);
                    act(buf, TRUE, ch, 0, 0, TO_ROOM);
                    LearnFromMistake(ch, SKILL_TAN, 0, 95);
                    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
                    return (TRUE);
                }
                
                /* make corpse unusable for another tan */
                object->affected[1].modifier = 0;
                sprintf(buf, "It seems you are able to make some very "
                             "stylish body armor.");
                send_to_char(buf, ch);
                sprintf(buf, "%s fashions some very fine body armor out of"
                             " the %s.", GET_NAME(ch), 
                             object->short_description);
                act(buf, TRUE, ch, 0, 0, TO_ROOM);
                object = objectRead(BAHAMUT_SKIN);
                objectGiveToChar(object, ch);
                return (TRUE);
            } else {
                return (FALSE);
            }
        } else {
            send_to_char("Sorry, nothing left of the carcass to make an item "
                         "with.\n\r", ch);
            return (TRUE);
        }
    }

    return (FALSE);
}

int Magic_Pool(struct char_data *ch, int cmd, char *arg,
               struct room_data *rp, int type)
{
    char           *buf;

    if (cmd == 11) {
        /* 
         * 11 = drink 
         */
        arg = get_argument(arg, &buf);

        if (!buf || strcmp(buf, "pool")) {
            return (FALSE);
        }

        if (GET_COND(ch, THIRST) > 21 || GET_COND(ch, FULL) > 21) {
            act("Ohhh.. Your stomach can't handle anymore.", FALSE, ch, 0,
                0, TO_CHAR);
            return (TRUE);
        }
        send_to_char("You drink from the pool\n\r", ch);
        act("$n drinks from the pool", FALSE, ch, 0, 0, TO_ROOM);

        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) += 1;

        act("You suddenly feel much better.", FALSE, ch, 0, 0, TO_CHAR);
        cast_heal(50, ch, "", SPELL_TYPE_SCROLL, ch, 0);
        return (TRUE);
    }
    return (FALSE);
}

/*
 * Morrigans proc.. read book.. transfers to room+1 kind of cludgy and
 * ugly.. i'm lazy @author : Banon @date : May 17, 2002 
 */
int Read_Room(struct char_data *ch, int cmd, char *arg,
              struct room_data *rp, int type)
{
    int             key_room;
    char           *buf;
    struct obj_data *obj;

    if (cmd != 63) {
        /* 
         * 63 = read 
         */
        return FALSE;
    }

    if (!HasClass(ch, CLASS_PSI)) {
        return FALSE;
    }

    buf = get_argument(arg, &buf);
    if (!buf || strcmp(buf, "book")) {
        return (FALSE);
    }
    
    /*
     * get obj in list vis 
     */
    obj = objectGetOnChar(ch, "book", ch);
    if (!obj) {
        return (FALSE);
    }
    if (obj->item_number == ch->in_room) {
        key_room = 1 + ch->in_room;
        act("$n reads the book then disappears.", FALSE, ch, 0, 0, TO_ROOM);
        act("You read the book and feel your body being torn to another "
            "location", FALSE, ch, 0, 0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, key_room);

        act("$n suddenly appears.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return TRUE;
    }

    return FALSE;
}

/*
 * church bell for ators zone in town 
 */
#define PULL    224
int ChurchBell(struct char_data *ch, int cmd, char *arg,
               struct room_data *rp, int type)
{
    char           *buf;

    if (cmd == PULL) {
        arg = get_argument(arg, &buf);   /* buf == object */

        if (buf && !strcmp("rope", buf)) {
            SendToAll("The bells of Karsynia's church sound 'GONG! GONG! "
                      "GONG! GONG!'\n");
            return (TRUE);
        }
    }
    return (FALSE);
}

int pride_disabler(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{
    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 91 || cmd == 150 || 
        cmd == 258 || cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold follow grab doorbash run flee draw 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_one(struct char_data *ch, int cmd, char *arg,
                      struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee draw 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 3) {
        /* south */
        /*
         * remove weapon 
         */
        if (ch->equipment[WIELD]) {
            obj = ch->equipment[WIELD];
            if ((obj = unequip_char(ch, WIELD)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }
        /*
         * remove held 
         */
        if (ch->equipment[HOLD]) {
            obj = ch->equipment[HOLD];
            if ((obj = unequip_char(ch, HOLD)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }
        /*
         * remove light 
         */
        if (ch->equipment[WEAR_LIGHT]) {
            obj = ch->equipment[WEAR_LIGHT];
            if ((obj = unequip_char(ch, WEAR_LIGHT)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }
        /*
         * remove about 
         */
        if (ch->equipment[WEAR_ABOUT]) {
            obj = ch->equipment[WEAR_ABOUT];
            if ((obj = unequip_char(ch, WEAR_ABOUT)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }
        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides south, proceeding on the Path of Pride.", FALSE,
            ch, 0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51935);        
        /* 
         * move char to room two 
         */

        act("$n strides in from the north.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_two(struct char_data *ch, int cmd, char *arg,
                      struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 4) {
        /* 
         * west
         *
         * remove headwear 
         */
        if (ch->equipment[WEAR_HEAD]) {
            obj = ch->equipment[WEAR_HEAD];
            if ((obj = unequip_char(ch, WEAR_HEAD)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove shield 
         */
        if (ch->equipment[WEAR_SHIELD]) {
            obj = ch->equipment[WEAR_SHIELD];
            if ((obj = unequip_char(ch, WEAR_SHIELD)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides west, proceeding on the Path of Pride.", FALSE, ch,
            0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51934);        
        /* 
         * move char to room three 
         */

        act("$n strides in from the east.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_three(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 3) {
        /* 
         * south 
         *
         * remove bag 
         */
        if (ch->equipment[WEAR_BACK]) {
            obj = ch->equipment[WEAR_BACK];
            if ((obj = unequip_char(ch, WEAR_BACK)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove hands 
         */
        if (ch->equipment[WEAR_HANDS]) {
            obj = ch->equipment[WEAR_HANDS];
            if ((obj = unequip_char(ch, WEAR_HANDS)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides south, proceeding on the Path of Pride.", FALSE,
            ch, 0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51933);        
        /* 
         * move char to room four 
         */

        act("$n strides in from the north.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_four(struct char_data *ch, int cmd, char *arg,
                       struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 6) {
        /* 
         * down
         *
         * remove eyewear 
         */
        if (ch->equipment[WEAR_EYES]) {
            obj = ch->equipment[WEAR_EYES];
            if ((obj = unequip_char(ch, WEAR_EYES)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove armwear 
         */
        if (ch->equipment[WEAR_ARMS]) {
            obj = ch->equipment[WEAR_ARMS];
            if ((obj = unequip_char(ch, WEAR_ARMS)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides down, proceeding on the Path of Pride.", FALSE, ch,
            0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51932);        
        /* 
         * move char to room five 
         */

        act("$n strides in from above.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_five(struct char_data *ch, int cmd, char *arg,
                       struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 1) {
        /* 
         * north
         *
         * remove rings 
         */
        if (ch->equipment[WEAR_FINGER_R]) {
            obj = ch->equipment[WEAR_FINGER_R];
            if ((obj = unequip_char(ch, WEAR_FINGER_R)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        if (ch->equipment[WEAR_FINGER_L]) {
            obj = ch->equipment[WEAR_FINGER_L];
            if ((obj = unequip_char(ch, WEAR_FINGER_L)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove necklaces 
         */
        if (ch->equipment[WEAR_NECK_1]) {
            obj = ch->equipment[WEAR_NECK_1];
            if ((obj = unequip_char(ch, WEAR_NECK_1)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        if (ch->equipment[WEAR_NECK_2]) {
            obj = ch->equipment[WEAR_NECK_2];
            if ((obj = unequip_char(ch, WEAR_NECK_2)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove bracelets 
         */
        if (ch->equipment[WEAR_WRIST_R]) {
            obj = ch->equipment[WEAR_WRIST_R];
            if ((obj = unequip_char(ch, WEAR_WRIST_R)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        if (ch->equipment[WEAR_WRIST_L]) {
            obj = ch->equipment[WEAR_WRIST_L];
            if ((obj = unequip_char(ch, WEAR_WRIST_L)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove earrings 
         */
        if (ch->equipment[WEAR_EAR_R]) {
            obj = ch->equipment[WEAR_EAR_R];
            if ((obj = unequip_char(ch, WEAR_EAR_R)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        if (ch->equipment[WEAR_EAR_L]) {
            obj = ch->equipment[WEAR_EAR_L];
            if ((obj = unequip_char(ch, WEAR_EAR_L)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides north, proceeding on the Path of Pride.", FALSE,
            ch, 0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51931);        
        /* 
         * move char to room six 
         */

        act("$n strides in from the south.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_six(struct char_data *ch, int cmd, char *arg,
                      struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 || 
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 2) {
        /* 
         * east
         *
         * remove bodywear 
         */
        if (ch->equipment[WEAR_BODY]) {
            obj = ch->equipment[WEAR_BODY];
            if ((obj = unequip_char(ch, WEAR_BODY)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove footwear 
         */
        if (ch->equipment[WEAR_FEET]) {
            obj = ch->equipment[WEAR_FEET];
            if ((obj = unequip_char(ch, WEAR_FEET)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides east, proceeding on the Path of Pride.", FALSE, ch,
            0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51930);        
        /* 
         * move char to room seven 
         */

        act("$n strides in from the west.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

int pride_remover_seven(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    struct obj_data *obj;

    if (cmd == 13 || cmd == 14 || cmd == 65 || cmd == 150 || cmd == 258 ||
        cmd == 384 || cmd == 151 || cmd == 567) {
        /*
         * wear wield hold grab doorbash run flee 
         */
        act("$n shines with pride.", FALSE, ch, 0, 0, TO_ROOM);
        act("You feel far too proud to do that.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 1) {
        /* 
         * north
         *
         * remove legwear 
         */
        if (ch->equipment[WEAR_LEGS]) {
            obj = ch->equipment[WEAR_LEGS];
            if ((obj = unequip_char(ch, WEAR_LEGS)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        /*
         * remove waistwear 
         */
        if (ch->equipment[WEAR_WAIST]) {
            obj = ch->equipment[WEAR_WAIST];
            if ((obj = unequip_char(ch, WEAR_WAIST)) != NULL) {
                objectGiveToChar(obj, ch);
            }
        }

        act("Smirking, $n stops using some equipment.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n strides north, proceeding on the Path of Pride.", FALSE,
            ch, 0, 0, TO_ROOM);
        act("Feeling you don't need it, you decide to remove some stuff.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You then proceed along the Path of Pride.\n\r", FALSE, ch, 0,
            0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, 51836);        /* move char to The Naked Truth */

        act("$n strides in from the south.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

#define MAGE_CAVE 45461
int rope_room(struct char_data *ch, int cmd, char *arg,
              struct room_data *rp, int type)
{
    char           *buf;

    if (cmd == 5) {
        /* 
         * up 
         */
        act("A magical force blocks your ascent.", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 15) {
        /* 
         * look 
         */
        arg = get_argument(arg, &buf);
        if (buf && (!strcasecmp("up", buf) || !strcasecmp("u", buf) || 
                    !strcasecmp("Up", buf))) {
            send_to_char("All you can see up there is a cliff wall.\n\r", ch);
            return (TRUE);
        }
    }

    if (cmd == 224) {
        /* 
         * pull 
         */
        buf = get_argument(arg, &buf);
        if (buf && (!strcasecmp("rope", buf) || !strcasecmp("Rope", buf))) {
            act("You grab the rope and give it a tug, but can't",
                FALSE, ch, 0, 0, TO_CHAR);
            send_to_char("let go.  The rope jerks back and pulls you up\n\r",
                         ch);
            send_to_char("with it.\n\r", ch);

            act("$n pulls on a rope and is suddenly pulled up into the sky!",
                FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, MAGE_CAVE);

            act("$n suddenly appears from below.", FALSE, ch, 0, 0,
                TO_ROOM);
            do_look(ch, NULL, 0);
            return (TRUE);
        }
    }
    return (FALSE);
}

#define DRINK   11
int sinbarrel(struct char_data *ch, int cmd, char *arg,
              struct room_data *rp, int type)
{
    char           *buf;
    
    if (cmd == DRINK) {
        arg = get_argument(arg, &buf);

        if (buf && !strcmp("mead", buf)) {
            send_to_char("\n\r", ch);
            send_to_char("Out of options, you finally decide to succumb to "
                         "the tantalizing toxins of the mead \n\rand take a "
                         "long draught. You must have quite a tummy!\n\r", ch);
            send_to_char("\n\r", ch);

            act("$n meditates a moment, takes a long draught of the mead and "
                "disappears!", FALSE, ch, 0, 0, TO_ROOM);
            GET_COND(ch, DRUNK) = 25;
            GET_COND(ch, FULL) = 25;
            GET_COND(ch, THIRST) = 20;
            char_from_room(ch);
            char_to_room(ch, 51828);

            act("$n climbs out of the barrel, swaying slightly.", FALSE,
                ch, 0, 0, TO_ROOM);
            do_look(ch, NULL, 0);

            send_to_char("\n\r", ch);
            send_to_char("You feel a bit dizzy from so much mead.\n\r", ch);
            return (TRUE);
        }
    }
    return (FALSE);
}


int Thunder_Fountain(struct char_data *ch, int cmd, char *arg,
                     struct room_data *rp, int type)
{
    char           *buf;

    if (cmd == 11) {
        /* 
         * drink 
         */

        arg = get_argument(arg, &buf);

        send_to_char("You drink from the fountain\n\r", ch);
        act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);

        if (GET_COND(ch, THIRST) > 20) {
            act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        if (GET_COND(ch, FULL) > 20) {
            act("You are full.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) += 1;

        switch (number(0, 5)) {
        case 1:
            send_to_char("You see something glistening in the water.\n\r", ch);
            send_to_char ("As you reach to touch it, some magical force pulls "
                          "you under.\n\r", ch);
            act("$n is suddenly sucked into the fountain!", FALSE, ch, 0,
                0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, 46201);
            do_look(ch, NULL, 0);
            break;
        default:
            send_to_char("The water tastes so good, you are tempted to try "
                         "some more.\n\r", ch);
            break;
        }
        return (TRUE);
    }

    /*
     * All commands except fill and drink 
     */
    return (FALSE);
}


#define CLIMB_ROOM 52856
int climb_room(struct char_data *ch, int cmd, char *arg,
               struct room_data *rp, int type)
{
    char           *buf,
                    buffer[254];
    struct obj_data *obj;

    if (cmd == 5) {
        /* 
         * up 
         */
        act("How did ya want to get up there?", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }

    if (cmd == 15) {
        /* 
         * look 
         */
        arg = get_argument(arg, &buf);
        if (buf && (!strcasecmp("up", buf) || !strcasecmp("u", buf) || 
                    !strcasecmp("Up", buf))) {
            send_to_char("One would have a marvelous view when high up in the"
                         " canopy.\n\r", ch);
            return (TRUE);
        }
    }

    if (cmd == 287) {
        /* 
         * climb 
         */
        arg = get_argument(arg, &buf);
        if (buf) {
            if (!strcasecmp("tree", buf) || !strcasecmp("Tree", buf)) {
                /* 
                 * climb tree 
                 */
                act("Freeing your hands, you climb up the tree.\n\r",
                    FALSE, ch, 0, 0, TO_CHAR);
                act("$n stows away his weaponry and climbs up the tree.",
                    FALSE, ch, 0, 0, TO_ROOM);

                /*
                 * remove weapon 
                 */
                if (ch->equipment[WIELD]) {
                    obj = ch->equipment[WIELD];
                    if ((obj = unequip_char(ch, WIELD)) != NULL) {
                        objectGiveToChar(obj, ch);
                    }
                }

                /*
                 * remove held 
                 */
                if (ch->equipment[HOLD]) {
                    obj = ch->equipment[HOLD];
                    if ((obj = unequip_char(ch, HOLD)) != NULL) {
                        objectGiveToChar(obj, ch);
                    }
                }

                /*
                 * remove light 
                 */
                if (ch->equipment[WEAR_LIGHT]) {
                    obj = ch->equipment[WEAR_LIGHT];
                    if ((obj = unequip_char(ch, WEAR_LIGHT)) != NULL) {
                        objectGiveToChar(obj, ch);
                    }
                }

                char_from_room(ch);
                char_to_room(ch, CLIMB_ROOM);
                act("$n climbs up from below.", FALSE, ch, 0, 0, TO_ROOM);
                do_look(ch, NULL, 0);
                return (TRUE);
            }
            sprintf(buffer, "You don't see any %s to climb around here.\n\r",
                    buf);
            send_to_char(buffer, ch);
            return (TRUE);
        }
        send_to_char("Climb what?\n\r", ch);
        return (TRUE);
    }
    return (FALSE);
}

int close_doors(struct char_data *ch, struct room_data *rp, int cmd)
{
    struct room_direction_data *exitp,
                   *back;
    char            doorname[MAX_STRING_LENGTH + 30],
                    buf[MAX_STRING_LENGTH + 30];
    int             doordir = 0;
    char           *dir_name[] = {
        "to the north",
        "to the east",
        "to the south",
        "to the west",
        "above",
        "below"
    };

    /*
     * initialize info for different rooms here 
     * use (0,1,2,3,4,5) for dirs (north,east,south,west,up,down) 
     */
    if (ch->in_room == 17429) {
        sprintf(doorname, "door");
        doordir = 3;
    } else if (ch->in_room == 17430) {
        sprintf(doorname, "door");
        doordir = 3;
    } else if (ch->in_room == 17431) {
        sprintf(doorname, "door");
        doordir = 3;
    } else if (ch->in_room == 17432) {
        sprintf(doorname, "door");
        doordir = 3;
    }

    rp = roomFindNum(ch->in_room);
    exitp = rp->dir_option[doordir];

    if (IS_SET(exitp->exit_info, EX_CLOSED)) {
        /* 
         * already closed, no need to run 
         */
        return (FALSE);
    }

    if (cmd) {
        /* 
         * when command is given, boom, door closed and locked on this side 
         */
        SET_BIT(exitp->exit_info, EX_CLOSED);
        SET_BIT(exitp->exit_info, EX_LOCKED);
        sprintf(buf, "The %s %s slams shut.\n\r\n\r", doorname,
                dir_name[doordir]);
        send_to_room(buf, ch->in_room);

        /*
         * other side closes too, but not locked 
         */
        if (exit_ok(exitp, &rp) && 
            (back = rp->dir_option[direction[doordir].rev]) && 
            back->to_room == ch->in_room) {

            SET_BIT(back->exit_info, EX_CLOSED);
            sprintf(buf, "The %s %s slams shut.\n\r", doorname,
                    dir_name[direction[doordir].rev]);
            send_to_room(buf, exitp->to_room);
        }
    }
    return (FALSE);
}

#define CHESTS_ROOM 51161
int cog_room(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{
    struct obj_data    *obj;
    char               *cogname;
    LinkedListItem_t   *item;

    if (!cmd || !ch || cmd != 374 || !ch->in_room || !arg || !*arg) {
        return (FALSE);
    }

    arg = get_argument(arg, &cogname);

    if (cogname && !strcmp(cogname, "cog")) {
        if (cog_sequence < 0) {
            /*
             * alas, you made an error earlier 
             */
            send_to_char("Nothing seems to happen. the gears must have jammed "
                         "somewhere.\n\r", ch);
            return (TRUE);
        } 
        
        switch(cog_sequence) {
        case 0:
            /*
             * we got a new chance. determine chest by this room 
             */
            switch (ch->in_room) {
            case 51195:
                chest_pointer = 51168;
                break;
            case 51204:
                chest_pointer = 51169;
                break;
            case 51231:
                chest_pointer = 51170;
                break;
            case 51258:
                chest_pointer = 51171;
                break;
            case 51300:
                chest_pointer = 51172;
                break;
            default:
                chest_pointer = 0;
                cog_sequence = 0;
                break;
            }
            cog_sequence++;
            send_to_room("As the cog moves into place, the hum of machinery "
                         "running fills the air.\n\r", ch->in_room);
            break;
        case 1:
            if ((ch->in_room == 51204 && chest_pointer == 51168) ||
                (ch->in_room == 51231 && chest_pointer == 51169) ||
                (ch->in_room == 51258 && chest_pointer == 51170) ||
                (ch->in_room == 51300 && chest_pointer == 51171) ||
                (ch->in_room == 51258 && chest_pointer == 51172)) {

                cog_sequence++;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines becomes louder.\n\r", ch->in_room);
            } else {
                cog_sequence = -1;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines creaks and stops.\n\r", ch->in_room);
            }
            break;
        case 2:
            if ((ch->in_room == 51231 && chest_pointer == 51168) ||
                (ch->in_room == 51258 && chest_pointer == 51169) ||
                (ch->in_room == 51300 && chest_pointer == 51170) ||
                (ch->in_room == 51195 && chest_pointer == 51171) ||
                (ch->in_room == 51231 && chest_pointer == 51172)) {

                cog_sequence++;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines becomes louder.\n\r", ch->in_room);
            } else {
                cog_sequence = -1;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines creaks and stops.\n\r", ch->in_room);
            }
            break;
        case 3:
            if ((ch->in_room == 51258 && chest_pointer == 51168) ||
                (ch->in_room == 51300 && chest_pointer == 51169) ||
                (ch->in_room == 51195 && chest_pointer == 51170) ||
                (ch->in_room == 51204 && chest_pointer == 51171) ||
                (ch->in_room == 51204 && chest_pointer == 51172)) {

                cog_sequence++;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines becomes louder.\n\r", ch->in_room);
            } else {
                cog_sequence = -1;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines creaks and stops.\n\r", ch->in_room);
            }
            break;
        case 4:
            if ((ch->in_room == 51300 && chest_pointer == 51168) ||
                (ch->in_room == 51195 && chest_pointer == 51169) ||
                (ch->in_room == 51204 && chest_pointer == 51170) ||
                (ch->in_room == 51231 && chest_pointer == 51171) ||
                (ch->in_room == 51195 && chest_pointer == 51172)) {

                cog_sequence = -1;
                send_to_room("As the cog moves into place, the machine grinds "
                             "to a halt and a loud click can be heard "
                             "nearby.\n\r", ch->in_room);

                /*
                 * unlock and open chest 
                 */
                rp = roomFindNum(CHESTS_ROOM);
                if (!rp) {
                    Log("no room found for chest storage in cog_room proc");
                    return (TRUE);
                }

                LinkedListLock( rp->contentList );
                for( item = rp->contentList->head; item; item = item->next ) {
                    obj = CONTENT_LINK_TO_OBJ(item);

                    if (obj->item_number == chest_pointer) {
                        if (IS_SET(obj->value[1], CONT_LOCKED)) {
                            REMOVE_BIT(obj->value[1], CONT_LOCKED);
                        }
                        if (IS_SET(obj->value[1], CONT_CLOSED)) {
                            REMOVE_BIT(obj->value[1], CONT_CLOSED);
                        }
                        send_to_room("The machine grinds to a halt and one of "
                                     "the chests emits a loud click.\n\r",
                                     CHESTS_ROOM);
                        LinkedListUnlock( rp->contentList );
                        return (TRUE);
                    }
                }
                LinkedListUnlock( rp->contentList );
                Log("Couldn't find appropriate chest in CHESTS_ROOM");
            } else {
                cog_sequence = -1;
                send_to_room("As the cog moves into place, the hum of of the "
                             "machines creaks and stops.\n\r", ch->in_room);
            }
            break;
        default:
            Log("got to bad spot in cog_room");
            break;
        }
        return (TRUE);
    }
    return (FALSE);
}


#define GNOME_HOME 52857
#define GNOME_MOB  52853
int gnome_home(struct char_data *ch, int cmd, char *arg,
               struct room_data *rp, int type)
{
    char           *buf;
    struct char_data *gnome;

    if (cmd != 429 || !ch || !ch->in_room) {
        /* 
         * knock 
         */
        return (FALSE);
    }

    rp = roomFindNum(ch->in_room);

    if (!rp) {
        return (FALSE);
    }
    arg = get_argument(arg, &buf);

    if (buf && (!strcasecmp("door", buf) || !strcasecmp("Door", buf) || 
                !strcasecmp("DOOR", buf))) {
        /* 
         * knock door 
         */
        if (get_char_vis_world(ch, "gnome female collector", NULL)) {
            send_to_char("But it's already open!\n\r", ch);
            return (TRUE);
        } 

        if ((gnome = read_mobile(GNOME_MOB))) {
            send_to_char("You courteously knock on the little door, and it "
                         "opens.\n\r", ch);
            char_to_room(gnome, ch->in_room);

            send_to_char("Out comes a tiny gnomish woman, who peeps up at "
                         "you.\n\r", ch);
            send_to_char("She says, 'You got anything for me? I'm getting "
                         "desperate.'\n\r", ch);
            send_to_char("She looks at you in a meaningful way, obviously "
                         "expecting something.\n\r", ch);
            return (TRUE);
        } else {
            Log("could not find GNOME_MOB in gnome_home proc");
        }
    }
    return (FALSE);
}

int greed_disabler(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{
    if (cmd == 60) {
        /* 
         * drop 
         */
        act("Just when $n is about to drop something, $e seems to change $s "
            "mind.", FALSE, ch, 0, 0, TO_ROOM);
        act("Drop something, with all these filthy thieves around? Better not, "
            "they're bound to steal it!", FALSE, ch, 0, 0, TO_CHAR);
        return (TRUE);
    }
    return (FALSE);
}

int lag_room(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{
    if (cmd) {
        /* 
         * if a player enters ANY command, they'll experience two rounds of 
         * lag 
         */

        if (IS_NPC(ch) || IS_IMMORTAL(ch)) {
            return (FALSE);
        }
        act("$n yawns and stretches $s muscles, such activity!", FALSE, ch,
            0, 0, TO_ROOM);
        act("Yawning, you decide it's time to put in a bit of effort.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("Though you sure are entitled to some rest after this.", FALSE,
            ch, 0, 0, TO_CHAR);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
    return (FALSE);
}

/*
 * @Name:           guardianroom 
 * @description:    A room which will create guardians,
 *                  prevent people from travelling north 
 *                  the first time they try, then they 
 *                  have to fight their way through and 
 *                  more mobiles will jump out and join 
 *                  the fight 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    room(37823) 
 */

int guardianroom(struct char_data *ch, int cmd, char *arg,
                 struct room_data *rp, int type)
{
    struct char_data *tempchar;
    int             i = 0;
    int             j = 0;
    int             numbertocreate = 1;

    if(IS_IMMORTAL(ch)) {
        return(FALSE);
    }

    if (cmd == 1 && rp->special < 1) {
        for (tempchar = rp->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (IS_PC(tempchar) && !IS_IMMORTAL(tempchar)) {
                i++;
            }
        }
        if (i == 1) {
            CreateAMob(ch, GUARDIANMOBVNUM, 4,
                       "One of the reliefs on the walls jumps out and attacks "
                       "you!");
            CreateAMob(ch, GUARDIANMOBVNUM, 4,
                       "One of the reliefs on the walls jumps out and attacks "
                       "you!");
        } else {
            for (; i > 0; i--) {
                CreateAMob(ch, GUARDIANMOBVNUM, 4,
                           "One of the reliefs on the walls jumps out and "
                           "attacks you!");
            }
        }
        rp->special = 1;
        return (TRUE);
    } 
    
    if (rp->special >= 1) {
        i = 0;
        for (tempchar = rp->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (IS_PC(tempchar) && !IS_IMMORTAL(tempchar)) {
                i++;
            }
        }

        if (i >= 1) {
            if (rp->special % 3 == 0) {
                /* 
                 * create more mobs
                 */
                numbertocreate = MAX(1, i >> 1);
                for (; j < numbertocreate; j++) {
                    CreateAMob(ch, GUARDIANMOBVNUM, 4,
                               "Another relief jumps off the wall, becoming "
                               "real and deadly!");
                }
                if(cmd == 1) {
                    return(TRUE);
                }
            }
            rp->special = rp->special + 1;
        }
    }
    return (FALSE);
}

int knockproc(struct char_data *ch, int cmd, char *arg,
              struct room_data *rp, int type)
{
    time_t          curr_time;
    struct tm      *t_info;

    Log("Knockproc");
    if (cmd != 429) {
        return (FALSE);
    }
    curr_time = time(0);
    t_info = localtime(&curr_time);

    curr_time = time(NULL);
    if (t_info->tm_hour != THE_HOUR && 
        IS_SET(rp->dir_option[0]->exit_info, EX_LOCKED)) {
        command_interpreter(ch, "zload 188");
        raw_unlock_door(ch, EXIT(ch, 0), 0);
        open_door(ch, 0);
        oldSendOutput(ch, "You knock on the big wooden door and then slowly, it "
                      "opens.\n\r");

        act("$n knocks on the big door and then suddenly, the big door opens "
            "up.", TRUE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    }

    oldSendOutput(ch, "You knock on the big wooden door but nothing seems to "
                  "happen.\n\r");
    return (TRUE);
}

int legendfountain(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{
    int             level = 50;

    if (cmd != 11) {
        /* 
         * drink 
         */
        return (FALSE);
    }

    if (GET_COND(ch, THIRST) > 18) {
        send_to_char("You don't feel thirsty, and decide against it.\n\r", ch);
        return (TRUE);
    }

    if (GET_COND(ch, FULL) > 18) {
        send_to_char("You're too full to drink.\n\r", ch);
        return (TRUE);
    }

    send_to_char("You drink from the Fountain of Legends.\n\r", ch);
    act("$n drinks from the Fountain of Legends.", FALSE, ch, 0, 0, TO_ROOM);
    GET_COND(ch, THIRST) = 20;
    GET_COND(ch, FULL) += 1;

    if (GET_COND(ch, FULL) > 18) {
        act("Too much water has sated your appetite.", FALSE, ch, 0, 0,
            TO_CHAR);
        return (TRUE);
    }

    /*
     * now for the magic spellup bonus 
     */
    switch (number(0, 30)) {
    case 1:
        cast_refresh(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 2:
        cast_stone_skin(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 3:
        cast_cure_serious(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 4:
        cast_cure_light(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 5:
        cast_armor(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 6:
        cast_bless(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 7:
        cast_invisibility(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 8:
        cast_strength(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 9:
        cast_remove_poison(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 10:
        cast_true_seeing(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 11:
        cast_flying(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 12:
        cast_fly_group(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 13:
        cast_cure_critic(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 14:
        cast_heal(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 15:
        cast_second_wind(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 16:
        cast_water_breath(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 17:
        cast_shield(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 18:
        cast_find_traps(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 19:
        cast_protection_from_evil_group(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 20:
        cast_protection_from_evil(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 21:
        cast_sanctuary(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 22:
        cast_sense_life(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    case 23:
        cast_aid(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
        break;
    default:
        send_to_char("Tasty stuff, no?\n\r", ch);
        break;
    }
    return (TRUE);
}


#define ACORN 52885
int pick_acorns(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    char           *buf;
    struct obj_data *obj;

    if (!ch || !cmd || cmd != 155) {
        return (FALSE);
    }

    arg = get_argument(arg, &buf);
    if (buf && (!strcasecmp("acorn", buf) || !strcasecmp("acorns", buf))) {
        if (number(0, 2)) {
            act("You pick a delicious looking acorn.", FALSE, ch,
                0, 0, TO_CHAR);
            act("$n picks an acorn.", FALSE, ch, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);

            if ((obj = objectRead(ACORN))) {
                objectGiveToChar(obj, ch);
            } else {
                Log("no acorns found for pick_acorns");
            }
        } else {
            act("You try to pick an acorn, but the one you want hangs just out"
                " of your reach.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n tries to pick an acorn, but can't reach it.",
                FALSE, ch, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);
        }
        return (TRUE);
    }
    return (FALSE);
}

#define BERRYBASE 52861

int pick_berries(struct char_data *ch, int cmd, char *arg,
                 struct room_data *rp, int type)
{
    char           *buf;
    int             affect = 1;
    int             berry = 0;
    struct obj_data *obj;

    if (!ch || !cmd || cmd != 155) {
        return (FALSE);
    }

    arg = get_argument(arg, &buf);
    if (buf && (!strcasecmp("berry", buf) || !strcasecmp("berries", buf))) {
        if (number(0, 4)) {
            switch (GET_WIS(ch) + number(0, 10)) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                affect = 0;
                break;
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
                affect = 2;
                break;
            case 26:
            case 27:
            case 28:
                affect = 3;
                break;
            case 29:
                affect = 4;
                break;
            default:
                affect = 1;
                break;
            }
            berry = BERRYBASE + affect;
            act("You pick some berries.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n picks some berries.", FALSE, ch, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);

            obj = objectRead(berry);
            objectGiveToChar(obj, ch);
        } else {
            act("You try to pick some berries, but hurt yourself on a thorn.",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$n tries to pick some berries, but hurts $mself on a thorn.",
                FALSE, ch, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE);
        }
        return (TRUE);
    }
    return (FALSE);
}

/*
 * @Name: This is the chest part of the royal rumble proc. @description:
 * Once there is only 1 person left in the arena, the chest will open.
 * @Author: Greg Hovey (Banon) @Assigned to obj/mob/room: Room(51151) 
 */
#define THE_HOUR  17
int preperationproc(struct char_data *ch, int cmd, char *arg,
                    struct room_data *rp, int type)
{
    struct char_data *i;
    int             count = 0,
                    randnum = 0,
                    x = 0,
                    zone = 0;
    time_t            curr_time;
    struct tm      *t_info;

    if (cmd != 224) {
        /* 
         * pull
         */
        return (FALSE);
    }

    curr_time = time(0);
    t_info = localtime(&curr_time);

    curr_time = time(NULL);
    count = count_People_in_room(ch->in_room);
    zone = countPeople(RUMBLE_ZONE);

    if ((zone - count == 1 || zone - count == 0) && 
        t_info->tm_hour == THE_HOUR) {

        if (!IS_SET(rp->dir_option[2]->exit_info, EX_LOCKED) || 
            !IS_SET(rp->dir_option[2]->exit_info, EX_CLOSED)) {
            command_interpreter(ch, "close door");
            raw_lock_door(ch, EXIT(ch, 2), 2);
        }

        oldSendOutput(ch, "You knock on the big wooden door and then slowly, it "
                      "opens.\n\r");

        randnum = number(0, count - 1);

        for (i = roomFindNum(ch->in_room)->people; i; i = i->next_in_room) {
            if (i && x == randnum) {
                send_to_char("You pull the rope of the gong.\n\r", ch);
                send_to_zone("$c000BThe gong sounds as someone new gets pushed"
                             " into the arena.$c000w\n\r", ch);
                send_to_char("$c000BThe gong sounds as someone new gets pushed"
                             " into the arena.$c000w\n\r", ch);
                char_from_room(i);
                send_to_char("You blink for a second and find yourself "
                             "elsewhere.\n\r", i);

                char_to_room(i, number(51153, 51158));
                do_look(i, NULL, 0);
                return (TRUE);
            }
            x++;
        }
    } else {
        oldSendOutput(ch, "You ring the gong but nothing happens.\n\r");
        command_interpreter(ch, "zload 188");
    }

    return (TRUE);
}

/*
 * Stuff for Heximal's new zones -Lennya 20030315
 */
int riddle_exit(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    struct room_direction_data *exitp,
                   *back;
    char           *guess,
                    answer[MAX_STRING_LENGTH + 30];
    char            doorname[MAX_STRING_LENGTH + 30],
                    buf[MAX_STRING_LENGTH + 30];
    char           *buffer;
    int             doordir = 0;

    /*
     * initialize info for different rooms here 
     */
    /*
     * use (0,1,2,3,4,5) for dirs (north,east,south,west,up,down) 
     */
    if (ch->in_room == 50962) {
        sprintf(answer, "coffin");
        sprintf(doorname, "portal");
        doordir = 0;
    } else if (ch->in_room == 50963) {
        sprintf(answer, "fire");
        sprintf(doorname, "portal");
        doordir = 0;
    } else if (ch->in_room == 50964) {
        sprintf(answer, "rainbow");
        sprintf(doorname, "portal");
        doordir = 0;
    }

    rp = roomFindNum(ch->in_room);
    exitp = rp->dir_option[doordir];
    if (!IS_SET(exitp->exit_info, EX_CLOSED)) {
        /* 
         * if door is open, no need for proc 
         */
        return (FALSE);
    }

    if (cmd == 258) {
        /* hey now, this one can't be opened with force! */
        sprintf(buf, "You charge at the %s, only to be repelled by a strange "
                     "force.\n\r", doorname);
        send_to_char(buf, ch);

        send_to_char("You are overcome by a strange exhaustion.\n\r", ch);
        sprintf(buf, "$n charges at the %s but is repelled by a strange force "
                     "and collapses.", doorname);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);

        GET_HIT(ch) -= number(1, 10) * 2;
        if (GET_HIT(ch) < 0) {
            GET_HIT(ch) = 0;
        }
        GET_POS(ch) = POSITION_STUNNED;
        return (TRUE);
    }

    if (cmd == 99) {
        /* 
         * open 
         */
        arg = get_argument(arg, &buffer);
        if (buffer) {
            if (strcasecmp(buffer, doorname)) {
                return (FALSE);
            } else {
                /* 
                 * open portal 
                 */
                sprintf(buf, "You attempt to open the %s, but a strange force "
                             "prevents\n\r", doorname);
                send_to_char(buf, ch);

                send_to_char("you from touching it. There must be another way "
                             "to open it.\n\r", ch);
                sprintf(buf, "$n attemtps to open the %s, but seems unable to "
                             "touch it.", doorname);
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
    }

    if (cmd == 17) {
        /* 
         * say 
         */
        arg = get_argument(arg, &guess);
        if (guess && !strcasecmp(guess, answer)) {
            /*
             * open the exit in this room 
             */
            rp = roomFindNum(ch->in_room);
            exitp = rp->dir_option[doordir];
            REMOVE_BIT(exitp->exit_info, EX_CLOSED);
            act("$n whispers something in a low voice, and the light in "
                "the room dims.", FALSE, ch, 0, 0, TO_ROOM);
            sprintf(buf, "You softly whisper '%s' and the light in the "
                         "room dims.\n\r", answer);
            send_to_char(buf, ch);
            sprintf(buf, "The %s silently opens.\n\r", doorname);
            send_to_room(buf, ch->in_room);
            /*
             * gotta open the exit at the other side as well 
             */
            if (exit_ok(exitp, &rp) && 
                (back = rp->dir_option[direction[doordir].rev]) && 
                back->to_room == ch->in_room) {
                REMOVE_BIT(back->exit_info, EX_CLOSED);
                send_to_room(buf, exitp->to_room);
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

/*
 * @Name:           trapjawsroom 
 * @description:    A room which prevents people from going 
 *                  north, slashing them, unless they deactivate 
 *                  the trap using the toss object command.  toss 
 *                  person will result in a pissed off person
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    room(37857) 
 */
int trapjawsroom(struct char_data *ch, int cmd, char *arg,
                 struct room_data *rp, int type)
{
    char           *buf;
    char            newdesc[400];
    struct obj_data *tossitem;
    struct char_data *tempchar;
    int             dam = 0;

    if (cmd != 1 && cmd != 459 && cmd != 258) {
        return (FALSE);
    }

    if (rp->special == 1) {
        return (FALSE);
    }

    if (cmd == 1 || cmd == 258) {
        /* 
         * prevent them from going north, hit character with trap damage
         */
        act("You head north, but as you pass through the jaws, they suddenly "
            "slam down upon you, cutting you badly!", FALSE, ch, 0, 0, TO_CHAR);
        act("$n starts to head north through the jaws, but they clamp down "
            "just as $e passes through them, slicing $m badly!", FALSE, ch, 0,
            ch, TO_ROOM);

        dam = dice(30, 8);
        if (GET_DEX(ch) < dice(4, 6)) {
            dam = dam >> 1;
        }

        doroomdamage(ch, dam, SPELL_BLADE_BARRIER);
        act("If only there was a way to trick the jaws into closing on nothing,"
            " perhaps they would cease to function.", FALSE, ch, 0, ch,
            TO_CHAR);
        act("If only there was a way to trick the jaws into closing on nothing,"
            " perhaps they would cease to function.", FALSE, ch, 0, ch,
            TO_ROOM);
        /*
         * slice up character badly
         */
        return (TRUE);
    } 
    
    if (cmd == 459) {
        /* 
         * toss object, disarm trap, scrap object
         * toss person, ouch, hit them with big trap damage
         */

        arg = get_argument(arg, &buf);

        if (buf && (tossitem = objectGetOnChar(ch, buf, ch))) {
            act("You toss $p towards the gaping jaws and they crash down "
                "suddenly on it, smashing it to pieces.", FALSE, ch, tossitem,
                0, TO_CHAR);
            act("$n tosses $p towards the gaping jaws and they crash down "
                "suddenly on it, smashing it to pieces.", FALSE, ch, tossitem,
                0, TO_ROOM);
            act("The jaws crash down together, and with a sharp snapping sound,"
                " they break apart!", FALSE, ch, 0, 0, TO_CHAR);
            act("The jaws crash down together, and with a sharp snapping sound,"
                " they break apart!", FALSE, ch, 0, 0, TO_ROOM);
            /* 
             * scrap item
             */
            MakeScrap(ch, NULL, tossitem);

            strcpy(newdesc, "");
            strcat(newdesc, "  This room's former skull shape has been "
                            "completely ruined\n\r");
            strcat(newdesc, "by the collapse of the jaw.  The broken pieces of"
                            " the jaw\n\r");
            strcat(newdesc, "show an advanced mechanism inside to drive the "
                            "former trap.\n\r");
            strcat(newdesc, "Now you can make your way through the rubble "
                            "towards the north\n\r");
            strcat(newdesc, "or clamber up the remains of the staircase.\n\r");

            if (rp->description) {
                free(rp->description);
            }
            rp->description = (char *) strdup(newdesc);

            rp->special = 1;
            return (TRUE);
        } 
        
        if (buf && (tempchar = get_char_room(buf, ch->in_room))) {
            if (tempchar == ch) {
                act("You want to toss yourself?", FALSE, ch, 0, 0, TO_CHAR);
            } else {
                act("You toss $N in the air, but they drift too close to the "
                    "jaws and the jaws suddenly slam down, slicing $M badly!",
                    FALSE, ch, 0, tempchar, TO_CHAR);
                act("$n grabs you and tosses you in the air, you fly too close "
                    "to the jaws of the doorway and they slam shut, cutting "
                    "you!", FALSE, ch, 0, tempchar, TO_VICT);
                act("$n tosses $N in the air, but the jaws suddenly slam down "
                    "on $M, slashing $M terribly!.", FALSE, ch, 0, tempchar,
                    TO_NOTVICT);
                /* 
                 * slice up character badly
                 */
                dam = dice(40, 8);
                doroomdamage(tempchar, dam, SPELL_BLADE_BARRIER);
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

/*
 * @Name:           traproom 
 * @description:    A room which does fire damage, but only
 *                  once, and changes the room description 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    room(37840) 
 */

int traproom(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{
    struct char_data *tempchar;
    char           *buf;
    char            newdesc[400];
    int             trapdam = 0;
    int             trapdamsave = 0;

    if (cmd != 15) {
        return (FALSE);
    }
    if (rp->special == 1) {
        return (FALSE);
    }

    arg = get_argument(arg, &buf);

    if (!buf) {
        return (FALSE);
    }

    if (!strcmp(buf, "at")) {
        arg = get_argument(arg, &buf);
    }

    if (buf && (!strcasecmp("green", buf) || !strcmp("powder", buf) || 
                !strcmp("powder-green", buf) || !strcmp("green-powder", buf))) {
        act("As you lean over to look at the strange powder, a drop of your "
            "sweat falls.", FALSE, ch, 0, 0, TO_CHAR);
        act("You hardly pay it any attention but then it sparks as it hits the"
            " powder.", FALSE, ch, 0, 0, TO_CHAR);
        act("$c000RSuddenly, the whole room bursts into flame!", FALSE,
            ch, 0, 0, TO_CHAR);
        act("As $n leans over to look at the strange powder, $e jerks back "
            "suddenly!", FALSE, ch, 0, 0, TO_ROOM);
        act("$c000RThere is a flash, and the room bursts into flames!",
            FALSE, ch, 0, 0, TO_ROOM);

        /* 
         * randomize it, but average 150
         */
        trapdam = dice(50, 5);
        trapdamsave = trapdam >> 1;

        for (tempchar = rp->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (!IS_IMMORTAL(tempchar)) {
                if (!saves_spell(tempchar, SAVING_SPELL)) {
                    if (!saves_spell(tempchar, SAVING_SPELL - 4)) {
                        BurnWings(tempchar);
                        /* 
                         * Fail two saves, wings burn 
                         */
                    }
                    heat_blind(tempchar);
                    doroomdamage(tempchar, trapdam, SPELL_INCENDIARY_CLOUD);
                } else {
                    heat_blind(tempchar);
                    doroomdamage(tempchar, trapdamsave, SPELL_INCENDIARY_CLOUD);
                }
            }
        }

        /* 
         * Now do room description change, and set a flag to indicate
         * no more damage can be done
         */
        rp->special = 1;

        strcpy(newdesc, "");
        strcat(newdesc, "  This room has been completely stripped by fire.  "
                        "Scorch\n\r");
        strcat(newdesc, "marks are now the only thing of note.  The fact that "
                        "every\n\r");
        strcat(newdesc, "surface of the room is almost uniformly blackened by "
                        "char\n\r");
        strcat(newdesc, "makes this room darker than it should be.  The scorch "
                        "marks\n\r");
        strcat(newdesc, "bleed out into the hallway slightly and into the "
                        "closet.\n\r");

        if (rp->description) {
            free(rp->description);
        }
        rp->description = (char *) strdup(newdesc);

        return (TRUE);
    }
    return (FALSE);
}

/*
 * @Name:           ventroom 
 * @description:    A room will capture commands, looking for 
 *                  object based commands to use on the vent, 
 *                  which exists in another room
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    room(37839) 
 */
int ventroom(struct char_data *ch, int cmd, char *arg,
             struct room_data *rp, int type)
{
    char           *buf1;
    char           *buf2;
    struct obj_data *ventobj;
    struct room_data *ventroom;

    if (!cmd) {
        return (FALSE);
    }

    if (cmd != 99 && cmd != 15 && cmd != 10 && cmd != 167 && cmd != 67 && 
        cmd != 100) {
        /* 
         * open, look, get, take, put, close
         */
        return (FALSE);
    }

    ventroom = roomFindNum(VENTROOMVNUM);
    ventobj = objectGetInRoomNum(VENTOBJVNUM, ventroom);

    if (!ventobj) {
        return (FALSE);
    }

    if(rp->special == 0) {
        /* This verifies the ventobject is actually closed when it should be */
        if (!IS_SET(ventobj->value[1], CONT_CLOSED)) {
            SET_BIT(ventobj->value[1], CONT_CLOSED);
        }
    }

    arg = get_argument(arg, &buf1);
    arg = get_argument(arg, &buf2);

    if (!buf1 || !buf2 || (strcmp(buf1, "vent") && strcmp(buf2, "vent"))) {
        return (FALSE);
    }

    if (cmd == 99) {
        /* 
         * open (only with 19 strength)
         */
        if (GET_STR(ch) == 19) {
            /* 
             * go to room, do command
             */
            char_from_room(ch);
            char_to_room(ch, VENTROOMVNUM);
            do_open(ch, arg, -1);
            char_from_room(ch);
            char_to_room(ch, CLOSETROOMVNUM);
            rp->special = 1;
        } else {
            act("You are not strong enough to pull open the vent.",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$n attempts to pull open the vent, but fails, chest "
                "heaving from the exertion.", FALSE, ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    } else if (cmd == 15) {
        /* 
         * look
         * go to room, do command
         */
        char_from_room(ch);
        char_to_room(ch, VENTROOMVNUM);
        do_look(ch, arg, -1);
        char_from_room(ch);
        char_to_room(ch, CLOSETROOMVNUM);
    } else if (cmd == 10 || cmd == 167) {
        /* 
         * get, take
         * go to room, do command (NO TAKE FLAG on VENT OBJECT)
         */
        if (rp->special != 1) {
            act("You attempt to fiddle with the vent, but realize that you"
                " can't take it, and it's not opened.", FALSE, ch, 0, 0,
                TO_CHAR);
        } else {
            char_from_room(ch);
            char_to_room(ch, VENTROOMVNUM);
            do_get(ch, arg, -1);
            char_from_room(ch);
            char_to_room(ch, CLOSETROOMVNUM);
            rp->special = 0;
            act("You manage to accidently slam the vent closed, you will "
                "have to open it again to get back into it.", FALSE, ch, 0,
                0, TO_CHAR);
            act("With a 'BANG!' $n accidently slams the vent closed, you "
                "will have to open it again to get back into it.", FALSE, 
                ch, 0, 0, TO_ROOM);
            if (!IS_SET(ventobj->value[1], CONT_CLOSED)) {
                SET_BIT(ventobj->value[1], CONT_CLOSED);
            }
        }
    } else if (cmd == 67) {
        /* 
         * put
         */
        char_from_room(ch);
        char_to_room(ch, VENTROOMVNUM);
        do_put(ch, arg, -1);
        char_from_room(ch);
        char_to_room(ch, CLOSETROOMVNUM);
    } else if (cmd == 100) {
        /*
         * close
         */
        if (IS_SET(ventobj->value[1], CONT_CLOSED)) {
            act("Oh, it looks like the vent is already closed.", FALSE,
                ch, 0, 0, TO_CHAR);
        }
        else {
            act("You manage to accidently slam the vent closed, you will "
                "have to open it again to get back into it.", FALSE, ch, 0,
                0, TO_CHAR);
            act("With a 'BANG!' $n accidently slams the vent closed, you "
                "will have to open it again to get back into it.", FALSE,
                ch, 0, 0, TO_ROOM);
            if (!IS_SET(ventobj->value[1], CONT_CLOSED)) {
                SET_BIT(ventobj->value[1], CONT_CLOSED);
            }
        }
    }
    return (TRUE);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
