#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

/*
 * external vars 
 */

struct riddle_answer {
    char   *answer;
    int     reward;
    char   *rewardText;
};
    
int mazekeeper_riddle_common(struct char_data *ch, char *arg,
                             struct char_data *mob, struct riddle_answer *rid,
                             int ridCount, int exp, int portal);

void            printmap(struct char_data *ch, int x, int y, int sizex,
                         int sizey);
struct obj_data *SailDirection(struct obj_data *obj, int direction);
int             CanSail(struct obj_data *obj, int direction);
int ReadObjs(FILE * fl, struct obj_file_u *st);

/*
 * two global integers for Sentinel's cog room procedure 
 */
extern int      cog_sequence;
int             chest_pointer = 0;

#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))
extern struct obj_data *object_list;



int vampiric_embrace(struct char_data *ch, struct char_data *vict)
{
    struct obj_data *obj;
    int             dam;

    if (IsImmune(vict, IMM_DRAIN)) {
        return (FALSE);
    }
    if (ch->equipment[WIELD]) {
        obj = ch->equipment[WIELD];
        act("$c0008The negative aura surrounding your $p lashes out at $N, "
            "draining some of $S life.", FALSE, ch, obj, vict, TO_CHAR);
        act("$c0008The negative aura surrounding $n's $p lashes out at $N, "
            "draining some of $S life.", FALSE, ch, obj, vict, TO_NOTVICT);
        act("$c0008The negative aura surrounding $n's $p lashes out at you, "
            "draining some of your life.", FALSE, ch, obj, vict, TO_VICT);
    } else {
        act("$c0008The negative aura surrounding your hands lashes out at $N, "
            "draining some of $S life.", FALSE, ch, 0, vict, TO_CHAR);
        act("$c0008The negative aura surrounding $n's hands lashes out at $N, "
            "draining some of $S life.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("$c0008The negative aura surrounding $n's hands lashes out at you, "
            "draining some of your life.", FALSE, ch, 0, vict, TO_VICT);
    }
    dam = dice(3, 8);
    if (IsResist(vict, IMM_DRAIN)) {     
        /* 
         * half damage for resist 
         */
        dam >>= 1;
    }
    GET_HIT(ch) += dam;
    GET_HIT(vict) -= dam;
    return (FALSE);
}

/*
 * procs for the King's Grove 
 */
