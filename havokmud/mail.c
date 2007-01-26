
#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <time.h>

#include "protos.h"
#include "externs.h"


#define BUGMAIL 521

/*
 * command numbers of the "mail", "check", and "receive" commands in your
 * interpreter. 
 */
#define CMD_MAIL        366
#define CMD_CHECK       367
#define CMD_RECEIVE     368

int PostMaster(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    if (!ch->desc) {
        /* so mobs don't get caught here */
        return (FALSE);
    }

    switch (cmd) {
    case CMD_MAIL:
        /* mail */
        postmaster_send_mail(ch, cmd, arg);
        return 1;
        break;
    case CMD_CHECK: 
        /* check */
        postmaster_check_mail(ch, cmd, arg);
        return 1;
        break;
    case CMD_RECEIVE:
        /* receive */
        postmaster_receive_mail(ch, cmd, arg);
        return 1;
        break;
    default:
        return (FALSE);
        break;
    }

    return (FALSE);
}


/*
 * find the postmaster in the room where ch is standing 
 */
struct char_data *find_mailman(struct char_data *ch)
{
    struct char_data *mailman;

    mailman = FindMobInRoomWithFunction(ch->in_room, PostMaster);

    if (!mailman) {
        send_to_char("Whoa!  Buggy post office.  Please report this.  "
                     "Error #10.\n\r", ch);
    }
    return mailman;
}

void postmaster_send_mail(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *mailman = NULL;
    char            buf[200];

    if (cmd != BUGMAIL) {
        if (!(mailman = find_mailman(ch))) {
            return;
        }
        if (GetMaxLevel(ch) < MIN_MAIL_LEVEL) {
            sprintf(buf, "$N tells you, 'Sorry, you have to be level %d to "
                         "send mail!'", MIN_MAIL_LEVEL);
            act(buf, FALSE, ch, 0, mailman, TO_CHAR);
            return;
        }
    }

    if (!arg || !*arg) {
        act("$N tells you, 'Who did you want me to send this to?'",
            FALSE, ch, 0, mailman, TO_CHAR);
        return;
    }

    if (cmd != BUGMAIL && GET_GOLD(ch) < STAMP_PRICE) {
        sprintf(buf, "$N tells you, 'It will costs %d coins to deliever "
                     "this.'\n\r"
                     "$N tells you, '...which I see you can't afford.'",
                     STAMP_PRICE);
        act(buf, FALSE, ch, 0, mailman, TO_CHAR);
        return;
    }

    if (find_name(arg) < 0) {
        act("$N tells you, 'Never heard of that person!'",
            FALSE, ch, 0, mailman, TO_CHAR);
        return;
    }

    act("$n starts to write a note.", TRUE, ch, 0, 0, TO_ROOM);
    if (cmd != BUGMAIL) {
        sprintf(buf, "$N tells you, 'I'll take %d coins for the delivery.'\n\r"
                     "$N tells you, 'Write your message, use /w when done. "
                     "Use /? for more info on editing strings.'", STAMP_PRICE);
        act(buf, FALSE, ch, 0, mailman, TO_CHAR);
        GET_GOLD(ch) -= STAMP_PRICE;
    } else {
        act("Havok MUD thanks you for reporting your bugs.\n\r"
            "Write your message, use /w when done.\n\r"
            "Use /? for more info on editing strings.", FALSE, ch, 0, mailman,
            TO_CHAR);
    }
    SET_BIT(ch->specials.act, PLR_MAILING);

    ch->desc->name = (char *) strdup(arg);
    ch->desc->str = (char **) malloc(sizeof(char *));
    *(ch->desc->str) = NULL;
    ch->desc->max_str = MAX_MAIL_SIZE;

    /* 
     * The mail will be actually sent by db_store_mail() once the editor's 
     * done 
     */
}

void bugmail(struct char_data *ch, char *arg, int cmd)
{
    postmaster_send_mail(ch, cmd, "Gordon");
}

void postmaster_check_mail(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *mailman;
    char            buf[200];

    if (!(mailman = find_mailman(ch))) {
        return;
    }

    if (db_has_mail(GET_NAME(ch))) {
        sprintf(buf, "$n tells you, 'You DO have waiting mail.'");
    } else {
        sprintf(buf, "$n tells you, 'Sorry, you DON'T have any messages "
                     "waiting.'");
    }
    act(buf, FALSE, mailman, 0, ch, TO_VICT);
}

void postmaster_receive_mail(struct char_data *ch, int cmd, char *arg)
{
    struct char_data *mailman;
    char            buf[200];
    struct obj_data *tmp_obj;
    int             count;
    int             i;
    int            *messageNum;

    if (!(mailman = find_mailman(ch))) {
        return;
    }

    if (!(count = db_has_mail(GET_NAME(ch)))) {
        sprintf(buf, "$n tells you, 'Sorry, you don't have any messages "
                     "waiting.'");
        act(buf, FALSE, mailman, 0, ch, TO_VICT);
        return;
    }

    messageNum = (int *)malloc(count * sizeof(int));
    if( !messageNum ) {
        Log("Out of memory in postmaster_receive_mail!");
        send_to_char("Ouch, I'm out of memory!\n\r", ch);
        return;
    }

    if( !(count = db_get_mail_ids(GET_NAME(ch), messageNum, count)) ) {
        Log("Mail wierdness!");
        send_to_char("I can't seem to find the messages!\n\r", ch);
        free(messageNum);
        return;
    }

    for( i = 0; i < count; i++ ) {
        CREATE(tmp_obj, struct obj_data, 1);
        clear_object(tmp_obj);

        tmp_obj->name = strdup("mail paper letter");
        tmp_obj->short_description = strdup("a piece of mail");
        tmp_obj->description = strdup("Someone has left a piece of mail here.");

        tmp_obj->obj_flags.type_flag = ITEM_NOTE;
        tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
        tmp_obj->obj_flags.weight = 1;
        tmp_obj->obj_flags.cost = 30;
        tmp_obj->obj_flags.cost_per_day = 10;

        tmp_obj->action_description = db_get_mail_message(messageNum[i]);
        if (!tmp_obj->action_description) {
            tmp_obj->action_description = strdup("Mail system error.\n\r");
        }

        db_delete_mail_message(messageNum[i]);

        tmp_obj->next = object_list;
        object_list = tmp_obj;

        obj_to_char(tmp_obj, ch);

        tmp_obj->item_number = -1;

        act("$n gives you a piece of mail.", FALSE, mailman, 0, ch, TO_VICT);
        act("$N gives $n a piece of mail.", FALSE, ch, 0, mailman, TO_ROOM);
    }

    ch->player.has_mail = FALSE;
    free(messageNum);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
