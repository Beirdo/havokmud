/*
 * Online helpfile editor. Reads, edits, and saves existing files.
 * Creates new files. Done by Lennya, Sept 2003 testline testline for
 * v2.0 
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "protos.h"

extern FILE    *wizhelp_fl;
extern FILE    *help_fl;
extern struct help_index_element *wizhelp_index;
extern struct help_index_element *help_index;
extern int      top_of_helpt;
extern int      top_of_wizhelpt;

#define HELP_MAIN_MENU            0
#define CHANGE_HELP_NAME          1
#define CHANGE_HELP_USAGE         2
#define CHANGE_HELP_ACCUMULATIVE  3
#define CHANGE_HELP_DURATION      4
#define CHANGE_HELP_LEVEL         5
#define CHANGE_HELP_DAMTYPE       6
#define CHANGE_HELP_SAVES         7
#define CHANGE_HELP_DESCRIPTION   8
#define CHANGE_HELP_REFERENCES    9
#define CHANGE_HELP_WIZARD       10

#define HELP_HIT_RETURN          15

#define ENTER_CHECK        1

char           *help_edit_menu =
    "    1) Name(s) of file         2) Usage\n\r"
    "    3) Accumulative?           4) Duration?\n\r"
    "    5) Level?                  6) Damagetype?\n\r"
    "    7) Saving Throw?           8) Description\n\r"
    "    9) References             10) Wizard Only?\n\r\n\r"
    "   99) Save file and exit    100) Don't save and exit\n\r\n\r";
int             is_help_elem(struct help_file_u *hlp, char *arg);
int             read_help_from_file(struct char_data *ch, char *argument,
                                    int cmd);
int             write_one_help(struct char_data *ch, FILE * f_temp);
int             write_help_to_file(struct char_data *ch,
                                   struct help_file_u *hlp);
void            ChangeHelpName(struct char_data *ch, char *arg, int type);
void            ChangeHelpUsage(struct char_data *ch, char *arg, int type);
void            ChangeHelpAccumulative(struct char_data *ch, char *arg,
                                       int type);
void            ChangeHelpDuration(struct char_data *ch, char *arg,
                                   int type);
void            ChangeHelpLevel(struct char_data *ch, char *arg, int type);
void            ChangeHelpDamtype(struct char_data *ch, char *arg,
                                  int type);
void            ChangeHelpSaves(struct char_data *ch, char *arg, int type);
void            ChangeHelpDescription(struct char_data *ch, char *arg,
                                      int type);
void            ChangeHelpReferences(struct char_data *ch, char *arg,
                                     int type);
void            ChangeHelpWizard(struct char_data *ch, char *arg,
                                 int type);
void            ChangeHelpSave(struct char_data *ch, char *arg, int type);
struct help_index_element *build_help_index(FILE *fl, int *num);


void do_hedit(struct char_data *ch, char *argument, int cmd)
{
    char            name[30];
    struct help_file_u *hlp;

    if (IS_NPC(ch)) {
        return;
    }
    if ((IS_NPC(ch)) || !IS_IMMORTAL(ch)) {
        return;
    }

    /* 
     * someone is forced to do something. can be bad!
     * the ch->desc->str field will cause problems... 
     */
    if (!ch->desc) {
        return;
    }

    CREATE(hlp, struct help_file_u, 1);
    ch->specials.help = hlp;
    if (argument && !(read_help_from_file(ch, argument, 2))) {
        /*
         * it's a new help file 
         */
        hlp->newfile = 1;
        strcpy(name, argument);
        hlp->name = (char *) strdup(name);
    }

    ch->specials.help = hlp;
    ch->specials.hedit = HELP_MAIN_MENU;
    ch->desc->connected = CON_HELP_EDITING;
    act("$n has begun editing a helpfile.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;
    UpdateHelpMenu(ch);
} 

/*
 * checks both old and new help files for most common formats, and 
 * writes them into their own place in the help_file_u format 
 */
