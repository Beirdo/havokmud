
/************************************************************************
 *  Usage : Informative commands.                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 *************************************************************************
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"
#include "version.h"
#include "externs.h"

int             GET_RADIUS(struct char_data *ch);
int             MobLevBonus(struct char_data *ch);
void            generate_map(struct char_data *ch, int size, int x, int y);
void            print_map(struct char_data *ch);
void            recurse_map(struct room_data *rp, int size, int x, int y);
struct time_info_data real_time_passed(time_t t2, time_t t1);

void show_who_immortal(struct char_data *ch, struct char_data *person, 
                       char type);
void show_who_mortal(struct char_data *ch, struct char_data *person, char type);
void do_help_common(struct char_data *ch, char *argument, int type);
void list_obj_in_pile(void *pile, PileFirst_t getFirst, PileNext_t getNext,
                      int offset, struct char_data *ch);


int singular(struct obj_data *o)
{

    if (IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_HANDS) ||
        IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_FEET) ||
        IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_LEGS) ||
        IS_OBJ_STAT(o, wear_flags, ITEM_WEAR_ARMS)){
        return (FALSE);
	}
    return (TRUE);
}



void show_obj_to_char(struct obj_data *object, struct char_data *ch,
                      int mode)
{

    char            buffer[MAX_STRING_LENGTH];
    buffer[0] = 0;
    if ((mode == 0) && object->description){
        strcpy(buffer, object->description);
    } else if (object->short_description &&
             ((mode == 1) || (mode == 2) || (mode == 3) || (mode == 4))) {
        strcpy(buffer, object->short_description);
    } else if (mode == 5) {
        if (object->type_flag == ITEM_TYPE_NOTE) {
            if (object->action_description) {
                strcpy(buffer, "There is something written upon it:\n\r\n\r");
                strcat(buffer, object->action_description);
                page_string(ch->desc, buffer, 1);
            } else {
                act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
            }
            return;
            /*
             * mail fix, thanks brett
             */

        } else if ((object->type_flag != ITEM_TYPE_DRINKCON)) {
            strcpy(buffer, "You see nothing special..");
        } else {
            strcpy(buffer, "It looks like a drink container.");
        }
    }

    if (mode != 3) {
        if (IS_OBJ_STAT(object, extra_flags, ITEM_INVISIBLE)) {
            strcat(buffer, " (invisible)");
        }
        if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_GOOD) && 
            IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
            if (singular(object)) {
                strcat(buffer, "..It glows red");
            } else {
                strcat(buffer, "..They glow red");
			}
        }
        if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_EVIL) && 
            IS_AFFECTED(ch, AFF_DETECT_GOOD)) {
            if (singular(object)) {
                strcat(buffer, "..It glows white");
            } else {
                strcat(buffer, "..They glow white");
			}
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_MAGIC) && 
            IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
            if (singular(object)) {
                strcat(buffer, "..It glows blue");
            } else {
                strcat(buffer, "..They glow blue");
			}
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_GLOW)) {
            if (singular(object)) {
                strcat(buffer, "..It glows softly");
            } else {
                strcat(buffer, "..They glow softly");
			}
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_HUM)) {
            if (singular(object)) {
                strcat(buffer, "..It hums powerfully");
            } else {
                strcat(buffer, "..They hum with power");
			}
        }

        if (object->type_flag == ITEM_TYPE_ARMOR) {
            if (object->value[0] < (object->value[1] / 4)) {
                if (singular(object)) {
                    strcat(buffer, "..It is falling apart");
                } else {
                    strcat(buffer, "..They are falling apart");
				}
            } else if (object->value[0] < (object->value[1] / 3)) {
                if (singular(object)) {
                    strcat(buffer, "..It is need of much repair.");
                } else {
                    strcat(buffer, "..They are in need of much repair");
				}
            } else if (object->value[0] < (object->value[1] / 2)) {
                if (singular(object)) {
                    strcat(buffer, "..It is in fair condition");
                } else {
                    strcat(buffer, "..They are in fair condition");
				}
            } else if (object->value[0] < object->value[1]) {
                if (singular(object)) {
                    strcat(buffer, "..It is in good condition");
                } else {
                    strcat(buffer, "..They are in good condition");
				}
            } else {
                if (singular(object)) {
                    strcat(buffer, "..It is in excellent condition");
                } else {
                    strcat(buffer, "..They are in excellent condition");
				}
            }
        }
    }

    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);

}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch,
                           int mode, int num)
{
    char            buffer[MAX_STRING_LENGTH];
    char            tmp[10];

    buffer[0] = 0;
    tmp[0] = 0;

    if ((mode == 0) && object->description) {
        strcpy(buffer, object->description);
    } else if (object->short_description &&
             ((mode == 1) || (mode == 2) || (mode == 3) || (mode == 4))) {
        strcpy(buffer, object->short_description);
    } else if (mode == 5) {
        if (object->type_flag == ITEM_TYPE_NOTE) {
            if (object->action_description) {
                strcpy(buffer, "There is something written upon it:\n\r\n\r");
                strcat(buffer, object->action_description);
                page_string(ch->desc, buffer, 1);
            } else {
                act("It's blank.", FALSE, ch, 0, 0, TO_CHAR);
			}
            return;
        } else if ((object->type_flag != ITEM_TYPE_DRINKCON)) {
            strcpy(buffer, "You see nothing special..");
        } else {
            strcpy(buffer, "It looks like a drink container.");
        }
    }

    if (mode != 3) {
        if (IS_OBJ_STAT(object, extra_flags, ITEM_INVISIBLE)) {
            strcat(buffer, " (invisible)");
        }
        if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_GOOD) && 
            IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
            strcat(buffer, "..It glows red!");
        }
        if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_EVIL) && 
            IS_AFFECTED(ch, AFF_DETECT_GOOD)) {
            strcat(buffer, "..It glows white!");
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_MAGIC) && 
            IS_AFFECTED(ch, AFF_DETECT_MAGIC)) {
            strcat(buffer, "..It glows blue!");
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_GLOW)) {
            strcat(buffer, "..It has a soft glowing aura!");
        }
        if (IS_OBJ_STAT(object, extra_flags, ITEM_HUM)) {
            strcat(buffer, "..It emits a faint humming sound!");
        }
    }

    if (num > 1) {
        sprintf(tmp, "[%d]", num);
        strcat(buffer, tmp);
    }
    strcat(buffer, "\n\r");
    page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
    struct obj_data *i,
                   *cond_ptr[50];
    int             Inventory_Num = 1;
    int             k,
                    cond_top,
                    cond_tot[50],
                    found = FALSE;

    cond_top = 0;

    for (i = list; i; i = i->next_content) {
        if (objectIsVisible(ch, i)) {
            if (cond_top < 50) {
                found = FALSE;
                for (k = 0; (k < cond_top && !found); k++) {
                    if (cond_top > 0) {
                        if ((i->item_number == cond_ptr[k]->item_number) &&
                            (i->description && cond_ptr[k]->description &&
                             !strcmp(i->description,
                                     cond_ptr[k]->description))) {
                            cond_tot[k] += 1;
                            found = TRUE;
                        }
                    }
                }
                if (!found) {
                    cond_ptr[cond_top] = i;
                    cond_tot[cond_top] = 1;
                    cond_top += 1;
                }
            } else {
                if ((ITEM_TYPE(i) == ITEM_TYPE_TRAP) || 
                    (GET_TRAP_CHARGES(i) > 0)) {
                    if (objectIsVisible(ch, i)) {
                        show_obj_to_char(i, ch, 0);
                    }
                } else {
                    show_obj_to_char(i, ch, 0);
                }
            }
        }
    }

    if (cond_top) {
        for (k = 0; k < cond_top; k++) {
            if ((ITEM_TYPE(cond_ptr[k]) == ITEM_TYPE_TRAP)
                && (GET_TRAP_CHARGES(cond_ptr[k]) > 0)) {
                if (objectIsVisible(ch, cond_ptr[k])) {
                    if (cond_tot[k] > 1) {
                        oldSendOutput(ch, "[%2d] ", Inventory_Num++);
                        show_mult_obj_to_char(cond_ptr[k], ch, 0,
                                              cond_tot[k]);
                    } else {
                        show_obj_to_char(cond_ptr[k], ch, 0);
                    }
                }
            } else {
                if (cond_tot[k] > 1) {
                    oldSendOutput(ch, "[%2d] ", Inventory_Num++);
                    show_mult_obj_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
                } else {
                    show_obj_to_char(cond_ptr[k], ch, 0);
                }
            }
        }
    }
}

typedef struct obj_data *(*getFirst)( void *pile, int offset );
typedef struct obj_data *(*getNextChain)( void *pile, int offset, 
                                          struct obj_data *obj );

struct obj_data *getFirstChain( void *pile, int offset )
{
    struct obj_data    *chain;
    struct obj_data    *obj;

    chain = (struct obj_data *)pile;
    obj = chain;

    return( obj );
}

struct obj_data *getNextChain( void *pile, int offset, struct obj_data *obj )
{
    if( !obj ) {
        return( NULL );
    }
    obj = obj->next_content;

    return( obj );
}

void list_obj_in_heap(struct obj_data *list, struct char_data *ch)
{
    list_obj_in_pile( list, getFirstChain, getNextChain, 0, ch );
}

struct obj_data *getFirstList( void *pile, int offset )
{
    LinkedList_t       *list;
    LinkedListItem_t   *item;
    struct obj_data    *obj;

    list = (LinkedList *)pile;
    if( !list ) {
        return( NULL );
    }

    /* Assumes the list is locked */
    item = list->head;
    obj = (struct obj_data *)PTR_AT_OFFSET(-offset, item);

    return( obj );
}

struct obj_data *getNextList( void *pile, int offset, struct obj_data *obj )
{
    LinkedListItem_t   *item;

    if( !obj ) {
        return( NULL );
    }

    item = (LinkedListItem_t *)PTR_AT_OFFSET(offset, obj);
    if( !item ) {
        return( NULL );
    }

    /* Assumes the list is locked */
    item = item->next;
    obj = (struct obj_data *)PTR_AT_OFFSET(-offset, item);

    return( obj );
}

void list_obj_in_list( LinkedList_t *list, int offset, struct char_data *ch)
{
    if( !list ) {
        return;
    }

    LinkedListLock( list );
    list_obj_in_pile( list, getFirstList, getNextList, offset, ch );
    LinkedListUnlock( list );
}

#define MAX_COND_PTR 50
void list_obj_in_pile(void *pile, PileFirst_t getFirst, PileNext_t getNext,
                      int offset, struct char_data *ch)
{
    struct obj_data *i,
                   *cond_ptr[MAX_COND_PTR];
    int             k,
                    cond_top,
                    cond_tot[MAX_COND_PTR],
                    found = FALSE;

    int             Num_Inventory = 1;
    cond_top = 0;

    for (i = getFirst(pile, offset); i; i = getNext(pile, offset, i) {
        if (objectIsVisible(ch, i)) {
            if (cond_top < MAX_COND_PTR) {
                found = FALSE;
                for (k = 0; k < cond_top && !found; k++) {
                    if (cond_top > 0) {
                        if ((i->item_number == cond_ptr[k]->item_number) &&
                            (i->short_description && 
                             cond_ptr[k]->short_description &&
                             (!strcmp(i->short_description,
                                      cond_ptr[k]->short_description)))) {
                            cond_tot[k] += 1;
                            found = TRUE;
                        }
                    }
                }
                if (!found) {
                    cond_ptr[cond_top] = i;
                    cond_tot[cond_top] = 1;
                    cond_top += 1;
                }
            } else {
                oldSendOutput(ch, "[%2d] ", Num_Inventory++);
                show_obj_to_char(i, ch, 2);
            }
        }
    }

    if (cond_top) {
        for (k = 0; k < cond_top; k++) {
            oldSendOutput(ch, "[%2d] ", Num_Inventory++);
            if (cond_tot[k] > 1) {
                Num_Inventory += cond_tot[k] - 1;
                show_mult_obj_to_char(cond_ptr[k], ch, 2, cond_tot[k]);
            } else {
                show_obj_to_char(cond_ptr[k], ch, 2);
            }
        }
    }
}

void list_obj_to_char(struct obj_data *list, struct char_data *ch,
                      int mode, bool show)
{
    int             Num_In_Bag = 1;
    struct obj_data *i;
    bool            found;

    found = FALSE;
    for (i = list; i; i = i->next_content) {
        if (objectIsVisible(ch, i)) {
            oldSendOutput(ch, "[%2d] ", Num_In_Bag++);
            show_obj_to_char(i, ch, mode);
            found = TRUE;
        }
    }
    if (!found && show) {
        send_to_char("Nothing\n\r", ch);
	}
}

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
    char            buffer[MAX_STRING_LENGTH];
    int             j,
                    found,
                    percent,
                    otype;
    struct obj_data *tmp_obj,
                   *tqp;
    struct affected_type *aff;
    int             k = 0;

    if (!ch || !i) {
        Log("!ch || !i in act.info.c show_char_to_char");
        return;
    }

    if (mode == 0) {
        /*
         * Don't show linkdead imms to mortals, messy stuff -Lennya
         */
        if (!IS_NPC(i) && IS_IMMORTAL(i) && IS_LINKDEAD(i) &&
            !IS_IMMORTAL(ch)) {
            return;
		}

        if ((IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i)) && 
            !IS_IMMORTAL(ch) && IS_AFFECTED(ch, AFF_SENSE_LIFE) && 
            !IS_IMMORTAL(i)) {

            send_to_char("You sense a hidden life form in the room.\n\r", ch);
            return;
        }

        *buffer = '\0';
        if (!i->player.long_descr || GET_POS(i) != i->specials.default_pos) {
            /*
             * A player char or a mobile without long descr, or not in
             * default pos.
             */
            if (!IS_NPC(i)) {
                /*
                 * if (!GET_TITLE(i)) {
                 * strcpy(buffer,GET_NAME(i));
                 * strcat(buffer," ");
                 * }
                 */
                if (GET_TITLE(i)) {
                    strcat(buffer, GET_TITLE(i));
				}
            } else {
                strcpy(buffer, i->player.short_descr);
                sprintf(buffer, "%s", CAP(buffer));
            }

            if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch)) {
                strcat(buffer, "$c000w (Hiding)");
			}
            if (IS_AFFECTED(i, AFF_INVISIBLE)
                || i->invis_level == IMMORTAL) {
                strcat(buffer, "$c000w (invisible)");
			}
            if (IS_AFFECTED(i, AFF_CHARM)) {
                strcat(buffer, "$c000w (pet)$c0007");
			}
            switch (GET_POS(i)) {
            case POSITION_STUNNED:
                strcat(buffer, "$c000w is lying here, stunned.");
                break;
            case POSITION_INCAP:
                strcat(buffer, "$c000w is lying here, incapacitated.");
                break;
            case POSITION_MORTALLYW:
                strcat(buffer, "$c000w is lying here, mortally wounded.");
                break;
            case POSITION_DEAD:
                strcat(buffer, "$c000w is lying here, dead.");
                break;
            case POSITION_MOUNTED:
                if (MOUNTED(i)) {
                    strcat(buffer, "$c000w is here, riding ");
                    strcat(buffer, MOUNTED(i)->player.short_descr);
                } else {
                    strcat(buffer, "$c000w is standing here.");
                }
                break;
            case POSITION_STANDING:
                if (!IS_AFFECTED(i, AFF_FLYING)
                    && !affected_by_spell(i, SKILL_LEVITATION)) {
                    if (real_roomp(i->in_room)->sector_type ==
                        SECT_WATER_NOSWIM) {
                        strcat(buffer, "$c000w is floating here.");
                    } else {
                        strcat(buffer, "$c000w is standing here.");
					}
                } else {
                    strcat(buffer, "$c000w is flying about.");
                }
                break;
            case POSITION_SITTING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000w is floating here.");
                } else {
                    strcat(buffer, "$c000w is sitting here.");
				}
                break;
            case POSITION_RESTING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000w is resting here in the water.");
                } else {
                    strcat(buffer, "$c000w is resting here.");
				}
                break;
            case POSITION_SLEEPING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000w is sleeping here in the water.");
                } else {
                    strcat(buffer, "$c000w is sleeping here.");
				}
                break;
            case POSITION_FIGHTING:
                if (i->specials.fighting) {
					strcat(buffer, "$c000w is here, fighting ");
                    if (i->specials.fighting == ch) {
                        strcat(buffer, " YOU!");
                    } else {
                        if (i->in_room == i->specials.fighting->in_room) {
                            if (IS_NPC(i->specials.fighting)) {
                                strcat(buffer,
                                       i->specials.fighting->player.
                                       short_descr);
							} else {
                            	strcat(buffer,
                                    GET_NAME(i->specials.fighting));
							}
                        } else {
                            strcat(buffer,
                                   "someone who has already left.");
						}
					}
                } else {
					/*
					 * NIL fighting pointer
					 */
                    strcat(buffer,
                           "$c000w is here struggling with thin air.");
				}
                break;
            default:
                strcat(buffer, " is floating here.");
                break;
            }

            if (IS_AFFECTED2(i, AFF2_AFK)) {
                strcat(buffer, "$c0006 (AFK)$c0007");
			}
            if (IS_LINKDEAD(i)) {
                strcat(buffer, "$c0015 (Linkdead)$c0007");
			}
            if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
                strcat(buffer, "$c0009 (Red Aura)");
            }
			if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_GOOD(i)) {
                strcat(buffer, "$c0015 (White Aura)");
            }

            act(buffer, FALSE, ch, 0, 0, TO_CHAR);
        } else {
			/*
			 * npc with long
			 */
            sprintf(buffer, "%s", (i->player.long_descr));
            /*
             * lennyatest
             */
            while ((buffer[strlen(buffer) - 1] == '\r') ||
                   (buffer[strlen(buffer) - 1] == '\n') ||
                   (buffer[strlen(buffer) - 1] == ' ')) {
                buffer[strlen(buffer) - 1] = '\0';
            }
            if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch)) {
                strcat(buffer, " (Hiding)");
			}
            if (IS_AFFECTED(i, AFF_INVISIBLE)
                || i->invis_level == IMMORTAL) {
                strcat(buffer, " (invisible)");
			}
            if (IS_AFFECTED(i, AFF_CHARM)) {
                strcat(buffer, " (pet)$c0007");
			}
            if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
                strcat(buffer, "$c0009 (Red Aura)$c0007");
            }
            if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_GOOD(i)) {
                strcat(buffer, "$c0015 (White Aura)$c0007");
            }
            if (IS_AFFECTED2(i, AFF2_AFK)) {
                strcat(buffer, "$c0006 (AFK)$c0007");
			}
            if (IS_LINKDEAD(i)) {
                strcat(buffer, "$c0015 (Linkdead)$c0007");
			}
            /*
             * strip \n\r's off
             */
            while ((buffer[strlen(buffer) - 1] == '\r') ||
                   (buffer[strlen(buffer) - 1] == '\n') ||
                   (buffer[strlen(buffer) - 1] == ' ')) {
                buffer[strlen(buffer) - 1] = '\0';
            }
            act(buffer, FALSE, ch, 0, 0, TO_CHAR);
        }

        if (IS_AFFECTED(i, AFF_SANCTUARY) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s glows with a bright light!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000W%s", CAP(buffer));
        }
        
        if (IS_AFFECTED(i, AFF_GROWTH)) {
            sprintf(buffer, "%s is extremely large!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c0003%s", CAP(buffer));
        }
        
        if (IS_AFFECTED(i, AFF_FIRESHIELD) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by burning flames!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c00001%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_CHILLSHIELD) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by cold flames!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000C%s", CAP(buffer));
        }

        if (affected_by_spell(i, SPELL_MANA_SHIELD) && 
            !IS_AFFECTED(i, AFF_DARKNESS)) {
            oldSendOutput(ch, "$c0000yA $c000Ygolden $c000yball floats close "
                           "to %s's ear.\n\r",
                      IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
        }

        if (IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by darkness!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c0008%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_BLADE_BARRIER)) {
            sprintf(buffer, "%s is surrounded by whirling blades!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000B%s", CAP(buffer));
        }

        for (k = 1; k <= TQP_AMOUNT; k++) {
            if ((tqp = find_tqp(k)) && i == tqp->carried_by) {
                sprintf(buffer, "%s is surrounded by a $c000Rm$c000Yu$c000Gl"
                                "$c000Bt$c000Ci$c000wcolored hue!\n\r",
                        IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
                send_to_char(CAP(buffer), ch);
            }
        }
    } else if (mode == 1) {
        if (i->player.description) {
            send_to_char(i->player.description, ch);
        } else {
            act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
        }

        /*
         * personal descriptions.
         */

        if (IS_PC(i)) {
            sprintf(buffer, "$n is %s", races[GET_RACE(i)].racename);
            act(buffer, FALSE, i, 0, ch, TO_VICT);
        }

        if (MOUNTED(i)) {
            sprintf(buffer, "$n is mounted on %s",
                    MOUNTED(i)->player.short_descr);
            act(buffer, FALSE, i, 0, ch, TO_VICT);
        }

        if (RIDDEN(i)) {
            sprintf(buffer, "$n is ridden by %s",
                    IS_NPC(RIDDEN(i)) ? RIDDEN(i)->player.
                    short_descr : GET_NAME(RIDDEN(i)));
            act(buffer, FALSE, i, 0, ch, TO_VICT);
        }

        /*
         * Show a character to another
         */

        if (GET_MAX_HIT(i) > 0) {
            percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
        } else {
            percent = -1;
            /*
             * How could MAX_HIT be < 1??
             */
		}

        if (IS_NPC(i)) {
            strcpy(buffer, i->player.short_descr);
        } else {
            strcpy(buffer, GET_NAME(i));
		}

        if (percent >= 100) {
            strcat(buffer, " is in an excellent condition.");
        } else if (percent >= 90) {
            strcat(buffer, " has a few scratches.");
        } else if (percent >= 75) {
            strcat(buffer, " has some small wounds and many bruises.");
        } else if (percent >= 50) {
            strcat(buffer, " is wounded, and bleeding.");
        } else if (percent >= 30) {
            strcat(buffer, " has some big nasty wounds and scratches.");
        } else if (percent >= 15) {
            strcat(buffer, " is badly wounded");
        } else if (percent >= 0) {
            strcat(buffer, " $c0001is in an awful condition.");
        } else {
            strcat(buffer,
                   " $c0009is bleeding badly from large, gaping wounds.");
		}
        act(buffer, FALSE, ch, 0, 0, TO_CHAR);

        /*
         * spell_descriptions, etc.
         */
        for (aff = i->affected; aff; aff = aff->next) {
            if (aff->type < MAX_EXIST_SPELL) {
                otype = -1;
                if (skills[aff->type].message[MSG_SKILL_DESCRIPTIVE]) {
                    if (aff->type != otype) {
                        act(skills[aff->type].message[MSG_SKILL_DESCRIPTIVE],
                            FALSE, i, 0, ch,
                            TO_VICT);
                        otype = aff->type;
                    }
				}
            }
        }

        found = FALSE;
        for (j = 0; j < MAX_WEAR; j++) {
            if (i->equipment[j]) {
                if (objectIsVisible(ch, i->equipment[j])) {
                    found = TRUE;
                }
            }
        }
        if (found) {
            if (IS_SET(i->player.user_flags, CLOAKED)
                && i->equipment[WEAR_ABOUT]) {
                act("$p covers much of $n's body.", FALSE, i,
                    i->equipment[WEAR_ABOUT], ch, TO_VICT);
            }
            act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);

            for (j = 0; j < MAX_WEAR; j++) {
                if (i->equipment[j]) {
                    if (IS_SET(i->player.user_flags, CLOAKED) &&
                        i->equipment[WEAR_ABOUT] &&
                        !(j == WEAR_LIGHT || j == WEAR_HEAD ||
                          j == WEAR_HANDS || j == WEAR_SHIELD ||
                          j == WEAR_ABOUT || j == WEAR_EAR_R ||
                          j == WEAR_EAR_L || j == WEAR_EYES) &&
                        !IS_OBJ_STAT(i->equipment[WEAR_ABOUT], extra_flags, 
                                     ITEM_INVISIBLE) && !IS_IMMORTAL(ch) && 
                        (i != ch)) {
                        /*
                         * see through cloak -Lennya
                         */
                    } else if (objectIsVisible(ch, i->equipment[j])) {
                        send_to_char(where[j], ch);
                        show_obj_to_char(i->equipment[j], ch, 1);
                    }
                }
            }
        }
        if (HasClass(ch, CLASS_THIEF) && (ch != i) && (!IS_IMMORTAL(ch))) {
            found = FALSE;
            send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
            for (tmp_obj = i->carrying; tmp_obj;
                 tmp_obj = tmp_obj->next_content) {
                if (objectIsVisible(ch, tmp_obj) &&
                    (number(0, MAX_MORT) < GetMaxLevel(ch))) {
                    show_obj_to_char(tmp_obj, ch, 1);
                    found = TRUE;
                }
            }
            if (!found) {
                send_to_char("You can't see anything.\n\r", ch);
			}
        } else if (IS_IMMORTAL(ch)) {
            send_to_char("Inventory:\n\r", ch);
            for (tmp_obj = i->carrying; tmp_obj;
                 tmp_obj = tmp_obj->next_content) {
                show_obj_to_char(tmp_obj, ch, 1);
                found = TRUE;
            }
            if (!found) {
                send_to_char("Nothing\n\r", ch);
            }
        }
    } else if (mode == 2) {

        /*
         * Lists inventory
         */
        act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
        list_obj_in_heap(i->carrying, ch);
    }
}

