/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD
 */

#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"
#include "externs.h"


void do_say(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH + 80];

    dlog("in do_say");

    if (apply_soundproof(ch))
        return;

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
    } else {
        sprintf(buf, "$c0015$n says '%s'", argument);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
            ch_printf(ch, "$c0015You say '%s'\n\r", argument);
        }
    }
}

void do_report(struct char_data *ch, char *argument, int cmd)
{
    char            buf[100];

    dlog("in do_report");

    if (apply_soundproof(ch)) {
        return;
	}
    if (IS_NPC(ch)) {
        return;
	}

    sprintf(buf,
            "$c0014[$c0015$n$c0014] reports 'HP:%2.0f%% MANA:%2.0f%% "
            "MV:%2.0f%%'",
            ((float) GET_HIT(ch) / (int) GET_MAX_HIT(ch)) * 100.0 + 0.5,
            ((float) GET_MANA(ch) / (int) GET_MAX_MANA(ch)) * 100.0 + 0.5,
            ((float) GET_MOVE(ch) / (int) GET_MAX_MOVE(ch)) * 100.0 + 0.5);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    sprintf(buf, "$c0014You report 'HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%'",
            ((float) GET_HIT(ch) / (int) GET_MAX_HIT(ch)) * 100.0 + 0.5,
            ((float) GET_MANA(ch) / (int) GET_MAX_MANA(ch)) * 100.0 + 0.5,
            ((float) GET_MOVE(ch) / (int) GET_MAX_MOVE(ch)) * 100.0 + 0.5);
	act(buf, FALSE, ch, 0, 0, TO_CHAR);

}

void do_shout(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_INPUT_LENGTH + 80];
    struct descriptor_data *i;

    dlog("in do_shout");

    if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
        send_to_char("You can't shout!!\n\r", ch);
        return;
    }

    if (IS_NPC(ch) &&
        (Silence == 1) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
        send_to_char("Polymorphed shouting has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
        return;
    }

    if (apply_soundproof(ch)) {
        return;
	}

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
        if (!IS_IMMORTAL(ch->master)) {
            send_to_char("I don't think so :-)", ch->master);
            return;
        }
    }

    if ((GET_MOVE(ch) < 5 || GET_MANA(ch) < 5) && !IS_IMMORTAL(ch)) {
        send_to_char("You do not have the strength to shout!\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Shout? Yes! Fine! Shout we must, but WHAT??\n\r", ch);
    } else {
		if (argument[0] == '%') {
            do_OOCaction(ch, ++argument, cmd);
            return;
        } else if (argument[0] == '#') {
            do_OOCemote(ch, ++argument, cmd);
            return;
        }

        if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
            sprintf(buf1, "$c0009You shout '%s'", argument);
            act(buf1, FALSE, ch, 0, 0, TO_CHAR);
        }
        sprintf(buf1, "$c0009[$c0015$n$c0009] shouts '%s'", argument);
		act("$c0009[$c0015$n$c0009] lifts up $s head and shouts loudly",
            FALSE, ch, 0, 0, TO_ROOM);

        if (!IS_IMMORTAL(ch)) {
            GET_MOVE(ch) -= 5;
            GET_MANA(ch) -= 5;
        }

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                (IS_NPC(i->character) ||
                 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                  !IS_SET(i->character->specials.act, PLR_DEAF))) &&
                !check_soundproof(i->character)) {
                act(buf1, 0, ch, 0, i->character, TO_VICT);
            }
		}
    }
}

