#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

#define MOB_MAIN_MENU         0
#define CHANGE_MOB_NAME       1
#define CHANGE_MOB_SHORT      2
#define CHANGE_MOB_LONG       3
#define CHANGE_MOB_DESC       4
#define CHANGE_MOB_ACT_FLAGS  5
#define CHANGE_MOB_AFF_FLAGS  6
#define CHANGE_MOB_ALIGN      7
#define CHANGE_MOB_LEVEL      8
#define CHANGE_MOB_ARMOR      9
#define CHANGE_MOB_DAMSIZE   10
#define CHANGE_MOB_DAMNUMB   11
#define CHANGE_MOB_DAMPLUS   12
#define CHANGE_MOB_RACE      13
#define CHANGE_MOB_MULTATT   14
#define CHANGE_MOB_EXP       15
#define CHANGE_MOB_DPOS      16
#define CHANGE_MOB_RESIST    17
#define CHANGE_MOB_IMMUNE    18
#define CHANGE_MOB_SUSCEP    19
#define CHANGE_MOB_SOUND     20
#define CHANGE_MOB_DSOUND    21
#define CHANGE_MOB_SEX       22
#define CHANGE_MOB_HITPLUS   23
#define CHANGE_MOB_PROCS     24
#define CHANGE_MOB_TALKS     25
#define CHANGE_MOB_QUEST_YES 26
#define CHANGE_MOB_QUEST_NO  27
#define CHANGE_MOB_HPS       28
#define CHANGE_MOB_COINAGE   29

#define MOB_HIT_RETURN       99

#define ENTER_CHECK        1

extern const char *action_bits[];
extern const char *procedure_bits[];
extern const char *affected_bits[];
extern const char *RaceName[];
extern const char *immunity_names[];

char           *mob_edit_menu1 =
    "    1) Name                    2) Short description\n\r"
    "    3) Long description        4) Description\n\r"
    "    5) Action flags            6) Affect flags\n\r"
    "    7) Alignment               8) Level\n\r"
    "    9) Armor class            10) Damage size of die\n\r"
    "   11) Damage number of die   12) Damage roll plus\n\r"
    "   13) Race                   14) Number of attacks\n\r"
    "   15) Exp flags/amount       16) Default position\n\r";

char           *mob_edit_menu2 =
    "   17) Resistances            18) Immunities\n\r"
    "   19) Susceptibilities       20) Sounds\n\r"
    "   21) Distant sounds         22) Sex\n\r"
    "   23) Hit roll plus          24) Common Procedure\n\r"
    "   25) Talk response          26) Quest solved response\n\r"
    "   27) Quest wrong response   28) Max Hitpoints\n\r"
    "   29) Mobile carried money   \n\r\n\r";

void            ChangeMobName(struct char_data *ch, char *arg, int type);
void            ChangeMobSex(struct char_data *ch, char *arg, int type);
void            ChangeMobShort(struct char_data *ch, char *arg, int type);
void            ChangeMobLong(struct char_data *ch, char *arg, int type);
void            ChangeMobDesc(struct char_data *ch, char *arg, int type);
void            ChangeMobActFlags(struct char_data *ch, char *arg, int type);
void            ChangeMobAffFlags(struct char_data *ch, char *arg, int type);
void            UpdateMobMenu(struct char_data *ch);
void            ChangeMobAlign(struct char_data *ch, char *arg, int type);
void            ChangeMobHitp(struct char_data *ch, char *arg, int type);
void            ChangeMobArmor(struct char_data *ch, char *arg, int type);
void            ChangeMobDamplus(struct char_data *ch, char *arg, int type);
void            ChangeMobDamsize(struct char_data *ch, char *arg, int type);
void            ChangeMobDamnumb(struct char_data *ch, char *arg, int type);
void            ChangeMobMultatt(struct char_data *ch, char *arg, int type);
void            ChangeMobExp(struct char_data *ch, char *arg, int type);
void            ChangeMobDpos(struct char_data *ch, char *arg, int type);
void            MobHitReturn(struct char_data *ch, char *arg, int type);
void            ChangeMobRace(struct char_data *ch, char *arg, int type);
void            ChangeMobImmune(struct char_data *ch, char *arg, int type);
void            ChangeMobResist(struct char_data *ch, char *arg, int type);
void            ChangeMobSuscep(struct char_data *ch, char *arg, int type);
void            ChangeMobSound(struct char_data *ch, char *arg, int type);
void            ChangeMobDsound(struct char_data *ch, char *arg, int type);
void            ChangeMobLevel(struct char_data *ch, char *arg, int type);
void            ChangeMobHitplus(struct char_data *ch, char *arg, int type);
void            ChangeMobProcedureFlags(struct char_data *ch, char *arg,
                                        int type);