void glance_at_char(struct char_data *i, struct char_data *ch)
{
    char            buffer[MAX_STRING_LENGTH];
    int             otype,
                    percent;
    struct affected_type *aff;

    if (!ch || !i) {
        Log("!ch || !i in act.info.c show_char_to_char");
        return;
    }
    /*
     * Show a character to another
     */

    if (GET_MAX_HIT(i) > 0) {
        percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
    } else {
        percent = -1;
        /*
         * How could MAX_HIT be < 1??
         */
	}
    if (IS_NPC(i)) {
        strcpy(buffer, i->player.short_descr);
    } else {
        strcpy(buffer, GET_NAME(i));
	}
    if (percent >= 100) {
        strcat(buffer, " is in an excellent condition.");
    } else if (percent >= 90) {
        strcat(buffer, " has a few scratches.");
    } else if (percent >= 75) {
        strcat(buffer, " has some small wounds and many bruises.");
    } else if (percent >= 50) {
        strcat(buffer, " is wounded, and bleeding.");
    } else if (percent >= 30) {
        strcat(buffer, " has some big nasty wounds and scratches.");
    } else if (percent >= 15) {
        strcat(buffer, " is badly wounded");
    } else if (percent >= 0) {
        strcat(buffer, " $c0001is in an awful condition.");
    } else {
        strcat(buffer,
               " $c0009is bleeding badly from large, gaping wounds.");
	}
    act(buffer, FALSE, ch, 0, 0, TO_CHAR);

    /*
     * spell_descriptions, etc.
     */

    for (aff = i->affected; aff; aff = aff->next) {
        if (aff->type < MAX_EXIST_SPELL) {
            otype = -1;
            if (skills[aff->type].message[MSG_SKILL_DESCRIPTIVE]) {
                if (aff->type != otype) {
                    act(skills[aff->type].message[MSG_SKILL_DESCRIPTIVE], 
                        FALSE, i, 0, ch, TO_VICT);
                    otype = aff->type;
                }
			}
        }
    }
}

void show_mult_char_to_char(struct char_data *i, struct char_data *ch,
                            int mode, int num)
{
    char            buffer[MAX_STRING_LENGTH];
    char            tmp[10];
    int             j,
                    found,
                    percent;
    int             k = 0;
    struct obj_data *tmp_obj,
                   *tqp;

    /*
     * Don't show linkdead imms to mortals. Should never be multiple, but
     * hey..  -Lennya
     */
    if (!IS_NPC(i) && IS_IMMORTAL(i) && IS_LINKDEAD(i) && !IS_IMMORTAL(ch)) {
        return;
	}
    if (mode == 0) {
        if ((IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch, i))
            && !IS_IMMORTAL(ch)) {
            if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i)) {
                			      /*
                			       * yer alone, what you doin in this func
                                   * boy? -Lennya
                                   */
                if (num == 1) {
                    act("You sense a hidden life form in the room.", FALSE,
                        ch, 0, 0, TO_CHAR);
                } else {
                    act("$c0002You sense a hidden life form in the room.",
                        FALSE, ch, 0, 0, TO_CHAR);
				}
                return;
            } else {
                /*
                 * no see nothing
                 */
                return;
            }
        }
        /*
         * lets see if below is more effective...sprintf(buffer,"");
         */
        *buffer = '\0';
        if (!(i->player.long_descr)
            || (GET_POS(i) != i->specials.default_pos)) {
            /*
             * A player char or a mobile without long descr, or not in
             * default pos.
             */
            if (!IS_NPC(i)) {
                /*
                 * strcpy(buffer,GET_NAME(i));
                 * strcat(buffer," ");
                 */
                if (GET_TITLE(i)) {
                    strcat(buffer, GET_TITLE(i));
				}
            } else {
                strcpy(buffer, i->player.short_descr);
                sprintf(buffer, "%s", CAP(buffer));
            }

            if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch)) {
                strcat(buffer, "$c000w (Hiding)");
			}
            if (IS_AFFECTED(i, AFF_INVISIBLE)
                || i->invis_level == IMMORTAL) {
                strcat(buffer, "$c000w (invisible)");
			}
            if (IS_AFFECTED(i, AFF_CHARM)) {
                strcat(buffer, "$c000w (pet)$c0007");
			}

            switch (GET_POS(i)) {
            case POSITION_STUNNED:
                strcat(buffer, "$c0005 is lying here, stunned.");
                break;
            case POSITION_INCAP:
                strcat(buffer, "$c0006 is lying here, incapacitated.");
                break;
            case POSITION_MORTALLYW:
                strcat(buffer, "$c0009 is lying here, mortally wounded.");
                break;
            case POSITION_DEAD:
                strcat(buffer, " is lying here, dead.");
                break;
            case POSITION_STANDING:
                if (!IS_AFFECTED(i, AFF_FLYING)
                    && !affected_by_spell(i, SKILL_LEVITATION)) {
                    if (real_roomp(i->in_room)->sector_type ==
                        SECT_WATER_NOSWIM) {
                        strcat(buffer, "$c000wis floating here.");
                    } else {
                        strcat(buffer, "$c000w is standing here.");
					}
                } else {
                    strcat(buffer, "$c000w is flying about.");
                }
                break;
            case POSITION_SITTING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000wis floating here.");
                } else {
                    strcat(buffer, "$c000w is sitting here.");
				}
                break;
            case POSITION_RESTING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000wis resting here in the water");
				} else {
                    strcat(buffer, "$c000w is resting here.");
				}
                break;
            case POSITION_SLEEPING:
                if (real_roomp(i->in_room)->sector_type ==
                    SECT_WATER_NOSWIM) {
                    strcat(buffer, "$c000wis sleeping here in the water");
                } else {
                    strcat(buffer, "$c000w is sleeping here.");
				}
                break;
            case POSITION_FIGHTING:
                if (i->specials.fighting) {
					strcat(buffer, "$c000w is here, fighting ");
                    if (i->specials.fighting == ch) {
                        strcat(buffer, " YOU!");
					} else {
                        if (i->in_room == i->specials.fighting->in_room) {
                            if (IS_NPC(i->specials.fighting)) {
                                strcat(buffer,
                                       i->specials.fighting->player.
                                       short_descr);
							} else {
                                strcat(buffer,
                                       GET_NAME(i->specials.fighting));
							}
                        } else {
                            strcat(buffer,
                                   "someone who has already left.");
						}
                    }
                } else {
					/*
					 * NIL fighting pointer
					 */
                    strcat(buffer, " is here struggling with thin air.");
				}
                break;
            default:
                strcat(buffer, "$c0006 is floating here.");
                break;
            }
            if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
                strcat(buffer, "$c0009 (Red Aura)");
            }

            if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_GOOD(i)) {
                strcat(buffer, "$c0015 (White Aura)");
            }

            if (IS_AFFECTED2(i, AFF2_AFK)) {
                strcat(buffer, "$c0006 (AFK)$c0007");
			}
            if (IS_LINKDEAD(i)) {
                strcat(buffer, "$c0015 (Linkdead)$c0007");
			}
            if (num > 1) {
                sprintf(tmp, " [%d]", num);
                strcat(buffer, tmp);
            }
            act(buffer, FALSE, ch, 0, 0, TO_CHAR);
        } else {
			/*
			 * npc with long
			 */
            sprintf(buffer, "%s", (i->player.long_descr));
            /*
             * lennyatest
             */
            while ((buffer[strlen(buffer) - 1] == '\r') ||
                   (buffer[strlen(buffer) - 1] == '\n') ||
                   (buffer[strlen(buffer) - 1] == ' ')) {
                buffer[strlen(buffer) - 1] = '\0';
            }
            if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch)) {
                strcat(buffer, " (Hiding)");
			}
            if (IS_AFFECTED(i, AFF_INVISIBLE)
                || i->invis_level == IMMORTAL) {
                strcat(buffer, " (invisible)");
			}
            if (IS_AFFECTED(i, AFF_CHARM)) {
                strcat(buffer, " (pet)$c0007");
			}
            if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_EVIL(i)) {
                strcat(buffer, "$c0009 (Red Aura)$c0007");
            }
            if (IS_AFFECTED(ch, AFF_DETECT_GOOD) && IS_GOOD(i)) {
                strcat(buffer, "$c0015 (White Aura)$c0007");
            }
            if (IS_AFFECTED2(i, AFF2_AFK)) {
                strcat(buffer, "$c0006 (AFK)$c0007");
			}
            if (IS_LINKDEAD(i)) {
                strcat(buffer, "$c0015 (Linkdead)$c0007");
			}
            /*
             * this gets a little annoying
             */

            if (num > 1) {
                while ((buffer[strlen(buffer) - 1] == '\r') ||
                       (buffer[strlen(buffer) - 1] == '\n') ||
                       (buffer[strlen(buffer) - 1] == ' ')) {
                    buffer[strlen(buffer) - 1] = '\0';
                }
                sprintf(tmp, " [%d]", num);
                strcat(buffer, tmp);
            }

            act(buffer, FALSE, ch, 0, 0, TO_CHAR);
        }

        if (IS_AFFECTED(i, AFF_SANCTUARY) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s glows with a bright light!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000W%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_GROWTH)) {
            sprintf(buffer, "%s is extremely large!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c0003%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_FIRESHIELD) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by burning flames!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c0001%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_CHILLSHIELD) && !IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by cold flames!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000C%s", CAP(buffer));
        }

        if (affected_by_spell(i, SPELL_MANA_SHIELD) &&
            !IS_AFFECTED(i, AFF_DARKNESS)) {
            oldSendOutput(ch, "$c000yA $c000Ygolden $c000yball floats close "
                           "to %s's ear.\n\r",
                      IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
        }

        if (IS_AFFECTED(i, AFF_DARKNESS)) {
            sprintf(buffer, "%s is surrounded by darkness!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c0008%s", CAP(buffer));
        }

        if (IS_AFFECTED(i, AFF_BLADE_BARRIER)) {
            sprintf(buffer, "%s is surrounded by whirling blades!\n\r",
                    IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
            oldSendOutput(ch, "$c000B%s", CAP(buffer));
        }

        for (k = 1; k <= TQP_AMOUNT; k++) {
            if ((tqp = find_tqp(k)) && i == tqp->carried_by) {
                sprintf(buffer, "%s is surrounded by a "
                                "$c000Rm$c000Yu$c000Gl$c000Bt$c000Ci"
                                "$c000wcolored hue!\n\r",
                        IS_NPC(i) ? i->player.short_descr : GET_NAME(i));
                send_to_char(CAP(buffer), ch);
            }
        }

    } else if (mode == 1) {

        if (i->player.description) {
            send_to_char(i->player.description, ch);
		} else {
            act("You see nothing special about $m.", FALSE, i, 0, ch,
                TO_VICT);
        }

        /*
         * Show a character to another
         */

        if (GET_MAX_HIT(i) > 0) {
            percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
        } else {
            percent = -1;
            /*
             * How could MAX_HIT be < 1??
             */
		}
        if (IS_NPC(i)) {
            strcpy(buffer, i->player.short_descr);
        } else {
            strcpy(buffer, GET_NAME(i));
		}
        if (percent >= 100) {
            strcat(buffer, " is in an excellent condition.\n\r");
        } else if (percent >= 90) {
            strcat(buffer, " has a few scratches.\n\r");
        } else if (percent >= 75) {
            strcat(buffer, " has some small wounds and bruises.\n\r");
        } else if (percent >= 50) {
            strcat(buffer, " has quite a few wounds.\n\r");
        } else if (percent >= 30) {
            strcat(buffer, " has some big nasty wounds and scratches.\n\r");
        } else if (percent >= 15) {
            strcat(buffer, " looks pretty hurt.\n\r");
        } else if (percent >= 0) {
            strcat(buffer, " $c0001is in an awful condition.\n\r");
        } else {
            strcat(buffer, " $c0009is bleeding awfully from big wounds.\n\r");
		}
        act(buffer, FALSE, ch, 0, 0, TO_CHAR);

        found = FALSE;
        for (j = 0; j < MAX_WEAR; j++) {
            if (i->equipment[j]) {
                if (objectIsVisible(ch, i->equipment[j])) {
                    found = TRUE;
                }
            }
        }

        if (found) {
            act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);

            for (j = 0; j < MAX_WEAR; j++) {
                if (i->equipment[j]) {
                    if (objectIsVisible(ch, i->equipment[j])) {
                        send_to_char(where[j], ch);
                        show_obj_to_char(i->equipment[j], ch, 1);
                    }
                }
            }
        }

        if ((HasClass(ch, CLASS_THIEF)) && ch != i) {
            found = FALSE;
            send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
            for (tmp_obj = i->carrying; tmp_obj;
                 tmp_obj = tmp_obj->next_content) {
                if (objectIsVisible(ch, tmp_obj) && 
                    number(0, MAX_MORT) < GetMaxLevel(ch)) {
                    show_obj_to_char(tmp_obj, ch, 1);
                    found = TRUE;
                }
            }

            if (!found) {
                send_to_char("You can't see anything.\n\r", ch);
			}
        }

    } else if (mode == 2) {

        /*
         * Lists inventory
         */
        act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
        list_obj_in_heap(i->carrying, ch);
    }
}