void do_yell(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_INPUT_LENGTH + 80];
    char            buf2[MAX_INPUT_LENGTH + 80];
    char            buf3[MAX_INPUT_LENGTH + 80];
    struct descriptor_data *i;

    dlog("in do_yell");

    if (cmd == 302) {
        send_to_char("Gossip?? Why don't ya try yelling.."
                     " They might hear ya better.\n\r", ch);
	}
    if (!IS_NPC(ch) && IS_SET(ch->specials.act, PLR_NOSHOUT)) {
        send_to_char("You can't shout, yell or auction.\n\r", ch);
        return;
    }

    if (IS_NPC(ch) && Silence == 1 && IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Polymorphed yelling has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
        return;
    }

    if (apply_soundproof(ch))  {
        return;
	}

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM) && !IS_IMMORTAL(ch->master)) {
        send_to_char("I don't think so :-)", ch->master);
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yell? Yes! but what!\n\r", ch);
    } else {
        if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
            sprintf(buf1, "$c0011You yell '%s'", argument);
            act(buf1, FALSE, ch, 0, 0, TO_CHAR);
        }

        sprintf(buf1, "$c0011[$c0015$n$c0011] yells '%s'", argument);
        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected &&
                (IS_NPC(i->character) ||
                 (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
                  !IS_SET(i->character->specials.act, PLR_NOGOSSIP))) && 
                !check_soundproof(i->character)) {

#ifdef ZONE_COMM_ONLY
                if (i->character->in_room != NOWHERE) {
                    if (real_roomp(ch->in_room)->zone ==
                        real_roomp(i->character->in_room)->zone &&
                        !IS_IMMORTAL(i->character) && !IS_IMMORTAL(ch)) {
                        act(buf1, 0, ch, 0, i->character, TO_VICT);
                    } else if (IS_IMMORTAL(ch)) {
                        sprintf(buf3,
                                "$c0011[$c0015$n$c0011] yells across "
                                "the world '%s'", argument);
                        act(buf3, 0, ch, 0, i->character, TO_VICT);
                    } else if (IS_IMMORTAL(i->character)) {
                        sprintf(buf2, "$c0011[$c0015$n$c0011] yells from zone "
                                      "%ld '%s'", real_roomp(ch->in_room)->zone,
                                      argument);
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

void do_commune(struct char_data *ch, char *argument, int cmd)
{
    char     buf[MAX_INPUT_LENGTH + 80];
    struct descriptor_data *i;

    dlog("in do_commune,think");

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Communing among the gods is fine, but WHAT?\n\r", ch);
        return;
	} 
    
    if (argument[0] == '%') {
        do_OOCaction(ch, ++argument, cmd);
        return;
    } 
    
    if (argument[0] == '#') {
        do_OOCemote(ch, ++argument, cmd);
        return;
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        ch_printf(ch, "$c0012You think '%s'", argument);
    }
    sprintf(buf, "$c0012::$c0015$n$c0012:: '%s'", argument);

    for (i = descriptor_list; i; i = i->next) {
        if (i->character != ch && !i->connected && !IS_NPC(i->character) &&
            !IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
            IS_IMMORTAL(i->character)) {
            act(buf, 0, ch, 0, i->character, TO_VICT);
        }
    }
}

void do_mobTell(struct char_data *ch, char *mob, char *sentence)
{
    char            buf[256];

    sprintf(buf, "$c0013[$c0015%s$c0013] tells you '%s'", mob, sentence);
    act(buf, FALSE, ch, 0, 0, TO_CHAR);
}

void do_mobTell2(struct char_data *ch, struct char_data *mob,
                 char *sentence)
{
    char            buf[256];

    sprintf(buf, "$c0013[$c0015$N$c0013] tells you '%s'", sentence);
    act(buf, FALSE, ch, 0, mob, TO_CHAR);
}

void do_tell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name,
                   *message;

    dlog("in do_tell");

    if (apply_soundproof(ch)) {
        return;
	}

    argument = get_argument(argument, &name);
    message = skip_spaces(argument);

    if (!name || !message) {
        send_to_char("Who do you wish to tell what??\n\r", ch);
        return;
    } 
    
    if (!(vict = get_char_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
        return;
    } 
    
    if (ch == vict) {
        send_to_char("You try to tell yourself something.\n\r", ch);
        return;
    } 
    
    if (GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
        act("$E is asleep, shhh.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } 
    
    if (IS_NPC(vict) && !(vict->desc)) {
        send_to_char("No-one by that name here..\n\r", ch);
        return;
    } 
    
    if (!IS_IMMORTAL(ch) && IS_SET(vict->specials.act, PLR_NOTELL)) {
        act("$N is not listening for tells right now.", FALSE, ch, 0, vict,
            TO_CHAR);
        return;
    } 
    
    if (IS_IMMORTAL(vict) && IS_IMMORTAL(ch) && 
        GetMaxLevel(ch) < GetMaxLevel(vict) && 
        IS_SET(vict->specials.act, PLR_NOTELL)) {
        act("$N is not listening for tells right now!", FALSE, ch, 0, vict,
            TO_CHAR);
        return;
    } 
    
    if (!vict->desc) {
        send_to_char("They can't hear you, link dead.\n\r", ch);
        return;
    }

    if (check_soundproof(vict) && !IS_IMMORTAL(ch)) {
        send_to_char("In a silenced room, try again later.\n\r", ch);
        return;
    }

#ifdef ZONE_COMM_ONLY
    if (real_roomp(ch->in_room)->zone != real_roomp(vict->in_room)->zone &&
        !IS_IMMORTAL(ch) && !IS_IMMORTAL(vict)) {
        send_to_char("That person is not near enough for you to tell.\n\r", ch);
        return;
    }
#endif

    ch_printf(vict, "$c0013[$c0015%s$c0013] tells you '%s'",
              (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);

    /*
     * Used for reply
     */
    vict->last_tell = GET_NAME(ch);

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        ch_printf(ch, "$c0013You tell %s %s'%s'",
                  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)),
                  (IS_AFFECTED2(vict, AFF2_AFK) ? "(who is AFK) " : ""),
                  message);
    }
}

void do_whisper(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name,
                   *message,
                    buf[MAX_INPUT_LENGTH + 80];

    dlog("in do_whisper");

    if (apply_soundproof(ch)) {
        return;
	}

    argument = get_argument(argument, &name);
    message = skip_spaces(argument);

    if (!name || !message) {
        send_to_char("Who do you want to whisper to.. and what??\n\r", ch);
    } else if (!(vict = get_char_room_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
    } else if (vict == ch) {
        act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("You can't seem to get your mouth close enough to your "
                     "ear...\n\r", ch);
    } else {
		if (check_soundproof(vict)) {
            return;
		}

        sprintf(buf, "$c0005[$c0015$n$c0005] whispers to you, '%s'", message);
        act(buf, FALSE, ch, 0, vict, TO_VICT);
        if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
            sprintf(buf, "$c0005You whisper to %s%s, '%s'",
                    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)),
                    (IS_AFFECTED2(vict, AFF2_AFK) ? " (who is AFK)" : ""),
                    message);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }
        act("$c0005$n whispers something to $N.", FALSE, ch, 0, vict,
            TO_NOTVICT);
    }
}

