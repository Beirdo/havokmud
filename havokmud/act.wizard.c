/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"

#define STATUE_ZONE 198

void            switch_light(byte why);
void            PulseMobiles(int cmd);
int             ZoneCleanable(int zone);
char           *view_newhelp(void);

/*
 * external vars
 */

char            EasySummon = 1;
int             MinArenaLevel = 0,
                MaxArenaLevel = 0,
                Quadrant = 0;
int             ArenaNoGroup,
                ArenaNoAssist,
                ArenaNoDispel,
                ArenaNoMagic,
                ArenaNoWSpells,
                ArenaNoSlay,
                ArenaNoFlee,
                ArenaNoHaste,
                ArenaNoPets,
                ArenaNoTravel,
                ArenaNoBash;


void            update_pos(struct char_data *victim);
void do_load_mobile(struct char_data *ch, char *argument, int number);
void do_load_object(struct char_data *ch, char *argument, int number);
void do_load_room(struct char_data *ch, char *argument);

void do_show_zones( struct char_data *ch, struct string_block *sb, 
                    char *argument )
void do_show_objects( struct char_data *ch, struct string_block *sb, 
                      char *argument );
void do_show_wearslot( struct char_data *ch, struct string_block *sb, 
                       char *argument );
void do_show_itemtype( struct char_data *ch, struct string_block *sb, 
                       char *argument );
void do_show_mobiles( struct char_data *ch, struct string_block *sb, 
                      char *argument );
void do_show_rooms( struct char_data *ch, struct string_block *sb, 
                    char *argument );
void do_show_report( struct char_data *ch, struct string_block *sb, 
                     char *argument );
void do_show_maxxes( struct char_data *ch, struct string_block *sb, 
                     char *argument );


void do_auth(struct char_data *ch, char *argument, int cmd)
{
    char           *name;
    int             done = FALSE;
    struct descriptor_data *d;

    dlog("in do_auth");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     * get char name
     */
    argument = get_argument(argument, &name);
    if( !name ) {
        send_to_char( "Just who are you trying to auth?\n\r", ch );
    }

    /*
     * search through descriptor list for player name
     */
    for (d = descriptor_list; d && !done; d = d->next) {
        if (d->character && GET_NAME(d->character) &&
            strcasecmp(GET_NAME(d->character), name) == 0) {
            done = TRUE;
            break;
        }
    }

    /*
     * if not found, return error
     */
    if (!d || IS_IMMORTAL(d->character)) {
        send_to_char("That player was not found.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);

    if (argument) {
        /*
         * get response (rest of argument)
         */
        if (strcasecmp(argument, "yes") == 0) {
            d->character->generic = NEWBIE_START;
            Log("%s has just accepted %s into the game.", ch->player.name,
                name);
            SEND_TO_Q("You have been accepted.  Press enter\n\r", d);
        } else if (strcasecmp(argument, "no") == 0) {
            SEND_TO_Q("You have been denied.  Press enter\n\r", d);
            Log("%s has just denied %s from the game.", ch->player.name, name);
            d->character->generic = NEWBIE_AXE;
        } else {
            SEND_TO_Q(argument, d);
            SEND_TO_Q("\n\r", d);
            oldSendOutput(ch, "You send '%s'\n\r", argument);
        }
    } else {
        send_to_char("Auth[orize] {Yes | No | Message} \n\r", ch);
    }
}


void do_passwd(struct char_data *ch, char *argument, int cmdnum)
{
    int             player_i,
                    pos;
    char           *name,
                   *npasswd,
                    pass[20];
    struct char_file_u tmp_store;
    FILE           *fl;

    dlog("in do_passwd");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     *  get user's name:
     */
    argument = get_argument(argument, &name);
    argument = get_argument(argument, &npasswd);

    /*
     *   Look up character
     */
    if (name && (player_i = load_char(name, &tmp_store)) > -1) {
        /*
         *  encrypt new password.
         */
        if (!npasswd || strlen(npasswd) > 10) {
            send_to_char("Illegal password\n\r", ch);
            return;
        }
        strncpy(pass, (char *) crypt(npasswd, tmp_store.name), 10);

        /*
         *  put new password in place of old password
         */
        *(pass + 10) = '\0';
        strcpy(tmp_store.pwd, pass);

        /*
         *   save char to file
         */
        pos = player_table[player_i].nr;
        if (!(fl = fopen(PLAYER_FILE, "r+"))) {
            perror("do_passwd");
            assert(0);
        }

        rewind(fl);
        fseek(fl, pos * sizeof(struct char_file_u), 0);
        fwrite(&tmp_store, sizeof(struct char_file_u), 1, fl);
        fclose(fl);
    } else {
        send_to_char("I don't recognize that name\n\r", ch);
    }
}

void do_setsev(struct char_data *ch, char *arg, int cmd)
{
    char           *arg1;
    int             sev;

    dlog("in do_setsev");
    arg = get_argument(arg, &arg1);

    if (arg1) {
        sev = atoi(arg1);
        if (sev < 0) {
            send_to_char("That number was incorrect, please choose > 0\n\r",
                         ch);
            return;
        }
        ch->specials.sev = sev;
        oldSendOutput(ch, "Your severety level have been set to %d.\n\r",
                      ch->specials.sev);
    } else {
        send_to_char("Please give a number between 0 and 5\n\r", ch);
    }
}

void dsearch(char *string, char *tmp)
{
    char           *c,
                    buf[255],
                    buf2[255],
                    buf3[255];
    int             i,
                    j;

    i = 0;
    while (i == 0) {
        if (strchr(string, '~') == NULL) {
            i = 1;
            strcpy(tmp, string);
        } else {
            c = strchr(string, '~');
            j = c - string;
            switch (string[j + 1]) {
            case 'N':
                strcpy(buf2, "$n");
                break;
            case 'H':
                strcpy(buf2, "$s");
                break;
            default:
                strcpy(buf2, "");
                break;
            }
            strcpy(buf, string);
            buf[j] = '\0';
            strcpy(buf3, (string + j + 2));
            sprintf(tmp, "%s%s%s", buf, buf2, buf3);
            sprintf(string, tmp);
        }
    }
}

void do_bamfin(struct char_data *ch, char *arg, int cmd)
{
    char            buf[255];
    int             len;

    dlog("in do_bamfin");
    /*
     * pass all those spaces
     */
    arg = skip_spaces(arg);
    if (!arg) {
        send_to_char("Your current bamfin is:\n\r", ch);
        act(ch->specials.poofin, FALSE, ch, 0, NULL, TO_CHAR);
        send_to_char("Bamfin <bamf definition>\n\r"
                     " Additional arguments can include ~N for where you\n\r"
                     " Want your name (if you want your name).  If you use\n\r",
                     ch);
        send_to_char(" ~H, it puts in either his or her depending on your\n\r"
                     " sex.  If you use the keyword 'def' for your bamf,\n\r"
                     " it turns on the default bamf.  \n\r", ch);
        return;
    }

    if (!strcmp(arg, "def")) {
        REMOVE_BIT(ch->specials.pmask, BIT_POOF_IN);
        if (ch->specials.poofin) {
            free(ch->specials.poofin);
        }
        ch->specials.poofin = 0;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    len = strlen(arg);

    if (ch->specials.poofin && len >= strlen(ch->specials.poofin)) {
        free(ch->specials.poofin);
        ch->specials.poofin = (char *) malloc(len + 1);
    } else if (!ch->specials.poofin) {
        ch->specials.poofin = (char *) malloc(len + 1);
    }
    strcpy(buf, arg);
    dsearch(buf, ch->specials.poofin);
    SET_BIT(ch->specials.pmask, BIT_POOF_IN);
    send_to_char("Ok.\n\r", ch);
}

void do_bamfout(struct char_data *ch, char *arg, int cmd)
{
    char            buf[255];
    int             len;

    dlog("in do_bamfout");
    /*
     * pass all those spaces
     */
    arg = skip_spaces(arg);
    if (!arg) {
        send_to_char("Your current bamfout is:\n\r", ch);
        act(ch->specials.poofout, FALSE, ch, 0, NULL, TO_CHAR);
        send_to_char("Bamfout <bamf definition>\n\r"
                     "  Additional arguments can include ~N for where you\n\r"
                     "  Want your name (if you want your name). If you use\n\r",
                     ch);
        send_to_char("  ~H, it puts in either his or her depending on your\n\r"
                     "  sex.  If you use the keyword 'def' for your bamf,\n\r"
                     "  it turns on the default bamf.  \n\r", ch);
        return;
    }

    if (!strcmp(arg, "def")) {
        REMOVE_BIT(ch->specials.pmask, BIT_POOF_OUT);
        if (ch->specials.poofout) {
            free(ch->specials.poofout);
        }
        ch->specials.poofout = 0;
        send_to_char("Ok.\n\r", ch);
        return;
    }

    len = strlen(arg);
    if (ch->specials.poofout && len >= strlen(ch->specials.poofout)) {
        free(ch->specials.poofout);
        ch->specials.poofout = (char *) malloc(len + 1);
    } else if (!ch->specials.poofout) {
        ch->specials.poofout = (char *) malloc(len + 1);
    }

    strcpy(buf, arg);
    dsearch(buf, ch->specials.poofout);
    SET_BIT(ch->specials.pmask, BIT_POOF_OUT);
    send_to_char("Ok.\n\r", ch);
}

void do_zsave(struct char_data *ch, char *argument, int cmdnum)
{
    int             start_room,
                    end_room,
                    zone;
    FILE           *fp;
    char           *arg1,
                   *arg2,
                   *arg3;

    dlog("in do_zsave");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     *   read in parameters (room #s)
     */

    argument = get_argument(argument, &arg1);
    argument = get_argument(argument, &arg2);
    argument = get_argument(argument, &arg3);

    if( !arg1 ) {
        send_to_char("Zsave <zone_number> [<start_room> <end_room>]\n\r", ch);
        return;
    }
    zone = atoi(arg1);

    if (zone > top_of_zone_table) {
        send_to_char("Invalid zone number\r\n", ch);
        return;
    }

    /*
     * make some permission checks
     */
    if (GetMaxLevel(ch) < 56 && zone != GET_ZONE(ch)) {
        send_to_char("Sorry, you are not authorized to save this zone.\n\r",
                     ch);
        return;
    }

    if (!zone_table[zone].start) {
        send_to_char("Sorry, that zone isn't initialized yet\r\n", ch);
        return;
    }

    if( !arg2 || !arg3 ) {
        start_room = zone ? (zone_table[zone - 1].top + 1) : 0;
        end_room = zone_table[zone].top;
    } else {
        start_room = atoi(arg2);
        end_room   = atoi(arg3);
    }

    fp = (FILE *) MakeZoneFile(ch, zone);
    if (!fp) {
        send_to_char("Couldn't make file.. try again later\n\r", ch);
        return;
    }

    fprintf(fp, "*Zone %d, rooms %d-%d, last modified by %s\n", zone,
                start_room, end_room, ch->player.name);

    SaveZoneFile(fp, start_room, end_room);
    fclose(fp);
    send_to_char("Ok\r\n", ch);
}

void do_zload(struct char_data *ch, char *argument, int cmdnum)
{
    int             zone;
    FILE           *fp;

    dlog("in do_zload");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     *   read in parameters (room #s)
     */
    zone = -1;

    argument = skip_spaces(argument);
    if (!argument || (zone = atoi(argument)) < 1) {
        send_to_char("Zload <zone_number>\n\r", ch);
        return;
    }
    if (zone > top_of_zone_table) {
        send_to_char("Invalid zone number\r\n", ch);
        return;
    }
    /*
     * make some permission checks
     */
    if ((GetMaxLevel(ch) < 56 && zone != GET_ZONE(ch)) ) {
        send_to_char("Sorry, you are not authorized to load this zone.\n\r",
                     ch);
        return;
    }
    fp = (FILE *) OpenZoneFile(ch, zone);
    if (!fp) {
        send_to_char("Couldn't open zone file..\n\r", ch);
        return;
    }
    /*
     * oh well, do some things with that zone
     */
    CleanZone(zone);

#if 0
    send_to_zone("\r\n\r\nSwirling winds of Chaos reform reality around "
            "you!\r\n\r\n",ch);
#endif

    LoadZoneFile(fp, zone);
    fclose(fp);
    renum_zone_table(zone);
    zone_table[zone].start = 0;
    reset_zone(zone, cmdnum);
    if (zone == STATUE_ZONE) {
        /*
         * if statue zone, make sure the statues are rebuilt too!
         */
        generate_legend_statue();
    }
    send_to_char("Ok\r\n", ch);
}

void do_zclean(struct char_data *ch, char *argument, int cmdnum)
{
    int             zone = -1;
    struct room_data *rp;

    dlog("in do_zclean");

    if (IS_NPC(ch)) {
        return;
    }

    argument = skip_spaces(argument);
    if (!argument || (zone = atoi(argument)) < 1) {
        send_to_char("Zclean <zone_number> (and don't even think about "
                     "cleaning Void)\n\r", ch);
        return;
    }
    /*
     * make some permission checks
     */
    rp = real_roomp(ch->in_room);
    if (GetMaxLevel(ch) < 56 && rp->zone != GET_ZONE(ch)) {
        send_to_char("Sorry, you are not authorized to clean this zone.\n\r",
                     ch);
        return;
    }
    CleanZone(zone);
    send_to_char("4 3 2 1 0, Boom!\r\n", ch);
}

void do_highfive(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;
    char            mess[120];
    struct char_data *tch;

    dlog("in do_highfive");

    if (argument) {
        argument = get_argument(argument, &buf);
        if (buf && (tch = get_char_room_vis(ch, buf)) != 0) {
            if ((GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch)) &&
                (GetMaxLevel(ch) >= DEMIGOD) && (!IS_NPC(ch))) {
                sprintf(mess, "Time stops for a moment as %s and %s high "
                              "five.\n\r",
                        ch->player.name, tch->player.name);
                send_to_all(mess);
            } else {
                act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
                act("You give a hearty high five to $N", TRUE, ch, 0, tch,
                    TO_CHAR);
                act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
            }
        } else {
            sprintf(mess, "I don't see anyone here like that.\n\r");
            send_to_char(mess, ch);
        }
    }
}

void do_silence(struct char_data *ch, char *argument, int cmd)
{
    extern int      Silence;

    dlog("in do_silence");

    if ((GetMaxLevel(ch) < DEMIGOD) || (IS_NPC(ch))) {
        send_to_char("You cannot Silence.\n\r", ch);
        return;
    }
    if (Silence == 0) {
        Silence = 1;
        send_to_char("You have now silenced polyed mobles.\n\r", ch);
        Log("%s has stopped Polymophed characters from shouting.",
            ch->player.name);
    } else {
        Silence = 0;
        send_to_char("You have now unsilenced mobles.\n\r", ch);
        Log("%s has allowed Polymophed characters to shout.", ch->player.name);
    }
}

void do_rload(struct char_data *ch, char *argument, int cmd)
{
    int             i;
    int             start,
                    end;
    char           *arg1,
                   *arg2;

    dlog("in do_rload");

    if (IS_NPC(ch)) {
        return;
    }
    if (GetMaxLevel(ch) < IMMORTAL) {
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("rload <start> [<end>]\n\r", ch);
        return;
    }

    argument = get_argument( argument, &arg1 );
    argument = get_argument( argument, &arg2 );

    if( !arg1 ) {
        return;
    }
    start = atoi(arg1);

    if( !arg2 ) {
        end = start;
    } else {
        end = atoi(arg2);
    }

    if (end < start) {
        send_to_char("Hey, end room must be >= start room\r\n", ch);
        return;
    }
    /*
     * We have a problem here.. Infinite loop? (GH) Sept15,2002
     */
    for (i = 0; start > zone_table[i].top && i <= top_of_zone_table; i++) {
#if 0
        sprintf(buf,"I=%d start=%d zoneTableI=%d top of zone=%d",
        i,start,zone_table[i].top, top_of_zone_table);
        printf(buf);
#endif
    }
    if (i > top_of_zone_table) {
        send_to_char("Strange, start room is outside of any zone.\r\n", ch);
        return;
    }
    if (end > zone_table[i].top) {
        send_to_char("Forget about it, end room is outside of start room "
                     "zone ;)\r\n", ch);
        return;
    }
    RoomLoad(ch, start, end);
}

void do_rsave(struct char_data *ch, char *argument, int cmd)
{
    long            start,
                    end,
                    i;
    char           *arg1,
                   *arg2;

    dlog("in do_rsave");

    if (IS_NPC(ch)) {
        return;
    }
    if (GetMaxLevel(ch) < IMMORTAL) {
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        start = ch->in_room;
        end   = ch->in_room;
        if (!(start > 0 && start < WORLD_SIZE)) {
            send_to_char("Save? rsave <startnum> [<endnum>].\n\r", ch);
            return;
        }
    } else {
        argument = get_argument(argument, &arg1);
        argument = get_argument(argument, &arg2);

        if( !arg1 ) {
            return;
        }

        start = atol(arg1);

        if( !arg2 ) {
            end = start;
        } else {
            end = atol(arg2);
        }
    }

    if (end < start) {
        send_to_char("Hey, end room must be >= start room\r\n", ch);
        return;
    }
    if (start > WORLD_SIZE || end > WORLD_SIZE) {
        send_to_char("Nope, range to large.\n\r", ch);
        return;
    }
    for (i = 0; start > zone_table[i].top && i <= top_of_zone_table; i++) {
        /*
         * Empty loop
         */
    }
    if (i > top_of_zone_table) {
        send_to_char("Strange, start room is outside of any zone.\r\n", ch);
        return;
    }
    if (end > zone_table[i].top) {
        send_to_char("Forget about it, end room is outside of start room "
                     "zone ;)\r\n", ch);
        return;
    }
    RoomSave(ch, start, end);
}

void do_emote(struct char_data *ch, char *arg, int cmd)
{
    char            buf[MAX_INPUT_LENGTH + 40];
    dlog("in do_emote");

    if (check_soundproof(ch)) {
        return;
    }

    arg = skip_spaces(arg);
    if (!arg) {
        send_to_char("Yes.. But what?\n\r", ch);
        return;
    }

    sprintf(buf, "$n %s", arg);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if (IS_SET(ch->specials.act, PLR_ECHO)) {
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    } else {
        send_to_char("Ok.\n\r", ch);
    }
}


void do_echo(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH + 80];

    dlog("in do_echo");

    if (IS_NPC(ch)) {
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        if (IS_SET(ch->specials.act, PLR_ECHO)) {
            send_to_char("echo off\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_ECHO);
        } else {
            SET_BIT(ch->specials.act, PLR_ECHO);
            send_to_char("echo on\n\r", ch);
        }
    } else {
        if (IS_IMMORTAL(ch)) {
            sprintf(buf, "%s\n\r", argument);
            send_to_room_except(buf, ch->in_room, ch);
            send_to_char("Ok.\n\r", ch);
        }
    }
}

void do_system(struct char_data *ch, char *argument, int cmd)
{
    char            buf[2256];

    dlog("in do_system");

    if (IS_NPC(ch)) {
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("That must be a mistake...\n\rTry arguments 1-8 (Info, "
                     "Ann, Upd, sys, warn, reb, wel, note)\n\r", ch);
        return;
    }

    switch (*argument) {
    case '1':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RINFO$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '2':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RANNOUNCEMENT$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '3':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RUPDATE$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '4':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RSYSTEM$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '5':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RWARNING$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '6':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RREBOOT$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '7':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RWELCOME$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;
    case '8':
        /*
         * Remove the number..
         */
        argument++;
        sprintf(buf, "$c000Y-=$c000RNOTE$c000Y=-$c000w %s\n\r$c000w",
                argument);
        break;

    default:
        sprintf(buf, "%s\n\r", argument);
        break;
    }

    send_to_all(buf);
}

void do_trans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char           *buf;
    long            target;

    dlog("in do_trans");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        send_to_char("Who do you wich to transfer?\n\r", ch);
    } else if (strcasecmp("all", buf)) {
        if (!(victim = get_char_vis_world(ch, buf, NULL))) {
            send_to_char("No-one by that name around.\n\r", ch);
        } else {
            act("$n disappears in a cloud of mushrooms.", FALSE, victim, 0,
                0, TO_ROOM);
            target = ch->in_room;
            char_from_room(victim);
            char_to_room(victim, target);
            act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0,
                TO_ROOM);
            act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
            do_look(victim, NULL, 15);
            send_to_char("Ok.\n\r", ch);
        }
    } else {
        /*
         * Trans All
         */
        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected) {
                victim = i->character;
                act("$n disappears in a cloud of mushrooms.", FALSE,
                    victim, 0, 0, TO_ROOM);
                target = ch->in_room;
                char_from_room(victim);
                char_to_room(victim, target);
                act("$n arrives from a puff of smoke.", FALSE, victim, 0,
                    0, TO_ROOM);
                act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
                do_look(victim, NULL, 15);
            }
        }
        send_to_char("Ok.\n\r", ch);
    }
}

void do_qtrans(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char           *buf;
    long            target;

    dlog("in do_qtrans");

    if (IS_NPC(ch)) {
        return;
    }
    if (!IS_AFFECTED2(ch, AFF2_QUEST)) {
        send_to_char("Alas, you cannot qtrans without sporting a quest "
                     "flag.\n\r", ch);
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        send_to_char("Usage: qtrans <name/all>\n\r", ch);
    } else if (strcasecmp("all", buf)) {
        if (!(victim = get_char_vis_world(ch, buf, NULL))) {
            send_to_char("No-one by that name around.\n\r", ch);
        } else {
            if (IS_AFFECTED2(victim, AFF2_QUEST)) {
                act("$n has gone off to help out an immortal.", FALSE,
                    victim, 0, 0, TO_ROOM);
                target = ch->in_room;
                char_from_room(victim);
                char_to_room(victim, target);
                act("$n suddenly materializes, ready to embark on the quest.",
                    FALSE, victim, 0, 0, TO_ROOM);
                act("$n has called upon your help, you're unable to resist!",
                    FALSE, ch, 0, victim, TO_VICT);
                do_look(victim, NULL, 15);
                send_to_char("Ok.\n\r", ch);
            } else {
                send_to_char("Alas, this person does not wish to aid you in "
                             "your quest.\n\r", ch);
            }
        }
    } else {
        /*
         * qtrans all
         */
        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                IS_AFFECTED2(i->character, AFF2_QUEST)) {
                victim = i->character;
                act("$n has gone off to help out an immortal.", FALSE, victim,
                    0, 0, TO_ROOM);
                target = ch->in_room;
                char_from_room(victim);
                char_to_room(victim, target);
                act("$n suddenly materializes, ready to embark on the quest.",
                    FALSE, victim, 0, 0, TO_ROOM);
                act("$n has called upon your help, you're unable to resist!",
                    FALSE, ch, 0, victim, TO_VICT);
                do_look(victim, NULL, 15);
            }
        }
        send_to_char("Ok.\n\r", ch);
    }
}

void do_set_nooutdoor(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_set_nooutdoor");

    if (!ch) {
        return;
    }
    if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
        return;
    }
    if (IS_AFFECTED2(ch, AFF2_NO_OUTDOOR)) {
        /*
         * clean up that flag outta sa2
         */
        REMOVE_BIT(ch->specials.affected_by2, AFF2_NO_OUTDOOR);
    }
    /*
     * now use the proper place for this one
     */
    if (IS_SET(ch->specials.act, PLR_NOOUTDOOR)) {
        act("You open your senses to time and weather conditions in the realm.",
            TRUE, ch, 0, 0, TO_CHAR);
        REMOVE_BIT(ch->specials.act, PLR_NOOUTDOOR);
    } else {
        /*
         * imms can go into nooutdoor mode
         */
        act("You decide to ignore time and weather.", TRUE, ch, 0, 0, TO_CHAR);
        SET_BIT(ch->specials.act, PLR_NOOUTDOOR);
    }
}

void do_at(struct char_data *ch, char *argument, int cmd)
{
    char           *command,
                   *loc_str;
    int             loc_nr,
                    location,
                    original_loc;
    struct char_data *target_mob;
    struct obj_data *target_obj;

    dlog("in do_at");
    /*
     * this command is used for scrying.. mobiles should be able to scry,
     * in case of switch/poly
     */
#if 0
    if (IS_NPC(ch)) {
        return;
    }
#endif

    argument = get_argument(argument, &loc_str);
    command = skip_spaces(argument);

    if (!loc_str || !command) {
        send_to_char("You must supply a room number or a name and a "
                     "command.\n\r", ch);
        return;
    }

    if (isdigit((int)*loc_str)) {
        loc_nr = atoi(loc_str);
        if (NULL == real_roomp(loc_nr)) {
            send_to_char("No room exists with that number.\n\r", ch);
            return;
        }
        location = loc_nr;
    } else if ((target_mob = get_char_vis(ch, loc_str)) != NULL) {
        location = target_mob->in_room;
    } else if ((target_obj = get_obj_vis_world(ch, loc_str, NULL)) != NULL) {
        if (target_obj->in_room != NOWHERE) {
            location = target_obj->in_room;
        } else {
            send_to_char("The object is not available.\n\r", ch);
            return;
        }
    } else {
        send_to_char("No such creature or object around.\n\r", ch);
        return;
    }
    /*
     * a location has been found.
     */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    /*
     * check if the guy's still there
     */
    for (target_mob = real_roomp(location)->people; target_mob;
         target_mob = target_mob->next_in_room) {
        if (ch == target_mob) {
            char_from_room(ch);
            char_to_room(ch, original_loc);
        }
    }
}

void do_goto(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;
    int             loc_nr,
                    location,
                    i;
    struct char_data *target_mob,
                   *pers,
                   *v = NULL;
    struct obj_data *target_obj;

    dlog("in do_goto");

    if (!IS_IMMORTAL(ch)) {
        send_to_char("Huh?\n\r", ch);
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        send_to_char("You must supply a room number or a name.\n\r", ch);
        return;
    }
    if (isdigit((int)*buf) && !index(buf, '.')) {
        loc_nr = atoi(buf);
        if (!real_roomp(loc_nr)) {
            if (GetMaxLevel(ch) < CREATOR || loc_nr < 0) {
                send_to_char("No room exists with that number.\n\r", ch);
                return;
            } else {
#ifndef HASH
                if (loc_nr < WORLD_SIZE) {
#endif
                    send_to_char("You form order out of chaos.\n\r", ch);
                    CreateOneRoom(loc_nr);

#ifndef HASH
                } else {
                    send_to_char("Sorry, that room # is too large.\n\r",
                                 ch);
                    return;
                }
#endif
            }
        }
        location = loc_nr;
    } else if ((target_mob = get_char_vis_world(ch, buf, NULL))) {
        location = target_mob->in_room;
    } else if ((target_obj = get_obj_vis_world(ch, buf, NULL))) {
        if (target_obj->in_room != NOWHERE) {
            location = target_obj->in_room;
        } else {
            send_to_char("The object is not available.\n\r"
                         "Try where #.object to nail its room number.\n\r", ch);
            return;
        }
    } else {
        send_to_char("No such creature or object around.\n\r", ch);
        return;
    }

    /*
     * a location has been found.
     */

    if (!real_roomp(location)) {
        Log("Massive error in do_goto. Everyone Off NOW.");
        return;
    }

    if (IS_SET(real_roomp(location)->room_flags, PRIVATE)) {
        for (i = 0, pers = real_roomp(location)->people; pers; pers =
                pers->next_in_room, i++) {
            /*
             * Empty loop
             */
        }
        if (i > 1) {
            if (GetMaxLevel(ch) < 60) {
                send_to_char("There's a private conversation going on in that "
                             "room.\n\r", ch);
                return;
            } 
            
            send_to_char("There is a private converstion but you're a "
                         "level 60\n\r", ch);
        }
    }

    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
        for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
            if ((ch != v) && (CAN_SEE(v, ch))) {
                if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) ||
                    !ch->specials.poofout) {
                    act("$n disappears in a cloud of mushrooms.", FALSE,
                        ch, 0, v, TO_VICT);
                } else {
                    act(ch->specials.poofout, FALSE, ch, 0, v, TO_VICT);
                }
            }
        }
    } else {
        if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) ||
            !ch->specials.poofout) {
            act("$n disappears in a cloud of mushrooms.", FALSE, ch, 0, 0,
                TO_ROOM);
        } else if (*ch->specials.poofout != '!') {
            act(ch->specials.poofout, FALSE, ch, 0, 0, TO_ROOM);
        } else {
            command_interpreter(ch, (ch->specials.poofout + 1));
        }
    }

    if (ch->specials.fighting) {
        stop_fighting(ch);
    }
    char_from_room(ch);
    char_to_room(ch, location);

    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
        for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
            if ((ch != v) && (CAN_SEE(v, ch))) {
                if (!IS_SET(ch->specials.pmask, BIT_POOF_IN)
                    || !ch->specials.poofin) {
                    act("$n appears with an explosion of rose-petals.",
                        FALSE, ch, 0, v, TO_VICT);
                } else {
                    act(ch->specials.poofin, FALSE, ch, 0, v, TO_VICT);
                }
            }
        }
    } else {
        if (!IS_SET(ch->specials.pmask, BIT_POOF_IN) || !ch->specials.poofin) {
            act("$n appears with an explosion of rose-petals.", FALSE, ch,
                0, v, TO_ROOM);
        } else if (*ch->specials.poofin != '!') {
            act(ch->specials.poofin, FALSE, ch, 0, v, TO_ROOM);
        } else {
            command_interpreter(ch, (ch->specials.poofin + 1));
        }
    }
    do_look(ch, NULL, 15);
}

