
#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

struct board_def *find_board(struct obj_data *obj);
int show_board(struct char_data *ch, char *arg, struct board_def *board);
int display_board_message(struct char_data * ch, char *arg,
                          struct board_def * board);
int remove_board_message(struct char_data *ch, char *arg,
                         struct board_def *board);
int reply_board_message(struct char_data *ch, char *arg,
                        struct board_def *board);
void write_board_message(struct char_data *ch, char *arg,
                         struct board_def *board);
void new_board_message(struct char_data *ch, struct board_def *board, 
                       char *title, int reply_to);
void list_board_messages(struct board_def *board, struct descriptor_data *d,
                         int message_id, short depth);

int board(struct char_data *ch, int cmd, char *arg,
          struct obj_data *obj, int type)
{
    char           *argument;
    char           *arg1;
    int             ret = FALSE;
    struct board_def *board = NULL;

    if (type != PULSE_COMMAND || !ch || !ch->desc || !obj) {
        return( FALSE );
    }

    /* So we don't bugger it up if the proc returns FALSE */
    if( arg ) {
        argument = strdup(arg);
    } else {
        argument = NULL;
    }

    /* leave the original argument for the caller */
    arg = argument;

    switch( cmd ) {
    case CMD_LOOK:
        arg = get_argument(arg, &arg1);
        if (!arg1 || !isname(arg1, "board bulletin")) {
            return( FALSE );
        }

        board = find_board(obj);
        if( board ) {
            ret = show_board(ch, arg, board);
        }
        break;
    case CMD_READ:
        board = find_board(obj);
        if( board ) {
            ret = display_board_message(ch, arg, board);
        }
        break;
    case CMD_REMOVE:
        board = find_board(obj);
        if( board ) {
            ret = remove_board_message(ch, arg, board);
        }
        break;
    case CMD_REPLY:
        board = find_board(obj);
        if( board ) {
            ret = reply_board_message(ch, arg, board);
        }

        /* so it doesn't get freed below */
        board = NULL;
        break;
    case CMD_WRITE:
        board = find_board(obj);
        if( board ) {
            write_board_message(ch, arg, board);
            ret = TRUE;
        }

        /* so it doesn't get freed below */
        board = NULL;
        break;
    }

    if( board ) {
        free( board );
    }

    /* Get rid of our local argument */
    free( argument );
    return( ret );
}


struct board_def *find_board(struct obj_data *obj)
{
    struct board_def *board;

    /* Look up the board ID */
    board = db_lookup_board(obj->item_number);
    if( board ) {
        board->obj = obj;
        board->vnum = obj->item_number;
    }

    return( board );
}


void new_board_message(struct char_data *ch, struct board_def *board, 
                       char *title, int reply_to)
{
    CREATE(ch->desc->msg, struct bulletin_board_message, 1);
    ch->desc->msg->author = strdup(GET_NAME(ch));
    ch->desc->msg->reply_to = reply_to;

    title = skip_spaces(title);
    if( title ) {
        ch->desc->msg->title = strdup(title);
    } else {
        ch->desc->msg->title = strdup("Untitled");
    }

    /*
     * Save a pointer to this board in the character's descriptor so
     * we can add the message when $e is done writing.
     */

    ch->desc->board = board;

    send_to_char("Write your message.\r\n"
                 "Type /? and hit enter for help on using the editor.\r\n\r\n",
                 ch);
    act("$n starts writing a message.", TRUE, ch, NULL, NULL, TO_ROOM);

    /*
     * Initiate the string_add procedures
     */
    ch->desc->str = &ch->desc->msg->text;
    ch->desc->max_str = MAX_MESSAGE_LENGTH;
    SET_BIT(ch->specials.act, PLR_POSTING);
}


void write_board_message(struct char_data *ch, char *arg,
                         struct board_def *board)
{
    if (GetMaxLevel(ch) < board->minLevel) {
        act("You start to write a message upon $p but quickly pull away as "
            "flames engulf your hand.", TRUE, ch, board->obj, NULL, TO_CHAR);
        act("$n started to write upon $p but quickly pulled back in pain.",
            TRUE, ch, board->obj, NULL, TO_ROOM);
        return;
    }
    
    new_board_message(ch, board, arg, -1);
}

int reply_board_message(struct char_data *ch, char *arg,
                        struct board_def *board)
{
    struct bulletin_board_message *msg;
    int             tmessage;
    char            buf[128];
    char           *arg1;

    arg = get_argument(arg, &arg1);
    if (!arg1 || !(tmessage = atoi(arg1))) {
        return( FALSE );
    }

    if (GetMaxLevel(ch) < board->minLevel) {
        act("You start to write a message upon $p but quickly pull away as "
            "flames engulf your hand.", TRUE, ch, board->obj, NULL, TO_CHAR);
        act("$n started to write upon $p but quickly pulled back in pain.",
            TRUE, ch, board->obj, NULL, TO_ROOM);
        return( TRUE );
    }

    msg = db_get_board_message(board->boardId, tmessage);
    if( !msg ) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return( TRUE );
    }

    /*
     * skip blanks
     */
    arg = skip_spaces(arg);
    if (!arg) {
        sprintf(buf, "re: %s", msg->title);
    } else {
        strncpy(buf, arg, 128);
    }

    db_free_board_message(msg);
    new_board_message(ch, board, buf, tmessage);
    return( TRUE );
}


int remove_board_message(struct char_data *ch, char *arg,
                         struct board_def *board)
{
    struct bulletin_board_message *msg;
    int            tmessage;
    char          *arg1,
                   buf[MAX_STRING_LENGTH];