int is_help_elem(struct help_file_u *hlp, char *arg)
{
    int             i;
    char            arg1[120],
                    arg2[120],
                    arg3[120];

    /*
     * new file formats 
     */
    char           *usage = "   Usage        : ";
    char           *accum = "   Accumulative : ";
    char           *durat = "   Duration     : ";
    char           *level = "   Level        : ";
    char           *damag = "   Damagetype   : ";
    char           *saves = "   Save         : ";
    char           *refer = "Also see";

    /*
     * other common formats 
     */
    char           *usag2 = "Usage";
    char           *accu2 = "Accumulative: ";
    char           *dura2 = "Duration";
    char           *leve2 = "Level";
    char           *save2 = "Save";
    char           *refe2 = "See also";
    char           *refe3 = "See Also";
    char           *refe4 = "Also See";
    char           *refe5 = "Also see : ";
    char           *refe6 = "Also See : ";
    char           *refe7 = "See also : ";
    char           *refe8 = "See Also : ";

    /*
     * ignore formats of this kind 
     */
    char           *modby = "Done by ";
    if (!hlp) {
        return (0);
    }
    if (!arg) {
        return (0);
    }
    for (i = 0; i < 18; i++) {
        if (arg[i] != usage[i]) {
            break;
        }
        if (i == 17) {
            /* 
             * first 18 chars are identical, should be good 
             *
             * put everything from : * to end of line into * arg2 
             */
            half_chop(arg, arg1, arg2); 

            /* 
             * get rid of the : 
             */
            half_chop(arg2, arg1, arg3);

            /* 
             * get rid of * carriage return 
             */
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->usage = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 18; i++) {
        if (arg[i] != accum[i]) {
            break;
        }
        if (i == 17) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->accumulative = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 18; i++) {
        if (arg[i] != durat[i]) {
            break;
        }
        if (i == 17) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->duration = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 18; i++) {
        if (arg[i] != level[i]) {
            break;
        }
        if (i == 17) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->level = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 18; i++) {
        if (arg[i] != damag[i]) {
            break;
        }
        if (i == 17) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->damagetype = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 18; i++) {
        if (arg[i] != saves[i]) {
            break;
        }
        if (i == 17) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->saves = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != refer[i]) {
            break;
        }
        if (i == 7) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 5; i++) {
        if (arg[i] != usag2[i]) {
            break;
        }
        if (i == 4) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->usage = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 14; i++) {
        if (arg[i] != accu2[i]) {
            break;
        }
        if (i == 13) {
            /*
             * take one out, missing space 
             */
            half_chop(arg, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->accumulative = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != dura2[i]) {
            break;
        }
        if (i == 7) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->duration = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 5; i++) {
        if (arg[i] != leve2[i]) {
            break;
        }
        if (i == 4) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->level = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 4; i++) {
        if (arg[i] != save2[i]) {
            break;
        }
        if (i == 3) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->saves = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != refe2[i]) {
            break;
        }
        if (i == 7) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != refe3[i]) {
            break;
        }
        if (i == 7) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != refe4[i]) {
            break;
        }
        if (i == 7) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 10; i++) {
        if (arg[i] != refe5[i]) {
            break;
        }
        if (i == 9) {
            /*
             * add another halfchop due to extra space 
             */
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg);
            half_chop(arg, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 10; i++) {
        if (arg[i] != refe6[i]) {
            break;
        }
        if (i == 9) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg);
            half_chop(arg, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 10; i++) {
        if (arg[i] != refe7[i]) {
            break;
        }
        if (i == 9) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg);
            half_chop(arg, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 10; i++) {
        if (arg[i] != refe8[i]) {
            break;
        }
        if (i == 9) {
            half_chop(arg, arg1, arg2);
            half_chop(arg2, arg1, arg);
            half_chop(arg, arg1, arg3);
            if (arg3[strlen(arg3) - 1] == '\n') {
                arg3[strlen(arg3) - 1] = '\0';
            }
            hlp->references = (char *) strdup(arg3);
            return (1);
        }
    }

    for (i = 0; i < 8; i++) {
        if (arg[i] != modby[i]) {
            break;
        }
        if (i == 7) {
            return (1);
        }
    }
    return (0);
}