/**
 * @todo split this monstrosity into separate functions for each type of stat
 *       to show
 */
void do_stat(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type *aff;
    char           *arg1;
    char            buf[MAX_STRING_LENGTH];
    char            buf2[MAX_STRING_LENGTH];
    struct room_data *rm = 0;
    struct char_data *k = 0;
    struct obj_data *j = 0;
    struct obj_data *j2 = 0;
    struct extra_descr_data *desc;
    struct follow_type *fol;
    int             i,
                    virtual;
    int             i2,
                    count;
    bool            found;
    char            buff[50];
    char            color1[10],
                    color2[10],
                    color3[10];
    struct time_info_data ma;
    char           *proc;
    struct index_data *index;
    char           *objname;

    dlog("in do_stat");

    if (IS_NPC(ch)) {
        return;
    }
    if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
        strcpy(color1, "$c000p");
        strcpy(color2, "$c000C");
        strcpy(color3, "$c000W");
    } else {
        strcpy(color1, "$c000B");
        strcpy(color2, "$c000w");
        strcpy(color3, "$c000Y");
    }

    argument = get_argument(argument, &arg1);

    /*
     * no argument
     */
    if (!arg1) {
        send_to_char("Stats on who or what?\n\r", ch);
        return;
    }
    
    if (!strcasecmp("room", arg1)) {
        /*
         * stats on room
         */
        rm = real_roomp(ch->in_room);
        oldSendOutput(ch, "Room name: %s, Of zone : %ld. V-Number : %ld, "
                      "R-number : %ld (%d)\n\r",
                      rm->name, rm->zone, rm->number, ch->in_room, rm->special);

        oldSendOutput(ch, "Sector type : %s\n\r", sectors[rm->sector_type].type);

        oldSendOutput(ch, "Special procedure : %s\n\r",
                      ((proc = procGetNameByFunc( rm->funct, PROC_ROOM )) ?
                       proc : "None") );
        sprintbit((unsigned long) rm->room_flags, room_bits, buf);
        oldSendOutput(ch, "Room flags: %s\n\r", buf);

        send_to_char("Description:\n\r", ch);
        send_to_char(rm->description, ch);

        oldSendOutput(ch, "Extra description keywords(s): ");
        if (rm->ex_description) {
            for (desc = rm->ex_description; desc; desc = desc->next) {
                oldSendOutput(ch, "\n\r%s", desc->keyword);
            }
            oldSendOutput(ch, "\n\r");
        } else {
            oldSendOutput(ch, "None");
        }

        oldSendOutput(ch, "\n\r------- Chars present -------\n\r");
        for (k = rm->people; k; k = k->next_in_room) {
            if (CAN_SEE(ch, k)) {
                oldSendOutput(ch, "%s %s\n\r", GET_NAME(k),
                              (!IS_NPC(k) ? "(PC)" :
                               (!IS_MOB(k) ? "(NPC)" : "(MOB)")));
            }
        }

        oldSendOutput(ch, "\n\r--------- Contents ---------\n\r");
        for (j = rm->contents; j; j = j->next_content) {
            oldSendOutput(ch, "%s\n\r", j->name);
        }

        if (rm->tele_time > 0) {
            send_to_char("\n\r--------- Teleport ---------\n\r", ch);
            oldSendOutput(ch, " Teleport to room %d after %d seconds.(%d)\n\r",
                      rm->tele_targ, rm->tele_time / 10, rm->tele_cnt);
        }

        send_to_char("\n\r------- Exits defined -------\n\r", ch);
        for (i = 0; i <= 5; i++) {
            if (rm->dir_option[i]) {
                if (rm->dir_option[i]->keyword) {
                    oldSendOutput(ch, "Direction %s . Keyword : %s\n\r",
                                  direction[i].dir, rm->dir_option[i]->keyword);
                } else {
                    oldSendOutput(ch, "Direction %s \n\r", direction[i].dir);
                }

                oldSendOutput(ch, "Description:\n\r  %s\n\r",
                              (rm->dir_option[i]->general_description ?
                               rm->dir_option[i]->general_description :
                               "UNDEFINED"));

                sprintbit((unsigned) rm->dir_option[i]->exit_info,
                          exit_bits, buf2);
                oldSendOutput(ch, "Exit flag: %s \n\rKey no: %ld\n\rTo room "
                              "(R-Number): %ld\n\r",
                              buf2, rm->dir_option[i]->key,
                              rm->dir_option[i]->to_room);
                if (rm->dir_option[i]->open_cmd != -1) {
                    oldSendOutput(ch, " OpenCommand: %ld\n\r",
                                  rm->dir_option[i]->open_cmd);
                }
                send_to_char("\n\r", ch);
            }
        }
        return;
    }

    count = 1;

    /*
     * mobile in world PERSON!
     */
    if ((k = get_char_vis_world(ch, arg1, &count)) != NULL) {
        switch (k->player.sex) {
        case SEX_NEUTRAL:
            strcpy(buf2, "NEUTRAL");
            break;
        case SEX_MALE:
            strcpy(buf2, "MALE");
            break;
        case SEX_FEMALE:
            strcpy(buf2, "FEMALE");
            break;

        default:
            strcpy(buf2, "INVALID");
            break;
        }
        sprintf(buf, "%s%s%s %s %s- Name : %s%s%s [R-Number %s%ld%s], "
                     "In room [%s%ld%s]",
                color2, buf2, color3,
                (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
                color1, color2, GET_NAME(k), color1, color2, k->nr,
                color1, color2, k->in_room, color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if (IS_MOB(k)) {
            sprintf(buf, "%sV-Number [%s%ld%s]", color1, color2,
                    mob_index[k->nr].vnum, color1);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        sprintf(buf, "%sShort description: %s%s", color1, color2,
                (k->player.short_descr ? k->player.
                 short_descr : "None"));
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sTitle: %s%s", color1, color2,
                (k->player.title ? k->player.title : "None"));
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sLong description: %s%s", color1, color2,
                k->player.long_descr ? k->player.long_descr : "None");
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if (IS_NPC(k)) {
            sprinttype(k->player.class, npc_class_types, buf2);
            sprintf(buf, "%sMonster Class: %s%s", color1, color2, buf2);
        } else {
            sprintclasses((unsigned) k->player.class, buf2);
            sprintf(buf, "%sClass: %s%s", color1, color2, buf2);
        }
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sLevel [%s%d%s/%s%d%s/%s%d%s/%s%d%s/%s%d%s/%s%d%s/"
                     "%s%d%s/%s%d%s/%s%d%s/%s%d%s/%s%d%s/%s%d%s] "
                     "Alignment[%s%d%s]",
                color1, color2, k->player.level[0], color1, color3,
                k->player.level[1], color1, color2, k->player.level[2],
                color1, color3, k->player.level[3], color1, color2,
                k->player.level[4], color1, color3, k->player.level[5],
                color1, color2, k->player.level[6], color1, color3,
                k->player.level[7], color1, color2, k->player.level[8],
                color1, color3, k->player.level[9], color1, color2,
                k->player.level[10], color1, color3,
                k->player.level[11], color1, color3, GET_ALIGNMENT(k),
                color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if( IS_PC(k) ) {
            oldSendOutput(ch, "%sRemort Class: %s%s%s.\n\r", color1, color2,
                      classes[k->specials.remortclass - 1].name, color1);
        }

        sprintf(buf, "%sBirth : [%s%ld%s]secs, Logon[%s%ld%s]secs, "
                     "Played[%s%d%s]secs",
                color1, color2, (long)k->player.time.birth, color1, color2,
                (long)k->player.time.logon, color1, color2,
                k->player.time.played, color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        age2(k, &ma);
        sprintf(buf, "%sAge: [%s%d%s] Y, [%s%d%s] M, [%s%d%s] D, [%s%d%s] "
                     "H.",
                color1, color2, ma.year, color1, color2, ma.month,
                color1, color2, ma.day, color1, color2, ma.hours,
                color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sHeight [%s%d%s]cm, Wgt [%s%d%s]pounds "
                     "NumAtks[%s%.1f%s+(%s%.2f%s)]",
                color1, color2, GET_HEIGHT(k), color1, color2,
                GET_WEIGHT(k), color1, color2, k->mult_att, color1,
                color2,
                (ch->equipment[WIELD] ?
                 (float) ch->equipment[WIELD]->speed / 100 : 0), color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sStr:[%s%d%s/%s%d%s] Int:[%s%d%s] Ws:[%s%d%s] "
                     "Dex:[%s%d%s] Con:[%s%d%s] Ch:[%s%d%s]",
                color1, color3, GET_STR(k), color1, color3, GET_ADD(k),
                color1, color3, GET_INT(k), color1, color3, GET_WIS(k),
                color1, color3, GET_DEX(k), color1, color3, GET_CON(k),
                color1, color3, GET_CHR(k), color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sMana:[%s%d%s/%s%d%s+%s%d%s] "
                     "Hit:[%s%d%s/%s%d%s+%s%d%s] "
                     "Move:[%s%d%s/%s%d%s+%s%d%s]",
                color1, color2, GET_MANA(k), color1, color2,
                mana_limit(k), color1, color3, mana_gain(k), color1,
                color2, GET_HIT(k), color1, color2, hit_limit(k),
                color1, color3, hit_gain(k), color1, color2,
                GET_MOVE(k), color1, color2, move_limit(k), color1,
                color3, move_gain(k), color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buff, "%s", formatNum(GET_EXP(k)));
        sprintf(buf, "%sAC:[%s%d%s/%s10%s], Coins: [%s%s%s], "
                     "Exp: [%s%s%s], Hitroll: [%s%d%s+(%s%d%s)], "
                     "Damroll: [%s%d%s+(%s%d%s)] sf[%s%d%s]",
                color1, color2, GET_AC(k), color1, color2, color1,
                color2, formatNum(GET_GOLD(k)), color1, color2, buff,
                color1, color2, k->points.hitroll, color1, color3,
                str_app[STRENGTH_APPLY_INDEX(k)].tohit, color1, color2,
                k->points.damroll, color1, color3,
                str_app[STRENGTH_APPLY_INDEX(k)].todam, color1, color2,
                k->specials.spellfail, color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if (!IS_NPC(k)) {
            sprintf(buf, "%sLeadership Exp: [%s%s%s],  Clan[$c0007%s%s]",
                    color1, color2, formatNum(GET_LEADERSHIP_EXP(k)),
                    color1, clan_list[GET_CLAN(k)].name, color1);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        sprinttype(GET_POS(k), position_types, buf2);
        sprintf(buf, "%sPosition: %s%s%s, Fighting: %s%s", color1,
                color2, buf2, color1, color2,
                ((k->specials.fighting) ?
                 GET_NAME(k->specials.fighting) : "Nobody"));
        if (k->desc) {
            sprintf(buf2, ", %sConnected: %s", color1, color2);
            strcat(buf, buf2);
            sprinttype(k->desc->connected, connected_types, buf2);
            strcat(buf, buf2);
        }
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sDefault position: %s", color1, color2);
        sprinttype((k->specials.default_pos), position_types, buf2);
        strcat(buf, buf2);
        if (IS_NPC(k)) {
            sprintf(buf2, "%s, NPC Action flags: %s", color1, color2);
            strcat(buf, buf2);
            sprintbit(k->specials.act, action_bits, buf2);
        } else {
            sprintf(buf2, "%s, PC flags: %s", color1, color2);
            strcat(buf, buf2);
            sprintbit(k->specials.act, player_bits, buf2);
        }
        strcat(buf, buf2);
        sprintf(buf2, "%s, Timer [%s%d%s]", color1, color2,
                k->specials.timer, color1);
        strcat(buf, buf2);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if (IS_NPC(k)) {
            sprintf(buf, "%sMobile common procedure:  %s%s%s.", color1,
                    color2, procedure_bits[k->specials.proc], color1);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        if (IS_MOB(k)) {
            sprintf(buf, "%sMobile special procedure: %s", color1, color2);
            if( (proc = procGetNameByFunc( mob_index[k->nr].func,
                                           PROC_MOBILE )) ) {
                strcat( buf, proc );
                strcat( buf, "\n\r" );
            } else {
                strcat( buf, "None\n\r" );
            }
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        if (IS_NPC(k)) {
            sprintf(buf, "%sNPC Bare Hand Damage %s%d%sd%s%d%s.",
                    color1, color3, k->specials.damnodice, color2,
                    color3, k->specials.damsizedice, color1);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        sprintf(buf, "%sCarried weight: %s%d%s   Carried items: %s%d ",
                color1, color2, IS_CARRYING_W(k),
                color1, color2, IS_CARRYING_N(k));
        for (i = 0, i2 = 0; i < MAX_WEAR; i++) {
            if (k->equipment[i]) {
                i2++;
            }
        }
        sprintf(buf2, "%sItems in equipment: %s%d", color1, color2, i2);
        strcat(buf, buf2);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sApply saving throws: [%s%d%s] [%s%d%s] [%s%d%s] "
                     "[%s%d%s] [%s%d%s]",
                color1, color2, k->specials.apply_saving_throw[0],
                color1, color2, k->specials.apply_saving_throw[1],
                color1, color2, k->specials.apply_saving_throw[2],
                color1, color2, k->specials.apply_saving_throw[3],
                color1, color2, k->specials.apply_saving_throw[4],
                color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sThirst: %s%d%s, Hunger: %s%d%s, Drunk: %s%d",
                color1, color2, k->specials.conditions[THIRST],
                color1, color2, k->specials.conditions[FULL],
                color1, color2, k->specials.conditions[DRUNK]);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sMaster is '%s%s%s'", color1, color2,
                ((k->master) ? GET_NAME(k->master) : "none"), color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        sprintf(buf, "%sFollowers are:", color1);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
        if( k->followers ) {
            for (fol = k->followers; fol; fol = fol->next) {
                sprintf(buf, "%s    %s", color2,
                                         fol->follower->player.name);
                act(buf, FALSE, ch, 0, 0, TO_CHAR);
            }
        } else {
            sprintf(buf, "%s  None", color2 );
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * Hates
         */
        sprintf(buf, "%sHates:  %s", color1, color2);
        ShowHates(k, buf);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        /*
         * Fears
         */
        sprintf(buf, "%sFears:  %s", color1, color2);
        ShowFears(k, buf);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        /*
         * immunities
         */
        if (k->M_immune) {
            sprintf(buf, "%sImmune to: %s", color1, color2);
            sprintbit(k->M_immune, immunity_names, buf2);
            strcat(buf, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * resistances
         */
        if (k->immune) {
            sprintf(buf, "%sResistant to: %s", color1, color2);
            sprintbit(k->immune, immunity_names, buf2);
            strcat(buf, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * Susceptible
         */
        if (k->susc) {
            sprintf(buf, "%sSusceptible to: %s", color1, color2);
            sprintbit(k->susc, immunity_names, buf2);
            strcat(buf, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        if (k->player.user_flags) {
            sprintf(buf, "%sSpecial Flags: %s", color1, color2);
            sprintbit(k->player.user_flags, special_user_flags, buf2);
            strcat(buf, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * race, action pointer
         */
        sprintf(buf, "%sRace: %s%s%s  Generic pointer: %s%d", 
                     color1, color3, races[k->race].racename,
                     color1, color2, (int) k->generic);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        /*
         * language
         */
         if (!k->player.speaks) {
             k->player.speaks = SPEAK_COMMON;
         }
         sprintf(buf, "%sCurrently Speaking: %s%s%s", color1, color2,
                 languages[k->player.speaks-1].name, color1);
         act(buf, FALSE, ch, 0, 0, TO_CHAR);


        /*
         * Showing the bitvector
         */
        sprintf(buf, "%sA1: %s%ld    %sA2: %s%ld", color1, color2,
                k->specials.affected_by, color1, color2,
                k->specials.affected_by2);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);

        if (k->specials.affected_by) {
            sprintbit((unsigned) k->specials.affected_by,
                      affected_bits, buf2);
            sprintf(buf, "%sAffected by : %s%s", color1, color2, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }
        if (k->specials.affected_by2) {
            sprintbit((unsigned) k->specials.affected_by2,
                      affected_bits2, buf2);
            sprintf(buf, "%sAffected by2: %s%s", color1, color2, buf2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * Routine to show what spells a char is affected by
         */
        if (k->affected) {
            sprintf(buf, "\n\r%sAffecting Spells:\n\r"
                         "%s---------------------------------",
                    color1, color2);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);

            for (aff = k->affected; aff; aff = aff->next) {
                if (aff->type <= MAX_EXIST_SPELL) {
                    sprintf(buf, "%sSpell : '%s%s%s (spell number %s%d%s)'",
                            color1, color2, spells[aff->type - 1],
                            color1, color2, aff->type, color1);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                    
                    if (aff->location == APPLY_IMMUNE) {
                        sprintf(buf, "     %sModifies %s%s%s by %s",
                            color1, color2, apply_types[aff->location], 
                            color1, color2); 
                        sprintbit(aff->modifier, immunity_names, buf2);
                        strcat(buf, buf2);
                        sprintf(buf2, "%s", color1);
                        strcat(buf, buf2);
                        act(buf, FALSE, ch, 0, 0, TO_CHAR);
                    } else {
                        sprintf(buf, "     %sModifies %s%s%s by %s%ld%s "
                                     "points", color1, color2, 
                                apply_types[aff->location], color1, color2,
                                aff->modifier, color1); 
                        act(buf, FALSE, ch, 0, 0, TO_CHAR);
                    }
                    if (aff->location == APPLY_BV2 ||
                        aff->location == APPLY_SPELL2) {
                        sprintbit((unsigned) aff->bitvector,
                                  affected_bits2, buf2);
                    } else {
                        sprintbit((unsigned) aff->bitvector,
                                  affected_bits, buf2);
                    }
                    sprintf(buf, "     %sExpires in %s%3d%s hours, Bits "
                                 "set %s%s",
                            color1, color2, aff->duration, color1,
                            color2, buf2);
                    act(buf, FALSE, ch, 0, 0, TO_CHAR);
                } else {
                    /*
                     * log bogus aff->type
                     */
                    Log("<%s> had a bogus aff->type act.wizard, do_stat",
                        GET_NAME(k));
                }
            }
        }
        return;
    }

    /*
     * stat on object
     */
    if ((j = get_obj_vis_world(ch, arg1, &count)) != NULL) {
        virtual = MAX( 0, j->item_number );
        objname = KeywordsToString( &j->keywords, " " );
        sprintf(buf, "Object name: [%s]\n\r R-number: [%d], "
                     "V-number: [%d] Item type: ",
                objname, j->item_number, virtual);
        free( objname );
        sprinttype(ITEM_TYPE(j), item_types, buf2);

        strcat(buf, buf2);

        if (IS_WEAPON(j)) {
            sprintf(buf2, ", Weapon Speed: %s(%.2f)",
                    SpeedDesc(j->speed), (float) j->speed / 100);
            strcat(buf, buf2);
        }

        strcat(buf, "\n\r");
        send_to_char(buf, ch);
        oldSendOutput(ch, "Tweak rate: %d%s\n\r", j->tweak, "%");
        oldSendOutput(ch, "Short description: %s\n\rLong description:\n\r%s\n\r",
                  ((j->short_description) ? j->short_description : "None"),
                  ((j->description) ? j->description : "None"));

        if (j->ex_description && j->ex_description_count) {
            strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
            for (desc = j->ex_description, i = 0; 
                 i < j->ex_description_count; desc++) {
                objname = KeywordsToString( desc, " " );
                strcat(buf, objname);
                free(objname);
                strcat(buf, "\n\r");
            }
            strcat(buf, "----------\n\r");
            send_to_char(buf, ch);
        } else {
            oldSendOutput(ch, "Extra description keyword(s): None\n\r");
        }

        sprintbit((unsigned) j->wear_flags, wear_bits, buf);
        oldSendOutput(ch, "Can be worn on : %s\n\r", buf);

        sprintbit((unsigned) j->bitvector, affected_bits, buf);
        oldSendOutput(ch, "Set char bits  : %s\n\r", buf);

        sprintbit((unsigned) j->extra_flags, extra_bits, buf);
        oldSendOutput(ch, "Extra flags: %s\n\r", buf);

        oldSendOutput(ch, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
                      j->weight, j->cost,
                      j->cost_per_day, j->timer);

        if (j->level == 0) {
            oldSendOutput(ch, "Ego: None, ");
        } else {
            oldSendOutput(ch, "Ego: Level %d, ", j->level);
        }

        if (j->max == 0) {
            oldSendOutput(ch, "Loadrate: None, ");
        } else {
            oldSendOutput(ch, "Loadrate: Level %d, ", j->max);
        }

        oldSendOutput(ch, "Last modified by %s on %s",
                      j->modBy, asctime(localtime(&j->modified)));

        strcpy(buf, "In room: ");
        if (j->in_room == NOWHERE) {
            strcat(buf, "Nowhere");
        } else {
            sprintf(buf2, "%d", j->in_room);
            strcat(buf, buf2);
        }
        strcat(buf, ", In object: ");
        strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));

        switch (j->type_flag) {
        case ITEM_TYPE_LIGHT:
            sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
                    j->value[0], j->value[1],
                    j->value[2]);
            break;
        case ITEM_TYPE_SCROLL:
        case ITEM_TYPE_POTION:
            sprintf(buf, "Spells (level %d): %d, %d, %d",
                    j->value[0], j->value[1],
                    j->value[2], j->value[3]);
            break;
        case ITEM_TYPE_WAND:
        case ITEM_TYPE_STAFF:
            sprintf(buf, "Level: %d Spell : %d\n\rCharges : %d",
                    j->value[0], j->value[3],
                    j->value[2]);
            break;
        case ITEM_TYPE_WEAPON:
            sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
                    j->value[0], j->value[1],
                    j->value[2], j->value[3]);
            break;
        case ITEM_TYPE_FIREWEAPON:
            sprintf(buf, "Tohit: %d\n\rDam: %d\n\rType: %d Min Weight: %d\n\r",
                    j->value[2], j->value[3],
                    j->value[0], j->value[1]);
            break;
        case ITEM_TYPE_MISSILE:
            sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
                    j->value[0], j->value[1],
                    j->value[3]);
            break;
        case ITEM_TYPE_ARMOR:
            sprintf(buf, "AC-apply : [%d]\n\rFull Strength : [%d]",
                    j->value[0], j->value[1]);

            break;
        case ITEM_TYPE_TRAP:
            sprintf(buf,
                    "Eff type: %d, Dam type: %d, level: %d, charges: %d",
                    j->value[0], j->value[1],
                    j->value[2], j->value[3]);
            break;
        case ITEM_TYPE_CONTAINER:
            sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\r"
                         "Key to unlock: %d\n\rCorpse : %s\n\r",
                    j->value[0], j->value[1],
                    j->value[2],
                    (j->value[3] ? "Yes" : "No"));
            break;
        case ITEM_TYPE_DRINKCON:
            sprinttype(j->value[2], drinks, buf2);
            sprintf(buf, "Max-contains : %d\n\rContains : %d\n\r"
                         "Poisoned : %d\n\rLiquid : %s",
                    j->value[0], j->value[1],
                    j->value[3], buf2);
            break;
        case ITEM_TYPE_NOTE:
            sprintf(buf, "Tongue : %d", j->value[0]);
            break;
        case ITEM_TYPE_KEY:
            sprintf(buf, "Keytype : %d", j->value[0]);
            break;
        case ITEM_TYPE_FOOD:
            sprintf(buf, "Makes full : %d\n\rPoisoned : %d",
                    j->value[0], j->value[3]);
            break;
        case ITEM_TYPE_PORTAL:
            sprintf(buf, "Portals to room : %d", j->value[0]);
            break;
        case ITEM_TYPE_AUDIO:
            sprintf(buf, "Sound : %s\n\r", (j->action_description ?
                                            j->action_description :
                                            "None"));
            break;
        case ITEM_TYPE_INSTRUMENT:
            sprintf(buf, "Mana reduction : %d\n\r",
                    (j->value[0]));
            break;
        default:
            sprintf(buf, "Values 0-3 : [%d] [%d] [%d] [%d]",
                    j->value[0], j->value[1],
                    j->value[2], j->value[3]);
            break;
        }
        send_to_char(buf, ch);

        strcpy(buf, "\n\rEquipment Status: ");
        if (!j->carried_by) {
            strcat(buf, "NONE");
        } else {
            found = FALSE;
            for (i = 0; i < MAX_WEAR; i++) {
                if (j->carried_by->equipment[i] == j) {
                    sprinttype(i, equipment_types, buf2);
                    strcat(buf, buf2);
                    found = TRUE;
                }
            }
            if (!found) {
                strcat(buf, "Inventory");
            }
        }
        send_to_char(buf, ch);

        strcpy(buf, "\n\rSpecial procedure : ");
        if (j->item_number >= 0 && 
            (proc = procGetNameByFunc( j->index->func, PROC_OBJECT )) ) {
            strcat(buf, proc);
            strcat(buf, "\n\r");
        } else {
            strcat(buf, "None\n\r");
        }
        send_to_char(buf, ch);

        strcpy(buf, "Contains :\n\r");
        found = FALSE;
        for (j2 = j->contains; j2; j2 = j2->next_content) {
            strcat(buf, fname(j2->name));
            strcat(buf, "\n\r");
            found == TRUE;
        }
        if (!found) {
            strcpy(buf, "Contains : Nothing\n\r");
        }
        send_to_char(buf, ch);

        send_to_char("Can affect char :\n\r", ch);
        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            sprinttype(j->affected[i].location, apply_types, buf2);
            oldSendOutput(ch, "    Affects : %s By ", buf2);

            switch (j->affected[i].location) {
            case APPLY_M_IMMUNE:
            case APPLY_IMMUNE:
            case APPLY_SUSC:
                sprintbit(j->affected[i].modifier, immunity_names, buf2);
                strcat(buf2, "\n\r");
                break;
            case APPLY_ATTACKS:
                sprintf(buf2, "%f\n\r",
                        (float) j->affected[i].modifier / 10);
                break;
            case APPLY_WEAPON_SPELL:
            case APPLY_EAT_SPELL:
                sprintf(buf2, "%s\n\r",
                        spells[j->affected[i].modifier - 1]);
                break;
            case APPLY_SPELL:
                sprintbit(j->affected[i].modifier, affected_bits, buf2);
                strcat(buf2, "\n\r");
                break;
            case APPLY_BV2:
            case APPLY_SPELL2:
                sprintbit(j->affected[i].modifier, affected_bits2, buf2);
                strcat(buf2, "\n\r");
                break;
            case APPLY_RACE_SLAYER:
                sprintf(buf2, "%s\n\r", 
                              races[j->affected[i].modifier].racename);
                break;
            case APPLY_ALIGN_SLAYER:
                if (j->affected[i].modifier > 1)
                    sprintf(buf2, "SLAY GOOD\n\r");
                else if (j->affected[i].modifier == 1)
                    sprintf(buf2, "SLAY NEUTRAL\n\r");
                else
                    sprintf(buf2, "SLAY EVIL\n\r");
                break;

            default:
                sprintf(buf2, "%ld\n\r", j->affected[i].modifier);
                break;
            }
            send_to_char(buf2, ch);
        }
    } else {
        send_to_char("No mobile or object by that name in the world\n\r", ch);
    }
}

void do_ooedit(struct char_data *ch, char *argument, int cmd)
{
    char           *item,
                   *field,
                   *parmstr,
                   *parmstr2;
    struct obj_data *j = 0;
    int             virtual;

    dlog("in do_ooedit");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &item);

    if (!item) {
        send_to_char("Oedit what?!? (oedit <item> <field> <num>) Use "
                     "'ooedit help'.\n\r", ch);
        return;
    }

    if (!strcmp(item, "help")) {
        send_to_char("Help for Oedit. Command line Parameters OEDIT <NAME> "
                     "<FIELD> <VALUE>\n\r"
                     "List of Fields :\n\r"
                     "ldesc  = Long Item description | sdesc  = Short "
                     "description\n\r"
                     "extra  = Extra descriptions*NI*| name   = Item name\n\r"
                     "wflags = wear flags            | afflags= affect "
                     "flags\n\r"
                     "exflags= extra flags           | weight = item weight\n\r"
                     "cost   = item cost to rent per day\n\r"
                     "value  = Item value if sold    | timer  = item timer\n\r"
                     "type   = item type\n\r"
                     "v0     = value[0] of item      | v1     = value[1] of "
                     "item\n\r", ch );
        send_to_char("v2     = value[2] of item      | v3     = value[3] of "
                     "item\n\r"
                     "aff1   = special affect 1 (requires another value, oedit"
                     " aff1 <modifer> <type>)\n\r"
                     "aff2   = special affect 2      | aff3   = special affect"
                     " 3\n\r"
                     "aff4   = special affect 4      | aff5   = special affect"
                     " 5\n\r"
                     "speed  = speed of weapon       | ego    = level of item"
                     "\n\r"
                     "max    = max of item           | tweak  = tweak rate\n\r"
                     "\n\rNote: NI = Not implemented.\n\r", ch);
        return;
    }
    argument = get_argument(argument, &field);

    if (!field) {
        send_to_char("Oedit what?!? I need a FIELD! (oedit <item> <field> "
                     "<num>)\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Oedit what?!? I need a <num/change>!(oedit <item> "
                     "<field> <num>)\n\r", ch);
        return;
    }

    /*
     * object
     */

    if ((j = (struct obj_data *) get_obj_in_list_vis(ch, item, ch->carrying))) {
        virtual = MAX( 0, j->item_number );

        if (!strcmp(field, "name")) {
            FreeKeywords( &j->keywords, FALSE );
            StringToKeywords( argument, &j->keywords );
            return;
        }

        if (!strcmp(field, "ldesc")) {
            if (j->description) {
                free(j->description);
            }
            j->description = strdup(argument);
            return;
        }

        if (!strcmp(field, "sdesc")) {
            if (j->short_description) {
                free(j->short_description);
            }
            j->short_description = strdup(argument);
            return;
        }

        if (!strcmp(field, "extra")) {
            send_to_char("Not able modify extra descriptions (yet).\n\r", ch);
            return;
        }

        argument = get_argument(argument, &parmstr);
        if( !parmstr ) {
            send_to_char("Not enough arguments.\n\r", ch);
        }

        if (!strcmp(field, "wflags")) {
            j->wear_flags = atol(parmstr);
            return;
        }

        if (!strcmp(field, "afflags")) {
            j->bitvector = atol(parmstr);
            return;
        }

        if (!strcmp(field, "exflags")) {
            j->extra_flags = atol(parmstr);
            return;
        }

        if (!strcmp(field, "weight")) {
            j->weight = atol(parmstr);
            return;
        }

        if (!strcmp(field, "cost")) {
            j->cost_per_day = atol(parmstr);
            return;
        }

        if (!strcmp(field, "value")) {
            j->cost = atol(parmstr);
            return;
        }

        if (!strcmp(field, "ego")) {
            j->level = atol(parmstr);
            return;
        }

        if (!strcmp(field, "speed")) {
            j->speed = atol(parmstr);
            return;
        }

        if (!strcmp(field, "max")) {
            j->max = atol(parmstr);
            return;
        }

        if (!strcmp(field, "tweak")) {
            j->tweak = atol(parmstr);
            return;
        }

        if (!strcmp(field, "rent")) {
            j->cost_per_day = atol(parmstr);
            return;
        }

        if (!strcmp(field, "timer")) {
            j->timer = atol(parmstr);
            return;
        }

        if (!strcmp(field, "type")) {
            j->type_flag = atol(parmstr);
            return;
        }

        if (!strcmp(field, "v0")) {
            j->value[0] = atol(parmstr);
            return;
        }

        if (!strcmp(field, "v1")) {
            j->value[1] = atol(parmstr);
            return;
        }

        if (!strcmp(field, "v2")) {
            j->value[2] = atol(parmstr);
            return;
        }

        if (!strcmp(field, "v3")) {
            j->value[3] = atol(parmstr);
            return;
        }


        argument = get_argument(argument, &parmstr2);
        if( !parmstr2 ) {
            send_to_char("Not enough arguments\n\r", ch);
            return;
        }

        if (!strcmp(field, "aff1")) {
            j->affected[0].location = atol(parmstr2);
            j->affected[0].modifier = atol(parmstr);
            return;
        }

        if (!strcmp(field, "aff2")) {
            j->affected[1].location = atol(parmstr2);
            j->affected[1].modifier = atol(parmstr);
            return;
        }

        if (!strcmp(field, "aff3")) {
            j->affected[2].location = atol(parmstr2);
            j->affected[2].modifier = atol(parmstr);
            return;
        }

        if (!strcmp(field, "aff4")) {
            j->affected[3].location = atol(parmstr2);
            j->affected[3].modifier = atol(parmstr);
            return;
        }

        if (!strcmp(field, "aff5")) {
            j->affected[4].location = atol(parmstr2);
            j->affected[4].modifier = atol(parmstr);
            return;
        }
    } else {
        send_to_char("You do not have that object.\n\r", ch);
    }
}

void do_set(struct char_data *ch, char *argument, int cmd)
{
    char           *field,
                   *name,
                   *parmstr,
                   *parmstr2;
    struct char_data *mob = NULL;
    int             parm = 0,
                    parm2 = 0;
    extern char     PeacefulWorks;
    extern char     EasySummon;

    dlog("in do_set");

    if ((GetMaxLevel(ch) < SILLYLORD) || (IS_NPC(ch))) {
        return;
    }
    argument = get_argument(argument, &field);
    argument = get_argument(argument, &name);
    argument = get_argument(argument, &parmstr);
    argument = get_argument(argument, &parmstr2);

    if( !field ) {
        send_to_char("Pardon?\n\r", ch);
        return;
    }

    if (!strcmp(field, "help")) {
        send_to_char("@"
                     "\n\r"
                     "Usage :@ <field> <user name> <value>\n\r"
                     "\n\r"
                     "align - Sets Alignment\n\r"
                     "class - Sets Class\n\r"
                     "    1 = Mage       2 = Cleric      4 = Warrior      8"
                     " = Thief\n\r"
                     "   16 = Druid     32 = Monk       64 = Barbarian  128"
                     " = Sorceror\n\r"
                     "  256 = Paladin  512 = Ranger   1024 = Psionist  2048"
                     " = Necromancer\n\r"
                     "For Multi-class characters add the numbers of the "
                     "required classes together\n\r"
                     "ie: Mu/Cl/Wa would be 1 + 2 + 4 = 7.\n\r"
                     "\n\r", ch );
        send_to_char("clan - Clan number\n\r"
                     "exp - Total Experience\n\r"
                     "lev - Level (only sets Mage level, use advance for "
                     "other classes)\n\r"
                     "sex - Sex  0 = Neutral   1 = Male   2 = Female\n\r"
                     "race - Race of character or mob.  Use 'help allrace' "
                     "for listing\n\r"
                     "hunger - Hunger level of PC\n\r"
                     "thirst - Thirst level of PC\n\r"
                     "zone - Gives editing access to a zone\n\r"
                     "hit - Current Hitpoints\n\r"
                     "mhit - Max Hitpoints\n\r"
                     "tohit - To hit modifier\n\r"
                     "todam - Damange modifier\n\r", ch );
        send_to_char("ac - Armor Class of PC\n\r"
                     "bank - Amount of coins in bank\n\r"
                     "gold - Amount of coins on PC\n\r"
                     "age - Age of PC.  Postive numbers will add to age.  "
                     "Negative will subtract.\n\r"
                     "prac - Number of Practices\n\r"
                     "str - Strength of PC\n\r"
                     "add - Strength Modifier (ie 18/75 using this one could "
                     "change the 75)\n\r"
                     "saves - Saving throws (doesn't work)\n\r"
                     "skills - how learned a skill is. @ skills <target> "
                     "<skill number> <level> - See allspells for skill and "
                     "spell list.\n\r", ch );
        send_to_char("stadd - Strength Modifier (ie 18/75 using this one "
                     "could change the 75)\n\r"
                     "int - Intelligence\n\r"
                     "wis - Wisdom\n\r"
                     "dex - Dexterity\n\r"
                     "con - Constitution\n\r"
                     "chr - Charisma\n\r"
                     "pkill - Enable and Disable Player killing\n\r"
                     "mana - Set current and max Mana\n\r"
                     "start - Set starting room\n\r"
                     "murder - Flag PC as murderer\n\r"
                     "stole - Flag PC as thief\n\r"
                     "known - Make spell or skill known to PC.  @ known "
                     "<target> <skill number> - See allspells for listing.\n\r"
                     "nodelete - Set NODELETE flag on PC.\n\r"
                     "specflags - Does nothing.\n\r", ch );
        send_to_char("racewar - Flag PC as part of race wars\n\r"
                     "numatks - Number of attacks PC has.\n\r"
                     "objedit - Enable or Disable an Immortals ability to "
                     "edit objects.\n\r"
                     "mobedit - Enable or disable an Immortals ability to "
                     "edit mobiles.\n\r"
                     "remaffect - Remove all affects from target.\n\r"
                     "wizreport - Set wizreport flag on target.\n\r"
                     "nofly - Flag target as not being able to fly.\n\r"
                     "wingsburn - Flag target as not being able to fly.  "
                     "Burns wings of winged mobiles/PCs.\n\r"
                     "move - Set current Movement.\n\r"
                     "mmove - Set Max Movement.\n\r", ch );
        send_to_char("mkills - Set number of mobs killed.\n\r"
                     "mdeaths - Set number of deaths by mobiles.\n\r"
                     "akills - Set number of arena kills.\n\r"
                     "adeaths - Set number of arena deaths.\n\r"
                     "remortclass - set the class the char is remorting "
                     "into.\n\r"
                     "\n\r"
                     "Remember, be careful how you use this command!\n\r", ch);
        return;
    }

    if (!name || !(mob = get_char_vis(ch, name))) {
        send_to_char("@\n\r"
                     "Usage :@ <field> <user name> <value>\n\r"
                     "\n\r"
                     "This is a Implementor command and should be used with "
                     "care as it can\n\r"
                     "change any ability/skill/attr of a character. Here is a"
                     " list of fields,\n\r"
                     "the value types will differ with each (i.e. number/"
                     "alpha char)\n\r"
                     "For more help type '@ help'\n\r"
                     "\n\r", ch);
        send_to_char("align class exp lev sex race hunger thirst zone hit mhit"
                     " tohit todam\n\r"
                     "ac bank gold age prac str add saves skills stadd int wis"
                     " dex con chr\n\r"
                     "pkill mana start murder stole known zone nodelete "
                     "specflags racewar \n\r"
                     "numatks objedit mobedit remaffect wizreport nofly "
                     "wingsburn move mmove\n\r"
                     "Remember, be careful how you use this command!\n\r", ch);
        return;
    }

    /* These require no arguments */
    if (!strcmp(field, "wizreport")) {
        if (GetMaxLevel(ch) < GetMaxLevel(mob)) {
            send_to_char("I don't think so.\n\r", ch);
            oldSendOutput(mob, "%s tried to set your wizreport flag!\n\r",
                           GET_NAME(ch));
        } else if (IS_SET(mob->specials.act, PLR_WIZREPORT) && IS_PC(mob)) {
            REMOVE_BIT(mob->specials.act, PLR_WIZREPORT);
            send_to_char("Wizreport flag removed.\n\r", ch);
            if (ch != mob) {
                send_to_char("You can no longer wizreport!\n\r", mob);
            }
        } else {
            SET_BIT(mob->specials.act, PLR_WIZREPORT);
            send_to_char("Wizreport flag set!\n\r", ch);
            if (mob != ch) {
                send_to_char("You now have the power to wizreport!\n\r", mob);
            }
        }
    } else if (!strcmp(field, "wingburn")) {
        BurnWings(mob);
        send_to_char("Wings Burned to a crisp\n\r", ch);
    } else if (!strcmp(field, "nofly")) {
        if (IS_SET(mob->specials.act, PLR_NOFLY)) {
            REMOVE_BIT(mob->specials.act, PLR_NOFLY);
            send_to_char("NO-FLY flag removed.\n\r", ch);
        } else {
            SET_BIT(mob->specials.act, PLR_NOFLY);
            send_to_char("NO-FLY flag set\n\r", ch);
        }
    } else if (!strcmp(field, "remaffect")) {
        mob->affected = NULL;
        mob->specials.affected_by = 0;
        mob->specials.affected_by2 = 0;
        send_to_char("All affects removed from char!\n\r", ch);
    } else if (!strcmp(field, "clanleader")) {
        if (!IS_SET(mob->specials.act, PLR_CLAN_LEADER)) {
            SET_BIT(mob->specials.act, PLR_CLAN_LEADER);
            send_to_char("Setting clan leader flag.\n\r", ch);
        } else {
            REMOVE_BIT(mob->specials.act, PLR_CLAN_LEADER);
            send_to_char("Removing clan leader flag\n\r", ch);
        }
    } else if (!strcmp(field, "legend")) {
        if (!IS_SET(mob->specials.act, PLR_LEGEND)) {
            SET_BIT(mob->specials.act, PLR_LEGEND);
            send_to_char("Setting legend flag.\n\r", ch);
        } else {
            REMOVE_BIT(mob->specials.act, PLR_LEGEND);
            send_to_char("Removing legend flag.\n\r", ch);
        }
    } else if (!strcmp(field, "nodelete")) {
        if (IS_SET(mob->player.user_flags, NO_DELETE)) {
            send_to_char("Setting no delete flag OFF.\n\r", ch);
            send_to_char("Your no delete flag has been removed.\n\r", mob);
            REMOVE_BIT(mob->player.user_flags, NO_DELETE);
        } else {
            send_to_char("Setting no delete flag ON.\n\r", ch);
            send_to_char("You are now flagged as NO DELETE.\n\r", mob);
            SET_BIT(mob->player.user_flags, NO_DELETE);
        }
#if 0
    } else if (!strcmp(field, "dimd")) {
        if (IS_SET(mob->specials.act, PLR_NODIMD)) {
            send_to_char("Setting DIMD OFF.\n\r", ch);
            send_to_char("Your DIMD privilages have been removed.\n\r", mob);
            REMOVE_BIT(mob->specials.act, PLR_NODIMD);
        } else {
            send_to_char("Setting DIMD flag ON.\n\r", ch);
            send_to_char("You can now use DIMD.\n\r", mob);
            SET_BIT(mob->specials.act, PLR_NODIMD);
        }
#endif
    } else if (!strcmp(field, "murder")) {
        if (GetMaxLevel(ch) < GetMaxLevel(mob)) {
            send_to_char("I don't think so.\n\r", ch);
            oldSendOutput(mob, "%s tried to set your murder flag!\n\r",
                           GET_NAME(ch));
        } else if (IS_SET(mob->player.user_flags, MURDER_1) && IS_PC(mob)) {
            REMOVE_BIT(mob->player.user_flags, MURDER_1);
            send_to_char("Murder flag removed.\n\r", ch);
            if (ch != mob)
                send_to_char("You have been pardoned for murder!\n\r", mob);
        } else {
            SET_BIT(mob->player.user_flags, MURDER_1);
            send_to_char("Murder flag set!\n\r", ch);
            if (mob != ch) {
                send_to_char("You have been accused of MURDER!\n\r", mob);
            }
        }
    } else if (!strcmp(field, "objedit")) {
        if (IS_SET(mob->player.user_flags, CAN_OBJ_EDIT) && IS_PC(mob)) {
            REMOVE_BIT(mob->player.user_flags, CAN_OBJ_EDIT);
            if (mob != ch) {
                send_to_char("You can no longer edit objects.\n\r", mob);
            }
            send_to_char("Object edit flag removed.\n\r", ch);
        } else {
            SET_BIT(mob->player.user_flags, CAN_OBJ_EDIT);
            if (ch != mob) {
                send_to_char("You can now edit objects.\n\r", mob);
            }
            send_to_char("Object edit flag set.\n\r", ch);
        }
    } else if (!strcmp(field, "mobedit")) {
        if (IS_SET(mob->player.user_flags, CAN_MOB_EDIT) && IS_PC(mob)) {
            REMOVE_BIT(mob->player.user_flags, CAN_MOB_EDIT);
            if (mob != ch) {
                send_to_char("You can no longer edit mobiles.\n\r", mob);
            }
            send_to_char("Mobile edit flag removed.\n\r", ch);
        } else {
            SET_BIT(mob->player.user_flags, CAN_MOB_EDIT);
            if (ch != mob) {
                send_to_char("You can now edit mobiles.\n\r", mob);
            }
            send_to_char("Mobile edit flag set.\n\r", ch);
        }
    } else if (!strcmp(field, "stole")) {
        if (GetMaxLevel(ch) < GetMaxLevel(mob)) {
            send_to_char("I don't think so.\n\r", ch);
            oldSendOutput(mob, "%s tried to set your stole flag!\n\r",
                           GET_NAME(ch));
        } else if (IS_SET(mob->player.user_flags, STOLE_1) && IS_PC(mob)) {
            REMOVE_BIT(mob->player.user_flags, STOLE_1);
            send_to_char("Thief flag removed.\n\r", ch);
            if (ch != mob) {
                send_to_char("You have been pardoned for robbery!\n\r", mob);
            }
        } else {
            SET_BIT(mob->player.user_flags, STOLE_1);
            send_to_char("Thief flag set!\n\r", ch);
            if (mob != ch) {
                send_to_char("You have been accused of robbery!\n\r", mob);
            }
        }
    } else if (!strcmp(field, "pkill")) {
        if (PeacefulWorks) {
            PeacefulWorks = FALSE;
            EasySummon = FALSE;
            Log("Peaceful rooms and Easy Summon disabled by %s", GET_NAME(ch));
        } else {
            PeacefulWorks = TRUE;
            EasySummon = TRUE;
            Log("Peaceful rooms and Easy Summon enabled by %s", GET_NAME(ch));
        }
    /* Require one argument */
    } else if( !parmstr ) {
        send_to_char( "You need an argument for that, go read the help\n\r",
                      ch);
        return;
    } else if (!strcmp(field, "align")) {
        GET_ALIGNMENT(mob) = atoi(parmstr);
    } else if (!strcmp(field, "class")) {
        /*
         ** this will do almost nothing. (hopefully);
         */
        mob->player.class = atoi(parmstr);
    } else if (!strcmp(field, "exp")) {
        GET_EXP(mob) = atoi(parmstr);
    } else if (!strcmp(field, "specflags")) {
        GET_SPECFLAGS(mob) = atoi(parmstr);
        send_to_char("Changed flags.\n\r", ch);
    } else if (!strcmp(field, "zone")) {
        GET_ZONE(mob) = atoi(parmstr);
        oldSendOutput(ch, "Setting zone access to %d.\n\r", parm);
    } else if (!strcmp(field, "racewar")) {
        REMOVE_BIT(mob->player.user_flags, RACE_WAR);
        oldSendOutput(ch, "Removed RACE WAR from %s\n\r", GET_NAME(mob));
    } else if (!strcmp(field, "aff1")) {
        mob->specials.act = atoi(parmstr);
    } else if (!strcmp(field, "numatks")) {
        mob->mult_att = atoi(parmstr);
    } else if (!strcmp(field, "sex")) {
        GET_SEX(mob) = atoi(parmstr);
    } else if (!strcmp(field, "clan")) {
        GET_CLAN(mob) = atoi(parmstr);
    } else if (!strcmp(field, "leaderexp")) {
        GET_LEADERSHIP_EXP(mob) = atoi(parmstr);
    } else if (!strcmp(field, "race")) {
        GET_RACE(mob) = atoi(parmstr);
    } else if (!strcmp(field, "hunger")) {
        GET_COND(mob, FULL) = atoi(parmstr);
    } else if (!strcmp(field, "thirst")) {
        GET_COND(mob, THIRST) = atoi(parmstr);
    } else if (!strcmp(field, "zone")) {
        GET_ZONE(mob) = atoi(parmstr);
    } else if (!strcmp(field, "hit")) {
        GET_HIT(mob) = atoi(parmstr);
    } else if (!strcmp(field, "remortclass")) {
        mob->specials.remortclass = atoi(parmstr);
    } else if (!strcmp(field, "mhit")) {
        mob->points.max_hit = atoi(parmstr);
    } else if (!strcmp(field, "move")) {
        GET_MOVE(mob) = atoi(parmstr);
    } else if (!strcmp(field, "mmove")) {
        mob->points.max_move = atoi(parmstr);
    } else if (!strcmp(field, "tohit")) {
        GET_HITROLL(mob) = atoi(parmstr);
    } else if (!strcmp(field, "todam")) {
        GET_DAMROLL(mob) = atoi(parmstr);
    } else if (!strcmp(field, "ac")) {
        GET_AC(mob) = atoi(parmstr);
    } else if (!strcmp(field, "bank")) {
        GET_BANK(mob) = atoi(parmstr);
    } else if (!strcmp(field, "gold")) {
        GET_GOLD(mob) = atoi(parmstr);
    } else if (!strcmp(field, "prac")) {
        mob->specials.spells_to_learn = atoi(parmstr);
    } else if (!strcmp(field, "age")) {
        mob->player.time.birth -= SECS_PER_MUD_YEAR * atoi(parmstr);
    } else if (!strcmp(field, "str")) {
        parm = atoi(parmstr);
        mob->abilities.str = parm;
        mob->tmpabilities.str = parm;
    } else if (!strcmp(field, "add")) {
        parm = atoi(parmstr);
        mob->abilities.str_add = parm;
        mob->tmpabilities.str_add = parm;
    } else if (!strcmp(field, "dcredit")) {
        if (ch->pc) {
            GET_DIMD(mob) = atoi(parmstr);
        }
    } else if (!strcmp(field, "stadd")) {
        parm = atoi(parmstr);
        mob->abilities.str_add = parm;
        mob->tmpabilities.str_add = parm;
    } else if (!strcmp(field, "known")) {
        parm = atoi(parmstr);
        SET_BIT(mob->skills[parm].flags, SKILL_KNOWN);
        oldSendOutput(ch, "You set skill %d to known.\n\r", parm);
    } else if (!strcmp(field, "int")) {
        parm = atoi(parmstr);
        mob->abilities.intel = parm;
        mob->tmpabilities.intel = parm;
    } else if (!strcmp(field, "wis")) {
        parm = atoi(parmstr);
        mob->abilities.wis = parm;
        mob->tmpabilities.wis = parm;
    } else if (!strcmp(field, "dex")) {
        parm = atoi(parmstr);
        mob->abilities.dex = parm;
        mob->tmpabilities.dex = parm;
    } else if (!strcmp(field, "con")) {
        parm = atoi(parmstr);
        mob->abilities.con = parm;
        mob->tmpabilities.con = parm;
    } else if (!strcmp(field, "chr")) {
        parm = atoi(parmstr);
        mob->abilities.chr = parm;
        mob->tmpabilities.chr = parm;
    } else if (!strcmp(field, "mkills")) {
        mob->specials.m_kills = atoi(parmstr);
    } else if (!strcmp(field, "mdeaths")) {
        mob->specials.m_deaths = atoi(parmstr);
    } else if (!strcmp(field, "akills")) {
        mob->specials.a_kills = atoi(parmstr);
    } else if (!strcmp(field, "adeaths")) {
        mob->specials.a_deaths = atoi(parmstr);
    } else if (!strcmp(field, "mana")) {
        parm = atoi(parmstr);
        mob->points.mana = parm;
        mob->points.max_mana = parm;
    } else if (!strcmp(field, "start")) {
        mob->specials.start_room = atoi(parmstr);
    /* Require two parameters */
    } else if( !parmstr2 ) {
        send_to_char( "You need two arguments for that, go read the help\n\r",
                      ch);
        return;
    } else if (!strcmp(field, "lev")) {
        parm = atoi(parmstr);
        parm2 = atoi(parmstr2);

        if (!IS_NPC(mob)) {
            if ((GetMaxLevel(mob) >= GetMaxLevel(ch)) && (ch != mob)) {
                oldSendOutput(mob, "%s just tried to change your level.\n\r",
                               GET_NAME(ch));
                return;
            } else if (!IS_IMMORTAL(mob) &&
                       GetMaxLevel(ch) < IMPLEMENTOR && parm2 > MAX_MORT) {
                send_to_char("Thou shalt not create new immortals.\n\r", ch);
            }
        } else {
            if (parm2 <= MAX_CLASS) {
                GET_LEVEL(mob, parm2) = parm;
            }
            return;
        }

        if (parm < 0) {
            send_to_char("bug fix. :-)\n\r", ch);
            return;
        }

        if (parm < GetMaxLevel(ch)) {
            if (GetMaxLevel(ch) >= IMPLEMENTOR) {
                if (parm2 <= MAX_CLASS) {
                    GET_LEVEL(mob, parm2) = parm;
                }
            } else {
                if (parm > DEMIGOD) {
                    send_to_char("Sorry, you can't advance past 54th level\n",
                                 ch);
                    return;
                }
                if (parm2 <= MAX_CLASS) {
                    GET_LEVEL(mob, parm2) = parm;
                }
            }
        }
    } else if (!strcmp(field, "saves")) {
        parm = atoi(parmstr);
        parm2 = atoi(parmstr2);
        mob->specials.apply_saving_throw[parm] = parm2;
    } else if (!strcmp(field, "skills")) {
        parm = atoi(parmstr);
        parm2 = atoi(parmstr2);

        if (mob->skills) {
            mob->skills[parm].learned = parm2;
            oldSendOutput(ch, "You just set skill %d to value %d\n\r", parm, parm2);
        }
    } else {
        send_to_char("What the did you wanna set?\n\r", ch);
    }
}

void do_snoop(struct char_data *ch, char *argument, int cmd)
{
    char          *arg;
    struct char_data *victim;

    dlog("in do_snoop");

    if (!ch->desc) {
        return;
    }

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Ok, you just snoop yourself.\n\r", ch);
        if (ch->desc->snoop.snooping) {
            if (ch->desc->snoop.snooping->desc) {
                ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
            } else {
                Log("caught %s snooping %s who didn't have a descriptor!",
                    ch->player.name, ch->desc->snoop.snooping->player.name);
            }
            ch->desc->snoop.snooping = 0;
        }
        return;
    }
    if (!(victim = get_char_vis(ch, arg))) {
        send_to_char("No such person around.\n\r", ch);
        return;
    }
    if (!victim->desc) {
        send_to_char("There's no link.. nothing to snoop.\n\r", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("Ok, you just snoop yourself.\n\r", ch);
        if (ch->desc->snoop.snooping) {
            if (ch->desc->snoop.snooping->desc) {
                ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
            } else {
                Log("caught %s snooping %s who didn't have a descriptor!",
                    ch->player.name, ch->desc->snoop.snooping->player.name);
                /*
                 * logically.. this person has returned from being a
                 * creature?
                 */
            }
            ch->desc->snoop.snooping = 0;
        }
        return;
    }

    if (victim->desc->snoop.snoop_by) {
        send_to_char("Busy already. \n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    send_to_char("Ok. \n\r", ch);

    if (ch->desc->snoop.snooping && ch->desc->snoop.snooping->desc) {
        ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
    }

    ch->desc->snoop.snooping = victim;
    victim->desc->snoop.snoop_by = ch;
}

void do_switch(struct char_data *ch, char *argument, int cmd)
{
    char     *arg;
    struct char_data *victim;

    dlog("in do_switch");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Switch with who?\n\r", ch);
    } else if (!(victim = get_char(arg))) {
        send_to_char("They aren't here.\n\r", ch);
    } else {
        if (ch == victim) {
            send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
            return;
        }

        if (!ch->desc || ch->desc->snoop.snoop_by ||
            ch->desc->snoop.snooping) {
            send_to_char("Mixing snoop & switch is bad for your health.\n\r",
                         ch);
            return;
        }

        if (victim->desc || !IS_NPC(victim)) {
            send_to_char("You can't do that, the body is already in use!\n\r",
                         ch);
        } else {
            if (GetMaxLevel(victim) > GetMaxLevel(ch) &&
                GetMaxLevel(ch) < IMPLEMENTOR) {
                send_to_char("That being is much more powerful than "
                             "you!\n\r", ch);
                return;
            }

            send_to_char("Ok.\n\r", ch);

            ch->desc->character = victim;
            ch->desc->original = ch;

            victim->desc = ch->desc;
            ch->desc = 0;
        }
    }
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob = NULL,
                   *per;

    dlog("in do_return");

    if (!ch->desc) {
        return;
    }

    if (!ch->desc->original) {
        send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
        return;
    } 
    
    if (GET_POS(ch) == POSITION_FIGHTING && !IS_IMMORTAL(ch)) {
        send_to_char("Not in a fight!\n\r", ch);
        return;
    }

    send_to_char("You return to your original body.\n\r", ch);

    if (ch->desc->snoop.snoop_by) {
        /*
         * force the snooper to stop
         */
        do_snoop(ch->desc->snoop.snoop_by,
                 GET_NAME(ch->desc->snoop.snoop_by), 0);
    }

    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) {
        mob = ch;
        per = ch->desc->original;

        act("$n turns liquid, and reforms as $N.", TRUE, mob, 0, per,
            TO_ROOM);

        char_from_room(per);
        char_to_room(per, mob->in_room);

        SwitchStuff(mob, per);
    }

    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;

    ch->desc->character->desc = ch->desc;
    ch->desc = 0;

    if (IS_SET(ch->specials.act, ACT_POLYSELF) && cmd) {
        extract_char(mob);
    }
}

void do_genstatue(struct char_data *ch, char *argument, int cmd)
{
    char           *buf;
    void            update_pos(struct char_data *victim);

    dlog("in do_genstatue");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        generate_legend_statue();
#if 0
        ch, "", 584);
#endif
    } else {
        send_to_char("No need to add an argument, Usage: 'genstatue'.\n\r", ch);
    }
}

/*
 * Flux, restores hps/mana/mv for all mortals Lennya 20030408
 */
void do_flux(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    struct descriptor_data *i;
    char            buf[MAX_STRING_LENGTH];

    dlog("in do_flux");

    if (IS_NPC(ch)) {
        return;
    }

    if (!argument) {
        sprintf(buf, "%s just did a flux.", GET_NAME(ch));
        /*
         * want to keep an eye on how often it's used
         */
        log_sev(buf, 0);
        send_to_char("You send a warm aura resonating throughout the "
                     "lands.\n\r", ch);
        act("A warm aura emanates from $n.", TRUE, ch, 0, 0, TO_ROOM);
        for (i = descriptor_list; i; i = i->next) {
            if (!i->connected && !IS_IMMORTAL(i->character)) {
                /*
                 * only connected mortals will be fluxxed
                 */
                victim = i->character;
                /*
                 * flux restores hp, ma, mv
                 */
                GET_MANA(victim) = GET_MAX_MANA(victim);
                GET_HIT(victim) = GET_MAX_HIT(victim);
                GET_MOVE(victim) = GET_MAX_MOVE(victim);
                update_pos(victim);
                act("The resonance of a higher being surges through your soul,"
                    " leaving you $c0006r$c0002ef$c0010res$c0002he$c0006d"
                    "$c0007.", FALSE, victim, 0, ch, TO_CHAR);
            }
        }
    } else {
        send_to_char("No need to add an argument, Usage: 'flux'.\n\r", ch);
    }
}

void do_force(struct char_data *ch, char *argument, int cmd)
{
    struct descriptor_data *i;
    struct char_data *vict;
    char           *name,
                   *to_force,
                    buf[100];

    dlog("in do_force");
    if (IS_NPC(ch) && (cmd != 0)) {
        return;
    }

    argument = get_argument(argument, &name);
    to_force = skip_spaces(argument);
    if (!name || !to_force) {
        send_to_char("Who do you wish to force to do what?\n\r", ch);
    } else if (strcasecmp("all", name)) {
        if (!(vict = get_char_vis(ch, name))) {
            send_to_char("No-one by that name here..\n\r", ch);
        } else if (GetMaxLevel(ch) <= GetMaxLevel(vict) && !IS_NPC(vict)) {
            if (CAN_SEE(ch, vict)) {
                send_to_char("Oh no you don't!!\n\r", ch);
            }
        } else {
            sprintf(buf, "$n has forced you to '%s'.", to_force);
            act(buf, FALSE, ch, 0, vict, TO_VICT);
            send_to_char("Ok.\n\r", ch);
            command_interpreter(vict, to_force);
        }
    } else {
        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected) {
                vict = i->character;
                if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) &&
                    (!IS_NPC(vict))) {
                    if (CAN_SEE(ch, vict))
                        send_to_char("Oh no you don't!!\n\r", ch);
                } else {
                    sprintf(buf, "$n has forced you to '%s'.", to_force);
                    act(buf, FALSE, ch, 0, vict, TO_VICT);
                    command_interpreter(vict, to_force);
                }
            }
        }
        send_to_char("Ok.\n\r", ch);
    }
}

void do_load_mobile(struct char_data *ch, char *argument, int number)
{
    struct char_data *mob;

    if (number < 0) {
        for (number = 0; number < top_of_mobt; number++) {
            if (isname(argument, mob_index[number].name)) {
                break;
            }
        }
        if (number == top_of_mobt) {
            number = -1;
        }
    } else {
        number = real_mobile(number);
    }
    if (number < 0 || number >= top_of_mobt) {
        send_to_char("There is no such monster.\n\r", ch);
        return;
    }
    mob = read_mobile(number, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
        0, 0, TO_ROOM);
    act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
    act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0,
        mob, TO_CHAR);
}

void do_load_object(struct char_data *ch, char *argument, int number)
{
    struct obj_data *obj;

    if (number < 0) {
        number = db_find_object_named(argument, -1, -1);
    }

    if (number < 0 || !(obj = objectRead(number, VIRTUAL))) {
        send_to_char("There is no such object.\n\r", ch);
        return;
    }

    objectGiveToChar(obj, ch);

    if (GetMaxLevel(ch) < MAX_IMMORT) {
        sprintf(buf, "%s loaded %s (%d)", GET_NAME(ch), 
                      obj->short_description, obj->item_number);
        log_sev(buf, 0);
    }

    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
}

void do_load_room(struct char_data *ch, char *argument)
{
    char           *arg1,
                   *arg2;
    int             start,
                    end;

    if (GetMaxLevel(ch) < CREATOR) {
        return;
    }

    num = get_argument(argument, &arg1);
    num = get_argument(argument, &arg2);

    if( !arg1 ) {
        send_to_char("Load? Fine!  Load we must, But what?\n\r", ch);
        return;
    }
    start = atoi(arg1);

    if( !arg2 ) {
        end = start;
    } else {
        end = atoi(arg2);
    }
    RoomLoad(ch, start, end);
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
    char           *type,
                    buf[100];
    int             number;

    dlog("in do_load");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &type);

    if( !type || !argument ) {
        send_to_char( "Hmmm, what?\n\r", ch );
        return;
    }

    if (isdigit((int)*argument)) {
        number = atoi(argument);
    } else {
        number = -1;
    }

    if (is_abbrev(type, "mobile")) {
        do_load_mobile(ch, argument, number);
    } else if (is_abbrev(type, "object")) {
        do_load_object(ch, argument, number);
    } else if (is_abbrev(type, "room")) {
        do_load_room(ch, argument);
    } else {
        send_to_char("Usage: load (object|mobile) (number|name)\n\r"
                     "       load room start [end]\n\r", ch);
    }
}

