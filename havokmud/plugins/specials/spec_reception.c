/*
 * To calc last time online...
 *  computeminuteslast = (time(0)-st.last_update)/SECS_PER_REAL_MIN;
 *  computehourslast = (time(0)-st.last_update)/SECS_PER_REAL_HOUR;
 *  The last time you were on was %d minutes (%d hrs) ago according to rent.
 , computeminuteslast, computehourslast
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include "protos.h"

#define OBJ_SAVE_FILE "pcobjs.obj"
#define OBJ_FILE_FREE "\0\0\0"

extern struct room_data *world;
extern struct index_data *mob_index;
extern int      top_of_objt;
extern struct player_index_element *player_table;
extern int      top_of_p_table;

int             cur_depth = 0;

void WriteObjs(FILE * fl, struct obj_file_u *st);
int ReadObjs(FILE * fl, struct obj_file_u *st);

/*
 ************************************************************************
 * Routines used for the "Offer"                                           *
 ************************************************************************* */

void add_obj_cost(struct char_data *ch, struct char_data *re,
                  struct obj_data *obj, struct obj_cost *cost)
{
    char            buf[MAX_INPUT_LENGTH];
    char            tmp_str[MAX_INPUT_LENGTH * 2];
    char           *str_pos,
                   *i;
    int             temp;

    /*
     * Add cost for an item and it's contents, and next->contents 
     */

    if (obj) {
        if (obj->item_number > -1 && cost->ok && 
            ItemEgoClash(ch, obj, 0) > -5) {
            if (obj->cost_per_day > 9000) {
                /* 
                 * 1/2 price rent 
                 */
                temp = MAX(0, obj->cost_per_day);
            } else {
                temp = 0;
            }

            if (!IS_RARE(obj) || obj->cost_per_day <= 10000) {
                /* 
                 * Let's not charge for normal items 
                 */
                temp = 0;
            }

            cost->total_cost += temp;

            if (re) {
                /* 
                 * set up starting position 
                 */
                str_pos = tmp_str;
                for (i = obj->short_description; *i; i++) {
                    /*
                     * If we find an ANSI string, add spaces to the left
                     * of the string. This accounts for correct alignment
                     * b/c the ansi code will be taken out. 
                     */
                    if (*i == '$' && (*(i + 1) == 'c' || (*i + 1) == 'C')) {
                        memcpy(str_pos, "      ", 6);
                        str_pos += 6;
                    }
                }

                /*
                 * Now fill in the original description, making sure not
                 * to overrun the buffer. 
                 */
                strncpy(str_pos, obj->short_description,
                        &tmp_str[MAX_INPUT_LENGTH * 2 - 1] - str_pos);

                if (IS_RARE(obj)) {
                    sprintf(buf, "%30s : %d coins/day  $c000W[$c000RRARE"
                                 "$c000W]$c0007\n\r", tmp_str, temp);
                } else {
                    sprintf(buf, "%30s : %d coins/day\n\r", tmp_str, temp);
                }

                /*
                 * And send it to the player 
                 */
                send_to_char(buf, ch);
            }
            cost->no_carried++;
            add_obj_cost(ch, re, obj->contains, cost);
            add_obj_cost(ch, re, obj->next_content, cost);
        } else if (cost->ok) {
            if (re) {
                act("$c0013[$c0015$n$c0013] tells you 'I refuse storing $p'",
                    FALSE, re, obj, ch, TO_VICT);
                cost->ok = FALSE;
            } else {
#ifdef DUPLICATES
                act("Sorry, but $p don't keep in storage.", FALSE, ch, obj,
                    0, TO_CHAR);
#endif
                cost->ok = FALSE;
            }
        }
    }
}

/*
 * only time forcerent is true is when they idle off, in limits.c 
 */
bool recep_offer(struct char_data *ch, struct char_data *receptionist,
                 struct obj_cost *cost, int forcerent)
{
    int             discount = 0,
                    nbdays,
                    i,
                    ii,
                    limited_items = 0,
                    exit = 0;
    char            buf[MAX_STRING_LENGTH];
    struct obj_data *tmp,
                   *tmp_next_obj,
                   *rare;