void list_char_in_room(struct char_data *list, struct char_data *ch)
{
    struct char_data *i,
                   *cond_ptr[50];
    int             k,
                    cond_top,
                    cond_tot[50],
                    found = FALSE;

    cond_top = 0;

    for (i = list; i; i = i->next_in_room) {
        if ((ch != i) && (!RIDDEN(i))
            && (IS_AFFECTED(ch, AFF_SENSE_LIFE)
                || (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
            if ((cond_top < 50) && !MOUNTED(i)) {
                found = FALSE;
                if (IS_NPC(i)) {
                    for (k = 0; (k < cond_top && !found); k++) {
                        if (cond_top > 0) {
                            if (i->nr == cond_ptr[k]->nr &&
                                (GET_POS(i) == GET_POS(cond_ptr[k])) &&
                                (i->specials.affected_by ==
                                 cond_ptr[k]->specials.affected_by)
                                && (i->specials.fighting ==
                                    cond_ptr[k]->specials.fighting)
                                && (i->player.short_descr
                                    && cond_ptr[k]->player.short_descr
                                    && 0 == strcmp(i->player.short_descr,
                                                   cond_ptr[k]->player.
                                                   short_descr))) {
                                cond_tot[k] += 1;
                                found = TRUE;
                            }
                        }
                    }
                }
                if (!found) {
                    cond_ptr[cond_top] = i;
                    cond_tot[cond_top] = 1;
                    cond_top += 1;
                }
            } else {
                show_char_to_char(i, ch, 0);
            }
        }
    }

    if (cond_top) {
        for (k = 0; k < cond_top; k++) {
            if (cond_tot[k] > 1) {
                show_mult_char_to_char(cond_ptr[k], ch, 0, cond_tot[k]);
            } else {
                show_char_to_char(cond_ptr[k], ch, 0);
            }
        }
    }
}

void list_char_to_char(struct char_data *list, struct char_data *ch,
                       int mode)
{
    struct char_data *i;

    for (i = list; i; i = i->next_in_room) {
        if ((ch != i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
                          (CAN_SEE(ch, i) && !IS_AFFECTED(i, AFF_HIDE)))) {
            show_char_to_char(i, ch, 0);
		}
    }
}

/*
 * Added by Mike Wilson 9/23/93
 */

void list_exits_in_room(struct char_data *ch)
{
    int             door,
                    seeit = FALSE;
    char            buf[MAX_STRING_LENGTH],
                    buf2[MAX_STRING_LENGTH];
    struct room_direction_data *exitdata;

    *buf = '\0';

    for (door = 0; door <= 5; door++) {
        exitdata = EXIT(ch, door);
        if (exitdata) {
            if (real_roomp(exitdata->to_room)) {
                if (GET_RACE(ch) == RACE_MOON_ELF ||
                    GET_RACE(ch) == RACE_GOLD_ELF ||
                    GET_RACE(ch) == RACE_WILD_ELF ||
                    GET_RACE(ch) == RACE_SEA_ELF ||
                    GET_RACE(ch) == RACE_AVARIEL) {
                    /*
                     * elves can see secret doors 1-2 on d6
                     */
                    seeit = (number(1, 6) <= 2);
                } else if (GET_RACE(ch) == RACE_HALF_ELF) {
                    /*
                     * half-elves can see at 1
                     */
                    seeit = (number(1, 6) <= 1);
                } else {
                    seeit = FALSE;
                }

                if (exitdata->to_room != NOWHERE || IS_IMMORTAL(ch)) {
                    if ((!IS_SET(exitdata->exit_info, EX_SECRET) ||
                         IS_IMMORTAL(ch)) ||
                        (IS_SET(exitdata->exit_info, EX_SECRET) && seeit)) {
                        sprintf(buf2, " %s", direction[door].listexit);
                        strcat(buf, buf2);
                        if (IS_SET(exitdata->exit_info, EX_CLOSED)) {
                            strcat(buf, "$c0015 (closed)");
						}
                        if (IS_SET(exitdata->exit_info, EX_SECRET)
                            && (seeit || IS_IMMORTAL(ch))) {
                            strcat(buf, " $c0009(secret)$c0007");
						}
                    }
                }
            }
        }
    }

    send_to_char("Exits:", ch);

    if (*buf) {
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    } else {
        send_to_char("None!\n\r", ch);
	}
}

void do_look(struct char_data *ch, char *argument, int cmd)
{
    char            buffer[MAX_STRING_LENGTH];
    char           *arg1;
    char           *arg2;
    int             keyword_no,
                    res;
    float           fullperc;
    float           weight = 0;
    int             j,
                    bits,
                    temp;
    bool            found;
    struct obj_data *tmp_object,
                   *found_object,
                   *i;
    struct char_data *tmp_char;
    char           *tmp_desc;
    struct room_direction_data *exitp;
    struct room_data *tmprp;
    struct room_data *rp;
    struct room_data *inroom;
    Keywords_t     *secret;
    char           *temp;
    LinkedListItem_t   *item;
    static char    *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down",
        "in",
        "at",
        "room",
        "\n"
    };

    dlog("in do_look");

    if (!ch->desc) {
        return;
    }

    inroom = real_roomp(ch->in_room);

    if (GET_POS(ch) < POSITION_SLEEPING) {
        send_to_char("You can't see anything but stars!\n\r", ch);
    } else if (GET_POS(ch) == POSITION_SLEEPING) {
        send_to_char("You can't see anything, you're sleeping!\n\r", ch);
    } else if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_char("You can't see a damn thing, you're blinded!\n\r",
                     ch);
    } else if ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) && 
               (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
        send_to_char("It is very dark in here.. Find a lightsource to "
                     "see.\n\r", ch);
        if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
            list_char_in_room(inroom->people, ch);
        }
    } else {
        argument = get_argument_nofill( argument, &arg1 );
        argument = get_argument( argument, &arg2 );
        if( !arg1 ) {
            keyword_no = 8;
        } else if (!strcmp(arg1, "at") && arg2) {
            keyword_no = 7;
        } else if (!strcmp(arg1, "in") && arg2) {
            keyword_no = 6;
        } else {
            keyword_no = search_block(arg1, keywords, FALSE);
        }

        if (arg1 && keyword_no == -1) {
            keyword_no = 7;
            arg2 = arg1;
        }

        found = FALSE;
        tmp_object = 0;
        tmp_char = 0;
        tmp_desc = 0;

        switch (keyword_no) {
            /*
             * look <dir>
             */
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            exitp = EXIT(ch, keyword_no);
            if (exitp) {
                if (exitp->general_description) {
                    send_to_char(exitp->general_description, ch);
                } else {
                    send_to_char("You see nothing special.\n\r", ch);
                }

                if (affected_by_spell(ch, SKILL_DANGER_SENSE)) {
                    tmprp = real_roomp(exitp->to_room);
                    if (tmprp && IS_SET(tmprp->room_flags, DEATH)) {
                        send_to_char("You sense great dangers in that "
                                     "direction.\n\r", ch);
                    }
                }

                if (IS_SET(exitp->exit_info, EX_CLOSED) && exitp->keywords) {
                    secret = StringToKeywords("secret", NULL);
                    if (!KeywordsMatch(secret, exitp->keywords) &&
                        (!IS_SET(exitp->exit_info, EX_SECRET))) {
                        temp = KeywordsToString( exitp->keywords, " " );
                        sprintf(buffer, "The %s is closed.\n\r", temp );
                        send_to_char(buffer, ch);
                        free(temp);
                    }
                    FreeKeywords( secret, TRUE );
                } else {
                    if (IS_SET(exitp->exit_info, EX_ISDOOR) && 
                        exitp->keywords) {
                        temp = KeywordsToString( exitp->keywords, " " );
                        sprintf(buffer, "The %s is open.\n\r", temp );
                        send_to_char(buffer, ch);
                        free(temp);
                    }
                }
            } else {
                send_to_char("You see nothing special.\n\r", ch);
            }

            if (exitp && exitp->to_room && 
                (!IS_SET(exitp->exit_info, EX_ISDOOR) ||
                 !IS_SET(exitp->exit_info, EX_CLOSED))) {

                if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
                    oldSendOutput(ch, "You look %swards.\n\r", 
                              direction[keyword_no].dir);

                    sprintf(buffer, "$n looks %swards.",
                            direction[keyword_no].dir);
                    act(buffer, FALSE, ch, 0, 0, TO_ROOM);

                    rp = real_roomp(exitp->to_room);
                    if (!rp) {
                        send_to_char("You see swirling chaos.\n\r", ch);
                    } else {
                        /*
                         * NO_SPY flag on rooms, Lennya 20030602
                         */
                        if (IS_SET(rp->room_flags, NO_SPY)) {
                            oldSendOutput(ch, "A strange magic blurs your vision "
                                          "as you attempt to look into %s.\n\r",
                                      rp->short_description);
                            /*
                             * imms should be able to see through the
                             * blur:
                             */
                            if (!IS_IMMORTAL(ch)) {
                                return;
                            }
                        }
                        sprintf(buffer, "%ld look", exitp->to_room);
                        do_at(ch, buffer, 0);
                    }
                }
            }
            break;

            /*
             * look 'in'
             */
        case 6:
            if (arg2) {
                /*
                 * Item carried
                 */
                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                          FIND_OBJ_EQUIP, ch, &tmp_char,
                                    &tmp_object);
                if (bits) {
                    /*
                     * Found something
                     */
                    if (ITEM_TYPE(tmp_object) == ITEM_TYPE_DRINKCON) {
                        if (tmp_object->value[1] <= 0) {
                            act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
                        } else {
                            temp = ((tmp_object->value[1] * 3) /
                                    tmp_object->value[0]);
                            sprintf(buffer, "It's %sfull of a %s liquid.\n\r",
                                 fullness[temp],
                                 color_liquid[tmp_object->value[2]]);
                            send_to_char(buffer, ch);
                        }
                    } else if (ITEM_TYPE(tmp_object) == ITEM_TYPE_CONTAINER) {
                        if (!IS_SET(tmp_object->value[1], CONT_CLOSED)) {
                            if (!IS_CORPSE(tmp_object)) {
                                /*
                                 * If it's not a corpse, calculate how
                                 * much stuff is in this container
                                 */
                                LinkedListLock( tmp_object->containList );
                                for( item = tmp_object->containList->head;
                                     item; item = item->next ) {
                                    i = CONTAIN_LINK_TO_OBJ(item);
                                    weight += (float) i->weight;
                                }

                                /**
                                 * @todo this is messy!
                                 */
                                fullperc = 100.0 * (weight /
                                            ((float) tmp_object->value[0] -
                                             ((float) tmp_object->weight - 
                                              weight) - 1));
                                sprintf(buffer, "%s %.0f%s full",
                                        fname(tmp_object->short_description), 
                                              fullperc, "%");
                            } else {
                                /*
                                 * it's a corpse
                                 */
                                sprintf(buffer, "%s ", 
                                        fname(tmp_object->short_description));
                            }
                            send_to_char(buffer, ch);

                            switch (bits) {
                            case FIND_OBJ_INV:
                                send_to_char(" (carried) : \n\r", ch);
                                break;
                            case FIND_OBJ_ROOM:
                                send_to_char(" (here) : \n\r", ch);
                                break;
                            case FIND_OBJ_EQUIP:
                                send_to_char(" (used) : \n\r", ch);
                                break;
                            }
                            list_obj_in_list(tmp_object->containList, 
                                             CONTAIN_LINK_OFFSET, ch);
                        } else {
                            send_to_char("It is closed.\n\r", ch);
                        }
                    } else {
                        send_to_char("That is not a container.\n\r", ch);
                    }
                } else {
                    /*
                     * wrong argument
                     */
                    send_to_char("You do not see that item here.\n\r", ch);
                }
            } else {
                /*
                 * no argument
                 */
                send_to_char("Look in what?!\n\r", ch);
            }
            break;

            /*
             * look 'at'
             */
        case 7:
            if (arg2) {
                bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
                                          FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch,
                                    &tmp_char, &found_object);
                if (tmp_char) {
                    show_char_to_char(tmp_char, ch, 1);
                    if (ch != tmp_char) {
                        act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
                        act("$n looks at $N.", TRUE, ch, 0, tmp_char,
                            TO_NOTVICT);
                    }
                    return;
                }
                /*
                 * Search for Extra Descriptions in room and items
                 */

                /*
                 * Extra description in room??
                 */
                if (!found) {
                    tmp_desc = find_ex_description(arg2, inroom->ex_description,
                                  inroom->ex_description_count);
                    if (tmp_desc) {
                        page_string(ch->desc, tmp_desc, 0);
                        return;
                    }
                }

                /*
                 * Equipment Used
                 */
                if (!found) {
                    for (j = 0; j < MAX_WEAR && !found; j++) {
                        if (ch->equipment[j] &&
                            objectIsVisible(ch, ch->equipment[j])) {
                            tmp_desc = find_ex_description(arg2,
                                        ch->equipment[j]->ex_description,
                                        ch->equipment[j]->ex_description_count);

                            if (tmp_desc) {
                                page_string(ch->desc, tmp_desc, 1);
                                found = TRUE;
                            }
                        }
                    }
                }
                /*
                 * In inventory
                 */
                if (!found) {
                    for (tmp_object = ch->carrying;
                         tmp_object && !found;
                         tmp_object = tmp_object->next_content) {
                        if (objectIsVisible(ch, tmp_object)) {
                            tmp_desc = find_ex_description(arg2,
                                           tmp_object->ex_description,
                                           tmp_object->ex_description_count);
                            if (tmp_desc) {
                                page_string(ch->desc, tmp_desc, 1);
                                found = TRUE;
                            }
                        }
                    }
                }
                /*
                 * Object In room
                 */

                if (!found) {
                    for (tmp_object = real_roomp(ch->in_room)->contents;
                         tmp_object && !found;
                         tmp_object = tmp_object->next_content) {
                        if (objectIsVisible(ch, tmp_object)) {
                            tmp_desc = find_ex_description(arg2,
                                           tmp_object->ex_description,
                                           tmp_object->ex_description_count);
                            if (tmp_desc) {
                                page_string(ch->desc, tmp_desc, 1);
                                found = TRUE;
                            }
                        }
                    }
                }
                /*
                 * wrong argument
                 */
                if (bits) {
                    /*
                     *If an object was found
                     */
                    if (!found) {
                        show_obj_to_char(found_object, ch, 5);
                    } else {
                        show_obj_to_char(found_object, ch, 6);
                    }
                    /*
                     * Find hum, glow etc
                     */
                } else if (!found) {
                    send_to_char("You do not see that here.\n\r", ch);
                }
            } else {
                /*
                 * no argument
                 */
                send_to_char("Look at what?\n\r", ch);
            }
            break;

            /*
             * look ''
             */
        case 8:
            oldSendOutput(ch, "$c000W%s\n\r", inroom->name);

            if (!IS_SET(ch->specials.act, PLR_BRIEF)) {
                send_to_char(inroom->description, ch);
            }

            if (!IS_NPC(ch)) {
                if (IS_SET(ch->specials.act, PLR_HUNTING)) {
                    if (ch->specials.hunting) {
                        res = track(ch, ch->specials.hunting);
                        if (!res) {
                            ch->specials.hunting = 0;
                            ch->hunt_dist = 0;
                            REMOVE_BIT(ch->specials.act, PLR_HUNTING);
                        }
                    } else {
                        ch->hunt_dist = 0;
                        REMOVE_BIT(ch->specials.act, PLR_HUNTING);
                    }
                }
            } else {
                if (IS_SET(ch->specials.act, ACT_HUNTING)) {
                    if (ch->specials.hunting) {
                        res = track(ch, ch->specials.hunting);
                        if (!res) {
                            ch->specials.hunting = 0;
                            ch->hunt_dist = 0;
                            REMOVE_BIT(ch->specials.act, ACT_HUNTING);
                        }
                    } else {
                        ch->hunt_dist = 0;
                        REMOVE_BIT(ch->specials.act, ACT_HUNTING);
                    }
                }
            }

            if (ValidRoom(ch) == TRUE && !IS_SET(ch->specials.act, PLR_BRIEF) &&
                IS_SET(inroom->room_flags, ROOM_WILDERNESS)) {
                generate_map(ch, GET_RADIUS(ch), 3, 3);
                print_map(ch);
            }

            list_exits_in_room(ch);
            list_obj_in_room(inroom->contents, ch);
            list_char_in_room(inroom->people, ch);

            break;

            /*
             * wrong arg
             */
        case -1:
            send_to_char("Sorry, I didn't understand that!\n\r", ch);
            break;

        }
    }
}

int GET_RADIUS(struct char_data *ch)
{
    int             radius = 2;

    if (IS_DARK(ch->in_room) && !IS_IMMORTAL(ch) && 
        !IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        radius--;
	}

    if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
        radius++;
	}

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        radius = 0;
	}

    return radius;
}

/*
 * end of look
 */

void print_map(struct char_data *ch)
{
    char            buf[MAX_STRING_LENGTH];
    int             x,
                    y;
    *buf = 0;

    strcat(buf, " _______\n()______)\n");

    for (x = 0; x < 7; x++) {
        for (y = 0; y < 7; y++) {
            if (y == 0) {
                strcat(buf, "$c000w|");
			}
            if (x == 3 && y == 3) {
                sprintf(buf + strlen(buf), "$c000RX");
            } else {
                sprintf(buf + strlen(buf), "%s",
                        map[x][y] - 1 <
                        0 ? "$c000w:" : sectors[map[x][y] - 1].mapChar);
			}
        }
        strcat(buf, "$c000w|\n\r");
    }
    strcat(buf, "@______/\n\r");
    send_to_char(buf, ch);
}

void generate_map(struct char_data *ch, int size, int x, int y)
{
    int             a,
                    b;
    struct room_data *rm;

    if (!IS_PC(ch)) {
        return;
	}
    for (a = 0; a < 7; a++) {
        for (b = 0; b < 7; b++) {
            map[a][b] = 0;
		}
	}
	rm = real_roomp(ch->in_room);
    if (!rm) {
        send_to_char("Error??", ch);
        return;
	}

#if 0
    if(ValidRoom(rm)==TRUE)
#endif
    recurse_map(rm, size, x, y);
#if 0
    else
        send_to_char("Error somewhere in generate map",ch);
#endif
}

void recurse_map(struct room_data *rp, int size, int x, int y)
{
    struct room_data *rm;

#if 0
   if(map[x][y]!=0) /* May wasn't printing out completely. */
       return;
#endif

    if (!rp) {
        map[x][y] = 22;
        return;
    }

    map[x][y] = rp->sector_type + 1;

    if (size == 0) {
        return;
    }

	/*
	 * north
	 */
    if (rp->dir_option[0]) {
        rm = real_roomp(rp->dir_option[0]->to_room);
        if (!rm) {
            return;
		}
        recurse_map(rm, size - 1, x - 1, y);
    }

	/*
	 * east
	 */
    if (rp->dir_option[1]) {
        rm = real_roomp(rp->dir_option[1]->to_room);
        if (!rm) {
            return;
		}
        recurse_map(rm, size - 1, x, y + 1);
    }

	/*
	 * south
	 */
    if (rp->dir_option[2]) {
        rm = real_roomp(rp->dir_option[2]->to_room);
        if (!rm) {
            return;
		}
        recurse_map(rm, size - 1, x + 1, y);
    }

	/*
	 * west
	 */
    if (rp->dir_option[3]) {
        rm = real_roomp(rp->dir_option[3]->to_room);
        if (!rm) {
            return;
		}
        recurse_map(rm, size - 1, x, y - 1);
    }
}