void purge_one_room(int rnum, struct room_data *rp, int *range)
{
    struct char_data *ch;
    struct obj_data *obj;
    extern long     room_count;

    /*
     * purge the void? I think not
     */
    if (rnum == 0 || rnum < range[0] || rnum > range[1]) {
        return;
    }
    while (rp->people) {
        ch = rp->people;
        send_to_char("A god strikes the heavens making the ground around you "
                     "erupt into a\n\r"
                     "fluid fountain boiling into the ether.  All that's left "
                     "is the Void.", ch);
        char_from_room(ch);
        /*
         * send character to the void
         */
        char_to_room(ch, 0);
        do_look(ch, NULL, 15);
        act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
    }

    while (rp->contents) {
        obj = rp->contents;
        objectTakeFromRoom(obj);
        /*
         * send item to the void
         */
        objectPutInRoom(obj, 0);
    }

    completely_cleanout_room(rp);
#ifdef HASH
    hash_remove(&room_db, rnum);
#else
    room_remove(room_db, rnum);
#endif
    room_count--;
}

/*
 * clean a room of all mobiles and objects
 */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict,
                   *next_v;
    struct obj_data *obj,
                   *next_o;

    char           *name;
    struct descriptor_data *d;
    int             range[2];
    register int    i;
    struct room_data *rp;

    dlog("in do_purge");

    if (IS_NPC(ch) && cmd != 0) {
        return;
    }

    argument = get_argument(argument, &name);
    if (name) {
        /*
         * argument supplied. destroy single object or char
         */
        if (strcmp(name, "links") == 0 && GetMaxLevel(ch) >= IMPLEMENTOR) {
            for (d = descriptor_list; d; d = d->next) {
                close_socket(d);
            }
            return;
        }

        if ((vict = get_char_room_vis(ch, name)) != NULL) {
            if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) &&
                GetMaxLevel(ch) < IMPLEMENTOR) {
                send_to_char("I'm sorry...  I can't let you do that.\n\r", ch);
                return;
            }

            act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

            if (IS_NPC(ch) && IS_NPC(vict)) {
                Log("npc just purged a mob! COOL!");
            }

            if (IS_NPC(vict)) {
                extract_char(vict);
            } else if (vict->desc) {
                close_socket(vict->desc);
                vict->desc = 0;
                extract_char(vict);
            } else {
                extract_char(vict);
            }
        } else if ((obj = get_obj_in_list_vis(ch, name,
                                         real_roomp(ch->in_room)->contents))) {
            act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
            objectExtract(obj);
        } else if (!strcasecmp("room", name)) {
            if (GetMaxLevel(ch) < IMPLEMENTOR) {
                send_to_char("I'm sorry, I can't let you do that.\n\r", ch);
                return;
            }

            argument = get_argument(argument, &name);
            if (!name || !isdigit((int)*name)) {
                send_to_char("purge room start [end]", ch);
                return;
            }

            range[0] = atoi(name);

            argument = get_argument(argument, &name);
            if (name && isdigit((int)*name)) {
                range[1] = atoi(name);
            } else {
                range[1] = range[0];
            }

            if (range[0] == 0 || range[1] == 0) {
                send_to_char("usage: purge room start [end]\n\r", ch);
                return;
            }

#ifdef HASH
            hash_iterate(&room_db, purge_one_room, range);
#else
            if (range[0] >= WORLD_SIZE || range[1] >= WORLD_SIZE) {
                send_to_char("only purging to WORLD_SIZE\n\r", ch);
                return;
            }
            for (i = range[0]; i <= range[1]; i++) {
                if ((rp = real_roomp(i)) != 0) {
                    purge_one_room(i, rp, range);
                }
            }
#endif
        } else {
            send_to_char("I don't see that here.\n\r", ch);
            return;
        }

        send_to_char("Ok.\n\r", ch);
    } else {
        /*
         * no argument. clean out the room
         */
        if (GetMaxLevel(ch) < DEMIGOD) {
            return;
        }
        if (IS_NPC(ch)) {
            send_to_char("You would only kill yourself..\n\r", ch);
            return;
        }

        act("$n gestures... You are surrounded by thousands of tiny scrubbing"
            " bubbles!", FALSE, ch, 0, 0, TO_ROOM);
        send_to_room("The world seems a little cleaner.\n\r", ch->in_room);

        for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict) &&
                !IS_SET(vict->specials.act, ACT_POLYSELF)) {
                extract_char(vict);
            }
        }

        for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
            next_o = obj->next_content;
            objectExtract(obj);
        }
    }
}