    arg = get_argument(arg, &arg1);
    if (!arg1 || !(tmessage = atoi(arg1))) {
        return( FALSE );
    }

    msg = db_get_board_message(board->boardId, tmessage);
    if( !msg ) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return( TRUE );
    }


    if (GetMaxLevel(ch) < board->minLevel &&
        strcasecmp(GET_NAME(ch), msg->author) ) {
        act("You try to grab one of the notes off $p but only end up getting a"
            " nasty shock.", TRUE, ch, board->obj, NULL, TO_CHAR);
        act("$n tried to remove a note from $p but only received a "
            "hair-styling shock!", TRUE, ch, board->obj, NULL, TO_ROOM);
        db_free_board_message(msg);
        return( TRUE );
    }
    db_free_board_message(msg);

    db_delete_board_message(board, tmessage);
    send_to_char("Message removed.\r\n", ch);
    sprintf(buf, "$n just removed message %d.", tmessage);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
    Log("%s just removed message %d from board %d", GET_NAME(ch), tmessage,
        board->boardId);
    return( TRUE );
}

int display_board_message(struct char_data * ch, char *arg,
                           struct board_def * board)
{
    struct bulletin_board_message *msg;
    char           buffer[MAX_STRING_LENGTH];
    int            tmessage;
    char          *arg1;

    arg = get_argument(arg, &arg1);
    if (!arg1 || !(tmessage = atoi(arg1))) {
        return( FALSE );
    }

    if (GetMaxLevel(ch) < board->minLevel) {
        act("You simply cannot comprehend the alien markings upon $p.",
            TRUE, ch, board->obj, NULL, TO_CHAR);
        act("$n tries to read $p, but looks bewildered.", TRUE, ch, board->obj,
            NULL, TO_ROOM);
        return( TRUE );
    }

    msg = db_get_board_message(board->boardId, tmessage);
    if( !msg ) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return( TRUE );
    }

    /*
     * We've got the message, simply print it out to the character now
     */
    sprintf(buffer, " Message [%3d]  Author: %s\r\n"
            " Date: %s Topic: %s\r\n"
            "----------------------------------------------------------------"
            "--------\r\n\r\n%s",
            tmessage, msg->author, asctime(localtime(&msg->date)),
            msg->title, msg->text ? msg->text : "N/M\r\n");
    page_string(ch->desc, buffer, 1);

    act("$n examines one of the posts on $p.", TRUE, ch, board->obj, NULL,
        TO_ROOM);

    db_free_board_message(msg);
    return( TRUE );
}

/*
 *  Recursively display all the replies to a top level post.
 */
void list_board_messages(struct board_def *board, struct descriptor_data *d,
                         int message_id, short depth)
{
    struct bulletin_board_message *msg;
    int             i,
                    j;
    char            buf[128],
                    buf2[255];
    int             count;

    count = db_get_board_replies(board, message_id, &msg);
    for( i = 0; i < count; i++ ) {
        /*
         * found a reply to an upper post
         * indent the reply according to depth
         */
        for (j = 0; j < depth; j++) {
            list_append(d, " ");
        }

        /*
         *  asctime has to be difficult and return a newline in
         *  it's output, we need to get rid of it
         */
        strncpy(buf, asctime(localtime(&msg[i].date)), 128);

        while( buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r' ) {
            buf[strlen(buf)-1] = '\0';
        }

        /*
         * chop the last characters to make it line out pretty
         */
        sprintf(buf2, "%-32s", msg[i].title);

        for (j = 0; j < depth; j++) {
            buf2[strlen(buf2) - 1] = '\0';
        }

        if( message_id == -1 ) {
            list_append(d, "$c000w%3d$c000w : $c000w%s$c000w %-12s "
                           "$c000x[$c000w%s$c000x]\r\n",
                           msg[i].message_id, buf2, msg[i].author, buf);
        } else {
            list_append(d, "$c000x%3d$c000w : $c000w%s$c000x %-12s "
                           "$c000x-$c000w%s$c000x-\r\n",
                           msg[i].message_id, buf2, msg[i].author, buf);
        }

        list_board_messages(board, d, msg[i].message_id, depth + 1);
    }
    db_free_board_replies(msg, count);
}


/*
 *  Show topics and their replies in a threaded format.
 */
int show_board(struct char_data *ch, char *arg, struct board_def *board)
{
    /*
     * See if character's level is high enough to read board
     */
    if (GetMaxLevel(ch) < board->minLevel) {
        act("You simply cannot comprehend the alien markings upon $p.",
             TRUE, ch, board->obj, NULL, TO_CHAR);
        act("$n tries to read $p, but looks bewildered.", TRUE, ch, board->obj,
             NULL, TO_ROOM);
        return( TRUE );
    }

    act("$n studies $p.", TRUE, ch, board->obj, NULL, TO_ROOM);
    list_init(ch->desc);
    list_append(ch->desc, "This is a bulletin board.\r\n"
                          "Usage: READ/REMOVE <#>, WRITE <title>, "
                          "REPLY <#> [title]\r\n");

    if (board->messageCount == 0) {
        list_append(ch->desc, "%s appears to be empty.\r\n",
                     CAP(OBJS(board->obj, ch)));
    } else {
        list_append(ch->desc, "There are %d messages on %s.\r\n\r\n"
                              " $c000W-=$c000RCurrent Discussions$c000W=-$c000w"
                              "\r\n"
                              "---------------------------------------------"
                              "---------------------------------\r\n",
                    board->messageCount, OBJS(board->obj, ch));
        list_board_messages(board, ch->desc, -1, 0);
    }
    list_end(ch->desc);
    return( TRUE );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