void do_ask(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name,
                   *message,
                    buf[MAX_INPUT_LENGTH + 80];

    dlog("in do_ask");

    if (apply_soundproof(ch)) {
        return;
	}

    argument = get_argument(argument, &name);
    message = skip_spaces(argument);

    if (!name || !message) {
        send_to_char("Who do you want to ask something.. and what??\n\r", ch);
    } else if (!(vict = get_char_room_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
    } else if (vict == ch) {
        act("$c0006[$c0015$n$c0006] quietly asks $mself a question.",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0006You think about it for a while...", FALSE, ch, 0, 0,
            TO_CHAR);
    } else {
        if (check_soundproof(vict)) {
            return;
		}

        sprintf(buf, "$c0006[$c0015$n$c0006] asks you '%s'", message);
        act(buf, FALSE, ch, 0, vict, TO_VICT);

        if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
            sprintf(buf, "$c0006You ask %s%s, '%s'",
                    (IS_NPC(vict) ? vict->player.name : GET_NAME(vict)),
                    (IS_AFFECTED2(vict, AFF2_AFK) ? " (who is AFK)" : ""),
                    message);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        act("$c0006$n asks $N a question.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
}

#define MAX_NOTE_LENGTH 1000
/*
 * arbitrary
 */

void do_write(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *paper = 0,
                   *pen = 0;
    char           *papername,
                   *penname;

    dlog("in do_write");

    if (!ch->desc) {
        return;
	}

    argument = get_argument(argument, &papername);
    argument = get_argument(argument, &penname);

    if (!papername) {
		/*
		 * nothing was delivered
		 */
        send_to_char("write (on) papername (with) penname.\n\r", ch);
        return;
    }

    if (!penname) {
        send_to_char("write (on) papername (with) penname.\n\r", ch);
        return;
    }

    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
        ch_printf(ch, "You have no %s.\n\r", papername);
        return;
    }

    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
        ch_printf(ch, "You have no %s.\n\r", penname);
        return;
    }

    /*
     * ok.. now let's see what kind of stuff we've found
     */
    if (pen->obj_flags.type_flag != ITEM_PEN) {
        act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
    } else if (paper->obj_flags.type_flag != ITEM_NOTE) {
        act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
    } else if (paper->action_description) {
        send_to_char("There's something written on it already.\n\r", ch);
        return;
    } else {
        /*
         * we can write - hooray!
         */
        send_to_char("Ok.. go ahead and write.. Use /? for help on editing "
                     "strings.\n\r", ch);
        act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
        ch->desc->str = &paper->action_description;
        ch->desc->max_str = MAX_NOTE_LENGTH;
    }
}

char           *RandomWord()
{
    static char    *rstring[] = {
        "argle",
        "bargle",
        "glop",
        "glyph",
        "hussamah",             /* 5 */
        "rodina",
        "mustafah",
        "angina",
        "the",
        "fribble",              /* 10 */
        "fnort",
        "frobozz",
        "zarp",
        "ripple",
        "yrk",                  /* 15 */
        "yid",
        "yerf",
        "oork",
        "grapple",
        "red",                  /* 20 */
        "blue",
        "you",
        "me",
        "ftagn",
        "hastur",               /* 25 */
        "brob",
        "gnort",
        "lram",
        "truck",
        "kill",                 /* 30 */
        "cthulhu",
        "huzzah",
        "acetacytacylic",
        "hydrooxypropyl",
        "summah",               /* 35 */
        "hummah",
        "cookies",
        "stan",
        "will",
        "wadapatang",           /* 40 */
        "pterodactyl",
        "frob",
        "yuma",
        "gumma",
        "lo-pan",               /* 45 */
        "sushi",
        "yaya",
        "yoyodine",
        "yaazr",
        "bipsnop",              /* 50 */
    };
    int rstringcount = NELEMENTS(rstring);

    /*
     * count max number in
     * array
     */
    return (rstring[number(0, rstringcount-1)]);
}

void do_sign(struct char_data *ch, char *argument, int cmd)
{
    char            buf[MAX_INPUT_LENGTH + 80];
    char            buf2[MAX_INPUT_LENGTH + 80];
    char           *p;
    int             diff;
    struct char_data *t;
    struct room_data *rp;

    dlog("in do_sign");

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yes, but WHAT do you want to sign?\n\r", ch);
        return;
    }

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return;
    }

    if (!HasHands(ch)) {
        send_to_char("Yeah right... WHAT HANDS!!!!!!!!\n\r", ch);
        return;
    }

    strcpy(buf, argument);
    buf2[0] = '\0';         /*
                             * work through the argument, word by
                             * word.  if you fail your skill roll, the
                             * word comes out garbled. */
    p = strtok(buf, " ");   /* first word */

    diff = strlen(buf);

    while (p) {
        if (ch->skills && 
            number(1, 75 + strlen(p)) < ch->skills[SKILL_SIGN].learned) {
            strcat(buf2, p);
        } else {
            strcat(buf2, RandomWord());
        }
        strcat(buf2, " ");
        diff -= 1;
        p = strtok(0, " ");
        /*
         * next word
         */
    }
    /*
     * if a recipient fails a roll, a word comes out garbled.
     */

    /*
     * buf2 is now the "corrected" string.
     */

    sprintf(buf, "$n signs '%s'", buf2);

    for (t = rp->people; t; t = t->next_in_room) {
        if (t != ch) {
            if (t->skills && number(1, diff) < t->skills[SKILL_SIGN].learned) {
                act(buf, FALSE, ch, 0, t, TO_VICT);
            } else {
                act("$n makes funny motions with $s hands",
                    FALSE, ch, 0, t, TO_VICT);
            }
        }
    }

    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        ch_printf(ch, "You sign '%s'\n\r", argument);
    }
}