void do_read(struct char_data *ch, char *argument, int cmd)
{
    char            buf[100];

    dlog("in do_read");

    /*
     * This is just for now - To be changed later.!
     */
    sprintf(buf, "look at %s", argument);
    command_interpreter(ch, buf);
}

void do_examine(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                    buf[1000];
    struct char_data *tmp_char;
    struct obj_data *tmp_object;

    dlog("in do_examine");

    sprintf(buf, "look at %s", argument);
    command_interpreter(ch, buf);

    argument = get_argument(argument, &name);

    if (!name) {
        send_to_char("Examine what?\n\r", ch);
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
                 FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

    if (tmp_object && (ITEM_TYPE(tmp_object) == ITEM_TYPE_DRINKCON ||
                       ITEM_TYPE(tmp_object) == ITEM_TYPE_CONTAINER)) {
        send_to_char("When you look inside, you see:\n\r", ch);
        sprintf(buf, "look in %s", name);
        command_interpreter(ch, buf);
    }
}


void do_exits(struct char_data *ch, char *argument, int cmd)
{
    /*
     * NOTE: Input var 'cmd' is not used.
     */
    int             door;
    char            buf[1000];
    struct room_direction_data *exitdata;

    dlog("in do_exits2");

    *buf = '\0';

    for (door = 0; door <= 5; door++) {
        exitdata = EXIT(ch, door);
        if (!exitdata) {
            continue;
        }

        if (!real_roomp(exitdata->to_room)) {
            if (IS_IMMORTAL(ch)) {
                sprintf(&buf[strlen(buf)],
                        "%s - swirling chaos of #%ld\n\r",
                        direction[door].exit, exitdata->to_room);
            }
        } else if (exitdata->to_room != NOWHERE) {
            if (IS_IMMORTAL(ch)) {
                sprintf(&buf[strlen(buf)], "%s - %s", direction[door].exit,
                        real_roomp(exitdata->to_room)->name);
                if (IS_SET(exitdata->exit_info, EX_SECRET)) {
                    strcat(buf, " (secret)");
                }
                if (IS_SET(exitdata->exit_info, EX_CLOSED)) {
                    strcat(buf, " (closed)");
                }
                if (IS_DARK(exitdata->to_room)) {
                    strcat(buf, " (dark)");
                }
                sprintf(&buf[strlen(buf)], " #%ld\n\r",
                        exitdata->to_room);
            } else if (!IS_SET(exitdata->exit_info, EX_CLOSED) &&
                       !IS_SET(exitdata->exit_info, EX_SECRET)) {
                if (IS_DARK(exitdata->to_room)) {
                    sprintf(&buf[strlen(buf)], "%s - Too dark to tell\n\r", 
                            direction[door].exit);
                } else {
                    sprintf(&buf[strlen(buf)], "%s - %s\n\r",
                            direction[door].exit,
                            real_roomp(exitdata->to_room)->name);
                }
            }
        }
    }

    send_to_char("Obvious exits:\n\r", ch);

    if (*buf) {
        send_to_char(buf, ch);
    } else {
        send_to_char("None.\n\r", ch);
	}
}

/*
 * Score
 */

void do_score(struct char_data *ch, char *argument, int cmd)
{
    struct time_info_data playing_time;
    char            buf[MAX_STRING_LENGTH];
    struct time_info_data my_age;
    int             x;
    char           *color  = "$c000B",
                   *color2 = "$c000w";
    int             level,
                    exp;

    dlog("in do_score");

    age2(ch, &my_age);

    if (GET_TITLE(ch)) {
        oldSendOutput(ch, "%sYou are %s%s\n\r", color, color2, GET_TITLE(ch));
    }

    playing_time = real_time_passed((time(0) - ch->player.time.logon) +
                   ch->player.time.played, 0);

    oldSendOutput(ch, "%sYou are %s%d%s years old and %s%s%s. (Play time: %s%d%s "
                  "days and %s%d%s hours)\n\r%s",
              color, color2, my_age.year, color, color2,
              DescAge(my_age.year, GET_RACE(ch)), color, color2,
              playing_time.day, color, color2, playing_time.hours, color,
              color2, (my_age.month == 0 && my_age.year == 0 ?
                       "$c000w It's your birthday today.\n\r" : ""));

    if (!ch->player.speaks) {
        ch->player.speaks = SPEAK_COMMON;
	}

    oldSendOutput(ch, "%sYou belong to the %s%s%s race, and speak the %s%s%s "
                  "language.\n\r",
              color, color2, races[GET_RACE(ch)].racename, color, color2,
              languages[ch->player.speaks-1].name, color);

    oldSendOutput(ch, "%sYou have %s%d%s($c0011%d%s) hit, %s%d%s($c0011%d%s) mana,"
                  " %s%d%s($c0011%d%s) mv points.\n\r",
              color, color2, GET_HIT(ch), color, GET_MAX_HIT(ch), color,
              color2, GET_MANA(ch), color, GET_MAX_MANA(ch), color, color2,
              GET_MOVE(ch), color, GET_MAX_MOVE(ch), color);

    oldSendOutput(ch, "%sYou won %s%d%s Quests and own %s%d%s quest points.\n\r",
              color, color2, ch->specials.questwon, color, color2,
              ch->player.q_points, color);

    oldSendOutput(ch, "%sYou carry %s%s%s coins, and have an additional %s%d%s in "
                  "the bank.\n\r",
              color, color2, formatNum(GET_GOLD(ch)), color, color2,
              (ch->points.bankgold), color);

    oldSendOutput(ch, "%sYour alignment is: %s%s\n\r", color, color2,
              AlignDesc(GET_ALIGNMENT(ch)), color);

    if (!IS_IMMORTAL(ch) && IS_PC(ch)) {
        oldSendOutput(ch, "%sCombat experience:%s %s %s    ", color, color2,
                      formatNum(GET_EXP(ch)), color);
        oldSendOutput(ch, "Leadership experience: %s%s%s\n\r",
                  color2, formatNum(GET_LEADERSHIP_EXP(ch)), color);

        for (x = 0; x < MAX_CLASS; x++) {
            if (HasClass(ch, pc_num_class(x))) {
                level = GET_LEVEL(ch, x);
                oldSendOutput(ch, "%s%s     Level: %s%2d  %-15s%s", buf, color,
                               color2, level, classes[x].name, color);

                exp = classes[x].levels[level+1].exp - GET_EXP(ch);
                oldSendOutput(ch, "%sExp needed: %s%s\n\r", buf, color2,
                               formatNum(exp));
            }
        }
    }

    if (IS_IMMORTAL(ch)) {
        oldSendOutput(ch, "%sYou are a level %s%d %simmortal.\n\r", color,
                  color2, GetMaxLevel(ch), color);
    }

    oldSendOutput(ch, "%sYour main character class is a %s%s%s.\n\r", color,
              color2, classes[ch->specials.remortclass - 1].name, color);

    oldSendOutput(ch, "%sYou have killed %s%d%s monsters, and have died %s%d%s "
                  "times. Arena: %s%d%s/%s%d%s\n\r",
              color, color2, ch->specials.m_kills, color, color2,
              ch->specials.m_deaths, color, color2, ch->specials.a_kills,
              color, color2, ch->specials.a_deaths, color);

    /*
     * Drow fight -4 in lighted rooms!
     */
    if (!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DROW && 
        !IS_AFFECTED(ch, AFF_DARKNESS) && !IS_UNDERGROUND(ch)) {
        oldSendOutput(ch, "$c0015The light is the area causes you great pain!\n\r");
    }

    if IS_AFFECTED2(ch, AFF2_WINGSBURNED) {
        send_to_char("$c0009Your burned and tattered wings are a source of "
                     "great pain.\n\r", ch);
    }

    if (IS_SET(ch->specials.act, PLR_NOFLY)) {
        oldSendOutput(ch, "%sYou are on the ground in spite of your fly item.\n\r",
                  color2);
    }

    if (IS_PC(ch)) {
        ch->skills[STYLE_STANDARD].learned = 95;
        oldSendOutput(ch, "%sYou are currently fighting %s%s%s.\n\r", color,
                  color2, fight_styles[ch->style], color);
    }

    if (GET_CLAN(ch) == 0) {
        oldSendOutput(ch, "%sYou do not belong to a clan.\n\r", color);
    } else if (GET_CLAN(ch) == 1) {
        oldSendOutput(ch, "%sYou have recently been %sexiled%s from a clan.\n\r",
                  color, color2, color);
    } else {
        oldSendOutput(ch, "%sYou belong to %s%s%s.\n\r", color, color2,
                  clan_list[GET_CLAN(ch)].name, color);
    }

    oldSendOutput(ch, "%sYou have %s%d%s practice sessions remaining.\n\r",
              color, color2, ch->specials.spells_to_learn, color);

    switch (GET_POS(ch)) {
    case POSITION_DEAD:
        send_to_char("$c0009You are DEAD!\n\r", ch);
        break;
    case POSITION_MORTALLYW:
        send_to_char("$c0009You are mortally wounded!, you should seek "
                     "help!\n\r", ch);
        break;
    case POSITION_INCAP:
        send_to_char("$c0009You are incapacitated, slowly fading away.\n\r",
                     ch);
        break;
    case POSITION_STUNNED:
        send_to_char("$c0011You are stunned! You can't move.\n\r", ch);
        break;
    case POSITION_SLEEPING:
        send_to_char("$c0010You are $c000wsleeping.\n\r", ch);
        break;
    case POSITION_RESTING:
        send_to_char("$c0012You are $c000wresting.\n\r", ch);
        break;
    case POSITION_SITTING:
        send_to_char("$c0013You are $c000wsitting.\n\r", ch);
        break;
    case POSITION_FIGHTING:
        if (ch->specials.fighting) {
            act("$c1009You are fighting $N.", FALSE, ch, 0,
                ch->specials.fighting, TO_CHAR);
        } else {
            send_to_char("$c1009You are fighting thin air.\n\r", ch);
		}
        break;
    case POSITION_STANDING:
        send_to_char("$c000BYou are $c000wstanding.\n\r", ch);
        break;
    case POSITION_MOUNTED:
        if (MOUNTED(ch)) {
            oldSendOutput(ch, "$c000BYou are riding on $c000w%s.\n\r",
                      MOUNTED(ch)->player.short_descr);
        } else {
            send_to_char("$c000BYou are $c000wstanding.\n\r", ch);
            break;
        }
        break;
    default:
        send_to_char("$c000BYou are $c000wfloating.\n\r", ch);
        break;
    }
    if (!IS_IMMORTAL(ch) && (!IS_NPC(ch))) {
        if (GET_COND(ch, DRUNK) > 10) {
            send_to_char("$c0011You are intoxicated.\n\r", ch);
		}
        if (GET_COND(ch, FULL) < 2 && GET_COND(ch, FULL) != -1) {
            send_to_char("$c000BYou are $c000whungry$c000B...\n\r", ch);
		}
        if (GET_COND(ch, THIRST) < 2 && GET_COND(ch, THIRST) != -1) {
            send_to_char("$c000BYou are $c000wthirsty$c000B...\n\r", ch);
		}
    }
}

void do_time(struct char_data *ch, char *argument, int cmd)
{
    char           *suf;
    int             weekday,
                    day;

    dlog("in do_time");

    /* 35 days in a month */
    weekday = ((35 * time_info.month) + time_info.day + 1) % 7;

    oldSendOutput(ch, "It is %d o'clock %s, on %s\n\r",
              (time_info.hours % 12 == 0 ? 12 : time_info.hours % 12),
              (time_info.hours >= 12 ? "pm" : "am"), weekdays[weekday]);

    /* day in [1..35] */
    day = time_info.day + 1;

    if (day == 1) {
        suf = "st";
    } else if (day == 2) {
        suf = "nd";
    } else if (day == 3) {
        suf = "rd";
    } else if (day < 20) {
        suf = "th";
    } else if ((day % 10) == 1) {
        suf = "st";
    } else if ((day % 10) == 2) {
        suf = "nd";
    } else if ((day % 10) == 3) {
        suf = "rd";
    } else {
        suf = "th";
	}
    oldSendOutput(ch, "The %d%s Day of the %s, Year %d.\n\r",
              day, suf, month_name[(int) time_info.month], time_info.year);
}

void do_weather(struct char_data *ch, char *argument, int cmd)
{
    static char    *sky_look[4] = {
        "cloudless",
        "cloudy",
        "rainy",
        "lit by flashes of lightning"
    };

    dlog("in do_weather");

    if (OUTSIDE(ch)) {
        oldSendOutput(ch, "The sky is %s and %s.\n\r", sky_look[weather_info.sky],
                  (weather_info.change >= 0 ? 
                   "you feel a warm wind from south" :
                   "your foot tells you bad weather is due"));
    } else {
        send_to_char("You have no feeling about the weather at all.\n\r", ch);
	}
}


void do_list_zones(struct char_data *ch, char *argument, int cmd)
{
    command_interpreter(ch, "help newbie zones");
}


void do_help(struct char_data *ch, char *argument, int cmd)
{
    do_help_common(ch, argument, HELP_MORTAL);
}

void do_help_common(struct char_data *ch, char *argument, int type)
{
    char           *helptext;

    dlog("in do_help");

    if (!ch->desc) {
        return;
	}

    argument = skip_spaces(argument);

    if(!argument) {
        argument = "help help";
    }

    /* Look for an exact match */
    helptext = db_lookup_help( type, argument );

    /* Look for close matches */
    if( !helptext ) {
        helptext = db_lookup_help_similar( type, argument );
    }

    if( !helptext ) {
        /* no matches at all! */
        send_to_char("No remote or exact matches found.\n\r", ch);

        switch(type) {
        case HELP_MORTAL:
            Log("%s is looking for help on \"%s\". Can someone help %s?", 
                GET_NAME(ch), argument, HMHR(ch));

            db_report_entry( REPORT_HELP, ch, argument );
            break;
        case HELP_IMMORTAL:
            Log("%s is looking for wizhelp on \"%s\". Can someone help %s?", 
                GET_NAME(ch), argument, HMHR(ch));

            db_report_entry( REPORT_WIZHELP, ch, argument );
            break;
        }
        return;
    }

    page_string(ch->desc, helptext, 1);
    free( helptext );
}
        

void do_wizhelp(struct char_data *ch, char *arg, int cmd)
{
    char            buf[MAX_STRING_LENGTH];
    char           *keyword;

    int             i,
                    j = 1;
    NODE           *n;

    dlog("in do_wizhelp");

    if (IS_NPC(ch)) {
        return;
	}

    arg = get_argument(arg, &keyword);
    if (keyword) {
        /*
         * asking for help on keyword, try looking in file
         */
        do_help_common(ch, keyword, HELP_IMMORTAL);
        return;
    }

    strcpy(buf, "Wizhelp <keyword>\n\r"
                "Wizard Commands Available To You:\n\r\n\r");

    for (i = 0; i < 27; i++) {
        n = radix_head[i].next;
        while (n) {
            if (n->min_level <= GetMaxLevel(ch) && 
                n->min_level >= IMMORTAL) {
                if (n->min_level == GetMaxLevel(ch)) {
                    sprintf((buf + strlen(buf)),
                            "$c000BL:$c000Y%d $c000w%-14s", n->min_level,
                            n->name);
                } else {
                    sprintf((buf + strlen(buf)),
                            "$c000BL:$c000Y%d $c000w%-14s", n->min_level,
                            n->name);
				}
                if (!(j % 4)) {
                    strcat(buf, "\n\r");
                }
                j++;
            }
            n = n->next;
        }
    }
    strcat(buf, "\n\r");

    page_string(ch->desc, buf, 1);
}


void do_command_list(struct char_data *ch, char *arg, int cmd)
{
    char            buf[MAX_STRING_LENGTH];
    int             i,
                    j = 1;
    NODE           *n;

    dlog("in do_command_list");

    if (IS_NPC(ch)) {
        return;
	}
    sprintf(buf, "Commands Available To You:\n\r\n\r");

    for (i = 0; i < 27; i++) {
        n = radix_head[i].next;
        while (n) {
            if (n->min_level <= GetMaxLevel(ch)) {
                if (strlen(buf) + strlen(n->name) <= MAX_STRING_LENGTH) {
                    sprintf((buf + strlen(buf)), "%-10s", n->name);
				}
                if (!(j % 7)) {
                    if (strlen(buf) + 4 <= MAX_STRING_LENGTH) {
                        sprintf((buf + strlen(buf)), "\n\r");
					}
				}
                j++;
            }
            n = n->next;
        }
    }

    strcat(buf, "\n\r");

    page_string(ch->desc, buf, 1);
}

#define OK_NAME(name,mask)      (mask[0]=='\0' || \
                                strncmp(strcpy(tmpname1,lower(GET_NAME(name))),\
                                        strcpy(tmpname2,lower(mask)),\
                                        strlen(mask))==0)

char           *GetLevelTitle(struct char_data *ch)
{
    char            color[25];
    int             level;
    static char     buf[256];

    int             class = 0;


    level = GetMaxLevel(ch);
    buf[0] = '\0';

    /*
     * get color of title
     */
    if (level < 11) {
        sprintf(color, "$c0008");
    } else if (level < 21) {
        sprintf(color, "$c0004");
    } else if (level < 31) {
        sprintf(color, "$c0006");
    } else if (level < 41) {
        sprintf(color, "$c0014");
    } else if (level < MAX_MORT) {
        sprintf(color, "$c000W");
    } else if (level < IMMORTAL) {
        sprintf(color, "$c000B");
    } else {
        sprintf(color, "$c000Y");
		if (level < 52) {
            sprintf(buf, "%s%s", color, "Lesser Deity");
        } else if (level < 53) {
            sprintf(buf, "%s%s", color, "Deity");
        } else if (level < 54) {
            sprintf(buf, "%s%s", color, "Greater Deity");
        } else if (level < 55) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Lesser Goddess");
            } else {
                sprintf(buf, "%s%s", color, "Lesser God");
			}
        } else if (level < 56) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Goddess");
            } else {
                sprintf(buf, "%s%s", color, "God");
			}
        } else if (level < 57) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Greater Goddess");
            } else {
                sprintf(buf, "%s%s", color, "Greater God");
			}
        } else if (level < 58) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Goddess of Judgement");
            } else {
                sprintf(buf, "%s%s", color, "God of Judgement");
			}
        } else if (level < 59) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Lady");
            } else {
                sprintf(buf, "%s%s", color, "Lord");
			}
        } else if (level < 60) {
            if (GET_SEX(ch) == SEX_FEMALE) {
                sprintf(buf, "%s%s", color, "Implementrix");
            } else {
                sprintf(buf, "%s%s", color, "Implementor");
			}
        } else {
            sprintf(buf, "%s%s", color, "Creator");
		}
        return buf;
    }

    if (!IS_IMMORTAL(ch) && (GET_EXP(ch) > 200000000 || 
                             IS_SET(ch->specials.act, PLR_LEGEND))) {
        sprintf(buf, "%s", "$c0008L$c000we$c000Wge$c000wn$c0008d");
        return buf;
    } else {
        /*
         * Now that we have a working main class, we may as welluse the
         * main class for title
         */
        class = ch->specials.remortclass - 1;
        if (class < 0) {
            class = 0;
		}

        if (GET_SEX(ch) == SEX_FEMALE) {
            sprintf(buf, "%s%s", color,
                    classes[class].levels[(int) GET_LEVEL(ch, class)].title_f);
            return buf;
        } else {
            sprintf(buf, "%s%s", color,
                    classes[class].levels[(int) GET_LEVEL(ch, class)].title_m);
            return buf;
        }
    }
}

