#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protos.h"

#define OBJ_MAIN_MENU         0
#define CHANGE_OBJ_NAME       1
#define CHANGE_OBJ_SHORT      2
#define CHANGE_OBJ_DESC       3
#define CHANGE_OBJ_TYPE       4
#define CHANGE_OBJ_WEAR       5
#define CHANGE_OBJ_FLAGS      6
#define CHANGE_OBJ_WEIGHT     7
#define CHANGE_OBJ_VALUE      8
#define CHANGE_OBJ_COST       9
#define CHANGE_OBJ_AFFECTS   10
#define CHANGE_OBJ_AFFECT1   11
#define CHANGE_OBJ_AFFECT2   12
#define CHANGE_OBJ_AFFECT3   13
#define CHANGE_OBJ_AFFECT4   14
#define CHANGE_AFFECT1_MOD   15
#define CHANGE_AFFECT2_MOD   16
#define CHANGE_AFFECT3_MOD   17
#define CHANGE_AFFECT4_MOD   18
#define CHANGE_OBJ_VALUES    19
#define CHANGE_OBJ_VALUE1    20
#define CHANGE_OBJ_VALUE2    21
#define CHANGE_OBJ_VALUE3    22
#define CHANGE_OBJ_VALUE4    23
#define OBJ_HIT_RETURN       24

#define CHANGE_OBJ_AFFECT5   25
#define CHANGE_AFFECT5_MOD   26

#define CHANGE_SAFFECT1_MOD 27
#define CHANGE_SAFFECT2_MOD 28
#define CHANGE_SAFFECT3_MOD 29
#define CHANGE_SAFFECT4_MOD 30
#define CHANGE_SAFFECT5_MOD 31

#define CHANGE_S2AFFECT1_MOD 32
#define CHANGE_S2AFFECT2_MOD 33
#define CHANGE_S2AFFECT3_MOD 34
#define CHANGE_S2AFFECT4_MOD 35
#define CHANGE_S2AFFECT5_MOD 36
#define CHANGE_OBJ_SPECIAL   37
#define CHANGE_OBJ_EGO       38
#define CHANGE_OBJ_SPEED     39
#define CHANGE_OBJ_MAX       40
#define CHANGE_OBJ_TWEAK         41
#define CHANGE_OBJ_WTYPE         42

#define ENTER_CHECK        1

extern struct index_data *obj_index;    /* index table for object file */
extern const char *item_types[];
extern const char *extra_bits[];
extern const char *wear_bits[];
extern const char *apply_types[];
extern const char *immunity_names[];
extern const char *affected_bits[];
extern const char *affected_bits2[];
int             functionflag;

char           *obj_edit_menu =
    "    1) Name                    2) Short description\n\r"
    "    3) Description             4) Type\n\r"
    "    5) Wear positions          6) Extra flags\n\r"
    "    7) Weight                  8) Value\n\r"
    "    9) Rent cost              10) Extra affects\n\r"
    "   11) Object values          12) Object Specials\n\r\n\r";

void            ChangeObjWear(struct char_data *ch, char *arg, int type);
void            UpdateObjMenu(struct char_data *ch);
void            ChangeObjName(struct char_data *ch, char *arg, int type);
void            ChangeObjShort(struct char_data *ch, char *arg, int type);
void            ChangeObjDesc(struct char_data *ch, char *arg, int type);
void            ChangeObjType(struct char_data *ch, char *arg, int type);
void            ChangeObjFlags(struct char_data *ch, char *arg, int type);
void            ChangeObjWeight(struct char_data *ch, char *arg, int type);
void            ChangeObjPrice(struct char_data *ch, char *arg, int type);
void            ChangeObjCost(struct char_data *ch, char *arg, int type);
void            ChangeObjAffects(struct char_data *ch, char *arg, int type);
void            ChangeObjAffect(struct char_data *ch, char *arg, int type);
void            ChangeAffectMod(struct char_data *ch, char *arg, int type);
void            ChangeObjValues(struct char_data *ch, char *arg, int type);
void            ChangeObjValue(struct char_data *ch, char *arg, int type);
void            ObjHitReturn(struct char_data *ch, char *arg, int type);
void            ChangeObjSAffect(struct char_data *ch, char *arg, int type);
void            ChangeObjS2Affect(struct char_data *ch, char *arg, int type);
void            ChangeObjSpecials(struct char_data *ch, char *arg, int type);
void            ChangeObjSpecial(struct char_data *ch, char *arg, int type);