void do_start(struct char_data *ch)
{
    int             r_num,
                    temp = 0;
    struct obj_data *obj;
    char            buf[256];

    void            advance_level(struct char_data *ch, int i);

    dlog("in do_start");

    send_to_char("Welcome to Havok Mud.  Enjoy the game...\n\r", ch);

    sprintf(buf, "$c0015A new apprentice named $c0009%s$c0015 joins the "
                 "ranks of $c0012Havok$c0014\n\r", GET_NAME(ch));
    send_to_all(buf);

    ch->specials.start_room = NOWHERE;
    StartLevels(ch);
    GET_EXP(ch) = 1;
    set_title(ch);

    /*
     *  This is the old style of determining hit points.  I modified it so that
     *  characters get the standard AD&D + 10 hp to start.
     *  ch->points.max_hit  = 10;
     */

    /*
     * outfit char with valueless items
     */

    /*
     * bread
     */
    obj = objectRead(12, VIRTUAL);
    objectGiveToChar(obj, ch);
    obj = objectRead(12, VIRTUAL);
    objectGiveToChar(obj, ch);

    /*
     * water
     */
    obj = objectRead(13, VIRTUAL);
    objectGiveToChar(obj, ch);
    obj = objectRead(13, VIRTUAL);
    objectGiveToChar(obj, ch);

    ch->skills[STYLE_STANDARD].learned = 95;
    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        command_interpreter(ch, "group all");
    }

    if (HasClass(ch, CLASS_CLERIC | CLASS_MAGIC_USER | CLASS_SORCERER |
                     CLASS_PSI | CLASS_PALADIN | CLASS_RANGER | CLASS_DRUID |
                     CLASS_NECROMANCER)) {
        ch->skills[SKILL_READ_MAGIC].learned = 95;
    }

    SetDefaultLang(ch);

    if( IS_SET(ch->specials.act, PLR_POSTING) ) {
        REMOVE_BIT(ch->specials.act, PLR_POSTING);
    }

    if (IS_SET(ch->player.user_flags, USE_ANSI)) {
        temp = 1;
    }
    GET_SPECFLAGS(ch) = 0;
    SET_BIT(ch->player.user_flags, USE_PAGING);

    if (temp == 1) {
        SET_BIT(ch->player.user_flags, USE_ANSI);
    }
    if (!IS_SET(ch->specials.act, NEW_USER)) {
        SET_BIT(ch->player.user_flags, NEW_USER);
    }
    send_to_char("New user mode activated.\n\r", ch);

    if (!IS_SET(ch->specials.act, PLR_WIMPY)) {
        SET_BIT(ch->specials.act, PLR_WIMPY);
    }
    send_to_char("Wimpy mode activated\n\r", ch);

    if (!IS_SET(ch->player.user_flags, SHOW_EXITS)) {
        SET_BIT(ch->player.user_flags, SHOW_EXITS);
    }
    send_to_char("Autoexits activated\n\r", ch);

    if (IS_SET(ch->player.class, CLASS_THIEF)) {
        switch (GET_RACE(ch)) {
            case RACE_HUMAN:
            case RACE_MOON_ELF:
            case RACE_GOLD_ELF:
            case RACE_WILD_ELF:
            case RACE_SEA_ELF:
            case RACE_AVARIEL:
            case RACE_DWARF:
            case RACE_HALFLING:
            case RACE_ROCK_GNOME:
            case RACE_DEEP_GNOME:
            case RACE_FOREST_GNOME:
                ch->skills[SKILL_SNEAK].learned = 10;
                ch->skills[SKILL_HIDE].learned = 5;
                ch->skills[SKILL_STEAL].learned = 15;
                ch->skills[SKILL_BACKSTAB].learned = 10;
                ch->skills[SKILL_PICK_LOCK].learned = 10;
                break;
            case RACE_DROW:
                ch->skills[SKILL_SNEAK].learned = 20;
                ch->skills[SKILL_HIDE].learned = 15;
                ch->skills[SKILL_STEAL].learned = 25;
                ch->skills[SKILL_BACKSTAB].learned = 20;
                ch->skills[SKILL_PICK_LOCK].learned = 5;
                break;
            case RACE_HALF_ELF:
                ch->skills[SKILL_HIDE].learned = 5;
                ch->skills[SKILL_STEAL].learned = 10;
                break;
            default:
                break;
        }
    }

    ch->skills[SKILL_BASH].learned = 0;
    ch->skills[SKILL_KICK].learned = 0;

    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MANA(ch) = GET_MAX_MANA(ch);

    /*
     * ch->points.max_move += GET_CON(ch) + number(1,20) - 9;
     */

    GET_MOVE(ch) = GET_MAX_MOVE(ch);

    GET_COND(ch, THIRST) = 24;
    GET_COND(ch, FULL) = 24;
    GET_COND(ch, DRUNK) = 0;

    ch->points.gold = 150;

    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

}

void do_advance(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *name,
                   *levelarg,
                   *class;
    int             adv,
                    newlevel,
                    lin_class,
                    level;

    void            gain_exp(struct char_data *ch, int gain);

    dlog("in do_advance");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &name);
    if (!name) {
        send_to_char("Advance who?\n\r", ch);
        return;
    }

    if (!(victim = get_char_room_vis(ch, name))) {
        send_to_char("That player is not here.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("NO! Not on NPC's.\n\r", ch);
        return;
    }

    argument = get_argument(argument, &class);
    if (!class) {
        send_to_char("Must supply a class (M C W T D K B P R I S N)\n\r", ch);
        return;
    }

    switch (tolower(*class)) {
    case 'm':
        lin_class = MAGE_LEVEL_IND;
        break;

    case 't':
        lin_class = THIEF_LEVEL_IND;
        break;

    case 'w':
    case 'f':
        lin_class = WARRIOR_LEVEL_IND;
        break;

    case 'c':
        lin_class = CLERIC_LEVEL_IND;
        break;

    case 'd':
        lin_class = DRUID_LEVEL_IND;
        break;

    case 'k':
        lin_class = MONK_LEVEL_IND;
        break;

    case 'B':
        lin_class = BARBARIAN_LEVEL_IND;
        break;

    case 'p':
        lin_class = PALADIN_LEVEL_IND;
        break;

    case 'r':
        lin_class = RANGER_LEVEL_IND;
        break;

    case 'i':
        lin_class = PSI_LEVEL_IND;
        break;

    case 's':
        lin_class = SORCERER_LEVEL_IND;
        break;

    case 'n':
        lin_class = NECROMANCER_LEVEL_IND;
        break;

    default:
        send_to_char("Must supply a class (M C W T D K B P R I S N)\n\r", ch);
        return;
        break;
    }

    argument = get_argument(argument, &levelarg);

    level = GET_LEVEL(victim, lin_class);

    if (level == 0) {
        adv = 1;
    } else {
        if (!levelarg) {
            send_to_char("You must supply a level number.\n\r", ch);
            return;
        }

        if (!isdigit((int)*levelarg)) {
            send_to_char("Third argument must be a positive integer.\n\r", ch);
            return;
        }

        newlevel = atoi(levelarg);
        if (newlevel < level) {
            send_to_char("Can't dimish a players status.\n\r", ch);
            return;
        }
        adv = newlevel - level;
    }

    newlevel = level + adv;
    if (newlevel > 1 && GetMaxLevel(ch) < IMPLEMENTOR) {
        send_to_char("Thou art not godly enough.\n\r", ch);
        return;
    }

    if (newlevel > IMPLEMENTOR) {
        send_to_char("Implementor is the highest possible level.\n\r", ch);
        return;
    }

    if (newlevel < 1) {
        send_to_char("1 is the lowest possible level.\n\r", ch);
        return;
    }

    send_to_char("You feel generous.\n\r", ch);
    act("$n makes some strange gestures.\n\r"
        "A strange feeling comes upon you, Like a giant hand, light comes "
        "down from\n\r"
        "above, grabbing your body, that begins to pulse with coloured lights "
        "from\n\r"
        "inside.  Your head seems to be filled with demons from another plane "
        "as your\n\r"
        "body dissolves into the elements of time and space itself.  Suddenly a"
        " silent\n\r"
        "explosion of light snaps you back to reality. You feel slightly "
        "different.", FALSE, ch, 0, victim, TO_VICT);

    if (level == 0) {
        do_start(victim);
    } else if (level < IMPLEMENTOR) {
        gain_exp_regardless(victim, 
                            (classes[lin_class].levels[newlevel].exp -
                             GET_EXP(victim)),
                            lin_class);

        send_to_char("Character is now advanced.\n\r", ch);
    } else {
        send_to_char("Some idiot just tried to advance your level.\n\r",
                     victim);
        send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
    }
}

/**
 * @todo remove this POS
 */
void do_reroll(struct char_data *ch, char *argument, int cmd)
{
    dlog("in do_reroll");
    send_to_char("Use @ command instead.\n\r", ch);
}

void do_restore(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *victim;
    char           *buf;
    int             i;
    void            update_pos(struct char_data *victim);

    dlog("in do_restore");

    if (cmd == 0) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        send_to_char("Who do you wish to restore?\n\r", ch);
    } else if (!(victim = get_char(buf))) {
        send_to_char("No-one by that name in the world.\n\r", ch);
    } else {
        GET_MANA(victim) = GET_MAX_MANA(victim);
        GET_HIT(victim) = GET_MAX_HIT(victim);
        GET_MOVE(victim) = GET_MAX_MOVE(victim);

        affect_from_char(victim, COND_WINGS_BURNED);
        affect_from_char(victim, COND_WINGS_TIRED);
        if (IS_NPC(victim)) {
            return;
        }
        if (victim->player.time.logon + victim->player.time.played < 0) {
            victim->player.time.logon = 0;
            victim->player.time.played = 0;
        }
        if (!IS_IMMORTAL(victim)) {
            GET_COND(victim, THIRST) = 24;
            GET_COND(victim, FULL) = 24;
        } else {
            GET_COND(victim, THIRST) = -1;
            GET_COND(victim, FULL) = -1;
            GET_COND(victim, DRUNK) = -1;
        }
        if (IS_IMMORTAL(victim)) {
            for (i = 0; i < MAX_SKILLS; i++) {
                victim->skills[i].learned = 100;
                victim->skills[i].special = 1;
                /*
                 * specialized
                 */
                SET_BIT(victim->skills[i].flags, SKILL_KNOWN);
                victim->skills[i].nummem = 99;
                /*
                 * clear memorized
                 */
            }
        }
        if (GetMaxLevel(victim) >= GOD) {
            victim->abilities.str_add = 100;
            victim->abilities.intel = 25;
            victim->abilities.wis = 25;
            victim->abilities.dex = 25;
            victim->abilities.str = 25;
            victim->abilities.con = 25;
            victim->abilities.chr = 25;
            victim->tmpabilities = victim->abilities;
        }

        /*
         * this should give all immortals all classes and set
         * all levels to the max level they are (i.e. 51 level warrior )
         */

        if (GetMaxLevel(victim) > MAX_MORT) {
            for (i = 0; i < MAX_CLASS; i++) {
                if (GET_LEVEL(victim, i) < GetMaxLevel(victim))
                    GET_LEVEL(victim, i) = GetMaxLevel(victim);
            }
            for (i = 1; i <= CLASS_NECROMANCER; i *= 2) {
                if (!HasClass(victim, i))
                    victim->player.class += i;
            }
        }

        update_pos(victim);
        send_to_char("Done.\n\r", ch);
        act("The hand of $N touches you lightly on the forehead, you have "
            "been healed!", FALSE, victim, 0, ch, TO_CHAR);
    }
}

void do_noshout(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char           *buf;

    dlog("in do_noshout");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
            send_to_char("You can now hear shouts again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
        } else {
            send_to_char("From now on, you won't hear shouts.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOSHOUT);
        }
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
        send_to_char("Couldn't find any such creature.\n\r", ch);
    } else if (IS_NPC(vict)) {
        send_to_char("Can't do that to a beast.\n\r", ch);
    } else if (GetMaxLevel(vict) >= GetMaxLevel(ch)) {
        act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    } else if (IS_SET(vict->specials.act, PLR_NOSHOUT) && 
               GetMaxLevel(ch) >= SAINT) {
        send_to_char("You can shout again.\n\r", vict);
        send_to_char("NOSHOUT removed.\n\r", ch);
        REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
    } else if (GetMaxLevel(ch) >= SAINT) {
        send_to_char("The gods take away your ability to shout!\n\r", vict);
        send_to_char("NOSHOUT set.\n\r", ch);
        SET_BIT(vict->specials.act, PLR_NOSHOUT);
    } else {
        send_to_char("Sorry, you can't do that\n\r", ch);
    }
}

void do_nohassle(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char           *buf;

    dlog("in do_nohassle");

    if (IS_NPC(ch)){
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
            send_to_char("You can now be hassled again.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
        } else {
            send_to_char("From now on, you won't be hassled.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOHASSLE);
        }
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
        send_to_char("Couldn't find any such creature.\n\r", ch);
    } else if (IS_NPC(vict)) {
        send_to_char("Can't do that to a beast.\n\r", ch);
    } else if (GetMaxLevel(vict) > GetMaxLevel(ch)) {
        act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    } else {
        send_to_char("The implementor won't let you set this on "
                     "mortals...\n\r", ch);
    }
}

void do_stealth(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char           *buf;

    dlog("in do_stealth");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        if (IS_SET(ch->specials.act, PLR_STEALTH)) {
            send_to_char("STEALTH mode OFF.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_STEALTH);
        } else {
            send_to_char("STEALTH mode ON.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_STEALTH);
        }
    } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
        send_to_char("Couldn't find any such creature.\n\r", ch);
    } else if (IS_NPC(vict)) {
        send_to_char("Can't do that to a beast.\n\r", ch);
    } else if (GetMaxLevel(vict) > GetMaxLevel(ch)) {
        act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    } else {
        send_to_char("The implementor won't let you set this on "
                     "mortals...\n\r", ch);
    }
}

void print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
    char            buf[MAX_STRING_LENGTH];
    int             dink,
                    bits,
                    scan;

    if ((rp->sector_type < 0) || (rp->sector_type > 9)) {
        /*
         * non-optimal
         */
        rp->sector_type = 0;
    }
    sprintf(buf, "%5ld %4d %-12s %s", rp->number, rnum,
            sectors[rp->sector_type].type, (rp->name ? rp->name : "Empty"));
    strcat(buf, " [");

    dink = 0;
    for (bits = rp->room_flags, scan = 0; bits; scan++) {
        if (bits & (1 << scan)) {
            if (dink) {
                strcat(buf, " ");
            }
            strcat(buf, room_bits[scan]);
            dink = 1;
            bits ^= (1 << scan);
        }
    }
    strcat(buf, "]\n\r");

    append_to_string_block(sb, buf);
}