    cost->total_cost = 100;
    cost->no_carried = 0;
    cost->ok = TRUE;

    if (forcerent) {
        Log("%s is being force rented!", GET_NAME(ch));
    }

    if (ch->specials.auction) {
        send_to_char("You are currently auctioning an item.\n\r", ch);
        return (FALSE);
    }

    if (ch->specials.minbid) {
        send_to_char("You are currently bidding on an item.\n\r", ch);
        return (FALSE);
    }

    add_obj_cost(ch, receptionist, ch->carrying, cost);
    limited_items += CountLims(ch->carrying);

    for (i = 0; i < MAX_WEAR; i++) {
        add_obj_cost(ch, receptionist, ch->equipment[i], cost);
        limited_items += CountLims(ch->equipment[i]);
    }

    if (cost->no_carried == 0) {
        if (receptionist) {
            act("$c0013[$c0015$n$c0013] tells you 'But you are not carrying "
                "anything?'", FALSE, receptionist, 0, ch, TO_VICT);
        }
        return (FALSE);
    }

#ifdef LIMITED_ITEMS
    if (limited_items > MaxLimited(GetMaxLevel(ch))) {
        if (receptionist) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'Sorry, but I can't"
                         " store more than %d limited items.",
                    MaxLimited(GetMaxLevel(ch)));
            act(buf, FALSE, receptionist, 0, ch, TO_VICT);
            return (FALSE);
        }

        /*
         * auto renting an idle person, lets wack items they should not
         * carry here! (limited items) 
         */
        if (!receptionist && forcerent) {
            i = (limited_items - MaxLimited(GetMaxLevel(ch)));

            /*
             * check carrying items first, least important 
             */
            if (CountLims(ch->carrying)) {
                if (i > 0 && (!IS_IMMORTAL(ch) || ch->invis_level <= 58)) {
                    Log("Removing carried items from %s in force rent.", 
                        GET_NAME(ch));
                }

                for (tmp = ch->carrying; tmp; tmp = tmp_next_obj) {
                    tmp_next_obj = tmp->next_content;
                    if (CountLims(tmp) && i > 0) {
                        exit = 0;
                        while (tmp && !exit && i > 0) {
                            rare = find_a_rare(tmp);
                            if (rare) {
                                if (rare != tmp) {
                                    obj_from_char(rare);
                                    objectExtract(rare);
                                } else {
                                    obj_from_char(tmp);
                                    objectExtract(tmp);
                                    tmp = 0;
                                }
                                i--;
                            } else {
                                exit = 1;
                            }
                        }
                    }
                }
            }

            /*
             * check equiped items next if still over max limited 
             */
            if (i > 0) {
                Log("Removing equipped items from %s in force rent.",
                    GET_NAME(ch));

                for (ii = 0; ii < MAX_WEAR; ii++) {
                    tmp = ch->equipment[ii];
                    if (CountLims(tmp) && i > 0) {
                        exit = 0;
                        while (tmp && !exit && i > 0) {
                            rare = find_a_rare(tmp);
                            if (rare) {
                                if (rare != tmp) {
                                    obj_from_char(rare);
                                    objectExtract(rare);
                                } else {
                                    obj_from_char(tmp);
                                    objectExtract(tmp);
                                    tmp = 0;
                                }
                                i--;
                            } else {
                                exit = 1;
                            }
                        }
                    }
                }
            }

            if (i > 0 && (!IS_IMMORTAL(ch) || ch->invis_level <= 58)) {
                Log( "%s force rented and still had more limited items"
                     " than supposed to.", GET_NAME(ch));
            }
        }
    }
#endif

    if (cost->no_carried > MAX_OBJ_SAVE) {
        if (receptionist) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'Sorry, but I can't"
                         " store more than %d items.", MAX_OBJ_SAVE);
            act(buf, FALSE, receptionist, 0, ch, TO_VICT);
        }
        return (FALSE);
    }
    