char           *SPECIAL_FLAGS(struct char_data *ch,
                              struct char_data *person)
{
    static char     buffer[MAX_STRING_LENGTH];

    buffer[0] = '\0';

    if (IS_SET(person->player.user_flags, NEW_USER)) {
        sprintf(buffer, "%s$c000G[$c000WNEW$c000G]$c0007", buffer);
	}
    if (IS_AFFECTED2(person, AFF2_AFK)) {
        sprintf(buffer, "%s$c0008[AFK]$c0007", buffer);
	}
    if (IS_AFFECTED2(person, AFF2_QUEST)) {
        sprintf(buffer,
                "%s$c0008[$c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008]$c0007",
                buffer);
	}
    if (IS_LINKDEAD(person)) {
        sprintf(buffer, "%s$c0015[LINKDEAD]$c0007", buffer);
	}
    if (IS_IMMORTAL(ch) && person->invis_level > MAX_MORT) {
        sprintf(buffer, "%s(invis %d)", buffer, person->invis_level);
	}

    /*
     * sprintf(buffer,"%s\n\r",buffer);
     */

    return buffer;
}

char           *PrintTitle(struct char_data *person, char type)
{
    static char     buffer[MAX_STRING_LENGTH];
    int             i;
    int             j;

    buffer[0] = '\0';

    switch (type) {
    case 'r':
        sprintf(buffer, "%s     [ $c000BRace - < $c000W%s$c000B >$c000w ]", 
                GET_NAME(person), races[GET_RACE(person)].racename);
        break;

    case 'i':
        sprintf(buffer, "%s     [ $c000BIdle: < $c000W%-3d$c000B>$c000w ]", 
                GET_NAME(person), person->specials.timer);
        break;

    case 'l':
        sprintf(buffer, "%s [ ", GET_NAME(person));

        if( IS_IMMORTAL(person) ) {
            sprintf( buffer, "%s$c000BImmort $c000W%-2d", buffer,
                     GetMaxLevel(person) );
        } else {
            j = 0;
            for( i = 0; i < MAX_CLASS; i++ ) {
                if( GET_LEVEL(person, i) ) {
                    sprintf( buffer, "%s$c000B%s%s $c000W%-2d", buffer, 
                             ( j != 0 ? " / " : "" ), classes[i].abbrev,
                             GET_LEVEL(person, i));
                    j = 1;
                }
            }
        }

        strcat( buffer, "$c000w ]" );
        break;

    case 'h':
        sprintf(buffer, "%s [ $c000BHit:<$c000W%d$c000B/$c000W%-3d$c000B> "
                        "$c000BMana:<$c000W%d$c000B/$c000W%-3d$c000B> "
                        "$c000BMove:<$c000W%d$c000B/$c000W%-3d$c000B>$c000w ]",
                GET_NAME(person), GET_HIT(person), GET_MAX_HIT(person),
                GET_MANA(person), GET_MAX_MANA(person), GET_MOVE(person),
                GET_MAX_MOVE(person));
        break;
    case 's':
        if (GET_STR(person) < 18) {
            sprintf(buffer, "%s [ $c000BSTR:$c000W%-2d  "
                            "$c000BINT:$c000W%-2d  $c000BWIS:$c000W%-2d  "
                            "$c000BCON:$c000W%-2d  $c000BDEX:$c000W%-2d  "
                            "$c000BCHR:$c000W%-2d$c000w ]",
                    GET_NAME(person), GET_STR(person), GET_INT(person),
                    GET_WIS(person), GET_CON(person), GET_DEX(person),
                    GET_CHR(person));
        } else {
            sprintf(buffer, "%s [ $c000BSTR:$c000W%-2d$c000B"
                            "($c000W%1d%s)  $c000BINT:$c000W%-2d  "
                            "$c000BWIS:$c000W%-2d  $c000BCON:$c000W%-2d  "
                            "$c000BDEX:$c000W%-2d  $c000BCHR:$c000W%-2d "
                            "$c000w ]",
                    GET_NAME(person), GET_STR(person), GET_ADD(person),
                    "$c000B",
                    GET_INT(person), GET_WIS(person), GET_CON(person),
                    GET_DEX(person), GET_CHR(person));
		}
        break;
    case 't':
        sprintf(buffer, "%s ", (person->player.title ? person->player.title :
                                GET_NAME(person)));
        break;

    case 'v':
        sprintf(buffer, "%s     [ $c000BInvis Level - <$c000W%d$c000B>"
                        "$c000w ]", GET_NAME(person), person->invis_level);
        break;

    case 'q':
        sprintf(buffer, "%s     [ $c000BQuest Points - <$c000W%d$c000B>"
                        "$c000w ]", GET_NAME(person), person->player.q_points);
        break;

    case 'p':
        sprintf(buffer, "%s     [ $c000BPower Level- <$c000W%ld$c000B>"
                        "$c000w ] ", GET_NAME(person), CalcPowerLevel(person));
        break;
       
    case 'a':
        if (IS_GOOD(person)) {
            sprintf(buffer, "%s     [ $c000BAlignment - <$c000W%d$c000B>"
                            "$c000w ]", GET_NAME(person), 
                    GET_ALIGNMENT(person));
        } else if (IS_EVIL(person)) {
            sprintf(buffer, "%s     [ $c000BAlignment - <$c000R%d$c000B>"
                            "$c000w ]", GET_NAME(person), 
                    GET_ALIGNMENT(person));
        } else {
            sprintf(buffer, "%s     [ $c000BAlignment - <$c000Y%d$c000B>"
                            "$c000w ]", GET_NAME(person), 
                    GET_ALIGNMENT(person));
        }
        break;

    default:
        sprintf(buffer, "%s ", (person->player.title ? person->player.title :
                                GET_NAME(person)));
        break;
    }
    return (buffer);
}

void show_who_immortal(struct char_data *ch, struct char_data *person, 
                       char type)
{
    char           *immbuf;
    int             left,
                    right;
    char           *color = "$c000B";

    immbuf = (person->specials.immtitle ?
              person->specials.immtitle :
              GetLevelTitle(person));

    left = (34 - strlen(immbuf)) / 2;
    right = 34 - left - strlen(immbuf);

    oldSendOutput(ch, "%s%*s%s%*s %s:$c000w %s%s\n\r", 
                  "$c000Y", left, "", immbuf, right, "", color, 
                  PrintTitle(person, type), SPECIAL_FLAGS(ch, person));
}

void show_who_mortal(struct char_data *ch, struct char_data *person, char type)
{
    int             left,
                    right;
    int             i,
                    bit;
    char           *color = "$c000B";

    /* Leave extra space for the multicolored Legend */
    left = (GET_EXP(person) > 200000000 ||
            IS_SET(person->specials.act, PLR_LEGEND) ?  49 : 25 );

    oldSendOutput(ch, "%*s $c0012", left, GetLevelTitle(person));

    right = 8;

    for (bit = 1, i = 0; i < CLASS_COUNT; i++, bit <<= 1) {
        if (HasClass(person, bit)) {
            if (right != 8) {
                send_to_char("/", ch);
                right--;
            }
            send_to_char(classes[i].abbrev, ch);
            right -= strlen(classes[i].abbrev);
        }
    }

    oldSendOutput(ch, "%*s %s:$c000w %s%s\n\r", right, "", color,
                  PrintTitle(person, type), SPECIAL_FLAGS(ch, person));
}

void do_who(struct char_data *ch, char *argument, int cmd)
{

    struct zone_data *zd;
    struct room_data *rm = 0;
    struct descriptor_data *d;
    struct char_data *person;
    char           *tbuf;
    int             count = 0,
                    partialName = 0;
    char            type;
    char           *color = "$c000B";
    int             zone;
    int             found;

    /*
     * Header for who
     */
    if (IS_IMMORTAL(ch)) {
        oldSendOutput(ch, "%s         Havok Players [God Version | 'who -?' for "
                      "help]\n\r"
                      "         ---------------------------------------------"
                      "--\n\r", color);
    } else if (cmd == 234) {
        oldSendOutput(ch, "%sPlayers\n\r------------\n\r", color);
    } else {
        oldSendOutput(ch, "%s                       Havok Players\n\r"
                      "                       -------------\n\r", color);
    }

    /*
     * If its a whozone commmand
     */
    if (cmd == 234) {
        rm = real_roomp(ch->in_room);
        zd = zone_table + rm->zone;
        oldSendOutput(ch, "%sZone: $c0015%s", color, zd->name);
        if (IS_IMMORTAL(ch)) {
            oldSendOutput(ch, "%s($c0015%ld%s)", color, rm->zone, color);
        }
        send_to_char("\n\r\n\r", ch);
    }

    argument = get_argument(argument, &tbuf);
    type = 't';

    if (IS_IMMORTAL(ch)) {
        if (tbuf && tbuf[0] == '-' && tbuf[1] != '\0' && tbuf[1] != '?') {
            type = tbuf[1];
        } else if (tbuf && tbuf[0] == '-' && tbuf[1] == '?') {
            send_to_char("\n\rUsage:  who -r    for race\n\r"
                         "            -i    for idle time\n\r"
                         "            -l    for levels\n\r"
                         "            -h    for hps/mana/moves\n\r", ch);
            send_to_char("            -s    for stats\n\r"
                         "            -t    for title\n\r"
                         "            -q    for questpoints\n\r"
                         "            -p    for powerlevel\n\r", ch);
            send_to_char("            -v    for visibility level\n\r"
                         "            -a    for alignment\n\r\n\r", ch);
            oldSendOutput(ch, "%sConnects since last reboot: $c0015%ld\n\r",
                      color, total_connections);
            oldSendOutput(ch, "%sMax players online since last reboot: "
                          "$c0015%ld\n\r", color, total_max_players);
            return;
        }
    }

    if (tbuf && tbuf[0] != '-') {
        partialName = 1;
        oldSendOutput(ch, "$c000WPartial name search for $c000Y%s.\n\r", tbuf);
    }


    if(cmd == 234) {
        /* Whozone */

        zone = real_roomp(ch->in_room)->zone;

        for (d = descriptor_list; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (person && person->in_room && real_roomp(person->in_room) &&
                GetMaxLevel(person) && CAN_SEE(ch, person) &&
                real_roomp(person->in_room)->zone == zone && 
                (!IS_AFFECTED(person, AFF_HIDE) || IS_IMMORTAL(ch))) {

                oldSendOutput(ch, "$c000w%-25s - %s", GET_NAME(person),
                              real_roomp(person->in_room)->name);
                if (IS_IMMORTAL(ch)) {
                    oldSendOutput(ch, " [%ld]", person->in_room);
                }
                send_to_char("\n\r", ch);
                    count++;
                }
            }
    } else if ( partialName ) {
        /* who command with partial name search */

        for (d = descriptor_list; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (person && person->in_room && real_roomp(person->in_room) &&
                GetMaxLevel(person) && CAN_SEE(ch, person) &&
                isname2(tbuf, GET_NAME(person)) &&
                (!IS_AFFECTED(person, AFF_HIDE) || IS_IMMORTAL(ch))) {

                oldSendOutput(ch, "$c000w%-25s %s", " ",
                          (person->player.title ?  person->player.title :
                           GET_NAME(person)));

                if (IS_IMMORTAL(ch)) {
                    oldSendOutput(ch, " [%ld]", person->in_room);
                }

                send_to_char("\n\r", ch);
                count++;
            }
        }
    } else {
        /* who command, not partial name search */

        /* Show non-quested immortals */
        found = 0;
        for (d = descriptor_list; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (person && person->in_room && real_roomp(person->in_room) &&
                GetMaxLevel(person) && CAN_SEE(ch, person) &&
                IS_IMMORTAL(person) && !IS_AFFECTED2(person, AFF2_QUEST)) {

                if(!found) {
                    found = 1;
                    oldSendOutput(ch, "%-25s%sImmortals\n\r$c000w", "", color);
                }

                show_who_immortal(ch, person, type);
                count++;
            }
        }

        /* Show quested */
        found = 0;
        for (d = descriptor_list; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (person && person->in_room && real_roomp(person->in_room) &&
                GetMaxLevel(person) && CAN_SEE(ch, person) &&
                IS_AFFECTED2(person, AFF2_QUEST)) {

                if(!found) {
                    found = 1;
                    oldSendOutput(ch, "\n\r%-27s%sQuest\n\r$c000w", "", color);
                }

                if( IS_IMMORTAL(person) ) {
                    show_who_immortal(ch, person, type);
                } else {
                    show_who_mortal(ch, person, type);
                }
                count++;
            }
        }

        /* Show non-quested mortals */
        found = 0;
        for (d = descriptor_list; d; d = d->next) {
            person = (d->original ? d->original : d->character);
            if (person && person->in_room && real_roomp(person->in_room) &&
                GetMaxLevel(person) && CAN_SEE(ch, person) && 
                !IS_IMMORTAL(person) && !IS_AFFECTED2(person, AFF2_QUEST)) {

                if(!found) {
                    found = 1;
                    oldSendOutput(ch, "\n\r%-26s%sMortals\n\r$c000w", "", color);
                }

                show_who_mortal(ch, person, type);
                count++;
            }
        }
    }

    if (count == 0) {
        oldSendOutput(ch, "\n\r $c000W No visible characters found!!\n\r");
    } else {
        oldSendOutput(ch, "\n\r%sTotal visible players: $c0015%d\n\r", color,
                      count);
    }

    oldSendOutput(ch, "%sConnects since last reboot: $c0015%ld\n\r", color,
                  total_connections);
    oldSendOutput(ch, "%sPlayers online since last reboot: $c0015%ld\n\r",
                  color, total_max_players);
}


void do_users(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_STRING_LENGTH];

    struct descriptor_data *d;

    dlog("in do_users");

    strcpy(buf, "$c000BConnections:\n\r$c000W------------$c000w\n\r");

    for (d = descriptor_list; d; d = d->next) {
        if (d->character) {
            if (CAN_SEE(ch, d->character) ||
                (IS_IMMORTAL(ch) &&
                 d->character->invis_level <= GetMaxLevel(ch))) {
                if (d && d->character && d->character->player.name) {
                    oldSendOutput(ch, "$c000W%-16s$c000w: ", 
                                  (d->original ? d->original->player.name :
                                   d->character->player.name));
                } else {
                    oldSendOutput(ch, "$c000RUNDEFINED$c000w       : ");
				}

                oldSendOutput(ch, "$c000Y%-22s $c000B[$c000W%s$c000B]$c000w\n\r",
                              connected_types[d->connected], 
                              ( d->host && *d->host ? d->host :
                                "$c000R????$c000w") );
            }
        }
    }
}

void do_inventory(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_inventory");

    send_to_char("You are carrying:\n\r", ch);
    list_obj_in_heap(ch->carrying, ch);
}

void do_equipment(struct char_data *ch, char *argument, int cmd)
{
    int             j,
                    Worn_Index;
    bool            found;

    dlog("in do_equip");

    send_to_char("You are using:\n\r", ch);
    found = FALSE;
    for (Worn_Index = j = 0; j < (MAX_WEAR - 1); j++) {
        oldSendOutput(ch, "%s\n\r", where[j]);
        if (ch->equipment[j]) {
            if (objectIsVisible(ch, ch->equipment[j])) {
                show_obj_to_char(ch->equipment[j], ch, 1);
                found = TRUE;
            } else {
                send_to_char("Something.\n\r", ch);
                found = TRUE;
            }
        } else {
            send_to_char("Nothing.\n\r", ch);
		}
    }
}

void do_credits(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_credits");
    page_string(ch->desc, credits, 0);
}

void do_news(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_news");
    page_string(ch->desc, news, TRUE);
}

void do_info(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_info");
    page_string(ch->desc, info, 0);
}

void do_iwizlist(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_iwizlist");
    page_string(ch->desc, iwizlist, TRUE);
}

void do_wizlist(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_wizlist");
    page_string(ch->desc, wizlist, TRUE);
}

int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
    struct char_data *i;
    char           *name;
    int             number;

    name = fname(mob->player.name);
    for (i = character_list, number = 0; i; i = i->next) {
        if (isname(name, i->player.name) && i->in_room != NOWHERE) {
            number++;
            if (i == mob) {
                return number;
			}
        }
    }
    return 0;
}

char           *numbered_person(struct char_data *ch,
                                struct char_data *person)
{
    static char     buf[MAX_STRING_LENGTH];

    if (IS_NPC(person) && IS_IMMORTAL(ch)) {
        sprintf(buf, "%d.%s", which_number_mobile(ch, person),
                fname(person->player.name));
    } else {
        strcpy(buf, PERS(person, ch));
    }
    return buf;
}

void do_where_person(struct char_data *ch, struct char_data *person,
                     struct string_block *sb)
{
    char            buf[MAX_STRING_LENGTH];

    dlog("in do_where_person");

    if (!CAN_SEE(ch, person)) {
        return;
	}

    sprintf(buf, "$c000W%-30s$c000B- $c000Y%s ", PERS(person, ch),
            (person->in_room > -1 ?
             real_roomp(person->in_room)->name : "$c000RNowhere$c000w"));

    if (IS_IMMORTAL(ch)) {
        sprintf(buf + strlen(buf), "$c000B[$c000W%ld$c000B]$c000w", 
                person->in_room);
	}
    strcpy(buf + strlen(buf), "\n\r");

    append_to_string_block(sb, buf);
}