/**
 * @todo replace with macro
 */
void print_death_room(int rnum, struct room_data *rp,
                      struct string_block *sb)
{
    if (rp && rp->room_flags & DEATH) {
        print_room(rnum, rp, sb);
    }
}

/**
 * @todo replace with macro
 */
void print_private_room(int rnum, struct room_data *rp,
                        struct string_block *sb)
{
    if (rp && rp->room_flags & PRIVATE) {
        print_room(rnum, rp, sb);
    }
}

struct show_room_zone_struct {
    int             blank;
    int             startblank,
                    lastblank;
    int             bottom,
                    top;
    struct string_block *sb;
};

void show_room_zone(int rnum, struct room_data *rp,
                    struct show_room_zone_struct *srzs)
{
    char            buf[MAX_STRING_LENGTH];

    /*
     * optimize later
     */
    if (!rp || rp->number < srzs->bottom || rp->number > srzs->top) {
        return;
    }

    if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
        sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank,
                srzs->lastblank);
        append_to_string_block(srzs->sb, buf);
        srzs->blank = 0;
    }

    if (sscanf(rp->name, "%d", &srzs->lastblank) == 1 &&
        srzs->lastblank == rp->number) {
        if (!srzs->blank) {
            srzs->startblank = srzs->lastblank;
            srzs->blank = 1;
        }
        return;
    } 
    
    if (srzs->blank) {
        sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank,
                srzs->lastblank);
        append_to_string_block(srzs->sb, buf);
        srzs->blank = 0;
    }

    print_room(rnum, rp, srzs->sb);
}

void do_show_zones( struct char_data *ch, struct string_block *sb, 
                    char *argument )
{
    int             zone;
    struct zone_data *zd;
    int             bottom = 0,
                    top = 0;
    char           *mode;

    append_to_string_block(sb, "# Zone   name                        "
                               "        lifespan age     rooms     "
                               "deinit   reset\n\r");

    for (zone = 0; zone <= top_of_zone_table; zone++) {
        zd = &zone_table[zone];
        switch (zd->reset_mode) {
        case 0:
            mode = "never    never";
            break;
        case 1:
            mode = "ifempty  ifempty";
            break;
        case 2:
            mode = "ifempty  always";
            break;
        case 3:
            mode = "closed   closed";
            break;
        case 128:
            mode = "never    always";
            break;
        default:
            if (zd->reset_mode > 2) {
                if (IS_SET(zd->reset_mode, ZONE_ALWAYS))
                    mode = "*never   *never";
                else if (IS_SET(zd->reset_mode, ZONE_EMPTY))
                    mode = "*ifempty *ifempty";
                else if (IS_SET(zd->reset_mode, ZONE_CLOSED))
                    mode = "*closed  *closed";
                else if (IS_SET(zd->reset_mode, ZONE_NODEINIT))
                    mode = "*never   *always";
                else
                    mode = "*never   *never";
            } else {
                mode = "!unknown!";
            }
        }
        sprintf(buf, "%4d %-40s %4dm %4dm %6d-%-6ld %s\n\r", zone,
                zd->name, zd->lifespan, zd->age, bottom, zd->top, mode);
        append_to_string_block(sb, buf);
        bottom = zd->top + 1;
    }
}

void do_show_objects( struct char_data *ch, struct string_block *sb, 
                      char *argument )
{
    char                   *zonenum;
    int                     zone;
    int                     bottom = 0,
                            top = 0;
    struct obj_data        *obj;
    int                     i;
    struct index_data      *index;
    char                   *objname;
    Keywords_t             *key;
    char                    buf[MAX_STRING_LENGTH];
    char                    color[10];
    BalancedBTreeItem_t    *item;

    zonenum = skip_spaces(argument);
    if( !zonenum ) {
        send_to_char("What zone is that again?  I can't hear ya!\n\r", ch);
        return;
    }

    if( isdigit((int)*zonenum) ) {
        zone = atoi( zonenum );
        if (zone <= 0 || zone > top_of_zone_table) {
            send_to_char("That is not a valid zone_number\n\r", ch);
            return;
        }
        bottom = zone_table[zone - 1].top + 1;
        top = zone_table[zone].top;
    } else {
        zone = -1;
        /* Just so they are initialized */
        bottom = zone_table[0].top + 1;
        top = zone_table[top_of_zone_table].top;
    }

    key = StringToKeywords(zonenum, NULL);
    append_to_string_block(&sb, "VNUM  count e-value names\n\r");
    BalancedBTreeLock(objectTree);
    for (item = BalancedBTreeFindLeastInRange( objectTree, LOCKED, &bottom,
                                               &top );
         item;
         item = BalancedBTreeFindNextInRange( objectTree, item, LOCKED, &bottom,
                                              &top )) {
        index = (struct index_data *)item->item;
        if( !index || (zone < 0 && !KeywordsMatch(key, &index->keywords))) {
            continue;
        }

        obj = objectRead(index->vnum, VIRTUAL);
        if (obj) {
            if (eval(obj) < -5) {
                sprintf(color, "%s", "$c0008");
            } else if (eval(obj) < 20) {
                sprintf(color, "%s", "");
            } else {
                sprintf(color, "%s", "$c000W");
            }

            objname = KeywordsToString( &index->keywords, " " );
            sprintf(buf, "%5ld %3d %s%7d   $c000w%s\n\r", index->vnum, 
                    (index->number - 1), color, eval(obj), objname);
            free( objname );
            append_to_string_block(sb, buf);
            objectExtract(obj);
        }
    }
    BalancedBTreeUnlock(objectTree);
    FreeKeywords(key, TRUE);
}


void do_show_wearslot( struct char_data *ch, struct string_block *sb, 
                       char *argument )
{
    char               *arg;
    int                 wearslot;
    int                 i;
    struct obj_data    *obj;
    struct index_data  *index;
    char                buf[MAX_STRING_LENGTH];
    char                color[10];
    char               *objname;
    BalancedBTreeItem_t *item;

    arg = skip_spaces(argument);
    if (!arg || !(isdigit((int)*arg))) {
        append_to_string_block(sb, "Usage:\n\r"
                               "  show wearslot #\n\r"
                               "  Number ranging from  0   TAKE\n\r"
                               "                       1   FINGER\n\r"
                               "                       2   NECK\n\r"
                               "                       3   BODY\n\r"
                               "                       4   HEAD\n\r"
                               "                       5   LEGS\n\r"
                               "                       6   FEET\n\r"
                               "                       7   HANDS\n\r"
                               "                       8   ARMS\n\r"
                               "                       9   SHIELD\n\r");
        append_to_string_block(sb, "                      10   ABOUT\n\r"
                               "                      11   WAIST\n\r"
                               "                      12   WRIST\n\r"
                               "                      13   WIELD\n\r"
                               "                      14   HOLD\n\r"
                               "                      15   THROW\n\r"
                               "                      16   LIGHT-SOURCE\n\r"
                               "                      17   BACK\n\r"
                               "                      18   EARS\n\r"
                               "                      19   EYES\n\r");
        return;
    } 
    
    wearslot = atoi(arg);
    append_to_string_block(sb, "VNUM  count e-value names\n\r");

    BalancedBTreeLock( objectTree );
    BalancedBTreeClearVisited( objectTree, LOCKED );

    for (item = BalancedBTreeFindLeast( objectTree->root );
         item;
         item = BalancedBTreeFindNext( objectTree, item, LOCKED ) ) {
        index = (struct index_data *)item->item;

        obj = objectRead(index->vnum, VIRTUAL);
        if (obj) {
            if (IS_SET(obj->wear_flags, pc_num_class(wearslot))) {
                if (eval(obj) < -5) {
                    sprintf(color, "%s", "$c0008");
                } else if (eval(obj) < 20) {
                    sprintf(color, "%s", "");
                } else {
                    sprintf(color, "%s", "$c000W");
                }
                objname = KeywordsToString( &index->keywords, " " );
                sprintf(buf, "%5ld %3d %s%7d   $c000w%s\n\r", index->vnum, 
                        (index->number - 1), color, eval(obj), objname);
                free( objname );
                append_to_string_block(sb, buf);
            }
            objectExtract(obj);
        }
    }
    BalancedBTreeUnlock( objectTree );
}

/** 
 * @todo redo itemtype by traversing the binary tree, this is silly 
 */
void do_show_itemtype( struct char_data *ch, struct string_block *sb, 
                       char *argument )
{
    char               *arg;
    int                 type;
    int                 i;
    struct obj_data    *obj;
    struct index_data  *index;
    char                buf[MAX_STRING_LENGTH];
    char                color[10];
    char               *objname;
    BalancedBTreeItem_t *item;
    
    arg = skip_spaces(argument);
    if (!arg || !(isdigit((int)*arg))) {
        append_to_string_block(sb, "Usage:\n\r"
                               "  show itemtype #\n\r"
                               "  Number ranging from  0   UNDEFINED\n\r"
                               "                       1   LIGHT SOURCE\n\r"
                               "                       2   SCROLL\n\r"
                               "                       3   WAND\n\r"
                               "                       4   STAFF\n\r"
                               "                       5   WEAPON\n\r"
                               "                       6   FIREWEAPON\n\r"
                               "                       7   MISSILE\n\r"
                               "                       8   TREASURE\n\r"
                               "                       9   ARMOR\n\r"
                               "                      10   POTION\n\r"
                               "                      11   WORN\n\r"
                               "                      12   OTHER\n\r");
        append_to_string_block(sb,
                               "                      13   THRASH\n\r"
                               "                      14   TRAP\n\r"
                               "                      15   CONTAINER\n\r"
                               "                      16   NOTE\n\r"
                               "                      17   LIQ "
                               "CONTAINER\n\r"
                               "                      18   KEY\n\r"
                               "                      19   FOOD\n\r"
                               "                      20   MONEY\n\r"
                               "                      21   PEN\n\r"
                               "                      22   BOAT\n\r"
                               "                      23   AUDIO\n\r"
                               "                      24   BOARD\n\r"
                               "                      25   TREE\n\r");
        append_to_string_block(sb,
                               "                      26   ROCK\n\r"
                               "                      27   PORTAL\n\r"
                               "                      28   INSTRUMENT\n\r");
        return;
    } 
    
    type = atoi(arg);
    append_to_string_block(sb, "VNUM  count e-value names\n\r");

    BalancedBTreeLock( objectTree );
    BalancedBTreeClearVisited( objectTree, LOCKED );
    
    for (item = BalancedBTreeFindLeast( objectTree->root );
         item;
         item = BalancedBTreeFindNext( objectTree, item, LOCKED ) ) {

        index = (struct index_data *)item->item;

        obj = objectRead(index->vnum, VIRTUAL);
        if (obj) {
            if (ITEM_TYPE(obj) == type) {
                if (eval(obj) < -5) {
                    sprintf(color, "%s", "$c0008");
                } else if (eval(obj) < 20) {
                    sprintf(color, "%s", "");
                } else {
                    sprintf(color, "%s", "$c000W");
                }
                objname = KeywordsToString( &index->keywords, " " );
                sprintf(buf, "%5ld %3d %s%7d   $c000w%s\n\r", index->vnum,
                        (index->number - 1), color, eval(obj), objname);
                free(objname);
                append_to_string_block(sb, buf);
            }
            objectExtract(obj);
        }
    }
    BalancedBTreeUnlock( objectTree );
}

void do_show_mobiles( struct char_data *ch, struct string_block *sb, 
                      char *argument )
{
    char               *zonenum;
    int                 zone;
    int                 top = 0,
                        bottom = 0;
    struct index_data  *index;
    int                 i;
    char                buf[MAX_STRING_LENGTH];
    Keywords_t         *key;

    zonenum = skip_spaces(argument);
    zone = -1;

    if( zonenum && isdigit((int)*zonenum) ) {
        zone = atoi(zonenum);
        if (!zonenum || zone < 0 || zone > top_of_zone_table) {
            send_to_char("That is not a valid zone_number\n\r",ch);
            return;
        }
    }

    if (zone >= 0) {
        bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
        top = zone_table[zone].top;
    }

    key = StringToKeywords(zonenum, NULL);
    append_to_string_block(sb, "VNUM  rnum count names\n\r");

    for (i = 0; i < top_of_mobt; i++) {
        index = &mob_index[i];
        if ((zone >= 0 && (index->vnum < bottom || index->vnum > top)) ||
            (zone < 0 && !KeywordsMatch(key, &index->keywords))) {
            continue;
        }
        /*
         * optimize later
         */
        sprintf(buf, "%5ld %4d %3d  %s\n\r", index->vnum, i, index->number, 
                index->name);
        append_to_string_block(sb, buf);
    }
    FreeKeywords(key, TRUE);
}

void do_show_rooms( struct char_data *ch, struct string_block *sb, 
                    char *argument )
{
    char               *zonenum;
    int                 zone;
    struct show_room_zone_struct srzs;

    zonenum = skip_spaces(argument);
    zone = -1;

    if( !zonenum ) {
        send_to_char( "Show what rooms, silly?\n\r", ch );
        return;
    }

    if( isdigit((int)*zonenum) ) {
        zone = atoi(zonenum);
    }

    append_to_string_block(sb, "VNUM  rnum type         name [BITS]\n\r");

    if (is_abbrev(zonenum, "death")) {
#ifdef HASH
        hash_iterate(&room_db, print_death_room, sb);
#else
        room_iterate(room_db, print_death_room, sb);
#endif
    } else if (is_abbrev(zonenum, "private")) {
#ifdef HASH
        hash_iterate(&room_db, print_private_room, sb);
#else
        room_iterate(room_db, print_private_room, sb);
#endif
    } else if (zone < 0 || zone > top_of_zone_table) {
        append_to_string_block(sb, "I need a zone number with this "
                                    "command\n\r");
    } else {
        srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
        srzs.top = zone_table[zone].top;
        srzs.blank = 0;
        srzs.sb = &sb;
#ifdef HASH
        hash_iterate(&room_db, show_room_zone, &srzs);
#else
        room_iterate(room_db, show_room_zone, &srzs);
#endif
        if (srzs.blank) {
            sprintf(buf, "rooms %d-%d are blank\n\r", srzs.startblank,
                    srzs.lastblank);
            append_to_string_block(sb, buf);
            srzs.blank = 0;
        }
    }
}

void do_show_report( struct char_data *ch, struct string_block *sb, 
                     char *argument )
{
    char               *zonenum;
    int                 zone;
    int                 top = 0,
                        bottom = 0;
    Keywords_t         *key;
    struct index_data  *index;
    int                 i;
    struct obj_data    *obj;
    char               *objname;
    char                buf[MAX_STRING_LENGTH];
    char                buf2[256],
                        color[10];
    char                temp1[128],
                        temp2[128];
    BalancedBTreeItem_t *item;

    if (GetMaxLevel(ch) < 56) {
        send_to_char("Alas, the report option is only viewable for level "
                     "56 and higher.\n\r", ch);
        return;
    }

    zonenum = skip_spaces(argument);
    zone = -1;
    if( zonenum && isdigit((int)*zonenum) ) {
        zone = atoi(zonenum);
    }

    if ( !zonenum || zone > top_of_zone_table) {
        append_to_string_block(sb, "That is not a valid zone_number\n\r");
        return;
    }

    bottom = zone > 0 ? (zone_table[zone - 1].top + 1) : 0;
    top = zone >= 0 ? zone_table[zone].top : zone_table[top_of_zone_table].top;

    key = StringToKeywords(zonenum, NULL);
    append_to_string_block(&sb, "VNUM  count names\n\r");

    BalancedBTreeLock( objectTree );

    for (item = BalancedBTreeFindLeastInRange( objectTree, LOCKED, &bottom, 
                                               &top );
         item;
         item = BalancedBTreeFindNextInRange( objectTree, item, LOCKED, &bottom,
                                              &top ) ) {

        index = (struct index_data *)item->item;

        if (zone < 0 && !KeywordsMatch(key, &index->keywords)) {
            continue;
        }

        obj = objectRead(index->vnum, VIRTUAL);
        if (obj) {
            sprintbit((unsigned) obj->wear_flags, wear_bits, temp1);
            sprintbit((unsigned) obj->extra_flags, extra_bits, temp2);
            if (index->number - 1 != 0) {
                /*
                 * VNUM; NAME; TYPE; FLAGS; Affect1; Affect2; Affect3;
                 * Affect4
                 */
                objname = KeywordsToString( &index->keywords, " " );
                sprintf(buf, "%d;%ld;%d;%d;%s;%s;%s;", zone, index->vnum, 
                        ((index->number - 1 == 0) ? 0 : 1),
                        index->cost_per_day, objname, temp1, temp2);
                free( objname );
                append_to_string_block(sb, buf);

                switch (ITEM_TYPE(obj)) {
                case ITEM_TYPE_SCROLL:
                case ITEM_TYPE_POTION:
                    sprintf(buf, "Level %d:", obj->value[0]);
                    append_to_string_block(sb, buf);
                    if (obj->value[1] >= 1) {
                        sprinttype(obj->value[1] - 1, spells, buf);
                        sprintf(buf2, "%s", buf);
                        append_to_string_block(sb, buf2);
                    }
                    if (obj->value[2] >= 1) {
                        sprinttype(obj->value[2] - 1, spells, buf);
                        sprintf(buf2, "%s", buf);
                        append_to_string_block(sb, buf2);
                    }
                    if (obj->value[3] >= 1) {
                        sprinttype(obj->value[3] - 1, spells, buf);
                        sprintf(buf2, "%s", buf);
                        append_to_string_block(sb, buf2);
                    }
                    break;
                case ITEM_TYPE_WAND:
                case ITEM_TYPE_STAFF:
                    sprintf(buf, "L:%d spell of:", obj->value[0]);
                    append_to_string_block(sb, buf);
                    if (obj->value[3] >= 1) {
                        sprinttype(obj->value[3] - 1, spells, buf);
                        sprintf(buf2, "%s", buf);
                        append_to_string_block(sb, buf2);
                    }
                    break;
                case ITEM_TYPE_WEAPON:
                    sprintf(buf, "damage:'%dD%d'[%s]", obj->value[1],
                            obj->value[2], AttackType[obj->value[3] /*-1*/ ]);
                    append_to_string_block(sb, buf);
                    break;
                case ITEM_TYPE_ARMOR:
                    sprintf(buf, "AC-apply: %d,", obj->value[0]);
                    append_to_string_block(sb, buf);
                    sprintf(buf, "Size:%d", obj->value[2]);
                    append_to_string_block(sb, buf);
                    break;
                default:
                    append_to_string_block(sb, "None");
                }

                for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                    append_to_string_block(sb, ";");
                    sprinttype(obj->affected[i].location, apply_types, temp1);
                    sprintf(buf, "%s ", temp1);
                    append_to_string_block(sb, buf);

                    switch (obj->affected[i].location) {
                    case APPLY_M_IMMUNE:
                    case APPLY_IMMUNE:
                    case APPLY_SUSC:
                        sprintbit(obj->affected[i].modifier, immunity_names, 
                                  buf2);
                        break;
                    case APPLY_ATTACKS:
                        sprintf(buf2, "%f",
                                (float) (obj->affected[i].modifier / 10));
                        break;
                    case APPLY_WEAPON_SPELL:
                    case APPLY_EAT_SPELL:
                        sprintf(buf2, "%s",
                                spells[obj->affected[i].modifier - 1]);
                        break;
                    case APPLY_SPELL:
                        sprintbit(obj->affected[i].modifier, affected_bits,
                                  buf2);
                        break;
                    case APPLY_BV2:
                    case APPLY_SPELL2:
                        sprintbit(obj->affected[i].modifier, affected_bits2, 
                                  buf2);
                        break;
                    case APPLY_RACE_SLAYER:
                        sprintf(buf2, "%s",
                                races[obj->affected[i].modifier].racename);
                        break;
                    case APPLY_ALIGN_SLAYER:
                        if (obj->affected[i].modifier > 1) {
                            sprintf(buf2, "SLAY GOOD");
                        } else if (obj->affected[i].modifier == 1) {
                            sprintf(buf2, "SLAY NEUTRAL");
                        } else {
                            sprintf(buf2, "SLAY EVIL");
                        }
                        break;
                    default:
                        sprintf(buf2, "%ld", obj->affected[i].modifier);
                        break;
                    }
                    append_to_string_block(sb, buf2);
                }
                append_to_string_block(sb, "\n\r");
            }
            objectExtract(obj);
        }
    }
    BalancedBTreeUnlock(objectTree);
    FreeKeywords(key, TRUE);
}

void do_show_maxxes( struct char_data *ch, struct string_block *sb, 
                     char *argument )
{
    char               *zonenum;
    int                 zone;
    int                 top = 0,
                        bottom = 0;
    Keywords_t         *key;
    struct index_data  *index;
    struct obj_data    *obj;
    int                 i;
    BalancedBTreeItem_t *item;

    zonenum = skip_spaces(argument);
    zone = -1;
    if( zonenum && isdigit((int)*zonenum) ) {
        zone = atoi(zonenum);
    }

    if ( !zonenum || zone > top_of_zone_table) {
        append_to_string_block(sb, "That is not a valid zone_number\n\r");
        return;
    }

    bottom = zone > 0 ? (zone_table[zone - 1].top + 1) : 0;
    top = zone >= 0 ? zone_table[zone].top : zone_table[top_of_zone_table].top;

    key = StringToKeywords(zonenum, NULL);
    append_to_string_block(&sb, "VNUM  count/max names\n\r");

    BalancedBTreeLock( objectTree );
    for (item = BalancedBTreeFindLeastInRange( objectTree, LOCKED, &bottom, 
                                               &top );
         item;
         item = BalancedBTreeFindNextInRange( objectTree, item, LOCKED, &bottom,
                                              &top ) ) {

        index = (struct index_data *)item->item;
        if( zone < 0 && !KeywordsMatch(key, &index->keywords)) {
            continue;
        }

        obj = objectRead(index->vnum, VIRTUAL);
        if (obj && obj->max != 0) {
            objname = KeywordsToString( &index->keywords, " " );
            sprintf(buf, "%5ld  %3d/%3d  %s \n\r", index->vnum, 
                    index->number - 1, index->max, objname);
            free(objname);
            append_to_string_block(sb, buf);
        }
        objectExtract(obj);
    }
    BalancedBTreeUnlock( objectTree );
    FreeKeywords(key, TRUE);
}

typedef struct {
    char    *command;
    void    (*func)(struct char_data *, struct string_block *, char *);
} ShowFunc_t;

static ShowFunc_t showFuncs[] = {
    { "zones", do_show_zones },
    { "objects", do_show_objects },
    { "wearslot", do_show_wearslot },
    { "itemtype", do_show_itemtype },
    { "mobiles", do_show_mobiles },
    { "rooms", do_show_rooms },
    { "report", do_show_report },
    { "maxxes", do_show_maxxes }
};
static int showFuncCount = NELEMENTS(showFuncs);


void do_show(struct char_data *ch, char *argument, int cmd)
{
    struct string_block sb;
    char               *arg1;
    int                 i;

    dlog("in do_show");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg1);
    if( !arg1 ) {
        send_to_char( "Show what?\n\r", ch );
        return;
    }

    init_string_block(&sb);

    for( i = 0; i < showFuncCount; i++ ) {
        if( is_abbrev( arg1, showFunc[i].command ) ) {
            (showFunc[i].func)( ch, &sb, argument );
            break;
        }
    }

    if( i >= showFuncCount ) {
        append_to_string_block(&sb, "Usage:\n\r"
                               "  show zones\n\r"
                               "  show (objects|mobiles|maxxes) "
                               "(zone#|name)\n\r"
                               "  show rooms (zone#|death|private)\n\r"
                               "  show itemtype (#)\n\r"
                               "  show wearslot (#)\n\r");
        if (GetMaxLevel(ch) > 55) {
            append_to_string_block(&sb, "  show report (zone#)\n\r");
        }
    }
    page_string_block(&sb, ch);
    destroy_string_block(&sb);
}


void do_invis(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH + 40];
    int             level;
    char           *arg1;

    dlog("in do_invis");

    if (cmd == 242 && !IS_IMMORTAL(ch)) {
        return;
    }

    if (cmd != 242) {
        if (affected_by_spell(ch, SPELL_INVISIBLE)) {
            affect_from_char(ch, SPELL_INVISIBLE);
        }
        REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
        ch->invis_level = 0;
        return;
    }

    argument = get_argument(argument, &arg1);

    if (arg1) {
        level = atoi(arg1);
        if (level < 0) {
            level = 0;
        }
        if (level > GetMaxLevel(ch)) {
            level = GetMaxLevel(ch);
        }
        ch->invis_level = level;
        sprintf(buf, "Invis level set to %d.\n\r", level);
        send_to_char(buf, ch);
    } else if (ch->invis_level > 0) {
        ch->invis_level = 0;
        do_save(ch, "", 0);
        send_to_char("You are now totally visible.\n\r", ch);
    } else {
        ch->invis_level = IMMORTAL;
        do_save(ch, "", 0);
        send_to_char("You are now invisible to all but gods.\n\r", ch);
    }
}

void do_create(struct char_data *ch, char *argument, int cmd)
{
    int             i,
                    start,
                    end;
    char           *arg1,
                   *arg2;

    dlog("in do_create");

    if (!IS_IMMORTAL(ch) || IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg1);
    argument = get_argument(argument, &arg2);

    if(!arg1 || !arg2) {
        send_to_char(" create <start> <end>\n\r", ch);
        return;
    }


    start = atoi(arg1);
    end   = atoi(arg2);

    if (start > end) {
        send_to_char(" create <start> <end>\n\r", ch);
        return;
    }

    if (cmd != 0) {
        send_to_char("You form much order out of Chaos\n\r", ch);
    }

    for (i = start; i <= end; i++) {
        if (!real_roomp(i)) {
            CreateOneRoom(i);
        }
    }

}

void CreateOneRoom(int loc_nr)
{
    struct room_data *rp;
    extern int      top_of_zone_table;

    char            buf[256];
    int             zone;

    allocate_room(loc_nr);
    rp = real_roomp(loc_nr);
    memset(rp, 0, sizeof(struct room_data));

    rp->number = loc_nr;
    if (top_of_zone_table >= 0) {
        for (zone = 0;
             rp->number > zone_table[zone].top && zone <= top_of_zone_table;
             zone++) {
            /*
             * Empty loop
             */
        }

        if (zone > top_of_zone_table) {
            fprintf(stderr, "Room %ld is outside of any zone.\n", rp->number);
            zone--;
        }
        rp->zone = zone;
    }
    sprintf(buf, "%d", loc_nr);
    rp->name = (char *) strdup(buf);
    rp->description = (char *) strdup("Empty\n");
}

void do_set_log(struct char_data *ch, char *arg, int cmd)
{
    char           *name;
    struct char_data *victim;
    struct obj_data *dummy;

    dlog("in do_set_log");

    if (IS_NPC(ch)) {
        return;
    }

    arg = get_argument(arg, &name);
    if (!name) {
        send_to_char("Usage:log <character>\n\r", ch);
        return;
    }

    if (!generic_find(arg, FIND_CHAR_WORLD, ch, &victim, &dummy)) {
        send_to_char("No such person in the world.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Victim is an NPC.\n\r", ch);
        return;
    }

    if (IS_AFFECTED2(victim, AFF2_LOG_ME)) {
        REMOVE_BIT(victim->specials.affected_by2, AFF2_LOG_ME);
        send_to_char("Log Bit Removed.\n\r", ch);
        return;
    } 
    
    SET_BIT(victim->specials.affected_by2, AFF2_LOG_ME);
    send_to_char("Log Bit Set.\n\r", ch);
}