int read_help_from_file(struct char_data *ch, char *argument, int cmd)
{
    struct help_file_u *hlp;
    int             minlen,
                    i;
    char            buf[MAX_STRING_LENGTH],
                    buffer[MAX_STRING_LENGTH];

    if (!ch->desc) {
        return (FALSE);
    }
    
    for (; isspace(*argument); argument++) {
        /* 
         * Empty loop 
         */
    }

    if (*argument) {
        if (!help_index) {
            send_to_char("No help available.\n\r", ch);
            return (FALSE);
        }

        /*
         * first look in help 
         */
        for (i = 0; i <= top_of_helpt; i++) {
            minlen = strlen(argument);
            if (!(strn_cmp(argument, help_index[i].keyword, minlen))) {
                if (!(hlp = ch->specials.help)) {
                    return (FALSE);
                }
                hlp->wizard = 0;        
                /* 
                 * found in mortal helpfiles 
                 */
                hlp->newfile = 0;
                hlp->index = i;
                rewind(help_fl);
                fseek(help_fl, help_index[i].pos, 0);
                *buffer = '\0';
                fgets(buf, 80, help_fl);

                if (buf[strlen(buf) - 1] == '~') {
                    buf[strlen(buf) - 1] = '\0';
                }
                hlp->name = (char *) strdup(buf);

                for (;;) {
                    fgets(buf, 80, help_fl);

                    /*
                     * don't store whitelines 
                     */
                    if (*buf == '\n' || *buf == '\r') {
                        *buf = '\0';
                    }
                    if (*buf == '#') {
                        break;
                    }
                    if (strlen(buf) > MAX_STRING_LENGTH - 2) {
                        break;
                    }
                    if (buf[strlen(buf) - 1] == '~') {
                        buf[strlen(buf) - 1] = '\0';
                    }
                    if (!is_help_elem(hlp, buf)) {
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                }

                hlp->description = (char *) strdup(buffer);
                return (TRUE);
            }
        }

        /*
         * now look in wizhelp 
         */
        if (!wizhelp_index) {
            send_to_char("No wizhelp available.\n\r", ch);
            return (FALSE);
        }
        for (i = 0; i <= top_of_wizhelpt; i++) {
            minlen = strlen(argument);
            if (!(strn_cmp(argument, wizhelp_index[i].keyword, minlen))) {
                if (!(hlp = ch->specials.help)) {
                    return (FALSE);
                }
                hlp->wizard = 1;        
                /* 
                 * found in wizhelpfiles 
                 */
                hlp->newfile = 0;
                hlp->index = wizhelp_index[i].pos;
                rewind(wizhelp_fl);
                fseek(wizhelp_fl, wizhelp_index[i].pos, 0);
                *buffer = '\0';
                fgets(buf, 80, wizhelp_fl);
                if (buf[strlen(buf) - 1] == '\n') {
                    buf[strlen(buf) - 1] = '\0';
                }
                hlp->name = (char *) strdup(buf);
                for (;;) {
                    fgets(buf, 80, wizhelp_fl);
                    if (*buf == '\n' || *buf == '\r') {
                        *buf = '\0';
                    }
                    if (*buf == '#') {
                        break;
                    }
                    if (strlen(buf) > MAX_STRING_LENGTH - 2) {
                        break;
                    }
                    if (!is_help_elem(hlp, buf)) {
                        strcat(buffer, buf);
                        strcat(buffer, "\r");
                    }
                }
                hlp->description = (char *) strdup(buffer);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

void UpdateHelpMenu(struct char_data *ch)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    send_to_char(VT_HOMECLR, ch);
    sprintf(buf, VT_CURSPOS, 1, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Helpfile Name: %s\n\r", hlp->name);
    send_to_char(buf, ch);

    if ((char *) hlp->usage) {
        send_to_char("\n\r   Usage        : ", ch);
        send_to_char(hlp->usage, ch);
    }

    if ((char *) hlp->accumulative) {
        send_to_char("\n\r   Accumulative : ", ch);
        send_to_char(hlp->accumulative, ch);
    }

    if ((char *) hlp->duration) {
        send_to_char("\n\r   Duration     : ", ch);
        send_to_char(hlp->duration, ch);
    }

    if ((char *) hlp->level) {
        send_to_char("\n\r   Level        : ", ch);
        send_to_char(hlp->level, ch);
    }

    if ((char *) hlp->damagetype) {
        send_to_char("\n\r   Damagetype   : ", ch);
        send_to_char(hlp->damagetype, ch);
    }

    if ((char *) hlp->saves) {
        send_to_char("\n\r   Save         : ", ch);
        send_to_char(hlp->saves, ch);
    }

    if ((char *) hlp->description) {
        send_to_char("\n\r\n\r", ch);
        send_to_char(hlp->description, ch);
    }

    if ((char *) hlp->references) {
        send_to_char("\n\rAlso see: ", ch);
        send_to_char(hlp->references, ch);
    } else {
        send_to_char("\n\rAlso see: -", ch);
    }

    if (hlp->wizard) {
        send_to_char("\n\r\n\rThis is a wizhelp file.", ch);
    }

    send_to_char("\n\r\n\r", ch);
    send_to_char("Menu:\n\r", ch);
    send_to_char(help_edit_menu, ch);
    send_to_char("Your choice : \n\r", ch);
}


void HelpEdit(struct char_data *ch, char *arg)
{
    if (ch->specials.hedit == HELP_MAIN_MENU) {
        if (!*arg || *arg == '\n') {
            send_to_char("Please enter a choice from the above.\n\r", ch);
            return;
        }
        switch (atoi(arg)) {
        case 0:
            UpdateHelpMenu(ch);
            break;
        case 1:
            ch->specials.hedit = CHANGE_HELP_NAME;
            ChangeHelpName(ch, "", ENTER_CHECK);
            break;
        case 2:
            ch->specials.hedit = CHANGE_HELP_USAGE;
            ChangeHelpUsage(ch, "", ENTER_CHECK);
            break;
        case 3:
            ch->specials.hedit = CHANGE_HELP_ACCUMULATIVE;
            ChangeHelpAccumulative(ch, "", ENTER_CHECK);
            break;
        case 4:
            ch->specials.hedit = CHANGE_HELP_DURATION;
            ChangeHelpDuration(ch, "", ENTER_CHECK);
            break;
        case 5:
            ch->specials.hedit = CHANGE_HELP_LEVEL;
            ChangeHelpLevel(ch, "", ENTER_CHECK);
            break;
        case 6:
            ch->specials.hedit = CHANGE_HELP_DAMTYPE;
            ChangeHelpDamtype(ch, "", ENTER_CHECK);
            break;
        case 7:
            ch->specials.hedit = CHANGE_HELP_SAVES;
            ChangeHelpSaves(ch, "", ENTER_CHECK);
            break;
        case 8:
            ch->specials.hedit = CHANGE_HELP_DESCRIPTION;
            ChangeHelpDescription(ch, "", ENTER_CHECK);
            break;
        case 9:
            ch->specials.hedit = CHANGE_HELP_REFERENCES;
            ChangeHelpReferences(ch, "", ENTER_CHECK);
            break;
        case 10:
            ch->specials.hedit = CHANGE_HELP_WIZARD;
            ChangeHelpWizard(ch, "", ENTER_CHECK);
            break;
        case 99:
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing a helpfile.", FALSE, ch, 0,
                0, TO_ROOM);
            send_to_char("Exiting help editor, saving file...\n\r", ch);
            GET_POS(ch) = POSITION_STANDING;

            /*
             * write helpfile here 
             */
            if (write_help_to_file(ch, ch->specials.help)) {
                send_to_char("File saved successfully.\n\r", ch);
            } else {
                send_to_char("File save unsuccessful. Something went pear "
                             "shaped :(\n\r", ch);
            }
            if (ch->specials.help->wizard) {
                send_to_char("Reindexing wizhelp table...\n\r", ch);
                wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);
                send_to_char("Done.\n\r", ch);
            } else {
                send_to_char("Reindexing help table...\n\r", ch);
                help_index = build_help_index(help_fl, &top_of_helpt);
                send_to_char("Done.\n\r", ch);
            }
            ch->specials.help = 0;
            break;
        case 100:
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing a helpfile.", FALSE, ch, 0,
                0, TO_ROOM);
            send_to_char("Exiting help editor, file not saved.\n\r", ch);
            GET_POS(ch) = POSITION_STANDING;
            ch->specials.help = 0;
            break;
        default:
            UpdateHelpMenu(ch);
            break;
        }
        return;
    }

    switch (ch->specials.hedit) {
    case CHANGE_HELP_NAME:
        ChangeHelpName(ch, arg, 0);
        break;
    case CHANGE_HELP_USAGE:
        ChangeHelpUsage(ch, arg, 0);
        break;
    case CHANGE_HELP_ACCUMULATIVE:
        ChangeHelpAccumulative(ch, arg, 0);
        break;
    case CHANGE_HELP_DURATION:
        ChangeHelpDuration(ch, arg, 0);
        break;
    case CHANGE_HELP_LEVEL:
        ChangeHelpLevel(ch, arg, 0);
        break;
    case CHANGE_HELP_DAMTYPE:
        ChangeHelpDamtype(ch, arg, 0);
        break;
    case CHANGE_HELP_SAVES:
        ChangeHelpSaves(ch, arg, 0);
        break;
    case CHANGE_HELP_DESCRIPTION:
        ChangeHelpDescription(ch, arg, 0);
        break;
    case CHANGE_HELP_REFERENCES:
        ChangeHelpReferences(ch, arg, 0);
        break;
    case CHANGE_HELP_WIZARD:
        ChangeHelpWizard(ch, arg, 0);
        break;
    default:
        Log("Got to bad spot in HelpEdit");
        break;
    }
}

int write_one_help(struct char_data *ch, FILE * f_temp)
{
    char            buf[MAX_STRING_LENGTH * 2],
                    buffer[MAX_STRING_LENGTH * 2];
    struct help_file_u *hlp;

    if (!f_temp) {
        return (FALSE);
    }
    if (!ch) {
        return (FALSE);
    }
    hlp = ch->specials.help;
    if (!hlp) {
        return (FALSE);
    }
    if (hlp->modBy) {
        free(hlp->modBy);
    }

    hlp->modBy = (char *) strdup(GET_NAME(ch));
    hlp->modified = time(0);
    sprintf(buf, "%s", hlp->name);
    if (buf[strlen(buffer) - 1] == '~') {
        buf[strlen(buffer) - 1] = '\0';
    }
    strcat(buf, "\n");
    fputs(buf, f_temp);

    if (hlp->usage) {
        sprintf(buf, "   Usage        : %s", hlp->usage);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->accumulative) {
        sprintf(buf, "   Accumulative : %s", hlp->accumulative);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->duration) {
        sprintf(buf, "   Duration     : %s", hlp->duration);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->level) {
        sprintf(buf, "   Level        : %s", hlp->level);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->damagetype) {
        sprintf(buf, "   Damagetype   : %s", hlp->damagetype);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->saves) {
        sprintf(buf, "   Save         : %s", hlp->saves);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, f_temp);
    }

    if (hlp->description) {
        fputs("\n", f_temp);
        remove_cr(buf, hlp->description);
        if (buf[strlen(buffer) - 1] == '~') {
            buf[strlen(buffer) - 1] = '\0';
        }
        fputs(buf, f_temp);
    }

    sprintf(buf, "\nAlso see: %s\n", hlp->references ? hlp->references : "-");
    if (buf[strlen(buffer) - 1] == '~') {
        buf[strlen(buffer) - 1] = '\0';
    }
    fputs(buf, f_temp);

    sprintf(buf, "\nDone by %s, %s", hlp->modBy,
            asctime(localtime(&hlp->modified)));
    fputs(buf, f_temp);
    fputs("#\n", f_temp);
    return (TRUE);
}

int write_help_to_file(struct char_data *ch, struct help_file_u *hlp)
{
    FILE           *f;
    FILE           *f_temp;
    char            buf[MAX_STRING_LENGTH * 2];
    int             i,
                    top = 0,
                    tmp = 0;
    char            store[3];

    if (!hlp) {
        send_to_char("Trying to save a non-existent help file "
                     "(write_help_to_file). Aborting.\n\r", ch);
        return (FALSE);
    }

    if (strlen(hlp->name) < 2) {
        send_to_char("Cannot save unnamed helpfiles.\n\r", ch);
        return (FALSE);
    }

    if (hlp->wizard) {
        sprintf(buf, "wizhelp_temp");
        if ((f_temp = fopen(buf, "w")) == NULL) {
            send_to_char("Cannot access tmp wizhelpfile.\n\r", ch);
            return (FALSE);
        }
        sprintf(buf, "wizhelp_table");
        if ((f = fopen(buf, "r")) == NULL) {
            send_to_char("Cannot access wizhelpfile.\n\r", ch);
            return (FALSE);
        }
        top = top_of_wizhelpt;
    } else {
        sprintf(buf, "help_temp");
        if ((f_temp = fopen(buf, "w")) == NULL) {
            send_to_char("Cannot access tmp helpfile.\n\r", ch);
            return (FALSE);
        }

        sprintf(buf, "help_table");
        if ((f = fopen(buf, "r")) == NULL) {
            send_to_char("Cannot access helpfile.\n\r", ch);
            return (FALSE);
        }
        top = top_of_helpt;
    }

    send_to_char("Saving into temporary file.\n\r", ch);
    if (hlp->index) {
        send_to_char("Oversaving existing file.\n\r", ch);
        rewind(f);
        while (ftell(f) != help_index[hlp->index].pos) {
            tmp = fgetc(f);
            store[0] = (char) tmp;
            fprintf(f_temp, store);
        }

        /*
         * now move cursor to where the next help element starts 
         */
        do
            fgets(buf, 81, f);
        while (*buf != '#');

        /*
         * write edited file 
         */
        if (!(write_one_help(ch, f_temp))) {
            send_to_char("Single file not saved right, Aborting.\n\r", ch);
            return (FALSE);
        } else {
            send_to_char("Inserted file.\n\r", ch);
        }

        /*
         * write remainder 
         */
        while (!feof(f)) {
            tmp = fgetc(f);
            store[0] = (char) tmp;
            fprintf(f_temp, store);
        }

        /*
         * make sure it ends with #~ 
         */
        fprintf(f_temp, "#~");
    } else {
        send_to_char("Adding new file.\n\r", ch);
        rewind(f);
        *(buf + strlen(buf) - 1) = '\0';
        i = 0;

        /*
         * place new files second from top, 
         */
        while (i < 2) {
            fgets(buf, 81, f);
            if (*buf == '#') {
                i++;
            }
            fputs(buf, f_temp);
        }

        /*
         * write new file 
         */
        if (!(write_one_help(ch, f_temp))) {
            send_to_char("Single file not saved right, Aborting.\n\r", ch);
            return (FALSE);
        } else {
            send_to_char("Added file.\n\r", ch);
        }

        /*
         * write remainder 
         */
        for (;;) {
            fgets(buf, 81, f);
            fputs(buf, f_temp);
            if (*buf == '#' && *(buf + 1) == '~') {
                break;
            }
        }
    }

    send_to_char("Done.\n\r", ch);
    fclose(f_temp);
    fclose(f);

    /*
     * now copy f_temp into f but close global helps first 
     */
    fclose(wizhelp_fl);
    fclose(help_fl);
    if (hlp->wizard) {
        sprintf(buf, "wizhelp_temp");
        if ((f_temp = fopen(buf, "r")) == NULL) {
            send_to_char("Cannot access tmp wizhelpfile.\n\r", ch);
            return (FALSE);
        }

        sprintf(buf, "wizhelp_table");
        if ((f = fopen(buf, "w")) == NULL) {
            send_to_char("Cannot access wizhelpfile.\n\r", ch);
            return (FALSE);
        }
    } else {
        sprintf(buf, "help_temp");
        if ((f_temp = fopen(buf, "r")) == NULL) {
            send_to_char("Cannot access tmp helpfile.\n\r", ch);
            return (FALSE);
        }
        
        sprintf(buf, "help_table");
        if ((f = fopen(buf, "w")) == NULL) {
            send_to_char("Cannot access helpfile.\n\r", ch);
            return (FALSE);
        }
    }
    send_to_char("Overwriting old file...\n\r", ch);
    rewind(f_temp);

    for (;;) {
        fgets(buf, 81, f_temp);
        if (*buf == '#' && *(buf + 1) == '~') {
            break;
        }
        fputs(buf, f);
    }

    fputs("#~", f);
    send_to_char("Done.\n\r", ch);
    fclose(f_temp);
    fclose(f);

    /*
     * open up the global helps 
     */
    if ((wizhelp_fl = fopen(WIZ_HELP_FILE, "r")) == NULL) {
        send_to_char("Cannot open new version of wizhelpfile.\n\r", ch);
        return (FALSE);
    }

    if ((help_fl = fopen(HELP_KWRD_FILE, "r")) == NULL) {
        send_to_char("Cannot open new version of helpfile.\n\r", ch);
        return (FALSE);
    }
    return (TRUE);
}

void ChangeHelpName(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    hlp = ch->specials.help;
    if (type != ENTER_CHECK) {
        if (hlp->name) {
            free(hlp->name);
        }
        hlp->name = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current Helpfile Name: %s", hlp->name);
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew Helpfile Name: \n\r\n\r", ch);
    send_to_char("Example: WIMP \"WIMP MODE\" WIMPY\n", ch);
    return;
}


void ChangeHelpUsage(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->usage) {
            free(hlp->usage);
        }
        hlp->usage = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->usage) {
            free(hlp->usage);
        }
        hlp->usage = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current command usage: %s", hlp->usage ? hlp->usage : "None");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rNew command usage: ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Example: wield <weapon>\n", ch);
}

