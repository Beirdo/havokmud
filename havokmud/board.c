/*
 *****************************************************************************
 * board.c
 * Part of DaleMUD 3.? Millenium Edition
 * Re-written and re-released by Chris Lack (psycho_driver@yahoo.com)
 * You are obliged to follow the DaleMUD and Diku licenses if you use any 
 * portion of this source code.
 *****************************************************************************/
     
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
    
#include "protos.h"
#include "utils.h"
    
#define MAX_MESSAGE_LENGTH 2048     

struct bulletin_board *bboards = NULL;
extern struct index_data *obj_index;

char *fread_string(FILE *fl);

void save_board(struct bulletin_board *bd, int vnum) 
{
    FILE * fl;
    struct bulletin_board_message *msg;
    char           buf[256],
                   msg_txt[MAX_MESSAGE_LENGTH];

    sprintf(buf, "boards/%05d", vnum);
    if ((fl = fopen(buf, "w")) == NULL) {
        Log("Unable to save board");
        return;
    }
    
        /*
         * Don't bother if the board is empty 
         */ 
        if (bd->num_posts == 0) {
        fclose(fl);
        unlink(buf);
        return;
    }
    fprintf(fl, "%d\n", bd->num_posts);

    for (msg = bd->messages; msg; msg = msg->next) {
        fwrite_string(fl, msg->author);
        fwrite_string(fl, msg->title);
        fwrite_string(fl, strip_cr(msg_txt, msg->text, MAX_MESSAGE_LENGTH));
        fprintf(fl, "%ld %d %d %d\n", msg->date, msg->char_id,
                 msg->reply_to, msg->language);
    }
    fclose(fl);
}

void new_board_message(struct char_data *ch, struct bulletin_board *bd,
                       char *title, short reply_to) 
{
    char           buf[128];
    
    CREATE(ch->desc->msg, struct bulletin_board_message, 1);
    ch->desc->msg->author = strdup(GET_NAME(ch));
    if (IS_IMMORTAL(ch)) {
        if (reply_to == -1) {
            sprintf(buf, "%s", (title && *title) ? title : "Untitled");
        } else {
            sprintf(buf, "%s", (title && *title) ? title : "Untitled");
        }
    } else if (reply_to == -1) {
        sprintf(buf, "%s", (title && *title) ? title : "Untitled");
    } else {
        strncpy(buf, (title && *title) ? title : "Untitled", 128);
    }
    ch->desc->msg->title = strdup(buf);
    ch->desc->msg->char_id = ch->desc->pos;
    ch->desc->msg->reply_to = reply_to;
    ch->desc->msg->language = 0;
    
    /*
     *  Save a pointer to this board in the character's descriptor so 
     * we can add the message when $e is done writing. 
     */ 

    ch->desc->board = bd;

    send_to_char("Write your message.\r\n" 
                 "Type /? and hit enter for help on using the editor.\r\n\r\n",
                 ch);
    act("$n starts writing a message.", TRUE, ch, NULL, NULL, TO_ROOM);
    
    /*
     * Initiate the string_add procedures 
     */ 
    ch->desc->str = &ch->desc->msg->text;
    ch->desc->max_str = MAX_MESSAGE_LENGTH;
    SET_BIT(ch->specials.act, PLR_NODIMD);
    /*
     * should be plr_posting
     */
}


void write_board_message(struct char_data *ch, char *arg,
                         struct obj_data *board) 
{
    struct bulletin_board *bd;

    if (GetMaxLevel(ch) < board->obj_flags.value[1]) {
        act("You start to write a message upon $p but quickly pull away as "
            "flames engulf your hand.", TRUE, ch, board, NULL, TO_CHAR);
        act("$n started to write upon $p but quickly pulled back in pain.", 
            TRUE, ch, board, NULL, TO_ROOM);
        return;
    }
    
    /*
     * Find the board we're dealing with 
     */ 
    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == obj_index[board->item_number].virtual) {
            break;
        }
    }

    if (bd == NULL) {
#if 0
        sprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL,
                "Bugginess in board %ld", 
                obj_index[board->item_number].virtual);
#endif    
        Log("SOmething is wrong in a board");
        return;
    }
    
    /*
     * skip blanks 
     */ 
    for (; isspace(*arg); arg++) {
        /* 
         * Empty loop 
         */
    }
    new_board_message(ch, bd, arg, -1);
}

