/*
 * To calc last time online...
 *  computeminuteslast = (time(0)-st.last_update)/SECS_PER_REAL_MIN;
 *  computehourslast = (time(0)-st.last_update)/SECS_PER_REAL_HOUR;
 *  The last time you were on was %d minutes (%d hrs) ago according to rent.
 , computeminuteslast, computehourslast
 */

#include "config.h"
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
extern struct index_data *obj_index;
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
            if (obj->obj_flags.cost_per_day > 9000) {
                /* 
                 * 1/2 price rent 
                 */
                temp = MAX(0, obj->obj_flags.cost_per_day);
            } else {
                temp = 0;
            }

            if (!IS_RARE(obj) || obj->obj_flags.cost_per_day <= 10000) {
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
#if NODUPLICATES
#else
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
    char            buf[MAX_INPUT_LENGTH];
    struct obj_data *tmp,
                   *tmp_next_obj,
                   *rare;

    cost->total_cost = 100;
    cost->no_carried = 0;
    cost->ok = TRUE;

    if (forcerent) {
        sprintf(buf, "%s is being force rented!", GET_NAME(ch));
        Log(buf);
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

    if (!cost->ok) {
        return (FALSE);
    }
    if (cost->no_carried == 0) {
        if (receptionist) {
            act("$c0013[$c0015$n$c0013] tells you 'But you are not carrying "
                "anything?'", FALSE, receptionist, 0, ch, TO_VICT);
        }
        return (FALSE);
    }

#if LIMITED_ITEMS
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
                    sprintf(buf, "Removing carried items from %s in force "
                                 "rent.", GET_NAME(ch));
                    Log(buf);
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
#if 0                                    
                                    obj_index[rare->item_number].number++;
#endif
                                    extract_obj(rare);
                                } else {
                                    obj_from_char(tmp);
#if 0                                    
                                    obj_index[tmp->item_number].number++;
#endif
                                    extract_obj(tmp);
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
                sprintf(buf, "Removing equipped items from %s in force rent.",
                        GET_NAME(ch));
                Log(buf);

                for (ii = 0; ii < MAX_WEAR; ii++) {
                    tmp = ch->equipment[ii];
                    if (CountLims(tmp) && i > 0) {
                        exit = 0;
                        while (tmp && !exit && i > 0) {
                            rare = find_a_rare(tmp);
                            if (rare) {
                                if (rare != tmp) {
                                    obj_from_char(rare);
#if 0                                    
                                    obj_index[rare->item_number].number++;
#endif
                                    extract_obj(rare);
                                } else {
                                    obj_from_char(tmp);
#if 0                                    
                                    obj_index[tmp->item_number].number++;
#endif
                                    extract_obj(tmp);
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

            if (i > 0&& (!IS_IMMORTAL(ch) || ch->invis_level <= 58)) {
                sprintf(buf, "%s force rented and still had more limited items"
                             " than supposed to.", GET_NAME(ch));
                Log(buf);
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
#if 0
    if (!IS_IMMORTAL(ch) && HasClass(ch, CLASS_MONK) && 
        cost->no_carried > 20) { 
        send_to_char("Your vows forbid you to carry more"
                         " than 20 items\n\r", ch); return(FALSE); 
    } 
#endif
     
#if NEW_RENT
    /*
     * RENTAL COST ADJUSTMENT 
     */
    cost->total_cost = 100;
#else
#endif

    if (receptionist) {
#if 1
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
#endif

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
            if (GetMaxLevel(ch) < LOW_IMMORTAL) {
                act("$c0013[$c0015$n$c0013] tells you 'Which I can see you "
                    "can't afford'", FALSE, receptionist, 0, ch, TO_VICT);
            } else if (GetMaxLevel(ch) > LOW_IMMORTAL) {
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
 * General save/load routines                                              *
 ************************************************************************* */

void update_file(struct char_data *ch, struct obj_file_u *st)
{
    FILE           *fl;
    char            buf[200];

    /*
     * write the aliases and bamfs 
     */
    write_char_extra(ch);

#if 0
    /*
     * this appears to fuck with saving for polies.. not exactly sure why. 
     * (jdb) 
     */
    if (IS_SET(ch->specials.act, ACT_POLYSELF))
        sprintf(buf, "rent/%s", lower(ch->desc->original->player.name));
    else
#endif
        sprintf(buf, "rent/%s", lower(ch->player.name));

    if (!(fl = fopen(buf, "w"))) {
        perror("saving PC's objects");
        assert(0);
        /* 
         * .. rent directory not made temporary for cygwin. Change back
         * before port...(GH)
         * printf("Error accessing pc objects");
         */
    } else {
        rewind(fl);
        strcpy(st->owner, GET_NAME(ch));
        WriteObjs(fl, st);
        fclose(fl);
    }
}

void update_reimb_file(struct char_data *ch, struct obj_file_u *st)
{
    FILE           *fl;
    char            buf[200];

    /*
     * write the aliases and bamfs: 
     */

    write_char_extra(ch);
    sprintf(buf, "reimb/%s", lower(ch->player.name));

    if (!(fl = fopen(buf, "w"))) {
        perror("saving PC's reimb file");
        assert(0);
    } else {
        rewind(fl);
        strcpy(st->owner, GET_NAME(ch));
        WriteObjs(fl, st);
        fclose(fl);
    }
}

/*
 ************************************************************************
 * Routines used to load a characters equipment from disk                  *
 ************************************************************************* */

void obj_store_to_char(struct char_data *ch, struct obj_file_u *st)
{
    char            buf[128];

    struct obj_data *obj;
    struct obj_data *in_obj[64],
                   *last_obj = NULL;
    int             tmp_cur_depth = 0;
    int             i,
                    j;

    void            obj_to_char(struct obj_data *object,
                                struct char_data *ch);

    for (i = 0; i < st->number; i++) {
        if (st->objects[i].item_number > -1
            && real_object(st->objects[i].item_number) > -1) {
            obj = read_object(st->objects[i].item_number, VIRTUAL);
            if (IS_RARE(obj)) {
                obj_index[obj->item_number].number--;
            }
            obj->obj_flags.value[0] = st->objects[i].value[0];
            obj->obj_flags.value[1] = st->objects[i].value[1];
            obj->obj_flags.value[2] = st->objects[i].value[2];
            obj->obj_flags.value[3] = st->objects[i].value[3];
            obj->obj_flags.extra_flags = st->objects[i].extra_flags;
            obj->obj_flags.weight = st->objects[i].weight;
            obj->obj_flags.timer = st->objects[i].timer;
            obj->obj_flags.bitvector = st->objects[i].bitvector;

            /*
             * new, saving names and descrips stuff o_s_t_c
             */
            if (obj->name) {
                free(obj->name);
            }
            if (obj->short_description) {
                free(obj->short_description);
            }
            if (obj->description) {
                free(obj->description);
            }
            obj->name = (char *) malloc(strlen(st->objects[i].name) + 1);
            obj->short_description =
                (char *) malloc(strlen(st->objects[i].sd) + 1);
            obj->description =
                (char *) malloc(strlen(st->objects[i].desc) + 1);

            strcpy(obj->name, st->objects[i].name);
            strcpy(obj->short_description, st->objects[i].sd);
            strcpy(obj->description, st->objects[i].desc);
            /*
             * end of new, possibly buggy stuff 
             */

            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                obj->affected[j] = st->objects[i].affected[j];
            }
            /*
             * item restoring 
             */
            if (st->objects[i].depth > 60) {
                Log("weird! object have depth >60.\r\n");
                st->objects[i].depth = 0;
            }

            if (st->objects[i].depth && st->objects[i].wearpos) {
                sprintf(buf, "weird! object (%s) worn AND in container.\r\n",
                        obj->name);
                Log(buf);
                st->objects[i].depth = st->objects[i].wearpos = 0;
            }

            if (st->objects[i].depth > tmp_cur_depth) {
                if (st->objects[i].depth != tmp_cur_depth + 1) {
                    sprintf(buf, "weird! object depth changed from %d to %d",
                            tmp_cur_depth, st->objects[i].depth);
                    Log(buf);
                }
                in_obj[tmp_cur_depth++] = last_obj;
            } else if (st->objects[i].depth < tmp_cur_depth) {
                tmp_cur_depth--;
            }

            if (st->objects[i].wearpos) {
                equip_char(ch, obj, st->objects[i].wearpos - 1);
            }
            if (tmp_cur_depth && !st->objects[i].wearpos) {
                obj_to_obj(obj, in_obj[tmp_cur_depth - 1]);
            } else if (st->objects[i].wearpos == 0) {
                obj_to_char(obj, ch);
            }
            last_obj = obj;
        }
    }
}

void load_char_objs(struct char_data *ch)
{
    FILE           *fl;
    bool            found = FALSE;
    float           timegold;
    struct obj_file_u st;
    char            tbuf[200];
    char            buf[MAX_STRING_LENGTH];

    /*
     * load in aliases and poofs first 
     */
    load_char_extra(ch);
    sprintf(tbuf, "rent/%s", lower(ch->player.name));

    /*
     * r+b is for Binary Reading/Writing 
     */
    if (!(fl = fopen(tbuf, "r+b"))) {
        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            Log("Char has no equipment");
        }
        return;
    }

    rewind(fl);
    if (!ReadObjs(fl, &st)) {
        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            Log("No objects found");
        }
        return;
    }

    if (str_cmp(st.owner, GET_NAME(ch)) != 0) {
        Log("Hmm.. bad item-file write. someone is losing their objects");
        fclose(fl);
        return;
    }

    /*
     * if the character has been out for 12 real hours, they are fully
     * healed upon re-entry.  if they stay out for 24 full hours, all
     * affects are removed, including bad ones. 
     */
    if (st.last_update + 12 * SECS_PER_REAL_HOUR < time(0)) {
        RestoreChar(ch);
    }
    if (st.last_update + 24 * SECS_PER_REAL_HOUR < time(0)) {
        RemAllAffects(ch);
    }
    if (ch->in_room == NOWHERE && 
        st.last_update + 1 * SECS_PER_REAL_HOUR > time(0)) {
        /*
         * you made it back from the crash in time, 1 hour grace period. 
         */
        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            Log("Character reconnecting.");
        }
        found = TRUE;
    } else {
        if (ch->in_room == NOWHERE &&
            (!IS_IMMORTAL(ch) || ch->invis_level <= 58)) {
            Log("Char reconnecting after autorent");
        }
#if NEW_RENT
        timegold = (int) ((100 * ((float) time(0) - st.last_update)) /
                   (SECS_PER_REAL_DAY));
#else
        timegold = (int) ((st.total_cost * ((float) time(0) - st.last_update)) /
                   (SECS_PER_REAL_DAY));
#endif

        if (!IS_IMMORTAL(ch) || ch->invis_level <= 58) {
            sprintf(buf, "Char ran up charges of %g gold in rent", timegold);
            Log(buf);
        }

        sprintf(buf, "You ran up charges of %g gold in rent.\n\r", timegold);
        send_to_char(buf, ch);
        GET_GOLD(ch) -= timegold;
        found = TRUE;

        if (GET_GOLD(ch) < 0) {
            if (GET_BANK(ch) + GET_GOLD(ch) < 0) {
                Log("** Char ran out of money in rent **");
                send_to_char("You ran out of money, you deadbeat.\n\r", ch);
                send_to_char("You don't even have enough in your bank "
                             "account!!\n\r", ch);
                GET_GOLD(ch) = 0;
                GET_BANK(ch) = 0;
                found = FALSE;
            } else {
                Log("** Char ran out of rent so lets dip into bank **");
                send_to_char("You ran out rent money.. lets dip into your bank"
                             " account.\n\r", ch);
                send_to_char("I'm going to have to charge you a little extra "
                             "for that though.\n\r", ch);

                GET_BANK(ch) = GET_BANK(ch) + GET_GOLD(ch) + 
                               0.05 * GET_GOLD(ch);

                if (GET_BANK(ch) < 0) {
                    GET_BANK(ch) = 0;
                }
                GET_GOLD(ch) = 0;
                found = TRUE;
            }
        }

        if (has_mail((char *) GET_NAME(ch))) {
            send_to_char("$c0013[$c0015The scribe$c0013] bespeaks you: 'You"
                         " have mail waiting!'", ch);
            ch->player.has_mail = TRUE;
        }
    }

    fclose(fl);

    if (found) {
        obj_store_to_char(ch, &st);
    } else {
        ZeroRent(GET_NAME(ch));
    }

    /*
     * Save char, to avoid strange data if crashing 
     */
    save_char(ch, AUTO_RENT);
}

int reimb_char_objs(struct char_data *ch)
{
    FILE           *fl;
    struct obj_file_u st;
    char            tbuf[200];
    int             i;

    /*
     * load in aliases and poofs first 
     */
#if 0    
    load_char_extra(ch);
#endif
    sprintf(tbuf, "reimb/%s", lower(ch->player.name));

    /*
     * r+b is for Binary Reading/Writing 
     */
    if (!(fl = fopen(tbuf, "r+b"))) {
        Log("Cannot find a reimb file for this character");
        return (FALSE);
    }

    rewind(fl);

    if (!ReadObjs(fl, &st)) {
        Log("This char's reimb file has been set to empty, sorry. Remind them"
            " not to RENT when needing a reimb");
        return (FALSE);
    }

    if (str_cmp(st.owner, GET_NAME(ch)) != 0) {
        Log("Incompatible names in char file and reimb file. Aborting");
        fclose(fl);
        return (FALSE);
    }

    /*
     * restore char, remove affects 
     */
    RestoreChar(ch);
    RemAllAffects(ch);

    if (ch->in_room == NOWHERE) {
        Log("Character in invalid room. Aborting");
        return (FALSE);
    }

    /*
     * restore gold 
     */
    GET_GOLD(ch) = st.gold_left;

    fclose(fl);

    /*
     * remove anything char is currently wearing/carrying 
     */
    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            extract_obj(unequip_char(ch, i));
        }
    }

    while (ch->carrying) {
        extract_obj(ch->carrying);
    }

    /*
     * restore items from st 
     */
    obj_store_to_char(ch, &st);

    /*
     * save char 
     */
    save_char(ch, AUTO_RENT);

    return (TRUE);
}

/*
 ************************************************************************
 * Routines used to save a characters equipment from disk                  *
 ************************************************************************* */

/*
 * Puts object in store, at first item which has no -1 
 */
void put_obj_in_store(struct obj_data *obj, struct obj_file_u *st)
{
    int             j;
    struct obj_file_elem *oe;
    char            buf[256];

    if (st->number >= MAX_OBJ_SAVE) {
        printf("you want to rent more than %d items?!\n", st->number);
        return;
    }

    oe = st->objects + st->number;

    oe->item_number = obj_index[obj->item_number].virtual;
    oe->value[0] = obj->obj_flags.value[0];
    oe->value[1] = obj->obj_flags.value[1];
    oe->value[2] = obj->obj_flags.value[2];
    oe->value[3] = obj->obj_flags.value[3];

    oe->extra_flags = obj->obj_flags.extra_flags;
    oe->weight = obj->obj_flags.weight;
    oe->timer = obj->obj_flags.timer;
    oe->bitvector = obj->obj_flags.bitvector;

    /*
     * new, saving names and descrips stuff 
     */
    if (obj->name) {
        strcpy(oe->name, obj->name);
    } else {
        sprintf(buf, "object %ld has no name!",
                obj_index[obj->item_number].virtual);
        Log(buf);
    }

    if (obj->short_description) {
        strcpy(oe->sd, obj->short_description);
    } else {
        *oe->sd = '\0';
    }
    if (obj->description) {
        strcpy(oe->desc, obj->description);
    } else {
        *oe->desc = '\0';
    }
    /*
     * end of new, possibly buggy stuff 
     */

    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        oe->affected[j] = obj->affected[j];
    }
    oe->depth = cur_depth;
    st->number++;
}

int contained_weight(struct obj_data *container)
{
    struct obj_data *tmp;
    int             rval = 0;

    for (tmp = container->contains; tmp; tmp = tmp->next_content) {
        rval += GET_OBJ_WEIGHT(tmp);
    }
    return rval;
}

/*
 * Destroy inventory after transferring it to "store inventory" 
 */
#if 1
void obj_to_store(struct obj_data *obj, struct obj_file_u *st,
                  struct char_data *ch, int delete)
{
    int             weight;

    if (!obj) {
        return;
    }

    if (obj->obj_flags.timer < 0 && obj->obj_flags.timer != OBJ_NOTIMER) {
#if NODUPLICATES
#else
        sprintf(buf, "You're told: '%s is just old junk, I'll throw it away "
                     "for you.'\n\r", obj->short_description);
        send_to_char(buf, ch);
#endif
    } else if (obj->obj_flags.cost_per_day < 0) {

#if NODUPLICATES
#else
        if (ch != '/0') {
            sprintf(buf, "You're told: '%s is just old junk, I'll throw it "
                         "away for you.'\n\r", obj->short_description);
            send_to_char(buf, ch);
        }
#endif
    } else if (obj->item_number != -1) {
        weight = contained_weight(obj);

        GET_OBJ_WEIGHT(obj) -= weight;
        put_obj_in_store(obj, st);
        GET_OBJ_WEIGHT(obj) += weight;
    }

    if (obj->contains) {
        cur_depth++;
        obj_to_store(obj->contains, st, ch, delete);
        cur_depth--;
    }

    obj_to_store(obj->next_content, st, ch, delete);

    /*
     * and now we can destroy object 
     */
    if (delete) {
        if (obj->in_obj) {
            obj_from_obj(obj);
        }
        if (IS_RARE(obj)) {
            obj_index[obj->item_number].number++;
        }
        extract_obj(obj);
    }
}

#else
void obj_to_store(struct obj_data *obj, struct obj_file_u *st,
                  struct char_data *ch, int delete)
{
    static char     buf[240];
    int             weight;

    if (!obj) {
        return;
    }
    obj_to_store(obj->contains, st, ch, delete);
    obj_to_store(obj->next_content, st, ch, delete);

    if (obj->obj_flags.timer < 0 && obj->obj_flags.timer != OBJ_NOTIMER) {
#if NODUPLICATES
#else
        sprintf(buf, "You're told: '%s is just old junk, I'll throw it away "
                     "for you.'\n\r", obj->short_description);
        send_to_char(buf, ch);
#endif
    } else if (obj->obj_flags.cost_per_day < 0) {

#if NODUPLICATES
#else
        if (ch != '\0') {
            sprintf(buf, "You're told: '%s is just old junk, I'll throw it "
                         "away for you.'\n\r", obj->short_description);
            send_to_char(buf, ch);
        }
#endif

        if (delete) {
            if (obj->in_obj) {
                obj_from_obj(obj);
            }
            extract_obj(obj);
        }
    } else if (obj->item_number == -1) {
        if (delete) {
            if (obj->in_obj) {
                obj_from_obj(obj);
            }
            extract_obj(obj);
        }
    } else {
        weight = contained_weight(obj);
        GET_OBJ_WEIGHT(obj) -= weight;
        put_obj_in_store(obj, st);
        GET_OBJ_WEIGHT(obj) += weight;

        if (delete) {
            if (obj->in_obj) {
                obj_from_obj(obj);
            }
            extract_obj(obj);
        }
    }
}

#endif

/*
 * write the vital data of a player to the player file 
 */
void save_obj(struct char_data *ch, struct obj_cost *cost, int delete)
{
    static struct obj_file_u st;
    int             i;
    char            buf[128];

    st.number = 0;
    st.gold_left = GET_GOLD(ch);

    sprintf(buf, "Saving %s:%d", fname(ch->player.name), GET_GOLD(ch));
    slog(buf);

    st.total_cost = cost->total_cost;
    st.last_update = time(0);
    st.minimum_stay = 0;        /* where does this belong? */

    cur_depth = 0;

    for (i = 0; i < MAX_OBJ_SAVE; i++) {
        st.objects[i].wearpos = 0;
        st.objects[i].depth = 0;
    }

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            if (ch->equipment[i]->obj_flags.cost_per_day != -1) {
                st.objects[st.number].wearpos = i + 1;
            }

            if (delete) {
                obj_to_store(unequip_char(ch, i), &st, ch, delete);
            } else {
                obj_to_store(ch->equipment[i], &st, ch, delete);
            }
        }
    }

    obj_to_store(ch->carrying, &st, ch, delete);
    if (delete) {
        ch->carrying = 0;
    }
    update_file(ch, &st);
}

void save_obj_reimb(struct char_data *ch)
{
    static struct obj_file_u st;
    int             i;
    char            buf[128];

    st.number = 0;
    st.gold_left = GET_GOLD(ch);

    sprintf(buf, "Saving reimb info %s:%d", fname(ch->player.name),
            GET_GOLD(ch));
    slog(buf);

    st.total_cost = 0;          
#if 0    
    cost->total_cost;
#endif    
    st.last_update = time(0);
    st.minimum_stay = 0;        
    /* 
     * where does this belong? 
     */

    cur_depth = 0;

    for (i = 0; i < MAX_OBJ_SAVE; i++) {
        st.objects[i].wearpos = 0;
        st.objects[i].depth = 0;
    }

    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            if (ch->equipment[i]->obj_flags.cost_per_day != -1) {
                st.objects[st.number].wearpos = i + 1;
            }
            obj_to_store(ch->equipment[i], &st, ch, 0);
        }
    }
    obj_to_store(ch->carrying, &st, ch, 0);
    update_reimb_file(ch, &st);
}