void ChangeHelpAccumulative(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->accumulative) {
            free(hlp->accumulative);
        }
        hlp->accumulative = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }
    
    if (type != ENTER_CHECK) {
        if (hlp->accumulative) {
            free(hlp->accumulative);
        }
        hlp->accumulative = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current status of accumulative: %s",
            hlp->accumulative ? hlp->accumulative : "Not specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rIs this skill accumulative? (enter Yes or No): ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Example: Yes\n", ch);
}

void ChangeHelpDuration(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->duration) {
            free(hlp->duration);
        }
        hlp->duration = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->duration) {
            free(hlp->duration);
        }
        hlp->duration = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current skill/spell duration: %s",
            hlp->duration ? hlp->duration : "Not specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter skill/spell duration: ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Examples: 24 hours\n", ch);
    send_to_char("          (level of caster) hours\n", ch);
    send_to_char("          Instantaneous\n", ch);
    send_to_char("          Permanent\n", ch);
}

void ChangeHelpLevel(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;
    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || (*arg == '\n'))) {
        if (hlp->level) {
            free(hlp->level);
        }
        hlp->level = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->level) {
            free(hlp->level);
        }
        hlp->level = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current level of skill: %s",
            hlp->level ? hlp->level : "Not specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter skill/spell levels (for various "
                 "classes if needed): ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Examples: 4 Mage, 1 Cleric\n", ch);
    send_to_char("          53\n", ch);
}