bool reply_board_message(struct char_data *ch, char *arg,
                         struct obj_data *board) 
{
    struct bulletin_board *bd;
    struct bulletin_board_message *msg;
    short          tmessage;
    char           buf[128];
    char           a1[MAX_INPUT_LENGTH];

    arg = one_argument(arg, a1);
    if (!*a1 || !(tmessage = atoi(a1))) {
        return FALSE;
    }

    if (GetMaxLevel(ch) < board->obj_flags.value[1]) {
        act("You start to write a message upon $p but quickly pull away as "
            "flames engulf your hand.", TRUE, ch, board, NULL, TO_CHAR);
        act("$n started to write upon $p but quickly pulled back in pain.", 
            TRUE, ch, board, NULL, TO_ROOM);
        return TRUE;
    }
    
    /*
     * Find the board we're dealing with 
     */ 
    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == obj_index[board->item_number].virtual) {
            break;
        }
    }

    if (bd == NULL) {
#if 0  
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, 
                "Bugginess in board %ld", 
                obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in reply_board_message - bd = 0");
        return TRUE;
    }
    if ((bd->num_posts == 0) || (tmessage < 0) || (tmessage > bd->num_posts)) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return TRUE;
    }
    
    /*
     * Need to find the message we're replying to in order to get the
     * title 
     */ 
    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->message_id == tmessage) {
            break;
        }
    }

    if (msg == NULL) {
#if 0  
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, 
                "Bugginess in board %ld", 
                obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in reply_board_message - msg = 0");
        return TRUE;
    }
    
    /*
     * skip blanks 
     */ 
    for (; isspace(*arg); arg++) {
        /* 
         * Empty loop 
         */
    }

    if (!*arg) {
        sprintf(buf, "re: %s", msg->title);
#if 0
        sprintf(buf, "re: %s", strip_ansi(ansi_buf, msg->title, 128) );
#endif
    } else {
        strncpy(buf, arg, 128);
    }
    new_board_message(ch, bd, buf, tmessage);
    return TRUE;
}

/*
 * Step through the messages and renumber them after one has been
 * removed. 
 */ 
void renumber_board(struct bulletin_board *bd, short message_id) 
{
    struct bulletin_board_message *msg;

    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->message_id > message_id) {
            msg->message_id--;
        }
        if (msg->reply_to > message_id) {
            msg->reply_to--;
        }
    }
    bd->num_posts--;
}

/*
 *  Recursively delete a post and all it's replies. 
 */ 
void delete_board_message(struct bulletin_board *bd, short message_id) 
{
    struct bulletin_board_message *msg,
                   *temp;
    
    /*
     * First check to see if we have a reply to this message -- if so
     * axe it 
     */ 
    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->reply_to == message_id) {
            /* 
             * we do, delete it as well 
             */
            delete_board_message(bd, msg->message_id);
        }
    }
    
    /*
     * Now find this message within the list 
     */ 
    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->message_id == message_id) {
            break;
        }
    }

    if (msg == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, 
                "Bugginess in board %ld", bd->board_num);
#endif 
        Log("Something went wrong in delete_board_message - msg = 0");
    return;
    }
    
    /*
     * remove it from the list of messages on this board 
     */ 
    if (msg == bd->messages) {
        bd->messages = msg->next;
    } else {
        temp = bd->messages;
        while (temp && (temp->next != msg)) {
            temp = temp->next;
        }
        if (temp) {
            temp->next = msg->next;
        }
    }
    free(msg->author);
    free(msg->title);
    free(msg->text);
    free(msg);
    renumber_board(bd, message_id);
}

bool remove_board_message(struct char_data *ch, char *arg,
                          struct obj_data *board) 
{
    struct bulletin_board *bd;
    struct bulletin_board_message *msg;
    int            tmessage;
    char           a1[MAX_INPUT_LENGTH],
                   buf[MAX_STRING_LENGTH];

    one_argument(arg, a1);
    if (!*a1 || !(tmessage = atoi(a1))) {
        return FALSE;
    }
    