#ifdef MONK_ITEM_LIMIT
    if (!IS_IMMORTAL(ch) && HasClass(ch, CLASS_MONK) && 
        cost->no_carried > 20) { 
        send_to_char("Your vows forbid you to carry more"
                         " than 20 items\n\r", ch); return(FALSE); 
    } 
#endif

    if (!cost->ok) {
        return (FALSE);
    }

     
#ifdef NEW_RENT
    /*
     * RENTAL COST ADJUSTMENT 
     */
    cost->total_cost = 100;
#endif

    if (receptionist) {
        discount = 0;
        /* 
         * (GH) discounts for same race and 
         * same alignment. and charisma..
         */

        if (GET_RACE(ch) == GET_RACE(receptionist)) {
            discount = discount + 5;
        }
        if (IS_GOOD(receptionist) && IS_GOOD(ch)) {
            discount = discount + 5;
        } else if (IS_EVIL(receptionist) && IS_EVIL(ch)) {
            discount = discount + 5;
        } else if (IS_NEUTRAL(receptionist) && IS_NEUTRAL(ch)) {
            discount = discount + 5;
        }
        if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
            /* 
             * 15% discount for paladins
             */
            discount += 15;
        }

        if (GET_CLAN(ch) > 1) {
            discount = discount + 5;
        }
        if (GET_CHR(ch) > GET_CHR(receptionist)) {
            discount = discount + (1 + GET_CHR(ch) - GET_CHR(receptionist));
        }
        if (IS_SET(ch->specials.act, PLR_CLAN_LEADER)) {
            /* 
             * 10% discount for clan leaders..
             */
            discount += 10;
        }

        if (discount > 0 && discount != 100) {
            sprintf(buf, "$n winks at you and offers you a %d%% discount. "
                         "(%d coins)",
                    discount, ((cost->total_cost * discount / 100)));
            cost->total_cost = cost->total_cost - 
                              (cost->total_cost * discount / 100 + 1);
            act(buf, FALSE, receptionist, 0, ch, TO_VICT);
        }

        if (discount >= 100) {
            sprintf(buf, "$n quivers in fear and then offers you to stay for "
                         "free.");
            cost->total_cost = 1;
            act(buf, FALSE, receptionist, 0, ch, TO_VICT);
        }

        sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'It will cost you %d "
                     "coin%s per day.'", cost->total_cost, 
                     cost->total_cost != 1 ? "s" : "" );
        act(buf, FALSE, receptionist, 0, ch, TO_VICT);

        /*
         * just a bit on informative coding, wasted space... msw 
         */
        if (cost->total_cost <= GET_GOLD(ch)) {
            nbdays = (GET_GOLD(ch) / cost->total_cost) - 1;
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'You can stay for "
                         "%d day%s'", nbdays, nbdays != 1 ? "s" : "");
            act(buf, FALSE, receptionist, 0, ch, TO_VICT);
        }

        if (limited_items <= 5) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'You carry %d rare"
                         " item%s.'", 
                    limited_items, limited_items != 1 ? "s" : "");
        } else if (limited_items <= 12) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'Hum, You carry %d "
                         "rare items, nice.'", limited_items);
        } else if (limited_items < 18) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'You've got %d rare "
                         "items, great job.'", limited_items);
        } else if (limited_items >= 18) {
            sprintf(buf, "$c0013[$c0015$n$c0013] tells you 'WOW! You carry %d "
                         "rare items, super job!'", limited_items);
        }
        act(buf, FALSE, receptionist, 0, ch, TO_VICT);

        if (cost->total_cost > GET_GOLD(ch)) {
            if (!IS_IMMORTAL(ch)) {
                act("$c0013[$c0015$n$c0013] tells you 'Which I can see you "
                    "can't afford'", FALSE, receptionist, 0, ch, TO_VICT);
            } else {
                act("$c0013[$c0015$n$c0013] tells you 'Well, since you're a "
                    "God, I guess it's okay'", FALSE, receptionist, 0, ch, 
                    TO_VICT);
                cost->total_cost = 0;
            }
        }
    }

    if (cost->total_cost > GET_GOLD(ch)) {
        if (forcerent) {
            sprintf(buf, "%s is being force rented and does not have gold!",
                    GET_NAME(ch));
            log_sev(buf, 3);
            slog(buf);
        }
        return (FALSE);
    } else {
        return (TRUE);
    }

}