/*
 * speak elvish, speak dwarvish, etc...
 */
void do_speak(struct char_data *ch, char *argument, int cmd)
{
    char           *arg;
    int             i;
    int             found;
    int             lang = -1;

    dlog("in do_speak");

    arg = skip_spaces(argument);
    if (!arg) {
        send_to_char("Speak what language?\n\r", ch);
        return;
    }

    
    for( i = 0, found = 1; i < languageCount && !found; i++ ) {
        if( languages[i].name && 
            !strncasecmp( arg, languages[i].name, strlen(arg) ) ) {

            found = 1;

            if( languages[i].langSkill || IS_IMMORTAL(ch) ) {
                lang = languages[i].langSpeaks;
            } else {
                send_to_char("Un-recognized language!\n\r", ch);
                return;
            }
        }
    }


    /*
     * set language that we're gonna speak
     */
    ch->player.speaks = lang;
    ch_printf(ch, "You concentrate on speaking %s.\n\r", 
                  languages[lang - 1].name);
}

/*
 * this is where we do the language says
 */
void do_new_say(struct char_data *ch, char *argument, int cmd)
{
    int             learned,
                    skill_num;
    char            buf[MAX_INPUT_LENGTH + 80];
    char            buf2[MAX_INPUT_LENGTH + 80];
    char            buf3[MAX_INPUT_LENGTH + 80];
    char           *p;
    int             diff;
    struct char_data *t;
    struct room_data *rp;

    dlog("in do_new_say");

    if (!argument)
        return;

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
        return;
    }

    if (apply_soundproof(ch)) {
        return;
    }

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return;
    }

    if (!ch->skills) {
        learned = 0;
        skill_num = LANG_COMMON;
    } else {
        /*
         * find the language we are speaking
         */

        switch (ch->player.speaks) {
        case SPEAK_COMMON:
            learned = ch->skills[LANG_COMMON].learned;
            skill_num = LANG_COMMON;
            break;
        case SPEAK_ELVISH:
            learned = ch->skills[LANG_ELVISH].learned;
            skill_num = LANG_ELVISH;
            break;
        case SPEAK_HALFLING:
            learned = ch->skills[LANG_HALFLING].learned;
            skill_num = LANG_HALFLING;
            break;
        case SPEAK_DWARVISH:
            learned = ch->skills[LANG_DWARVISH].learned;
            skill_num = LANG_DWARVISH;
            break;
        case SPEAK_ORCISH:
            learned = ch->skills[LANG_ORCISH].learned;
            skill_num = LANG_ORCISH;
            break;
        case SPEAK_GIANTISH:
            learned = ch->skills[LANG_GIANTISH].learned;
            skill_num = LANG_GIANTISH;
            break;
        case SPEAK_OGRE:
            learned = ch->skills[LANG_OGRE].learned;
            skill_num = LANG_OGRE;
            break;
        case SPEAK_GNOMISH:
            learned = ch->skills[LANG_GNOMISH].learned;
            skill_num = LANG_GNOMISH;
            break;
        case SPEAK_ALL:
            learned = 100;
            skill_num = LANG_COMMON;
            break;
        default:
            learned = ch->skills[LANG_COMMON].learned;
            skill_num = LANG_COMMON;
            break;
        }
    }

    strcpy(buf, argument);
    buf2[0] = '\0';

    /*
     * we use this for ESP and immortals and comprehend lang
     */
    sprintf(buf3, "$c0015[$c0005$n$c0015] says '%s'", buf);

    /*
     * work through the argument, word by word.  if you fail your
     * skill roll, the word comes out garbled.
     */
    p = strtok(buf, " ");
    /*
     * first word
     */

    diff = strlen(buf);

    while (p) {
        if (number(1, 75 + strlen(p)) < learned || IS_IMMORTAL(ch)) {
            strcat(buf2, p);
        } else {
            /*
             * add case statement here to use random words from clips
             * of elvish, dwarvish etc so the words look like they came
             * from that language
             */
            strcat(buf2, RandomWord());
        }
        strcat(buf2, " ");
        diff -= 1;
        p = strtok(0, " ");
        /*
         * next word
         */
    }
    /*
     * if a recipient fails a roll, a word comes out garbled.
     */

    /*
     * buf2 is now the "corrected" string.
     */
    if (!buf2 || !*buf2) {
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
        return;
    }

    sprintf(buf, "$c0015[$c0005$n$c0015] says '%s'", buf2);

    for (t = rp->people; t; t = t->next_in_room) {
        if (t != ch) {
            if ((t->skills) && 
                (number(1, diff) < t->skills[skill_num].learned || 
                 IS_IMMORTAL(t) || IS_NPC(t) || 
                 affected_by_spell(t, SKILL_ESP) || 
                 affected_by_spell(t, SPELL_COMP_LANGUAGES) || 
                 ch->player.speaks == SPEAK_ALL)) {
                /*
                 * these guys always understand
                 */
                if (IS_IMMORTAL(t)  || IS_NPC(t) ||
                    affected_by_spell(t, SKILL_ESP) ||
                    affected_by_spell(t, SPELL_COMP_LANGUAGES) ||
                    IS_NPC(t) || ch->player.speaks == SPEAK_ALL) {
                    act(buf3, FALSE, ch, 0, t, TO_VICT);
                } else {
                    act(buf, FALSE, ch, 0, t, TO_VICT);
                }
            } else {
                act("$c0010$n speaks in a language you can't quite "
                    "understand.", FALSE, ch, 0, t, TO_VICT);
            }
        }
    }

    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        sprintf(buf, "$c0015You say '%s'", argument);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
}