void ChangeObjFlags(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 31) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.objedit->obj_flags.extra_flags, i)) {
            REMOVE_BIT(ch->specials.objedit->obj_flags.extra_flags, i);
        } else {
            SET_BIT(ch->specials.objedit->obj_flags.extra_flags, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Object Extra Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 32; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.objedit->obj_flags.extra_flags & (check)) ? 
                 "X" : " "), extra_bits[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 22, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeObjWear(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 19) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.objedit->obj_flags.wear_flags, i)) {
            REMOVE_BIT(ch->specials.objedit->obj_flags.wear_flags, i);
        } else {
            SET_BIT(ch->specials.objedit->obj_flags.wear_flags, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Object Wear Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 20; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.objedit->obj_flags.wear_flags & (check)) ? 
                 "X" : " "), wear_bits[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void do_oedit(struct char_data *ch, char *argument, int cmd)
{
    char            name[20];
    struct obj_data *obj;
    int             i;

    if (IS_NPC(ch)) {
        return;
    }
    if ((IS_NPC(ch)) || (GetMaxLevel(ch) < CREATOR)) {
        return;
    }
    /* 
     * someone is forced to do something. can be bad!
     * the ch->desc->str field will cause problems...
     */
    if (!ch->desc) {
        return;
    }
#if 0    
    if (GetMaxLevel(ch) < GREATER_GOD && 
            !IS_SET(ch->player.user_flags,CAN_OBJ_EDIT)) { 
        send_to_char("You do not have access to object editing.\n\r",ch); 
        return; 
    } 
    /*
     * this has been temporarilly removed... 
     */
#endif
    for (i = 0; *(argument + i) == ' '; i++) {
        /*
         * Empty loop
         */
    }
    if (!*(argument + i)) {
        send_to_char("Oedit what?\n\r", ch);
        return;
    }

    argument = one_argument(argument, name);

    if (!(obj = (struct obj_data *) get_obj_vis_accessible(ch, name))) {
        send_to_char("I don't see that object here.\n\r", ch);
        return;
    }

#if 0
    if (obj_index[obj->item_number].data == NULL) {
        read_object_to_memory(obj_index[obj->item_number].virtual);
    }
    ch->specials.objedit = obj_index[obj->item_number].data;
#else
    
    ch->specials.objedit = obj;
#endif

    ch->specials.oedit = OBJ_MAIN_MENU;
    ch->desc->connected = CON_OBJ_EDITING;

    act("$n has begun editing an object.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;

#if 0
    if(GetMaxLevel(ch)<ROLORD) {
        ch->specials.objedit->mortal_can_use=FALSE;
    }
#endif

    UpdateObjMenu(ch);
}

void UpdateObjMenu(struct char_data *ch)
{
    char            buf[255];
    struct obj_data *obj;

    obj = ch->specials.objedit;

    send_to_char(VT_HOMECLR, ch);
    sprintf(buf, VT_CURSPOS, 1, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Object Name: %s", obj->name);
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, 3, 1);
    send_to_char(buf, ch);
    send_to_char("Menu:\n\r", ch);
    send_to_char(obj_edit_menu, ch);
    send_to_char("--> ", ch);
}

void ObjEdit(struct char_data *ch, char *arg)
{
    if (ch->specials.oedit == OBJ_MAIN_MENU) {
        if (!*arg || *arg == '\n') {
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing an object.", FALSE, ch, 0, 0,
                TO_ROOM);
            GET_POS(ch) = POSITION_STANDING;
            return;
        }
        switch (atoi(arg)) {
        case 0:
            UpdateObjMenu(ch);
            break;
        case 1:
            ch->specials.oedit = CHANGE_OBJ_NAME;
            ChangeObjName(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.oedit = CHANGE_OBJ_SHORT;
            ChangeObjShort(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.oedit = CHANGE_OBJ_DESC;
            ChangeObjDesc(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.oedit = CHANGE_OBJ_TYPE;
            ChangeObjType(ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.oedit = CHANGE_OBJ_WEAR;
            ChangeObjWear(ch, "", ENTER_CHECK);
            break;
        case 6:
            ch->specials.oedit = CHANGE_OBJ_FLAGS;
            ChangeObjFlags(ch, "", ENTER_CHECK);
            break;
        case 7:
            ch->specials.oedit = CHANGE_OBJ_WEIGHT;
            ChangeObjWeight(ch, "", ENTER_CHECK);
            break;
        case 8:
            ch->specials.oedit = CHANGE_OBJ_VALUE;
            ChangeObjPrice(ch, "", ENTER_CHECK);
            break;
        case 9:
            ch->specials.oedit = CHANGE_OBJ_COST;
            ChangeObjCost(ch, "", ENTER_CHECK);
            break;
        case 10:
            ch->specials.oedit = CHANGE_OBJ_AFFECTS;
            ChangeObjAffects(ch, "", ENTER_CHECK);
            break;
        case 11:
            ch->specials.oedit = CHANGE_OBJ_VALUES;
            ChangeObjValues(ch, "", ENTER_CHECK);
            break;
        case 12:
            ch->specials.oedit = CHANGE_OBJ_SPECIAL;
            ChangeObjSpecial(ch, "", ENTER_CHECK);
            break;

        default:
            UpdateObjMenu(ch);
            break;
        }
        return;
    }

    switch (ch->specials.oedit) {
    case CHANGE_OBJ_NAME:
        ChangeObjName(ch, arg, 0);
        break;
    case CHANGE_OBJ_SHORT:
        ChangeObjShort(ch, arg, 0);
        break;
    case CHANGE_OBJ_DESC:
        ChangeObjDesc(ch, arg, 0);
        break;
    case CHANGE_OBJ_WEAR:
        ChangeObjWear(ch, arg, 0);
        break;
    case CHANGE_OBJ_TYPE:
        ChangeObjType(ch, arg, 0);
        break;
    case CHANGE_OBJ_FLAGS:
        ChangeObjFlags(ch, arg, 0);
        break;
    case CHANGE_OBJ_WEIGHT:
        ChangeObjWeight(ch, arg, 0);
        break;
    case CHANGE_OBJ_VALUE:
        ChangeObjPrice(ch, arg, 0);
        break;
    case CHANGE_OBJ_COST:
        ChangeObjCost(ch, arg, 0);
        break;
    case CHANGE_OBJ_AFFECTS:
        ChangeObjAffects(ch, arg, 0);
        break;
    case CHANGE_OBJ_AFFECT1:
    case CHANGE_OBJ_AFFECT2:
    case CHANGE_OBJ_AFFECT3:
    case CHANGE_OBJ_AFFECT4:
    case CHANGE_OBJ_AFFECT5:
        ChangeObjAffect(ch, arg, 0);
        break;
    case CHANGE_AFFECT1_MOD:
    case CHANGE_AFFECT2_MOD:
    case CHANGE_AFFECT3_MOD:
    case CHANGE_AFFECT4_MOD:
    case CHANGE_AFFECT5_MOD:
        ChangeAffectMod(ch, arg, 0);
        break;
    case CHANGE_OBJ_VALUES:
        ChangeObjValues(ch, arg, 0);
        break;
    case CHANGE_OBJ_VALUE1:
    case CHANGE_OBJ_VALUE2:
    case CHANGE_OBJ_VALUE3:
    case CHANGE_OBJ_VALUE4:
        ChangeObjValue(ch, arg, 0);
        break;
    case OBJ_HIT_RETURN:
        ObjHitReturn(ch, arg, 0);
        break;
    case CHANGE_SAFFECT1_MOD:
    case CHANGE_SAFFECT2_MOD:
    case CHANGE_SAFFECT3_MOD:
    case CHANGE_SAFFECT4_MOD:
    case CHANGE_SAFFECT5_MOD:
        ChangeObjSAffect(ch, arg, 0);
        break;
    case CHANGE_S2AFFECT1_MOD:
    case CHANGE_S2AFFECT2_MOD:
    case CHANGE_S2AFFECT3_MOD:
    case CHANGE_S2AFFECT4_MOD:
    case CHANGE_S2AFFECT5_MOD:
        ChangeObjS2Affect(ch, arg, 0);
        break;
    case CHANGE_OBJ_SPECIAL:
        ChangeObjSpecial(ch, arg, 0);
        break;
    case CHANGE_OBJ_EGO:
    case CHANGE_OBJ_MAX:
    case CHANGE_OBJ_SPEED:
    case CHANGE_OBJ_TWEAK:
    case CHANGE_OBJ_WTYPE:
        ChangeObjSpecials(ch, arg, 0);
        break;
    default:
        Log("Got to bad spot in ObjEdit");
        break;
    }
}

void ChangeObjName(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        if (obj->name) {
            free(obj->name);
        }
        obj->name = (char *) strdup(arg);
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Name: %s", obj->name);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Name: ", ch);
}

void ChangeObjShort(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        if (obj->short_description) {
            free(obj->short_description);
        }
        obj->short_description = (char *) strdup(arg);
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Short Description: %s",
            obj->short_description);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Short Description: ", ch);
}

void ChangeObjDesc(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        if (obj->description) {
            free(obj->description);
        }
        obj->description = (char *) strdup(arg);
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Description: %s", obj->description);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Description: ", ch);
}

void ChangeObjType(struct char_data *ch, char *arg, int type)
{
    int             i,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;

    if (type != ENTER_CHECK) {
        switch (ch->specials.oedit) {
        case CHANGE_OBJ_TYPE:
            if (update < 0 || update > 29) {
                return;
            } else {
                ch->specials.objedit->obj_flags.type_flag = update;
                ch->specials.oedit = OBJ_MAIN_MENU;
                UpdateObjMenu(ch);
                return;
            }
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Object Type: %s",
            item_types[(int)ch->specials.objedit->obj_flags.type_flag]);
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 30; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        sprintf(buf, "%-2d %s", i + 1, item_types[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to set to, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeObjWeight(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;
    long            change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0) {
            change = 0;
        }
        obj->obj_flags.weight = change;
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Weight: %d", obj->obj_flags.weight);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Weight: ", ch);
}

void ChangeObjCost(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;
    long            change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0) {
            change = -1;
        }
        obj->obj_flags.cost_per_day = change;
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Rental Cost Per Day: %d",
            obj->obj_flags.cost_per_day);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Rental Cost Per Day: ", ch);
}

void ObjHitReturn(struct char_data *ch, char *arg, int type)
{
    if (type != ENTER_CHECK) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    send_to_char("\n\rHit return: ", ch);
}

void ChangeObjPrice(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct obj_data *obj;
    long            change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    obj = ch->specials.objedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0) {
            change = 0;
        }
        obj->obj_flags.cost = change;
        ch->specials.oedit = OBJ_MAIN_MENU;
        UpdateObjMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Object Value: %d", obj->obj_flags.cost);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Object Value: ", ch);
}