#define LEGEND_STATUE 52851
#define LEGEND_PAINTING 52852
#define LEGEND_BIOGRAPHY 52853
#if 0 
struct char_data *ch, char *argument, int cmd)
#endif
int generate_legend_statue(void)
{
    struct obj_data *obj;
    struct char_data *tmp;
    struct char_file_u player;
    struct extra_descr_data *ed;
    char            name[254],
                    shdesc[254],
                    desc[254],
                    exdesc[500];
    int             i = 0,
                    itype = 0,
                    rnum = 0;
    extern int      top_of_p_table;
    extern struct player_index_element *player_table;

    /*
     * Determine number of pfiles. Add one for last player made, though
     * that one isn't very likely to have enough kills. Still, we wanna be 
     * thorough. 
     */
    for (i = 0; i < top_of_p_table + 1; i++) {
        /*
         * load up each of them 
         */
        if (load_char((player_table + i)->name, &player) > -1) {
            /*
             * store to a tmp char that we can deal with 
             */
            CREATE(tmp, struct char_data, 1);
            clear_char(tmp);
            store_to_char(&player, tmp);
            /*
             * are they entitled to an item? 
             */
            if (tmp->specials.m_kills >= 10000) {
                /*
                 * coolness, the are! Determine the item. 
                 */
                if (tmp->specials.m_kills >= 40000) {
                    itype = LEGEND_BIOGRAPHY;
                    rnum = number(52892, 52895);
                    sprintf(name, "biography tome %s", GET_NAME(tmp));
                    sprintf(shdesc, "a biography of %s", GET_NAME(tmp));
                    sprintf(desc, "A large tome lies here, titled "
                                  "'The Biography of %s'.", GET_NAME(tmp));
                    sprintf(exdesc, "This book is a treatise on the life and "
                                    "accomplishments of %s.\n\rIt is an "
                                    "extensive volume, detailing many a feat. "
                                    "Most impressive.", GET_NAME(tmp));
                } else if (tmp->specials.m_kills >= 20000) {
                    itype = LEGEND_PAINTING;
                    rnum = number(52886, 52891);
                    sprintf(name, "painting %s", GET_NAME(tmp));
                    sprintf(shdesc, "a painting of %s", GET_NAME(tmp));
                    sprintf(desc, "On the wall, one can admire a painting of "
                                  "%s, slaying a fearsome beast.", 
                                  GET_NAME(tmp));
                    sprintf(exdesc, "%s is in the process of slaying a fearsome"
                                    " beast.\n\rTruly, %s is one of the "
                                    "greatest of these times.",
                            GET_NAME(tmp), GET_NAME(tmp));
                } else {
                    itype = LEGEND_STATUE;
                    rnum = number(52861, 52884);
                    sprintf(name, "statue %s", GET_NAME(tmp));
                    sprintf(shdesc, "a statue of %s", GET_NAME(tmp));
                    sprintf(desc, "A statue of the legendary %s has been "
                                  "erected here.", GET_NAME(tmp));
                    sprintf(exdesc, "This is a statue of %s, the legendary "
                                    "slayer.", GET_NAME(tmp));
                }
                if (itype == 0) {
                    Log("Oddness in statue generation, no type found");
                    return (TRUE);
                }
                if (rnum == 0) {
                    Log("Oddness in statue generation, no rnum found");
                    return (TRUE);
                }

                /*
                 * load the generic item 
                 */
                if ((obj = read_object(itype, VIRTUAL))) {
                    /*
                     * and string it up a bit 
                     */
                    if (obj->short_description) {
                        free(obj->short_description);
                        obj->short_description = strdup(shdesc);
                    }

                    if (obj->description) {
                        free(obj->description);
                        obj->description = strdup(desc);
                    }

                    if (obj->name) {
                        free(obj->name);
                        obj->name = strdup(name);
                    }

                    if (obj->ex_description) {
                        Log("trying to string invalid item in statue "
                            "generation");
                        return (TRUE);
                    } else {
                        /*
                         * create an extra desc structure for the object 
                         */
                        CREATE(ed, struct extra_descr_data, 1);
                        ed->next = obj->ex_description;
                        obj->ex_description = ed;

                        /*
                         * define the keywords 
                         */
                        CREATE(ed->keyword, char, strlen(name) + 1);
                        strcpy(ed->keyword, name);

                        /*
                         * define the description 
                         */
                        CREATE(ed->description, char, strlen(exdesc) + 1);
                        strcpy(ed->description, exdesc);
                    }

                    /*
                     * and finally place it in a room 
                     */
                    obj_to_room(obj, rnum);
                }
            }
            free(tmp);
        } else {
            Log("screw up bigtime in load_char");
            return (TRUE);
        }
    }

    Log("processed %d pfiles for legend statue check", top_of_p_table + 1);
    return( TRUE );
}