void do_gtell(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *k;
    struct follow_type *f;
    char            buf[MAX_STRING_LENGTH];

    dlog("in do_gtell");

    if (apply_soundproof(ch)) {
        return;
	}

    argument = skip_spaces(argument);

    if (!argument) {
        send_to_char("What do you want to group tell!??\n\r", ch);
        return;
    }

    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("But you are a member of no group?!\n\r", ch);
        return;
    }

    if (ch->master) {
        k = ch->master;
    } else {
        k = ch;
    }
    for (f = k->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP)) {
            if (!f->follower->desc) {
                /*
                 * link dead
                 */
            } else if (ch == f->follower) {
                /*
                 * can't tell yourself!
                 */
            } else if (!check_soundproof(f->follower)) {
                sprintf(buf,
                        "$c0012[$c0015%s$c0012] group tells you '%s'",
                        (IS_NPC(ch) ? ch->player.
                         short_descr : GET_NAME(ch)), argument);
                act(buf, FALSE, f->follower, 0, 0, TO_CHAR);
            }
        }
    }

    /*
     * send to master now
     */
    if (ch->master) {
        if (IS_AFFECTED(ch->master, AFF_GROUP)) {
            if (!ch->master->desc) {
                /*
                 * link dead
                 */
            } else if (ch == ch->master) {
                /*
                 * can't tell yourself!
                 */
            } else if (!check_soundproof(ch->master)) {
                sprintf(buf,
                        "$c0012[$c0015%s$c0012] group tells you '%s'",
                        (IS_NPC(ch) ? ch->player.
                         short_descr : GET_NAME(ch)), argument);
                act(buf, FALSE, ch->master, 0, 0, TO_CHAR);
            }
        }
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        sprintf(buf, "$c0012You group tell '%s'", argument);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
}