void do_event(struct char_data *ch, char *arg, int cmd)
{
    int             i;
    char           *buf;

    dlog("in do_event");

    arg = get_argument(arg, &buf);
    if (IS_NPC(ch)) {
        return;
    }
    if (!buf) {
        send_to_char("Event what? (event <mobnum>)\r\n", ch);
        return;
    }

    i = atoi(buf);

    PulseMobiles(i);
}

void do_beep(struct char_data *ch, char *argument, int cmd)
{
    char           *name;
    struct char_data *victim;
    struct obj_data *dummy;

    dlog("in do_beep");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &name);
    if (!name) {
        if (IS_SET(ch->specials.act, PLR_NOBEEP)) {
            send_to_char("Beep now ON.\n\r", ch);
            REMOVE_BIT(ch->specials.act, PLR_NOBEEP);
        } else {
            send_to_char("Beep now OFF.\n\r", ch);
            SET_BIT(ch->specials.act, PLR_NOBEEP);
        }
        return;
    }

    if (!generic_find(name, FIND_CHAR_WORLD, ch, &victim, &dummy)) {
        send_to_char("No such person in the world.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Victim is an NPC.\n\r", ch);
        return;
    }

    if (IS_SET(victim->specials.act, PLR_NOBEEP)) {
        oldSendOutput(ch, "%s can not be beeped right now.\n\r", GET_NAME(victim));
    } else {
        oldSendOutput(victim, "%c%s is beeping you.\n\r", 7, GET_NAME(ch));
        oldSendOutput(ch, "%s has been beeped.\n\r", GET_NAME(victim));
    }
}

void do_cset(struct char_data *ch, char *arg, int cmd)
{
    char           *buf1,
                   *buf2,
                   *buf3,
                   *buf4;
    int             i,
                    radix;
    NODE           *n;
    extern struct radix_list radix_head[];
    extern byte     HashTable[];

    dlog("in do_cset");

    if (IS_NPC(ch)) {
        return;
    }
    arg = get_argument(arg, &buf1);
    arg = get_argument(arg, &buf2);
    arg = get_argument(arg, &buf3);
    arg = get_argument(arg, &buf4);

    if( !buf1 ) {
        send_to_char("Usage: # <\"set\" | \"show\" | \"log\"> <cmd> <\"level\" "
                     "| \"position\"> <level>\n\r", ch);
        return;
    }

    if (!strcmp(buf1, "show") && buf2) {
        radix = HashTable[(int) *buf2];
        if (!radix_head[radix].next) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }

        n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
        if (!n) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }
        oldSendOutput(ch, "Name: %s\n\rMinimum Position: %d\n\rMinimum Level: "
                      "%d\n\rNumber: %d\n\rLog Bit: %s\n\r",
                      n->name, n->min_pos, n->min_level, n->number,
                      (n->log ? "On" : "Off"));
        return;
    } 
    
    if (!strcmp(buf1, "set") && buf2 && buf3 && buf4) {
        i = atoi(buf4);
        radix = HashTable[(int) *buf2];
        if (!radix_head[radix].next) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }

        n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
        if (!n) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }

        if (!strcmp(buf3, "level")) {
            if (i < 0 || i > 60) {
                send_to_char("Level must be between 0 and 60.\n\r", ch);
                return;
            }

            n->min_level = i;
            send_to_char("Level Changed.\n\r", ch);
            return;
        }

        if (!strcmp(buf3, "position")) {
            if (i < 0 || i > 10) {
                send_to_char("Position must be between 0 and 10.\n\r", ch);
                return;
            }

            n->min_pos = i;
            send_to_char("Position Changed.\n\r", ch);
            return;
        }
    } else if (!strcmp(buf1, "log") && buf2) {
        radix = HashTable[(int) *buf2];
        if (!radix_head[radix].next) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }

        n = SearchForNodeByName(radix_head[radix].next, buf2, strlen(buf2));
        if (!n) {
            send_to_char("Sorry, command not found.\n\r", ch);
            return;
        }

        if (n->log) {
            send_to_char("Command logging removed.\n\r", ch);
            n->log = 0;
            return;
        } else {
            send_to_char("Command logging set.\n\r", ch);
            n->log = 1;
            return;
        }
    }
    send_to_char("Usage: # <\"set\" | \"show\" | \"log\"> <cmd> <\"level\" "
                 "| \"position\"> <level>\n\r", ch);
}


void do_disconnect(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    struct descriptor_data *d;
    struct char_data *victim;
    
    dlog("in do_disconnect");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Disconnect whom?(discon <name>)\n\r", ch);
        return;
    }

#if 0
    if(!(victim = get_char(arg))) {
        send_to_char("No-one by that name in the world.\n\r",ch); return;
    }
    if ( victim->desc == NULL ) {
        act("$N doesn't have a descriptor.", 0, ch, 0, victim, TO_CHAR);
        return;
    }
#endif

    for (d = descriptor_list; d != NULL; d = d->next) {
        victim = d->character;
        if (d->character) {
            if (GET_NAME(d->character) &&
                (strcasecmp(GET_NAME(d->character), arg) == 0)) {

                if ((GetMaxLevel(victim) > 51) && !(ch == victim)) {
                    Log("%s just disconnected %s!", GET_NAME(ch),
                        GET_NAME(victim));
                }
                close_socket(d);
                send_to_char("Ok.\n\r", ch);
                return;
            }
        }
    }

    Log("Descriptor not found, do_disconnect");
    send_to_char("Descriptor not found!\n\r", ch);
}

void do_freeze(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    struct char_data *victim;

    dlog("in do_freeze");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);

    if (!arg) {
        send_to_char("Freeze whom?(freeze <name>)\n\r", ch);
        return;
    }

    if (!(victim = get_char(arg))) {
        send_to_char("No-one by that name in the world.\n\r", ch);
        return;
    }

    if (!IS_PC(victim) && !IS_SET(victim->specials.act, ACT_POLYSELF)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
        send_to_char("You can't freeze them!\n\r", ch);
        return;
    } else {
        if (IS_SET(victim->specials.act, PLR_FREEZE)) {
            REMOVE_BIT(victim->specials.act, PLR_FREEZE);
            send_to_char("You can play again.\n\r", victim);
            send_to_char("FREEZE removed.\n\r", ch);
        } else {
            SET_BIT(victim->specials.act, PLR_FREEZE);
            send_to_char("You can't do ANYthing!\n\r", victim);
            send_to_char("FREEZE set.\n\r", ch);
        }
        do_save(victim, "", 0);
        return;
    }
}

void do_drainlevel(struct char_data *ch, char *argument, int cmd)
{
    char           *arg1,
                   *arg2;
    int             numtolose,
                    i;
    struct char_data *victim;

    dlog("in do_drainlevel");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     * victim name
     */
    argument = get_argument(argument, &arg1);

    /*
     * levels to drain
     */
    argument = get_argument(argument, &arg2);

    if (!arg1 || !arg2) {
        send_to_char("Drain levels from whom? (drain <name> "
                     "<numbertodrain>)\n\r", ch);
        return;
    }

    if (!(victim = get_char(arg1))) {
        send_to_char("No-one by that name in the world.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
        send_to_char("You can't drain them!!\n\r", ch);
        oldSendOutput(victim, "%s tried to drain levels from you!\n\r",
                          GET_NAME(ch));
        return;
    }

    numtolose = atoi(arg2);
    send_to_char( "You are struck by a black beam from above, it hurts!\n\r"
        "The life force from your body fades and you feel yourself lose\n\r"
        "memories of old times and battles.\n\r"
        "The feeling fades and you shiver at a cold gust of wind.\n\r",
        victim);

    oldSendOutput(ch, "You drained %d level(s). How evil!\n\r", numtolose);

    for (i = 0; i <= numtolose - 1; i++) {
        if (GetMaxLevel(victim) <= 1) {
            i = numtolose;
            send_to_char("\n\rTried to lower them below 1, can't do that.\n\r",
                         ch);
        } else {
            drop_level(victim, BestClassIND(victim), TRUE);
            send_to_char(".", ch);
        }
    }

    /*
     * all done, save the mess!
     */
    send_to_char("\n\rOk.\r\n", ch);
    do_save(victim, "", 0);
}

void do_god_interven(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;

    dlog("in do_god_interven");

    argument = get_argument(argument, &arg);

    if (IS_NPC(ch)) {
        return;
    }

    if (!arg) {
        send_to_char("Eh? What do you wanna intervene upon?\n\r"
                     "interven <portal|summon|astral|locobj|zonelocate|kill|"
                     "logall|eclipse|dns|color|wizlock|nopoly|req|rp|"
                     "worldarena|deinit|tweak>\n\r", ch);
        return;
    }

    if (!strcmp("eclipse", arg)) {
        if (IS_SET(SystemFlags, SYS_ECLIPS)) {
            REMOVE_BIT(SystemFlags, SYS_ECLIPS);
            send_to_char("You part the planets and the sun shines through!\n",
                         ch);
            send_to_outdoor("The planets return to their normal orbit, slowly"
                            " the light will return.\n");
            Log("The world is enlightend");
        } else {
            SET_BIT(SystemFlags, SYS_ECLIPS);
            weather_info.sunlight = SUN_DARK;
            switch_light(SUN_DARK);
            send_to_char("You summon the planets and force an eclipse!\n", ch);
            send_to_outdoor("The planets eclipse and hide the sun spreading "
                            "darkness through out the land!\n");
            Log("World has been darkened");
        }
    } else if (!strcmp("req", arg)) {
        if (!IS_SET(SystemFlags, 128)) {
            SET_BIT(SystemFlags, 128);
            send_to_char("Newbie character approval required.\n\r", ch);
            Log("New character approval REQUIRED");
        } else {
            REMOVE_BIT(SystemFlags, 128);
            send_to_char("Newbie character approval REMOVED.\n\r", ch);
            Log("New character approval REMOVED");
        }
    } else if (!strcmp("color", arg)) {
        if (!IS_SET(SystemFlags, SYS_NOANSI)) {
            SET_BIT(SystemFlags, SYS_NOANSI);
            send_to_char("Color codes disabled world wide.\n\r", ch);
            Log("Global colors disabled");
        } else {
            REMOVE_BIT(SystemFlags, SYS_NOANSI);
            send_to_char("Color codes enabled for everyone that uses "
                         "them.\n\r", ch);
            Log("Global colors enabled");
        }
    } else if (!strcmp("dns", arg)) {
        if (IS_SET(SystemFlags, SYS_SKIPDNS)) {
            REMOVE_BIT(SystemFlags, SYS_SKIPDNS);
            send_to_char("Domain name searches enabled.\n\r", ch);
            Log("DNS Enabled");
        } else {
            SET_BIT(SystemFlags, SYS_SKIPDNS);
            send_to_char("Domain name searches Disabled.\n\r", ch);
            Log("DNS Disabled");
        }
    } else if (!strcmp("portal", arg)) {
        if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
            REMOVE_BIT(SystemFlags, SYS_NOPORTAL);
            send_to_char("You sort out the planes and allow portaling.\n", ch);
            Log("Portaling enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NOPORTAL);
            send_to_char("You scramble the planes to make portaling "
                         "impossible.\n", ch);
            Log("Portaling disabled");
        }
    } else if (!strcmp("astral", arg)) {
        if (IS_SET(SystemFlags, SYS_NOASTRAL)) {
            REMOVE_BIT(SystemFlags, SYS_NOASTRAL);
            send_to_char("You shift the planes and allow astral travel.\n", ch);
            Log("Astral enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NOASTRAL);
            send_to_char("You shift the astral planes and make astral travel "
                         "impossible.\n", ch);
            Log("Astral disabled");
        }
    } else if (!strcmp("summon", arg)) {
        if (IS_SET(SystemFlags, SYS_NOSUMMON)) {
            REMOVE_BIT(SystemFlags, SYS_NOSUMMON);
            send_to_char("You clear the fog to enable summons.\n", ch);
            Log("Summons enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NOSUMMON);
            send_to_char("A magical fog spreads throughout the land making "
                         "summons impossible.\n", ch);
            Log("Summons disabled");
        }
    } else if (!strcmp("kill", arg)) {
        if (IS_SET(SystemFlags, SYS_NOKILL)) {
            REMOVE_BIT(SystemFlags, SYS_NOKILL);
            send_to_char("You let the anger lose inside you and the people of"
                         " the land fight.\n", ch);
            Log("Killing enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NOKILL);
            send_to_char("You spread thoughts of peace through the people of "
                         "the land.\n", ch);
            Log("Killing disabled");
        }
    } else if (!strcmp("logall", arg)) {
        if (IS_SET(SystemFlags, SYS_LOGALL)) {
            REMOVE_BIT(SystemFlags, SYS_LOGALL);
            send_to_char("You fire the scribe writting the history for poor "
                         "workmanship.\n\r", ch);
            Log("Logging all disabled.");
        } else {
            SET_BIT(SystemFlags, SYS_LOGALL);
            send_to_char("You hire a scribe to write the history of the "
                         "world.\n\r", ch);
            Log("Logging all enabled");
        }
    } else if (!strcmp("wizlock", arg)) {
        if (IS_SET(SystemFlags, SYS_WIZLOCKED)) {
            REMOVE_BIT(SystemFlags, SYS_WIZLOCKED);
            send_to_char("You open the world to the mortals\n\r", ch);
            Log("System is no longer wizlocked");
        } else {
            SET_BIT(SystemFlags, SYS_WIZLOCKED);
            send_to_char("Only the Gods will be able to log on\n\r", ch);
            Log("System is now Wizlocked");
            send_to_all("$c0009System is now locked for repair! Please log "
                        "out!");
        }
    } else if (!strcmp("nopoly", arg)) {
        if (IS_SET(SystemFlags, SYS_NO_POLY)) {
            REMOVE_BIT(SystemFlags, SYS_NO_POLY);
            send_to_char("All people may now polymorph\n\r", ch);
            Log("Polymorph is now permitted");
        } else {
            SET_BIT(SystemFlags, SYS_NO_POLY);
            send_to_char("People now can't polymorph\n\r", ch);
            Log("Polymorphing powers have been removed");
        }
    } else if (!strcmp("rp", arg)) {
        if (IS_SET(SystemFlags, SYS_NOOOC)) {
            REMOVE_BIT(SystemFlags, SYS_NOOOC);
            send_to_char("OOC channel re-enabled\n\r", ch);
            Log("OOC channel has been re-enabled");
            send_to_all("OOC channel has been re-enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NOOOC);
            send_to_char("OOC channel disabled, RP is now strictly "
                         "enforced\n\r", ch);
            Log("OOC channel disabled");
            send_to_all("The power of the OOC channel has been disabled...");
            send_to_all("RPing is now enforced on every channel");
        }
    } else if (!strcmp("locobj", arg)) {
        if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
            REMOVE_BIT(SystemFlags, SYS_LOCOBJ);
            send_to_char("Locate Object enabled\n\r", ch);
            Log("Locate object has been enabled");
        } else {
            SET_BIT(SystemFlags, SYS_LOCOBJ);
            send_to_char("Locate Object disabled\n\r", ch);
            Log("Locate Object disabled");
        }
    } else if (!strcmp("worldarena", arg)) {
        if (IS_SET(SystemFlags, SYS_WLD_ARENA)) {
            REMOVE_BIT(SystemFlags, SYS_WLD_ARENA);
            send_to_char("World arena disabled\n\r", ch);
            Log("World Arena has been disabled");
        } else {
            SET_BIT(SystemFlags, SYS_WLD_ARENA);
            send_to_char("World Arena enabled\n\r", ch);
            Log("World Arena enabled");
        }
    } else if (!strcmp("deinit", arg)) {
        if (IS_SET(SystemFlags, SYS_NO_DEINIT)) {
            REMOVE_BIT(SystemFlags, SYS_NO_DEINIT);
            send_to_char("Deinit zones enabled\n\r", ch);
            Log("Deinit Zones has been enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NO_DEINIT);
            send_to_char("Deinit zones disabled\n\r", ch);
            Log("Deinit zones disabled");
        }
    } else if (!strcmp("tweak", arg)) {
        if (IS_SET(SystemFlags, SYS_NO_TWEAK)) {
            REMOVE_BIT(SystemFlags, SYS_NO_TWEAK);
            send_to_char("Tweaking items enabled\n\r", ch);
            Log("Tweaking items have been enabled");
        } else {
            SET_BIT(SystemFlags, SYS_NO_TWEAK);
            send_to_char("Tweaking items disabled\n\r", ch);
            Log("Tweaking items disabled");
        }
    } else if (!strcmp("zonelocate", arg)) {
        if (IS_SET(SystemFlags, SYS_ZONELOCATE)) {
            REMOVE_BIT(SystemFlags, SYS_ZONELOCATE);
            send_to_char("Zone only locating has been enabled\n\r", ch);
            Log("Zone only locating has been disabled");
        } else {
            SET_BIT(SystemFlags, SYS_ZONELOCATE);
            send_to_char("Zone only locating has been enabled\n\r", ch);
            Log("Zone only locating has been enabled");
        }
    } else {
        send_to_char("Godly powers you have, but how do you wanna use them?\n",
                     ch);
    }
}

void do_nuke(struct char_data *ch, char *argument, int cmd)
{
    int             i;
    struct char_file_u ch_st;
    struct char_data *victim;
    FILE           *char_file;
    char            buf[254],
                   *arg;

    dlog("in do_nuke");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);

    if (!arg) {
        send_to_char("Nuke whom?! (nuke <name>)\n\r", ch);
        return;
    }

    if (!(victim = get_char(arg))) {
        send_to_char("No-one by that name in the world.\n\r", ch);
        return;
    }

    if (victim->in_room != ch->in_room) {
        send_to_char("That person is not in the same room as you.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
        send_to_char("You can't nuke them!!\n\r", ch);
        oldSendOutput(victim, "%s tried to nuke you!\n\r", GET_NAME(ch));
        return;
    } 
    
    Log("%s just nuked %s!", GET_NAME(ch), GET_NAME(victim));
    act("$n calls forth the wrath of the gods and destroys $N!", FALSE,
        ch, 0, victim, TO_NOTVICT);
    act("$n reaches into $N and pulls out a fighting soul!", FALSE, ch,
        0, victim, TO_NOTVICT);
    act("$N dies quickly without much a fight.", FALSE, ch, 0, victim,
        TO_NOTVICT);
    act("$n reaches into your chest and pulls your soul out, you die "
        "forever!", FALSE, ch, 0, victim, TO_VICT);
    act("You rip the heart and soul from $N condeming $M to instant "
        "death.", FALSE, ch, 0, victim, TO_CHAR);
    for (i = 0; i <= top_of_p_table; i++) {
        if (!strcasecmp(player_table[i].name, GET_NAME(victim))) {
            if (player_table[i].name) {
                free(player_table[i].name);
            }
            player_table[i].name = (char *) malloc(strlen("111111"));
            strcpy(player_table[i].name, "111111");
            break;
        }
    }
    /*
     * get the structure from player_table[i].nr
     */
    if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
        perror("Opening player file for updating. act.wizard.c");
        assert(0);
    }
    rewind(char_file);
    fseek(char_file,
          (long)(player_table[i].nr * sizeof(struct char_file_u)), 0);

    /*
     * read in the char, change the name, write back
     */
    fread(&ch_st, sizeof(struct char_file_u), 1, char_file);
    sprintf(ch_st.name, "111111");
    rewind(char_file);
    fseek(char_file,
          (long) (player_table[i].nr * sizeof(struct char_file_u)), 0);
    fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
    fclose(char_file);

    close_socket(victim->desc);
    sprintf(buf, "rent/%s", lower(GET_NAME(victim)));
    remove(buf);
    sprintf(buf, "rent/%s.aux", GET_NAME(victim));
    remove(buf);
    do_purge(ch, GET_NAME(victim), 0);
    send_to_char("Nuked.\n\r", ch);
}

void do_force_rent(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    struct char_data *victim;
    struct obj_cost cost;


    dlog("in do_forcerent");
    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);

    if (!arg) {
        send_to_char("Force rent whom? (forcerent <name>|<alldead>)\n\r", ch);
        return;
    }

    if (!strcmp(arg, "alldead")) {
        for (victim = character_list; victim; victim = victim->next) {
            if (IS_LINKDEAD(victim) &&
                !IS_SET(victim->specials.act, ACT_POLYSELF)) {
                if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
                    if (CAN_SEE(ch, victim)) {
                        send_to_char("You can't forcerent them!\n\r", ch);
                    }
                } else {
                    do_save(victim, "", 0);
                    if (victim->in_room != NOWHERE) {
                        char_from_room(victim);
                    }
                    char_to_room(victim, 4);
                    if (victim->desc) {
                        close_socket(victim->desc);
                    }
                    victim->desc = 0;
                    if (recep_offer(victim, NULL, &cost, TRUE)) {
                        cost.total_cost = 100;
                        save_obj(victim, &cost, 1);
                    } else {
                        Log("%s had a failed recp_offer, they are losing EQ!",
                            GET_NAME(victim));
                    }
                    extract_char(victim);
                }
            }
        }
        return;
    }

    /*
     * alldead
     */
    if (!(victim = get_char(arg))) {
        send_to_char("No-one by that name in the world.\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Not on NPC's.\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
        send_to_char("You can't forcerent them!\n\r", ch);
        return;
    } 
    
    if (victim->in_room != NOWHERE) {
        char_from_room(victim);
    }
    char_to_room(victim, 4);
    if (victim->desc) {
        close_socket(victim->desc);
    }

    victim->desc = 0;

    if (recep_offer(victim, NULL, &cost, TRUE)) {
        cost.total_cost = 100;
        save_obj(victim, &cost, 1);
    } else {
        Log("%s had a failed recp_offer, they are losing EQ!",
            GET_NAME(victim));
    }
    extract_char(victim);
}

void do_ghost(struct char_data *ch, char *argument, int cmd)
{
    extern int      plr_tick_count;
    char           *find_name;
    int             player_i;
    struct char_file_u tmp_store;
    struct char_data *tmp_ch,
                   *vict;

    dlog("in do_ghost");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &find_name);
    if (!find_name) {
        send_to_char("Ghost play who?? (ghost <name>)\n\r", ch);
        return;
    }

    if ((vict = get_char(find_name))) {
        send_to_char("Person is online, cannot control the living.\n\r", ch);
        return;
    }

    if ((player_i = load_char(find_name, &tmp_store)) > -1) {
        CREATE(tmp_ch, struct char_data, 1);
        clear_char(tmp_ch);
        store_to_char(&tmp_store, tmp_ch);

        reset_char(tmp_ch);
        load_char_objs(tmp_ch);
        save_char(tmp_ch, AUTO_RENT);
        tmp_ch->next = character_list;
        character_list = tmp_ch;
        tmp_ch->specials.tick = plr_tick_count++;
        if (plr_tick_count == PLR_TICK_WRAP) {
            plr_tick_count = 0;
        }
        char_to_room(tmp_ch, ch->in_room);
        tmp_ch->desc = NULL;

        act("$n calls forth the soul of $N and they come.", FALSE, ch, 0,
            tmp_ch, TO_ROOM);
        act("The soul of $N rises forth from the mortal lands.", FALSE, ch,
            0, tmp_ch, TO_ROOM);

        act("You call forth the soul of $N.", FALSE, ch, 0, tmp_ch, TO_CHAR);
        send_to_char("Be sure to forcerent them when done!\n\r", ch);
    } else {
        send_to_char("That person does not exist.\n\r", ch);
    }
}

void do_mforce(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name,
                   *to_force,
                    buf[100];

    dlog("in do_mforce");

    if (IS_NPC(ch) && (cmd != 0)) {
        return;
    }

    argument = get_argument(argument, &name);
    to_force = skip_spaces(argument);

    if (!name || !to_force) {
        send_to_char("Who do you wish to force to do what?\n\r", ch);
    } else if (strcasecmp("all", name)) {
        if (!(vict = get_char_room_vis(ch, name))) {
            send_to_char("No-one by that name here..\n\r", ch);
        } else if (IS_PC(vict)) {
            send_to_char("Oh no you don't!!\n\r", ch);
        } else {
            sprintf(buf, "$n has forced you to '%s'.", to_force);
            act(buf, FALSE, ch, 0, vict, TO_VICT);
            send_to_char("Ok.\n\r", ch);
            command_interpreter(vict, to_force);
        }
    } else {
        /*
         * force all
         */
        for (vict = real_roomp(ch->in_room)->people; vict;
             vict = vict->next_in_room) {
            if (vict != ch && !IS_PC(vict)) {
                sprintf(buf, "$n has forced you to '%s'.", to_force);
                act(buf, FALSE, ch, 0, vict, TO_VICT);
                command_interpreter(vict, to_force);
            }
        }
        send_to_char("Ok.\n\r", ch);
    }
}


void do_clone(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *mob,
                   *mcopy;
    struct obj_data *obj,
                   *ocopy;
    char           *type,
                   *name,
                   *buf;
    int             j,
                    i,
                    count,
                    where;

    dlog("in do_clone");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &type);
    if (!type) {
        send_to_char("usage: Clone <mob/obj> name [count]\r\n", ch);
        return;
    }

    argument = get_argument(argument, &name);
    if (!name) {
        send_to_char("usage: Clone <mob/obj> name [count]\r\n", ch);
        return;
    }

    argument = get_argument(argument, &buf);
    if (!buf) {
        count = 1;
    } else {
        count = atoi(buf);
    }

    if (!count || count > 20) {
        send_to_char("invalid Count\r\n", ch);
        return;
    }

    if (is_abbrev(type, "mobile")) {
        if ((mob = get_char_room_vis(ch, name)) == 0) {
            send_to_char("Can't find such mobile\r\n", ch);
            return;
        }
        if (IS_PC(mob)) {
            send_to_char("Cloning PC, hahahaha\t\n", ch);
            oldSendOutput(mob, "%s just tried to clone you...", GET_NAME(ch));
            return;
        }
        if (mob->nr < 0) {
            send_to_char("You can't clone it\r\n", ch);
            return;
        }
        for (i = 0; i < count; i++) {
            mcopy = read_mobile(mob->nr, REAL);
            /*
             * clear
             */
            if (mcopy->player.name) {
                free(mcopy->player.name);
            }
            if (mcopy->player.short_descr) {
                free(mcopy->player.short_descr);
            }
            if (mcopy->player.long_descr) {
                free(mcopy->player.long_descr);
            }
            if (mcopy->player.description) {
                free(mcopy->player.description);
            }
            /*
             * copy
             */
            if (mob->player.name) {
                mcopy->player.name = strdup(mob->player.name);
            }
            if (mob->player.short_descr) {
                mcopy->player.short_descr =
                    strdup(mob->player.short_descr);
            }
            if (mob->player.long_descr) {
                mcopy->player.long_descr = strdup(mob->player.long_descr);
            }
            if (mob->player.description) {
                mcopy->player.description =
                    strdup(mob->player.description);
            }

            /*
             * clone EQ equiped
             */
            if (mob->equipment) {
                for (j = 0; j < MAX_WEAR; j++) {
                    if (mob->equipment[j]) {
                        /*
                         * clone mob->equipment[j]
                         */
                        ocopy = objectClone(mob->equipment[j]);
                        if (mob->equipment[j]->contains) {
                            objectCloneContainer(ocopy, mob->equipment[j]);
                        }
                        equip_char(mcopy, ocopy, j);
                    }
                }
            }
            /*
             * clone EQ carried
             */
            if (mob->carrying) {
                for (obj = mob->carrying; obj; obj = obj->next_content) {
                    ocopy = objectClone(obj);
                    if (obj->contains) {
                        objectCloneContainer(ocopy, obj);
                    }
                    /*
                     * move obj to cloned mobs carrying
                     */
                    objectGiveToChar(ocopy, mcopy);
                }
            }

            /*
             * put
             */
            char_to_room(mcopy, ch->in_room);
            act("$n has cloned $N!", FALSE, ch, 0, mob, TO_ROOM);
            act("You clone $N.", FALSE, ch, 0, mob, TO_CHAR);
        }
    } else if (is_abbrev(type, "object")) {
        if ((obj = get_obj_in_list_vis(ch, name, ch->carrying)) != NULL) {
            where = 1;
        } else if ((obj = get_obj_in_list_vis(ch, name,
                                      real_roomp(ch->in_room)->contents)) ) {
            where = 2;
        } else {
            send_to_char("Can't find such object\r\n", ch);
            return;
        }
        if (obj->item_number < 0) {
            send_to_char("You can't clone that object\r\n", ch);
            return;
        }
        for (i = 0; i < count; i++) {
            ocopy = objectClone(obj);
            if (obj->contains) {
                objectCloneContainer(ocopy, obj);
            }
            /*
             * put
             */
            if (where == 1) {
                objectGiveToChar(ocopy, ch);
            } else {
                objectPutInRoom(ocopy, ch->in_room);
            }
            act("$n has cloned $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You cloned $p.", FALSE, ch, obj, 0, TO_CHAR);
        }
    } else {
        send_to_char("usage: Clone <mob/obj> name [count]\r\n", ch);
    }
}

/**
 * @todo db_get_report now returns ALL the results.  rewrite
 */
char *view_newhelp(void)
{
    struct user_report *report;
    char               *buf;
    char                buf2[MAX_STRING_LENGTH];
    int                 length;
    int                 lengthAvail;
    int                 done;

    buf = NULL;
    length = 0;
    lengthAvail = 0;
    report = NULL;

    for( done = 0; !done; ) {
        report = db_get_report( REPORT_HELP, report );
        if( !report ) {
            done = 1;
            continue;
        }

        sprintf( buf2, "**%s: help %s\n\r", report->character, report->report );
        while( length + strlen(buf2) >= lengthAvail ) {
            /* Need to grow the buffer */
            lengthAvail += MAX_STRING_LENGTH;
            buf = (char *)realloc(buf, lengthAvail);
            if( !buf ) {
                Log("Out of memory in view_newhelp!");
                return(NULL);
            }
        }

        buf[length] = '\0';
        strcat(buf, buf2);
        length += strlen(buf2);
    }

    return( buf );
}

/**
 * @todo db_get_report now returns ALL the results.  rewrite
 */
char *view_report(int reportId)
{
    struct user_report *report;
    char               *buf;
    char                buf2[MAX_STRING_LENGTH];
    int                 length;
    int                 lengthAvail;
    int                 done;

    buf = NULL;
    length = 0;
    lengthAvail = 0;
    report = NULL;

    for( done = 0; !done; ) {
        report = db_get_report( reportId, report );
        if( !report ) {
            done = 1;
            continue;
        }

        sprintf( buf2, "**%s[%d]: %s\n\r", report->character, report->roomNum,
                       report->report );
        while( length + strlen(buf2) >= lengthAvail ) {
            /* Need to grow the buffer */
            lengthAvail += MAX_STRING_LENGTH;
            buf = (char *)realloc(buf, lengthAvail);
            if( !buf ) {
                Log("Out of memory in view_report!");
                return(NULL);
            }
        }

        buf[length] = '\0';
        strcat(buf, buf2);
        length += strlen(buf2);
    }

    return( buf );
}

void do_viewfile(struct char_data *ch, char *argument, int cmd)
{
    char           *namefile;
    char           *buf;

    /*
     * extern char titlescreen[MAX_STRING_LENGTH];
     */

    dlog("in do_viewfile");

    argument = get_argument(argument, &namefile);
    if( !namefile ) {
        send_to_char("Commands: view <bug|typo|idea|mortbug|morttypo|"
                     "mortidea|motd|wmotd|help>.\n\r", ch);
        return;
    }

    if (!strcmp(namefile, "help")) {
        buf = view_newhelp();
    } else if (!strcmp(namefile, "quest")) {
        buf = view_report(REPORT_QUEST);
    } else if (!strcmp(namefile, "bug")) {
        buf = view_report(REPORT_WIZBUG);
    } else if (!strcmp(namefile, "idea")) {
        buf = view_report(REPORT_WIZIDEA);
    } else if (!strcmp(namefile, "typo")) {
        buf = view_report(REPORT_WIZTYPO);
    } else if (!strcmp(namefile, "morttypo")) {
        if (!IS_SET(ch->specials.act, PLR_WIZREPORT)) {
            send_to_char("You do not have the power to do this", ch);
            return;
        }
        buf = view_report(REPORT_TYPO);
    } else if (!strcmp(namefile, "mortbug")) {
        if (!IS_SET(ch->specials.act, PLR_WIZREPORT)) {
            send_to_char("You do not have the power to do this", ch);
            return;
        }
        buf = view_report(REPORT_BUG);
    } else if (!strcmp(namefile, "mortidea")) {
        if (!IS_SET(ch->specials.act, PLR_WIZREPORT)) {
            send_to_char("You do not have the power to do this", ch);
            return;
        }
        buf = view_report(REPORT_IDEA);
    } else if (!strcmp(namefile, "motd")) {
        send_to_char(motd, ch);
        return;
    }
    else if (!strcmp(namefile, "wmotd")) {
        send_to_char(wmotd, ch);
        return;
    } else {
        send_to_char("Commands: view <bug|typo|idea|mortbug|morttypo|"
                     "mortidea|motd|wmotd|help>.\n\r", ch);
        return;
    }

    if( buf ) {
        page_string(ch->desc, buf, 1);
        free( buf );
    }
}

void do_msave(struct char_data *ch, char *argument, int cmd)
{
    FILE           *f;
    struct char_data *mob;
    char           *mname,
                   *field,
                   *field2,
                    buf[254];
    long            vnum = -1;
    int             nr;
    long            start,
                    end;

    dlog("in do_msave");

    if (!IS_IMMORTAL(ch)) {
        return;
    }

    argument = get_argument(argument, &mname);
    if (!mname) {
        send_to_char("Msave <mobile name> <new_vnum>\n\r", ch);
        return;
    }

    argument = get_argument(argument, &field);
    if (!field) {
        send_to_char("Msave <mobile name> <vnum>\n\r", ch);
        return;
    }

    argument = get_argument(argument, &field2);
    if (field2) {
        send_to_char("Msave changed. Set hitpoints in medit. Msave <mobile "
                     "name> <vnum>\n\r", ch);
        return;
    }

    if (!(mob = get_char_room_vis(ch, mname))) {
        send_to_char("Hum, I do not know where it is?!?!?\n\r", ch);
        return;
    }

    vnum = atol(field);
    if (vnum < 1 || vnum > 99999) {
        send_to_char("Invalid VNUM\n\r", ch);
        return;
    }
    /*
     * check for valid VNUM in this zone
     */

    if (GetMaxLevel(ch) <= GOD && GET_ZONE(ch) == 0) {
        send_to_char("Sorry, you do not have access to a zone.\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) <= GOD) {
        start = GET_ZONE(ch) ? (zone_table[GET_ZONE(ch) - 1].top + 1) : 0;
        end = zone_table[GET_ZONE(ch)].top;
        if (vnum > end) {
            send_to_char("VNUM is larger than your zone allows.\n\r", ch);
            return;
        }
        if (vnum < start) {
            send_to_char("VNUM is smaller than your zone allows.\n\r", ch);
            return;
        }
    }

    /*
     * check for valid VNUM period
     */
    if ((nr = real_mobile(vnum)) != -1) {
        send_to_char("WARNING: Vnum already in use, OVER-WRITING\n\r", ch);
    }
    sprintf(buf, "mobiles/%ld", vnum);
    if ((f = fopen(buf, "wt")) == NULL) {
        send_to_char("Can't write to disk now..try later.\n\r", ch);
        return;
    }

    fprintf(f, "#%ld\n", vnum);
    write_mob_to_file(mob, f);
    fclose(f);
    if (nr == -1) {
        insert_mobile(mob, vnum);
    } else {
        mob_index[nr].pos = -1;
    }
    Log("Mobile %s saved as vnum %ld", mob->player.name, vnum);

    oldSendOutput(ch, "Mobile %s saved as vnum %ld\n\r", mob->player.name, vnum);
}

void do_osave(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct index_data *index;
    char           *oname,
                   *field;
    long            vnum = -1;
    int             nr;
    long            start,
                    end;

    dlog("in do_osave");

    if (!IS_IMMORTAL(ch)) {
        return;
    }

    argument = get_argument(argument, &oname);
    if (!oname) {
        send_to_char("Osave <object name> <new_vnum>\n\r", ch);
        return;
    }

    argument = get_argument(argument, &field);
    if (!field) {
        send_to_char("Osave <object name> <vnum>\n\r", ch);
        return;
    }

    if (!(obj = get_obj_vis_accessible(ch, oname))) {
        send_to_char("Hum, I do not know where that is?!?!?\n\r", ch);
        return;
    }

    vnum = atol(field);
    if (vnum < 1 || vnum > 99999) {
        send_to_char("Invalid object number\n\r", ch);
        return;
    }

    /*
     * check for valid VNUM in this zone
     */

    if (GetMaxLevel(ch) <= GOD && GET_ZONE(ch) == 0) {
        send_to_char("Sorry, you do not have access to a zone.\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) <= GOD) {
        start = GET_ZONE(ch) ? (zone_table[GET_ZONE(ch) - 1].top + 1) : 0;
        end = zone_table[GET_ZONE(ch)].top;
        if (vnum > end) {
            send_to_char("VNum is larger than your zone allows.\n\r", ch);
            return;
        }
        if (vnum < start) {
            send_to_char("VNUM is smaller than your zone allows.\n\r", ch);
            return;
        }
    }

    /*
     * check for valid VNUM period
     */
    index = objectIndex(vnum);
    if (index) {
        send_to_char("WARNING: Vnum already in use, OVER-WRITING\n\r", ch);
    }

    if (obj->modBy) {
        free(obj->modBy);
    }
    obj->item_number = vnum;
    obj->modBy = (char *) strdup(GET_NAME(ch));
    obj->modified = time(0);

    if (!index) {
        objectInsert(obj, vnum);
    }
    db_save_object(obj, 0, -1, -1, -1);

    Log("Object %s saved as vnum %ld", obj->name, vnum);
    oldSendOutput(ch, "Object %s saved as vnum %ld\n\r", obj->name, vnum);
}

void do_home(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *v = NULL;

    dlog("in do_home");

    if (ch->player.hometown > 0) {
        if (IS_SET(ch->specials.act, PLR_STEALTH)) {
            for (v = real_roomp(ch->in_room)->people; v;
                 v = v->next_in_room) {
                if ((ch != v) && (CAN_SEE(v, ch))) {
                    if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) ||
                        !ch->specials.poofout) {
                        act("A whirling vortex envelopes $n and $e is gone.",
                            FALSE, ch, 0, v, TO_VICT);
                    } else {
                        act(ch->specials.poofout, FALSE, ch, 0, v, TO_VICT);
                    }
                }
            }
        } else {
            if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) ||
                !ch->specials.poofout) {
                act("A whirling vortex envelopes $n and $e is gone.",
                    FALSE, ch, 0, 0, TO_ROOM);
            } else if (*ch->specials.poofout != '!') {
                act(ch->specials.poofout, FALSE, ch, 0, 0, TO_ROOM);
            } else {
                command_interpreter(ch, (ch->specials.poofout + 1));
            }
        }

        if (ch->specials.fighting) {
            stop_fighting(ch);
        }
        char_from_room(ch);
        char_to_room(ch, ch->specials.start_room);

        if (IS_SET(ch->specials.act, PLR_STEALTH)) {
            for (v = real_roomp(ch->in_room)->people; v;
                 v = v->next_in_room) {
                if ((ch != v) && (CAN_SEE(v, ch))) {
                    if (!IS_SET(ch->specials.pmask, BIT_POOF_IN) ||
                        !ch->specials.poofin) {
                        act("A whirling vortex appears and $n arrives.",
                            FALSE, ch, 0, v, TO_VICT);
                    } else {
                        act(ch->specials.poofin, FALSE, ch, 0, v, TO_VICT);
                    }
                }
            }
        } else if (!IS_SET(ch->specials.pmask, BIT_POOF_IN) ||
                   !ch->specials.poofin) {
            act("A whirling vortex appears and $n arrives.", FALSE, ch,
                    0, v, TO_ROOM);
        } else if (*ch->specials.poofin != '!') {
            act(ch->specials.poofin, FALSE, ch, 0, v, TO_ROOM);
        } else {
            command_interpreter(ch, (ch->specials.poofin + 1));
        }
#if 0
        act("You think of your home plane, and send yourself "
                "home.",FALSE,ch,0,0,TO_CHAR);
        if (!IS_SET(ch->specials.pmask, BIT_POOF_OUT) ||
                !ch->specials.poofout) {
            act("$n disappears in a cloud of mushrooms.",
                    FALSE, ch, 0, 0, TO_ROOM);
        } else if (*ch->specials.poofout != '!') {
            act(ch->specials.poofout, FALSE, ch, 0, 0, TO_ROOM);
        } else {
            command_interpreter(ch, (ch->specials.poofout+1));
        }
        char_from_room(ch);
        char_to_room(ch,ch->specials.start_room);
        act("A whirling vortex appears and $n arrives.",FALSE,ch,0,0,TO_ROOM);
#endif
        do_look(ch, NULL, 15);
    } else {
        send_to_char("You ain't got a home buddy!\n\r", ch);
    }
}