void ChangeObjAffects(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[1024];

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg);
        if (update == 0) {
            ChangeObjAffects(ch, "", ENTER_CHECK);
            return;
        }

        switch (update) {
        case 1:
            ch->specials.oedit = CHANGE_OBJ_AFFECT1;
            ChangeObjAffect(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.oedit = CHANGE_OBJ_AFFECT2;
            ChangeObjAffect(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.oedit = CHANGE_OBJ_AFFECT3;
            ChangeObjAffect(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.oedit = CHANGE_OBJ_AFFECT4;
            ChangeObjAffect(ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.oedit = CHANGE_OBJ_AFFECT5;
            ChangeObjAffect(ch, "", ENTER_CHECK);
            break;
        default:
            ChangeObjAffects(ch, "", ENTER_CHECK);
            break;
        }
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rChange object affect #(1-5) --> ", ch);
}

void ChangeObjAffect(struct char_data *ch, char *arg, int type)
{
    int             update,
                    affect = 0,
                    row = 0,
                    i,
                    a = 0,
                    column = 0,
                    check;
    char            buf[1024];

    switch (ch->specials.oedit) {
    case CHANGE_OBJ_AFFECT1:
        affect = 1;
        break;
    case CHANGE_OBJ_AFFECT2:
        affect = 2;
        break;
    case CHANGE_OBJ_AFFECT3:
        affect = 3;
        break;
    case CHANGE_OBJ_AFFECT4:
        affect = 4;
        break;
    case CHANGE_OBJ_AFFECT5:
        affect = 5;
        break;
    default:
        break;
    }

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg) - 1;

        if (update < 0 || update > 57) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        ch->specials.objedit->affected[affect - 1].location = update;
        ch->specials.objedit->affected[affect - 1].modifier = 0;

        switch (affect) {
        case 1:
            ch->specials.oedit = CHANGE_AFFECT1_MOD;
            break;
        case 2:
            ch->specials.oedit = CHANGE_AFFECT2_MOD;
            break;
        case 3:
            ch->specials.oedit = CHANGE_AFFECT3_MOD;
            break;
        case 4:
            ch->specials.oedit = CHANGE_AFFECT4_MOD;
            break;
        case 5:
            ch->specials.oedit = CHANGE_AFFECT5_MOD;
            break;
        default:
            break;
        }

        sprintf(buf, VT_HOMECLR);
        send_to_char(buf, ch);
        functionflag = FALSE;

        switch (update) {
        case 0:
        case 43:
        case 44:
        case 45:
        case 48:                /* check */
        case 49:                /* check */
        case 56:
            send_to_char("\n\rNote: Modifier does not affect anything in "
                         "this case.\n\r", ch);
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 40:
        case 41:
        case 42:
        case 50:
        case 51:    
        case 52:
        case 53:
        case 54:
        case 55:
            send_to_char("\n\rNote: Modifier will make field go up modifier "
                         "number of points.\n\r", ch);
            send_to_char("      Positive modifier will make field go up, "
                         "negative modifier will make\n\r      field go "
                         "down.\n\r", ch);
            break;
        case 7:
            send_to_char("\n\rNote: Modifier will change characters sex by "
                         "adding.\n\r      0=neutral, 1=male, 2=female\n\r",
                         ch);
            break;
        case 16:                /* check */
            ChangeObjS2Affect(ch, "", ENTER_CHECK);
            functionflag = TRUE;
            break;
        case 26:
        case 27:
        case 28:
            send_to_char("\n\rNote: Modifier should be ADDED together from "
                         "this list of immunity flags.\n\r", ch);
            row = 0;
            for (i = 0; i < 18; i++) {
                sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
                if (i & 1) {
                    row++;
                }
                send_to_char(buf, ch);
                check = 1 << i;

                sprintf(buf, "%-6d :   %s", check, immunity_names[i]);
                send_to_char(buf, ch);
            }
            sprintf(buf, VT_CURSPOS, 20, 1);
            send_to_char(buf, ch);
            break;
        case 29:
            ChangeObjSAffect(ch, "", ENTER_CHECK);
            functionflag = TRUE;
            break;
        case 30:
        case 31:
            send_to_char("\n\rNote: Modifier will be a spell # which you can "
                         "get from the allspells.\n\r", ch);
            break;
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 46:
        case 47:
            send_to_char("\n\rNote: Modifier will affect % learned of "
                         "skill.\n\r", ch);
            send_to_char("      Positive values will increase the % learned "
                         "making the char less likely\n\r      to fail while"
                         " negative numbers will do the opposite.\n\r", ch);
            break;
        }
        if (!functionflag) {
            send_to_char("\n\r\n\rEnter new Modifier (return for 0): ", ch);
            ChangeAffectMod(ch, "", ENTER_CHECK);
        }
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    for (i = 0; i < 57; i++) {
        a++;
        if (a == 1) {
            column = 5;
        } else if (a == 2) {
            column = 30;
        } else if (a == 3) {
            column = 55;
        }
        sprintf(buf, VT_CURSPOS, row + 1, column);
        if (a == 3) {
            row++;
            a = 0;
        }
        send_to_char(buf, ch);
        sprintf(buf, "%-2d %s", i + 1, apply_types[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 21, 1);
    send_to_char(buf, ch);
    send_to_char("Select the apply number or hit enter for the main "
                 "menu.\n\r--> ", ch);
}

void ChangeAffectMod(struct char_data *ch, char *arg, int type)
{
    signed long     update;
    int             affect = 0;
    bool            skill = FALSE;

    switch (ch->specials.oedit) {
    case CHANGE_AFFECT1_MOD:
        affect = 1;
        break;
    case CHANGE_AFFECT2_MOD:
        affect = 2;
        break;
    case CHANGE_AFFECT3_MOD:
        affect = 3;
        break;
    case CHANGE_AFFECT4_MOD:
        affect = 4;
        break;
    case CHANGE_AFFECT5_MOD:
        affect = 5;
        break;
    }

    if (type == ENTER_CHECK) {
        return;
    }
    update = atoi(arg);

    if (update > 32000) {
        update = 0;
    }
    if (ch->specials.objedit->affected[affect - 1].location == 29 ||
        ch->specials.objedit->affected[affect - 1].location == 30 ||
        ch->specials.objedit->affected[affect - 1].location == 31) {
        if (update >= 45 && update <= 52) {
            skill = TRUE;
        }
        if (update >= 120 && update <= 127) {
            skill = TRUE;
        }
        if (update >= 129 && update <= 163) {
            skill = TRUE;
        }
        if (update >= 180 && update <= 187) {
            skill = TRUE;
        }
        if (skill == TRUE) {
            send_to_char("You must use a spell, not a skill!\n\r"
                         "Setting modifier to 1 (armor spell).\n\r", ch);
            update = 1;
        }
    }

    ch->specials.objedit->affected[affect - 1].modifier = update;

    if (skill == TRUE) {
        ch->specials.oedit = OBJ_HIT_RETURN;
        ObjHitReturn(ch, "", ENTER_CHECK);
        return;
    }

    ch->specials.oedit = CHANGE_OBJ_AFFECTS;
    ChangeObjAffects(ch, "", ENTER_CHECK);
}

void ChangeObjValues(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[1024];

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg);
        if (update == 0) {
            ChangeObjValues(ch, "", ENTER_CHECK);
            return;
        }

        switch (update) {
        case 1:
            ch->specials.oedit = CHANGE_OBJ_VALUE1;
            ChangeObjValue(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.oedit = CHANGE_OBJ_VALUE2;
            ChangeObjValue(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.oedit = CHANGE_OBJ_VALUE3;
            ChangeObjValue(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.oedit = CHANGE_OBJ_VALUE4;
            ChangeObjValue(ch, "", ENTER_CHECK);
            break;
        default:
            ChangeObjValues(ch, "", ENTER_CHECK);
            break;
        }
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rChange object value #(1-4) --> ", ch);
}

void ChangeObjSpecial(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[1024];

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg);
        if (update == 0) {
            ChangeObjSpecial(ch, "", ENTER_CHECK);
            return;
        }

        switch (update) {
        case 1:
            ch->specials.oedit = CHANGE_OBJ_EGO;
            ChangeObjSpecials(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.oedit = CHANGE_OBJ_SPEED;
            ChangeObjSpecials(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.oedit = CHANGE_OBJ_MAX;
            ChangeObjSpecials(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.oedit = CHANGE_OBJ_TWEAK;
            ChangeObjSpecials(ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.oedit = CHANGE_OBJ_WTYPE;
            ChangeObjSpecials(ch, "", ENTER_CHECK);
            break;
        default:
            ChangeObjSpecial(ch, "", ENTER_CHECK);
            break;
        }
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rChange object special value #(1-Ego 2-Speed "
                 "3-Loadrate 4-Tweak 5-Weapontype) --> ", ch);
}

void ChangeObjSpecials(struct char_data *ch, char *arg, int type)
{
    int             temp = 0;
    long            update;

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg);

        if (update < 0) {
            ch->specials.oedit = CHANGE_OBJ_SPECIAL;
            ChangeObjSpecial(ch, "", ENTER_CHECK);
            return;
        }

        switch (ch->specials.oedit) {
        case CHANGE_OBJ_EGO:
            if (update < 51 && update >= 0) {
                ch->specials.objedit->level = update;
            } else {
                temp = 1;
            }
            break;
        case CHANGE_OBJ_SPEED:
            if (update < 101 && update >= 0) {
                ch->specials.objedit->speed = update;
            } else {
                temp = 1;
            }
            break;
        case CHANGE_OBJ_MAX:
            if (update < 101 && update >= 0) {
                ch->specials.objedit->max = update;
            } else {
                temp = 1;
            }
            break;
        case CHANGE_OBJ_TWEAK:
            if (update < 101 && update >= 0) {
                ch->specials.objedit->tweak = update;
            } else {
                temp = 1;
            }
            break;
        case CHANGE_OBJ_WTYPE:
            if (update <= WEAPON_LAST - 340
                && update >= WEAPON_FIRST - 340) {
                ch->specials.objedit->weapontype = update;
            } else {
                temp = 1;
            }
            break;
        default:
            break;
        }

        if (temp == 0) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
        }
    }

    switch (ch->specials.oedit) {
    case CHANGE_OBJ_EGO:
        ch_printf(ch, "Please select the level of the item. Param(1-50) "
                      "Current (%d)", ch->specials.objedit->level);
        break;
    case CHANGE_OBJ_SPEED:
        if (IS_WEAPON(ch->specials.objedit)) {
            ch_printf(ch, "Please enter weapon speed: Param(0-100)   "
                          "Current(%d)", ch->specials.objedit->speed);
        } else {
            send_to_char("Can only do this to weapons.\n\r", ch);
            ch->specials.oedit = CHANGE_OBJ_SPECIAL;
            ChangeObjSpecial(ch, "", ENTER_CHECK);
        }
        break;
    case CHANGE_OBJ_MAX:
        ch_printf(ch, "Please enter loadrate of item: Param(0-100)    "
                      "Current(%d)", ch->specials.objedit->max);
        break;
    case CHANGE_OBJ_TWEAK:
        ch_printf(ch, "Please enter the Tweak rating. Param(0-100)    "
                      "Current(%d)", ch->specials.objedit->tweak);
        break;
    case CHANGE_OBJ_WTYPE:
        if (IS_WEAPON(ch->specials.objedit)) {
            ch_printf(ch, "Please enter the Weapon Type. Param(0-59)    "
                          "Current(%d)", ch->specials.objedit->weapontype);
        } else {
            send_to_char("Can only do this to weapons.\n\r", ch);
            ch->specials.oedit = CHANGE_OBJ_SPECIAL;
            ChangeObjSpecial(ch, "", ENTER_CHECK);
        }
        break;
    default:
#if 0
        send_to_char("Bad value",ch);
#endif        
        break;
    }
}

void ChangeObjValue(struct char_data *ch, char *arg, int type)
{
    int             value = 0;
    long            update;
    char            buf[1024];
    bool            skill = FALSE;

    switch (ch->specials.oedit) {
    case CHANGE_OBJ_VALUE1:
        value = 0;
        break;
    case CHANGE_OBJ_VALUE2:
        value = 1;
        break;
    case CHANGE_OBJ_VALUE3:
        value = 2;
        break;
    case CHANGE_OBJ_VALUE4:
        value = 3;
        break;
    }

    if (type != ENTER_CHECK) {
        if (!*arg || (*arg == '\n')) {
            ch->specials.oedit = OBJ_MAIN_MENU;
            UpdateObjMenu(ch);
            return;
        }

        update = atoi(arg);

        if (update < 0) {
            ch->specials.oedit = CHANGE_OBJ_VALUES;
            ChangeObjValues(ch, "", ENTER_CHECK);
            return;
        }

        if ((ch->specials.objedit->obj_flags.type_flag == ITEM_SCROLL && 
             value != 0) || 
            (ch->specials.objedit->obj_flags.type_flag == ITEM_WAND && 
             value == 3) || 
            (ch->specials.objedit->obj_flags.type_flag == ITEM_STAFF && 
             value == 3) || 
            (ch->specials.objedit->obj_flags.type_flag == ITEM_POTION && 
             value != 0)) {
            if (update >= 45 && update <= 52) {
                skill = TRUE;
            }
            if (update >= 171 && update <= 214) {
                skill = TRUE;
            }
            if (update >= 221 && update <= 241) {
                skill = TRUE;
            }
            if (update >= 246 && update <= 254) {
                skill = TRUE;
            }
            if (update == 288) {
                skill = TRUE;
            }
            if (skill == TRUE) {
                send_to_char ("You must use a spell number, not a skill!\n\r"
                              "Setting modifier to 1 (armor spell).\n\r", ch);
                update = 1;
            }
        }

        ch->specials.objedit->obj_flags.value[value] = update;

        if (skill == TRUE) {
            ch->specials.oedit = OBJ_HIT_RETURN;
            ObjHitReturn(ch, "", ENTER_CHECK);
            return;
        }

        ch->specials.oedit = CHANGE_OBJ_VALUES;
        ChangeObjValues(ch, "", ENTER_CHECK);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    switch (ch->specials.objedit->obj_flags.type_flag) {
    case ITEM_LIGHT:
        if (value == 0) {
            send_to_char("\n\rValue1 is the color (Because we all know its "
                         "spelled without a 'u').", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the type.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the number of hours the light source "
                         "will last.", ch);
        } else {
            send_to_char("\n\rValue is not used for this item type.", ch);
        }
        break;
    case ITEM_SCROLL:
        if (value == 0) {
            send_to_char("\n\rValue1 is the level of casting this scroll "
                         "casts.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the first spell this scroll casts.", 
                         ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the second spell this scroll casts.",
                         ch);
        } else {
            send_to_char("\n\rValue4 is the third spell this scroll casts.",
                         ch);
        }
        break;
    case ITEM_WAND:
        if (value == 0) {
            send_to_char("\n\rValue1 is the level of casting this wand casts.",
                         ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the mana the wand takes to use each "
                         "time.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the number of charges the wand has.",
                         ch);
        } else {
            send_to_char("\n\rValue4 is the spell the wand casts.", ch);
        }
        break;
    case ITEM_STAFF:
        if (value == 0) {
            send_to_char("\n\rValue1 is the level of casting this staff "
                         "casts.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the mana the staff takes to use each "
                         "time.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the number of charges the staff has.",
                         ch);
        } else {
            send_to_char("\n\rValue4 is the spell the staff casts.", ch);
        }
        break;
    case ITEM_WEAPON:
        if (value == 0) {
            send_to_char("\n\rValue1 is the plus to hit.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the number of times the damage die is "
                         "rolled.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the number of of faces on damage die.",
                         ch);
        } else {
            send_to_char("\n\rValue4 is the type of weapon damage.\n\r\n\r",
                         ch);
            send_to_char("0  - smite\n\r", ch);
            send_to_char("1  - stab        2 - whip\n\r", ch);
            send_to_char("3  - slash       4 - smash\n\r", ch);
            send_to_char("5  - cleave      6 - crush\n\r", ch);
            send_to_char("7  - pound       8 - claw\n\r", ch);
            send_to_char("9  - bite       10 - sting\n\r", ch);
            send_to_char("11 - pierce     12 - blast\n\r", ch);
#if 0
            send_to_char("13 - smite 14 - impale weapon\n\r",ch);
#endif
            send_to_char("13 - impale     14 - ranged weapon\n\r", ch);
        }
        break;
    case ITEM_FIREWEAPON:
        if (value == 0) {
            send_to_char("\n\rValue1 is the strength required to draw.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the tohit bonus.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the max range [in rooms] and the "
                         "todam.", ch);
        } else {
            send_to_char("\n\rValue4 is the type of weapon damage.\n\r\n\r", 
                         ch);
            send_to_char("1  - hit         2 - pound\n\r", ch);
            send_to_char("3  - pierce      4 - slash\n\r", ch);
            send_to_char("5  - whip        6 - claw\n\r", ch);
            send_to_char("7  - bite        8 - sting\n\r", ch);
            send_to_char("9  - crush      10 - cleave\n\r", ch);
            send_to_char("11 - stab       12 - smash\n\r", ch);
            send_to_char("13 - smite      14 - impale\n\r", ch);
            send_to_char("13 - fire wpn   14 - impale weapon\n\r", ch);
        }
        break;
    case ITEM_MISSILE:
        if (value == 0) {
            send_to_char("\n\rValue1 is the % chance that the missile will "
                         "break.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the number of sides on the damage die.",
                         ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the number of times the damage die is "
                         "rolled.", ch);
        } else {
            send_to_char("\n\rValue4 is the type of weapon damage.\n\r\n\r",
                         ch);
            send_to_char("1  - hit         2 - pound\n\r", ch);
            send_to_char("3  - pierce      4 - slash\n\r", ch);
            send_to_char("5  - whip        6 - claw\n\r", ch);
            send_to_char("7  - bite        8 - sting\n\r", ch);
            send_to_char("9  - crush      10 - cleave\n\r", ch);
            send_to_char("11 - stab       12 - smash\n\r", ch);
            send_to_char("13 - smite      14 - impale\n\r", ch);
        }
        break;
    case ITEM_ARMOR:
        if (value == 0) {
            send_to_char("\n\rValue1 is the Armor Class Apply.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the Full Strengh of the armor.", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the size of the armor.\n\r"
                         " 0=Adjustable,1=Tiny,2=Small,3=Medium,4=large,"
                         "5=Huge,6=Enormous", ch);
        } else {
            send_to_char("\n\rValue is not used for this item type.", ch);
        }
        break;
    case ITEM_POTION:
        if (value == 0) {
            send_to_char("\n\rValue1 is the level of the spells this potion "
                         "casts.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the first spell this potion casts.",
                         ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the second spell this potion casts.",
                         ch);
        } else {
            send_to_char("\n\rValue4 is the third spell this potion casts.",
                         ch);
        }
        break;
    case ITEM_TRAP:
        if (value == 0) {
            send_to_char("\n\rValue1 is the trap damage level.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the attack type of the trap.\n\r\n\r",
                         ch);
            send_to_char(" 10 - blasted          26 - seared\n\r", ch);
            send_to_char(" 67 - corroded        203 - frozen\n\r", ch);
            send_to_char("207 - pounded         208 - pierced\n\r", ch);
            send_to_char("209 - sliced\n\r", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 should be set to 80.", ch);
        } else {
            send_to_char("\n\rValue4 are the number of trap charges "
                    "left.", ch);
        }
        break;
    case ITEM_CONTAINER:
        if (value == 0) {
            send_to_char("\n\rValue1 is the max number of items that can fit.",
                         ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the locktype.\n\r\n\r", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the vnum of the key needed to unlock "
                         "container, 0 for none.", ch);
        } else {
            send_to_char("\n\rValue4 determines whether the container is a "
                         "corpse (1=yes, 0=no).", ch);
        }
        break;
    case ITEM_DRINKCON:
        if (value == 0) {
            send_to_char("\n\rValue1 is the amount of fluid the container can "
                         "hold.", ch);
        } else if (value == 1) {
            send_to_char("\n\rValue2 is the amount of fluid left.\n\r\n\r", ch);
        } else if (value == 2) {
            send_to_char("\n\rValue3 is the type of liquid in the "
                         "container.\n\r\n\r", ch);
            send_to_char(" 0 - water        1 - beer\n\r", ch);
            send_to_char(" 2 - wine         3 - ale\n\r", ch);
            send_to_char(" 4 - dark ale     5 - whisky\n\r", ch);
            send_to_char(" 6 - lemonade     7 - firebreather\n\r", ch);
            send_to_char(" 8 - speciality   9 - slime mold juice\n\r", ch);
            send_to_char("10 - milk        11 - tea\n\r", ch);
            send_to_char("12 - coffee      13 - blood\n\r", ch);
            send_to_char("14 - salt water  15 - coca cola\n\r", ch);
        } else {
            send_to_char("\n\rValue4 determines whether the liquid is poisoned"
                         " (1=yes, 0=no).", ch);
        }
        break;
    case ITEM_NOTE:
        send_to_char("\n\rValue not used for this item type.\n\r", ch);
        break;
    case ITEM_KEY:
        if (value == 0) {
            send_to_char("\n\rValue1 is the keytype.", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_FOOD:
        if (value == 0) {
            send_to_char("\n\rValue1 is how full you get from eating the food.",
                         ch);
        } else if (value == 3) {
            send_to_char("\n\rValue4 is whether the food is poisoned (1=yes, "
                         "0=no).\n\r\n\r", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_AUDIO:
        if (value == 0) {
            send_to_char("\n\rTo set the sound on the item, use the setsound "
                         "command.", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_ROCK:
        if (value == 0) {
            send_to_char("\n\rRock Mobs are determined by weight, not obj "
                         "value.", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_PORTAL:
        if (value == 0) {
            send_to_char("\n\rValue1 is the room to portal to.", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_TREE:
        if (value == 0) {
            send_to_char("\n\rTrees are found by name search, make sure to set"
                         " name before tree keyword.\n\r", ch);
            send_to_char("Example: 'Rufus tree'.  Don't do: 'tree Rufus'.\n\r",
                         ch);
            send_to_char("Value not used for this item type.\n\r", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_INSTRUMENT:
        if (value == 0) {
            send_to_char("\n\rThis value determines the mana reduction of song"
                         " weaving (1-50%).", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    case ITEM_SHIPS_HELM:
        if (value == 0) {
            send_to_char("\n\rX coordinate of ship.", ch);
        } else if (value == 1) {
            send_to_char("\n\rY coordinate of ship.", ch);
        } else if (value == 2) {
            send_to_char("\n\rSpeed of ship.", ch);
        } else if (value == 3) {            
            send_to_char("\n\rSize of ship.", ch);
        } else {
            send_to_char("\n\rValue not used for this item type.\n\r", ch);
        }
        break;
    default:
        send_to_char("Value not used for this item type.\n\r", ch);
        break;
    }

    sprintf(buf, VT_CURSPOS, 21, 1);
    send_to_char(buf, ch);
    send_to_char("Enter the new value (press return for 0).\n\r--> ", ch);
}

void ChangeObjSAffect(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update,
                    affect = 0;
    char            buf[255];

    switch (ch->specials.oedit) {
    case CHANGE_SAFFECT1_MOD:
        affect = 1;
        break;
    case CHANGE_SAFFECT2_MOD:
        affect = 2;
        break;
    case CHANGE_SAFFECT3_MOD:
        affect = 3;
        break;
    case CHANGE_SAFFECT4_MOD:
        affect = 4;
        break;
    case CHANGE_SAFFECT5_MOD:
        affect = 5;
        break;
    default:
        break;
    }

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = CHANGE_OBJ_AFFECTS;
        UpdateObjMenu(ch);
        return;
    }

    if (type == ENTER_CHECK) {
        switch (ch->specials.oedit) {
        case CHANGE_AFFECT1_MOD:
            ch->specials.oedit = CHANGE_SAFFECT1_MOD;
            affect = 1;
            break;
        case CHANGE_AFFECT2_MOD:
            ch->specials.oedit = CHANGE_SAFFECT2_MOD;
            affect = 2;
            break;
        case CHANGE_AFFECT3_MOD:
            ch->specials.oedit = CHANGE_SAFFECT3_MOD;
            affect = 3;
            break;
        case CHANGE_AFFECT4_MOD:
            ch->specials.oedit = CHANGE_SAFFECT4_MOD;
            affect = 4;
            break;
        case CHANGE_AFFECT5_MOD:
            ch->specials.oedit = CHANGE_SAFFECT5_MOD;
            affect = 5;
            break;
        }
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 31) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.objedit->affected[affect - 1].modifier, i)) {
            REMOVE_BIT(ch->specials.objedit->affected[affect - 1].modifier, i);
        } else {
            SET_BIT(ch->specials.objedit->affected[affect - 1].modifier, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Object Spell Affects:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 32; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.objedit->affected[affect-1].modifier & check) ?
                 "X" : " "), affected_bits[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeObjS2Affect(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update,
                    affect = 0;
    char            buf[255];

    switch (ch->specials.oedit) {
    case CHANGE_S2AFFECT1_MOD:
        affect = 1;
        break;
    case CHANGE_S2AFFECT2_MOD:
        affect = 2;
        break;
    case CHANGE_S2AFFECT3_MOD:
        affect = 3;
        break;
    case CHANGE_S2AFFECT4_MOD:
        affect = 4;
        break;
    case CHANGE_S2AFFECT5_MOD:
        affect = 5;
        break;
    default:
        break;
    }

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.oedit = CHANGE_OBJ_AFFECTS;
        UpdateObjMenu(ch);
        return;
    }

    if (type == ENTER_CHECK) {
        switch (ch->specials.oedit) {
        case CHANGE_AFFECT1_MOD:
            ch->specials.oedit = CHANGE_S2AFFECT1_MOD;
            affect = 1;
            break;
        case CHANGE_AFFECT2_MOD:
            ch->specials.oedit = CHANGE_S2AFFECT2_MOD;
            affect = 2;
            break;
        case CHANGE_AFFECT3_MOD:
            ch->specials.oedit = CHANGE_S2AFFECT3_MOD;
            affect = 3;
            break;
        case CHANGE_AFFECT4_MOD:
            ch->specials.oedit = CHANGE_S2AFFECT4_MOD;
            affect = 4;
            break;
        case CHANGE_AFFECT5_MOD:
            ch->specials.oedit = CHANGE_S2AFFECT5_MOD;
            affect = 5;
            break;
        }
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 29) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.objedit->affected[affect - 1].modifier, i)) {
            REMOVE_BIT(ch->specials.objedit->affected[affect - 1].modifier, i);
        } else {
            SET_BIT(ch->specials.objedit->affected[affect - 1].modifier, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Object Spell Affects:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 9; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.objedit->affected[affect - 1].
                  modifier & (check)) ? "X" : " "), affected_bits2[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