void do_split(struct char_data *ch, char *argument, int cmd)
{

    bool            is_same_group(struct char_data *ach,
                                  struct char_data *bch);

    char            buf[MAX_STRING_LENGTH + 40];
    char           *arg;
    struct char_data *gch;
    int             members,
                    amount,
                    share,
                    extra;

    dlog("in do_split");

    argument = get_argument(argument, &arg);

    if (!arg) {
        send_to_char("Split how much?\n\r", ch);
        return;
    }

    amount = atoi(arg);

    if (amount < 0) {
        send_to_char("Your group wouldn't like that.\n\r", ch);
        return;
    }

    if (amount == 0) {
        send_to_char("You hand out zero coins, but no one notices.\n\r", ch);
        return;
    }

    if (ch->points.gold < amount) {
        send_to_char("You don't have that much gold.\n\r", ch);
        return;
    }

    members = 0;
    for (gch = real_roomp(ch->in_room)->people; gch != NULL;
         gch = gch->next_in_room) {
        if (is_same_group(gch, ch)){
            members++;
		}
    }

    if (members < 2) {
        send_to_char("Just keep it all.\n\r", ch);
        return;
    }

    share = amount / members;
    extra = amount % members;

    if (share == 0) {
        send_to_char("Don't even bother, cheapskate.\n\r", ch);
        return;
    }

    ch->points.gold -= amount;
    ch->points.gold += share + extra;

    ch_printf(ch, "You split %d gold coins.  Your share is %d gold coins.\n\r",
                  amount, share + extra);

    sprintf(buf, "$n splits %d gold coins.  Your share is %d gold coins.",
                 amount, share);

    for (gch = real_roomp(ch->in_room)->people; gch != NULL;
         gch = gch->next_in_room) {
        if (gch != ch && is_same_group(gch, ch)) {
            act(buf, FALSE, ch, NULL, gch, TO_VICT);
            gch->points.gold += share;
        }
    }
}

void do_pray(struct char_data *ch, char *argument, int cmd)
{
    struct affected_type af;
    char            buf1[MAX_INPUT_LENGTH + 80];
    struct descriptor_data *i;
    int             ii = 0;

    dlog("in do_pray");

    if (IS_NPC(ch)) {
        return;
	}
    if (affected_by_spell(ch, SPELL_PRAYER)) {
        send_to_char("You have already prayed today.\n\r", ch);
        return;
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Pray to a deity, but what?!?! (pray <DeityName> "
                     "<prayer>)\n\r", ch);
        return;
    }

    ii = ((int) GetMaxLevel(ch) * 3.5);
    if (HasClass(ch, CLASS_CLERIC | CLASS_DRUID)) {
        ii += 10;
    }

    if (ii > number(1, 101)) {
        if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
            ch_printf(ch, "You pray '%s'\n\r", argument);
        }

        sprintf(buf1, "$c0014:*:$c0012$n$c0014:*: prays '$c0012%s$c0014'",
                      argument);

        for (i = descriptor_list; i; i = i->next) {
            if (i->character != ch && !i->connected && 
                !IS_SET(i->character->specials.act, PLR_NOSHOUT) && 
                IS_IMMORTAL(i->character)) {
                act(buf1, 0, ch, 0, i->character, TO_VICT);
            }
        }
    } else {
        send_to_char("Your prayer is ignored at this time.\n\r", ch);
    }

    af.type = SPELL_PRAYER;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

bool is_same_group(struct char_data * ach, struct char_data * bch)
{
    if (!IS_AFFECTED(ach, AFF_GROUP) || !IS_AFFECTED(bch, AFF_GROUP)) {
        return 0;
	}
    if (ach->master != NULL) {
        ach = ach->master;
	}
    if (bch->master != NULL) {
        bch = bch->master;
	}
    return (ach == bch);
}