    /*
     * Find the board we're dealing with 
     */ 
    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == obj_index[board->item_number].virtual) {
            break;
        }
    }

    if (bd == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL,
            "Bugginess in board %ld",
            obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in remove_board_message - bd = 0");
        return TRUE;
    }

    if ((bd->num_posts == 0) || (tmessage < 0) || (tmessage > bd->num_posts)) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return TRUE;
    }

    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->message_id == tmessage) {
            break;
        }
    }

    if (msg == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL,
            "Bugginess in board %ld",
            obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in reply_board_message - msg = 0");
        return TRUE;
    }

    if (GetMaxLevel(ch) < board->obj_flags.value[2] && 
        ch->desc->pos != msg->char_id) {
        act("You try to grab one of the notes off $p but only end up getting a"
            " nasty shock.", TRUE, ch, board, NULL, TO_CHAR);
        act("$n tried to remove a note from $p but only received a "
            "hair-styling shock!", TRUE, ch, board, NULL, TO_ROOM);
        return TRUE;
    }

    delete_board_message(bd, tmessage);
    send_to_char("Message removed.\r\n", ch);
    sprintf(buf, "$n just removed message %d.", tmessage);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);
#if 0   
    mprintf(line_log, 0, SEV_LOW, "%s just removed message %d from board %ld",
        GET_NAME(ch), tmessage, obj_index[board->item_number].virtual);
#endif
    sprintf(buf, "%s just removed message %d from board %ld",
            GET_NAME(ch), tmessage,
            obj_index[board->item_number].virtual);
    Log(buf);
    save_board(bd, obj_index[board->item_number].virtual);
    return TRUE;
}

bool display_board_message(struct char_data * ch, char *arg,
                           struct obj_data * board) 
{
    struct bulletin_board *bd;
    struct bulletin_board_message *msg;
    char           buffer[MAX_MESSAGE_LENGTH + 512];
    int            tmessage;
    char           a1[MAX_INPUT_LENGTH];
    one_argument(arg, a1);
    if (!*a1 || !(tmessage = atoi(a1))) {
        return FALSE;
    }
    if (GetMaxLevel(ch) < board->obj_flags.value[0]) {
        act("You simply cannot comprehend the alien markings upon $p.",
            TRUE, ch, board, NULL, TO_CHAR);
        act("$n tries to read $p, but looks bewildered.", TRUE, ch, board,
            NULL, TO_ROOM);
        return TRUE;
    }
    
    /*
     * Find out which board we're looking at 
     */ 
    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == obj_index[board->item_number].virtual) {
            break;
        }
    }

    if (bd == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, "Bugginess "
                "in board %ld", obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in display_board_message - bd = 0");
        return TRUE;
    }

    if ((bd->num_posts == 0) || (tmessage < 0) || (tmessage > bd->num_posts)) {
        send_to_char("That message exists only in your imagination.\r\n", ch);
        return TRUE;
    }
    
    /*
     * Now we need to find which message it is the person it trying to 
     * look at 
     */ 
    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->message_id == tmessage) {
            break;
        }
    }

    if (msg == NULL) {
#if 0 
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, "Bugginess in "
                "board %ld", obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in display_board_message - msg = 0");
        return TRUE;
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
    act("$n examines one of the posts on $p.", TRUE, ch, board, NULL, TO_ROOM);
    return TRUE;
}

/*
 *  Recursively display all the replies to a top level post. 
 */ 
void list_board_replies(struct bulletin_board *bd,
                        struct descriptor_data *d, short message_id,
                        short depth) 
{
    struct bulletin_board_message *msg;
    short          i;
    char           buf[128],
                   buf2[255];

    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->reply_to == message_id) {
            /* 
             * found a reply to an upper post 
             * 
             * indent the reply according to depth 
             */ 
            for (i = 0; i < depth; i++) {
                list_append(d, " ");
            }
            /*
             *  asctime has to be difficult and return a newline in
             *  it's output, we need to get rid of it 
             */ 
            strncpy(buf, asctime(localtime(&msg->date)), 128);

            for (i = 0; (i < 128) && *(buf + i); i++) {
                if ((buf[i] == '\n') || (buf[i] == '\r')) {
                    buf[i] = '\0';
                    break;
                }
            }
            
            /*
             * chop the last characters to make it line out pretty
             */ 
            sprintf(buf2, "%-35s", msg->title);

            for (i = 0; i < depth; i++) {
                buf2[strlen(buf2) - 1] = '\0';
            }

            list_append(d, "$c000W%3d$c000w : $c000c%s$c000w %-12s  "
                           "[$c000c%s$c000w]\r\n",
                       msg->message_id, buf2, msg->author, buf);
            
#if 0
            list_append(d, "$c000W%3d$c000w : %-10s [$c000R%s$c000w] "
                    "-- %s\r\n", msg->message_id, msg->author, buf, 
                    msg->title);
#endif
            list_board_replies(bd, d, msg->message_id, depth + 1);
        }
    }
}