/*
 ************************************************************************
 * Routines used to update object file, upon boot time                     *
 ************************************************************************* */

void update_obj_file(void)
{
    FILE           *fl,
                   *char_file;
    struct obj_file_u st;
    struct char_file_u ch_st;
    long            i;
    long            days_passed,
                    secs_lost;
    char            buf[200];

    int             find_name(char *name);

    if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
        perror("Opening player file for reading. (reception.c, "
               "update_obj_file)");
        assert(0);
    }

    for (i = 0; i <= top_of_p_table; i++) {
        sprintf(buf, "rent/%s", lower(player_table[i].name));
        /*
         * r+b is for Binary Reading/Writing 
         */
        if ((fl = fopen(buf, "r+b")) != '\0' && ReadObjs(fl, &st)) {
            if (str_cmp(st.owner, player_table[i].name) != 0) {
                sprintf(buf, "Ack!  Wrong person written into object file!"
                             " (%s/%s)", st.owner, player_table[i].name);
                Log(buf);
                abort();
            } else {
                sprintf(buf, "   Processing %s[%ld].", st.owner, i);
                Log(buf);
                days_passed = (time(0) - st.last_update) / 
                              SECS_PER_REAL_DAY;
                secs_lost = (time(0) - st.last_update) % SECS_PER_REAL_DAY;

                rewind(char_file);
                fseek(char_file, (long) (player_table[i].nr *
                                         sizeof(struct char_file_u)), 0);
                fread(&ch_st, sizeof(struct char_file_u), 1, char_file);

                if (ch_st.load_room == AUTO_RENT) {
                    /* 
                     * this person was autorented 
                     */
                    ch_st.load_room = NOWHERE;

                    /* 
                     * one hour grace period 
                     */
                    st.last_update = time(0) + 3600;

                    sprintf(buf, "   Deautorenting %s", st.owner);
                    Log(buf);

#if LIMITED_ITEMS
                    CountLimitedItems(&st);
#endif
                    rewind(char_file);
                    fseek(char_file, (long) (player_table[i].nr *
                                             sizeof(struct char_file_u)),
                          0);
                    fwrite(&ch_st, sizeof(struct char_file_u), 1,
                           char_file);

                    rewind(fl);
                    WriteObjs(fl, &st);

                    fclose(fl);
                } else if (days_passed > 0) {
                    if ((st.total_cost * days_passed) > st.gold_left) {
                        sprintf(buf, "   Dumping %s from object file.",
                                ch_st.name);
                        Log(buf);

                        ch_st.points.gold = 0;
                        ch_st.load_room = NOWHERE;
                        rewind(char_file);
                        fseek(char_file, 
                              (long) (player_table[i].nr *
                                      sizeof(struct char_file_u)), 0);
                        fwrite(&ch_st, sizeof(struct char_file_u),
                               1, char_file);

                        fclose(fl);
                        ZeroRent(ch_st.name);
                    } else {
                        sprintf(buf, "   Updating %s", st.owner);
                        Log(buf);
                        st.gold_left -= (st.total_cost * days_passed);
                        st.last_update = time(0) - secs_lost;
                        rewind(fl);
                        WriteObjs(fl, &st);
                        fclose(fl);
#if LIMITED_ITEMS
                        CountLimitedItems(&st);
#endif
                    }
                } else {
#if LIMITED_ITEMS
                    CountLimitedItems(&st);
#endif
                    sprintf(buf, "  same day update on %s", st.owner);
                    Log(buf);
                    rewind(fl);
                    WriteObjs(fl, &st);
                    fclose(fl);
                }
            }
        }
    }
    fclose(char_file);
}