void do_telepathy(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name,
                   *message,
                    buf[MAX_INPUT_LENGTH + 80];

    dlog("in do_telepathy");

    if (!HasClass(ch, CLASS_PSI) && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
        send_to_char("What do you think you are? A Telepath?\n\r", ch);
        return;
    }

    if (GET_MANA(ch) < 5 && !IS_AFFECTED(ch, AFF_TELEPATHY)) {
        send_to_char("You do not have the mental power to bespeak anyone.\n\r",
                     ch);
        return;
    }

    argument = get_argument(argument, &name);
    message = skip_spaces(argument);

    if (!name || !message) {
        send_to_char("Who do you wish to bespeak what??\n\r", ch);
        return;
    } else if (!(vict = get_char_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
        return;
    } else if (ch == vict) {
        send_to_char("You try to bespeak yourself something.\n\r", ch);
        return;
    } else if (GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
        act("$E is asleep, shhh.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } else if (IS_NPC(vict) && !(vict->desc)) {
        send_to_char("No-one by that name here..\n\r", ch);
        return;
    } else if (!IS_IMMORTAL(ch) && IS_SET(vict->specials.act, PLR_NOTELL)) {
        act("$N is not listening for thoughts right now.", FALSE, ch, 0,
            vict, TO_CHAR);
        return;
    } else if (IS_IMMORTAL(vict) && IS_IMMORTAL(ch) && 
               GetMaxLevel(ch) < GetMaxLevel(vict) && 
               IS_SET(vict->specials.act, PLR_NOTELL)) {
        act("$N is not listening for thoughts right now!", FALSE, ch, 0,
            vict, TO_CHAR);
        return;
    } else if (!vict->desc) {
        send_to_char("They can't hear you, link dead.\n\r", ch);
        return;
    }

    if (!IS_AFFECTED(ch, AFF_TELEPATHY)) {
        GET_MANA(ch) -= 5;
	}

    sprintf(buf, "$c0013[$c0015%s$c0013] bespeaks you '%s'",
            (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), message);
    act(buf, FALSE, vict, 0, 0, TO_CHAR);

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        sprintf(buf, "$c0013You bespeak %s '%s'",
                (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)),
                message);
        act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
}

void do_ooc(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_INPUT_LENGTH + 80];
    struct descriptor_data *i;

    dlog("in do_ooc");

    if (!IS_NPC(ch) && (IS_SET(ch->specials.act, PLR_NOSHOUT) || 
                        IS_SET(ch->specials.act, PLR_NOOOC) || 
                        IS_SET(ch->specials.act, PLR_WIZNOOOC))) {
        send_to_char("You can't use this command!!\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOOOC)) {
        send_to_char("The use of OOC have been temporarilly banned.\n\r", ch);
        return;
    }

    if (IS_NPC(ch) && Silence == 1 && IS_SET(ch->specials.act, ACT_POLYSELF)) {
        send_to_char("Polymorphed worlwide comms has been banned.\n\r"
                     "It may return after a bit.\n\r", ch);
        return;
    }

    if ((GET_MOVE(ch) < 5 || GET_MANA(ch) < 5) && (!IS_IMMORTAL(ch))) {
        send_to_char("You do not have the strength to shout!\n\r", ch);
        return;
    }

    if (apply_soundproof(ch)) {
        return;
	}

    if (ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
        if (!IS_IMMORTAL(ch->master)) {
            send_to_char("I don't think so :-)", ch->master);
            return;
        }
    }

    argument = skip_spaces(argument);

    if (!argument) {
        send_to_char("Hrm... normally, you should OOC something...\n\r", ch);
        return;
    }

    if (*argument == '%') {
        do_OOCaction(ch, ++argument, cmd);
        return;
    } 
    
    if (*argument == '#') {
        do_OOCemote(ch, ++argument, cmd);
        return;
    }

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        ch_printf(ch, "$c0012You $c0015OOC$c0012 '$c0015%s$c0012'", argument);
    }

    sprintf(buf1, "$c0012-=$c0015$n$c0012=- OOCs '$c0015%s$c0012'", argument);

    if (!IS_IMMORTAL(ch)) {
        GET_MOVE(ch) -= 5;
        GET_MANA(ch) -= 5;
    }

    for (i = descriptor_list; i; i = i->next) {
        if (i->character != ch && !i->connected &&
            (IS_NPC(i->character) || 
             (!IS_SET(i->character->specials.act, PLR_NOSHOUT) && 
              !IS_SET(i->character->specials.act, PLR_NOOOC) &&
              !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) && 
            !check_soundproof(i->character)) {
            act(buf1, 0, ch, 0, i->character, TO_VICT);
        }
    }
}

void do_OOCemote(struct char_data *ch, char *argument, int cmd)
{
    char            buf1[MAX_INPUT_LENGTH + 80],
                    command[100];
    int             CMD_OOC = 497,
                    CMD_GOSSIP = 302,
                    CMD_SHOUT = 18;
    struct descriptor_data *i;

    if (cmd == CMD_OOC) {
        sprintf(command, "$c000B[$c000WOOC$c000B]$c000w");
    } else if (cmd == CMD_SHOUT) {
        sprintf(command, "$c000R[$c000WSHOUT$c000R]$c000w");
    } else if (cmd == CMD_GOSSIP) {
        sprintf(command, "$c000Y[$c000WGOSSIP$c000Y]$c000w");
    } else {
        sprintf(command, "$c000p[$c000WWORLD$c000p]$c000w");
	}
    argument[0] = ' ';

    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        sprintf(buf1, "%s $n%s", command, argument);
        act(buf1, FALSE, ch, 0, 0, TO_CHAR);
    }

    sprintf(buf1, "%s $n%s$c0012", command, argument);

    if (!IS_IMMORTAL(ch)) {
        GET_MOVE(ch) -= 5;
        GET_MANA(ch) -= 5;
    }

    for (i = descriptor_list; i; i = i->next) {
        if (i->character != ch && !i->connected &&
            (IS_NPC(i->character) ||
             (!IS_SET(i->character->specials.act, PLR_NOSHOUT) &&
              !IS_SET(i->character->specials.act, PLR_NOOOC) &&
              !IS_SET(i->character->specials.act, PLR_WIZNOOOC))) &&
            !check_soundproof(i->character)) {
            act(buf1, 0, ch, 0, i->character, TO_VICT);
        }
    }
}

