/*
 * Online textfile editor for motd, wmotd, news.
 * Done by Lennya, Sept 2003
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

extern char    *motd;
extern char    *wmotd;
extern char    *news;
extern const char *tfd_types[];

#define TFD_MAIN_MENU             0
#define VIEW_OLD_TFD              1
#define CHANGE_TFD_FILE           2
#define CHANGE_TFD_DATE           3
#define CHANGE_TFD_BODY           4
#define TFD_HIT_RETURN           15

#define ENTER_CHECK        1

char           *tfd_main_menu = 
    "    1) File to edit\n\r"
    "    2) View old file\n\r"
    "    3) Edit date of new file\n\r"
    "    4) Edit body of new file\n\r\n\r"
    "    8) Save and exit\n\r" 
    "    9) Don't save and exit\n\r\n\r";

void            UpdateTfdMenu(struct char_data *ch);
int             write_txt_to_file(struct char_data *ch);
void            ChangeTfdFile(struct char_data *ch, char *arg, int type);
void            ViewOldTfd(struct char_data *ch, char *arg, int type);
void            ChangeTfdDate(struct char_data *ch, char *arg, int type);
void            ChangeTfdBody(struct char_data *ch, char *arg, int type);

void do_chtextfile(struct char_data *ch, char *argument, int cmd)
{
    struct edit_txt_msg *tfd;

    dlog("in do_chtextfile");

    if (IS_NPC(ch)) {
        return;
    }
    /* 
     * someone is forced to do something. 
     * can be bad! 
     */
    if (!ch->desc) {
        return;
    }
    CREATE(tfd, struct edit_txt_msg, 1);
    ch->specials.txtedit = tfd;
    ch->specials.txtedit->author = (char *) strdup(GET_NAME(ch));

    ch->specials.tfd = TFD_MAIN_MENU;
    ch->desc->connected = CON_TFD_EDITING;

    act("$n has begun editing a textfile.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POSITION_SLEEPING;

    UpdateTfdMenu(ch);
}

void UpdateTfdMenu(struct char_data *ch)
{
    char            buf[255];
    struct edit_txt_msg *tfd;

    tfd = ch->specials.txtedit;

    send_to_char(VT_HOMECLR, ch);
    sprintf(buf, VT_CURSPOS, 1, 1);
    send_to_char(buf, ch);
    sprintf(buf, "Currently editing textfile: %s\n\r\n\r",
            tfd_types[tfd->file]);
    send_to_char(buf, ch);
    send_to_char("Preview:\n\r\n\r", ch);
    if (!((char *) tfd->file)) {
        send_to_char("No file selected.\n\r", ch);
    } else {
        if ((char *) tfd->date) {
            send_to_char(tfd->date, ch);
            send_to_char("\n\r\n\r", ch);
        }

        if ((char *) tfd->body) {
            send_to_char(tfd->body, ch);
        }

        if ((char *) tfd->author) {
            send_to_char("\nLast edited by ", ch);
            send_to_char(tfd->author, ch);
            send_to_char(".\n\r", ch);
        }
    }
    send_to_char("\n\r\n\r", ch);
    send_to_char("Menu:\n\r", ch);
    send_to_char(tfd_main_menu, ch);
    send_to_char("Your choice : \n\r", ch);
}