/*
 *  This function lists the top-level posts and farms out to
 *  list_board_replies for a recursive listing of the replies to these 
 *  posts. 
 */ 
void list_board_messages(struct bulletin_board *bd,
                         struct descriptor_data *d) 
{
    struct bulletin_board_message *msg;
    short          i;
    char           buf[128];

    for (msg = bd->messages; msg; msg = msg->next) {
        if (msg->reply_to == -1) {
            /* 
             *  top-level post 
             *
             *  asctime has to be difficult and return a newline in
             *  it's output, we need to get rid of it 
             */ 
            strncpy(buf, asctime(localtime(&msg->date)), 128);

            for (i = 0; (i < 128) && *(buf + i); i++) {
                if ((buf[i] == '\n') || (buf[i] == '\r')) {
                    buf[i] = '\0';
                    break;
                }
            }

            list_append(d, "$c000W%3d$c000w : $c0008%-35s$c000w %-12s  "
                           "[$c0008%s$c000w]\r\n",
                        msg->message_id, msg->title, msg->author, buf);

            list_board_replies(bd, d, msg->message_id, 1);
        }
    }
}

/*
 *  Show topics and their replies in a threaded format. 
 */ 
bool show_board(struct char_data *ch, char *arg, struct obj_data *board) 
{
    struct bulletin_board *bd;
    char           a1[MAX_INPUT_LENGTH];

    one_argument(arg, a1);
    if (!*a1 || !isname(a1, "board bulletin")) {
        return FALSE;
    }
    /*
     * See if character's level is high enough to read board 
     */ 
    if (GetMaxLevel(ch) < board->obj_flags.value[0]) {
        act("You simply cannot comprehend the alien markings upon $p.",
             TRUE, ch, board, NULL, TO_CHAR);
        act("$n tries to read $p, but looks bewildered.", TRUE, ch, board,
             NULL, TO_ROOM);
        return TRUE;
    }
    
    /*
     * Find out which board we're looking at 
     */ 
    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == obj_index[board->item_number].virtual) {
            break;
        }
    }

    if (bd == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP | LOG_ASSERT, SEV_ALL, "Bugginess in "
                "board %ld", obj_index[board->item_number].virtual);
#endif
        Log("Something went wrong in show_board - bd = 0");
        return TRUE;
    }

    act("$n studies $p.", TRUE, ch, board, NULL, TO_ROOM);
    list_init(ch->desc);
    list_append(ch->desc, "This is a bulletin board.\r\n" 
                          "Usage: READ/REMOVE <#>, WRITE <title>, "
                          "REPLY <#> [title]\r\n");
    if (bd->num_posts == 0) {
        list_append(ch->desc, "%s appears to be empty.\r\n",
                     CAP(OBJS(board, ch)));
    } else {
        list_append(ch->desc, "There are %d messages on %s.\r\n\r\n" 
                              " Current Discussions:\r\n" 
                              "-----------------------------------------------"
                              "-----------------------------------\r\n",
                    bd->num_posts, OBJS(board, ch));
        list_board_messages(bd, ch->desc);
    }
    list_end(ch->desc);
    return TRUE;
}

/*
 *  Free all memory allocated to a board and remove it from the
 * global list. 
 */ 
void free_board(int board_num) 
{
    struct bulletin_board *bd,
                   *tmp;
    struct bulletin_board_message *msg,
                   *next_msg;

    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == board_num) {
            break;
        }
    }

    if (bd == NULL) {
#if 0
        mprintf(line_log, LOG_BEEP, SEV_ALL, "Bulletin board #%d is "
                "non-existant according to free_board", board_num);
#endif
        Log("Something went wrong in free_board - bd = 0");
        return;
    }
    
    /*
     * walk through and delete all the messages and text contained
     * within 
     */ 
    for (msg = bd->messages; msg; msg = next_msg) {
        next_msg = msg->next;
        free(msg->author);
        free(msg->title);
        free(msg->text);
        free(msg);
    }
    
    /*
     * remove this board from the global list 
     */ 
    if (bd == bboards) {
        bboards = bd->next;
    } else {
        tmp = bboards;
        while (tmp && (tmp->next != bd)) {
            tmp = tmp->next;
        }
        if (tmp != NULL) {
            tmp->next = bd->next;
        }
    }
}