void            ChangeMobTalks(struct char_data *ch, char *arg, int type);
void            ChangeMobQuestYes(struct char_data *ch, char *arg, int type);
void            ChangeMobQuestNo(struct char_data *ch, char *arg, int type);
void            ChangeMobHps(struct char_data *ch, char *arg, int type);
void            ChangeMobCoinage(struct char_data *ch, char *arg, int type);

void ChangeMobActFlags(struct char_data *ch, char *arg, int type)
{
    int             i,
                    row,
                    len,
                    update;
    char            buf[255];
    int             totalActionFlags = 32;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;

    if (type != ENTER_CHECK) {
        if (update < 0 || update > 32) {
            return;
        }
        i = 1 << update;

        if (i != ACT_POLYSELF) {
            if (IS_SET(ch->specials.mobedit->specials.act, i))  {
                REMOVE_BIT(ch->specials.mobedit->specials.act, i);
            } else {
                SET_BIT(ch->specials.mobedit->specials.act, i);
            }
        }   
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Action Flags:\n\r\n\r");
    send_to_char(buf, ch);

    row = 0;

    for (i = 0; i < totalActionFlags; i++) {
        len = sprintf(buf, "%5s%-2d [%s] %s", "", i + 1,
                      ((ch->specials.mobedit->specials.act & (1 << i)) ? "X" :
                        " "), action_bits[i]);
        send_to_char(buf, ch);
        sprintf(buf, "%*s", ((i & 1) ? 2 : 38 - len), ((i & 1) ? "\n\r" : ""));
        send_to_char(buf, ch);
    }

    send_to_char("\n\r", ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeMobAffFlags(struct char_data *ch, char *arg, int type)
{
    int             row,
                    update;
    char            buf[255];
    long            i,
                    check = 0;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 31) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.mobedit->specials.affected_by, i)) {
            REMOVE_BIT(ch->specials.mobedit->specials.affected_by, i);
        } else {
            SET_BIT(ch->specials.mobedit->specials.affected_by, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Affect Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 32; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2ld [%s] %s", i + 1,
                ((ch->specials.mobedit->specials.affected_by & (check)) ? 
                 "X" : " "), affected_bits[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 21, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void do_medit(struct char_data *ch, char *argument, int cmd)
{
    char            name[20];
    struct char_data *mob;
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
    for (i = 0; *(argument + i) == ' '; i++) {
        /* 
         * Empty loop 
         */
    }

    if (!*(argument + i)) {
        send_to_char("Medit who?\n\r", ch);
        return;
    }

    argument = one_argument(argument, name);

    if (!(mob = (struct char_data *) get_char_room_vis(ch, name))) {
        send_to_char("I don't see that mobile here.\n\r", ch);
        return;
    }

    if (IS_PC(mob)) {
        send_to_char("You can not mobedit players.\n\r", ch);
        return;
    }
#if 0
    if (GetMaxLevel(ch) < GOD && !IS_SET(ch->player.user_flags,CAN_MOB_EDIT)) {         send_to_char("You do not have access to edit mobiles.\n\r",ch); 
        return; 
    } 
    /* 
     * this was
     * annoying for low-level builders
     */
#endif
    ch->specials.mobedit = mob;
    ch->specials.medit = MOB_MAIN_MENU;
    ch->desc->connected = CON_MOB_EDITING;

    act("$n has begun editing a mobile.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;

    UpdateMobMenu(ch);
}

void UpdateMobMenu(struct char_data *ch)
{
    char            buf[255];
    struct char_data *mob;

    mob = ch->specials.mobedit;

    send_to_char(VT_HOMECLR, ch);
    sprintf(buf, VT_CURSPOS, 1, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Name: %s", GET_NAME(mob));
    send_to_char(buf, ch);
    sprintf(buf, VT_CURSPOS, 3, 1);
    send_to_char(buf, ch);
    send_to_char("Menu:\n\r", ch);
    send_to_char(mob_edit_menu1, ch);
    send_to_char(mob_edit_menu2, ch);
    send_to_char("--> ", ch);
}

void MobEdit(struct char_data *ch, char *arg)
{
    if (ch->specials.medit == MOB_MAIN_MENU) {
        if (!*arg || *arg == '\n') {
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing a mobile.", FALSE, ch, 0, 0,
                TO_ROOM);
            GET_POS(ch) = POSITION_STANDING;
            return;
        }

        switch (atoi(arg)) {
        case 0:
            UpdateMobMenu(ch);
            break;
        case 1:
            ch->specials.medit = CHANGE_MOB_NAME;
            ChangeMobName(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.medit = CHANGE_MOB_SHORT;
            ChangeMobShort(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.medit = CHANGE_MOB_LONG;
            ChangeMobLong(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.medit = CHANGE_MOB_DESC;
            ChangeMobDesc(ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.medit = CHANGE_MOB_ACT_FLAGS;
            ChangeMobActFlags(ch, "", ENTER_CHECK);
            break;
        case 6:
            ch->specials.medit = CHANGE_MOB_AFF_FLAGS;
            ChangeMobAffFlags(ch, "", ENTER_CHECK);
            break;
        case 7:
            ch->specials.medit = CHANGE_MOB_ALIGN;
            ChangeMobAlign(ch, "", ENTER_CHECK);
            break;
        case 8:
            ch->specials.medit = CHANGE_MOB_LEVEL;
            ChangeMobLevel(ch, "", ENTER_CHECK);
            break;
        case 9:
            ch->specials.medit = CHANGE_MOB_ARMOR;
            ChangeMobArmor(ch, "", ENTER_CHECK);
            break;
        case 10:
            ch->specials.medit = CHANGE_MOB_DAMSIZE;
            ChangeMobDamsize(ch, "", ENTER_CHECK);
            break;
        case 11:
            ch->specials.medit = CHANGE_MOB_DAMNUMB;
            ChangeMobDamnumb(ch, "", ENTER_CHECK);
            break;
        case 12:
            ch->specials.medit = CHANGE_MOB_DAMPLUS;
            ChangeMobDamplus(ch, "", ENTER_CHECK);
            break;
        case 13:
            ch->specials.medit = CHANGE_MOB_RACE;
            ChangeMobRace(ch, "", ENTER_CHECK);
            break;
        case 14:
            ch->specials.medit = CHANGE_MOB_MULTATT;
            ChangeMobMultatt(ch, "", ENTER_CHECK);
            break;
        case 15:
            ch->specials.medit = CHANGE_MOB_EXP;
            ChangeMobExp(ch, "", ENTER_CHECK);
            break;
        case 16:
            ch->specials.medit = CHANGE_MOB_DPOS;
            ChangeMobDpos(ch, "", ENTER_CHECK);
            break;
        case 17:
            ch->specials.medit = CHANGE_MOB_RESIST;
            ChangeMobResist(ch, "", ENTER_CHECK);
            break;
        case 18:
            ch->specials.medit = CHANGE_MOB_IMMUNE;
            ChangeMobImmune(ch, "", ENTER_CHECK);
            break;
        case 19:
            ch->specials.medit = CHANGE_MOB_SUSCEP;
            ChangeMobSuscep(ch, "", ENTER_CHECK);
            break;
        case 20:
            ch->specials.medit = CHANGE_MOB_SOUND;
            ChangeMobSound(ch, "", ENTER_CHECK);
            break;
        case 21:
            ch->specials.medit = CHANGE_MOB_DSOUND;
            ChangeMobDsound(ch, "", ENTER_CHECK);
            break;
        case 22:
            ch->specials.medit = CHANGE_MOB_SEX;
            ChangeMobSex(ch, "", ENTER_CHECK);
            break;
        case 23:
            ch->specials.medit = CHANGE_MOB_HITPLUS;
            ChangeMobHitplus(ch, "", ENTER_CHECK);
            break;
        case 24:
            ch->specials.medit = CHANGE_MOB_PROCS;
            ChangeMobProcedureFlags(ch, "", ENTER_CHECK);
            break;
        case 25:
            ch->specials.medit = CHANGE_MOB_TALKS;
            ChangeMobTalks(ch, "", ENTER_CHECK);
            break;
        case 26:
            ch->specials.medit = CHANGE_MOB_QUEST_YES;
            ChangeMobQuestYes(ch, "", ENTER_CHECK);
            break;
        case 27:
            ch->specials.medit = CHANGE_MOB_QUEST_NO;
            ChangeMobQuestNo(ch, "", ENTER_CHECK);
            break;
        case 28:
            ch->specials.medit = CHANGE_MOB_HPS;
            ChangeMobHps(ch, "", ENTER_CHECK);
            break;
        case 29:
            ch->specials.medit = CHANGE_MOB_COINAGE;
            ChangeMobCoinage(ch, "", ENTER_CHECK);
            break;
        default:
            UpdateMobMenu(ch);
            break;
        }
        return;
    }

    switch (ch->specials.medit) {
    case CHANGE_MOB_NAME:
        ChangeMobName(ch, arg, 0);
        break;
    case CHANGE_MOB_SHORT:
        ChangeMobShort(ch, arg, 0);
        break;
    case CHANGE_MOB_LONG:
        ChangeMobLong(ch, arg, 0);
        break;
    case CHANGE_MOB_DESC:
        ChangeMobDesc(ch, arg, 0);
        break;
    case CHANGE_MOB_ACT_FLAGS:
        ChangeMobActFlags(ch, arg, 0);
        break;
    case CHANGE_MOB_AFF_FLAGS:
        ChangeMobAffFlags(ch, arg, 0);
        break;
    case CHANGE_MOB_ALIGN:
        ChangeMobAlign(ch, arg, 0);
        break;
    case CHANGE_MOB_ARMOR:
        ChangeMobArmor(ch, arg, 0);
        break;
    case CHANGE_MOB_DAMSIZE:
        ChangeMobDamsize(ch, arg, 0);
        break;
    case CHANGE_MOB_DAMNUMB:
        ChangeMobDamnumb(ch, arg, 0);
        break;
    case CHANGE_MOB_DAMPLUS:
        ChangeMobDamplus(ch, arg, 0);
        break;
    case CHANGE_MOB_RACE:
        ChangeMobRace(ch, arg, 0);
        break;
    case CHANGE_MOB_MULTATT:
        ChangeMobMultatt(ch, arg, 0);
        break;
    case CHANGE_MOB_EXP:
        ChangeMobExp(ch, arg, 0);
        break;
    case CHANGE_MOB_DPOS:
        ChangeMobDpos(ch, arg, 0);
        break;
    case CHANGE_MOB_RESIST:
        ChangeMobResist(ch, arg, 0);
        break;
    case CHANGE_MOB_IMMUNE:
        ChangeMobImmune(ch, arg, 0);
        break;
    case CHANGE_MOB_SUSCEP:
        ChangeMobSuscep(ch, arg, 0);
        break;
    case CHANGE_MOB_SOUND:
        ChangeMobSound(ch, arg, 0);
        break;
    case CHANGE_MOB_DSOUND:
        ChangeMobDsound(ch, arg, 0);
        break;
    case MOB_HIT_RETURN:
        MobHitReturn(ch, arg, 0);
        break;
    case CHANGE_MOB_LEVEL:
        ChangeMobLevel(ch, arg, 0);
        break;
    case CHANGE_MOB_SEX:
        ChangeMobSex(ch, arg, 0);
        break;
    case CHANGE_MOB_HITPLUS:
        ChangeMobHitplus(ch, arg, 0);
        break;
    case CHANGE_MOB_PROCS:
        ChangeMobProcedureFlags(ch, arg, 0);
        break;
    case CHANGE_MOB_TALKS:
        ChangeMobTalks(ch, arg, 0);
        break;
    case CHANGE_MOB_QUEST_YES:
        ChangeMobQuestYes(ch, arg, 0);
        break;
    case CHANGE_MOB_QUEST_NO:
        ChangeMobQuestNo(ch, arg, 0);
        break;
    case CHANGE_MOB_HPS:
        ChangeMobHps(ch, arg, 0);
        break;
    case CHANGE_MOB_COINAGE:
        ChangeMobCoinage(ch, arg, 0);
        break;
    default:
        Log("Got to bad spot in MobEdit");
        break;
    }
}

void ChangeMobName(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK&& (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        if (mob->player.name) {
            free(mob->player.name);
        }
        mob->player.name = (char *) strdup(arg);
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Name: %s", GET_NAME(mob));
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Mobile Name: ", ch);
}

void ChangeMobShort(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        if (mob->player.short_descr) {
            free(mob->player.short_descr);
        }
        mob->player.short_descr = (char *) strdup(arg);
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Short Description: %s",
            mob->player.short_descr);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Mobile Short Description: ", ch);
}

void ChangeMobLong(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        if (mob->player.long_descr) {
            free(mob->player.long_descr);
        }
        mob->player.long_descr = (char *) strdup(arg);
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Long Description: %s",
            mob->player.long_descr);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Mobile Long Description: ", ch);
}

void ChangeMobDesc(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    sprintf(buf, "Current Mobile Description:\n\r");
    send_to_char(buf, ch);
    send_to_char(mob->player.description, ch);
    send_to_char("\n\r\n\rNew Mobile Description:\n\r", ch);
    send_to_char("(Use /? for help on editing strings. Press <C/R> again to"
                 " continue)\n\r", ch);
    if (mob->player.description) {
        free(mob->player.description);
    }
    mob->player.description = NULL;
    ch->desc->str = &mob->player.description;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobAlign(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    long            change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < -1000 || change > 1000) {
            change = 0;
        }
        GET_ALIGNMENT(mob) = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Alignment: %d", GET_ALIGNMENT(mob));
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Alignment [between -1000[evil] and 1000[good]: ",
                 ch);
}


void ChangeMobArmor(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    signed int      change;

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < -100 || change > 100) {
            change = 0;
        }
        mob->points.armor = change;
        sprintf(buf, "%d %d", change, mob->points.armor);
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Armor Class: %d", mob->points.armor);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Mob Armor Class[between -100[armored] and "
                 "100[naked]: ", ch);
}

void ChangeMobDamplus(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change <= 0 || change > 255) {
            change = 1;
        }
        mob->points.damroll = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Damroll Plus: %d", mob->points.damroll);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Damroll Plus: ", ch);
}

void ChangeMobDamsize(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change <= 0 || change > 255) {
            change = 1;
        }
        mob->specials.damsizedice = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Damage Dice Number of Sides: %d",
            mob->specials.damsizedice);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Damage Dice Number of Sides: ", ch);
}

void ChangeMobDamnumb(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change <= 0) {
            change = 1;
        }
        if (change <= 0 || change > 255) {
            change = 1;
        }
        mob->specials.damnodice = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Number of Times Damage Dice is rolled: %d",
            mob->specials.damnodice);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Number of Times Damage Dice is rolled: ", ch);
}