void ChangeHelpDamtype(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->damagetype) {
            free(hlp->damagetype);
        }
        hlp->damagetype = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->damagetype) {
            free(hlp->damagetype);
        }
        hlp->damagetype = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current type of damage: %s",
            hlp->damagetype ? hlp->damagetype : "Not specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter the type of damage of this "
                 "skill/spell: ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Example: Fire\n", ch);
}

void ChangeHelpSaves(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->saves) {
            free(hlp->saves);
        }
        hlp->saves = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->saves) {
            free(hlp->saves);
        }
        hlp->saves = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current saving throw: %s",
            hlp->saves ? hlp->saves : "Not specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter the saving throw, or 'None' if no "
                 "save is possible: ", ch);
    send_to_char("\n\r(No input (hit ENTER) deletes this line from "
                 "file.\n\r\n\r", ch);
    send_to_char("Examples: None\n", ch);
    send_to_char("          vs. spell for half damage\n", ch);
    send_to_char("          vs. para for negate\n", ch);
}

void ChangeHelpDescription(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    if (type != ENTER_CHECK) {
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    hlp = ch->specials.help;
    sprintf(buf, "Current Helpfile Description:\n\r");
    send_to_char(buf, ch);
    send_to_char(hlp->description, ch);
    send_to_char("\n\r\n\rNew Helpfile Description:\n\r", ch);
    send_to_char("(Use /? for help on editing strings. Press <C/R> again to"
                 " continue)\n\r", ch);
    if (hlp->description) {
        free(hlp->description);
    }
    hlp->description = NULL;
    ch->desc->str = &hlp->description;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

void ChangeHelpReferences(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct help_file_u *hlp;

    hlp = ch->specials.help;
    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        if (hlp->references) {
            free(hlp->references);
        }
        hlp->references = NULL;
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (hlp->references) {
            free(hlp->references);
        }
        hlp->references = (char *) strdup(arg);
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);
    sprintf(buf, "Current cross-references: %s",
            hlp->references ? hlp->references : "None specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter any appropriate cross-references "
                 "(in caps, divided by commas): ", ch);
    send_to_char("\n\r(No input (hit ENTER) sets to default (-) \n\r\n\r", ch);
    send_to_char("Examples: SPELL COLOR SPRAY, SPELL INFRAVISION\n", ch);
    send_to_char("          WHO\n", ch);
}

void ChangeHelpWizard(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[255];
    struct help_file_u *hlp;

    if (type != ENTER_CHECK && (!*arg || *arg == '\n')) {
        ch->specials.hedit = HELP_MAIN_MENU;
        UpdateHelpMenu(ch);
        return;
    }

    update = atoi(arg);
    hlp = ch->specials.help;

    if (type != ENTER_CHECK) {
        switch (ch->specials.hedit) {
        case CHANGE_HELP_WIZARD:
            if (update < 0 || update > 1) {
                return;
            } else {
                if (hlp->newfile) {
                    hlp->wizard = update;
                }
                ch->specials.hedit = HELP_MAIN_MENU;
                UpdateHelpMenu(ch);
                return;
            }
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    if (!(hlp->newfile)) {
        send_to_char("This option is only settable for newly added "
                     "helpfiles.\n\r", ch);
        send_to_char("Edited helpfiles can not be interchanged between "
                     "wizhelp & help.\n\r", ch);
        send_to_char("Hit <enter> to return to main menu.\n\r", ch);
    } else {
        sprintf(buf, "%s\n\r\n\r", hlp->wizard ? "This is a wizhelp file." :
                                   "This is not a wizhelp file.");
        send_to_char(buf, ch);
        send_to_char("If this should be a wizhelp file, type 1, else type 0 ",
                     ch);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