void do_wizset(struct char_data *ch, char *argument, int cmd)
{
    char           *flag,
                   *name;
    char            temp[256];
    dlog("in do_wizset");

    if (IS_NPC(ch) || !ch) {
        return;
    }
    sprintf(temp, "%s", argument);

    argument = get_argument(argument, &flag);

    if (!flag) {
        send_to_char("Set what wizard flag? (fast/map/home/immtitle)\n\r", ch);
        return;
    }

    if (!strcmp("fast", flag)) {
        if (IS_SET(ch->player.user_flags, FAST_AREA_EDIT)) {
            send_to_char("Fast edit disabled.\n\r", ch);
            REMOVE_BIT(ch->player.user_flags, FAST_AREA_EDIT);
        } else {
            send_to_char("$c0001Fast edit enabled.\n\r", ch);
            SET_BIT(ch->player.user_flags, FAST_AREA_EDIT);
        }
    } else if (!strcmp("map", flag)) {
        if (IS_SET(ch->player.user_flags, FAST_MAP_EDIT)) {
            send_to_char("Map edit disabled.\n\r", ch);
            REMOVE_BIT(ch->player.user_flags, FAST_MAP_EDIT);
        } else {
            send_to_char("$c000GFast mapping mode edit enabled.\n\r", ch);
            if (!IS_SET(ch->player.user_flags, FAST_AREA_EDIT)) {
                SET_BIT(ch->player.user_flags, FAST_AREA_EDIT);
            }
            SET_BIT(ch->player.user_flags, FAST_MAP_EDIT);
        }
    } else if (!strcmp("immtitle", flag)) {
        name = skip_spaces(argument);
        if (name) {
            if (ch->specials.immtitle) {
                free(ch->specials.immtitle);
            }
            ch->specials.immtitle = strdup(temp);
            write_char_extra(ch);
            send_to_char("Immort title Field set.\n\r", ch);
        } else {
            if (ch->specials.immtitle) {
                free(ch->specials.immtitle);
            }
            write_char_extra(ch);
            send_to_char("Immort title removed.\n\r", ch);
        }
    } else if (!strcmp("home", flag)) {
        ch->specials.start_room = ch->in_room;
    } else {
        send_to_char("That is not a valid setting.\n\r", ch);
    }
    send_to_char("Ok.\n\r", ch);
}

void do_wiznoooc(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    struct obj_data *dummy;
    char           *buf;

    dlog("in do_wiznoooc");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &buf);

    if (!buf || !generic_find(buf, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
        send_to_char("Couldn't find any such creature.\n\r", ch);
    } else if (IS_NPC(vict)) {
        send_to_char("Why would you Wiznooc a Mob??.\n\r", ch);
    } else if (GetMaxLevel(vict) >= GetMaxLevel(ch)) {
        act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
    } else if (IS_SET(vict->specials.act, PLR_WIZNOOOC) &&
               (GetMaxLevel(ch) >= 55)) {
        send_to_char("Your ability to use the OOC channel is back :).\n\r",
                     vict);
        send_to_char("WIZNOOOC removed.\n\r", ch);
        REMOVE_BIT(vict->specials.act, PLR_WIZNOOOC);
    } else if (GetMaxLevel(ch) >= 55) {
        send_to_char("The gods take away your ability to use the OOC "
                     "channel!\n\r", vict);
        send_to_char("WIZNOOOC set.\n\r", ch);
        SET_BIT(vict->specials.act, PLR_WIZNOOOC);
    } else {
        send_to_char("Sorry, you can't do that\n\r", ch);
    }
}

void do_wizreport(struct char_data *ch, char *argument, int cmd)
{
    char           *arg1,
                   *arg2;

    if (!IS_SET(ch->specials.act, PLR_WIZREPORT) && GetMaxLevel(ch) < 60) {
        send_to_char("You do not have access to this command!\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg1);
    arg2 = skip_spaces(argument);

    if (!arg1) {
        send_to_char("Hrm... normally you wizreport [type] [something], "
                     "no?\n\r", ch);
        return;
    }

    if (!strcmp("bug", arg1)) {
        if (!arg2) {
            send_to_char("Which bug are you talking about??\n\r", ch);
            return;
        }

        db_report_entry( REPORT_WIZBUG, ch, arg2 );
        send_to_char("Done...", ch);
    } else if (!strcmp("idea", arg1)) {
        if (!arg2) {
            send_to_char("Hrm... this does not sound like a bright idea to "
                         "me :P\n\r", ch);
            return;
        }

        db_report_entry( REPORT_WIZIDEA, ch, arg2 );
        send_to_char("Done...", ch);
    } else if (!strcmp("typo", arg1)) {
        if (!arg2) {
            send_to_char("I do not think this is a typo...\n\r", ch);
            return;
        }

        db_report_entry( REPORT_WIZTYPO, ch, arg2 );
        send_to_char("Done...", ch);
    } else if (!strcmp("cleanbug", arg1)) {
        send_to_char("Cleaning the mortal bug file NOW!\n\r", ch);
        db_clean_report( REPORT_BUG );
        Log("%s just cleaned the bug file!", GET_NAME(ch));
    } else if (!strcmp("cleanidea", arg1)) {
        send_to_char("Cleaning the mortal idea file NOW!\n\r", ch);
        db_clean_report( REPORT_IDEA );
        Log("%s just cleaned the idea file!", GET_NAME(ch));
    } else if (!strcmp("cleantypo", arg1)) {
        send_to_char("Cleaning the mortal typo file NOW!\n\r", ch);
        db_clean_report( REPORT_TYPO );
        Log("%s just cleaned the typo file!", GET_NAME(ch));
    } else {
        send_to_char("What do you wanna do?!?!?\n\r", ch);
    }
}

void do_lgos(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_INPUT_LENGTH + 40];
    char            buf2[MAX_INPUT_LENGTH + 40];
    struct descriptor_data *i;
    extern int      Silence;

    dlog("in do_lyell");

    if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
        send_to_char("You can't shout, yell or auction.\n\r", ch);
        return;
    }

    if (IS_NPC(ch) && Silence == 1 && IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Polymorphed yelling has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
        return;
    }

    if (apply_soundproof(ch)) {
        return;
    }

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM) && !IS_IMMORTAL(ch->master)) {
        send_to_char("I don't think so :-)", ch->master);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("yell? Yes! but what!\n\r", ch);
    } else {
        if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
            sprintf(buf1, "$c0011You yell '%s'", argument);
            act(buf1, FALSE, ch, 0, 0, TO_CHAR);
        }

        /*
         * I really hate when people gossip about lag and it is not
         * caused by this machine NOR IS IT ON this machine. This
         * should grab all the gossips about it and make them think
         * that is was punched over the wire to everyone else!
         */

        if (strstr(argument, "lag") || strstr(argument, "LAG")
            || strstr(argument, "Lag") || strstr(argument, "LAg")
            || strstr(argument, "laG") || strstr(argument, "lAG")) {
            /*
             * do nothing....
             */
        } else {
            sprintf(buf1, "$c0011[$c0015$n$c0011] yells '%s'", argument);
            for (i = descriptor_list; i; i = i->next) {
                if (i->character != ch && !i->connected &&
                    (IS_NPC(i->character) ||
                     (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                      !IS_SET(i->character->specials.act, PLR_NOGOSSIP))) &&
                    !check_soundproof(i->character)) {

#ifdef ZONE_COMM_ONLY
                    /*
                     * gossip in zone only
                     */
                    if (i->character->in_room != NOWHERE) {
                        if (real_roomp(ch->in_room)->zone ==
                            real_roomp(i->character->in_room)->zone &&
                            !IS_IMMORTAL(i->character)) {
                            act(buf1, 0, ch, 0, i->character, TO_VICT);
                        } else if (IS_IMMORTAL(i->character)) {
                            sprintf(buf2, "$c0011[$c0015$n$c0011] yells from "
                                    "zone %ld '%s'",
                                    real_roomp(ch->in_room)->zone, argument);
                            act(buf2, 0, ch, 0, i->character, TO_VICT);
                        }
                    }
#else
                    act(buf1, 0, ch, 0, i->character, TO_VICT);
#endif
                }
            }
        }
    }
}

void do_set_spy(struct char_data *ch, char *argument, int cmd)
{
    extern int      spy_flag;

    dlog("in do_set_spy");

    if (IS_NPC(ch)) {
        return;
    }
    if ((GetMaxLevel(ch) < 60) || (IS_NPC(ch))) {
        send_to_char("You cannot Set_Spy.\n\r", ch);
        return;
    }

    spy_flag = !spy_flag;

    if (spy_flag) {
        send_to_char("You just set spy_flag to ON", ch);
    } else {
        send_to_char("You just set spy_flag to OFF", ch);
    }
}

void do_reward(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                   *word;
    char            buf[256];
    int             done = FALSE;
    int             amount = 1;
    int             temp;
    struct descriptor_data *d;

    dlog("in do_reward");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     * get char name
     */
    argument = get_argument(argument, &name);

    if( !name ) {
        send_to_char( "Reward who exactly?!\n\r", ch );
        return;
    }

    /*
     * search through descriptor list for player name
     */
    for (d = descriptor_list; d && !done; d = d->next) {
        if (d->character && GET_NAME(d->character) &&
            strcasecmp(GET_NAME(d->character), name) == 0) {
            done = TRUE;
            break;
        }
    }

    /*
     * if not found, return error
     */
    if (!done) {
        send_to_char("That player was not found.\n\r", ch);
        return;
    }

    if (IS_NPC(d->character)) {
        send_to_char("You can't reward a MOB\n\r", ch);
        return;
    }

    if (argument) {
        /*
         * get response amount of the reward reward)
         */
        argument = get_argument(argument, &word);
        amount = -1;

        if ( word ) {
            amount = atoi(word);
        }

        if( !word || amount <= 0 ) {
            oldSendOutput(ch, "Invalid amount for reward\n\r");
            return;
        }
    }

    temp = (int)d->character->player.q_points;
    temp += amount;
    if (temp >= 1 << 16) {
        oldSendOutput(ch, "Amount of reward too big, Quest points greater than "
                      "65535\n\r");
        return;
    }

    d->character->player.q_points += (unsigned short int)amount;
    oldSendOutput(ch, "You just awarded %d Quest points to %s\n\r", amount,
                  GET_NAME(d->character));

    sprintf(buf, "awarded %d quest points to %s", amount, GET_NAME(ch));
    qlog(d->character, buf);

    oldSendOutput(d->character, "You were just awarded %d Quest points. You now "
                            "have %d Quest points\n\r", amount, 
                            d->character->player.q_points);
    save_char(d->character, AUTO_RENT);
}

void do_punish(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                   *word;
    char            buf[256];
    int             done = FALSE;
    int             amount = 1,
                    temp;
    struct descriptor_data *d;

    dlog("in do_punish");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     * get char name
     */
    argument = get_argument(argument, &name);

    if( !name ) {
        send_to_char( "Of course we must punish, but whom?\n\r", ch );
        return;
    }

    /*
     * search through descriptor list for player name
     */
    for (d = descriptor_list; d && !done; d = d->next) {
        if (d->character && GET_NAME(d->character) &&
            strcasecmp(GET_NAME(d->character), name) == 0) {
            done = TRUE;
            break;
        }
    }

    /*
     * if not found, return error
     */
    if (!done) {
        send_to_char("That player was not found.\n\r", ch);
        return;
    }

    if (IS_NPC(d->character)) {
        send_to_char("You can't punish a MOB\n\r", ch);
        return;
    }

    if (argument) {
        /*
         * get response amount of the reward reward)
         */
        argument = get_argument(argument, &word);
        amount = -1;

        if( word ) {
            amount = atoi(word);
        }

        if ( !word || amount <= 0 ) {
            oldSendOutput(ch, "Invalid amount for punish\n\r");
            return;
        }
    }

    temp = (int)d->character->player.q_points;
    temp -= amount;

    if (temp < 0) {
        oldSendOutput(ch, "Amount of punish too big, Quest points would be less "
                      "than 0\n\r");
        return;
    }

    d->character->player.q_points -= (unsigned short int)amount;
    oldSendOutput(ch, "You just punished %s of  %u Quest points\n\r",
                  GET_NAME(d->character), amount);

    sprintf(buf, "punished %d quest points by %s", amount, GET_NAME(ch));
    qlog(d->character, buf);

    oldSendOutput(d->character, "You were just punished %u Quest points. You now "
                            "have %u Quest points\n\r", amount, 
                            d->character->player.q_points);
    save_char(d->character, AUTO_RENT);
}

void do_spend(struct char_data *ch, char *argument, int cmd)
{
    char           *name,
                   *word;
    char            buf[256];
    int             done = FALSE;
    int             amount = 1,
                    temp;
    struct descriptor_data *d;

    dlog("in do_spend");

    if (IS_NPC(ch)) {
        return;
    }

    /*
     * get char name
     */
    argument = get_argument(argument, &name);

    if( !name ) {
        send_to_char( "Who's supposed to spend?\n\r", ch );
        return;
    }

    /*
     * search through descriptor list for player name
     */
    for (d = descriptor_list; d && !done; d = d->next) {
        if (d->character && GET_NAME(d->character) &&
            strcasecmp(GET_NAME(d->character), name) == 0) {
            done = TRUE;
            break;
        }
    }

    /*
     * if not found, return error
     */
    if (!done) {
        send_to_char("That player was not found.\n\r", ch);
        return;
    }

    if (IS_NPC(d->character)) {
        send_to_char("A MOB can't spend\n\r", ch);
        return;
    }

    if (argument) {
        /*
         * get response amount of the reward reward)
         */
        argument = get_argument(argument, &word);
        amount = -1;
        if( word ) {
            amount = atoi(word);
        }

        if (!word || amount <= 0) {
            oldSendOutput(ch, "Invalid amount for spend\n\r");
            return;
        }
    }

    temp = (int)d->character->player.q_points;
    temp -= amount;
    if (temp < 0) {
        oldSendOutput(ch, "Amount of spending too big, Quest points would be "
                      "less than 0\n\r");
        return;
    }

    d->character->player.q_points -= (unsigned short int)amount;
    oldSendOutput(ch, "%s just spent %u Quest points\n\r",
                  GET_NAME(d->character), amount);

    printf(buf, "spent %d quest points (by %s)", amount, GET_NAME(ch));
    qlog(d->character, buf);

    oldSendOutput(d->character, "You just spent %u Quest points. You now have %u "
                            "Quest points\n\r", amount, 
                            d->character->player.q_points);
    save_char(d->character, AUTO_RENT);
}