void ChangeMobMultatt(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    float           change;

    mob = ch->specials.mobedit;

    if (type != ENTER_CHECK &&  
        (!*arg || *arg == '\n' || mob->specials.mobtype == 'S')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
#if 0
        change=atoi(arg); 
        /* make an argument into an integer?! 
         * no wonder it won't take floating 
         * #s -Lennya 
         */
#endif       
        change = arg_to_float(arg);
        if (change < 0.0) {
            change = 0.0;
        }
        mob->mult_att = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    if (mob->specials.mobtype == 'S') {
        send_to_char("This is a simple mobile, and does not have a number of"
                     " attacks field.\n\rHit return.\n\r\n\r", ch);
        ch->specials.medit = MOB_HIT_RETURN;
        MobHitReturn(ch, "", ENTER_CHECK);
        return;
    }
    sprintf(buf, "Current Mob Number of Attacks: %.1f", mob->mult_att);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Number of Attacks: ", ch);
}

void MobHitReturn(struct char_data *ch, char *arg, int type)
{
    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    send_to_char("\n\rHit return: ", ch);
}

void ChangeMobExp(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    long            change,
                    expamount;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0) {
            change = 0;
        }
        if (mob->specials.mobtype == 'S') {
            GET_EXP(mob) = change;
        } else {
            if (change > 32) {
                send_to_char("That expflag is too high!\n\r", ch);
                ch->specials.medit = MOB_HIT_RETURN;
                MobHitReturn(ch, "", ENTER_CHECK);
                return;
            }
            mob->specials.exp_flag = change;
            expamount = (DetermineExp(mob, mob->specials.exp_flag) + 
                         mob->points.gold);
            sprintf(buf, "This mobile will have around %ld exp the next time "
                         "it is loaded.\n\r", expamount);
            send_to_char(buf, ch);
            GET_EXP(mob) = expamount;
            ch->specials.medit = MOB_HIT_RETURN;
            MobHitReturn(ch, "", ENTER_CHECK);
            return;
        }

        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    if (mob->specials.mobtype == 'S') {
        sprintf(buf, "Current Mob Exp: %d", GET_EXP(mob));
    } else {
        sprintf(buf, "Current Mob Expflag: %ld", mob->specials.exp_flag);
    }
    send_to_char(buf, ch);
    if (mob->specials.mobtype == 'S') {
        send_to_char("\n\r\n\rNew Exp Amount: ", ch);
    } else {
        send_to_char("\n\r\n\rNew Expflag: ", ch);
    }
}