#define SHARPENING_STONE 52887
void do_sharpen(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj,
                   *cmp,
                   *stone;
    char            buf[254];
    char           *arg;
    int             w_type = 0;

    if (!ch || !cmd || cmd != 602) {
        /* 
         * sharpen 
         */
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("In the middle of a fight?! Hah.\n\r", ch);
        return;
    }

    if (ch->equipment && 
        (!(stone = ch->equipment[HOLD]) || 
         obj_index[stone->item_number].virtual != SHARPENING_STONE)) {
        send_to_char("How can you sharpen stuff if you're not holding a "
                     "sharpening stone?\n\r", ch);
        return;
    }

    /*
     * is holding the stone 
     */
    if (!argument) {
        send_to_char("Sharpen what?\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Sharpen what?\n\r", ch);
        return;
    }

    if ((obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        if ((ITEM_TYPE(obj) == ITEM_WEAPON)) {
            /*
             * can only sharpen edged weapons 
             */
            switch (obj->obj_flags.value[3]) {
            case 0:
                w_type = TYPE_SMITE;
                break;
            case 1:
                w_type = TYPE_STAB;
                break;
            case 2:
                w_type = TYPE_WHIP;
                break;
            case 3:
                w_type = TYPE_SLASH;
                break;
            case 4:
                w_type = TYPE_SMASH;
                break;
            case 5:
                w_type = TYPE_CLEAVE;
                break;
            case 6:
                w_type = TYPE_CRUSH;
                break;
            case 7:
                w_type = TYPE_BLUDGEON;
                break;
            case 8:
                w_type = TYPE_CLAW;
                break;
            case 9:
                w_type = TYPE_BITE;
                break;
            case 10:
                w_type = TYPE_STING;
                break;
            case 11:
                w_type = TYPE_PIERCE;
                break;
            case 12:
                w_type = TYPE_BLAST;
                break;
            case 13:
                w_type = TYPE_IMPALE;
                break;
            case 14:
                w_type = TYPE_RANGE_WEAPON;
                break;
            default:
                w_type = TYPE_HIT;
                break;
            }

            if ((w_type >= TYPE_PIERCE && w_type <= TYPE_STING) || 
                (w_type >= TYPE_CLEAVE && w_type <= TYPE_STAB) || 
                w_type == TYPE_IMPALE) {

                if (obj->obj_flags.value[2] == 0) {
                    Log("%s tried to sharpen a weapon with invalid value: %s, "
                        "vnum %d.", GET_NAME(ch), obj->short_description,
                        obj->item_number);
                    return;
                }

                if (!(cmp = read_object(obj->item_number, REAL))) {
                    Log("Could not load comparison weapon in do_sharpen");
                    return;
                }

                if (cmp->obj_flags.value[2] == 0) {
                    Log("%s tried to sharpen a weapon with invalid value: %s, "
                        "vnum %d.", GET_NAME(ch), obj->short_description,
                        obj->item_number);
                    extract_obj(cmp);
                    return;
                }

                if (cmp->obj_flags.value[2] == obj->obj_flags.value[2]) {
                    send_to_char("That item has no need of your attention.\n\r",
                                 ch);
                    extract_obj(cmp);
                    return;
                } 
                
                obj->obj_flags.value[2] = cmp->obj_flags.value[2];
                if (GET_POS(ch) > POSITION_RESTING) {
                    do_rest(ch, NULL, -1);
                }
                sprintf(buf, "%s diligently starts to sharpen %s.",
                        GET_NAME(ch), obj->short_description);
                act(buf, FALSE, ch, 0, 0, TO_ROOM);

                sprintf(buf, "You diligently sharpen %s.\n\r",
                        obj->short_description);
                send_to_char(buf, ch);

                extract_obj(cmp);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            } else {
                send_to_char("You can only sharpen edged or pointy "
                             "weapons.\n\r", ch);
            }
        } else {
            send_to_char("You can only sharpen weapons.\n\r", ch);
        }
    } else {
        send_to_char("You don't seem to have that.\n\r", ch);
    }
}



/*
 * start shopkeeper .. this to make shops easier to build -Lennya 20030731
 */
int shopkeeper(struct char_data *ch, int cmd, char *arg,
               struct char_data *shopkeep, int type)
{
    struct room_data *rp;
    struct obj_data *obj = NULL,
                   *cond_ptr[50],
                   *store_obj = NULL;
    char           *itemname,
                    newarg[100];
    float           modifier = 1.0;
    int             cost = 0,
                    chr = 1,
                    k,
                    i = 0,
                    stop = 0,
                    num = 1,
                    rnum = 0;
    int             tot_cost = 0,
                    cond_top = 0,
                    cond_tot[50],
                    found = FALSE;

    extern struct str_app_type str_app[];

    if (!ch) {
        return (FALSE);
    }
    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /*
     * define the shopkeep 
     */
    if (ch->in_room) {
        rp = real_roomp(ch->in_room);
    } else {
        Log("weirdness in shopkeeper, char not in a room");
        return (FALSE);
    }

    if (!rp) {
        Log("weirdness in shopkeeper, char's room does not exist");
        return (FALSE);
    }

    /*
     * let's make sure shopkeepers don't get killed or robbed 
     */
    if (!IS_SET(rp->room_flags, PEACEFUL)) {
        SET_BIT(rp->room_flags, PEACEFUL);
    }
    if (cmd != 59 && cmd != 56 && cmd != 93 && cmd != 57) {
        /* 
         * list  buy  offer  sell 
         */
        return (FALSE);
    }
    shopkeep = FindMobInRoomWithFunction(ch->in_room, shopkeeper);

    if (!shopkeep) {
        Log("weirdness in shopkeeper, shopkeeper assigned but not found");
        return (FALSE);
    }

    if (!IS_NPC(shopkeep)) {
        Log("weirdness in shopkeeper, shopkeeper is not a mob");
        return (FALSE);
    }

    if (!IS_SET(shopkeep->specials.act, ACT_SENTINEL)) {
        SET_BIT(shopkeep->specials.act, ACT_SENTINEL);
    }
    /*
     * players with 14 chr pay avg price 
     */
    chr = GET_CHR(ch);
    if (chr < 1) {
        chr = 1;
    }
    modifier = (float) 14 / chr;

    /*
     * list 
     */
    switch (cmd) {
    case 59:
        oldSendOutput(ch, "This is what %s currently has on store:\n\r\n\r",
                  shopkeep->player.short_descr);
        send_to_char("  Count  Item                                       "
                     "Price\n\r", ch);
        send_to_char("$c0008*---------------------------------------------"
                     "------------*\n\r", ch);
        obj = shopkeep->carrying;
        if (!obj) {
            send_to_char("$c0008|$c0007        Nothing.                    "
                         "                     $c0008|\n\r", ch);
            send_to_char("$c0008*------------------------------------------"
                         "---------------*\n\r", ch);
            break;
        }
        
        for (obj = shopkeep->carrying; obj; obj = obj->next_content) {
            if (CAN_SEE_OBJ(ch, obj)) {
                if (cond_top < 50) {
                    found = FALSE;
                    for (k = 0; (k < cond_top && !found); k++) {
                        if (cond_top > 0 &&
                            obj->item_number == cond_ptr[k]->item_number &&
                            obj->short_description && 
                            cond_ptr[k]->short_description && 
                            !strcmp(obj->short_description, 
                                    cond_ptr[k]->short_description)) {
                            cond_tot[k] += 1;
                            found = TRUE;
                        }
                    }

                    if (!found) {
                        cond_ptr[cond_top] = obj;
                        cond_tot[cond_top] = 1;
                        cond_top += 1;
                    }
                } else {
                    cost = (int) obj->obj_flags.cost * modifier;
                    if (cost < 0) {
                        cost = 0;
                    }
                    cost += 1000;   /* Trader's fee = 1000 */
                    oldSendOutput(ch, "$c0008|$c0007    1   %-41s %6d "
                                  "$c0008|\n\r", obj->short_description,
                              cost);
                }
            }
        }

        if (cond_top) {
            for (k = 0; k < cond_top; k++) {
                cost = (int) cond_ptr[k]->obj_flags.cost * modifier;
                if (cost < 0) {
                    cost = 0;
                }
                cost += 1000;   /* Trader's fee = 1000 */
                oldSendOutput(ch, "$c0008|$c0007 %4d   %-41s %6d $c0008|\n\r",
                          (cond_tot[k] > 1 ?  cond_tot[k] : 1),
                          cond_ptr[k]->short_description, cost);
            }
        }
        send_to_char("$c0008*------------------------------------------------"
                     "---------*\n\r", ch);
        break;
    case 56:
        /*
         * buy 
         */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            send_to_char("Buy what?\n\r", ch);
            return (TRUE);
        } 
        
        if ((num = getabunch(itemname, newarg)) != FALSE) {
            strcpy(itemname, newarg);
        }

        if (num < 1) {
            num = 1;
        }
        rnum = 0;
        stop = 0;
        i = 1;

        while (i <= num && stop == 0) {
            if ((obj = get_obj_in_list_vis(ch, itemname, shopkeep->carrying))) {
                cost = (int) obj->obj_flags.cost * modifier;
                if (cost < 0) {
                    cost = 0;
                }
                cost += 1000;       /* Trader's Fee is 1000 */

                if (GET_GOLD(ch) < cost) {
                    send_to_char("Alas, you cannot afford that.\n\r", ch);
                    stop = 1;
                } else if ((IS_CARRYING_N(ch) + 1) > (CAN_CARRY_N(ch))) {
                    oldSendOutput(ch, "%s : You can't carry that many items.\n\r",
                              obj->short_description);
                    stop = 1;
                } else
                    if ((IS_CARRYING_W(ch) + (obj->obj_flags.weight)) >
                        CAN_CARRY_W(ch)) {
                    oldSendOutput(ch, "%s : You can't carry that much weight.\n\r",
                              obj->short_description);
                    stop = 1;
                } else if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
                           anti_barbarian_stuff(obj) && 
                           !IS_IMMORTAL(ch)) {
                    send_to_char("You sense magic on the object and think "
                                 "better of buying it.\n\r", ch);
                    stop = 1;
                } else {
                    obj_from_char(obj);
                    obj_to_char(obj, ch);
                    GET_GOLD(ch) -= cost;
                    GET_GOLD(shopkeep) += cost;
                    store_obj = obj;
                    i++;
                    tot_cost += cost;
                    rnum++;
                }
            } else if (rnum > 0) {
                oldSendOutput(ch, "Alas, %s only seems to have %d %ss on "
                              "store.\n\r",
                          shopkeep->player.short_descr, rnum, itemname);
                stop = 1;
            } else {
                oldSendOutput(ch, "Alas, %s doesn't seem to stock any %ss..\n\r",
                          shopkeep->player.short_descr, itemname);
                stop = 1;

            }
        }

        if (rnum == 1) {
            oldSendOutput(ch, "You just bought %s for %d coins.\n\r",
                      store_obj->short_description, cost);
            act("$n buys $p from $N.", FALSE, ch, obj, shopkeep, TO_ROOM);
        } else if (rnum > 1) {
            oldSendOutput(ch, "You just bought %d items for %d coins.\n\r",
                      rnum, tot_cost);
            act("$n buys some stuff from $N.", FALSE, ch, obj, shopkeep,
                TO_ROOM);
        }
        break;
    case 57:
    /*
     * sell 
     */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            send_to_char("Sell what?\n\r", ch);
            return (TRUE);
        }
        
        if ((obj = get_obj_in_list_vis(ch, itemname, ch->carrying))) {
            cost = (int) obj->obj_flags.cost / (3 * modifier);
            /*
             * lets not have shops buying non-rentables
             */
            if (obj->obj_flags.cost_per_day == -1) {
                oldSendOutput(ch, "%s doesn't buy items that cannot be rented.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }

            if (cost < 400) {
                oldSendOutput(ch, "%s doesn't buy worthless junk like that.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }
            
            if (GET_GOLD(shopkeep) < cost) {
                oldSendOutput(ch, "Alas, %s cannot afford that right now.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }
            
            obj_from_char(obj);
            obj_to_char(obj, shopkeep);
            oldSendOutput(ch, "You just sold %s for %d coins.\n\r",
                      obj->short_description, cost);
            act("$n sells $p to $N.", FALSE, ch, obj, shopkeep, TO_ROOM);
            GET_GOLD(ch) += cost;
            GET_GOLD(shopkeep) -= cost;
            return (TRUE);
        }
        
        oldSendOutput(ch, "Alas, you don't seem to have the %s to sell.\n\r",
                  itemname);
        break;
    case 93:
    /*
     * offer 
     */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            oldSendOutput(ch, "What would you like to offer to %s?\n\r",
                      shopkeep->player.short_descr);
            return (TRUE);
        }
        
        if ((obj = get_obj_in_list_vis(ch, itemname, ch->carrying))) {
            cost = (int) obj->obj_flags.cost / (3 * modifier);
            if (cost < 400) {
                oldSendOutput(ch, "%s doesn't buy worthless junk like that.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }
            
            oldSendOutput(ch, "%s is willing to pay you %d coins for %s.\n\r",
                      shopkeep->player.short_descr, cost,
                      obj->short_description);
            return (TRUE);
        }
        
        oldSendOutput(ch, "You don't seem to have any %ss.\n\r", itemname);
        break;
    default:
        return (FALSE);
    }
    return (TRUE);
}





int troll_regen(struct char_data *ch)
{
    assert(ch);

    if (GET_HIT(ch) >= GET_MAX_HIT(ch)) {
        return (FALSE);
    }
    if (number(0, 2)) {
        return( FALSE );
    }
    GET_HIT(ch) += number(1, 3);
    if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
        GET_HIT(ch) = GET_MAX_HIT(ch);
    }
    act("$n's wounds seem to close of their own.", FALSE, ch, 0, 0, TO_ROOM);
    act("Your wounds close of their own accord.", FALSE, ch, 0, 0, TO_CHAR);
    return( TRUE );
}

int disembark_ship(struct char_data *ch, int cmd, char *argument,
                   struct obj_data *obj, int type)
{
    int             x = 0;

    if (cmd == 621) {
        /* 
         * disembark 
         */

        if (oceanmap[GET_XCOORD(obj)][GET_YCOORD(obj)] == '@') {
            if (ch->specials.fighting) {
                send_to_char("You can't while your fighting!", ch);
                return (TRUE);
            }

            /*
             * lets find out where they are going 
             */
            while (map_coords[x].x != -1) {
                if (map_coords[x].x == GET_XCOORD(obj) &&
                    map_coords[x].x == GET_YCOORD(obj)) {
                    char_from_room(ch);
                    char_to_room(ch, map_coords[x].room);
                    do_look(ch, NULL, 0);
                    return (TRUE);
                }

                x++;
            }
            send_to_char("Ahh.. maybe we don't have a place to disembark!\n\r",
                         ch);
        } else {
            send_to_char("There is no place around to disembark your ship!\n\r",
                         ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int steer_ship(struct char_data *ch, int cmd, char *argument,
               struct obj_data *obj, int type)
{
    static char    *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down"
    };
    int             keyword_no = 0;
    char           *buf;

    argument = get_argument(argument, &buf);
    if (!buf) {
        /* 
         * No arguments?? so which direction anyway? 
         */
        send_to_char("Sail in which direction?", ch);
        return (TRUE);
    }

    keyword_no = search_block(buf, keywords, FALSE);

    if ((keyword_no == -1)) {
        send_to_char("Sail in which direction?", ch);
        return (TRUE);
    }

    if (!CanSail(obj, keyword_no)) {
        send_to_char("You can't sail that way, you'd be bound to sink the "
                     "ship!!\n\r", ch);
        return (TRUE);
    }

    oldSendOutput(ch, "You sail %sward.\n\r", keywords[keyword_no]);

    switch (keyword_no) {
    case 0:
        /* 
         * North
         */
        GET_XCOORD(obj)--;
        break;
    case 1:
        /*
         * East
         */
        GET_YCOORD(obj)++;
        break;
    case 2:
        /*
         * south
         */
        GET_XCOORD(obj)++;
        break;
    case 3:
        /* 
         * west
         */
        GET_YCOORD(obj)--;
        break;
    default:
        break;
    }

    printmap(ch, GET_XCOORD(obj), GET_YCOORD(obj), 5, 10);
    return (TRUE);
}


/*
 * can they sail in that directioN?? 
 */
int CanSail(struct obj_data *obj, int direction)
{
    int             x = 0,
                    y = 0;

    x = GET_XCOORD(obj);
    y = GET_YCOORD(obj);

    switch (direction) {
    case 0:
        /* 
         * North
         */
        x--;
        break;
    case 1:
        /* 
         * East
         */
        y++;
        break;
    case 2:
        /* 
         * south
         */
        x++;
        break;
    case 3:
        /* 
         * west
         */
        y--;
        break;
    default:
        break;
    }

    if (oceanmap[x][y] == '~') {
        return (TRUE);
    } else if (oceanmap[x][y] == '@') {
        /* 
         * Entered port city.. lets place a ship in the board city
         */
        return (TRUE);
    } else if (oceanmap[x][y] == ':') {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*
 * lets print the map to the screen 
 */
void printmap(struct char_data *ch, int x, int y, int sizex, int sizey)
{
    int             loop = 0;
    void            printColors(struct char_data *ch, char *buf);
    char            buf[256];
    char            buf2[256];
#if 0
    printf("Displaying map at coord X%d-Y%d with display size of "
           "%d by %d.\n\r\n\r",x,y,sizex, sizey);
#endif
    oldSendOutput(ch, "Coords: %d-%d.\n\r", x, y);
    sprintf(buf, "\n\r$c000B]$c000W");

    for (loop = 0; loop < sizey * 2 + 1; loop++) {
        sprintf(buf, "%s=", buf);
    }

    sprintf(buf, "%s$c000B[$c000w\n\r", buf);
    send_to_char(buf, ch);

    for (loop = 0; loop < sizex * 2 + 1; loop++) {
        /* 
         * move that row of the ocean map into buf 
         */
        sprintf(buf, "%s", oceanmap[x - sizex + loop]);

        if (loop == sizex) {
            buf[y] = 'X';
        }
        /* 
         * mark of the end of where they should see 
         */
        buf[y + sizey + 1] = '\0';
        /* 
         * move to the start of where they should see on that row 
         * Print that mofo out 
         */
        sprintf(buf2, "|%s|\n\r", &buf[y - sizey]);
#if 0
        send_to_char(buf,ch);
        printmapcolors(buf);
#endif
        printColors(ch, buf2);
    }

    sprintf(buf, "$c000B]$c000W");
    for (loop = 0; loop < sizey * 2 + 1; loop++) {
        sprintf(buf, "%s=", buf);
    }

    sprintf(buf, "%s$c000B[$c000w", buf);
    send_to_char(buf, ch);
#if 0
    printColors(ch, buf);
#endif
}

/*
 * Lets go through and see what terrain needs what color 
 */
void printColors(struct char_data *ch, char *buf)
{
    int             x = 0;
#if 0
    int last=0;
#endif
    char            buffer[2048];
    char            last = ' ';

    buffer[0] = '\0';

    while (buf[x] != '\0') {
        switch (buf[x]) {
        case '~':
            if (last == '~') {
                strcat(buffer, "~");
            } else {
                strcat(buffer, "$c000b~");
                last = '~';
            }
            break;
        case '+':
            if (last == '+') {
                strcat(buffer, "+");
            } else {
                strcat(buffer, "$c000G+");
                last = '+';
            }
            break;
        case '^':
            if (last == '^') {
                strcat(buffer, "^");
            } else {
                strcat(buffer, "$c000Y^");
                last = '^';
            }
            break;
        case '.':
            if (last == '.') {
                strcat(buffer, ".");
            } else {
                strcat(buffer, "$c000y.");
                last = '.';
            }
            break;
        default:
            if (last == buf[x]) {
                sprintf(buffer, "%s%c", buffer, buf[x]);
            } else {
                sprintf(buffer, "%s$c000w%c", buffer, buf[x]);
                last = buf[x];
            }
            break;
        }
        x++;
    }

    send_to_char(buffer, ch);
}

int embark_ship(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             j;
    char           *buf;
    struct char_data *ship;

    if (cmd != 620) {
        /* 
         * board ship 
         */
        return (FALSE);  
    }

    arg = get_argument(arg, &buf);
    if (buf && !strcasecmp("ship", buf) &&
        (ship = get_char_room("", ch->in_room))) {
        j = mob_index[ship->nr].virtual;

        send_to_char("You enter the ship.\n\r", ch);
        act("$n enters the ship.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, j);

        act("Walks onto the ship.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }

    return (FALSE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