void do_where_object(struct char_data *ch, struct obj_data *obj,
                     int recurse, struct string_block *sb)
{
    char            buf[MAX_STRING_LENGTH];

    dlog("in do_where_object");

    if (obj->in_room != NOWHERE) {
		/*
		 * object in a room
		 */
        sprintf(buf, "$c000W%-30s$c000B- $c000Y%s $c000B[$c000W%d"
                     "$c000B]$c000w\n\r", obj->short_description,
                real_roomp(obj->in_room)->name, obj->in_room);
    } else if (obj->carried_by != NULL) {
		/*
		 * object carried by monster
		 */
        sprintf(buf, "$c000W%-30s$c000B- carried by: [$c000W%s$c000B]"
                     "$c000w\n\r", obj->short_description,
                numbered_person(ch, obj->carried_by));
    } else if (obj->equipped_by != NULL) {
		/*
		 * object equipped by monster
		 */
        sprintf(buf, "$c000W%-30s$c000B- equipped by: [$c000W%s$c000B]"
                     "$c000w\n\r", obj->short_description,
                numbered_person(ch, obj->equipped_by));
    } else if (obj->in_obj) {
		/*
		 * object in object
		 */
        sprintf(buf, "$c000W%-30s$c000B- in: [$c000W%s$c000B]$c000w\n\r",
                obj->short_description, obj->in_obj->short_description);
    } else {
        sprintf(buf, "$c000W%-30s$c000B- $c000Rgod doesn't even know "
                     "where...\n\r", obj->short_description);
    }
    if (*buf) {
        append_to_string_block(sb, buf);
	}
    if (recurse) {
        if (obj->in_room != NOWHERE) {
            return;
		} else if (obj->carried_by != NULL) {
            do_where_person(ch, obj->carried_by, sb);
        } else if (obj->equipped_by != NULL) {
            do_where_person(ch, obj->equipped_by, sb);
        } else if (obj->in_obj != NULL) {
            do_where_object(ch, obj->in_obj, TRUE, sb);
		}
    }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                    buf[MAX_STRING_LENGTH];
    char           *nameonly;
    register struct char_data *i;
    register struct obj_data *k;
    struct descriptor_data *d;
    int             number,
                    count;
    struct string_block sb;
    Keywords_t     *key;

    dlog("in do_where");

    argument = get_argument(argument, &name);
    if (!name) {
        if (!IS_IMMORTAL(ch)) {
            send_to_char("What are you looking for?\n\r", ch);
            return;
        }

        init_string_block(&sb);
        append_to_string_block(&sb, "$c000BPlayers:\n\r--------\n\r");

        for (d = descriptor_list; d; d = d->next) {
            if (d->character && d->connected == CON_PLYNG &&
                d->character->in_room != NOWHERE && CAN_SEE(ch, d->character)) {
                if (d->original) {
                /*
                 * If switched
                 */
                    sprintf(buf, "$c000W%-20s $c000B- $c000Y%s "
                                 "$c000B[$c000W%ld$c000B] In body of: "
                                 "[$c000W%s$c000B]$c000w\n\r",
                            d->original->player.name,
                            real_roomp(d->character->in_room)->name,
                            d->character->in_room,
                            fname(d->character->player.name));
                } else {
                    sprintf(buf, "$c000W%-20s $c000B- $c000Y%s "
                                 "$c000B[$c000W%ld$c000B]$c000w\n\r",
                            d->character->player.name,
                            real_roomp(d->character->in_room)->name,
                            d->character->in_room);
                }
                append_to_string_block(&sb, buf);
            }
        }
        page_string_block(&sb, ch);
        destroy_string_block(&sb);
        return;
    }

    if (isdigit((int)*name)) {
        nameonly = name;
        count = number = get_number(&nameonly);
    } else {
        count = number = 0;
    }

    *buf = '\0';

    init_string_block(&sb);

    for (i = character_list; i; i = i->next) {
        if (isname(name, i->player.name) && CAN_SEE(ch, i)) {
            if (i->in_room != NOWHERE && 
                (IS_IMMORTAL(ch) || real_roomp(i->in_room)->zone == 
                                    real_roomp(ch->in_room)->zone)) {
                if (number == 0 || (--count) == 0) {
                    if (number == 0) {
                        sprintf(buf, "[%2d] ", ++count);
                        append_to_string_block(&sb, buf);
                    }
                    do_where_person(ch, i, &sb);
                    *buf = 1;
                    if (number != 0) {
                        break;
					}
                }
                if (!IS_IMMORTAL(ch)) {
                    break;
				}
            }
        }
    }

    if (GetMaxLevel(ch) >= SAINT) {
        key = StringToKeywords( name, NULL );
        BalancedBTreeLock( objectKeywordTree );
        for (k = objectKeywordFindFirst( objectKeywordTree, key ); k; 
             k = objectKeywordFindNext( objectKeywordTree, KEYWORD_ITEM_OFFSET,
                                        key, k )) {
            if ( objectIsVisible(ch, k) && (number == 0 || (--count) == 0) ) {
                if (number == 0) {
                    sprintf(buf, "[%2d] ", ++count);
                    append_to_string_block(&sb, buf);
                }
                do_where_object(ch, k, number != 0, &sb);
                *buf = '\0';
                if (number != 0) {
                    break;
                }
            }
        }
        BalancedBTreeUnlock( objectKeywordTree );
        FreeKeywords(key, TRUE);
    }

    if (!*sb.data) {
        send_to_char("Couldn't find any such thing.\n\r", ch);
    } else {
        page_string_block(&sb, ch);
	}
    destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
    int             i,
                    RaceMax,
                    class;
    char            buf[MAX_STRING_LENGTH * 2],
                    buf2[MAX_STRING_LENGTH];

    dlog("in do_levels");

    if (IS_NPC(ch)) {
        send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
        return;
    }

    *buf = '\0';
    /*
     * get the class
     */

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("You must supply a class!\n\r", ch);
        return;
    }

    switch (*argument) {
    case 'C':
    case 'c':
        class = CLERIC_LEVEL_IND;
        break;
    case 'F':
    case 'f':
    case 'W':
    case 'w':
        class = WARRIOR_LEVEL_IND;
        break;
    case 'M':
    case 'm':
        class = MAGE_LEVEL_IND;
        break;
    case 'T':
    case 't':
        class = THIEF_LEVEL_IND;
        break;
    case 'D':
    case 'd':
        class = DRUID_LEVEL_IND;
        break;
    case 'K':
    case 'k':
        class = MONK_LEVEL_IND;
        break;

    case 'B':
    case 'b':
        class = BARBARIAN_LEVEL_IND;
        break;

    case 'S':
    case 's':
        class = SORCERER_LEVEL_IND;
        break;

    case 'P':
    case 'p':
        class = PALADIN_LEVEL_IND;
        break;

    case 'R':
    case 'r':
        class = RANGER_LEVEL_IND;
        break;

    case 'N':
    case 'n':
        class = NECROMANCER_LEVEL_IND;
        break;

    case 'I':
    case 'i':
        class = PSI_LEVEL_IND;
        break;

    default:
        oldSendOutput(ch, "I don't recognize %s\n\r", argument);
        return;
        break;
    }

    RaceMax = races[GET_RACE(ch)].racialMax[class];

    buf[0] = 0;

    for (i = 1; i <= RaceMax; i++) {
        sprintf(buf2, "[%2d] %9ld-%-9ld : %s\n\r", i,
                classes[class].levels[i].exp, classes[class].levels[i + 1].exp,
                (GET_SEX(ch) == SEX_FEMALE ?
                 classes[class].levels[i].title_f : 
                 classes[class].levels[i].title_m));
        strcat(buf, buf2);
    }
    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);

}

void do_consider(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name;
    int             diff;

    dlog("in do_consider");

    argument = get_argument(argument, &name);

    if (!name || !(victim = get_char_room_vis(ch, name))) {
        send_to_char("Consider killing who?\n\r", ch);
        return;
    }

    if (victim == ch) {
        send_to_char("Easy! Very easy indeed!\n\r", ch);
        return;
    }

    if (!IS_NPC(victim)) {
        send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
        return;
    }

    act("$n looks at $N", FALSE, ch, 0, victim, TO_NOTVICT);
    act("$n looks at you", FALSE, ch, 0, victim, TO_VICT);

    if (IS_IMMORTAL(ch)) {
        send_to_char("Consider this, What the heck do you need con for?\n\r",
                     ch);
        return;
    }

    diff = GET_AVE_LEVEL(victim) - GET_AVE_LEVEL(ch);

    diff += MobLevBonus(victim);

    if (diff <= -10) {
        send_to_char("Too easy to be believed.\n\r", ch);
    } else if (diff <= -5) {
        send_to_char("Not a problem.\n\r", ch);
    } else if (diff <= -3) {
        send_to_char("Rather easy.\n\r", ch);
    } else if (diff <= -2) {
        send_to_char("Easy.\n\r", ch);
    } else if (diff <= -1) {
        send_to_char("Fairly easy.\n\r", ch);
    } else if (diff == 0) {
        send_to_char("The perfect match!\n\r", ch);
    } else if (diff <= 1) {
        send_to_char("You would need some luck!\n\r", ch);
    } else if (diff <= 2) {
        send_to_char("You would need a lot of luck!\n\r", ch);
    } else if (diff <= 3) {
        send_to_char("You would need a lot of luck and great equipment!\n\r",
                     ch);
    } else if (diff <= 5) {
        send_to_char("Do you feel lucky, punk?\n\r", ch);
    } else if (diff <= 10) {
        send_to_char("Are you crazy?  Is that your problem?\n\r", ch);
    } else if (diff <= 30) {
        send_to_char("You ARE mad!\n\r", ch);
    } else {
        send_to_char("Why don't I just kill you right now and save you the "
                     "trouble?\n\r", ch);
	}
}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
    int             spl,
                    index,
                    i;
    char            buf[MAX_STRING_LENGTH];

    dlog("in do_spells");

    if (IS_NPC(ch)) {
        send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
        return;
    }

    *buf = 0;

    sprintf(buf + strlen(buf),
            "[  #] %-30s  MANA, Cl, Mu, Dr, Sc, Pa, Ra, Ps, Ne\n\r",
            "SPELL/SKILL\0");

    for (i = 1, spl = 0; i <= MAX_EXIST_SPELL; i++, spl++) {
        index = spell_index[i];
        if( index == -1 ) {
            continue;
        }
        if (IS_IMMORTAL(ch) ||
            spell_info[index].min_level_cleric < ABS_MAX_LVL) {
            if (!spells[spl]) {
                Log("!spells[spl] on %d, do_spells in act.info.c", i);
            } else {
                sprintf(buf + strlen(buf),
                        "[%3d] %-30s  <%3d> %2d %3d %3d %3d %3d %3d %3d "
                        "%3d\n\r",
                        i, spells[spl],
                        spell_info[index].min_usesmana,
                        spell_info[index].min_level_cleric,
                        spell_info[index].min_level_magic,
                        spell_info[index].min_level_druid,
                        spell_info[index].min_level_sorcerer,
                        spell_info[index].min_level_paladin,
                        spell_info[index].min_level_ranger,
                        spell_info[index].min_level_psi,
                        spell_info[index].min_level_necromancer);
			}
        }
    }
    strcat(buf, "\n\r");
    page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
    char            buf[1000];
    char            tbuf[256];
    time_t          ct,
                    ot;
    char           *tmstr,
                   *otmstr;

    dlog("in do_world");

    sprintf(buf, "$c000BBase Source: $c000pHavokMUD$c000B Version $c000W%s."
                 "$c000w", VERSION);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    ot = Uptime;
    otmstr = asctime(localtime((const time_t *)&ot));
    *(otmstr + strlen(otmstr) - 1) = '\0';
    sprintf(buf, "$c000BStart time was: $c000W%s $c000B(PST)$c000w", otmstr);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    ct = time(0);
    tmstr = asctime(localtime((const time_t *)&ct));
    *(tmstr + strlen(tmstr) - 1) = '\0';
    sprintf(buf, "$c000BCurrent time is: $c000W%s $c000B(PST)$c000w", tmstr);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    if (IS_IMMORTAL(ch)) {
        sprintbit((unsigned long) SystemFlags, system_flag_types, tbuf);
        sprintf(buf, "$c000BCurrent system settings: [$c000W%s$c000B]$c000w",
                tbuf);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
#ifdef HASH
    sprintf(buf, "$c000BTotal number of rooms in world: [$c000W%d$c000B]"
                 "$c000w", room_db.klistlen);
#else
    sprintf(buf, "$c000BTotal number of rooms in world: [$c000W%ld$c000B]"
                 "$c000w", room_count);
#endif
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$c000BTotal number of zones in world: [$c000W%d$c000B]"
                 "$c000w\n\r", top_of_zone_table + 1);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$c000BTotal number of distinct mobiles in world: [$c000W%d"
                 "$c000B]$c000w", top_of_mobt + 1);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$c000BTotal number of distinct objects in world: "
                 "[$c000W%d$c000B]$c000w\n\r", top_of_objt + 1);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
    sprintf(buf, "$c000BTotal number of registered players: [$c000W%d"
                 "$c000B]$c000w", top_of_p_table + 1);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    sprintf(buf, "$c000BTotal number of monsters in game: [$c000W%ld"
                 "$c000B]$c000w", mob_count);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    sprintf(buf, "$c000BTotal number of objects in game: [$c000W%ld"
                 "$c000B]$c000w", obj_count);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    sprintf(buf, "$c000BTotal number of connections since last reboot: "
                 "[$c000W%ld$c000B]$c000w", total_connections);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

    sprintf(buf, "$c000BMax. # of players online since last reboot: ["
                 "$c000W%ld$c000B]$c000w", total_max_players);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);

}

#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

void do_resistances(struct char_data *ch, char *argument, int cmd)
{
    int             x;
    char           *color1 = "$c000B",
                   *color2 = "$c000w";

    oldSendOutput(ch, "%sCurrent resistances:\n\r--------------\n\r", color1);

    for (x = 1; x <= BV12; x = x * 2) {
        if (IS_IMMUNE(ch, x)) {
            oldSendOutput(ch, "%sYou are %sImmune      %sto %s%s%s.\n\r",
                      color1, color2, color1, color2,
                      immunity_names[bitvector_num(x)], color1);
        } else if (IsResist(ch, x) && !IsSusc(ch, x)) {
            oldSendOutput(ch, "%sYou are %sResistant   %sto %s%s%s.\n\r",
                      color1, color2, color1, color2,
                      immunity_names[bitvector_num(x)], color1);
        } else if (IsSusc(ch, x) && !IsResist(ch, x)) {
            oldSendOutput(ch, "%sYou are %sSusceptible %sto %s%s%s.\n\r",
                      color1, color2, color1, color2,
                      immunity_names[bitvector_num(x)], color1);
        } else {
            oldSendOutput(ch, "%sYou are %sDefenseless %sto %s%s%s.\n\r",
                      color1, color2, color1, color2,
                      immunity_names[bitvector_num(x)], color1);
		}
    }
}

#if 0
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))
#endif