void ChangeMobDpos(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0 || change > 9) {
            change = 0;
        }
        mob->specials.default_pos = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    send_to_char("Positions:\n\r", ch);
    send_to_char(" 0 - dead            1 - mortally wounded\n\r", ch);
    send_to_char(" 2 - incapacitated   3 - stunned\n\r", ch);
    send_to_char(" 4 - sleeping        5 - resting\n\r", ch);
    send_to_char(" 6 - sitting         7 - fighting\n\r", ch);
    send_to_char(" 8 - standing        9 - mounted\n\r\n\r\n\r", ch);
    sprintf(buf, "Current Mob Default Position: %d",
            mob->specials.default_pos);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Default Position: ", ch);
}

void ChangeMobRace(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);

    if (type != ENTER_CHECK) {
        switch (ch->specials.medit) {
        case CHANGE_MOB_RACE:
            if (update < 0 || update > MAX_RACE) {
                return;
            } else {
                GET_RACE(ch->specials.mobedit) = update;
                ch->specials.medit = MOB_MAIN_MENU;
                UpdateMobMenu(ch);
                return;
            }
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile race: %s", RaceName[GET_RACE(ch->specials.mobedit)]);
    send_to_char(buf, ch);

    sprintf(buf, VT_CURSPOS, 21, 1);
    send_to_char(buf, ch);
    send_to_char("Select the race number to set to from the help allrace list,"
                 " <C/R> to return to main menu.\n\r--> ", ch);
}

void ChangeMobResist(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 17) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.mobedit->immune, i)) {
            REMOVE_BIT(ch->specials.mobedit->immune, i);
        } else {
            SET_BIT(ch->specials.mobedit->immune, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Resistances Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 18; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.mobedit->immune & (check)) ? "X" : " "),
                immunity_names[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeMobImmune(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 17) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.mobedit->M_immune, i)) {
            REMOVE_BIT(ch->specials.mobedit->M_immune, i);
        } else {
            SET_BIT(ch->specials.mobedit->M_immune, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Immunities Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 18; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.mobedit->M_immune & (check)) ? "X" : " "),
                immunity_names[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeMobSuscep(struct char_data *ch, char *arg, int type)
{
    int             i,
                    check = 0,
                    row,
                    update;
    char            buf[255];

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    update = atoi(arg);
    update--;
    if (type != ENTER_CHECK) {
        if (update < 0 || update > 17) {
            return;
        }
        i = 1 << update;

        if (IS_SET(ch->specials.mobedit->susc, i)) {
            REMOVE_BIT(ch->specials.mobedit->susc, i);
        } else {
            SET_BIT(ch->specials.mobedit->susc, i);
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Mobile Susceptibilities Flags:");
    send_to_char(buf, ch);

    row = 0;
    for (i = 0; i < 18; i++) {
        sprintf(buf, VT_CURSPOS, row + 4, ((i & 1) ? 45 : 5));
        if (i & 1) {
            row++;
        }
        send_to_char(buf, ch);
        check = 1 << i;

        sprintf(buf, "%-2d [%s] %s", i + 1,
                ((ch->specials.mobedit->susc & (check)) ? "X" : " "),
                immunity_names[i]);
        send_to_char(buf, ch);
    }

    sprintf(buf, VT_CURSPOS, 20, 1);
    send_to_char(buf, ch);
    send_to_char("Select the number to toggle, <C/R> to return to main "
                 "menu.\n\r--> ", ch);
}

void ChangeMobSound(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    
#if 0
    if(mob->specials.mobtype!='L') { 
        send_to_char("Please use a base mobile that already has "
                "sound, this one does not.\n\r",ch);
        ch->specials.medit = MOB_HIT_RETURN; MobHitReturn(ch, "", 
                ENTER_CHECK); 
        return; 
    } 
#endif
    
    if (mob->player.sounds != 0) {
        sprintf(buf, "Current Mobile Sound:\n\r");
        send_to_char(buf, ch);
        send_to_char(mob->player.sounds, ch);
    }
    send_to_char("\n\r\n\rNew Mobile Sound:\n\r", ch);
    send_to_char("(Terminate with a /w on a new line. Press <C/R> again to"
                 " continue)\n\r", ch);
    if (mob->player.sounds) {
        free(mob->player.sounds);
    }
    mob->player.sounds = NULL;
    ch->desc->str = &mob->player.sounds;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobDsound(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    
#if 0
    if (mob->specials.mobtype!='L') { 
        send_to_char("Please use a base mobile that already has sound, "
                "this one does not.\n\r",ch);
        ch->specials.medit = MOB_HIT_RETURN; MobHitReturn(ch, "", 
                ENTER_CHECK); 
        return; 
    }
#endif
          
    if (mob->player.distant_snds != 0) {
        sprintf(buf, "Current Mobile Distant Sound:\n\r");
        send_to_char(buf, ch);
        send_to_char(mob->player.distant_snds, ch);
    }

    send_to_char("\n\r\n\rNew Mobile Distant Sound:\n\r", ch);
    send_to_char("(Terminate with a /w on a new line. Press <C/R> again to "
                 "continue)\n\r", ch);
    if (mob->player.distant_snds) {
        free(mob->player.distant_snds);
    }
    mob->player.distant_snds = NULL;
    ch->desc->str = &mob->player.distant_snds;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobLevel(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    long            change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 1 || change > 60) {
            change = 1;
        }
        GET_LEVEL(mob, WARRIOR_LEVEL_IND) = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Level: %d", GET_LEVEL(mob, WARRIOR_LEVEL_IND));
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew level [between 1 and 60]: ", ch);
}

void ChangeMobSex(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    char           *Sex[] = { "Neutral", "Male", "Female" };

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        switch (arg[0]) {
        case 'n':
        case 'N':
            GET_SEX(mob) = SEX_NEUTRAL;
            break;
        case 'f':
        case 'F':
            GET_SEX(mob) = SEX_FEMALE;
            break;
        case 'm':
        case 'M':
            GET_SEX(mob) = SEX_MALE;
            break;
        default:
            GET_SEX(mob) = SEX_NEUTRAL;
            break;
        };

        ch->specials.medit = MOB_MAIN_MENU;

        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile sex is : %s", Sex[(int)GET_SEX(mob)]);
    send_to_char(buf, ch);
    send_to_char("\n\rEnter Mobile Sex as N=neutral F=female or M=male ", ch);
    send_to_char("\n\r\n\rNew Mobile Sex: ", ch);
}

void ChangeMobHitplus(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change <= 0 || change > 255) {
            change = 1;
        }
        mob->points.hitroll = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mob Hitroll Plus: %d", mob->points.hitroll);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Hitroll Plus: ", ch);
}

void ChangeMobProcedureFlags(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0 || change > 20) {
            change = 0;
        }
        mob->specials.proc = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Common Procedure: %s\n\r\n\r",
            procedure_bits[mob->specials.proc]);
    send_to_char(buf, ch);
    send_to_char("  0) None                             1) Shopkeeper\n\r", ch);
    send_to_char("  2) Guildmaster                      3) Swallower\n\r", ch);
    send_to_char("  4) Drainer                          5) Quest\n\r", ch);
    send_to_char("  6) Old Breath Weapon                7) Fire Breather\n\r",
                 ch);
    send_to_char("  8) Gas Breather                     9) Frost Breather "
                 "(fixed!)\n\r", ch);
    send_to_char(" 10) Acid Breather                   11) Electric "
                 "Breather\n\r", ch);
    send_to_char(" 12) Dehydration Breather            13) Vapor Breather\n\r",
                 ch);
    send_to_char(" 14) Sound Breather                  15) Shard Breather\n\r",
                 ch);
    send_to_char(" 16) Sleep Breather                  17) Light Breather\n\r",
                 ch);
    send_to_char(" 18) Dark Breather                   19) Receptionist\n\r",
                 ch);
    send_to_char(" 20) Repair Guy\n\r", ch);

    send_to_char("\n\r", ch);
    send_to_char("For Guildmasters: set (one!) class in Action Flags, and "
                 "mob\n\r", ch);
    send_to_char("level determines up to which level this GM will "
                 "train.\n\r\n\r", ch);

    send_to_char("For Quest mobs: If player gives item with vnum equal to "
                 "this\n\r", ch);
    send_to_char("mobile's vnum to this mobile, mobile will in turn load "
                 "object\n\r", ch);
    send_to_char("with vnum+1 (if any such thing exists) and give it to "
                 "player. \n\r", ch);
    send_to_char("It will also send the texts in QuestSolvedResponse to "
                 "the\n\r", ch);
    send_to_char("room, or QuestWrongResponse if player tries to return wrong "
                 "item.\n\r", ch);

    send_to_char("\n\r", ch);
    send_to_char("Procedure will start working after saving and loading this "
                 "mobile.\n\r\n\r", ch);
    send_to_char("New Mobile Common Procedure: ", ch);
}