void TfdEdit(struct char_data *ch, char *arg)
{
    if (ch->specials.tfd == TFD_MAIN_MENU) {
        if (!*arg || *arg == '\n') {
            send_to_char("Please enter a choice from the above.\n\r", ch);
            return;
        }

        switch (atoi(arg)) {
        case 0:
            UpdateTfdMenu(ch);
            break;
        case 1:
            ch->specials.tfd = CHANGE_TFD_FILE;
            ChangeTfdFile(ch, "", ENTER_CHECK);
            break;
        case 2:
            if (ch->specials.txtedit->file > 0 && 
                ch->specials.txtedit->file < 4) {
                ch->specials.tfd = VIEW_OLD_TFD;
                ViewOldTfd(ch, "", ENTER_CHECK);
            } else {
                send_to_char("No valid filetype selected.\n\r", ch);
            }
            break;
        case 3:
            if (ch->specials.txtedit->file > 0 && 
                ch->specials.txtedit->file < 4) {
                ch->specials.tfd = CHANGE_TFD_DATE;
                ChangeTfdDate(ch, "", ENTER_CHECK);
            } else {
                send_to_char("No valid filetype selected.\n\r", ch);
            }
            break;
        case 4:
            ch->specials.tfd = CHANGE_TFD_BODY;
            ChangeTfdBody(ch, "", ENTER_CHECK);
            break;

        case 8:
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing a textfile.", FALSE, ch, 0,
                0, TO_ROOM);
            send_to_char("Exiting text editor, saving file...\n\r", ch);
            GET_POS(ch) = POSITION_STANDING;
            /*
             * write txtfile here 
             */
            if (write_txt_to_file(ch)) {
                send_to_char("File saved successfully.\n\r", ch);
                send_to_char("Initializing new file into game...\n\r", ch);
                if( news ) {
                    free( news );
                }
                news = file_to_string(NEWS_FILE);

                if( motd ) {
                    free( motd );
                }
                motd = file_to_string(MOTD_FILE);

                if( wmotd ) {
                    free( wmotd );
                }
                wmotd = file_to_string(WMOTD_FILE);
                send_to_char("Done.\n\r", ch);
            } else {
                send_to_char("File save unsuccessful. Something went pear "
                             "shaped :(\n\r", ch);
            }
            ch->specials.tfd = 0;
            break;
        case 9:
            ch->desc->connected = CON_PLYNG;
            act("$n has returned from editing a textfile.", FALSE, ch, 0,
                0, TO_ROOM);
            send_to_char("Exiting text editor, file not saved.\n\r", ch);
            GET_POS(ch) = POSITION_STANDING;
            ch->specials.tfd = 0;
            break;
        default:
            UpdateHelpMenu(ch);
            break;
        }
        return;
    }

    switch (ch->specials.tfd) {
    case CHANGE_TFD_FILE:
        ChangeTfdFile(ch, arg, 0);
        break;
    case VIEW_OLD_TFD:
        ViewOldTfd(ch, arg, 0);
        break;
    case CHANGE_TFD_DATE:
        ChangeTfdDate(ch, arg, 0);
        break;
    case CHANGE_TFD_BODY:
        ChangeTfdBody(ch, arg, 0);
        break;
    default:
        Log("Got to bad spot in TfdEdit");
        break;
    }
}

int write_txt_to_file(struct char_data *ch)
{
    char            buf[MAX_STRING_LENGTH * 2];
    struct edit_txt_msg *tfd;
    FILE           *fl;

    if (!ch) {
        return (FALSE);
    }
    tfd = ch->specials.txtedit;

    if (!tfd) {
        return (FALSE);
    }
    switch (tfd->file) {
    case 1:
        sprintf(buf, "%s", NEWS_FILE);
        break;
    case 2:
        sprintf(buf, "%s", MOTD_FILE);
        break;
    case 3:
        sprintf(buf, "%s", WMOTD_FILE);
        break;
    default:
        send_to_char("Cannot save this file type.\n\r", ch);
        return (FALSE);
        break;
    }

    if ((fl = fopen(buf, "w")) == NULL) {
        send_to_char("Cannot open file.\n\r", ch);
        return (FALSE);
    }

    fputs("\n", fl);

    if (tfd->date) {
        sprintf(buf, "%s", tfd->date);
        if (buf[strlen(buf) - 1] == '~') {
            buf[strlen(buf) - 1] = '\0';
        }
        strcat(buf, "\n");
        fputs(buf, fl);
    }

    if (tfd->body) {
        fputs("\n", fl);
        remove_cr(buf, tfd->body);
        if (buf[strlen(buf) - 1] == '~') {
            buf[strlen(buf) - 1] = '\0';
        }
        fputs(buf, fl);
    }

    if (tfd->author) {
        sprintf(buf, "\nLast edited by %s.", tfd->author);
        fputs(buf, fl);
    }

    fputs("\n", fl);

    fclose(fl);
    return (TRUE);
}