/*
 ************************************************************************
 * Routine Receptionist                                                    *
 ************************************************************************* */

int receptionist(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct obj_cost cost;
    struct char_data *recep = 0;
    struct char_data *temp_char;
    struct room_data *rp;
    short int       save_room;
    short int       action_table[9];

    if (!ch->desc) {
        /* 
         * You've forgot FALSE - NPC couldn't leave 
         */
        return (FALSE);
    }

    action_table[0] = 23;
    action_table[1] = 24;
    action_table[2] = 36;
    action_table[3] = 105;
    action_table[4] = 106;
    action_table[5] = 109;
    action_table[6] = 111;
    action_table[7] = 142;
    action_table[8] = 147;

    for (temp_char = real_roomp(ch->in_room)->people;
         (temp_char) && (!recep); temp_char = temp_char->next_in_room) {
        if (IS_MOB(temp_char) && 
            mob_index[temp_char->nr].func == receptionist) {
            recep = temp_char;
        }
    }

    if (!recep) {
        Log("No_receptionist.\n\r");
        assert(0);
    }

    if (!(rp = real_roomp(recep->in_room))) {
        Log("receptionist found, but not in a valid room");
        return (FALSE);
    }

    if (!IS_SET(rp->room_flags, PEACEFUL)) {
        SET_BIT(rp->room_flags, PEACEFUL);
    }
    if (!IS_SET(rp->room_flags, NO_SUM)) {
        SET_BIT(rp->room_flags, NO_SUM);
    }
    if (IS_NPC(ch)) {
        return (FALSE);
    }
    if ((cmd != 92) && (cmd != 93)) {
        if (!cmd && recep->specials.fighting) {
            return (citizen(recep, 0, "", mob, type));
        }

        if (!number(0, 30)) {
            do_action(recep, "", action_table[number(0, 8)]);
        }
        return (FALSE);
    }

    if (!AWAKE(recep)) {
        act("$e isn't able to talk to you...", FALSE, recep, 0, ch, TO_VICT);
        return (TRUE);
    }

    if (!CAN_SEE(recep, ch)) {
        act("$n says, 'I just can't deal with people I can't see!'", FALSE,
            recep, 0, 0, TO_ROOM);
        act("$n bursts into tears", FALSE, recep, 0, 0, TO_ROOM);
        return (TRUE);
    }

    if (cmd == 92) {
        /* 
         * Rent 
         */
        if (recep_offer(ch, recep, &cost, FALSE)) {
            act("$n stores your stuff in the safe, and helps you into your "
                "chamber.", FALSE, recep, 0, ch, TO_VICT);
            act("$n helps $N into $S private chamber.", FALSE, recep, 0,
                ch, TO_NOTVICT);
            ch->old_exp = 0;
            save_obj_reimb(ch);
            save_obj(ch, &cost, 1);
            save_room = ch->in_room;

            if (ch->specials.start_room != 2 && 
                !IS_SET(ch->specials.act, PLR_HAVEROOM)) {
                /* 
                 * hell 
                 */
                ch->specials.start_room = save_room;
            }

            /* 
             * you don't delete CHARACTERS 
             * when you extract them 
             */
            extract_char(ch);
            save_char(ch, save_room);
            ch->in_room = save_room;
        }
    } else {
        /* 
         * Offer 
         */
        recep_offer(ch, recep, &cost, FALSE);
        act("$N gives $n an offer.", FALSE, ch, 0, recep, TO_ROOM);
    }

    return (TRUE);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