void ChangeMobTalks(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    sprintf(buf, "Current mobile talk response:\n\r");
    send_to_char(buf, ch);
    send_to_char(mob->specials.talks, ch);
    send_to_char("\n\r\n\rNew mobile talk response:\n\r", ch);
    send_to_char("\n\rTo delete, use /w only.\n\r", ch);
    send_to_char("Terminate with a /w on a new line. Press <C/R> again to "
                 "continue. /? for more help.\n\r", ch);
    if (mob->specials.talks) {
        free(mob->specials.talks);
    }
    mob->specials.talks = NULL;
    ch->desc->str = &mob->specials.talks;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobQuestYes(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    send_to_char("This function only works if the common Quest procedure is "
                 "set.\n\r", ch);
    sprintf(buf, "Current mobile gave-correct-quest-item response:\n\r");
    send_to_char(buf, ch);
    send_to_char(mob->specials.quest_yes, ch);
    send_to_char("\n\r\n\rNew mobile gave-correct-quest-item response:\n\r",
                 ch);
    send_to_char("(Terminate with a /w on a new line. Press <C/R> again to "
                 "continue)\n\r", ch);
    if (mob->specials.quest_yes) {
        free(mob->specials.quest_yes);
    }
    mob->specials.quest_yes = NULL;
    ch->desc->str = &mob->specials.quest_yes;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobQuestNo(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;

    if (type != ENTER_CHECK) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    mob = ch->specials.mobedit;
    send_to_char ("This function only works if the common Quest procedure is "
                  "set.\n\r", ch);
    sprintf(buf, "Current mobile gave-wrong-quest-item response:\n\r");
    send_to_char(buf, ch);
    send_to_char(mob->specials.quest_no, ch);
    send_to_char("\n\r\n\rNew mobile gave-wrong-quest-item response:\n\r", ch);
    send_to_char("(Terminate with a /w on a new line. Press <C/R> again to "
                 "continue)\n\r", ch);
    if (mob->specials.quest_no) {
        free(mob->specials.quest_no);
    }
    mob->specials.quest_no = NULL;
    ch->desc->str = &mob->specials.quest_no;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeMobHps(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 1 || change > 15000) {
            change = 1;
        }
        mob->points.max_hit = change;
        mob->points.hit = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Hps: %d\n\r\n\r", mob->points.max_hit);
    send_to_char("Pick number between 0 and 15000.\n\r\n\r", ch);
    send_to_char("New Mobile Hps:\n\r", ch);
}

void ChangeMobCoinage(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct char_data *mob;
    int             change;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    mob = ch->specials.mobedit;
    if (type != ENTER_CHECK) {
        change = atoi(arg);
        if (change < 0 || change > 20000) {
            change = 1;
        }
        mob->points.gold = change;
        ch->specials.medit = MOB_MAIN_MENU;
        UpdateMobMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current Mobile Money: %d\n\r\n\r", mob->points.gold);
    send_to_char("Pick number ranging from 0 to 20000.\n\r", ch);
    send_to_char("Mob will load with 0.9 to 1.1 times the entered "
                 "amount.\n\r\n\r", ch);
    send_to_char("New Mobile Money:\n\r", ch);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