void do_see_points(struct char_data *ch, char *argument, int cmd)
{
    char           *name;
    int             done = FALSE;

    struct descriptor_data *d;

    dlog("in do_see_points");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     * get char name
     */
    argument = get_argument(argument, &name);
    if( !name ) {
        send_to_char( "Whose points to look at?\n\r", ch );
        return;
    }

    /*
     * search through descriptor list for player name
     */
    for (d = descriptor_list; d && !done; d = d->next) {
        if (d->character && GET_NAME(d->character) &&
            strcasecmp(GET_NAME(d->character), name) == 0) {
            done = TRUE;
            break;
        }
    }

    /*
     * if not found, return error
     */
    if (!done) {
        send_to_char("That player was not found.\n\r", ch);
        return;
    }

    if (IS_NPC(d->character)) {
        send_to_char("MOB don't have Quest points\n\r", ch);
        return;
    }

    oldSendOutput(ch, "%s has %u Quest points\n\r", GET_NAME(d->character),
                  d->character->player.q_points);
}

void do_setobjmax(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    char           *objec,
                   *num;
    int             number;

    dlog("in do_setobjmax");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &objec);
    num = skip_spaces(argument);

    if (num && isdigit((int)*num)) {
        number = atoi(num);
    } else {
        send_to_char("usage is: setobjmax itemname loadrate.\n\r", ch);
        return;
    }

    if (!objec) {
        send_to_char("Give what?\n\r", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, objec, ch->carrying))) {
        send_to_char("Where is that?\n\r", ch);
        return;
    }

    obj->max = number;
    oldSendOutput(ch, "Set object loadrate to %d.\n\r", number);
}

void do_setobjspeed(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    char           *objec,
                   *num;
    long            number;
    dlog("in do_setobjspeed");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &objec);
    num = skip_spaces(argument);

    if (num && isdigit((int)*num)) {
        number = atoi(num);
    } else {
        send_to_char("usage is: setobjspeed item speed.\n\r", ch);
        return;
    }

    if (!objec) {
        send_to_char("Give what?\n\r", ch);
        return;
    }
    if (!(obj = get_obj_in_list_vis(ch, objec, ch->carrying))) {
        send_to_char("where is that?\n\r", ch);
        return;
    }
    if (number >= 0 && number <= 100) {
        obj->speed = number;
        oldSendOutput(ch, "Set object speed to %.2f\n\r", (float) number / 100);
    } else {
        send_to_char("Speed values between 0 and 100 please. (0 is slow, "
                     "100 is fast)", ch);
    }
}

void do_setwtype(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    char           *objec,
                   *num;
    long            number;

    dlog("in do_setobjspeed");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &objec);
    num = skip_spaces(argument);

    if (num && isdigit((int)*num)) {
        number = atoi(num);
    } else {
        send_to_char("Usage: setwtype item wtype.  Get wtype from allweapons "
                     "list.\n\r", ch);
        return;
    }

    if (!objec) {
        send_to_char("Set which weapon?\n\r", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, objec, ch->carrying))) {
        send_to_char("Where is that?\n\r", ch);
        return;
    }

    if (!IS_WEAPON(obj)) {
        send_to_char("That's not a weapon.\n\r", ch);
        return;
    }

    if (number >= 0 && number <= 59) {
        obj->weapontype = number;
        oldSendOutput(ch, "Set weapontype to %s\n\r",
                      weaponskills[obj->weapontype].name);
    } else {
        send_to_char("Invalid weapontype. Get wnum from allweapons list.", ch);
    }
}

void do_setsound(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj;
    struct char_data *dummy;
    char           *name,
                   *sound;

    dlog("in do_setsound");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &name);
    sound = skip_spaces(argument);

    if (!name) {
        send_to_char("Usage:\n\r  To set a sound:     setsound <itemname> "
                     "<sound string>\n\r  To remove a sound:  setsound "
                     "<itemname>\n\r", ch);
        return;
    }

    if (generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &dummy, &obj)) {
        if (ITEM_TYPE(obj) == ITEM_TYPE_AUDIO) {
            if (!sound) {
                oldSendOutput(ch, "Setting sound for %s to none.\n\r",
                              obj->short_description);
                if (obj->action_description) {
                    free(obj->action_description);
                }
                send_to_char("Okay.\n\r", ch);
            } else {
                oldSendOutput(ch, "Setting sound '%s' to %s.\n\r", sound,
                              obj->short_description);
                strcat(sound, "\n\r");
                obj->action_description = strdup(sound);
                send_to_char("Okay.\n\r", ch);
            }
        } else {
            send_to_char("That object isn't flagged TYPE_AUDIO.\n\r", ch);
        }
    } else {
        send_to_char("Which item was that?\n\r", ch);
    }
}

void do_wclean(struct char_data *ch, char *argument, int cmdnum)
{
    int             zone = -1;

    dlog("in do_wclean");

    if (IS_NPC(ch)) {
        return;
    }
    for (zone = 2; zone <= top_of_zone_table; zone++) {
        if ((zone_table[zone].start != 0) && ZoneCleanable(zone)) {
            CleanZone(zone);
            zone_table[zone].start = 0;
        }
    }
    send_to_char("The Time has stopped while Roger Wilco has swept the "
                 "world cleaner !\r\n", ch);
}

void do_startarena(struct char_data *ch, char *argument, int cmd)
{
    char           *arg1,
                   *arg2,
                   *arg3;
    int             tmp1,
                    tmp2,
                    tmp3;
    char           *flag;
    char            buf[MAX_STRING_LENGTH];

    dlog("startarena");

    if (IS_NPC(ch)) {
        return;
    }
    argument = get_argument(argument, &arg1);
    argument = get_argument(argument, &arg2);
    argument = get_argument(argument, &arg3);

    if( !arg1 ) {
        if( MinArenaLevel != 0 && MaxArenaLevel != 0 ) {
            MinArenaLevel = 0;
            MaxArenaLevel = 0;

            sprintf(buf, "$c000cThe $c000CArena $c000cis now closed!\n\r");
            send_to_all(buf);
            Log("%s closed the arena!\n\r", GET_NAME(ch));
        } else {
            send_to_char("The arena isn't open, numbskull!\n\r", ch);
        }
        return;
    }

    if (!arg2 || !arg3 || !strcmp(arg1, "help")) {
        send_to_char("Usage: startarena <minlevel> <maxlevel> <quadrant> "
                     "<flags>\n\r"
                     "       flags are optional, divide by spaces:\n\r"
                     "       -g  no grouping allowed\n\r"
                     "       -a  no assisting in ongoing fights\n\r", ch);
        send_to_char("       -d  no dispel magic\n\r"
                     "       -m  no magic\n\r"
                     "       -w  no weapon spells\n\r"
                     "       -s  no slay weapons\n\r", ch);
        send_to_char("       -f  no fleeing\n\r"
                     "       -h  no haste\n\r"
                     "       -p  no pets\n\r"
                     "       -t  no travelling spells\n\r", ch);
#if 0
        send_to_char(" -b no bash", ch);
#endif
        send_to_char("Example: startarena 1 50 3 -s -w -f\n\r", ch);
        return;
    }

    tmp1 = atoi(arg1);
    tmp2 = atoi(arg2);
    tmp3 = atoi(arg3);

    if ((tmp1 > tmp2) || tmp1 < 0 || tmp1 > 60 || tmp2 < 0 || tmp2 > 60
        || tmp3 > 4 || tmp3 < 0) {
        send_to_char("Type 'startarena help' for more info.\n\r", ch);
        return;
    }

    if ((tmp1 == 0 && tmp2 == 0)) {
        MinArenaLevel = 0;
        MaxArenaLevel = 0;

        sprintf(buf, "$c000cThe $c000CArena $c000cis now closed!\n\r");
        send_to_all(buf);
        Log("%s closed the arena!\n\r", GET_NAME(ch));
    } else {
        /*
         * first set flags to be FALSE
         */
        ArenaNoGroup = 0;
        ArenaNoAssist = 0;
        ArenaNoDispel = 0;
        ArenaNoMagic = 0;
        ArenaNoWSpells = 0;
        ArenaNoSlay = 0;
        ArenaNoFlee = 0;
        ArenaNoHaste = 0;
        ArenaNoPets = 0;
        ArenaNoTravel = 0;
        ArenaNoBash = 0;

        while (argument) {
            argument = get_argument(argument, &flag);
            if( !flag ) {
                continue;
            }

            if (!strcmp(flag, "-g")) {
                ArenaNoGroup = 1;
            } else if (!strcmp(flag, "-a")) {
                ArenaNoAssist = 1;
                /*
                 * no point in grouping if you can't assist
                 */
                ArenaNoGroup = 1;
            } else if (!strcmp(flag, "-d")) {
                ArenaNoDispel = 1;
            } else if (!strcmp(flag, "-m")) {
                ArenaNoMagic = 1;
            } else if (!strcmp(flag, "-w")) {
                ArenaNoWSpells = 1;
            } else if (!strcmp(flag, "-s")) {
                ArenaNoSlay = 1;
            } else if (!strcmp(flag, "-f")) {
                ArenaNoFlee = 1;
            } else if (!strcmp(flag, "-h")) {
                ArenaNoHaste = 1;
            } else if (!strcmp(flag, "-p")) {
                ArenaNoPets = 1;
            } else if (!strcmp(flag, "-t")) {
                ArenaNoTravel = 1;
            }
#if 0
            else if (!strcmp(flag, "-b")) {
                ArenaNoBash = 1;
            }
#endif
            else {
                send_to_char("Invalid flag entered. Type 'startarena help' for"
                             " complete listing.\n\r", ch);
                return;
            }
        }

        MinArenaLevel = tmp1;
        MaxArenaLevel = tmp2;
        Quadrant = tmp3;
        sprintf(buf, "$c000cThe $c000CArena $c000cis now open for level "
                     "$c000C%d $c000cto $c000C%d$c000c.\n\r",
                MinArenaLevel, MaxArenaLevel);
        send_to_all(buf);
        if (ArenaNoGroup == 1) {
            ArenaNoGroup = 1;
            sprintf(buf, "$c000cNo groups allowed.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoAssist == 1) {
            sprintf(buf, "$c000cNo assisting of players.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoDispel == 1) {
            sprintf(buf, "$c000cDispel magic will not work.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoMagic == 1) {
            sprintf(buf, "$c000cUse of magic is not allowed.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoWSpells == 1) {
            sprintf(buf, "$c000cWeapon spells will have no effect.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoSlay == 1) {
            sprintf(buf, "$c000cSlay weapons will have no effect.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoFlee == 1) {
            sprintf(buf, "$c000cFleeing from fights is not possible.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoHaste == 1) {
            sprintf(buf, "$c000cHaste will have no effect.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoPets == 1) {
            sprintf(buf, "$c000cSummoning pets is not possible.\n\r");
            send_to_all(buf);
        }
        if (ArenaNoTravel == 1) {
            sprintf(buf, "$c000cTravelling spells are disabled.\n\r");
            send_to_all(buf);
        }
#if 0
        if (ArenaNoBash == 1) {
            sprintf(buf, "$c000cBashing in the arena is not allowed.\n\r");
            send_to_all(buf);
        }
#endif
        sprintf(buf, "$c000cType $c000CArena$c000c to enter\n\r");
        send_to_all(buf);
        Log("%s opened an arena for level %d to %d in quadrant %d",
            GET_NAME(ch), MinArenaLevel, MaxArenaLevel, Quadrant);
    }
}

void do_tweak(struct char_data *ch, char *arg, int cmd)
{
    struct obj_data *obj;
    struct char_data *dummy;
    char           *name;

    dlog("in do_tweak");

    if (!ch) {
        return;
    }
    if (cmd != 598) {
        /*
         * cmd 598 = tweak
         */
        return;
    }

    arg = get_argument(arg, &name);
    if (!name) {
        send_to_char("Tweak what?\n\r", ch);
        return;
    }

    if (generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &dummy, &obj)) {
        if (obj) {
            if (IS_OBJ_STAT(obj, extra_flags, ITEM_IMMUNE)) {
                send_to_char("You cannot tweak artifacts.\n\r", ch);
                return;
            }
            oldSendOutput(ch, "You tweak %s.\n\r", obj->short_description);
            act("$n tweaks $p!", FALSE, ch, obj, 0, TO_ROOM);
            tweak(obj);
        }
    } else {
        send_to_char("Hmm, can't find it.\n\r", ch);
    }
}

void do_zconv(struct char_data *ch, char *argument, int cmdnum)
{

    int             start_room,
                    end_room,
                    zone;
    FILE           *fp = NULL;
    char           *arg1,
                   *arg2,
                   *arg3;

    dlog("in do_zconv");

    if (IS_NPC(ch)) {
        return;
    }
    /*
     *   read in parameters (room #s)
     */
    argument = get_argument(argument, &arg1);
    argument = get_argument(argument, &arg2);
    argument = get_argument(argument, &arg3);

    if( !arg1 ) {
        send_to_char("Zsave <zone_number> [<start_room> <end_room>]\n\r", ch);
        return;
    }
    zone = atoi(arg1);


    if (zone > top_of_zone_table) {
        send_to_char("Invalid zone number\r\n", ch);
        return;
    }

    /*
     * make some permission checks
     */
    if (GetMaxLevel(ch) < 56 && zone != GET_ZONE(ch)) {
        send_to_char("Sorry, you are not authorized to save this zone.\n\r",
                     ch);
        return;
    }

    if (!zone_table[zone].start) {
        send_to_char("Sorry, that zone isn't initialized yet\r\n", ch);
        return;
    }

    if( !arg2 || !arg3 ) {
        start_room = zone ? (zone_table[zone - 1].top + 1) : 0;
        end_room = zone_table[zone].top;
    } else {
        start_room = atoi(arg2);
        end_room   = atoi(arg3);
    }

    /*
     * declare file
     * Do saving
     */

    fprintf(fp, "*Zone %d, rooms %d-%d, last modified by %s\n",
            zone, start_room, end_room, ch->player.name);

    fclose(fp);

    send_to_char("Ok\r\n", ch);
}

void do_eval(struct char_data *ch, char *arg, int cmd)
{
    struct obj_data *obj;
    struct char_data *dummy;
    char           *name;

    int             total = 0;

    dlog("in do_eval");

    if (!ch)
        return;
    if (cmd != 603) {
        /*
         * cmd 603 = EVAL
         */
        return;
    }

    arg = get_argument(arg, &name);
    if (!name) {
        send_to_char("Eval what?\n\r", ch);
        return;
    }

    if (generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &dummy, &obj)) {
        if (obj) {
            total = eval(obj);
            oldSendOutput(ch, "You evaluate %s: %d.\n\r", obj->short_description,
                      total);
        }
    } else {
        send_to_char("Hmm, can't find it.\n\r", ch);
    }
}

int eval(struct obj_data *object)
{
    int             total = 0;
    int             i;
    struct time_info_data age(struct char_data *ch);

    /*
     * anti stuff
     */
    if (IS_OBJ_STAT(object, extra_flags, ITEM_BRITTLE)) {
        total += -15;
    }
    if (IS_OBJ_STAT(object, extra_flags, ITEM_RESISTANT)) {
        total += 7;
    }
    if (IS_OBJ_STAT(object, extra_flags, ITEM_IMMUNE)) {
        total += 15;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_GOOD)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_EVIL)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_NEUTRAL)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_WOMEN)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_MEN)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ONLY_CLASS)) {
        total += -7;
    }
    if (IS_OBJ_STAT(object, extra_flags, ITEM_RARE)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_SUN)) {
        total += -3;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_GOOD)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_flags, ITEM_ANTI_GOOD)) {
        total += -5;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_BARBARIAN)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_PALADIN)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_MONK)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_FIGHTER)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_CLERIC)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_MAGE)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_NECROMANCER)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_PSI)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_RANGER)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_DRUID)) {
        total += -1;
    }
    if (IS_OBJ_STAT(object, anti_class, ITEM_ANTI_THIEF)) {
        total += -1;
    }

    if (object->cost_per_day > 10000) {
        total -= (int) (object->cost_per_day / 1000);
    } else if (object->cost_per_day < 0) {
        /*
         * unrentable
         */
        total -= 20;
    }

    switch (ITEM_TYPE(object)) {
    case ITEM_TYPE_SCROLL:
    case ITEM_TYPE_POTION:
    case ITEM_TYPE_STAFF:
    case ITEM_TYPE_FOOD:
    case ITEM_TYPE_DRINKCON:
    case ITEM_TYPE_TRAP:
    case ITEM_TYPE_NOTE:
    case ITEM_TYPE_PEN:
    case ITEM_TYPE_TRASH:
    case ITEM_TYPE_MONEY:
    case ITEM_TYPE_KEY:
        total -= 20;
        break;
    case ITEM_TYPE_CONTAINER:
    case ITEM_TYPE_LIGHT:
        total += 5;
        break;
    case ITEM_TYPE_ARMOR: /* armor class */
        total += object->value[0];
        break;
    case ITEM_TYPE_WEAPON: /* damage of sword */
        total -= 43;
        total += object->value[1] * 2;
        total += object->value[2] * 2;

        switch (object->value[3]) {
        case 0:     /* TYPE_SMITE */
        case 1:     /* TYPE_STAB */
        case 2:     /* TYPE_WHIP */
        case 5:     /* TYPE_CLEAVE */
        case 9:     /* TYPE_BITE */
        case 10:    /* TYPE_STING */
        case 12:    /* TYPE_BLAST */
        case 13:    /* TYPE_IMPALE */
            total += 6;
            break;

        default:    /* scrappy types, no bonus */
            break;
        }

        break;

    default:
        break;

    }

    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        switch (object->affected[i].location) {

        case APPLY_STR:
        case APPLY_DEX:
        case APPLY_INT:
        case APPLY_WIS:
        case APPLY_CON:
        case APPLY_CHR:
            if (object->affected[i].modifier < 0) {
                total += object->affected[i].modifier * 2;
            } else {
                total += object->affected[i].modifier * 6;
            }
            break;
        case APPLY_SEX:
            total += 1;
            break;
        case APPLY_LEVEL:
            total += 1;
            break;
        case APPLY_AGE:
            total += 1;
            break;
        case APPLY_CHAR_WEIGHT:
            total += 1;
            break;
        case APPLY_CHAR_HEIGHT:
            total += 1;
            break;
        case APPLY_MANA:
            total += object->affected[i].modifier;
            break;
        case APPLY_HIT:
            total += object->affected[i].modifier;
            break;
        case APPLY_MOVE:
            total += (int) object->affected[i].modifier / 3;
            break;
        case APPLY_GOLD:
            total += 1;
            break;
        case APPLY_ARMOR:
            total -= object->affected[i].modifier;
            break;
        case APPLY_HITROLL:
            if (ITEM_TYPE(object) == ITEM_TYPE_WEAPON) {
                if (object->affected[i].modifier > 4) {
                    total += 26;
                } else if (object->affected[i].modifier > 3) {
                    total += 18;
                } else {
                    total += object->affected[i].modifier * 4;
                }
            } else {
                total += object->affected[i].modifier * 4;
            }
            break;
        case APPLY_DAMROLL:
            if (ITEM_TYPE(object) == ITEM_TYPE_WEAPON) {
                if (object->affected[i].modifier > 4) {
                    total += 36;
                } else if (object->affected[i].modifier > 3) {
                    total += 26;
                } else {
                    total += object->affected[i].modifier * 6;
                }
            } else {
                if (object->affected[i].modifier > 2) {
                    total += 24;
                } else if (object->affected[i].modifier > 1) {
                    total += 14;
                } else {
                    total += object->affected[i].modifier * 6;
                }
            }

            break;
        case APPLY_SAVING_PARA:
        case APPLY_SAVING_ROD:
        case APPLY_SAVING_PETRI:
        case APPLY_SAVING_BREATH:
        case APPLY_SAVING_SPELL:
            total -= object->affected[i].modifier;
            break;
        case APPLY_SAVE_ALL:
            total -= object->affected[i].modifier * 3;
            break;

        case APPLY_M_IMMUNE:
            if (IS_SET(object->affected[i].modifier, IMM_POISON)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_CHARM)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLEEP)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_HOLD)) {
                total += 12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_DRAIN)) {
                total += 12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ACID)) {
                total += 20;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ELEC)) {
                total += 20;
            }
            if (IS_SET(object->affected[i].modifier, IMM_COLD)) {
                total += 20;
            }
            if (IS_SET(object->affected[i].modifier, IMM_FIRE)) {
                total += 24;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ENERGY)) {
                total += 24;
            }
            if (IS_SET(object->affected[i].modifier, IMM_PIERCE)) {
                total += 24;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLASH)) {
                total += 30;
            }
            if (IS_SET(object->affected[i].modifier, IMM_BLUNT)) {
                total += 30;
            }
            break;
        case APPLY_IMMUNE:
            if (IS_SET(object->affected[i].modifier, IMM_POISON)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_CHARM)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLEEP)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_HOLD)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_DRAIN)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ACID)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ELEC)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_COLD)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_FIRE)) {
                total += 12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ENERGY)) {
                total += 12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_PIERCE)) {
                total += 12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLASH)) {
                total += 15;
            }
            if (IS_SET(object->affected[i].modifier, IMM_BLUNT)) {
                total += 15;
            }
            break;
        case APPLY_SUSC:
            if (IS_SET(object->affected[i].modifier, IMM_POISON)) {
                total += -3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_CHARM)) {
                total += -3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLEEP)) {
                total += -3;
            }
            if (IS_SET(object->affected[i].modifier, IMM_HOLD)) {
                total += -6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_DRAIN)) {
                total += -6;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ACID)) {
                total += -10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ELEC)) {
                total += -10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_COLD)) {
                total += -10;
            }
            if (IS_SET(object->affected[i].modifier, IMM_FIRE)) {
                total += -12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_ENERGY)) {
                total += -12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_PIERCE)) {
                total += -12;
            }
            if (IS_SET(object->affected[i].modifier, IMM_SLASH)) {
                total += -15;
            }
            if (IS_SET(object->affected[i].modifier, IMM_BLUNT)) {
                total += -15;
            }
            break;
        case APPLY_SPELL:
            if (IS_SET(object->affected[i].modifier, AFF_BLIND)) {
                total += -10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_INVISIBLE)) {
                total += 8;
            }
            if (IS_SET(object->affected[i].modifier, AFF_DETECT_EVIL)) {
                total += 2;
            }
            if (IS_SET(object->affected[i].modifier, AFF_DETECT_INVISIBLE)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, AFF_DETECT_MAGIC)) {
                total += 2;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SENSE_LIFE)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SANCTUARY)) {
                total += 30;
            }
            if (IS_SET(object->affected[i].modifier, AFF_GROWTH)) {
                total += 1;
            }
            if (IS_SET(object->affected[i].modifier, AFF_CURSE)) {
                total += -6;
            }
            if (IS_SET(object->affected[i].modifier, AFF_FLYING)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_POISON)) {
                total += -8;
            }
            if (IS_SET(object->affected[i].modifier, AFF_TREE_TRAVEL)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, AFF_PARALYSIS)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_INFRAVISION)) {
                total += 2;
            }
            if (IS_SET(object->affected[i].modifier, AFF_WATERBREATH)) {
                total += 5;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SLEEP)) {
                total += 5;
            }
            if (IS_SET(object->affected[i].modifier, AFF_TRAVELLING)) {
                total += 4;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SNEAK)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_HIDE)) {
                total += 6;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SILENCE)) {
                total += -8;
            }
            if (IS_SET(object->affected[i].modifier, AFF_DARKNESS)) {
                total += 4;
            }
            if (IS_SET(object->affected[i].modifier, AFF_TRUE_SIGHT)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_SCRYING)) {
                total += 10;
            }
            if (IS_SET(object->affected[i].modifier, AFF_FIRESHIELD)) {
                total += 30;
            }
            if (IS_SET(object->affected[i].modifier, AFF_TELEPATHY)) {
                total += 3;
            }
            if (IS_SET(object->affected[i].modifier, AFF_CHILLSHIELD)) {
                total += 30;
            }
            if (IS_SET(object->affected[i].modifier, AFF_BLADE_BARRIER)) {
                total += 30;
            }
            break;
        case APPLY_SPELL2:
            total += 10;
            if (IS_SET(object->affected[i].modifier, AFF2_PROTECT_FROM_EVIL)) {
                total += 6;
            }
            break;
        case APPLY_WEAPON_SPELL:
            total += 30;
            break;
        case APPLY_EAT_SPELL:
            total += 1;
            break;
        case APPLY_BACKSTAB:
            total += object->affected[i].modifier * 2;
            break;
        case APPLY_KICK:
            total += object->affected[i].modifier;
            break;
        case APPLY_SNEAK:
            total += object->affected[i].modifier;
            break;
        case APPLY_HIDE:
            total += object->affected[i].modifier;
            break;
        case APPLY_BASH:
            total += object->affected[i].modifier * 2;
            break;
        case APPLY_PICK:
            total += object->affected[i].modifier;
            break;
        case APPLY_STEAL:
            total += object->affected[i].modifier;
            break;
        case APPLY_TRACK:
            total += 1;
            break;
        case APPLY_HITNDAM:
            if (ITEM_TYPE(object) == ITEM_TYPE_WEAPON) {
                if (object->affected[i].modifier > 4) {
                    total += 62;
                } else if (object->affected[i].modifier > 3) {
                    total += 44;
                } else {
                    total += object->affected[i].modifier * 10;
                }
            } else {
                if (object->affected[i].modifier > 2) {
                    total += 34;
                } else if (object->affected[i].modifier > 1) {
                    total += 22;
                } else {
                    total += object->affected[i].modifier * 10;
                }
            }
            break;
        case APPLY_SPELLFAIL:
            total -= object->affected[i].modifier;
            break;
        case APPLY_ATTACKS:
            total += 20;
            break;
        case APPLY_HASTE:
            total += 30;
            break;
        case APPLY_SLOW:
            total += 1;
            break;
        case APPLY_BV2:
            total += 10;
            break;
        case APPLY_FIND_TRAPS:
            total += 1;
            break;
        case APPLY_RIDE:
            total += 1;
            break;
        case APPLY_RACE_SLAYER:
            total += 7;
            break;
        case APPLY_ALIGN_SLAYER:
            total += 20;
            break;
        case APPLY_MANA_REGEN:
            total += object->affected[i].modifier;
            break;
        case APPLY_HIT_REGEN:
            total += object->affected[i].modifier;
            break;
        case APPLY_MOVE_REGEN:
            total += (int) object->affected[i].modifier / 3;
            break;
        case APPLY_MOD_THIRST:
            total += 1;
            break;
        case APPLY_MOD_HUNGER:
            total += 1;
            break;
        case APPLY_MOD_DRUNK:
            total += 1;
            break;
        default:
            break;
        }
    }
    if (!IS_OBJ_STAT(object, wear_flags, ITEM_TAKE)) {
        total = -100;
    }
    return total;
}

void do_reimb(struct char_data *ch, char *argument, int cmd)
{
    char          *arg;
    struct char_data *victim;

    dlog("in do_reimb");

    if (IS_NPC(ch)) {
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Usage: reimb <player name>\n\r", ch);
        return;
    }

    if (!(victim = get_char(arg))) {
        send_to_char("Who didya want to reimburse?\n\r", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Hah, reimburse mobs? Better make em return to character"
                     " form first!\n\r", ch);
        return;
    }

    if (reimb_char_objs(victim)) {
        Log("%s just granted %s a reimbursement", GET_NAME(ch),
            GET_NAME(victim));

        oldSendOutput(ch, "You reimbursed %s, resetting his gold and equipment to "
                      "the point when they last rented.\n\r", GET_NAME(victim));
        oldSendOutput(victim, "You have been fully reimbursed by %s.\n\r",
                          GET_NAME(ch));
    }
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