void do_reply(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name;

    dlog("in do_reply");

    if (apply_soundproof(ch)) {
        return;
	}

	if (!argument || !*argument) {
        send_to_char("Did they leave you speechless?\n\r", ch);
        return;
    }

    name = NULL;
    if( ch->last_tell ) {
        name = strdup(ch->last_tell);
    }

    if(!name || !strcasecmp(name, GET_NAME(ch))) {
        send_to_char("You would be replying to yourself, how strange.", ch);
        free(name);
        return;
    }

    if (!(vict = get_char(name))) {
        send_to_char("They seem to have left...", ch);
        free(name);
        return;
    }

	if (!(vict = get_char_vis(ch, name))) {
        send_to_char("They seem to have left...", ch);
        free(name);
        return;
    }

    free(name);

    if (GET_POS(vict) == POSITION_SLEEPING && !IS_IMMORTAL(ch)) {
        act("$E is asleep, shhh.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } else if (IS_NPC(vict) && !(vict->desc)) {
        send_to_char("You can't reply to them... strange...\n\r", ch);
        return;
    } else if (!vict->desc) {
        send_to_char("They can't hear you, link dead.\n\r", ch);
        return;
    }

	if (check_soundproof(vict)) {
        send_to_char("In a silenced room, try again later.\n\r", ch);
        return;
    }
    
#ifdef ZONE_COMM_ONLY
    if (real_roomp(ch->in_room)->zone != real_roomp(vict->in_room)->zone &&
        !IS_IMMORTAL(ch) && !IS_IMMORTAL(vict)) {
        send_to_char("That person is not near enough for you to reply.\n\r",
                     ch);
        return;
    }
#endif

    ch_printf(vict, "$c0013[$c0015%s$c0013] replies to you '%s'",
              (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)), argument);
	vict->last_tell = GET_NAME(ch);

   /*
	* Used for reply
	*/
    if (IS_NPC(ch) || IS_SET(ch->specials.act, PLR_ECHO)) {
        ch_printf(ch, "$c0013You reply to %s %s'%s'",
                  (IS_NPC(vict) ? vict->player.short_descr : GET_NAME(vict)),
                  (IS_AFFECTED2(vict, AFF2_AFK) ? "(who is AFK) " : ""),
                  argument);
    }
}

void do_talk(struct char_data *ch, char *argument, int cmd)
{
    struct char_data *vict;
    char           *name;

    dlog("in do_talk");

    if (apply_soundproof(ch)) {
        return;
	}

    argument = get_argument(argument, &name);

    if (!name) {
        send_to_char("Who do you want to talk to??\n\r", ch);
    } else if (!(vict = get_char_room_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
    } else if (vict == ch) {
        act("$n whispers quietly to $mself.", FALSE, ch, 0, 0, TO_ROOM);
        send_to_char("Don't you do that enough already?\n\r", ch);
    } else {
        if (check_soundproof(vict)) {
            return;
		}

        if (IS_NPC(vict)) {
            if (strcmp(vict->specials.talks, "")) {
                act("$n strikes up a conversation with $N.", FALSE, ch, 0,
                    vict, TO_ROOM);
                act("You strike up a conversation with $N.", FALSE, ch, 0,
                    vict, TO_CHAR);
                send_to_room(vict->specials.talks, ch->in_room);
            } else {
                act("$n tries to strike up a conversation with $N.", FALSE,
                    ch, 0, vict, TO_ROOM);
                act("You try to strike up a conversation with $N, but $E has "
                    "nothing to tell you.", FALSE, ch, 0, vict, TO_CHAR);
            }
        } else {
            send_to_char("This is a means of talking to mobiles. Please use "
                         "tell/say/shout/etc", ch);
		}
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