void ChangeTfdFile(struct char_data *ch, char *arg, int type)
{
    int             update;
    char            buf[255];
    struct edit_txt_msg *tfd;

    if (type != ENTER_CHECK && (!*arg || (*arg == '\n'))) {
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    update = atoi(arg);
    tfd = ch->specials.txtedit;

    if (type != ENTER_CHECK) {
        switch (ch->specials.tfd) {
        case CHANGE_TFD_FILE:
            if (update < 1 || update > 3) {
                return;
            } else {
                tfd->file = update;
                ch->specials.tfd = TFD_MAIN_MENU;
                UpdateTfdMenu(ch);
                return;
            }
            break;
        }
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current file: %s\n\r\n\r", tfd_types[tfd->file]);
    send_to_char(buf, ch);
    send_to_char("Options: \n\r", ch);
    send_to_char("  1) Newsfile\n\r", ch);
    send_to_char("  2) Message of the Day\n\r", ch);
    send_to_char("  3) Wizard's Message of the day\n\r", ch);
}

void ViewOldTfd(struct char_data *ch, char *arg, int type)
{
    char            buf[255],
                    buffer[254];
    char           *contents = NULL;

    switch (ch->specials.txtedit->file) {
    case 1:
        contents = file_to_string(NEWS_FILE);
        sprintf(buf, "news");
        break;
    case 2:
        contents = file_to_string(MOTD_FILE);
        sprintf(buf, "motd");
        break;
    case 3:
        contents = file_to_string(WMOTD_FILE);
        sprintf(buf, "wizmotd");
        break;
    default:
        send_to_char("No valid filetype selected.\n\r", ch);
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
        break;
    }

    if (type != ENTER_CHECK && (!*arg || (*arg == '\n'))) {
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buffer, "Current contents of %s file:\n\r\n\r", buf);
    send_to_char(buffer, ch);
    send_to_char(contents, ch);
    send_to_char("\n\r\n\r(hit ENTER to return to menu)\n\r", ch);
    if( contents ) {
        free( contents );
    }
}

void ChangeTfdDate(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct edit_txt_msg *tfd;

    tfd = ch->specials.txtedit;

    if (type != ENTER_CHECK && (!*arg || (*arg == '\n'))) {
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    if (type != ENTER_CHECK) {
        if (tfd->date) {
            free(tfd->date);
        }
        switch (tfd->file) {
        case 1:
            sprintf(buf, "$c000W[$c000BNews for %s$c000W]$c000w", arg);
            break;
        case 2:
            sprintf(buf, "$c000W[$c000BMessage of the Day, %s$c000W]$c000w",
                    arg);
            break;
        case 3:
            sprintf(buf, "$c000W[$c000BWizard's Message of the Day, "
                         "%s$c000W]$c000w", arg);
            break;
        default:
            send_to_char("No valid filetype selected.\n\r", ch);
            ch->specials.tfd = TFD_MAIN_MENU;
            UpdateTfdMenu(ch);
            return;
            break;
        }
        tfd->date = (char *) strdup(buf);
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    sprintf(buf, "Current file header: %s",
            tfd->date ? tfd->date : "None specified");
    send_to_char(buf, ch);
    send_to_char("\n\r\n\rPlease enter current date (for instance: March 24, "
                 "2003): ", ch);
}

void ChangeTfdBody(struct char_data *ch, char *arg, int type)
{
    char            buf[255];
    struct edit_txt_msg *tfd;

    if (type != ENTER_CHECK) {
        ch->specials.tfd = TFD_MAIN_MENU;
        UpdateTfdMenu(ch);
        return;
    }

    sprintf(buf, VT_HOMECLR);
    send_to_char(buf, ch);

    tfd = ch->specials.txtedit;

    sprintf(buf, "Current body of textfile:\n\r\n\r");
    send_to_char(buf, ch);
    send_to_char(tfd->body, ch);
    send_to_char("\n\r\n\rNew body of textfile: \n\r", ch);
    send_to_char("(Terminate with a /w. Press <C/R> again to continue. Use /? "
                 "for more info on editing strings.)\n\r", ch);
    if (tfd->body) {
        free(tfd->body);
    }
    tfd->body = NULL;
    ch->desc->str = &tfd->body;
    ch->desc->max_str = MAX_STRING_LENGTH;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