void CountLimitedItems(struct obj_file_u *st)
{
    int             i,
                    cost_per_day;
    struct obj_data *obj;

    if (!st->owner[0]) {
         /* 
          * don't count empty rent units 
          */
        return;
    }

    for (i = 0; i < st->number; i++) {
        if (st->objects[i].item_number > -1 &&
            real_object(st->objects[i].item_number) > -1) {
            /*
             * eek.. read in the object, and then extract it.
             * (all this just to find rent cost.)  *sigh*
             */
            obj = read_object(st->objects[i].item_number, VIRTUAL);
            cost_per_day = obj->obj_flags.cost_per_day;

            /*
             * if the cost is > LIM_ITEM_COST_MIN, 
             * then mark before extractin
             */
            if (IS_RARE(obj)) {
                if (obj->item_number < 0) {
                    abort();
                }
                obj_index[obj->item_number].number++;
            }
#if 0            
            obj_index[obj->item_number].number++;
#endif            
            extract_obj(obj);
        }
    }
}

void PrintLimitedItems(void)
{
    int             i;
    char            buf[200];

    for (i = 0; i <= top_of_objt; i++) {
        if (obj_index[i].number > 0) {
            sprintf(buf, "item> %ld [%d]", obj_index[i].virtual,
                    obj_index[i].number);
            Log(buf);
        }
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
 * removes a player from the list of renters 
 */

void zero_rent(struct char_data *ch)
{
    if (IS_NPC(ch)) {
        return;
    }
    ZeroRent(GET_NAME(ch));
}

void ZeroRent(char *n)
{
    FILE           *fl;
    char            buf[200];

    sprintf(buf, "rent/%s", lower(n));

    if (!(fl = fopen(buf, "w"))) {
        perror("saving PC's objects");
        assert(0);
    }

    fclose(fl);
    return;
}

int ReadObjs(FILE * fl, struct obj_file_u *st)
{
    int             i;

    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(st->owner, sizeof(st->owner), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->gold_left, sizeof(st->gold_left), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->total_cost, sizeof(st->total_cost), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->last_update, sizeof(st->last_update), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    fread(&st->number, sizeof(st->number), 1, fl);
    if (feof(fl)) {
        fclose(fl);
        return (FALSE);
    }

    for (i = 0; i < st->number; i++) {
        fread(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    }
    return( TRUE );
}

void WriteObjs(FILE * fl, struct obj_file_u *st)
{
    int             i;

    fwrite(st->owner, sizeof(st->owner), 1, fl);
    fwrite(&st->gold_left, sizeof(st->gold_left), 1, fl);
    fwrite(&st->total_cost, sizeof(st->total_cost), 1, fl);
    fwrite(&st->last_update, sizeof(st->last_update), 1, fl);
    fwrite(&st->minimum_stay, sizeof(st->minimum_stay), 1, fl);
    fwrite(&st->number, sizeof(st->number), 1, fl);

    for (i = 0; i < st->number; i++) {
        fwrite(&st->objects[i], sizeof(struct obj_file_elem), 1, fl);
    }
}

void load_char_extra(struct char_data *ch)
{
    FILE           *fp;
    char            buf[80];
    char            line[260];
    char            tmp[260];
    char            tmp2[256];
    char           *p,
                   *s,
                   *chk;
    int             n;

    sprintf(buf, "rent/%s.aux", GET_NAME(ch));

    /*
     * open the file.. read in the lines, use them as the aliases and
     * poofin and outs, depending on tags:
     * 
     * format:
     * 
     * <id>:string
     * 
     */

    if ((fp = fopen(buf, "r")) == '\0') {
        /* 
         * nothing to look at 
         */
        return;
    }

    while (!feof(fp)) {
        chk = fgets(line, 260, fp);

        if (chk) {
            p = (char *) strtok(line, ":");
            s = (char *) strtok(NULL, "\0");

            if( s ) {
                /* eat leading spaces and trailing carriage return/linefeed */
                for (; isspace(*s); s++) {
                    /*
                     * Empty loop
                     */
                }

                while(strchr(s, '\n')) {
                    *(strchr(s, '\n')) = '\0';
                }

                while(strchr(s, '\r')) {
                    *(strchr(s, '\r')) = '\0';
                }
            }

            if (p) {
                if (!strcmp(p, "out")) {
                    /* 
                     * setup bamfout 
                     */
                    do_bamfout(ch, s, 0);
                } else if (!strcmp(p, "in")) {
                    /* 
                     * setup bamfin 
                     */
                    do_bamfin(ch, s, 0);
                } else if (!strcmp(p, "zone")) {
                    /* 
                     * set zone permisions 
                     */
                    GET_ZONE(ch) = atoi(s);
                } else if (!strcmp(p, "bprompt")) {
                    /* 
                     * set upbattleprompt 
                     */
                    do_set_bprompt(ch, s, 0);
                } else if (!strcmp(p, "email")) {
                    /* 
                     * set up email finger info 
                     */
                    sprintf(tmp, "email %s", s);
                    do_set_flags(ch, tmp, 0);
                } else if (!strcmp(p, "clan")) {
                    /* 
                     * Clan info 
                     */
                    sprintf(tmp2, "clan %s", s);
                    do_set_flags(ch, tmp2, 0);
                } else if (!strcmp(p, "hostip")) {
                    /* 
                     * hostIP 
                     */
                    ch->specials.hostip = strdup(s);
                } else if (!strcmp(p, "rumored")) {
                    /* 
                     * Clan info 
                     */
#if 0                    
                    ch->specials.rumor = s;
                    strdup(s);
#endif                
                } else
                 if (!strcmp(p, "setsev")) {
                     /* 
                      * setup severity level 
                      */
                    do_setsev(ch, s, 0);
                } else
                    if (!strcmp(p, "invislev") && GetMaxLevel(ch) > MAX_MORT) {
                    /* 
                     * setup wizinvis level 
                     */
                    do_invis(ch, s, 242);       
                } else if (!strcmp(p, "prompt")) {
                    /* 
                     * setup prompt 
                     */
                    do_set_prompt(ch, s, 0);
                } else if (s) {
                    n = atoi(p);

                    if (n >= 0 && n <= 9) { 
                        /* 
                         * set up alias 
                         */
                        sprintf(tmp, "%d %s", n, s);
                        do_alias(ch, tmp, 260);
                    }
                }
            }
        }
    }
    fclose(fp);
}

void write_char_extra(struct char_data *ch)
{
    FILE           *fp;
    char            buf[80];
    int             i;

    sprintf(buf, "rent/%s.aux", GET_NAME(ch));

    /*
     * open the file.. read in the lines, use them as the aliases and
     * poofin and outs, depending on tags:
     * 
     * format:
     * 
     * <id>:string
     * 
     */

    if ((fp = fopen(buf, "w")) == NULL) {
        return;                 
        /* 
         * nothing to write 
         */
    }

    if (IS_IMMORTAL(ch)) {
        if (ch->specials.poofin) {
            fprintf(fp, "in: %s\n", ch->specials.poofin);
        }
        if (ch->specials.poofout) {
            fprintf(fp, "out: %s\n", ch->specials.poofout);
        }

        if (ch->specials.sev) {
            fprintf(fp, "setsev: %d\n", ch->specials.sev);
        }
        if (ch->invis_level) {
            fprintf(fp, "invislev: %d\n", ch->invis_level);
        }
        fprintf(fp, "zone: %d\n", GET_ZONE(ch));
    }
    if (ch->specials.prompt) {
        fprintf(fp, "prompt: %s\n", ch->specials.prompt);
    }
    if (ch->specials.email) {
        fprintf(fp, "email: %s\n", ch->specials.email);
    }
    if (ch->specials.bprompt) {
        fprintf(fp, "bprompt: %s\n", ch->specials.bprompt);
    }

    if (ch->specials.clan) {
        fprintf(fp, "clan: %s\n", ch->specials.clan);
    }
    if (ch->specials.hostip) {
        fprintf(fp, "hostip: %s\n", ch->specials.hostip);
    }

    if (ch->specials.rumor) {
#if 0        
        fprintf(fp, "rumor: %s\n",ch->specials.rumor);
#endif    
    }

    if (ch->specials.A_list) {
        for (i = 0; i < 10; i++) {
            if (GET_ALIAS(ch, i)) {
                fprintf(fp, "%d: %s\n", i, GET_ALIAS(ch, i));
            }
        }
    }
    fclose(fp);
}

void obj_store_to_room(int room, struct obj_file_u *st)
{
    struct obj_data *obj;
    int             i,
                    j;

    for (i = 0; i < st->number; i++) {
        if (st->objects[i].item_number > -1 &&
            real_object(st->objects[i].item_number) > -1) {

            obj = read_object(st->objects[i].item_number, VIRTUAL);
            if (IS_RARE(obj)) {
                obj_index[obj->item_number].number--;
            }
            obj->obj_flags.value[0] = st->objects[i].value[0];
            obj->obj_flags.value[1] = st->objects[i].value[1];
            obj->obj_flags.value[2] = st->objects[i].value[2];
            obj->obj_flags.value[3] = st->objects[i].value[3];
            obj->obj_flags.extra_flags = st->objects[i].extra_flags;
            obj->obj_flags.weight = st->objects[i].weight;
            obj->obj_flags.timer = st->objects[i].timer;
            obj->obj_flags.bitvector = st->objects[i].bitvector;

            /*
             * new, saving names and descrips stuff o_s_t_r 
             */
            if (obj->name) {
                free(obj->name);
            }
            if (obj->short_description) {
                free(obj->short_description);
            }
            if (obj->description) {
                free(obj->description);
            }
            obj->name = (char *) malloc(strlen(st->objects[i].name) + 1);
            obj->short_description =
                (char *) malloc(strlen(st->objects[i].sd) + 1);
            obj->description =
                (char *) malloc(strlen(st->objects[i].desc) + 1);

            strcpy(obj->name, st->objects[i].name);
            strcpy(obj->short_description, st->objects[i].sd);
            strcpy(obj->description, st->objects[i].desc);
            /*
             * end of new, possibly buggy stuff 
             */

            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                obj->affected[j] = st->objects[i].affected[j];
            }
            obj_to_room2(obj, room);
        }
    }
}

void load_room_objs(int room)
{
    FILE           *fl;
    struct obj_file_u st;
    char            buf[200];

    sprintf(buf, "world/%d", room);

    /*
     * r+b is for Binary Reading/Writing 
     */
    if (!(fl = fopen(buf, "r+b"))) {
        Log("Room has no equipment");
        return;
    }

    rewind(fl);

    if (!ReadObjs(fl, &st)) {
        Log("No objects found");
        fclose(fl);
        return;
    }

    fclose(fl);

    obj_store_to_room(room, &st);
    save_room(room);
}

void save_room(int room)
{

#ifdef SAVEWORLD

    struct obj_file_u st;
    struct obj_data *obj;
    struct room_data *rm = 0;
    char            buf[255];
    static int      last_room = -1;
    static FILE    *f1 = 0;

    rm = real_roomp(room);

    obj = rm->contents;
    sprintf(buf, "world/%d", room);
    st.number = 0;

    if (obj) {
        if (room != last_room) {
            if (f1) {
                fclose(f1);
            }
            f1 = fopen(buf, "w");
        }

        if (!f1) {
            return;
        }
        rewind(f1);
        obj_to_store(obj, &st, NULL, 0);
        sprintf(buf, "Room %d", room);
        strcpy(st.owner, buf);
        st.gold_left = 0;
        st.total_cost = 0;
        st.last_update = 0;
        st.minimum_stay = 0;
        WriteObjs(f1, &st);
    }
#endif
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