/*
 *  Reads in a non-empty board from file. 
 */ 
struct bulletin_board *board_from_file(FILE * fl) 
{
    struct bulletin_board *bd;
    struct bulletin_board_message *msg,
                   *tmp;
    int            i = 0,
                   t[2];

    CREATE(bd, struct bulletin_board, 1);
    
    /*
     * Find out how many posts are on the board 
     */ 
    fscanf(fl, "%d\n", t);

    bd->num_posts = t[0];
    while (i < bd->num_posts) {
        CREATE(msg, struct bulletin_board_message, 1);
        msg->author = fread_string(fl);
        msg->title = fread_string(fl);
        msg->text = fread_string(fl);
        
        /*
         * Below I'm assuming the messages will always be written to 
         * file in order. If you change some board stuff around and
         * they stop working correctly this might be a place to start 
         * looking for the problem. 
         */ 
        msg->message_id = ++i;
        fscanf(fl, "%ld %d %d %d\n", &msg->date, &msg->char_id, t, t + 1);
        msg->reply_to = t[0];
        msg->language = t[1];
        
        /*
         * add it to the end of the list of messages 
         */ 
        if (bd->messages == NULL) {
            bd->messages = msg;
        } else {
            tmp = bd->messages;
            while (tmp->next != NULL) {
                tmp = tmp->next;
            }
            tmp->next = msg;
        }
    }
    return bd;
}

/*
 *  Load a board if it hasn't already been done. 
 */ 
void check_board(int board_num) 
{
    FILE * fl;
    struct bulletin_board *bd;
    char           buf[128];

    for (bd = bboards; bd; bd = bd->next) {
        if (bd->board_num == board_num) {
            /* 
             * Already in the list, just leave 
             */
            return;
        }
    }
    
    /*
     * It wasn't in the list, we need to load it if it exists 
     */ 
    sprintf(buf, "boards/%05d", board_num);

    if ((fl = fopen(buf, "rt")) == NULL) {
        /*
         * the board is empty 
         */ 
        CREATE(bd, struct bulletin_board, 1);
        bd->num_posts = 0;
        bd->messages = NULL;
    } else {
        bd = board_from_file(fl);
        fclose(fl);
    }
    bd->board_num = board_num;
    
    /*
     * Board has been initialized, add it to the list 
     */ 
    bd->next = bboards;
    bboards = bd;
}

int board(struct char_data *ch, int cmd, char *arg,
          struct obj_data *obj, int type) 
{
    char           a1[MAX_INPUT_LENGTH];

    if ((type != PULSE_COMMAND) || (ch->desc == NULL)) {
        return FALSE;
    }
    
    /*
     *  Call me paranoid but I just want to make sure the check_board 
     * procedure isn't hit all the time on a big MUD with lots of
     * boards and busy board rooms. 
     */ 
    if ((cmd != cmd_look) && (cmd != cmd_write) && (cmd != cmd_read) && 
        (cmd != cmd_remove) && (cmd != cmd_reply) && (cmd != cmd_reload)) {
        return FALSE;
    }
    

    check_board(obj_index[obj->item_number].virtual);
    if (cmd == cmd_look) {
        return show_board(ch, arg, obj);
    }
    if (cmd == cmd_read) {
        return display_board_message(ch, arg, obj);
    }
    if (cmd == cmd_remove) {
        return remove_board_message(ch, arg, obj);
    }
    if (cmd == cmd_reply) {
        return reply_board_message(ch, arg, obj);
    }
    if (cmd == cmd_reload) {
        if (!IS_IMMORTAL(ch)) {
            return FALSE;
        }
        arg = one_argument(arg, a1);
        
        /*
         *  if we're reloading, we only have to free here.
         * check_board will do the actual reloading on the next
         * relevant board command. 
         */ 
        if (!strcmp("board", a1)) {
            free_board(obj_index[obj->item_number].virtual);
#if 0 
            mprintf(line_log, 0, SEV_LOW, "%s just reset bulletin board #%ld", 
                    GET_NAME(ch), obj_index[obj->item_number].virtual);
#endif
            Log("Someone just reset a board");
            return TRUE;
        }
        return FALSE;
    }
    
    /*
     * only other command 
     */ 
    write_board_message(ch, arg, obj);
    return TRUE;
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