void do_attribute(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type *aff;
    struct time_info_data my_age;
    int             i = 0,
                    j2 = 0,
                    Worn_Index = 0;
    short           last_type = 0;
    char            buf2[MAX_STRING_LENGTH];
    struct obj_data *j = 0;
    char           *color1 = "$c000B",
                   *color2 = "$c000w";

    dlog("in do_attrib");

    age2(ch, &my_age);

    oldSendOutput(ch, "%sYou are %s%d%s years and %s%d%s months, %s%d%s cms, "
                  "and you weigh %s%d%s lbs.\n\r",
                  color1, color2, my_age.year, color1, color2, my_age.month,
                  color1, color2, ch->player.height, color1, color2,
                  ch->player.weight, color1);

    oldSendOutput(ch, "%sYou are carrying %s%d%s lbs of equipment.\n\r", color1,
                  color2, IS_CARRYING_W(ch), color1);

    oldSendOutput(ch, "%sYou are %s%s%s.\n\r", color1, color2,
                  ArmorDesc(ch->points.armor), color1);

    if (GetMaxLevel(ch) > 5) {
        oldSendOutput(ch, "%sYou have %s%d%s/%s%d %sSTR, %s%d %sINT, %s%d %sWIS, "
                      "%s%d %sDEX, %s%d %sCON, %s%d %sCHR\n\r",
                      color1, color2, GET_STR(ch), color1, color2, GET_ADD(ch),
                      color1, color2, GET_INT(ch), color1, color2, GET_WIS(ch),
                      color1, color2, GET_DEX(ch), color1, color2, GET_CON(ch),
                      color1, color2, GET_CHR(ch), color1);
    }

    oldSendOutput(ch, "%sYour hit bonus and damage bonus are %s%s%s and %s%s%s "
                  "respectively.\n\r",
                  color1, color2, HitRollDesc(GET_HITROLL(ch)), color1, color2,
                  DamRollDesc(GET_DAMROLL(ch)), color1);

    oldSendOutput(ch, "\n\r%sType 'Resist' to see a list of your resistances:\n\r",
                  color1);

    oldSendOutput(ch, "\n\r%sCurrent affects:\n\r--------------\n\r", color1);

    if (ch->affected) {
        for (aff = ch->affected; aff; aff = aff->next) {
            if (aff->type <= MAX_EXIST_SPELL) {
                switch (aff->type) {
                case SKILL_SNEAK:
                case SPELL_PRAYER:
                case SKILL_HIDE:
                case SKILL_QUIV_PALM:
                case SKILL_HUNT:
                case SKILL_DISGUISE:
                case SKILL_SWIM:
                case SKILL_SPY:
                case SKILL_FIRST_AID:
                case SKILL_LAY_ON_HANDS:
                case SKILL_MEDITATE:
                    break;
                case SKILL_MEMORIZE:
                    oldSendOutput(ch, "%sMemorizing : '%s%s%s' will complete in "
                                  "%s%d%s minutes.\n\r",
                                  color1, color2, spells[aff->modifier - 1],
                                  color1, color2, (aff->duration * 4), color1);
                    break;

                default:
                    if (aff->type != last_type) {
                        oldSendOutput(ch, "%sSpell      : '%s%s%s' will expire in "
                                      "%s%d%s hours.\n\r",
                                      color1, color2, spells[aff->type - 1],
                                      color1, color2, aff->duration, color1);
					}
                    break;
                }
                last_type = aff->type;
            }
        }
    }

    for (Worn_Index = j2 = 0; j2 < (MAX_WEAR - 1); j2++) {
        if (ch->equipment[j2]) {
            j = ch->equipment[j2];
            for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                switch (j->affected[i].location) {
                case APPLY_SPELL:
                    sprintbit(j->affected[i].modifier, affected_bits, buf2);
                    if (strcmp(buf2, "NOBITS") == 0) {
                        break;
					}
                    oldSendOutput(ch, "%sSpell      : '%s%s%s' granted though an "
                                  "item.\n\r", color1, color2, buf2, color1);
                    break;
                case APPLY_SPELL2:
                    sprintbit(j->affected[i].modifier, affected_bits2,
                              buf2);
                    if (strcmp(buf2, "NOBITS") == 0) {
                        break;
					}
                    oldSendOutput(ch, "%sSpell      : '%s%s%s' granted though an "
                                  "item.\n\r", color1, color2, buf2, color1);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

void do_value(struct char_data *ch, char *argument, int cmd)
{
    char            buf[1000],
                   *name;
    struct obj_data *obj = 0;
    struct char_data *vict = 0;
    int             learned;

    dlog("in do_value");

    if (!HasClass(ch, CLASS_THIEF | CLASS_RANGER)) {
        send_to_char("Sorry, you can't do that here", ch);
        return;
    }

    argument = get_argument(argument, &name);

    if( !name ) {
        send_to_char("Who, or what are you talking about?\n\r", ch);
        return;
    }

    if (!(obj = objectGetOnChar(ch, name, ch))) {
        if (!(vict = get_char_room_vis(ch, name))) {
            send_to_char("Who, or what are you talking about?\n\r", ch);
            return;
        } 
        
        if (!(obj = objectGetOnChar(ch, argument, vict))) {
            act("You can't see that on $M", FALSE, ch, obj, vict, TO_CHAR);
            act("$n looks you over", FALSE, ch, 0, vict, TO_VICT);
            act("$n looks $N over", FALSE, ch, 0, vict, TO_NOTVICT);
            return;
        }
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    if (!SpyCheck(ch)) {
		/*
		 * failed spying check
		 */
        if (obj && vict) {
            act("$n looks at you, and $s eyes linger on $p",
                FALSE, ch, obj, vict, TO_VICT);
            act("$n studies $N", FALSE, ch, 0, vict, TO_ROOM);

        } else if (obj) {
            act("$n intensely studies $p", FALSE, ch, obj, 0, TO_ROOM);
        } else {
            return;
        }
    }

    sprinttype(ITEM_TYPE(obj), item_types, buf);
    oldSendOutput(ch, "Object: %s.  Item type: %s\n\r", obj->short_description,
                  buf);

    if (!ch->skills) {
        return;
	}

    learned = ch->skills[SKILL_EVALUATE].learned;

    if (number(1, 101) < learned / 3) {
        if (obj->bitvector) {
            sprintbit((unsigned long) obj->bitvector, affected_bits,
                      buf);
            oldSendOutput(ch, "Item will give you following abilities:  %s\n\r",
                          buf);
        }
    }

    if (number(1, 101) < learned / 2) {
        sprintbit((unsigned long) obj->extra_flags, extra_bits, buf);
        oldSendOutput(ch, "Item is: %s\n\r", buf);
    }

    oldSendOutput(ch, "Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
                  obj->weight,
                  GetApprox(obj->cost, learned - 10),
                  GetApprox(obj->cost_per_day, learned - 10),
                  IS_RARE(obj) ? "[RARE]" : " ");

    if (ITEM_TYPE(obj) == ITEM_TYPE_WEAPON) {
        oldSendOutput(ch, "Damage Dice is '%dD%d'\n\r",
                      GetApprox(obj->value[1], learned - 10),
                      GetApprox(obj->value[2], learned - 10));
    } else if (ITEM_TYPE(obj) == ITEM_TYPE_ARMOR) {
        oldSendOutput(ch, "AC-apply is %d\n\r",
                      GetApprox(obj->value[0], learned - 10));
    }
}

char           *ArmorDesc(int a)
{
    if (a >= 90) {
        return ("barely armored");
    } else if (a >= 50) {
        return ("Lightly armored");
    } else if (a >= 30) {
        return ("Medium-armored");
    } else if (a >= 10) {
        return ("Fairly well armored");
    } else if (a >= -10) {
        return ("Well armored");
    } else if (a >= -30) {
        return ("Quite well armored");
    } else if (a >= -50) {
        return ("Very well armored");
    } else if (a >= -90) {
        return ("Extremely well armored");
    } else {
        return ("armored like a Dragon");
    }
}

char           *HitRollDesc(int a)
{
    if (a < -5) {
        return ("Quite bad");
    } else if (a < -1) {
        return ("Pretty lousy");
    } else if (a <= 1) {
        return ("Not Much of one");
    } else if (a < 3) {
        return ("Not bad");
    } else if (a < 8) {
        return ("Pretty good");
    } else if (a < 15) {
        return ("Damn good");
    } else if (a < 30) {
        return ("Very good");
    } else if (a < 40) {
        return ("Extremely good");
    } else
        return ("Magnificent");
}

char           *DamRollDesc(int a)
{
    if (a < -5) {
        return ("Quite bad");
    } else if (a < -1) {
        return ("Pretty lousy");
    } else if (a <= 1) {
        return ("Not Much of one");
    } else if (a < 3) {
        return ("Not bad");
    } else if (a < 8) {
        return ("Pretty good");
    } else if (a < 15) {
        return ("Damn good");
    } else if (a < 30) {
        return ("Very good");
    } else if (a < 40) {
        return ("Extremely good");
    } else
        return ("Magnificent");
}

char           *DescRatio(float f)
{
    /*
     * theirs / yours
     */
    if (f > 1.0) {
        return ("More than twice yours");
    } else if (f > .75) {
        return ("More than half again greater than yours");
    } else if (f > .6) {
        return ("At least a third greater than yours");
    } else if (f > .4) {
        return ("About the same as yours");
    } else if (f > .3) {
        return ("A little worse than yours");
    } else if (f > .1) {
        return ("Much worse than yours");
    } else {
        return ("Extremely inferior");
    }
}

char           *DescDamage(float dam)
{
    if (dam < 1.0) {
        return ("Minimal Damage");
    } else if (dam <= 2.0) {
        return ("Slight damage");
    } else if (dam <= 4.0) {
        return ("A bit of damage");
    } else if (dam <= 10.0) {
        return ("A decent amount of damage");
    } else if (dam <= 15.0) {
        return ("A lot of damage");
    } else if (dam <= 25.0) {
        return ("A whole lot of damage");
    } else if (dam <= 35.0) {
        return ("A very large amount");
    } else {
        return ("A TON of damage");
    }
}

char           *DescAttacks(float a)
{
    if (a < 1.0) {
        return ("Not many");
    } else if (a < 2.0) {
        return ("About average");
    } else if (a < 3.0) {
        return ("A few");
    } else if (a < 5.0) {
        return ("A lot");
    } else if (a < 9.0) {
        return ("Many");
    } else {
        return ("A whole bunch");
    }
}

char           *SpeedDesc(int a)
{

    if (a < 10) {
        return ("Very Slow");
    } else if (a < 20) {
        return ("Slow");
    } else if (a < 30) {
        return ("About Average");
    } else if (a < 40) {
        return ("Fast");
    } else if (a < 50) {
        return ("Very Fast");
    } else {
        return ("Lightning Quick");
    }
}

char           *ArmorSize(int a)
{

    if (a == 0) {
        return ("Adjustable");
    } else if (a == 1) {
        return ("Tiny");
    } else if (a == 2) {
        return ("Small");
    } else if (a == 3) {
        return ("Medium");
    } else if (a == 4) {
        return ("Large");
    } else if (a == 5) {
        return ("Huge");
    } else if (a == 6) {
        return ("Enormous");
    } else {
        return ("Invalid Size");
    }
}

void do_display(struct char_data *ch, char *arg, int cmd)
{
    int             i;

    dlog("in do_display");

    if (IS_NPC(ch)) {
        return;
	}
    i = atoi(arg);

    switch (i) {
    case 0:
        if (ch->term == 0) {
            send_to_char("Display unchanged.\n\r", ch);
            return;
        }
        ch->term = 0;
        ScreenOff(ch);
        send_to_char("Display now turned off.\n\r", ch);
        return;

    case 1:
        if (ch->term == 1) {
            send_to_char("Display unchanged.\n\r", ch);
            return;
        }
        ch->term = VT100;
        InitScreen(ch);
        send_to_char("Display now set to VT100.\n\r", ch);
        return;

    default:
        if (ch->term == VT100) {
            send_to_char("Term type is currently VT100.\n\r", ch);
            return;
        }
        send_to_char("Display is currently OFF.\n\r", ch);
        return;
    }
}

void do_resize(struct char_data *ch, char *arg, int cmd)
{
    int             i;

    dlog("in do_resize");

    if (IS_NPC(ch)) {
        return;
	}

    if( !arg ) {
        send_to_char("Resize to what size?\n\r", ch);
        return;
    }

    i = atoi(arg);

    if (i < 7) {
        send_to_char("Screen size must be greater than 7.\n\r", ch);
        return;
    }

    if (i > 50) {
        send_to_char("Size must be smaller than 50.\n\r", ch);
        return;
    }

    ch->size = i;

    if (ch->term == VT100) {
        ScreenOff(ch);
        InitScreen(ch);
    }

    send_to_char("Ok.\n\r", ch);
    return;
}

int MobLevBonus(struct char_data *ch)
{
    int             t = 0;

    if (mob_index[ch->nr].func == magic_user) {
        t += 5;
	}
    if (mob_index[ch->nr].func == BreathWeapon) {
        t += 7;
	}
    if (mob_index[ch->nr].func == fighter) {
        t += 3;
	}
    if (mob_index[ch->nr].func == snake) {
        t += 3;
	}

    t += (ch->mult_att - 1) * 3;

    if (GET_HIT(ch) > GetMaxLevel(ch) * 8) {
        t += 1;
	}
    if (GET_HIT(ch) > GetMaxLevel(ch) * 12) {
        t += 2;
	}
    if (GET_HIT(ch) > GetMaxLevel(ch) * 16) {
        t += 3;
	}
    if (GET_HIT(ch) > GetMaxLevel(ch) * 20) {
        t += 4;
	}

    return (t);
}

void show_class_skills(struct char_data *ch, char *buffer, int classnum,
                       int skills)
{
    char            buf[254];

    if( classnum > MAX_CLASS ) {
        send_to_char( "I'm confused, I don't recognize that class.", ch );
        return;
    }

    if (!HasClass(ch, 1 << classnum)) {
        oldSendOutput( ch, "I bet you think you're a %s.\n\r", 
                       classes[classnum].name );
        return;
    }

    if( classnum == SORCERER_LEVEL_IND ) {
        sprintf(buf, "You can memorize one spell %d times, with a total "
                     "of %d spells memorized.\n\r"
                     "You currently have %d spells memorized.\n\r"
                     "Your spellbook holds these spells:\n\r",
                MaxCanMemorize(ch, 0), TotalMaxCanMem(ch),
                TotalMemorized(ch));
    } else if( skills ) {
        sprintf( buf, "You can learn these %s skills:\n\r", 
                 classes[classnum].name );
    } else {
        strcpy( buf, "You have knowledge of these skills:\n\r" );
    }
    strcat( buffer, buf );

    show_skills( ch, buffer, classes[classnum].skills, 
                 classes[classnum].skillCount );

    if (ch->specials.remortclass == classnum + 1 &&
        classes[classnum].mainskillCount ) {
        if( skills ) {
            sprintf( buf, "\n\rSince you picked %s as your main class, you get"
                          " these bonus skills:\n\r", classes[classnum].name );
            strcat(buffer, buf);
        }

        show_skills( ch, buffer, classes[classnum].mainskills,
                     classes[classnum].mainskillCount );
    }
}

void show_skills(struct char_data *ch, char *buffer, 
                 const struct skillset *skills, int skillcount)
{
    int             i;
    char            buf[254],
                    temp[20];
    int             sorc;

    if( skills == classes[SORCERER_LEVEL_IND].skills ) {
        sorc = TRUE;
    } else {
        sorc = FALSE;
    }

    for( i = 0; i < skillcount; i++ ) {
        sprintf(buf, "[%-2d] %-30s %-15s", skills[i].level, skills[i].name,
                how_good(ch->skills[skills[i].skillnum].learned));

        if (IsSpecialized(ch->skills[skills[i].skillnum].special)) {
            strcat(buf, " (special)");
        }

        if (sorc && MEMORIZED(ch, skills[i].skillnum)) {
            sprintf(temp, " [x%d]", ch->skills[skills[i].skillnum].nummem);
            strcat(buf, temp);
        }

        strcat(buf, " \n\r");
        if (strlen(buf) + strlen(buffer) > (MAX_STRING_LENGTH * 2) - 2) {
            break;
        }
        strcat(buffer, buf);
    }
}


struct skill_basics
{
    int  skillnum;
    char *name;
};

int skillsort(const void *s1, const void *s2) {
    const struct skill_basics *s1_1 = (struct skill_basics *)s1;
    const struct skill_basics *s2_2 = (struct skill_basics *)s2;
    return(strcmp(s1_1->name, s2_2->name));
}

char *list_knowns(struct char_data *ch, int index) {
    static char buf[MAX_STRING_LENGTH];
    char *output;
    int skillflags = ch->skills[index].flags;
    int foundskills = 0;

    buf[0] = '\0';

    if(IS_SET(skillflags, SKILL_KNOWN_BARBARIAN)) {
        strcat(buf, "Barbarian ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_CLERIC)) {
        strcat(buf, "Cleric ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_DRUID)) {
        strcat(buf, "Druid ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_MAGE)) {
        strcat(buf, "Mage ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_MONK)) {
        strcat(buf, "Monk ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_NECROMANCER)) {
        strcat(buf, "Necromancer ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_PALADIN)) {
        strcat(buf, "Paladin ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_PSI)) {
        strcat(buf, "Psionist ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_RANGER)) {
        strcat(buf, "Ranger ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_SORCERER)) {
        strcat(buf, "Sorcerer ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_THIEF)) {
        strcat(buf, "Thief ");
        foundskills = 1;
    }
    if(IS_SET(skillflags, SKILL_KNOWN_WARRIOR)) {
        strcat(buf, "Warrior ");
        foundskills = 1;
    }
    if(!foundskills) {
        return "Miscellaneous";
    }
    else {
        output = buf;
        return output;
    }
}

int lookthroughskillset(struct char_data *ch,
                        struct skill_basics foundskills[],
                        const struct skillset blah[],
                        int currentcount) {

    int j, k, alreadythere, currskillindex;

    for(j = 0; blah[j].skillnum != -1; j++) {
        currskillindex = blah[j].skillnum;
        if(IS_SET(ch->skills[currskillindex].flags, SKILL_KNOWN)) {
            alreadythere = 0;
            for(k = 0;foundskills[k].skillnum != -1 && !alreadythere;k++) {
                if(currskillindex == foundskills[k].skillnum) {
                    alreadythere = 1;
                }
            }
            if(!alreadythere) {
                currentcount++;
                foundskills[k].skillnum = currskillindex;
                foundskills[k].name = blah[j].name;
            }
        }
    }
    return currentcount;
}


void do_show_skill(struct char_data *ch, char *arg, int cmd)
{
    char buffer[MAX_STRING_LENGTH*2];
    char buf[256];
    int  index;
    int  i;
    int  finalcount = 0;
    struct skill_basics foundskills[MAX_SKILLS];

    buffer[0] = '\0';

    dlog("in do_show_skill");

    if (!ch->skills) {
        return;
	}

    if (!arg) {
        send_to_char("Your skills currently are:\n\r", ch);
        for(i = 0;i < MAX_SKILLS;i++) {
            foundskills[i].skillnum = -1;
        }
        for(i = 0;i < MAX_CLASS;i++) {
            if(HasClass(ch, 1 << i)) {
                finalcount = lookthroughskillset(ch, foundskills,
                                                 classes[i].skills,
                                                 finalcount);
                if(ch->specials.remortclass == i + 1) {
                    finalcount = lookthroughskillset(ch, foundskills,
                                                     classes[i].mainskills,
                                                     finalcount);
                }
            }
        }

        finalcount = lookthroughskillset(ch, foundskills, loreskills,
                                         finalcount);
        finalcount = lookthroughskillset(ch, foundskills, archerskills,
                                         finalcount);
        finalcount = lookthroughskillset(ch, foundskills, warninjaskills,
                                         finalcount);
        finalcount = lookthroughskillset(ch, foundskills, allninjaskills,
                                         finalcount);
        finalcount = lookthroughskillset(ch, foundskills, warmonkskills,
                                         finalcount);

        qsort(foundskills, finalcount, sizeof (struct skill_basics),
              skillsort);
        for(i = 0;i < finalcount;i++) {
            if((strlen(buffer) + strlen(foundskills[i].name) + 2)
                < MAX_STRING_LENGTH*2) {
                sprintf(buf, "%-20s %-20s %-40s\n\r", foundskills[i].name,
                        how_good(ch->skills[foundskills[i].skillnum].learned),
                        list_knowns(ch, foundskills[i].skillnum));
                strcat(buffer, buf);
            }
        }
        page_string(ch->desc, buffer, 1);
        return;
    }

    arg = skip_spaces(arg);
    if( !arg ) {
        send_to_char("Which class? (skill [m s c w t d r p k i n])\n\r", ch);
    }

    switch (*arg) {
    case 'w':
    case 'W':
    case 'f':
    case 'F':
        index = WARRIOR_LEVEL_IND;
        break;

    case 't':
    case 'T':
        index = THIEF_LEVEL_IND;
        break;

    case 'M':
    case 'm':
        index = MAGE_LEVEL_IND;
        break;

    case 'C':
    case 'c':
        index = CLERIC_LEVEL_IND;
        break;

    case 'D':
    case 'd':
        index = DRUID_LEVEL_IND;
        break;

    case 'K':
    case 'k':
        index = MONK_LEVEL_IND;
        break;

    case 'b':
    case 'B':
        index = BARBARIAN_LEVEL_IND;
        break;

    case 'n':
    case 'N':
        index = NECROMANCER_LEVEL_IND;
        break;

    case 'S':
    case 's':
        index = SORCERER_LEVEL_IND;
        break;

    case 'p':
    case 'P':
        index = PALADIN_LEVEL_IND;
        break;

    case 'R':
    case 'r':
        index = RANGER_LEVEL_IND;
        break;

    case 'i':
    case 'I':
        index = PSI_LEVEL_IND;
        break;

    default:
        send_to_char("Which class? (skill [m s c w t d r p k i n])\n\r", ch);
        return;
    }

    show_class_skills( ch, buffer, index, TRUE );
    page_string(ch->desc, buffer, 1);
}

void do_scan(struct char_data *ch, char *argument, int cmd)
{
    static char           *rng_desc[] = {
        "right here",
        "immediately",
        "nearby",
        "a ways",
        "a ways",
        "far",
        "far",
        "very far",
        "very far"
    };
    char            buf[MAX_STRING_LENGTH],
                    buf2[MAX_STRING_LENGTH];
    char           *arg1;
    int             sd,
                    smin,
                    smax,
                    swt,
                    i,
                    max_range = 6,
                    range,
                    rm,
                    nfnd;
    struct char_data *spud;

    dlog("in do_scan");

     /*
     * Check mortals spot skill, and give THEM a max scan of 2 rooms.
     */

    if (!ch->skills) {
        send_to_char("You do not have skills!\n\r", ch);
        return;
    }

    if (!IS_IMMORTAL(ch)) {
        if (!ch->skills[SKILL_SPOT].learned) {
            send_to_char("You have not been trained to spot.\n\r", ch);
            return;
        }

        if (dice(1, 101) > ch->skills[SKILL_SPOT].learned) {
            /*
             * failed
             */
            send_to_char("Absolutely no-one anywhere.\n\r", ch);
            WAIT_STATE(ch, 2);
            return;
        }

        max_range = 2;
    }

    argument = get_argument(argument, &arg1);

    if( arg1 && (sd = find_direction(arg1)) != -1 ) {
        smin = sd;
        smax = sd;
        swt = 1;
        sprintf(buf, "$n peers intently %s.", direction[sd].desc);
        oldSendOutput(ch, "You peer intently %s, and see :\n\r", 
                      direction[sd].desc);
    } else if (arg1 && (spud = get_char_room_vis(ch, arg1))) {
        sprintf(buf, "$n peers intently at $N.");
        sprintf(buf2, "You peer intently at $N.  You sense an aura power "
                      "of %ld", CalcPowerLevel(spud));
        act(buf, FALSE, ch, 0, spud, TO_ROOM);
        act(buf2, FALSE, ch, 0, spud, TO_CHAR);
        return;
    } else {
        smin = 0;
        smax = 5;
        swt = 3;
        sprintf(buf, "$n peers intently all around.");
        oldSendOutput(ch, "You peer intently all around, and see :\n\r");
    }

    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    nfnd = 0;

    /*
     * Check in room first
     */
    for (spud = real_roomp(ch->in_room)->people; spud;
         spud = spud->next_in_room) {
        if (CAN_SEE(ch, spud) && !IS_AFFECTED(spud, AFF_HIDE) && spud != ch) {
            if (IS_NPC(spud)) {
                oldSendOutput(ch, "You sense a %s aura here coming from %s.\n\r",
                              PowerLevelDesc(CalcPowerLevel(spud)),
                              spud->player.short_descr);
            } else {
                oldSendOutput(ch, "You sense a %s aura here coming from %s.\n\r",
                              PowerLevelDesc(CalcPowerLevel(spud)),
                              GET_NAME(spud));
            }
            nfnd++;
        }
    }
    for (i = smin; i <= smax; i++) {
        rm = ch->in_room;
        range = 0;
        while (range < max_range) {
            range++;
            if (clearpath(ch, rm, i)) {
                rm = real_roomp(rm)->dir_option[i]->to_room;
                for (spud = real_roomp(rm)->people; spud;
                     spud = spud->next_in_room) {
                    if (CAN_SEE(ch, spud) && !IS_AFFECTED(spud, AFF_HIDE)) {
                        if (IS_NPC(spud)) {
                            oldSendOutput(ch, "You sense a %s aura %s %s.\n\r",
                                          PowerLevelDesc(CalcPowerLevel(spud)),
                                          rng_desc[range], direction[i].desc);
                        } else {
                            oldSendOutput(ch, "You sense a %s aura %s %s.\n\r",
                                          PowerLevelDesc(CalcPowerLevel(spud)),
                                          rng_desc[range], direction[i].desc);
                        }
                        nfnd++;
                    }
                }
            } else {
                range = max_range + 1;
            }
        }
    }
    if (nfnd == 0) {
        send_to_char("Absolutely no-one anywhere.\n\r", ch);
    }
    WAIT_STATE(ch, swt);
}

/*
 * Calculates a char or mobs powerlevel @ returns :powerlevel @ param
 * :Character struct @ Author :Banon (GH) @ Date :May 16, 2002 maxlevel *
 * Hitroll * damroll * Numatts + HIT*100
 */
long CalcPowerLevel(struct char_data *ch)
{
    long            power;
    int             hitroll,
                    damroll;
    if (IS_IMMORTAL(ch)) {
        power = 2000000;
    } else {
        power = 0;
    }

    hitroll = ch->points.hitroll + str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
    damroll = ch->points.damroll + str_app[STRENGTH_APPLY_INDEX(ch)].todam;

    power = power + GetMaxLevel(ch) * hitroll * damroll * ch->mult_att +
            GET_HIT(ch) * 100;
    if (IS_AFFECTED(ch, AFF_SANCTUARY)) {
        power = power * 2;
    }

    return power;
}

/*
 * You sense a <Godlike> aura nearby to the north.
 */

char           *PowerLevelDesc(long a)
{

    if (a < 5000) {
        return ("very weak");
    } else if (a < 25000) {
        return ("weak");
    } else if (a <= 56000) {
        return ("small");
    } else if (a < 130000) {
        return ("medium");
    } else if (a < 210000) {
        return ("strong");
    } else if (a < 400000) {
        return ("very strong");
    } else if (a < 500000) {
        return ("powerful");
    } else if (a < 600000) {
        return ("extremely powerful");
    } else {
        return ("Godlike");
    }
}

void list_groups(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *person;
    struct follow_type *f;
    int             count = 0;
    char            buf[MAX_STRING_LENGTH * 2],
                    tbuf[MAX_STRING_LENGTH];

    sprintf(buf, "$c0015[------- Adventuring Groups -------]\n\r");

    /*
     * go through the descriptor list
     */
    for (i = descriptor_list; i; i = i->next) {
        /*
         * find everyone who is a master
         */
        if (!i->connected) {
            person = (i->original ? i->original : i->character);

            /*
             * list the master and the group name
             */
            if (!person->master && IS_AFFECTED(person, AFF_GROUP)) {
                if (person->specials.group_name && CAN_SEE(ch, person)) {

                    sprintf(tbuf, "          $c0015%s\n\r$c0014%s\n\r",
                            person->specials.group_name,
                            fname(GET_NAME(person)));
                    strcat(buf, tbuf);

                    /*
                     * list the members that ch can see
                     */
                    count = 0;
                    for (f = person->followers; f; f = f->next) {
                        if (IS_AFFECTED(f->follower, AFF_GROUP) &&
                            IS_PC(f->follower)) {
                            count++;
                            if (CAN_SEE(ch, f->follower) &&
                                strlen(GET_NAME(f->follower)) > 1) {
                                sprintf(tbuf, "$c0013%s\n\r",
                                        fname(GET_NAME(f->follower)));
                                strcat(buf, tbuf);
                            } else {
                                sprintf(tbuf, "$c0013Someone\n\r");
                                strcat(buf, tbuf);
                            }
                        }
                    }
                    /*
                     * if there are no group members, then remove the
                     * group title
                     */
                    if (count < 1) {
                        send_to_char("Your group name has been removed, your "
                                     "group is too small\n\r", person);
                        if (person->specials.group_name) {
                            free(person->specials.group_name);
                        }
                        person->specials.group_name = 0;
                    }
                }
            }
        }
    }
    strcat(buf, "\n\r$c0015[---------- End List --------------]\n\r");
    page_string(ch->desc, buf, 1);
}

int can_see_linear(struct char_data *ch, struct char_data *targ, int *rng,
                   int *dr)
{
    int             i,
                    rm,
                    max_range = 6,
                    range = 0;
    struct char_data *spud;

    for (i = 0; i < 6; i++) {
        rm = ch->in_room;
        range = 0;
        while (range < max_range) {
            range++;
            if (clearpath(ch, rm, i)) {
                rm = real_roomp(rm)->dir_option[i]->to_room;
                for (spud = real_roomp(rm)->people; spud;
                     spud = spud->next_in_room) {
                    if ((spud == targ) && (CAN_SEE(ch, spud))) {
                        *rng = range;
                        *dr = i;
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

/*
 * Returns direction if can see, -1 if not
 */
struct char_data *get_char_linear(struct char_data *ch, char *arg, int *rf,
                                  int *df)
{
    int             i,
                    rm,
                    max_range = 6,
                    range = 0,
                    n,
                    n_sofar = 0;
    struct char_data *spud;
    char           *tmp,
                    tmpname[MAX_STRING_LENGTH];

    strcpy(tmpname, arg);
    tmp = tmpname;
    if (!(n = get_number(&tmp))) {
        return NULL;
    }

    rm = ch->in_room;
    i = 0;
    range = 0;
    for (spud = real_roomp(rm)->people; spud; spud = spud->next_in_room) {
        if ((isname(tmp, GET_NAME(spud))) && (CAN_SEE(ch, spud))) {
            n_sofar++;
            if (n_sofar == n) {
                *rf = range;
                *df = i;
                return spud;
            }
        }
    }

    for (i = 0; i < 6; i++) {
        rm = ch->in_room;
        range = 0;
        while (range < max_range) {
            range++;
            if (clearpath(ch, rm, i)) {
                rm = real_roomp(rm)->dir_option[i]->to_room;
                for (spud = real_roomp(rm)->people; spud;
                     spud = spud->next_in_room) {
                    if ((isname(tmp, GET_NAME(spud)))
                        && (CAN_SEE(ch, spud))) {
                        n_sofar++;
                        if (n_sofar == n) {
                            *rf = range;
                            *df = i;
                            return spud;
                        }
                    }
                }
            } else {
                range = max_range + 1;
            }
        }
    }
    return NULL;
}

void do_glance(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    int             bits;
    struct char_data *tmp_char;
    struct obj_data *found_object;

    arg = skip_spaces(argument);
    if (arg) {
        bits = generic_find(arg, FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
        if (tmp_char) {
            glance_at_char(tmp_char, ch);
            if (ch != tmp_char) {
                act("$n glances at you.", TRUE, ch, 0, tmp_char, TO_VICT);
                act("$n glances at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
            }
            return;
        } 
        
        send_to_char("They are not here...", ch);
        return;
    } 

    send_to_char("Try to glance at someone...", ch);
}

void do_whoarena(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *d;
    struct char_data *person;
    char            buffer[MAX_STRING_LENGTH * 3] = "",
                    tbuf[1024];
    int             count;
    int             color_cnt = 1;

    char            name_mask[40] = "";
    char            tmpname1[80],
                    tmpname2[80];
    char            levels[40] = "";
    char            cls[256];
    int             i,
                    total,
                    classn;
    long            bit;

    dlog("in do_whoarena");

    sprintf(buffer, "$c0005                        Havok Arena Players\n\r");
    strcat(buffer, "                           -------------\n\r");

    count = 0;
    for (d = descriptor_list; d; d = d->next) {
        person = (d->original ? d->original : d->character);
        if (CAN_SEE(ch, d->character) &&
            (real_roomp(person->in_room)) &&
            (real_roomp(person->in_room)->zone == 124)) {
            if (OK_NAME(person, name_mask)) {
                count++;
                color_cnt = ((color_cnt + 1) % 9);
                /*
                 * range 1 to 9
                 */
                if (!IS_IMMORTAL(person)) {
                    for (bit = 1, i = total = classn = 0;
                         i < CLASS_COUNT; i++, bit <<= 1) {

                        if (HasClass(person, bit)) {
                            total += person->player.level[i];
                            if (strlen(cls) != 0) {
                                strcat(cls, "/");
                            }
                            strcat(cls, classes[i].abbrev);
                        }
                    }
                    if (total <= 0) {
                        total = 1;
                    }
                    if (classn <= 0) {
                        classn = 1;
                    }
                    total /= classn;
                    if (GetMaxLevel(person) == 50) {
                        strcpy(levels, "$c0012Hero");
                    } else if (total < 11) {
                        strcpy(levels, "$c0008Apprentice");
                    } else if (total < 21) {
                        strcpy(levels, "$c0004Pilgrim");
                    } else if (total < 31) {
                        strcpy(levels, "$c0006Explorer");
                    } else if (total < 41) {
                        strcpy(levels, "$c0014Adventurer");
                    } else if (total < 51) {
                        strcpy(levels, "$c0015Mystical");
                    }
                    sprintf(tbuf, "%s $c0012%s", levels, cls);
                    sprintf(levels, "%32s", "");
                    strcpy(levels + 10 - ((strlen(tbuf) - 12) / 2), tbuf);
                    sprintf(tbuf, "%-32s $c0005: $c0007%s", levels,
                            person->player.title ?
                            person->player.title : GET_NAME(person));
                } else {
                    switch (GetMaxLevel(person)) {
                    case 51:
                        sprintf(levels, "Lesser Deity");
                        break;
                    case 52:
                        sprintf(levels, "Deity");
                        break;
                    case 53:
                        sprintf(levels, "Greater Deity");
                        break;
                    case 54:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "Lesser God");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Lesser Goddess");
                            break;
                        }

                    case 55:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "God");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Goddess");
                            break;
                        }

                    case 56:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "Greater God");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Greater Goddess");
                            break;
                        }

                    case 57:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "God of Judgement");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Goddess of Judgement");
                            break;
                        }

                    case 58:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "Lord");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Lady");
                            break;
                        }

                    case 59:
                        if (GET_SEX(person) == SEX_MALE) {
                            sprintf(levels, "Supreme Lord");
                            break;
                        } else if (GET_SEX(person) == SEX_FEMALE) {
                            sprintf(levels, "Supreme Lady");
                            break;
                        }

                    case 60:
                        sprintf(levels, "Supreme Being");
                    }

                    sprintf(tbuf, "%s", levels);
                    sprintf(levels, "%30s", "");
                    strcpy(levels + 10 - (strlen(tbuf) / 2), tbuf);
                    if (real_roomp(ch->in_room)->zone == 124) {
                        sprintf(tbuf, "$c0011%-20s $c0005: $c0007%s", levels,
                                person->player.title ?
                                person->player.title : GET_NAME(person));
                    } else {
                        sprintf(tbuf, "$c0011%-20s $c0005: $c0007%s",
                                levels, GET_NAME(person));

                        switch (GET_POS(person)) {
                        case POSITION_DEAD:
                            sprintf(tbuf, "%s is dead", tbuf);
                            break;
                        case POSITION_MORTALLYW:
                            sprintf(tbuf, "%s is mortally wounded", tbuf);
                            break;
                        case POSITION_INCAP:
                            sprintf(tbuf, "%s is incapitated", tbuf);
                            break;
                        case POSITION_STUNNED:
                            sprintf(tbuf, "%s is stunned", tbuf);
                            break;
                        case POSITION_SLEEPING:
                            sprintf(tbuf, "%s is sleeping", tbuf);
                            break;
                        case POSITION_RESTING:
                            sprintf(tbuf, "%s is resting", tbuf);
                            break;
                        case POSITION_SITTING:
                            sprintf(tbuf, "%s is sitting", tbuf);
                            break;
                        case POSITION_STANDING:
                            sprintf(tbuf, "%s is standing", tbuf);
                            break;
                        default:
                            sprintf(tbuf, "%s", tbuf);
                            break;
                        }

                        if (person->specials.fighting) {
                            sprintf(tbuf, "%s and fighting %s.", tbuf,
                                    GET_NAME(person->specials.fighting));
                        }
                        sprintf(tbuf,
                                "%-40s [HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%]",
                                tbuf,
                                ((float) GET_HIT(person) /
                                 (int) GET_MAX_HIT(person)) * 100.0 + 0.5,
                                ((float) GET_MANA(person) /
                                 (int) GET_MAX_MANA(person)) * 100.0 + 0.5,
                                ((float) GET_MOVE(person) /
                                 (int) GET_MAX_MOVE(person)) * 100.0 + 0.5);
                    }
                }
            }
            if (IS_AFFECTED2(person, AFF2_AFK)) {
                sprintf(tbuf + strlen(tbuf), "$c0008 [AFK] $c0007");
            }
            if (IS_AFFECTED2(person, AFF2_QUEST)) {
                sprintf(tbuf + strlen(tbuf),
                        "$c0008 [$c000RQ$c000Yu$c000Ge$c000Bs$c000Ct"
                        "$c0008]$c0007");
            }
            if (IS_LINKDEAD(person)) {
                sprintf(tbuf + strlen(tbuf), "$c0015 [LINKDEAD] $c0007");
            }
            if (IS_IMMORTAL(ch) && person->invis_level > MAX_MORT) {
                sprintf(tbuf + strlen(tbuf), "(invis)");
            }
            sprintf(tbuf + strlen(tbuf), "\n\r");
            if (strlen(buffer) + strlen(tbuf) <
                (MAX_STRING_LENGTH * 2) - 512) {
                strcat(buffer, tbuf);
            }
        }
    }

    sprintf(tbuf, "\n\r$c0005Total visible arena players: $c0015%d\n\r",
            count);
    if (strlen(buffer) + strlen(tbuf) < MAX_STRING_LENGTH * 2 - 512) {
        strcat(buffer, tbuf);
    }
    page_string(ch->desc, buffer, TRUE);
}

void do_clanlist(struct char_data *ch, char *arg, int cmd)
{
    struct char_data *tmp;
    struct char_file_u player;
    char            name[MAX_STRING_LENGTH];
    char            saintbuf[MAX_STRING_LENGTH],
                    leaderbuf[MAX_STRING_LENGTH],
                    memberbuf[MAX_STRING_LENGTH];
    char            saints[MAX_STRING_LENGTH],
                    leaders[MAX_STRING_LENGTH],
                    members[MAX_STRING_LENGTH];
    int             x = 0,
                    j = 0,
                    i = 0,
                    clan = 0,
                    length = 35,
                    clength = 0;

    if (!ch) {
        return;
    }

    if (!arg || !*arg) {
        /*
         * list the clans
         */
        send_to_char("              $c000c-=* $c0008Clan List $c000c*=-\n\r"
                     "\n\r", ch);
        for ( x = 1; x < clanCount; x++ ) {
            if (clan_list[x].number == -1) {
                continue;
            }
            sprintf(name, "%s", clan_list[x].name);
            sprintf(name, "%s", CAP(name));
            /* this should be enough length for any clan name */
            length = 35;
            clength = length - color_strlen(ch, name);
            for (j = 1; j <= clength; j++) {
                strcat(name, " ");
            }
            oldSendOutput(ch, "$c000c[$c000w%2d%s] $c000w%s   $c000c"
                          "[$c000w%s%s]\n\r",
                          x, "$c000c", name, clan_list[x].shortname, "$c000c");
        }
        return;
    }
    
    if (!isdigit((int)*arg)) {
        send_to_char("Usage:  clanlist\n\r"
                     "        clanlist <clan number>\n\r", ch);
        return;
    }
    
    clan = atoi(arg);
    /* NOTE: the clan_list[] table is indexed 0 - clanCount-1, 0 is a null */
    if (clan < 1 || clan >= clanCount) {
        send_to_char("Unknown clan number.\n\r", ch);
        return;
    } 
    
    /*
     * valid clan number
     */
    oldSendOutput(ch, "    $c000c-=* $c000w%s $c000wClan info $c000c*=-\n\r",
              clan_list[clan].name);
    saints[0] = '\0';
    leaders[0] = '\0';
    members[0] = '\0';

    /*
     * loop through pfiles, check for [clan]
     */
    for (i = 0; i < top_of_p_table + 1; i++) {
        if (load_char((player_table + i)->name, &player) > -1) {
            /*
             * store to a tmp char that we can deal with
             */
            CREATE(tmp, struct char_data, 1);
            clear_char(tmp);
            store_to_char(&player, tmp);

            if (GET_CLAN(tmp) == clan) {
                if (IS_IMMORTAL(tmp)) {
                    sprintf(saintbuf,
                            "$c000c[$c0008%s$c000c] $c000w%s\n\r",
                            (GET_SEX(tmp) ?  ((GET_SEX(tmp) != SEX_FEMALE) ?
                              "Patron" : "Matron") : "Notron"),
                            (tmp->player.title ?  tmp->player.title :
                             GET_NAME(tmp)));
                    strcat(saints, saintbuf);
                } else if (IS_SET(tmp->specials.act, PLR_CLAN_LEADER)) {
                    sprintf(leaderbuf,
                            "$c000c[$c0008Leader$c000c] $c000w%s "
                            "$c000w[%ld]\n\r",
                            (tmp->player.title ? tmp->player.title : 
                             GET_NAME(tmp)),
                            CalcPowerLevel(tmp));
                    strcat(leaders, leaderbuf);
                } else {
                    /* just a member, apparently */
                    sprintf(memberbuf,
                            "$c000c[$c0008Member$c000c] $c000w%s\n\r",
                            (tmp->player.title ? tmp->player.title :
                             GET_NAME(tmp)));
                    strcat(members, memberbuf);
                }
            }
            free(tmp);
        } else {
            Log("screw up bigtime in load_char, saint part, in clanlist");
            return;
        }
    }

    /*
     * may have to append these to string blocks, if clans (or
     * titles!?) get real big
     */
    send_to_char(saints, ch);
    send_to_char(leaders, ch);
    send_to_char(members, ch);
}

void do_weapons(struct char_data *ch, char *argument, int cmd)
{
    int             i;

    if (!ch) {
        return;
    }

    if (!ch->skills) {
        return;
    }

    send_to_char("You currently have expertise in use of the following "
                 "weaponry:\n\r\n\r", ch);

    for( i = 0; i < MAX_WEAPONSKILLS; i++ ) {
        if (ch->weaponskills.slot[i]) {
            oldSendOutput(ch, "[%d]  %-20s %10s\n\r", i+1,
                   weaponskills[ch->weaponskills.slot[i] - WEAPON_FIRST].name,
                   how_good(ch->weaponskills.grade[i]));
        }
    }
}

void do_allweapons(struct char_data *ch, char *argument, int cmd)
{
    int             i;
    char            buf[MAX_STRING_LENGTH],
                    shbuf[500];

    dlog("in do_allweapons");

    if (IS_NPC(ch)) {
        send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
        return;
    }

    *buf = 0;

    sprintf(buf, "[  #] Weapontype\n\r");

    i = 0;
    while (weaponskills[i].level != -1) {
        sprintf(shbuf, "[%3d] %s\n\r", i, weaponskills[i].name);
        strcat(buf, shbuf);
        i++;
    }
    page_string(ch->desc, buf, 1);
}

void do_sea_commands(struct char_data *ch, char *argument, int cmd)
{
    send_to_char("Sorry, you need to be in your ship and on the open sea to "
                 "partake in those actions.\n\r"
                 "(Possible commands-] Plank, Disembark, Embark, "
                 "Sail/Steer)\n\r", ch);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
