#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"

#define INQ_SHOUT 1
#define INQ_LOOSE 0

#define SWORD_ANCIENTS 25000
/*
 *  list of room #s
 */
#define Elf_Home     1414
#define Bakery       3009
#define Dump         3030
#define Ivory_Gate    1499


#define CMD_SAY 17
#define CMD_ASAY 169

#define TONGUE_ITEM 22

#define GAIN 243

#define PRISON_ROOM 2639
#define PRISON_LET_OUT_ROOM 2640

#define HOLDING_MAX  10         /* max mobs that can be in tank :) */
#define HOLDING_TANK 60         /* room number to drop the mobs in */

/*
 * external vars
 */


/*
 * Data declarations
 */

struct social_type {
    char           *cmd;
    int             next_line;
};

struct memory {
    short           pointer;
    char          **names;
    int            *status;
    short           index;
    short           c;
};


int affect_status(struct memory *mem, struct char_data *ch,
                  struct char_data *t, int aff_status);
void            printmap(struct char_data *ch, int x, int y, int sizex,
                         int sizey);
struct obj_data *SailDirection(struct obj_data *obj, int direction);
int             CanSail(struct obj_data *obj, int direction);


extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int      top_of_world;
extern struct int_app_type int_app[26];
extern char    *pc_class_types[];
extern char    *dirs[];
extern struct QuestItem QuestList[4][IMMORTAL];
extern int      gSeason;        /* what season is it ? */

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
extern char    *spells[];
extern int      rev_dir[];




/*************************************/
/*
 * predicates for find_path function
 */

/**
 * @todo Fix for 64bit
 * @todo Is this even necessary?
 */
int is_target_room_p(int room, void *tgt_room)
{
    /* AMD64: Fix me */
    return( room == (int)(long) tgt_room );
}

/**
 * @todo Is this even necessary?
 */
int named_object_on_ground(int room, void *c_data)
{
    char           *name = c_data;
    return( (objectGetInRoom(name, real_roomp(room))) != NULL );
}

/*
 * predicates for find_path function
 */
/*************************************/

/*
 ********************************************************************
 *  Special procedures for rooms                                    *
 ******************************************************************** */

struct char_data *FindMobInRoomWithFunction(int room, int (*func) ())
{
    struct char_data *temp_char,
                   *targ;

    targ = 0;

    if (room > NOWHERE) {
        for (temp_char = real_roomp(room)->people; (!targ) && (temp_char);
             temp_char = temp_char->next_in_room) {
            if (IS_MOB(temp_char) && mob_index[temp_char->nr].func == func) {
                targ = temp_char;
            }
        }
    } else {
        return (NULL);
    }
    return (targ);
}



struct char_data *find_mobile_here_with_spec_proc(int (*fcn) (), int rnumber)
{
    struct char_data *temp_char;

    for (temp_char = real_roomp(rnumber)->people; temp_char;
         temp_char = temp_char->next_in_room) {
        if (IS_MOB(temp_char) && mob_index[temp_char->nr].func == fcn) {
            return temp_char;
        }
    }
    return NULL;
}

/*
 *******************************************************************
 *  General special procedures for mobiles                         *
 *******************************************************************
 */

/*
 * SOCIAL GENERAL PROCEDURES
 *
 * If first letter of the command is '!' this will mean that the following
 * command will be executed immediately.
 *
 * "G",n : Sets next line to n "g",n : Sets next line relative to n, fx.
 * line+=n "m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5 "w",n : Wake
 * up and set standing (if possible) "c<txt>",n : Look for a person named
 * <txt> in the room "o<txt>",n : Look for an object named <txt> in the
 * room "r<int>",n : Test if the npc in room number <int>? "s",n : Go to
 * sleep, return false if can't go sleep "e<txt>",n : echo <txt> to the
 * room, can use $o/$p/$N depending on contents of the **thing "E<txt>",n :
 * Send <txt> to person pointed to by thing "B<txt>",n : Send <txt> to
 * room, except to thing "?<num>",n : <num> in [1..99]. A random chance of
 * <num>% success rate. Will as usual advance one line upon sucess, and
 * change relative n lines upon failure. "O<txt>",n : Open <txt> if in
 * sight. "C<txt>",n : Close <txt> if in sight. "L<txt>",n : Lock <txt> if
 * in sight. "U<txt>",n : Unlock <txt> if in sight.
 */

/*
 * Execute a social command.
 */
void exec_social(struct char_data *npc, char *cmd, int next_line,
                 int *cur_line, void **thing)
{
    bool            ok;
    char           *arg;


    if (GET_POS(npc) == POSITION_FIGHTING) {
        return;
    }
    ok = TRUE;

    if( cmd && cmd[1] ) {
        arg = strdup(&cmd[1]);
    } else {
        arg = NULL;
    }

    switch (*cmd) {

    case 'G':
        *cur_line = next_line;
        break;

    case 'g':
        *cur_line += next_line;
        break;

    case 'e':
        if (arg) {
            act(arg, FALSE, npc, *thing, *thing, TO_ROOM);
        }
        break;

    case 'E':
        if (arg) {
            act(arg, FALSE, npc, 0, *thing, TO_VICT);
        }
        break;

    case 'B':
        if (arg) {
            act(arg, FALSE, npc, 0, *thing, TO_NOTVICT);
        }
        break;

    case 'm':
        if (arg) {
            do_move(npc, NULL, *arg - '0' + 1);
        }
        break;

    case 'w':
        if (GET_POS(npc) != POSITION_SLEEPING) {
            ok = FALSE;
        } else {
            GET_POS(npc) = POSITION_STANDING;
        }
        break;

    case 's':
        if (GET_POS(npc) <= POSITION_SLEEPING) {
            ok = FALSE;
        } else {
            GET_POS(npc) = POSITION_SLEEPING;
        }
        break;

    case 'c':
        /*
         * Find char in room
         */
        if (arg) {
            *thing = get_char_room_vis(npc, arg);
            ok = (*thing != NULL);
        }
        break;

    case 'o':
        /*
         * Find object in room
         */
        if (arg) {
            *thing = objectGetInRoom(npc, arg, real_roomp(npc->in_room));
            ok = (*thing != NULL);
        }
        break;

    case 'r':
        /*
         * Test if in a certain room
         */
        if (arg) {
            ok = (npc->in_room == atoi(arg));
        }
        break;

    case 'O':
        /*
         * Open something
         */
        if (arg) {
            do_open(npc, arg, 0);
        }
        break;

    case 'C':
        /*
         * Close something
         */
        if (arg) {
            do_close(npc, arg, 0);
        }
        break;

    case 'L':
        /*
         * Lock something
         */
        if (arg) {
            do_lock(npc, arg, 0);
        }
        break;

    case 'U':
        /*
         * UnLock something
         */
        if (arg) {
            do_unlock(npc, arg, 0);
        }
        break;

    case '?':
        /*
         * Test a random number
         */
        if (arg) {
            if (atoi(arg) <= number(1, 100)) {
                ok = FALSE;
            }
        }
        break;

    default:
        break;
    }

    free(arg);

    if (ok) {
        (*cur_line)++;
    } else {
        (*cur_line) += next_line;
    }
}

void npc_steal(struct char_data *ch, struct char_data *victim)
{
    int             gold;

    if (IS_NPC(victim) || GetMaxLevel(victim) > MAX_MORT) {
        return;
    }
    if (AWAKE(victim) && !number(0, GetMaxLevel(ch))) {
        act("You discover that $n has $s hands in your wallet.", FALSE, ch,
            0, victim, TO_VICT);
        act("$n tries to steal gold from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
        /*
         * Steal some gold coins
         */
        gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
        if (gold > 0) {
            GET_GOLD(ch) += gold;
            GET_GOLD(victim) -= gold;
        }
    }
}


#if 0

int GameGuard(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{

    if (!cmd) {
        if (ch->specials.fighting) {
            fighter(ch, cmd, arg, mob, type);
        }
    }

    if (cmd == 4) {             /* West is field */
        if ((IS_AFFECTED(ch, AFF_TEAM_GREY)) ||
            (IS_AFFECTED(ch, AFF_TEAM_AMBER))) {
            send_to_char("The guard wishes you good luck on the field.\n\r",
                         ch);
            return (FALSE);
        } else {
            send_to_char("The guard shakes his head, and blocks your way.\n\r",
                         ch);
            act("The guard shakes his head, and blocks $n's way.", TRUE,
                ch, 0, 0, TO_ROOM);
            send_to_char("The guard says 'Your not a player! You can't enter "
                         "the field!'.\n\r", ch);
            return (TRUE);
        }
        return (FALSE);
    } else
        return (FALSE);
}

int GreyParamedic(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int types)
{
    struct char_data *vict,
                   *most_hurt;

    if (!cmd) {
        if (ch->specials.fighting) {
            return (cleric(ch, 0, "", mob, types));
        } else {
            if (GET_POS(ch) == POSITION_STANDING) {

                /*
                 * Find a dude to do good things upon !
                 */

                most_hurt = real_roomp(ch->in_room)->people;
                for (vict = real_roomp(ch->in_room)->people; vict;
                     vict = vict->next_in_room) {
                    if (((float) GET_HIT(vict) / (float) hit_limit(vict) <
                         (float) GET_HIT(most_hurt) /
                         (float) hit_limit(most_hurt))
                        && (CAN_SEE(ch, vict)))
                        most_hurt = vict;
                }
                if (!most_hurt)
                    return (FALSE);     /* nobody here */
                if (IS_AFFECTED(most_hurt, AFF_TEAM_GREY)) {

                    if ((float) GET_HIT(most_hurt) /
                        (float) hit_limit(most_hurt) > 0.66) {
                        if (number(0, 5) == 0) {
                            act("$n shrugs helplessly.", 1, ch, 0, 0,
                                TO_ROOM);
                        }
                        return TRUE;    /* not hurt enough */
                    }

                    if (!check_soundproof(ch)) {
                        if (number(0, 4) == 0) {
                            if (most_hurt != ch) {
                                act("$n looks at $N.", 1, ch, 0, most_hurt,
                                    TO_NOTVICT);
                                act("$n looks at you.", 1, ch, 0,
                                    most_hurt, TO_VICT);
                            }

                            if (check_nomagic(ch, 0, 0))
                                return (TRUE);

                            act("$n utters the words 'judicandus dies'.",
                                1, ch, 0, 0, TO_ROOM);
                            cast_cure_light(GetMaxLevel(ch), ch, "",
                                            SPELL_TYPE_SPELL, most_hurt,
                                            0);
                            return (TRUE);
                        }
                    }
                } else {        /* Other Team? */
                    if (IS_AFFECTED(most_hurt, AFF_TEAM_AMBER)) {

                        if (check_nomagic(ch, 0, 0))
                            return (TRUE);

                        act("$n utters the words 'die punk'.",
                            1, ch, 0, 0, TO_ROOM);
                        cast_cause_light(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, most_hurt, 0);
                        return (TRUE);
                    } else
                        return (FALSE);
                }
            } else {            /* I'm asleep or sitting */
                return (FALSE);
            }
        }
    }
    return (FALSE);
}

int AmberParamedic(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict,
                   *most_hurt;

    if (!cmd) {
        if (ch->specials.fighting) {
            return (cleric(ch, 0, "", mob, type));
        } else {
            if (GET_POS(ch) == POSITION_STANDING) {

                /*
                 * Find a dude to do good things upon !
                 */

                most_hurt = real_roomp(ch->in_room)->people;
                for (vict = real_roomp(ch->in_room)->people; vict;
                     vict = vict->next_in_room) {
                    if (((float) GET_HIT(vict) / (float) hit_limit(vict) <
                         (float) GET_HIT(most_hurt) /
                         (float) hit_limit(most_hurt))
                        && (CAN_SEE(ch, vict)))
                        most_hurt = vict;
                }
                if (!most_hurt)
                    return (FALSE);     /* nobody here */
                if (IS_AFFECTED(most_hurt, AFF_TEAM_AMBER)) {

                    if ((float) GET_HIT(most_hurt) /
                        (float) hit_limit(most_hurt) > 0.66) {
                        if (number(0, 5) == 0) {
                            act("$n shrugs helplessly.", 1, ch, 0, 0,
                                TO_ROOM);
                        }
                        return TRUE;    /* not hurt enough */
                    }

                    if (!check_soundproof(ch)) {
                        if (number(0, 4) == 0) {
                            if (most_hurt != ch) {
                                act("$n looks at $N.", 1, ch, 0, most_hurt,
                                    TO_NOTVICT);
                                act("$n looks at you.", 1, ch, 0,
                                    most_hurt, TO_VICT);
                            }

                            if (check_nomagic(ch, 0, 0))
                                return (TRUE);

                            act("$n utters the words 'judicandus dies'.",
                                1, ch, 0, 0, TO_ROOM);
                            cast_cure_light(GetMaxLevel(ch), ch, "",
                                            SPELL_TYPE_SPELL, most_hurt,
                                            0);
                            return (TRUE);
                        }
                    }
                } else {        /* Other Team? */
                    if (IS_AFFECTED(most_hurt, AFF_TEAM_GREY)) {

                        if (check_nomagic(ch, 0, 0))
                            return (TRUE);

                        act("$n utters the words 'die punk'.",
                            1, ch, 0, 0, TO_ROOM);
                        cast_cause_light(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, most_hurt, 0);
                        return (TRUE);
                    } else
                        return (FALSE);
                }
            } else {            /* I'm asleep or sitting */
                return (FALSE);
            }
        }
    }
    return (FALSE);
}
#endif

int MidgaardCitizen(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (check_soundproof(ch)) {
            return (FALSE);
        }
        if (!number(0, 18)) {
            command_interpreter(ch, "shout Guards! Help me! Please!");
        } else {
            act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0,
                TO_ROOM);
        }

        if (ch->specials.fighting) {
            CallForGuard(ch, ch->specials.fighting, 3, MIDGAARD);
        }
        return (TRUE);
    }

    return (FALSE);
}



int WizardGuard(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *tch,
                   *evil;
    int             max_evil;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);
        CallForGuard(ch, ch->specials.fighting, 9, MIDGAARD);
    }
    max_evil = 1000;
    evil = 0;

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (tch->specials.fighting && GET_ALIGNMENT(tch) < max_evil &&
            (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
            max_evil = GET_ALIGNMENT(tch);
            evil = tch;
        }
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0) &&
        !check_peaceful(ch, "")) {
        if (!check_soundproof(ch)) {
            act("$n screams 'DEATH!!!!!!!!'", FALSE, ch, 0, 0, TO_ROOM);
        }
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }
    return (FALSE);
}



int DracoLich(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    return( FALSE );
}



/*
 ********************************************************************
 *  Special procedures for mobiles                                      *
 ******************************************************************** */


int Inquisitor(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (fighter(ch, cmd, arg, mob, type));
    }

    switch (ch->generic) {
    case INQ_SHOUT:
        if (!check_soundproof(ch)) {
            command_interpreter(ch, "shout NOONE expects the Spanish "
                                    "Inquisition!");
        }
        ch->generic = 0;
        break;
    default:
        break;
    }
    return (FALSE);
}

int puff(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    struct char_data *i,
                   *tmp_ch;
    char            buf[80];

    if (type == EVENT_DWARVES_STRIKE) {
        command_interpreter(ch, "shout Ack! Of all the stupid things! Those "
                                "damned dwarves are on strike again!");
        return (TRUE);
    }

    if (type == EVENT_END_STRIKE) {
        command_interpreter(ch, "shout Gee, about time those dwarves stopped "
                                "striking!");
        return (TRUE);
    }

    if (type == EVENT_DEATH) {
        command_interpreter(ch, "shout Ack! I've been killed! Have some God "
                                "Load me again!!!");
        return (TRUE);
    }

    if (type == EVENT_SPRING) {
        command_interpreter(ch, "shout Ahhh, spring is in the air.");
        return (TRUE);
    }

    if (cmd) {
        return (FALSE);
    }
    if (ch->generic == 1) {
        command_interpreter(ch, "shout When will we get there?");
        ch->generic = 0;
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }

    switch (number(0, 250)) {
    case 0:
        command_interpreter(ch, "say Anyone know where I am at?\n");
        return (TRUE);
    case 1:
        command_interpreter(ch, "say How'd all those fish get up here?");
        return (TRUE);
    case 2:
        command_interpreter(ch, "say I'm a very female dragon.");
        return (TRUE);
    case 3:
        command_interpreter(ch, "say Haven't I seen you at the Temple?");
        return (TRUE);
    case 4:
        command_interpreter(ch, "shout Bring out your dead, bring out your "
                                "dead!");
        return (TRUE);
    case 5:
        command_interpreter(ch, "emote gropes you.");
        return (TRUE);
    case 6:
        command_interpreter(ch, "emote gives you a long and passionate kiss.  "
                                "It seems to last forever.");
        return (TRUE);
    case 7:
        for (i = character_list; i; i = i->next) {
            if (!IS_NPC(i) && !number(0, 5)) {
                if (!strcmp(GET_NAME(i), "Celestian")) {
                    command_interpreter(ch, "shout Celestian, come ravish me "
                                            "now!");
                } else if (!strcmp(GET_NAME(i), "Fiona")) {
                    command_interpreter(ch, "shout I'm Puff the PMS dragon!");
                } else if (!strcmp(GET_NAME(i), "Stranger")) {
                    command_interpreter(ch, "shout People are strange, when "
                                            "they're with Stranger!");
                } else if (!strcmp(GET_NAME(i), "God")) {
                    command_interpreter(ch, "shout God!  Theres only room for "
                                            "one smartass robot on this mud!");
                } else if (GET_SEX(i) == SEX_MALE) {
                    sprintf(buf, "say Hey, %s, how about some MUDSex?",
                            GET_NAME(i));
                    command_interpreter(ch, buf);
                } else {
                    sprintf(buf, "say I'm much prettier than %s, don't you "
                                 "think?", GET_NAME(i));
                    command_interpreter(ch, buf);
                }
            }
            break;
        }
        return (TRUE);
    case 8:
        command_interpreter(ch, "say Celestian is my hero!");
        return (TRUE);
    case 9:
        command_interpreter(ch, "say So, wanna neck?");
        return (TRUE);
    case 10:
        tmp_ch = (struct char_data *) FindAnyVictim(ch);
        if (IS_NPC(ch)) {
            return (FALSE);
        }

        sprintf(buf, "say Party on, %s", GET_NAME(tmp_ch));
        command_interpreter(ch, buf);
        return (TRUE);
    case 11:
        if (!number(0, 30)) {
            command_interpreter(ch, "shout NOT!!!");
        }
        return (TRUE);
    case 12:
        command_interpreter(ch, "say Bad news.  Termites.");
        return (TRUE);
    case 13:
        for (i = character_list; i; i = i->next) {
            if (!IS_NPC(i) && !number(0, 30)) {
                sprintf(buf, "force %s shout I love Celestian!", GET_NAME(i));
                command_interpreter(ch, buf);

                sprintf(buf, "force %s bounce", GET_NAME(i));
                command_interpreter(ch, buf);

                sprintf(buf, "restore %s", GET_NAME(i));
                command_interpreter(ch, buf);
                return (TRUE);
            }
        }
        return (TRUE);
    case 14:
        command_interpreter(ch, "say I'll be back.");
        return (TRUE);
    case 15:
        command_interpreter(ch, "say Aren't wombat's so cute?");
        return (TRUE);
    case 16:
        command_interpreter(ch, "emote fondly fondles you.");
        return (TRUE);
    case 17:
        command_interpreter(ch, "emote winks at you.");
        return (TRUE);
    case 18:
        command_interpreter(ch, "say This mud is too silly!");
        return (TRUE);
    case 19:
        command_interpreter(ch, "say If the Mayor is in a room alone,");
        command_interpreter(ch, "say Does he say 'Good morning citizens.'?");
        return (TRUE);
    case 20:
        for (i = character_list; i; i = i->next) {
            if (!IS_NPC(i) && !number(0, 30)) {
                sprintf(buf, "shout Top of the morning to you %s!", 
                        GET_NAME(i));
                command_interpreter(ch, buf);
                return (TRUE);
            }
        }
        break;
    case 21:
        for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
            if (!IS_NPC(i) && !number(0, 3)) {
                sprintf(buf, "say Pardon me, %s, but are those bugle boy jeans "
                             "you are wearing?", GET_NAME(i));
                command_interpreter(ch, buf);
                return (TRUE);
            }
        }
        break;
    case 22:
        for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
            if (!IS_NPC(i) && !number(0, 3)) {
                sprintf(buf, "say Pardon me, %s, but do you have any Grey "
                             "Poupon?", GET_NAME(i));
                command_interpreter(ch, buf);
                return (TRUE);
            }
        }
        break;
    case 23:
        if (number(0, 80) == 0) {
            command_interpreter(ch, "shout Where are we going?");
            ch->generic = 1;
        }
        break;
    case 24:
        command_interpreter(ch, "say Blackstaff is a wimp!");
        return (TRUE);
        break;
    case 25:
        command_interpreter(ch, "say Better be nice or I will user spellfire "
                                "on you!");
        return (TRUE);
        break;
    case 26:
        if (number(0, 100) == 0) {
            command_interpreter(ch, "shout What is the greatest joy?");
        }
        break;
    case 27:
        command_interpreter(ch, "say Have you see Elminster? Gads he is "
                                "slow...");
        return (TRUE);
    case 28:
        if (number(0, 50)) {
            command_interpreter(ch, "shout SAVE!  I'm running out of cute "
                                    "things to say!");
        }
        command_interpreter(ch, "force all save");
        return (TRUE);
    case 29:
        command_interpreter(ch, "say I hear Strahd is a really mean vampire.");
        return (TRUE);
    case 30:
        command_interpreter(ch, "say I heard there was a sword that would kill"
                                " frost giants.");
        return (TRUE);
    case 31:
        command_interpreter(ch, "say Hear about the sword that kills Red "
                                "Dragons?");
        return (TRUE);
    case 32:
        if (number(0, 100) == 0) {
            command_interpreter(ch, "shout Help yourself and help a newbie!");
            return (TRUE);
        }
        break;
    case 33:
        if (number(0, 100) == 0) {
             command_interpreter(ch, "shout Kill all other dragons!");
            return (TRUE);
        }
        break;
    case 34:
        if (number(0, 50) == 0) {
            for (i = character_list; i; i = i->next) {
                if (mob_index[i->nr].func == Inquisitor) {
                    command_interpreter(ch, "shout I wasn't expecting the "
                                            "Spanish Inquisition!");
                    i->generic = INQ_SHOUT;
                    return (TRUE);
                }
            }
            return (TRUE);
        }
        break;
    case 35:
        command_interpreter(ch, "say Are you crazy, is that your problem?");
        return (TRUE);
    case 36:
        for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
            if (!IS_NPC(i) && !number(0, 3)) {
                sprintf(buf, "say %s, do you think I'm going bald?", 
                        GET_NAME(i));
                command_interpreter(ch, buf);
                return (TRUE);
            }
        }
        break;
    case 37:
        command_interpreter(ch, "say This is your brain.");
        command_interpreter(ch, "say This is MUD.");
        command_interpreter(ch, "say This is your brain on MUD.");
        command_interpreter(ch, "say Any questions?");
        return (TRUE);
    case 38:
        for (i = character_list; i; i = i->next) {
            if (!IS_NPC(i) && !number(0, 20) && i->in_room != NOWHERE) {
                sprintf(buf, "force %s save", GET_NAME(i));
                command_interpreter(ch, buf);
            }
        }
        return (TRUE);
    case 39:
        command_interpreter(ch, "say I'm Puff the Magic Dragon, who the hell "
                                "are you?");
        return (TRUE);
    case 40:
        command_interpreter(ch, "say Attention all planets of the Solar "
                                "Federation!");
        command_interpreter(ch, "say We have assumed control.");
        return (TRUE);
    case 41:
        if (!number(0, 50)) {
            command_interpreter(ch, "shout We need more explorers!");
            return (TRUE);
        }
        break;
    case 42:
        if (!number(0, 50)) {
            command_interpreter(ch, "shout Pray to Celestian, he might be in a"
                                    " good mood!");
            return (TRUE);
        }
        break;
    case 43:
        command_interpreter(ch, "say Pardon me boys, is this the road to Great"
                                " Cthulhu?");
        return (TRUE);
    case 44:
        command_interpreter(ch, "say May the Force be with you... Always.");
        return (TRUE);
    case 45:
        command_interpreter(ch, "say Eddies in the space time continuum.");
        return (TRUE);
    case 46:
        command_interpreter(ch, "say Quick!  Reverse the polarity of the "
                                "neutron flow!");
        return (TRUE);
    case 47:
        if (!number(0, 50)) {
            command_interpreter(ch, "shout Someone pray to Blackmouth, he is "
                                    "lonely.");
            return (TRUE);
        }
        break;
    case 48:
        command_interpreter(ch, "say Shh...  I'm beta testing.  I need "
                                "complete silence!");
        return (TRUE);
    case 49:
        command_interpreter(ch, "say Do you have any more of that Plutonium "
                                "Nyborg!");
        return (TRUE);
    case 50:
        command_interpreter(ch, "say I'm the real implementor, you know.");
        return (TRUE);
    case 51:
        command_interpreter(ch, "emote moshes into you almost causing you to "
                                "fall.");
        return (TRUE);
    case 52:
        if (!number(0, 30)) {
            command_interpreter(ch, "shout Everybody pray to Ator!");
        }
        return (TRUE);
    case 53:
        command_interpreter(ch, "say You know I always liked you the best "
                                "don't you?");
        command_interpreter(ch, "emote winks seductively at you.");
        return (TRUE);
    case 54:
        if (!number(0, 30)) {
            command_interpreter(ch, "shout Ack! Who prayed to Wert!");
        }
        return (TRUE);

    default:
        return (FALSE);
    }
    return( FALSE );
}


int replicant(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob1, int type)
{
    struct char_data *mob;

    if (cmd) {
        return FALSE;
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
        act("Drops of $n's blood hit the ground, and spring up into another "
            "one!", TRUE, ch, 0, 0, TO_ROOM);
        mob = read_mobile(ch->nr, REAL);
        char_to_room(mob, ch->in_room);
        act("Two undamaged opponents face you now.", TRUE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch) = GET_MAX_HIT(ch);
    }

    return FALSE;
}

#define TYT_NONE 0
#define TYT_CIT  1
#define TYT_WHAT 2
#define TYT_TELL 3
#define TYT_HIT  4

int Tytan(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    struct char_data *vict;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (magic_user(ch, cmd, arg, mob, type));
    }

    switch (ch->generic) {
    case TYT_NONE:
        if ((vict = FindVictim(ch))) {
            ch->generic = TYT_CIT;
            SetHunting(ch, vict);
        }
        break;
    case TYT_CIT:
        if (ch->specials.hunting) {
            if (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
                REMOVE_BIT(ch->specials.act, ACT_AGGRESSIVE);
            }
            if (ch->in_room == ch->specials.hunting->in_room) {
                act("Where is the Citadel?", TRUE, ch, 0, 0, TO_ROOM);
                ch->generic = TYT_WHAT;
            }
        } else {
            ch->generic = TYT_NONE;
        }
        break;
    case TYT_WHAT:
        if (ch->specials.hunting) {
            if (ch->in_room == ch->specials.hunting->in_room) {
                act("What must we do?", TRUE, ch, 0, 0, TO_ROOM);
                ch->generic = TYT_TELL;
            }
        } else {
            ch->generic = TYT_NONE;
        }
        break;
    case TYT_TELL:
        if (ch->specials.hunting) {
            if (ch->in_room == ch->specials.hunting->in_room) {
                act("Tell Us!  Command Us!", TRUE, ch, 0, 0, TO_ROOM);
                ch->generic = TYT_HIT;
            }
        } else {
            ch->generic = TYT_NONE;
        }
        break;
    case TYT_HIT:
        if (ch->specials.hunting) {
            if (ch->in_room == ch->specials.hunting->in_room) {
                if (!check_peaceful(ch, "The Tytan screams in anger")) {
                    hit(ch, ch->specials.hunting, TYPE_UNDEFINED);
                    if (!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
                        SET_BIT(ch->specials.act, ACT_AGGRESSIVE);
                    }
                    ch->generic = TYT_NONE;
                } else {
                    ch->generic = TYT_CIT;
                }
            }
        } else {
            ch->generic = TYT_NONE;
        }
        break;
    default:
        ch->generic = TYT_NONE;
    }
    return (FALSE);
}

int AbbarachDragon(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *targ;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!ch->specials.fighting) {
        targ = (struct char_data *) FindAnyVictim(ch);
        if (targ && !check_peaceful(ch, "")) {
            hit(ch, targ, TYPE_UNDEFINED);
            act("You have now payed the price of crossing.",
                TRUE, ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    } else {
        return (BreathWeapon(ch, cmd, arg, mob, type));
    }
    return( FALSE );
}




#if 0
#define WW_LOOSE 0
#define WW_FOLLOW 1

int Whirlwind(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    struct char_data *tmp;
    const char     *names[] = { "Loki", "Belgarath", 0 };
    int             i = 0;

    if (ch->in_room == -1)
        return (FALSE);

    if (cmd == 0 && ch->generic == WW_LOOSE) {
        for (tmp = real_roomp(ch->in_room)->people; tmp;
             tmp = tmp->next_in_room) {
            while (names[i]) {
                if (!strcmp(GET_NAME(tmp), names[i])
                    && ch->generic == WW_LOOSE) {
                    /*
                     * start following
                     */
                    if (circle_follow(ch, tmp))
                        return (FALSE);
                    if (ch->master)
                        stop_follower(ch);
                    add_follower(ch, tmp);
                    ch->generic = WW_FOLLOW;
                }
                i++;
            }
        }
        if (ch->generic == WW_LOOSE && !cmd) {
            act("The $n suddenly dissispates into nothingness.", 0, ch, 0,
                0, TO_ROOM);
            extract_char(ch);
        }
    }
}
#endif


int WarrenGuard(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *tch,
                   *good;
    int             max_good;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);
        return (TRUE);
    }

    max_good = -1000;
    good = 0;

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (tch->specials.fighting && GET_ALIGNMENT(tch) > max_good &&
            (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
            max_good = GET_ALIGNMENT(tch);
            good = tch;
        }
    }

    if (good && GET_ALIGNMENT(good->specials.fighting) <= 0) {
        if (!check_soundproof(ch)) {
            act("$n screams 'DEATH TO GOODY-GOODIES!!!!'",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        hit(ch, good, TYPE_UNDEFINED);
        return (TRUE);
    }

    return (FALSE);
}

int zm_tired(struct char_data *zmaster)
{
    return (GET_HIT(zmaster) < GET_MAX_HIT(zmaster) / 2 ||
            GET_MANA(zmaster) < 40);
}

int zm_stunned_followers(struct char_data *zmaster)
{
    struct follow_type *fwr;

    for (fwr = zmaster->followers; fwr; fwr = fwr->next) {
        if (GET_POS(fwr->follower) == POSITION_STUNNED) {
            return TRUE;
        }
    }
    return FALSE;
}

void zm_init_combat(struct char_data * zmaster, struct char_data * target)
{
    struct follow_type *fwr;
    for (fwr = zmaster->followers; fwr; fwr = fwr->next) {
        if (IS_AFFECTED(fwr->follower, AFF_CHARM) &&
            fwr->follower->specials.fighting == NULL &&
            fwr->follower->in_room == target->in_room) {
            if (GET_POS(fwr->follower) == POSITION_STANDING) {
                hit(fwr->follower, target, TYPE_UNDEFINED);
            } else if (GET_POS(fwr->follower) > POSITION_SLEEPING &&
                       GET_POS(fwr->follower) < POSITION_FIGHTING) {
                do_stand(fwr->follower, NULL, -1);
            }
        }
    }
}

int zm_kill_fidos(struct char_data *zmaster)
{
    struct char_data *fido_b;

    fido_b = find_mobile_here_with_spec_proc(fido, zmaster->in_room);
    if (fido_b) {
        if (!check_soundproof(zmaster)) {
            act("$n shrilly screams 'Kill that carrion beast!'", FALSE,
                zmaster, 0, 0, TO_ROOM);
            zm_init_combat(zmaster, fido_b);
        }
        return TRUE;
    }
    return FALSE;
}

int zm_kill_aggressor(struct char_data *zmaster)
{
    struct follow_type *fwr;

    if (zmaster->specials.fighting && !check_soundproof(zmaster)) {
        act("$n bellows 'Kill that mortal that dares lay hands on me!'",
            FALSE, zmaster, 0, 0, TO_ROOM);
        zm_init_combat(zmaster, zmaster->specials.fighting);
        return TRUE;
    }

    for (fwr = zmaster->followers; fwr; fwr = fwr->next) {
        if (fwr->follower->specials.fighting &&
            IS_AFFECTED(fwr->follower, AFF_CHARM)) {
            if (!check_soundproof(zmaster)) {
                act("$n bellows 'Assist your brethren, my loyal servants!'",
                    FALSE, zmaster, 0, 0, TO_ROOM);
                zm_init_combat(zmaster, fwr->follower->specials.fighting);
                return TRUE;
            }
        }
    }
    return FALSE;
}


#define ZM_MANA 10
#define ZM_NEMESIS 3060

int zombie_master(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct obj_data *temp1;
    struct char_data *zmaster;
    int             dir;

    zmaster = find_mobile_here_with_spec_proc(zombie_master, ch->in_room);

    if (cmd != 0 || ch != zmaster || !AWAKE(ch)) {
        return FALSE;
    }
    if (!check_peaceful(ch, "") && (zm_kill_fidos(zmaster) ||
                                    zm_kill_aggressor(zmaster))) {
        do_stand(zmaster, NULL, -1);
        return TRUE;
    }

    switch (GET_POS(zmaster)) {
    case POSITION_RESTING:
        if (!zm_tired(zmaster)) {
            do_stand(zmaster, NULL, -1);
        }
        break;
    case POSITION_SITTING:
        if (!zm_stunned_followers(zmaster)) {
            if (!check_soundproof(ch)) {
                act("$n says 'It took you long enough...'", FALSE,
                    zmaster, 0, 0, TO_ROOM);
            }
            do_stand(zmaster, NULL, -1);
        }
        break;
    case POSITION_STANDING:
        if (zm_tired(zmaster)) {
            do_rest(zmaster, NULL, -1);
            return TRUE;
        }

        temp1 = objectGetInRoom(zmaster, "corpse", 
                                real_roomp(zmaster->in_room));

        if (temp1) {
            if (GET_MANA(zmaster) < ZM_MANA) {
                if (dice(1, 20) == 1 && !check_soundproof(ch)) {
                    act("$n says 'So many bodies, so little time' and sighs.",
                        FALSE, zmaster, 0, 0, TO_ROOM);
                }
            } else {
                if (check_nomagic(ch, 0, 0)) {
                    return (FALSE);
                }
                if (!check_soundproof(ch)) {
                    act("$n says 'Wonderful, another loyal follower!' and "
                        "grins maniacly.", FALSE, zmaster, 0, 0, TO_ROOM);
                    GET_MANA(zmaster) -= ZM_MANA;
                    spell_animate_dead(GetMaxLevel(zmaster), ch, NULL, temp1);
                    if (zmaster->followers) {
                        /*
                         * assume the new follower is top of the list?
                         */
                        AddHatred(zmaster->followers->follower, OP_VNUM,
                                  ZM_NEMESIS);
                    }
                }
            }
            return TRUE;
        } else if (zm_stunned_followers(zmaster)) {
            do_sit(zmaster, NULL, -1);
            return TRUE;
        } else if (dice(1, 20) == 1) {
            act("$n searches for bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
            return TRUE;
        } else if ((dir = find_path(zmaster->in_room, named_object_on_ground,
                                    "corpse", -200, 0)) >= 0) {
            go_direction(zmaster, dir);
            return TRUE;
        } else if (dice(1, 5) == 1) {
            act("$n can't find any bodies.", FALSE, zmaster, 0, 0, TO_ROOM);
            return TRUE;
        } else {
            mobile_wander(zmaster);
        }
    }

    return FALSE;
}



/*
 ********************************************************************
 *  Special procedures for objects                                     *
 ******************************************************************** */

#define CHAL_ACT \
"You are torn out of reality!\n\r"\
"You roll and tumble through endless voids for what seems like eternity...\n\r"\
"\n\r"\
"After a time, a new reality comes into focus... you are elsewhere.\n\r"

int chalice(struct char_data *ch, int cmd, char *arg)
{
    struct obj_data *chalice;
    char           *buf1,
                   *buf2;
    static int      chl,
                    achl;

    /*
     * 222 is the normal chalice, 223 is chalice-on-altar
     */

    chl = 222;
    achl = 223;

    switch (cmd) {
    case 10:
        /*
         * get
         */
        if (!(chalice = get_obj_in_list_num(chl, 
                                        real_roomp(ch->in_room)->contents)) &&
            CAN_SEE_OBJ(ch, chalice)) {
            if (!(chalice = get_obj_in_list_num(achl,
                                        real_roomp(ch->in_room)->contents)) &&
                CAN_SEE_OBJ(ch, chalice)) {
                return (0);
            }
        }

        /*
         * we found a chalice.. now try to get us
         */
        do_get(ch, arg, cmd);

        /*
         * if got the altar one, switch her
         */
        if (chalice == get_obj_in_list_num(achl, ch->carrying)) {
            objectExtract(chalice);
            chalice = objectRead(chl);
            objectGiveToChar(chalice, ch);
        }
        return (TRUE);
        break;
    case 67:
        /*
         * put
         */
        if (!(chalice = get_obj_in_list_num(chl, ch->carrying))) {
            return (FALSE);
        }
        arg = get_argument(arg, &buf1);
        arg = get_argument(arg, &buf2);

        if (buf1 && buf2 && !strcasecmp(buf1, "chalice") && 
            !strcasecmp(buf2, "altar")) {
            objectExtract(chalice);
            chalice = objectRead(achl);
            objectPutInRoom(chalice, ch->in_room);
            send_to_char("Ok.\n\r", ch);
        }
        return (TRUE);
        break;
    case 176:
        /*
         * pray
         */
        if (!(chalice = get_obj_in_list_num(achl, 
                                        real_roomp(ch->in_room)->contents))) {
            return (FALSE);
        }

        do_action(ch, arg, cmd);
        /*
         * pray
         */
        send_to_char(CHAL_ACT, ch);
        objectExtract(chalice);
        act("$n is torn out of existence!", TRUE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, 2500);
        /*
         * before the fiery gates
         */
        do_look(ch, NULL, 15);
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}

int kings_hall(struct char_data *ch, int cmd, char *arg)
{
    if (cmd != 176) {
        return (FALSE);
    }
    do_action(ch, arg, 176);

    send_to_char("You feel as if some mighty force has been offended.\n\r", ch);
    send_to_char(CHAL_ACT, ch);

    act("$n is struck by an intense beam of light and vanishes.",
        TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, 1420);
    /*
     * behind the altar
     */
    do_look(ch, NULL, 15);
    return (TRUE);
}


/*
 * house routine for saved items.
 */

int House(struct char_data *ch, int cmd, char *arg, struct room_data *rp,
          int type)
{
    char            buf[100];
    struct obj_cost cost;
    int             i,
                    save;
    int             count = 0;

    if (IS_NPC(ch)) {
        return (FALSE);
    }
    /*
     * if (cmd != rent) ignore
     */
    if (cmd != 92) {
        return (FALSE);
    }

    /*
     * verify the owner
     */
    if (strncmp(GET_NAME(ch), real_roomp(ch->in_room)->name,
                strlen(GET_NAME(ch)))) {
        send_to_char("Sorry, you'll have to find your own house.\n\r", ch);
        return (FALSE);
    }

    cost.total_cost = 0;
    cost.no_carried = 0;
    cost.ok = TRUE;

    add_obj_cost(ch, 0, ch->carrying, &cost);
    count = CountLims(ch->carrying);
    for (i = 0; i < MAX_WEAR; i++) {
        add_obj_cost(ch, 0, ch->equipment[i], &cost);
        count += CountLims(ch->equipment[i]);
    }

    if (!cost.ok) {
        return (FALSE);
    }

#ifdef LIMITED_ITEMS
    if (count > MaxLimited(GetMaxLevel(ch))) {
        send_to_char("I'm sorry, but you to many limited items.\n\r", ch);
        return (FALSE);
    }
#endif

    sprintf(buf, "It will cost you %d coins per day\n\r", cost.total_cost);
    send_to_char(buf, ch);

    objectSaveForChar(ch, &cost, 1);
    save = ch->in_room;

    if (ch->specials.start_room != 2) {
        ch->specials.start_room = save;
    }
    /*
     * CHARACTERS aren't freed by this
     */
    extract_char(ch);
    save_char(ch, save);
    ch->in_room = save;
    return( TRUE );
}

/***********************************************************************

                           CHESSBOARD PROCS

 ***********************************************************************/


int jabberwocky(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING &&
            GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        } else {
            FighterMove(ch);
        }
    }
    return (FALSE);
}

int flame(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING &&
            GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        } else {
            FighterMove(ch);
        }
    }
    return (FALSE);
}

int banana(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    if (!cmd) {
        return (FALSE);
    }
    if (cmd >= 1 && cmd <= 6 && GET_POS(ch) == POSITION_STANDING &&
        !IS_NPC(ch)) {
        if (!saves_spell(ch, SAVING_PARA)) {
            act("$N tries to leave, but slips on a banana and falls.",
                TRUE, ch, 0, ch, TO_NOTVICT);
            act("As you try to leave, you slip on a banana.",
                TRUE, ch, 0, ch, TO_VICT);
            GET_POS(ch) = POSITION_SITTING;
            return (TRUE);
        }
        return (FALSE);
    }
    return (FALSE);
}

int paramedics(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict,
                   *most_hurt;

    if (!cmd) {
        if (ch->specials.fighting) {
            return (cleric(ch, 0, "", mob, type));
        }

        if (GET_POS(ch) == POSITION_STANDING) {
            /*
             * Find a dude to do good things upon !
             */
            most_hurt = real_roomp(ch->in_room)->people;
            for (vict = real_roomp(ch->in_room)->people; vict;
                 vict = vict->next_in_room) {
                if (((float) GET_HIT(vict) / (float) hit_limit(vict) <
                     (float) GET_HIT(most_hurt) /
                     (float) hit_limit(most_hurt)) && (CAN_SEE(ch, vict))) {
                    most_hurt = vict;
                }
            }

            if (!most_hurt) {
                /*
                 * nobody here
                 */
                return (FALSE);
            }

            if ((float) GET_HIT(most_hurt) /
                (float) hit_limit(most_hurt) > 0.66) {
                if (number(0, 5) == 0) {
                    act("$n shrugs helplessly in unison.", 1, ch, 0, 0,
                        TO_ROOM);
                }
                /*
                 * not hurt enough
                 */
                return TRUE;
            }

            if (!check_soundproof(ch) && !number(0, 4)) {
                if (most_hurt != ch) {
                    act("$n looks at $N.", 1, ch, 0, most_hurt, TO_NOTVICT);
                    act("$n looks at you.", 1, ch, 0, most_hurt, TO_VICT);
                }

                if (check_nomagic(ch, 0, 0)) {
                    return (FALSE);
                }
                act("$n utters the words 'judicandus dies' in unison.", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, most_hurt, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}


char    *elf_comm[] = {
    "wake", "yawn",
    "stand", "say Well, back to work.", "get all",
    "eat bread", "wink",
    "w", "w", "s", "s", "s", "d", "open gate", "e",     /* home to gate */
    "close gate",
    "e", "e", "e", "e", "n", "w", "n",  /* gate to baker */
    "give all.bread baker",     /* pretend to give a bread */
    "give all.pastry baker",    /* pretend to give a pastry */
    "say That'll be 33 coins, please.",
    "echo The baker gives some coins to the Elf",
    "wave",
    "s", "e", "n", "n", "e", "drop all.bread", "drop all.pastry",
    "w", "s", "s",              /* to main square */
    "s", "w", "w", "w", "w",    /* back to gate */
    "pat sisyphus",
    "open gate", "w", "close gate", "u", "n", "n", "n", "e", "e", /* to home */
    "say Whew, I'm exhausted.", "rest", "$"
};

#define ELF_INIT     0
#define ELF_RESTING  1
#define ELF_GETTING  2
#define ELF_DELIVERY 3
#define ELF_DUMP 4
#define ELF_RETURN_TOWER   5
#define ELF_RETURN_HOME    6

int delivery_elf(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    int             dir;

    if (cmd) {
        return (FALSE);
    }
    return (FALSE);

    if (ch->specials.fighting) {
        return FALSE;
    }
    switch (ch->generic) {

    case ELF_INIT:
        if (ch->in_room != 0 && ch->in_room != Elf_Home) {
            if (GET_POS(ch) == POSITION_SLEEPING) {
                do_wake(ch, NULL, 0);
                do_stand(ch, NULL, 0);
            }
            command_interpreter(ch, "say Woah! How did i get here!");
            command_interpreter(ch, "emote waves his arm, and vanishes!");
            char_from_room(ch);
            char_to_room(ch, Elf_Home);

            command_interpreter(ch, "emote arrives with a Bamf!");
            command_interpreter(ch, "emote yawns");
            do_sleep(ch, NULL, 0);
        }
        ch->generic = ELF_RESTING;
        return (FALSE);
        break;

    case ELF_RESTING:
        if ((time_info.hours > 6) && (time_info.hours < 9)) {
            do_wake(ch, NULL, 0);
            do_stand(ch, NULL, 0);
            ch->generic = ELF_GETTING;
        }
        return (FALSE);
        break;

    case ELF_GETTING:
        command_interpreter(ch, "get all.loaf");
        command_interpreter(ch, "get all.biscuit");
        ch->generic = ELF_DELIVERY;
        return (FALSE);
        break;
    case ELF_DELIVERY:
        if (ch->in_room != Bakery) {
            dir = choose_exit_global(ch->in_room, Bakery, -100);
            if (dir < 0) {
                ch->generic = ELF_INIT;
                return (FALSE);
            } else {
                go_direction(ch, dir);
            }
        } else {
            command_interpreter(ch, "give 6*biscuit baker");
            command_interpreter(ch, "give 6*loaf baker");
            command_interpreter(ch, "say That'll be 33 coins, please.");
            ch->generic = ELF_DUMP;
        }
        return (FALSE);
        break;
    case ELF_DUMP:
        if (ch->in_room != Dump) {
            dir = choose_exit_global(ch->in_room, Dump, -100);
            if (dir < 0) {
                ch->generic = ELF_INIT;
                return (FALSE);
            } else {
                go_direction(ch, dir);
            }
        } else {
            command_interpreter(ch, "drop 10*biscuit");
            command_interpreter(ch, "drop 10*loaf");
            ch->generic = ELF_RETURN_TOWER;
        }
        return (FALSE);
        break;
    case ELF_RETURN_TOWER:
        if (ch->in_room != Ivory_Gate) {
            dir = choose_exit_global(ch->in_room, Ivory_Gate, -200);
            if (dir < 0) {
                ch->generic = ELF_INIT;
                return (FALSE);
            } else {
                go_direction(ch, dir);
            }
        } else {
            ch->generic = ELF_RETURN_HOME;
        }
        return (FALSE);
        break;
    case ELF_RETURN_HOME:
        if (ch->in_room != Elf_Home) {
            dir = choose_exit_global(ch->in_room, Elf_Home, -200);
            if (dir < 0) {
                ch->generic = ELF_INIT;
                return (FALSE);
            } else {
                go_direction(ch, dir);
            }
        } else if (time_info.hours > 21) {
            command_interpreter(ch, "say Done at last!");
            do_sleep(ch, NULL, 0);
            ch->generic = ELF_RESTING;
        } else {
            command_interpreter(ch, "say An elf's work is never done.");
            ch->generic = ELF_GETTING;
        }
        return (FALSE);
        break;
    default:
        ch->generic = ELF_INIT;
        return (FALSE);
    }
}

int delivery_beast(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct obj_data *o;

    if (cmd) {
        return (FALSE);
    }
    return (FALSE);

    if (time_info.hours == 6) {
        command_interpreter(ch, "drop all.loaf");
        command_interpreter(ch, "drop all.biscuit");
    } else if (time_info.hours < 2) {
        if (!number(0, 1)) {
            o = objectRead(3012);
            objectGiveToChar(o, ch);
        } else {
            o = objectRead(3013);
            objectGiveToChar(o, ch);
        }
    } else if (GET_POS(ch) > POSITION_SLEEPING) {
        do_sleep(ch, NULL, 0);
    }
}




/*
 **  NEW THALOS MOBS:******************************************************
 */


int NewThalosCitizen(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (!check_soundproof(ch)) {
            if (number(0, 18) == 0) {
                command_interpreter(ch, "shout Guards! Help me! Please!");
            } else {
                act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0,
                    0, TO_ROOM);
            }

            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, NEWTHALOS);
            }
            return (TRUE);
        }
    }

    return (FALSE);
}


/*
 * New improved magic_user
 */

int magic_user2(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!ch->specials.fighting) {
        return FALSE;
    }
    if (GET_POS(ch) > POSITION_STUNNED && GET_POS(ch) < POSITION_FIGHTING) {
        StandUp(ch);
        return (TRUE);
    }

    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    lspell = number(0, GetMaxLevel(ch));

    if (lspell < 1) {
        lspell = 1;
    }
    if (vict != ch->specials.fighting && lspell > 19 &&
        !IS_AFFECTED(ch, AFF_FIRESHIELD)) {
        act("$n utters the words 'Fireshield'.", 1, ch, 0, 0, TO_ROOM);
        cast_fireshield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return TRUE;
    }

    switch (lspell) {
    case 1:
        act("$n utters the words 'Magic Missile'.", 1, ch, 0, 0, TO_ROOM);
        cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 2:
        act("$n utters the words 'Shocking Grasp'.", 1, ch, 0, 0, TO_ROOM);
        cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 3:
    case 4:
        act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
        cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 5:
        act("$n utters the words 'Burning Hands'.", 1, ch, 0, 0, TO_ROOM);
        cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 6:
        if (!IS_AFFECTED(vict, AFF_SANCTUARY)) {
            act("$n utters the words 'Dispel Magic'.", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
        } else {
            act("$n utters the words 'Chill Touch'.", 1, ch, 0, 0, TO_ROOM);
            cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
        }
        break;
    case 7:
        act("$n utters the words 'Ice Storm'.", 1, ch, 0, 0, TO_ROOM);
        cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 8:
        act("$n utters the words 'Blindness'.", 1, ch, 0, 0, TO_ROOM);
        cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 9:
        act("$n utters the words 'Fear'.", 1, ch, 0, 0, TO_ROOM);
        cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 10:
    case 11:
        act("$n utters the words 'Lightning Bolt'.", 1, ch, 0, 0, TO_ROOM);
        cast_lightning_bolt(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                            vict, 0);
        break;
    case 12:
    case 13:
        act("$n utters the words 'Color Spray'.", 1, ch, 0, 0, TO_ROOM);
        cast_colour_spray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 14:
        act("$n utters the words 'Cone Of Cold'.", 1, ch, 0, 0, TO_ROOM);
        cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
        act("$n utters the words 'Fireball'.", 1, ch, 0, 0, TO_ROOM);
        cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;

    default:
        act("$n utters the words 'frag'.", 1, ch, 0, 0, TO_ROOM);
        cast_meteor_swarm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;

    }

    return TRUE;
}

/******************Mordilnia citizens************************************/





char           *lattimore_descs[] = {
    "A small orc is trying to break into a locker.\n\r",
    "A small orc is walking purposefully down the hall.\n\r",
    "An orc is feeding it's face with rat stew.\n\r",
    "A small orc is cowering underneath a bunk\n\r",
    "An orc sleeps restlessly on a bunk.\n\r",
    "There is an orc stading on a barrel here.\n\r",
    "An orc is traveling down the corridor at high speed.\n\r"
};

#define Lattimore_Initialize  0
#define Lattimore_Lockers     1
#define Lattimore_FoodRun     2
#define Lattimore_Eating      3
#define Lattimore_GoHome      4
#define Lattimore_Hiding      5
#define Lattimore_Sleeping    6
#define Lattimore_Run         7
#define Lattimore_Item        8

#define Kitchen   21310
#define Barracks  21277
#define Storeroom 21319
#define Conf      21322
#define Trap      21335
#define EarthQ    21334

#define CrowBar   21114
#define PostKey   21150


int lattimore(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    struct memory  *mem;
    struct char_data *latt,
                   *t;
    struct obj_data *obj;
    char           *obj_name,
                   *player_name;
    int             dir;
    int             (*Lattimore) ();

    if (!cmd) {
        if (!ch->act_ptr) {
            ch->act_ptr = (struct memory *) malloc(sizeof(struct memory));
            mem = ch->act_ptr;
            mem->pointer = mem->c = mem->index = 0;
        } else {
            mem = (void *) ch->act_ptr;
        }
        if (ch->master) {
            mem->pointer = 0;
            return (FALSE);
        }
        if (!AWAKE(ch)) {
            return (FALSE);
        }
        if (ch->specials.fighting) {
            if (!IS_MOB(ch->specials.fighting) &&
                CAN_SEE(ch, ch->specials.fighting)) {
                affect_status(mem, ch, ch->specials.fighting, -5);
            }

            if (mem->status[mem->index] < 0) {
                strcpy(ch->player.long_descr, lattimore_descs[6]);
                mem->pointer = Lattimore_Run;
            } else if (mem->status[mem->index] > 19)
                mem->pointer = Lattimore_Item;
            return (FALSE);
        }

        /*
         * This case is used at startup, and after player interaction
         */
        switch (mem->pointer) {
        case Lattimore_Initialize:
            if (time_info.hours < 5 || time_info.hours > 21) {
                strcpy(ch->player.long_descr, lattimore_descs[3]);
                if (ch->in_room != Barracks) {
                    char_from_room(ch);
                    char_to_room(ch, Barracks);
                }
                mem->pointer = Lattimore_Hiding;
            } else if (time_info.hours < 11) {
                strcpy(ch->player.long_descr, lattimore_descs[4]);
                if (ch->in_room != Barracks) {
                    char_from_room(ch);
                    char_to_room(ch, Barracks);
                }
                mem->pointer = Lattimore_Sleeping;
            } else if (time_info.hours < 16 ||
                       (time_info.hours > 17 && time_info.hours < 22)) {
                strcpy(ch->player.long_descr, lattimore_descs[0]);
                if (ch->in_room != Barracks) {
                    char_from_room(ch);
                    char_to_room(ch, Barracks);
                }
                mem->pointer = Lattimore_Lockers;
            } else if (time_info.hours < 19) {
                strcpy(ch->player.long_descr, lattimore_descs[1]);
                mem->pointer = Lattimore_FoodRun;
            }
            return (FALSE);
            break;

        case Lattimore_Lockers:
            if (time_info.hours == 17) {
                strcpy(ch->player.long_descr, lattimore_descs[1]);
                mem->pointer = Lattimore_FoodRun;
            } else if (time_info.hours > 21) {
                act("$n cocks his head, as if listening.",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("$n looks frightened, and dives under the nearest bunk.",
                    FALSE, ch, 0, 0, TO_ROOM);
                strcpy(ch->player.long_descr, lattimore_descs[3]);
                mem->pointer = Lattimore_Hiding;
            }
            return (FALSE);
            break;

        case Lattimore_FoodRun:
            if (ch->in_room != Kitchen) {
                dir = choose_exit_global(ch->in_room, Kitchen, 100);
                if (dir < 0) {
                    act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);
                    dir = choose_exit_global(ch->in_room, Barracks, 100);
                    if (dir < 0) {
                        char_from_room(ch);
                        char_to_room(ch, Barracks);
                    }
                    return (FALSE);
                } else
                    go_direction(ch, dir);
            } else {
                act("$n gets a spoon off the counter, and ladles himself some"
                    " stew.", FALSE, ch, 0, 0, TO_ROOM);
                strcpy(ch->player.long_descr, lattimore_descs[2]);
                mem->pointer = Lattimore_Eating;
            }
            return (FALSE);
            break;

        case Lattimore_Eating:
            if (time_info.hours > 18) {
                act("$n rubs his stomach and smiles happily.",
                    FALSE, ch, 0, 0, TO_ROOM);
                strcpy(ch->player.long_descr, lattimore_descs[1]);
                mem->pointer = Lattimore_GoHome;
            } else if (!number(0, 2)) {
                act("$n gets some bread from the oven to go with his stew.",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("$n dips the bread in the stew and eats it.",
                    FALSE, ch, 0, 0, TO_ROOM);
            }
            return (FALSE);
            break;

        case Lattimore_GoHome:
            if (ch->in_room != Barracks) {
                dir = choose_exit_global(ch->in_room, Barracks, 100);
                if (dir < 0) {
                    act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);
                    dir = choose_exit_global(ch->in_room, Kitchen, 100);
                    if (dir < 0) {
                        char_from_room(ch);
                        char_to_room(ch, Barracks);
                    }
                    return (FALSE);
                } else
                    go_direction(ch, dir);
            } else {
                act("$n pulls out a crowbar and tries to open another locker.",
                    FALSE, ch, 0, 0, TO_ROOM);
                strcpy(ch->player.long_descr, lattimore_descs[0]);
                mem->pointer = Lattimore_Lockers;
            }
            return (FALSE);
            break;

        case Lattimore_Hiding:
            if (time_info.hours > 5 && time_info.hours < 22) {
                strcpy(ch->player.long_descr, lattimore_descs[4]);
                mem->pointer = Lattimore_Sleeping;
            }
            return (FALSE);
            break;

        case Lattimore_Sleeping:
            if (time_info.hours > 11) {
                act("$n awakens, rises and stretches with a yawn.",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("$n pulls out a crowbar and tries to open another locker.",
                    FALSE, ch, 0, 0, TO_ROOM);
                strcpy(ch->player.long_descr, lattimore_descs[0]);
                mem->pointer = Lattimore_Lockers;
            }
            return (FALSE);
            break;

        case Lattimore_Run:
            if (ch->in_room != Storeroom && ch->in_room != Trap) {
                if (ch->in_room == EarthQ) {
                    return (FALSE);
                }
                dir = choose_exit_global(ch->in_room, Storeroom, 100);
                if (dir < 0) {
                    act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);
                    dir = choose_exit_global(ch->in_room, Kitchen, 100);
                    if (dir < 0) {
                        char_from_room(ch);
                        char_to_room(ch, Barracks);
                    }
                    return (FALSE);
                } else
                    go_direction(ch, dir);
            } else if (ch->in_room == Trap) {
                if (!IS_AFFECTED(ch, AFF_FLYING)) {
                    /*
                     * Get him up off the floor
                     */
                    act("$n grins evilly, and quickly stands on a barrel.",
                        FALSE, ch, 0, 0, TO_ROOM);
                    SET_BIT(ch->specials.affected_by, AFF_FLYING);
                    strcpy(ch->player.long_descr, lattimore_descs[5]);
                    mem->index = 0;
                } else {
                    ++mem->index;
                }
                /*
                 * Wait a while, then go home
                 */
                if (mem->index == 50) {
                    go_direction(ch, 1);
                    mem->pointer = Lattimore_GoHome;
                    REMOVE_BIT(ch->specials.affected_by, AFF_FLYING);
                    strcpy(ch->player.long_descr, lattimore_descs[1]);
                }
            }
            return (FALSE);
            break;

        case Lattimore_Item:
            if (ch->in_room != Conf) {
                dir = choose_exit_global(ch->in_room, Conf, 100);
                if (dir < 0) {
                    act("$n says 'Man, am I lost!'", FALSE, ch, 0, 0, TO_ROOM);
                    dir = choose_exit_global(ch->in_room, Barracks, 100);
                    if (dir < 0) {
                        char_from_room(ch);
                        char_to_room(ch, Barracks);
                    }
                    return (FALSE);
                } else
                    go_direction(ch, dir);
            } else {
                for (t = real_roomp(ch->in_room)->people; t;
                     t = t->next_in_room) {
                    if (!IS_NPC(t) && CAN_SEE(ch, t) &&
                        !(strcmp(mem->names[mem->index], GET_NAME(t)))) {
                        act("$n crawls under the large table.",
                            FALSE, ch, 0, 0, TO_ROOM);
                        obj = objectRead(PostKey);
                        if ((IS_CARRYING_N(t) + 1) < CAN_CARRY_N(t)) {
                            act("$N emerges with $p, and gives it to you.",
                                FALSE, t, obj, ch, TO_CHAR);
                            act("$n emerges with $p, and gives it to $N.",
                                FALSE, ch, obj, t, TO_ROOM);
                            objectGiveToChar(obj, t);
                        } else {
                            act("$n emerges with $p, and drops it for $N.",
                                FALSE, ch, obj, t, TO_ROOM);
                            objectPutInRoom(obj, ch->in_room);
                        }
                    }
                }

                /*
                 * Dude's not here - oh well, go home.
                 */
                /* Duty discharged */
                mem->status[mem->index] = 0;
                mem->pointer = Lattimore_GoHome;
                return (FALSE);
            }
            break;
        default:
            mem->pointer = Lattimore_Initialize;
            return (FALSE);
            break;

        }
    } else if (cmd == 72) {
        arg = get_argument(arg, &obj_name);
        if (!obj_name || !(obj = objectGetOnChar(ch, obj_name, ch))) {
            return (FALSE);
        }

        player_name = skip_spaces(arg);
        if (!player_name || !(latt = get_char_room_vis(ch, player_name))) {
            return (FALSE);
        }

        /*
         * the target is Lattimore
         */
        Lattimore = lattimore;
        if (mob_index[latt->nr].func == Lattimore) {
            if (!latt->act_ptr) {
                ch->act_ptr = (struct memory *) malloc(sizeof(struct memory));
                mem = ch->act_ptr;
                mem->pointer = mem->c = mem->index = 0;
            } else {
                mem = (void *) latt->act_ptr;
            }
            act("You give $p to $N.", TRUE, ch, obj, latt, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, latt, TO_ROOM);

            switch (obj->type_flag) {

            case ITEM_TYPE_FOOD:
                if (obj->value[3]) {
                    act("$n sniffs $p, then discards it with disgust.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    objectTakeFromChar(obj);
                    objectPutInRoom(obj, ch->in_room);
                    if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                        mem->index = affect_status(mem, latt, ch, -5);
                    } else {
                        return (TRUE);
                    }
                } else {
                    act("$n takes $p and hungrily wolfs it down.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    objectExtract(obj);
                    if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                        mem->index = affect_status(mem, latt, ch, 4);
                    } else {
                        return (TRUE);
                    }
                }
                break;
            case ITEM_TYPE_KEY:
                /*
                 * What he really wants
                 */
                if (obj->item_number == CrowBar) {
                    act("$n takes $p and jumps up and down in joy.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    objectTakeFromChar(obj);
                    if (!ch->equipment[HOLD]) {
                        equip_char(ch, obj, HOLD);
                    }
                    if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                        mem->index = affect_status(mem, latt, ch, 20);
                    } else {
                        return (TRUE);
                    }
                }
                break;
            default:
                /*
                 * Any other types of items
                 */
                act("$n looks at $p curiously.", TRUE, latt, obj, 0,
                    TO_ROOM);
                if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                    mem->index = affect_status(mem, latt, ch, 1);
                } else {
                    return (TRUE);
                }
                break;
            }

            /*
             * They gave something to him, and the status was affected,
             * now we set the pointer according to the status value
             */
            if (mem->status[mem->index] < 0) {
                strcpy(latt->player.long_descr, lattimore_descs[6]);
                mem->pointer = Lattimore_Run;
            } else if (mem->status[mem->index] > 19) {
                strcpy(latt->player.long_descr, lattimore_descs[6]);
                mem->pointer = Lattimore_Item;
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

/*
 * Returns the index to the dude who did it
 */

/**
 * @todo Fix for 64bit
 */
int affect_status(struct memory *mem, struct char_data *ch,
                  struct char_data *t, int aff_status)
{
    int             i;

    if (mem->c) {
        for (i = 0; i < mem->c; ++i) {
            if (!(strcmp(GET_NAME(t), mem->names[i]))) {
                mem->status[i] += aff_status;
                return (i);
            }
        }

        mem->names = (char **) realloc(mem->names, (sizeof(long) * mem->c));
        mem->status = (int *) realloc(mem->status, (sizeof(long) * mem->c));
    } else {
        mem->names = (char **) malloc(sizeof(long));
        mem->status = (int *) malloc(sizeof(long));
    }

    mem->names[mem->c] = (char *) malloc(strlen(GET_NAME(t) + 2));
    strcpy(mem->names[mem->c], GET_NAME(t));
    /* AMD64: fix me */
    mem->status[mem->c] = (int)(long) malloc(sizeof(int));
    mem->status[mem->c] = aff_status;
    ++mem->c;
    return (mem->c - 1);
}




#define START_ROOM      21276
#define END_ROOM        21333
#define GhostSoldier    21138
#define GhostLieutenant 21139
#define Identifier      "gds"

int keystone(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    /*
     * Must be a unique identifier for this mob type, or we lose
     */

    struct char_data *ghost,
                   *t,
                   *master;
    int             i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (time_info.hours == 22 &&
        !(ghost = get_char_vis_world(ch, Identifier, 0))) {
        act("$n cries 'Awaken my soldiers! Our time is nigh!'",
            FALSE, ch, 0, 0, TO_ROOM);
        act("You suddenly feel very, very afraid.",
            FALSE, ch, 0, 0, TO_ROOM);

        for (i = START_ROOM; i < END_ROOM; ++i) {
            if (number(0, 2) == 0) {
                ghost = read_mobile(GhostSoldier, VIRTUAL);
                char_to_room(ghost, i);
            } else if (number(0, 7) == 0) {
                ghost = read_mobile(GhostLieutenant, VIRTUAL);
                char_to_room(ghost, i);
            }
        }

        for (t = character_list; t; t = t->next) {
            if (real_roomp(ch->in_room)->zone == real_roomp(t->in_room)->zone) {
                act("You hear a strange cry that fills your soul with fear!",
                    FALSE, t, 0, 0, TO_CHAR);
            }
        }
    }

    if (ch->specials.fighting) {
        if (IS_NPC(ch->specials.fighting) &&
            !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF) &&
            (master = (ch->specials.fighting)->master) && CAN_SEE(ch, master)) {

            stop_fighting(ch);
            hit(ch, master, TYPE_UNDEFINED);
        }

        if (GET_POS(ch) == POSITION_FIGHTING) {
            FighterMove(ch);
        } else {
            StandUp(ch);
        }
        CallForGuard(ch, ch->specials.fighting, 3, OUTPOST);
    }

    return (FALSE);
}



#define RHYODIN_FILE "rhyodin"
#define Necklace 21122

int guardian(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    FILE           *pass;
    struct char_data *g,
                   *master;
    struct obj_data *obj;
    struct room_data *rp;
    int             j = 0;
    struct follow_type *fol;
    char           *player_name,
                   *obj_name,
                    name[15];
    int             (*guard) ();

    struct Names {
        char          **names;
        short           num_names;
    }              *gstruct;

    if ((cmd && !(cmd == 72 || cmd == 3)) || !AWAKE(ch)) {
        return (FALSE);
    }

    if (!cmd && ch->generic == -1) {
        return (FALSE);
    }
    if (!cmd && !ch->generic) {
        /*
         * Open the file, read the names into an array in the act pointer
         */
        if (!(pass = fopen(RHYODIN_FILE, "r"))) {
            Log("Rhyodin access file unreadable or non-existant");
            ch->generic = -1;
            return (FALSE);
        }

        ch->act_ptr = (struct Names *) malloc(sizeof(struct Names));
        gstruct = ch->act_ptr;
        gstruct->names = (char **) malloc(sizeof(char));
        gstruct->num_names = 0;

        while (fscanf(pass, " %s\n", name) == 1) {
            gstruct->names = (char **)
                realloc(gstruct->names, (++gstruct->num_names) * sizeof(char));
            gstruct->names[gstruct->num_names - 1] = (char *)
                malloc(15 * (sizeof(char)));
            strcpy(gstruct->names[gstruct->num_names - 1], name);
        }
    } else {
        gstruct = (void *) ch->act_ptr;
    }

    if (!cmd) {
        if (ch->specials.fighting) {
            if (IS_NPC(ch->specials.fighting) &&
                !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF) &&
                (master = (ch->specials.fighting)->master) &&
                CAN_SEE(ch, master)) {

                stop_fighting(ch);
                hit(ch, master, TYPE_UNDEFINED);
            }

            if (GET_POS(ch) == POSITION_FIGHTING) {
                FighterMove(ch);
            } else {
                StandUp(ch);
            }
        }
        return (FALSE);
    }

    if (cmd == 72) {
        arg = get_argument(arg, &obj_name);
        if (!obj_name || !(obj = objectGetOnChar(ch, obj_name, ch))) {
            return (FALSE);
        }

        player_name = skip_spaces(arg);
        if (!player_name || !(g = get_char_room_vis(ch, player_name))) {
            return (FALSE);
        }
        guard = guardian;

        if (mob_index[g->nr].func == guard) {
            gstruct = (void *) g->act_ptr;

            act("You give $p to $N.", TRUE, ch, obj, g, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, g, TO_ROOM);

            if (obj->item_number == Necklace) {
                if (!check_soundproof(ch)) {
                    act("$n takes $p, and unlocks the gate.",
                        FALSE, g, obj, 0, TO_ROOM);
                    act("$p pulses in his hand, and disappears.",
                        FALSE, g, obj, 0, TO_ROOM);
                    act("$N says 'You have proven youself worthy.'",
                        FALSE, ch, 0, g, TO_CHAR);
                    act("$N says 'You are now an ambassador from the north "
                        "to Rhyodin.'", FALSE, ch, 0, g, TO_CHAR);
                }

                /*
                 * Take it away
                 */
                objectTakeFromChar(obj);
                objectExtract(obj);

                if (!IS_NPC(ch)) {
                    if (!(pass = fopen(RHYODIN_FILE, "a"))) {
                        Log("Couldn't open file for writing permanent Rhyodin "
                            "passlist.");
                        return (FALSE);
                    }

                    /*
                     * Go to the end of the file and write the character's
                     * name
                     */
                    fprintf(pass, " %s\n", GET_NAME(ch));
                    fclose(pass);
                }

                /*
                 * Okay, now take person and all followers in this room to
                 * next room
                 */
                act("$N opens the gate and guides you through.",
                    FALSE, ch, 0, g, TO_CHAR);
                rp = real_roomp(ch->in_room);

                char_from_room(ch);
                char_to_room(ch, rp->dir_option[2]->to_room);
                do_look(ch, NULL, 0);

                /*
                 * First level followers can tag along
                 */
                if (ch->followers) {
                    act("$N says 'If they're with you, they can enter as "
                        "well.'", FALSE, ch, 0, g, TO_CHAR);

                    for (fol = ch->followers; fol; fol = fol->next) {
                        if (fol->follower->specials.fighting) {
                            continue;
                        }
                        if (real_roomp(fol->follower->in_room) &&
                            EXIT(fol->follower, 2)->to_room != NOWHERE &&
                            GET_POS(fol->follower) >= POSITION_STANDING) {
                            char_from_room(fol->follower);
                            char_to_room(fol->follower,
                                         rp->dir_option[2]->to_room);
                            do_look(fol->follower, NULL, 0);
                        }
                    }
                }
                return (TRUE);
            } else {
                return (FALSE);
            }
        } else {
            return (FALSE);
        }
    } else if (cmd == 3 && !IS_NPC(ch)) {
        g = find_mobile_here_with_spec_proc(guardian, ch->in_room);
        gstruct = (void *) g->act_ptr;
        j = 0;

        /*
         * Trying to move south, check against namelist
         */
        while (j < gstruct->num_names) {
            if (!strcmp(gstruct->names[j], GET_NAME(ch))) {
                if (real_roomp(ch->in_room) && 
                    EXIT(ch, 2)->to_room != NOWHERE) {
                    if (ch->specials.fighting) {
                        return (FALSE);
                    }
                    act("$N recognizes you, and escorts you through the gate.",
                        FALSE, ch, 0, g, TO_CHAR);
                    act("$N recognizes $n, and escorts them through the gate.",
                        FALSE, ch, 0, g, TO_ROOM);
                    rp = real_roomp(ch->in_room);
                    char_from_room(ch);
                    char_to_room(ch, rp->dir_option[2]->to_room);
                    do_look(ch, NULL, 0);

                    /*
                     * Follower stuff again
                     */
                    if (ch->followers) {
                        act("$N says 'If they're with you, they can enter as "
                            "well.'", FALSE, ch, 0, g, TO_CHAR);

                        for (fol = ch->followers; fol; fol = fol->next) {
                            if (fol->follower->specials.fighting) {
                                continue;
                            }
                            if (real_roomp(fol->follower->in_room) &&
                                EXIT(fol->follower, 2)->to_room != NOWHERE &&
                                GET_POS(fol->follower) >= POSITION_STANDING) {

                                char_from_room(fol->follower);
                                char_to_room(fol->follower,
                                             rp->dir_option[2]->to_room);
                                do_look(fol->follower, NULL, 0);
                            }
                        }
                    }
                    return (TRUE);
                } else {
                    return (FALSE);
                }
            }
            j++;
        }
        return (FALSE);
    }
    return (FALSE);
}



/*---------------end of summoner---------------*/


char *invert(char *arg1)
{
    int    i;
    int    len;
    char  *arg2,
          *head;

    arg2 = strdup(arg1);
    if( !arg2 ) {
        return( arg1 );
    }
    head = arg2;

    len = strlen(arg1);
    arg1 += len - 1;

    for( i = 0; i < len; i++ ) {
        *(arg2++) = *(arg1--);
    }
    *arg2 = '\0';

    return( head );
}

int jive_box(struct char_data *ch, int cmd, char *arg,
             struct obj_data *obj, int type)
{
    char           *buf,
                   *buf2,
                    buf3[MAX_STRING_LENGTH],
                   *tmp;

    if (type != PULSE_COMMAND || !arg) {
        return (FALSE);
    }

    switch (cmd) {
    case 17:
    case 169:
        buf = invert(arg);
        do_say(ch, buf, cmd);
        free( buf );
        return (TRUE);
        break;
    case 19:
        arg = get_argument(arg, &tmp);
        buf = skip_spaces(arg);

        if( !tmp || !buf ) {
            return( FALSE );
        }

        buf2 = invert(buf);
        sprintf(buf3, "tell %s %s", tmp, buf2);
        command_interpreter(ch, buf3);
        free( buf2 );
        return (TRUE);
        break;
    case 18:
        buf = invert(arg);
        do_shout(ch, buf, cmd);
        free( buf );
        return (TRUE);
        break;
    default:
        return (FALSE);
        break;
    }
}


int necromancer(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell;
    int             to_room = 0;
    extern int      top_of_world;
    struct room_data *room = NULL;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if (!MobCastCheck(ch, 0)) {
        return (TRUE);
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    /*
     * random number from 0 to level 
     */
    lspell = number(0, GetMaxLevel(ch));
    lspell += GetMaxLevel(ch) / 5;
    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    /*
     * are we hurting so bad? 
     */
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && ch->specials.fighting && 
        !number(0, 2)) {
        /* 
         * cool, we're outtie!
         *
         * ouch, we are.. let's see if we can get outta here 
         */
        act("$n utters the words 'I see a shadow!'", 1, ch, 0, 0, TO_ROOM);
        spell_shadow_step(GetMaxLevel(ch), ch, 0, 0);
    }

    if (!ch->specials.fighting) {
        if ((GET_HIT(ch) < GET_MAX_HIT(ch) - 10)
            && (GetMaxLevel(ch) >= 10)) {
            /*
             * timer to heal up! 
             */
            act("$n utters the words 'mend my bones'", 1, ch, 0, 0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

#ifdef PREP_SPELLS
        if (!ch->desc) {
            /* 
             * is it a mob?
             *
             * low level spellup 
             */
            if (!affected_by_spell(ch, SPELL_SUIT_OF_BONE)) {
                act("$n utters the words 'bones of the dead'", FALSE, ch,
                    0, 0, TO_ROOM);
                cast_suit_of_bone(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_PROTECT_FROM_GOOD) &&
                IS_EVIL(ch)) {
                act("$n traces a protective rune in the air.", FALSE, ch,
                    0, 0, TO_ROOM);
                cast_protection_from_good(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_SPECTRAL_SHIELD)) {
                act("$n utters the words 'steel of the legions'", FALSE,
                    ch, 0, 0, TO_ROOM);
                cast_spectral_shield(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_GLOBE_DARKNESS) && 
                !IS_AFFECTED(ch, AFF_DARKNESS)) {
                act("$n utters the words 'home sweet home'", FALSE, ch, 0,
                    0, TO_ROOM);
                cast_globe_darkness(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * mid level spellup 
             */
            if (GetMaxLevel(ch) > 23) {
                if (!IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
                    act("$n utters the words 'something in my eye'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_eye_of_the_dead(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_ENDURE_COLD)) {
                    act("$n utters the words 'no more arthritis'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_endure_cold(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
                if (!affected_by_spell(ch, SPELL_GLOBE_MINOR_INV)) {
                    act("$n utters the words 'protect'", FALSE, ch, 0, 0,
                        TO_ROOM);
                    cast_globe_minor_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }

            if (GetMaxLevel(ch) > 31) {
                if (!affected_by_spell(ch, SPELL_GLOBE_MAJOR_INV)) {
                    act("$n utters the words 'protect, I said!'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_globe_major_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
                    act("$n utters the words 'tomb stone'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_stone_skin(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }

            if (GetMaxLevel(ch) > 40 &&
                !affected_by_spell(ch, SPELL_VAMPIRIC_EMBRACE)) {
                act("$n utters the words 'kiss of death'", FALSE, ch,
                    0, 0, TO_ROOM);
                cast_vampiric_embrace(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (GetMaxLevel(ch) > 44 &&
                !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                !affected_by_spell(ch, SPELL_CHILLSHIELD) && !number(0, 2)) {
                act("$n utters the words 'shroud of the dark lord'",
                    FALSE, ch, 0, 0, TO_ROOM);
                cast_chillshield(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * low level removes 
             */
            if (GetMaxLevel(ch) > 20 &&
                (affected_by_spell(ch, SPELL_POISON) || 
                 affected_by_spell(ch, SPELL_DISEASE) || 
                 affected_by_spell(ch, SPELL_DECAY))) {
                act("$n utters the words 'nullify'", FALSE, ch, 0, 0, TO_ROOM);
                cast_nullify(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (GET_MOVE(ch) < GET_MAX_MOVE(ch) / 2) {
                act("$n utters the words 'feet of the wight'", FALSE, ch,
                    0, 0, TO_ROOM);
                cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }
        }
#endif
    }

    /*
     * Find a dude to to evil things upon ! 
     */
    if ((vict = FindAHatee(ch)) == NULL &&
        IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        vict = FindVictim(ch);
    }

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    /*
     * get up defense 
     */
    if (GetMaxLevel(ch) > 44 && !IS_AFFECTED(ch, AFF_CHILLSHIELD)) {
        act("$n utters the words 'shroud of the dark lord'", FALSE,
            ch, 0, 0, TO_ROOM);
        cast_chillshield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (TRUE);
    }

    /*
     * then pester them 
     */
    switch (lspell) {
    case 1:
    case 2:
    case 3:
        act("$n utters the words 'quench my thirst'", 1, ch, 0, 0, TO_ROOM);
        cast_life_tap(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 4:
        act("$n utters the words 'wimp'", 1, ch, 0, 0, TO_ROOM);
        cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 5:
    case 6:
        act("$n utters the words 'puke'", 1, ch, 0, 0, TO_ROOM);
        cast_disease(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 7:
        act("$n utters the words 'drop'", 1, ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 8:
        act("$n utters the words 'serpent's bite'", 1, ch, 0, 0, TO_ROOM);
        cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 9:
    case 10:
        act("$n utters the words 'quench my thirst'", 1, ch, 0, 0, TO_ROOM);
        cast_life_tap(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 11:
        act("$n utters the words 'I'll take that, gov'nor'", 1, ch, 0,
            0, TO_ROOM);
        cast_siphon_strength(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
        break;
    case 12:
    case 13:
        act("$n utters the words 'tasty'", 1, ch, 0, 0, TO_ROOM);
        cast_life_draw(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        act("$n utters the words 'what a shame'", 1, ch, 0, 0, TO_ROOM);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
        if (!affected_by_spell(vict, SPELL_DECAY) && 
            !IS_SET(vict->M_immune, IMM_POISON)) {
            act("$n utters the words 'rot'", 1, ch, 0, 0, TO_ROOM);
            cast_decay(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'tasty'", 1, ch, 0, 0, TO_ROOM);
            cast_life_draw(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'bliss'", 1, ch, 0, 0, TO_ROOM);
            cast_soul_steal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        }
        break;
    case 25:
    case 26:
    case 27:
        if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'very tasty'", 1, ch, 0, 0, TO_ROOM);
            cast_life_leech(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'bliss'", 1, ch, 0, 0, TO_ROOM);
            cast_soul_steal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        }
        break;
    case 28:
    case 29:
    case 30:
    case 31:
        if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'touched by the lord'", 1, ch, 0,
                0, TO_ROOM);
            cast_finger_of_death(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'bliss'", 1, ch, 0, 0, TO_ROOM);
            cast_soul_steal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        }
        break;
    case 32:
    case 33:
    case 34:
        act("$n utters the words 'oh lordie'", 1, ch, 0, 0, TO_ROOM);
        cast_mist_of_death(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 35:
        if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'what a shame'", 1, ch, 0, 0, TO_ROOM);
            cast_energy_drain(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'bliss'", 1, ch, 0, 0, TO_ROOM);
            cast_soul_steal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        }
        break;
    case 36:
        act("$n utters the words 'bliss'", 1, ch, 0, 0, TO_ROOM);
        cast_soul_steal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 37:
    case 38:
    case 39:
    case 40:
        if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'very tasty'", 1, ch, 0, 0, TO_ROOM);
            cast_life_leech(GetMaxLevel(ch), ch, "",
                            SPELL_TYPE_SPELL, vict, 0);
        } else if (IS_AFFECTED(vict, AFF_SANCTUARY) && 
                   GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'no cheating'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, vict, 0);
        } else if (IS_AFFECTED(vict, AFF_FIRESHIELD) && 
                   GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'I hate spellups'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, vict, 0);
        } else if (!IS_SET(vict->M_immune, IMM_POISON)) {
            act("$n utters the words 'rot'", 1, ch, 0, 0, TO_ROOM);
            cast_decay(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'oh dearie me'", 1, ch, 0, 0, TO_ROOM);
            cast_shadow_step(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
        }
        break;
    default:
        if (!IS_SET(vict->M_immune, IMM_POISON)) {
            if (!(ch->equipment[WEAR_EYES])) {
                act("$n utters the words 'oh golly'", 1, ch, 0, 0, TO_ROOM);
                cast_scourge_warlock(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
            } else if ((ch->equipment[WEAR_EYES])->item_number == TONGUE_ITEM) {
                act("$n utters the words 'oh golly'", 1, ch, 0, 0, TO_ROOM);
                cast_scourge_warlock(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
            } else {
                objectGiveToChar(unequip_char(ch, WEAR_EYES), ch);
                act("$n utters the words 'oh golly'", 1, ch, 0, 0, TO_ROOM);
                cast_scourge_warlock(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
            }
        } else if (!IS_SET(vict->M_immune, IMM_DRAIN)) {
            act("$n utters the words 'touched by the lord'", 1, ch, 0,
                0, TO_ROOM);
            cast_finger_of_death(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
        } else {           
            /* 
             * can't touch them, better get outta here 
             */
            while (!room) {
                to_room = number(0, top_of_world);
                room = real_roomp(to_room);
                if (room && (IS_SET(room->room_flags, INDOORS) || 
                             IS_SET(room->room_flags, NO_MAGIC) || 
                             !IsOnPmp(to_room))) {
                    room = 0;
                }
            }
            act("$n utters the words 'can't touch this'", 1, ch, 0, 0, TO_ROOM);
            act("$c0008$n closes $s eyes and steps into the shadows.",
                FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, to_room);
            /*
             * Tataa! come chase me, you bastard! I'm gonna heal up a
             * bit in the mean time 
             */
        }
        break;
    }
    return (TRUE);
}

/*
 * for DRUID flagged mobs, -Lennya 20030604 
 */
int druid(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    struct char_data *vict = 0;
    struct follow_type *fol;
    byte            lspell;
    int             to_room = 0,
                    num = 2;
    extern int      top_of_world;
    struct room_data *room = NULL;

#if 0
    Log("entered druid proc");
#endif
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if (!MobCastCheck(ch, 0)) {
        return (TRUE);
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    /*
     * random number from 0 to level 
     */
    lspell = number(0, GetMaxLevel(ch));
    lspell += GetMaxLevel(ch) / 5;
    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    /*
     * are we hurting so bad? 
     */
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 7 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS) &&
        real_roomp(ch->in_room)->sector_type != SECT_AIR && !number(0, 2)) {
        /* 
         * cool, we're outtie! 
         */
        while (!room) {
            to_room = number(0, top_of_world);
            room = real_roomp(to_room);
            if (room && (IS_SET(room->room_flags, INDOORS) || 
                         IS_SET(room->room_flags, NO_MAGIC) || 
                         !IsOnPmp(to_room))) {
                room = 0;
            }
        }

        act("$n utters the words 'plant aid'", 1, ch, 0, 0, TO_ROOM);
        act("$n touches a nearby plant, and disappears!", 1, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, to_room);
        stop_fighting(ch);

        if (ch->specials.fighting && 
            ch->specials.fighting->specials.fighting == ch) {
            stop_fighting(ch->specials.fighting);
        }
        /*
         * Tataa! come chase me, you bastard! I'm gonna heal up a
         * bit in the mean time 
         */
    }

    if (!ch->specials.fighting) {
#if 0        
        Log("not fighting");
#endif        
        if (GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
            /*
             * timer to heal up! 
             */
            if (GetMaxLevel(ch) >= 25) {
                num = 3;
            } else {
                num = 2;
            }
            switch (number(0, num)) {
            case 1:
                act("$n utters the words 'Woah! I feel GOOD! Heh.'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
                break;
            case 2:
                act("$n utters the words 'I feel much better now!'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_serious(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                break;
            case 3:
                act("$n utters the words 'The Myrrhal shine upon me!'", 1,
                    ch, 0, 0, TO_ROOM);
                cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                break;
            default:
                act("$n utters the words 'I feel good!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
                break;
            }
            return (TRUE);
        }

#ifdef PREP_SPELLS
        if (!ch->desc) {
            /* 
             * is it a mob? 
             *
             *
             * low level spellup 
             */
            if (!ch->equipment[WIELD] && GetMaxLevel(ch) < 11) {
                act("$n utters the words 'Gimmie a light!'", FALSE, ch, 0,
                    0, TO_ROOM);
                cast_flame_blade(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_BARKSKIN)) {
                act("$n utters the words 'oakey dokey'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_barkskin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL) && 
                !IS_EVIL(ch)) {
                act("$n makes a protective gesture.", FALSE, ch, 0, 0, TO_ROOM);
                cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_PROTECT_FROM_GOOD) && 
                !IS_GOOD(ch)) {
                act("$n utters the words 'hide from white'", FALSE, ch, 0,
                    0, TO_ROOM);
                cast_protection_from_good(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * mid level spellup 
             */
            if (GetMaxLevel(ch) > 23) {
                if (!IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
                    act("$n utters the words 'view of the condor'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_true_seeing(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_FIRE)) {
                    act("$n utters the words 'feathers of the phoenix'",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_prot_fire(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_COLD)) {
                    act("$n utters the words 'pelt of the winter wolf'",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_prot_cold(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ENERGY)) {
                    act("$n utters the words 'hide of the Toad'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_prot_energy(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ELEC)) {
                    act("$n utters the words 'scales of the moray'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_prot_elec(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_WATER_BREATH)) {
                    act("$n utters the words 'gills of the amphibian'",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_water_breath(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }

            if (GetMaxLevel(ch) > 24) {
                /*
                 * let's give dr00d some pets 
                 */
                if (!affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                    OUTSIDE(ch) && !ch->followers && 
                    !IS_SET(real_roomp((ch)->in_room)->room_flags, PEACEFUL) &&
                    !IS_SET(real_roomp((ch)->in_room)->room_flags, TUNNEL)) {
                    act("$n whistles loudly.", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_3(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                        ch->followers) {
                        act("", FALSE, ch, 0, 0, TO_ROOM);
                        command_interpreter(ch, "order followers guard on");
                        act("", FALSE, ch, 0, 0, TO_ROOM);
                        command_interpreter(ch, "group all");
                        act("\n\r$n utters the words 'size of the behemoth "
                            "lord'", FALSE, ch, 0, 0, TO_ROOM);

                        for (fol = ch->followers; fol; fol = fol->next) {
                            if (!affected_by_spell(fol->follower,
                                                   SPELL_ANIMAL_GROWTH)) {
                                cast_animal_growth(GetMaxLevel(ch), ch, "",
                                                   SPELL_TYPE_SPELL,
                                                   fol->follower, 0);
                                WAIT_STATE(ch, PULSE_VIOLENCE);
                            }
                        }
                    }
                    return (TRUE);
                }
            }

            /*
             * high level spellup, whoopie, this will be a killer 
             */
            if (GetMaxLevel(ch) > 47 && 
                !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                !affected_by_spell(ch, SPELL_CHILLSHIELD) && !number(0, 2)) {
                act("$n utters the words 'shroud of the phoenix'",
                    FALSE, ch, 0, 0, TO_ROOM);
                cast_fireshield(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * let's mask these spells and look real cool 
             */
            if ((IS_AFFECTED(ch, AFF_FIRESHIELD) || 
                 IS_AFFECTED(ch, AFF_SANCTUARY)) && 
                !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) && 
                !IS_AFFECTED(ch, AFF_DARKNESS)) {
                act("$n utters the words 'shadow of the raven'", FALSE, ch,
                    0, 0, TO_ROOM);
                cast_globe_darkness(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * low level removes 
             */
            if (affected_by_spell(ch, SPELL_POISON)) {
                act("$n utters the words 'remove poison'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_remove_poison(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (affected_by_spell(ch, SPELL_BLINDNESS)) {
                act("$n utters the words 'see the light'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
                return (TRUE);
            }

            /*
             * hi level removes 
             */
            if (GetMaxLevel(ch) > 24) {
                if (affected_by_spell(ch, SPELL_CURSE)) {
                    act("$n utters the words 'neutralize'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_remove_curse(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (affected_by_spell(ch, SPELL_WEB)) {
                    act("$n utters the words 'glib my feet'", FALSE, ch, 0,
                        0, TO_ROOM);
                    act("$n doesn't look so webby anymore.'.", FALSE, ch,
                        0, 0, TO_ROOM);
                    affect_from_char(ch, SPELL_WEB);
                    return (TRUE);
                }
            }

            if (GET_MOVE(ch) < GET_MAX_MOVE(ch) / 2) {
                act("$n utters the words 'lemon aid'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }
        }
#endif
    }
    free(vict);

    /*
     * Find a dude to to evil things upon ! 
     */
    if (IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        vict = FindVictim(ch);
    }
    if (!vict && ch->specials.fighting) {
        vict = ch->specials.fighting;
    }

    if (!vict) {
        return (FALSE);
    }

    if (vict == ch) {
        Log("victim same as char");
        return (FALSE);
    }

    if (!vict->in_room) {
        Log("vict not in room");
        return (FALSE);
    }

    if (!real_roomp(vict->in_room)) {
        Log("vict in bad room");
        return (FALSE);
    }

    if (vict->in_room != ch->in_room) {
        Log("ch and vict in different rooms");
        return (FALSE);
    }

    if (GET_POS(vict) <= POSITION_DEAD) {
        Log("trying to cast a spell on dead victim");
        return (FALSE);
    }

    if (number(1, 9) > 3) {
#if 0        
        Log("pester em");
#endif        
        /*
         * pester them 
         */
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 && 
            !number(0, 4)) {
            act("$n whistles.", 1, ch, 0, 0, TO_ROOM);
            act("$n utters the words 'Here Lightning!'", 1, ch, 0, 0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
        case 3:
            act("$n utters the words 'muhahaha'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            act("$n utters the words 'frizzle sizzle'", 1, ch, 0, 0, TO_ROOM);
            cast_heat_stuff(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                            vict, 0);
            break;
        case 7:
            act("$n utters the words 'breach'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 8:
            act("$n utters the words 'wrath of the serpent'", 1, ch, 0, 0,
                TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            act("$n utters the words 'Pain? It helps clear the mind.'", 1,
                ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 11:
            act("$n utters the words 'burn baby burn'", 1, ch, 0, 0, TO_ROOM);
            cast_firestorm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 12:
        case 13:
        case 14:
            if (vict->equipment[WIELD]) {
                act("$n utters the words 'frogemoth'", 1, ch, 0, 0, TO_ROOM);
                cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 vict, 0);
            } else {
                act("$n utters the words 'frizzle sizzle'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_heat_stuff(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            }
            break;
        case 15:
            act("$n utters the words 'kazappapapappaa!'", 1, ch, 0, 0, TO_ROOM);
            cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 16:
            if (!IS_SET(vict->M_immune, IMM_FIRE)) {
                act("$n utters the words 'burn baby burn'", 1, ch, 0,
                    0, TO_ROOM);
                cast_firestorm(GetMaxLevel(ch), ch, "",
                               SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_SANCTUARY) && 
                       GetMaxLevel(ch) >= GetMaxLevel(vict)) {
                act("$n utters the words 'instant-away magic remover!'", 1, ch,
                    0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_FIRESHIELD) && 
                       GetMaxLevel(ch) >= GetMaxLevel(vict)) {
                act("$n utters the words 'instant-away magic remover!'", 1, ch,
                    0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'kazappapapappaa!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_chain_lightn(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 17:
            act("$n utters the words 'here boy'", FALSE, ch, 0, 0, TO_ROOM);
            cast_fire_servant(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
            command_interpreter(ch, "order followers guard on");
            break;
        case 18:
        case 19:
        default:
            if (GetMaxLevel(ch) >= 20 && !IS_AFFECTED(vict, AFF_SILENCE) && 
                !number(0, 2)) {
                act("$n utters the words 'hush honey'", 1, ch, 0, 0, TO_ROOM);
                cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            } else {
                act("$n utters the words 'kazappapapappaa!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_chain_lightn(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        }
        return (TRUE);
    } else {
#if 0        
        Log("better heal up");
#endif        
        /*
         * do heal 
         */
        if (IS_AFFECTED(ch, AFF_BLIND) && lspell >= 4 && !number(0, 3)) {
            act("$n utters the words 'Praise Pentak, I can see!'", 1, ch,
                0, 0, TO_ROOM);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_CURSE) && lspell >= 6 && !number(0, 6)) {
            act("$n utters the words 'I'm rubber, you're glue'", 1, ch, 0,
                0, TO_ROOM);
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_POISON) && lspell >= 5 && !number(0, 6)) {
            act("$n utters the words 'Praise Xenon, I don't feel sick no "
                "more!'", 1, ch, 0, 0, TO_ROOM);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'I feel good!'", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            act("$n utters the words 'I feel much better now!'", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
            act("$n utters the words 'Woah, I feel good! Heh.'", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            act("$n utters the words 'Praise the Lord of Balance, I'm good to "
                "go another round!'", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        }
        return (TRUE);
    }
}




int Samah(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    char           *p,
                    buf[256];
    /* 
     * Samah's own referent pointer 
     */
    struct char_data *Sammy;
    struct char_data *t,
                   *t2,
                   *t3;
    int             purge_nr;
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return (FALSE);
    }
    if (cmd) {
        if (GET_RACE(ch) == RACE_SARTAN || GET_RACE(ch) == RACE_PATRYN ||
            GetMaxLevel(ch) == MAX_IMMORT) {
            return (FALSE);
        }

        Sammy = (struct char_data *) FindMobInRoomWithFunction(ch->in_room,
                                                               Samah);

        arg = skip_spaces(arg);
        strcpy(buf, (arg ? arg : ""));

        if (cmd == 207) {
            /* 
             * recite 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            p = (char *) strtok(buf, " ");
            if (strncmp("recall", p, strlen(p)) == 0) {
                act("$n says 'And just where do you think you're going, "
                    "Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
                return (TRUE);
            }
        } else if (cmd == 84) {
            /* 
             * cast 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            /*
             * we use strlen(p)-1 because if we use the full length, there 
             * is the obligatory ' at the end.  We must ignore this ', and 
             * get on with our lives 
             */
            p = (char *) strtok(buf, " ");
            if (!strncmp("'word of recall'", p, strlen(p) - 1)) {
                act("$n says 'And just where do you think you're going, "
                    "Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (!strncmp("'astral walk'", p, strlen(p) - 1)) {
                act("$n says 'Do you think you can astral walk in and out of "
                    "here like the ", FALSE, Sammy, 0, 0, TO_ROOM);
                act("wind,...Mensch?'", FALSE, Sammy, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (!strncmp("'teleport'", p, strlen(p) - 1)) {
                act("$n says 'And just where do you think you're going, "
                    "Mensch?", FALSE, Sammy, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (!strncmp("'polymorph'", p, strlen(p) - 1)) {
                act("$n says 'I like you the way you are...Mensch.", FALSE,
                    Sammy, 0, 0, TO_ROOM);
                return (TRUE);
            }
        } else if (cmd == 17 || cmd == 169) {
            /* 
             * say 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Mensch should be seen, and not heard'", FALSE,
                Sammy, 0, 0, TO_ROOM);
            return (TRUE);
        } else if (cmd == 40 || cmd == 214 || cmd == 177) {
            /* 
             * emote 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Cease your childish pantonimes, Mensch.'", FALSE,
                Sammy, 0, 0, TO_ROOM);
            return (TRUE);
        } else if (cmd == 19 || cmd == 18 || cmd == 83) {
            /* 
             * say, shout whisp 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Speak only when spoken to, Mensch.'", FALSE,
                Sammy, 0, 0, TO_ROOM);
            return (TRUE);
        } else if (cmd == 86) {
            /* 
             * ask 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Your ignorance is too immense to be rectified at "
                "this time. Mensch.'", FALSE, Sammy, 0, 0, TO_ROOM);
            return (TRUE);
        } else if (cmd == 87 || cmd == 46) {
            /* 
             * order, force 
             */
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'I'll be the only one giving orders here, Mensch'",
                FALSE, Sammy, 0, 0, TO_ROOM);
            return (TRUE);
        } else if (cmd == 151) {
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Cease this cowardly behavior, Mensch'", FALSE,
                Sammy, 0, ch, TO_ROOM);
            return (TRUE);
        } else if (cmd == 63) {
            act("$n glares at you", FALSE, Sammy, 0, ch, TO_VICT);
            act("$n glares at $N", FALSE, Sammy, 0, ch, TO_NOTVICT);
            act("$n says 'Pay attention when I am speaking, Mensch'",
                FALSE, Sammy, 0, ch, TO_ROOM);
            return (TRUE);
        }
    } else if (!ch->specials.fighting) {
        /*
         * check for followers in the room 
         */
        for (t = rp->people; t; t = t->next_in_room) {
            if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch && 
                t->master != ch) {
                break;
            }
        }

        if (t) {
            act("$n says 'What is $N doing here?'", FALSE, ch, 0, t, TO_ROOM);
            act("$n makes a magical gesture", FALSE, ch, 0, 0, TO_ROOM);
            purge_nr = t->nr;

            for (t2 = rp->people; t2; t2 = t3) {
                t3 = t2->next_in_room;
                if (t2->nr == purge_nr && !IS_PC(t2)) {
                    act("$N, looking very surprised, quickly fades out of "
                        "existence.", FALSE, ch, 0, t2, TO_ROOM);
                    extract_char(t2);
                }
            }
        } else {
            /*
             * check for polymorphs in the room 
             */
            for (t = rp->people; t; t = t->next_in_room) {
                if (IS_NPC(t) && IS_PC(t)) {
                    /* 
                     * ah.. polymorphed :-) 
                     */
                    act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
                    act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
                    act("$n says 'Seek not to disguise your true form from "
                        "me...Mensch.", FALSE, ch, 0, t, TO_ROOM);
                    act("$n traces a small rune in the air", FALSE, ch,
                        0, 0, TO_ROOM);
                    act("$n has forced you to return to your original form!",
                        FALSE, ch, 0, t, TO_VICT);
                    command_interpreter(t, "return");
                    return (TRUE);
                }
            }
        }
    }
    return (FALSE);
}

int MakeQuest(struct char_data *ch, struct char_data *gm, int Class,
              char *arg, int cmd)
{
#ifdef EASY_LEVELS
    if (GET_LEVEL(ch, Class) > 0 && cmd == GAIN) {
        /* 
         * for now.. so as not to give it away 
         */
        GainLevel(ch, Class);
        return (TRUE);
    }
#endif
    return (FALSE);
}


#if 0
/*
 * shanty town kids 
 */
int shanty_town_kids(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{

    if (!AWAKE(ch))
        return (FALSE);

    /*
     * harrass low levellers. 
     */
    if (cmd >= 1 && cmd <= 6) {
        if (GetMaxLevel(ch) <= 5 && number(0, 2) == 0) {
            act("A street kid sticks out a foot and trips you as you try to "
                "leave", FALSE, ch, 0, 0, TO_CHAR);
            act("A street kid sticks out a foot and trips $n",
                FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POSITION_SITTING;
            act("The street kid laughs at you", FALSE, ch, 0, 0, TO_CHAR);
            act("The street kid laughs at $n", FALSE, ch, 0, 0, TO_ROOM);
        }
    }

    /*
     * steal from mid-levellers 
     * backstab high levellers 
     */

    if (cmd) {

    }

    if (ch->specials.fighting) {
        act("$N runs between $n's legs", FALSE, ch->specials.fighting, 0,
            ch, TO_ROOM);
        act("$N runs between your legs", FALSE, ch->specials.fighting, 0,
            ch, TO_CHAR);
        vict = ch->specials.fighting;
        stop_fighting(ch);
        stop_fighting(vict);
    }

}

#endif




int PrisonGuard(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
#if 0
    char           *p,
                    buf[256];
    struct char_data *PGuard;   /* guards own referent pointer */
    struct char_data *t,
                   *t2,
                   *t3;
    int             purge_nr,
                    i;
    struct room_data *rp;
    static int      timehere;

    rp = real_roomp(ch->in_room);
    if (!rp)
        return (FALSE);

    if (ch->in_room != PRISON_ROOM)
        return (FALSE);

    PGuard = (struct char_data *) FindMobInRoomWithFunction(ch->in_room,
                                                            PrisonGuard);

    for (t = character_list, i = 0; t; t = t2) {
        t2 = t->next;
        if (PGuard->in_room == t->in_room && PGuard != t && !IS_IMMORTAL(t)) {
            i++;
            if (GET_POS(t) < POSITION_STANDING) {
                do_stand(t, NULL, 0);
                sprintf(buf, "say This is not a vacation %s, get up!", 
                        GET_NAME(t));
                command_interpreter(PGuard, buf);
            }

            if (IS_MURDER(t)) { /* do murder kick out */
                if (timehere >= 80) {
                    act("$n glares at you and says 'You are free to roam the "
                        "world again, murderer.'", FALSE, PGuard, 0, t, 
                        TO_VICT);
                    act("$n glares at $N and says 'You are free to roam the "
                        "world again, murderer.'", FALSE, PGuard, 0, t, 
                        TO_NOTVICT);
                    REMOVE_BIT(t->player.user_flags, MURDER_1);
                    REMOVE_BIT(t->player.user_flags, STOLE_1);
                    Log("Removing MURDER and STOLE bit from %s.", GET_NAME(t));
                    char_from_room(t);
                    char_to_room(t, PRISON_LET_OUT_ROOM);
                    do_look(t, NULL, 0);
                    sprintf(buf, "shout The prisoner convicted of murder, %s, "
                                 "is now free!", GET_NAME(t));
                    command_interpreter(PGuard, buf);
                    timehere = 0;
                    return (TRUE);
                } else if (number(0, 1))
                    timehere++;
            }

            if (IS_STEALER(t)) {
                /* do stole kick out */
                if (timehere >= 30) {
                    act("$n glares at you and says 'You are free to roam the "
                        "world again, thief.'", FALSE, PGuard, 0, t, TO_VICT);
                    act("$n glares at $N and says 'You are free to roam the "
                        "world again, thief.'", FALSE, PGuard, 0, t, 
                        TO_NOTVICT);
                    REMOVE_BIT(t->player.user_flags, STOLE_1);
                    REMOVE_BIT(t->player.user_flags, MURDER_1);
                    Log("Removing STOLE and MURDER bit from %s.", GET_NAME(t));

                    char_from_room(t);
                    char_to_room(t, PRISON_LET_OUT_ROOM);
                    do_look(t, NULL, 0);

                    sprintf(buf, "shout The prisoner convicted of robbery, %s, "
                                 "is now free!", GET_NAME(t));
                    command_interpreter(PGuard, buf);
                    timehere = 0;
                    return (TRUE);
                } else if (number(0, 1)) {
                    timehere++;
                }
            }
        }
        
        if (i == 0)
            timehere = 0;

        if (cmd) {
            if (IS_IMMORTAL(ch))
                return (FALSE);

            PGuard = (struct char_data *) 
                      FindMobInRoomWithFunction(ch->in_room, PrisonGuard);

            arg = skip_spaces(arg);
            strcpy(buf, (arg ? arg : ""));

            if (cmd == 302) {   /* gos */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Shut up!", FALSE, PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 304) {    /* auc */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Shut up!", FALSE, PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 207) {    /* recite */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);

                p = (char *) strtok(buf, " ");
                if (strncmp("recall", p, strlen(p)) == 0) {
                    act("$n says 'And just where do you think you're going?",
                        FALSE, PGuard, 0, 0, TO_ROOM);
                    return (TRUE);
                }
            } else if (cmd == 84) {     /* cast */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                /*
                 * we use strlen(p)-1 because if we use the full length,
                 * there is the obligatory ' at the end.  We must ignore
                 * this ', and get on with our lives 
                 */
                p = (char *) strtok(buf, " ");
                if (strncmp("'word of recall'", p, strlen(p) - 1) == 0) {
                    act("$n says 'And just where do you think you're going?",
                        FALSE, PGuard, 0, 0, TO_ROOM);
                    return (TRUE);
                } else if (strncmp("'astral walk'", p, strlen(p) - 1) == 0) {
                    act("$n says 'Do you think you can astral walk in and out "
                        "of here like the ", FALSE, PGuard, 0, 0, TO_ROOM);
                    act("wind?'", FALSE, PGuard, 0, 0, TO_ROOM);
                    return (TRUE);
                } else if (strncmp("'teleport'", p, strlen(p) - 1) == 0) {
                    act("$n says 'And just where do you think you're going?",
                        FALSE, PGuard, 0, 0, TO_ROOM);
                    return (TRUE);
                } else if (strncmp("'polymorph'", p, strlen(p) - 1) == 0) {
                    act("$n says 'I like you the way you are.", FALSE,
                        PGuard, 0, 0, TO_ROOM);
                    return (TRUE);
                }
            } else if (cmd == 17 || cmd == 169) { 
                /* say */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Prisoners should be seen, and not heard'",
                    FALSE, PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 40 || cmd == 214 || cmd == 177) {
                /* emote */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Cease your childish pantonimes.'", FALSE,
                    PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 19 || cmd == 18 || cmd == 83) {
                /* say, shout whisp */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Speak only when spoken to.'", FALSE, PGuard,
                    0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 86) {
                /* ask */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Your ignorance is too immense to be rectified at"
                    " this time.'", FALSE, PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 87 || cmd == 46) {
                /* order, force */
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'I'll be the only one giving orders here.'",
                    FALSE, PGuard, 0, 0, TO_ROOM);
                return (TRUE);
            } else if (cmd == 151) {
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Cease this cowardly behavior.'", FALSE,
                    PGuard, 0, ch, TO_ROOM);
                return (TRUE);
            } else if (cmd == 63) {
                act("$n glares at you", FALSE, PGuard, 0, ch, TO_VICT);
                act("$n glares at $N", FALSE, PGuard, 0, ch, TO_NOTVICT);
                act("$n says 'Pay attention when I am speaking.'", FALSE,
                    PGuard, 0, ch, TO_ROOM);
                return (TRUE);
            }
        } else if (!h->specials.fighting) {
            /*
             * check for followers in the room 
             */
            for (t = rp->people; t; t = t->next_in_room) {
                if (IS_NPC(t) && !IS_PC(t) && t->master && t != ch &&
                    t->master != ch) {
                    break;
                }
            }

            if (t) {
                act("$n says 'What is $N doing here?'", FALSE, ch, 0,
                    t, TO_ROOM);
                act("$n makes a magical gesture", FALSE, ch, 0, 0,
                    TO_ROOM);
                purge_nr = t->nr;

                for (t2 = rp->people; t2; t2 = t3) {
                    t3 = t2->next_in_room;
                    if (t2->nr == purge_nr && !IS_PC(t2)) {
                        act("$N, looking very surprised, quickly fades out of"
                            " existence.", FALSE, ch, 0, t2, TO_ROOM);
                        extract_char(t2);
                    }
                }
            } else {
                /*
                 * check for polymorphs in the room 
                 */
                for (t = rp->people; t; t = t->next_in_room) {
                    if (IS_NPC(t) && IS_PC(t)) {
                        /* ah..  polymorphed :-) */
                        act("$n glares at $N", FALSE, ch, 0, t, TO_NOTVICT);
                        act("$n glares at you", FALSE, ch, 0, t, TO_VICT);
                        act("$n says 'Seek not to disguise your true form from"
                            " me.", FALSE, ch, 0, t, TO_ROOM);
                        act("$n traces a small rune in the air", FALSE,
                            ch, 0, 0, TO_ROOM);
                        act("$n has forced you to return to your original "
                            "form!", FALSE, ch, 0, t, TO_VICT);
                        command_interpreter(t, "return");
                        return (TRUE);
                    }
                }
            }
        }
    }
#endif
    return (FALSE);
}


/*
 * Realms of Delbrandor (Coal) Special Procedures 
 */


int DogCatcher(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    char            buf[128];
    struct char_data *tch;

    if (ch->specials.fighting) {
        if (magic_user(ch, cmd, arg, mob, type)) {
            return (TRUE);
        }
        if (!check_soundproof(ch) && !number(0, 120)) {
            command_interpreter(ch, "shout To me, my fellows! I am in need of"
                                    " thy aid!");
            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, type);
            }
            return (TRUE);
        }
    }

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    if (cmd && AWAKE(ch)) {
        if (ch->desc && GetMaxLevel(ch) < 15) {
            /* 
             * keep newbies from trying to kill elminster 
             */
            switch (cmd) {
            case 370:
            case 283:
            case 84:
                send_to_char("Elminster says ,'Do that somewhere else "
                             "please.'\n\r", ch);
                return (TRUE);
                break;
            case 25:
            case 154:
            case 157:
            case 159:
            case 156:
            case 354:
            case 346:
                send_to_char("You decide to not waste your life.\n\r", ch);
                return (TRUE);
                break;

            default:
                return (FALSE);
                break;
            }
        }
        return (FALSE);
    }

    /*
     * end was cmd 
     */
    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (!IsHumanoid(tch) && !IS_PC(tch) && IS_NPC(tch) && 
            CAN_SEE(ch, tch)) {

#if 0                           /* always true, find out why! */
            rp = real_roomp(HOLDING_TANK);
            if (MobCountInRoom(rp->people) >= HOLDING_MAX) {
                Log("Catcher room was full");
                return (FALSE);
            }
#endif
            /*
             * check level of mob, greater than catcher? 
             */
            if (GetMaxLevel(tch) > GetMaxLevel(ch) && number(1, 100) > 50) {
                /* 
                 * he was higher level and we just decided to ignore him for 
                 * now 
                 */
                return (FALSE);
            }

            /*
             * check if mob is follower 
             */

            if (tch->master && tch->master->in_room == ch->in_room &&
                CAN_SEE(ch, tch->master)) {
                switch (number(0, 3)) {
                case 0:
                    if (!check_soundproof(ch)) {
                        act("$n kicks some dirt at $N and says 'Luck is with "
                            "you today.'", FALSE, ch, 0, tch, TO_ROOM);
                    }
                    break;
                case 1:
                    if (!check_soundproof(ch)) {
                        act("$n points his wand at $N 'You best leave with "
                            "your master.'", FALSE, ch, 0, tch, TO_ROOM);
                    }
                    break;
                case 2:
                    sprintf(buf, "%s glances at %s with a look of anger, then "
                                 "snorts.",
                            ch->player.short_descr, GET_NAME(tch->master));

                    if (!check_soundproof(ch)) {
                        act(buf, FALSE, ch, 0, tch, TO_ROOM);
                    }
                    break;
                case 3:
                default:
                    if (!check_soundproof(ch)) {
                        act("$n growls at $N 'If your master were not here...'",
                            FALSE, ch, 0, tch, TO_ROOM);
                    }
                    break;
                }
                return (TRUE);
            }

            /*
             * mob was just a mob and was not a follower type 
             */

            if (!check_soundproof(ch)) {
                act("$n points a wand at $N and says 'Get thee to the woods "
                    "creature!'", FALSE, ch, 0, tch, TO_ROOM);
            } else {
                act("$n pulls out a wand and points it at $N.",
                    FALSE, ch, 0, tch, TO_ROOM);
            }

            act("A blinding flash of light envolopes $N and it is GONE!",
                FALSE, ch, 0, tch, TO_ROOM);
            char_from_room(tch);
            char_to_room(tch, HOLDING_TANK);
            act("In a blinding flash of light $n appears!", FALSE, tch, 0, 0,
                TO_ROOM);

            if (!check_soundproof(ch)) {
                act("$n smirks at his wand and says 'Teach $N to invade my "
                    "city!'", FALSE, ch, 0, tch, TO_ROOM);
            } else {
                act("$n blows at the end of the wand and smiles.",
                    FALSE, ch, 0, 0, TO_ROOM);
            }

            return (TRUE);
        }
    }

    if (!number(0, 10)) {
        switch (number(1, 4)) {
        case 1:
            command_interpreter(ch, "say Curses!  Animals in my city leaving "
                                    "droppings!");
            break;
        case 2:
            command_interpreter(ch, "say Got them locked in my pen, never "
                                    "bother me again.");
            break;
        case 3:
            command_interpreter(ch, "say Hrm... I do not have inspiration...");
            do_action(ch, NULL, 188);
            do_action(ch, NULL, 128);
            break;
        case 4:
            command_interpreter(ch, "say Where has that scribe run off to!?");
            break;
        default:
            command_interpreter(ch, "say You seen the invaders?");
            break;

        }
        return (TRUE);
    } else if (!number(0, 50)) {
        switch (number(1, 4)) {
        case 1:
            command_interpreter(ch, "shout Hey! Where is that beautiful babe "
                                    "Crystera?!?");
            command_interpreter(ch, "shout Uh... Noone tell Ugha i said that "
                                    "k?");
            break;
        case 2:
            command_interpreter(ch, "shout I found someone that spams even "
                                    "more than me!");
            command_interpreter(ch, "shout Where is that Ugha now??");
            break;
        case 3:
            command_interpreter(ch, "shout Anyone seen that scribe?");
            break;
        case 4:
            command_interpreter(ch, "shout Manwe will help us get rid of the "
                                    "invaders!");
            break;
        default:
            break;

        }
        return (TRUE);
    }

    GreetPeople(ch);
    return (FALSE);
}





/*
 * holy hand-grenade of antioch code 
 */

int antioch_grenade(struct char_data *ch, int cmd, char *arg,
                    struct obj_data *obj, int type)
{

    if (type == PULSE_TICK && obj->value[0]) {
        obj->value[0] -= 1;
    }

    if (type != PULSE_COMMAND) {
        return (0);
    }
    if (cmd == CMD_SAY || cmd == CMD_ASAY) {
        arg = skip_spaces(arg);
        if( !arg ) {
            return( 0 );
        }

        if (!strcmp(arg, "one")) {
            obj->value[0] = 4;
        } else if (!strcmp(arg, "two") && obj->value[0] >= 3 && 
                   obj->value[0] <= 4) {
            obj->value[0] = 15;
        } else if (!strcmp(arg, "three") && obj->value[0] >= 14) {
            obj->value[0] += 10;
        } else if (!strcmp(arg, "five")) {
            if (obj->value[0] >= 14 && 
                obj->value[0] <= 15) {
                obj->value[0] = 35;
            } else {
                obj->value[0] = 0;
            }
        } else {
            return (0);
        }
    }
    return (0);
}


/*
 * chess_game() stuff starts here 
 * Inspiration and original idea by Feith 
 * Implementation by Gecko 
 */

#define WHITE 0
#define BLACK 1

int             side = WHITE;   /* to avoid having to pass side with each
                                 * function call */

#define IS_BLACK(piece) (((piece) >= 1400) && ((piece) <= 1415))
#define IS_WHITE(piece) (((piece) >= 1448) && ((piece) <= 1463))
#define IS_PIECE(piece) ((IS_WHITE(piece)) || (IS_BLACK(piece)))
#define IS_ENEMY(piece) (side?IS_WHITE(piece):IS_BLACK(piece))
#define IS_FRIEND(piece) (side?IS_BLACK(piece):IS_WHITE(piece))
#define ON_BOARD(room) (((room) >= 1400) && ((room) <= 1463))
#define FORWARD (side?2:0)
#define BACK    (side?0:2)
#define LEFT    (side?1:3)
#define RIGHT   (side?3:1)

#define EXIT_ROOM(roomp,dir) ((roomp)?((roomp)->dir_option[dir]):NULL)
#define CAN_GO_ROOM(roomp,dir) (EXIT_ROOM(roomp,dir) && \
                               real_roomp(EXIT_ROOM(roomp,dir)->to_room))

/*
 * get pointer to room in the given direction 
 */
#define ROOMP(roomp,dir) ((CAN_GO_ROOM(roomp,dir)) ? \
                          real_roomp(EXIT_ROOM(roomp,dir)->to_room) : NULL)

struct room_data *forward_square(struct room_data *room)
{
    return ROOMP(room, FORWARD);
}

struct room_data *back_square(struct room_data *room)
{
    return ROOMP(room, BACK);
}

struct room_data *left_square(struct room_data *room)
{
    return ROOMP(room, LEFT);
}

struct room_data *right_square(struct room_data *room)
{
    return ROOMP(room, RIGHT);
}

struct room_data *forward_left_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, FORWARD), LEFT);
}

struct room_data *forward_right_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, FORWARD), RIGHT);
}

struct room_data *back_right_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, BACK), RIGHT);
}

struct room_data *back_left_square(struct room_data *room)
{
    return ROOMP(ROOMP(room, BACK), LEFT);
}

struct char_data *square_contains_enemy(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_ENEMY(mob_index[i->nr].vnum)) {
            return i;
        }
    }

    return NULL;
}

int square_contains_friend(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_FRIEND(mob_index[i->nr].vnum)) {
            return TRUE;
        }
    }

    return FALSE;
}

int square_empty(struct room_data *square)
{
    struct char_data *i;

    for (i = square->people; i; i = i->next_in_room) {
        if (IS_PIECE(mob_index[i->nr].vnum)) {
            return FALSE;
        }
    }

    return TRUE;
}

int chess_game(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct room_data *rp = NULL,
                   *crp = real_roomp(ch->in_room);
    struct char_data *ep = NULL;
    int             move_dir = 0,
                    move_amount = 0,
                    move_found = FALSE;
    int             c = 0;

    if (cmd || !AWAKE(ch)) {
        return FALSE;
    }
    /*  
     * keep original fighter() spec_proc for kings and knights 
     */
    if (ch->specials.fighting) {
        switch (mob_index[ch->nr].vnum) {
        case 1401:
        case 1404:
        case 1406:
        case 1457:
        case 1460:
        case 1462:
            return fighter(ch, cmd, arg, mob, type);
        default:
            return FALSE;
        }
    }
    if (!crp || !ON_BOARD(crp->number)) {
        return FALSE;
    }
    if (side == WHITE && IS_BLACK(mob_index[ch->nr].vnum)) {
        return FALSE;
    }
    if (side == BLACK && IS_WHITE(mob_index[ch->nr].vnum)) {
        return FALSE;
    }
    if (number(0, 15)) {
        return FALSE;
    }
    switch (mob_index[ch->nr].vnum) {
    case 1408:                  /* black pawns */
    case 1409:
    case 1410:
    case 1411:
    case 1412:
    case 1413:
    case 1414:
    case 1415:
    case 1448:
    case 1449:
    case 1450:
    case 1451:                  /* white pawns */
    case 1452:
    case 1453:
    case 1454:
    case 1455:
        move_dir = number(0, 3);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(crp);
            break;
        case 1:
            rp = forward_right_square(crp);
            break;
        case 2:
            rp = forward_square(crp);
            break;
        case 3:
            if (real_roomp(ch->in_room) &&
                real_roomp(ch->in_room)->number == mob_index[ch->nr].vnum) {
                rp = forward_square(crp);
                if (rp && square_empty(rp) && ON_BOARD(rp->number)) {
                    crp = rp;
                    rp = forward_square(crp);
                }
            }
        }
        if (rp && (!square_contains_friend(rp)) && ON_BOARD(rp->number)) {
            ep = square_contains_enemy(rp);
            if ((move_dir <= 1 && ep) || (move_dir > 1 && !ep)) {
                move_found = TRUE;
            }
        }
        break;

    case 1400:                  /* black rooks */
    case 1407:
    case 1456:                  /* white rooks */
    case 1463:
        move_dir = number(0, 3);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_square(crp);
                break;
            case 1:
                rp = back_square(crp);
                break;
            case 2:
                rp = right_square(crp);
                break;
            case 3:
                rp = left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1401:                  /* black knights */
    case 1406:
    case 1457:                  /* white knights */
    case 1462:
        move_dir = number(0, 7);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(forward_square(crp));
            break;
        case 1:
            rp = forward_right_square(forward_square(crp));
            break;
        case 2:
            rp = forward_right_square(right_square(crp));
            break;
        case 3:
            rp = back_right_square(right_square(crp));
            break;
        case 4:
            rp = back_right_square(back_square(crp));
            break;
        case 5:
            rp = back_left_square(back_square(crp));
            break;
        case 6:
            rp = back_left_square(left_square(crp));
            break;
        case 7:
            rp = forward_left_square(left_square(crp));
        }

        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;

    case 1402:                  /* black bishops */
    case 1405:
    case 1458:                  /* white bishops */
    case 1461:
        move_dir = number(0, 3);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_left_square(crp);
                break;
            case 1:
                rp = forward_right_square(crp);
                break;
            case 2:
                rp = back_right_square(crp);
                break;
            case 3:
                rp = back_left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1403:                  /* black queen */
    case 1459:                  /* white queen */
        move_dir = number(0, 7);
        move_amount = number(1, 7);
        for (c = 0; c < move_amount; c++) {
            switch (move_dir) {
            case 0:
                rp = forward_left_square(crp);
                break;
            case 1:
                rp = forward_square(crp);
                break;
            case 2:
                rp = forward_right_square(crp);
                break;
            case 3:
                rp = right_square(crp);
                break;
            case 4:
                rp = back_right_square(crp);
                break;
            case 5:
                rp = back_square(crp);
                break;
            case 6:
                rp = back_left_square(crp);
                break;
            case 7:
                rp = left_square(crp);
            }

            if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
                move_found = TRUE;
                if ((ep = square_contains_enemy(rp))) {
                    c = move_amount;
                } else {
                    crp = rp;
                }
            } else {
                c = move_amount;
                rp = crp;
            }
        }
        break;

    case 1404:                  /* black king */
    case 1460:                  /* white king */
        move_dir = number(0, 7);
        switch (move_dir) {
        case 0:
            rp = forward_left_square(crp);
            break;
        case 1:
            rp = forward_square(crp);
            break;
        case 2:
            rp = forward_right_square(crp);
            break;
        case 3:
            rp = right_square(crp);
            break;
        case 4:
            rp = back_right_square(crp);
            break;
        case 5:
            rp = back_square(crp);
            break;
        case 6:
            rp = back_left_square(crp);
            break;
        case 7:
            rp = left_square(crp);
        }

        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
            move_found = TRUE;
            ep = square_contains_enemy(rp);
        }
        break;
    }

    if (move_found && rp) {
        command_interpreter(ch, "emote leaves the room.");
        char_from_room(ch);
        char_to_room(ch, rp->number);
        command_interpreter(ch, "emote has arrived.");

        if (ep) {
            if (side) {
                switch (number(0, 3)) {
                case 0:
                    command_interpreter(ch, "emote grins evilly and says, "
                                            "'ONLY EVIL shall rule!'");
                    break;
                case 1:
                    command_interpreter(ch, "emote leers cruelly and says, "
                                            "'You will die now!'");
                    break;
                case 2:
                    command_interpreter(ch, "emote issues a bloodcurdling "
                                            "scream.");
                    break;
                case 3:
                    command_interpreter(ch, "emote glares with black anger.");
                    break;
                }
            } else {
                switch (number(0, 3)) {
                case 0:
                    command_interpreter(ch, "emote glows an even brighter "
                                            "pristine white.");
                    break;
                case 1:
                    command_interpreter(ch, "emote chants a prayer and begins "
                                            "battle.");
                    break;
                case 2:
                    command_interpreter(ch, "emote says, 'Black shall lose!");
                    break;
                case 3:
                    command_interpreter(ch, "emote shouts, 'For the Flame! The"
                                            " Flame!'");
                    break;
                }
            }
            hit(ch, ep, TYPE_UNDEFINED);
        }
        side = (side + 1) % 2;
        return TRUE;
    }
    return FALSE;
}

/**
 * @todo change to use the corpse flag to determine that it's a corpse or not
 */
int AcidBlob(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
        if (IS_OBJ_STAT(i, wear_flags, ITEM_TAKE) && 
            !strncmp(i->keywords.words[0], "corpse", 6)) {
            act("$n destroys some trash.", FALSE, ch, 0, 0, TO_ROOM);

            objectTakeFromRoom(i);
            objectExtract(i);
            return (TRUE);
        }
    }
    return (FALSE);
}


int avatar_celestian(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (number(0, 1)) {
        return (fighter(mob, cmd, arg, mob, type));
    } else {
        return (magic_user(mob, cmd, arg, mob, type));
    }
}







int EvilBlade(struct char_data *ch, int cmd, char *arg,
              struct obj_data *tobj, int type)
{
#ifdef USE_EGOS
    extern struct str_app_type str_app[];
    struct obj_data *obj,
                   *blade;
    struct char_data *joe,
                   *holder;
    struct char_data *lowjoe = 0;
    char           *arg1,
                    buf[250];
    struct index_data *index;
    Keywords_t     *key;

    if ((type != PULSE_COMMAND) || (IS_IMMORTAL(ch)) || 
        (!real_roomp(ch->in_room))) {
        return (FALSE);
    }
    for (obj = real_roomp(ch->in_room)->contents;
         obj; obj = obj->next_content) {
        if (obj->index->func == EvilBlade) {
            /*
             * I am on the floor 
             */
            for (joe = real_roomp(ch->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if ((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe) {
                if (CAN_GET_OBJ(lowjoe, obj)) {
                    objectTakeFromRoom(obj);
                    objectGiveToChar(obj, lowjoe);

                    sprintf(buf, "%s leaps into your hands!\n\r",
                            obj->short_description);
                    send_to_char(buf, lowjoe);

                    sprintf(buf, "%s jumps from the floor and leaps into %s's"
                                 " hands!\n\r",
                            obj->short_description, GET_NAME(lowjoe));
                    act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                    if (!EgoBladeSave(lowjoe)) {
                        if (!lowjoe->equipment[WIELD]) {
                            sprintf(buf, "%s forces you to wield it!\n\r",
                                    obj->short_description);
                            send_to_char(buf, lowjoe);
                            wear(lowjoe, obj, 12);
                            return (FALSE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to make you "
                                         "wield it.\n\r", 
                                    obj->short_description);
                            send_to_char(buf, lowjoe);
                            return (FALSE);
                        }
                    }
                }
            }
        }
    }

    for (holder = real_roomp(ch->in_room)->people; holder;
         holder = holder->next_in_room) {
        for (obj = holder->carrying; obj; obj = obj->next_content) {
            if (obj->index->func && obj->index->func != board) {
                /*
                 * held
                 */
                if (holder->equipment[WIELD] && !EgoBladeSave(holder) && 
                    !EgoBladeSave(holder)) {
                    sprintf(buf, "%s gets pissed off that you are wielding "
                                 "another weapon!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                    sprintf(buf, "%s knocks %s out of your hands!!\n\r",
                            obj->short_description,
                            holder->equipment[WIELD]->short_description);
                    send_to_char(buf, holder);

                    blade = unequip_char(holder, WIELD);
                    if (blade) {
                        objectPutInRoom(blade, holder->in_room);
                    }
                    if (!holder->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);
                        wear(holder, obj, 12);
                        return (FALSE);
                    }
                }

                if (!EgoBladeSave(holder) && !EgoBladeSave(holder) &&
                    !holder->equipment[WIELD]) {
                    sprintf(buf, "%s forces you to wield it!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                    wear(holder, obj, 12);
                    return (FALSE);
                }

                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the %s, you feel less enthused about"
                                 " your master.\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }
            }
        }

        if (holder->equipment[WIELD] && 
            (index = holder->equipment[WIELD]->index) &&
            index->func && index->func != board) {
            /*
             * YES! I am being held!
             */
            obj = holder->equipment[WIELD];
            if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                affect_from_char(holder, SPELL_CHARM_PERSON);
                sprintf(buf, "Due to the %s, you feel less enthused about "
                             "your master.\n\r", obj->short_description);
                send_to_char(buf, holder);
            }

            if (holder->specials.fighting) {
                sprintf(buf, "%s almost sings in your hand!\n\r",
                        obj->short_description);
                send_to_char(buf, holder);
                sprintf(buf, "You can hear $n's %s almost sing with joy!",
                        obj->short_description);
                act(buf, FALSE, holder, 0, 0, TO_ROOM);

                if (holder == ch && cmd == 151) {
                    if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
                        sprintf(buf, "You can feel %s attempt to stay in the "
                                     "fight!\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        return (FALSE);
                    } else {
                        sprintf(buf, "%s laughs at your attempt to flee from "
                                     "a fight!\n\r", obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s gives you a little warning...\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s twists around and smacks you!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "Wow! $n's %s just whipped around and "
                                     "smacked $m one!", obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);

                        GET_HIT(ch) -= 25;
                        if (GET_HIT(ch) < 0) {
                            GET_HIT(ch) = 0;
                            GET_POS(ch) = POSITION_STUNNED;
                        }
                        return (TRUE);
                    }
                }
            }
            
            if (cmd == 66 && holder == ch) {
                arg = get_argument(arg, &arg1);
                if( !arg1 ) {
                    send_to_char( "Remove what?\n\r", ch );
                    return( FALSE );
                }

                if (strcmp(arg1, "all") == 0) {
                    if (!EgoBladeSave(ch)) {
                        sprintf(buf, "%s laughs at your attempt remove it!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s gives you a little warning...\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "%s twists around and smacks you!\n\r",
                                obj->short_description);
                        send_to_char(buf, ch);

                        sprintf(buf, "Wow! $n's %s just whipped around and "
                                     "smacked $m one!", obj->short_description);
                        act(buf, FALSE, ch, 0, 0, TO_ROOM);

                        GET_HIT(ch) -= 25;
                        if (GET_HIT(ch) < 0) {
                            GET_HIT(ch) = 0;
                            GET_POS(ch) = POSITION_STUNNED;
                        }
                        return (TRUE);
                    } else {
                        sprintf(buf, "You can feel %s attempt to stay "
                                     "wielded!\n\r", obj->short_description);
                        send_to_char(buf, ch);
                        return (FALSE);
                    }
                } else {
                    key = StringToKeywords( arg1, NULL );
                    key->partial = TRUE;
                    if (KeywordsMatch(key, &obj->keywords)) {
                        FreeKeywords( key, TRUE );
                        if (!EgoBladeSave(ch)) {
                            sprintf(buf, "%s laughs at your attempt to remove"
                                         " it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!", 
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 25;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to stay "
                                         "wielded!\n\r", 
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        }
                    }
                    FreeKeywords( key, TRUE );
                }
            }

            for (joe = real_roomp(holder->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if (GET_ALIGNMENT(joe) >= 500 && IS_MOB(joe) && 
                    CAN_SEE(holder, joe) && holder != joe) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe) {
                if (EgoBladeSave(holder)) {
                    return (FALSE);
                }

                if (GET_POS(holder) != POSITION_STANDING) {
                    sprintf(buf, "%s yanks you to your feet!\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);
                    GET_POS(holder) = POSITION_STANDING;
                }

                sprintf(buf, "%s leaps out of control!!\n\r",
                        obj->short_description);
                send_to_char(buf, holder);
                sprintf(buf, "%s jumps for $n's neck!", obj->short_description);
                act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);
                sprintf(buf, "hit %s", GET_NAME(lowjoe));
                command_interpreter(holder, buf);
                return (TRUE);
            }

            if (cmd == 70 && holder == ch) {
                sprintf(buf, "%s almost sings in your hands!!\n\r",
                        obj->short_description);
                send_to_char(buf, ch);
                sprintf(buf, "You can hear $n's %s almost sing with joy!",
                        obj->short_description);

                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                return (FALSE);
            }
        }
    }
#endif
    return (FALSE);
}

int GoodBlade(struct char_data *ch, int cmd, char *arg,
              struct obj_data *tobj, int type)
{
#ifdef USE_EGOS
    extern struct str_app_type str_app[];
    struct obj_data *obj,
                   *blade;
    struct char_data *joe,
                   *holder;
    struct char_data *lowjoe = 0;
    char           *arg1,
                    buf[250];
    struct index_data *index;
    Keywords_t     *key;

    if ((type != PULSE_COMMAND) || (IS_IMMORTAL(ch)) || 
        (!real_roomp(ch->in_room))) {
        return (FALSE);
    }
    return (FALSE);
    /*
     * disabled
     */
    for (obj = real_roomp(ch->in_room)->contents;
         obj; obj = obj->next_content) {
        if (obj->index->func == GoodBlade) {
            /*
             * I am on the floor 
             */
            for (joe = real_roomp(ch->in_room)->people; joe;
                 joe = joe->next_in_room) {
                if (GET_ALIGNMENT(joe) >= 350 && !IS_IMMORTAL(joe)) {
                    if (lowjoe) {
                        if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)) {
                            lowjoe = joe;
                        }
                    } else
                        lowjoe = joe;
                }
            }

            if (lowjoe && CAN_GET_OBJ(lowjoe, obj)) {
                objectTakeFromRoom(obj);
                objectGiveToChar(obj, lowjoe);

                sprintf(buf, "%s leaps into your hands!\n\r",
                        obj->short_description);
                send_to_char(buf, lowjoe);

                sprintf(buf, "%s jumps from the floor and leaps into %s's "
                             "hands!\n\r", obj->short_description,
                        GET_NAME(lowjoe));
                act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                if (!EgoBladeSave(lowjoe)) {
                    if (!lowjoe->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, lowjoe);
                        wear(lowjoe, obj, 12);
                        return (FALSE);
                    } else {
                        sprintf(buf, "You can feel %s attept to make you wield"
                                     " it.\n\r", obj->short_description);
                        send_to_char(buf, lowjoe);
                        return (FALSE);
                    }
                }
            }
        }
    }

    for (holder = real_roomp(ch->in_room)->people; holder;
         holder = holder->next_in_room) {
        for (obj = holder->carrying; obj; obj = obj->next_content) {
            if (obj->index->func && obj->index->func != board) {
                /*
                 * held
                 */
                if (holder->equipment[WIELD] && !EgoBladeSave(holder) && 
                    !EgoBladeSave(holder)) {
                    sprintf(buf, "%s gets pissed off that you are wielding "
                                 "another weapon!\n\r", obj->short_description);
                    send_to_char(buf, holder);

                    sprintf(buf, "%s knocks %s out of your hands!!\n\r",
                            obj->short_description,
                            holder->equipment[WIELD]->short_description);
                    send_to_char(buf, holder);

                    blade = unequip_char(holder, WIELD);
                    if (blade) {
                        objectPutInRoom(blade, holder->in_room);
                    }
                    if (!holder->equipment[WIELD]) {
                        sprintf(buf, "%s forces you to wield it!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);
                        wear(holder, obj, 12);
                        return (FALSE);
                    }
                }

                if (!EgoBladeSave(holder) && !EgoBladeSave(holder) &&
                    !holder->equipment[WIELD]) {
                    sprintf(buf, "%s forces you yto wield it!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                    wear(holder, obj, 12);
                    return (FALSE);
                }

                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the effects of %s, you feel less "
                                 "enthused about your master.\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }
            }
        }

        if (holder->equipment[WIELD]) {
            index = holder->equipment[WIELD]->index;
            if ( index && index->func && index->func != board) {
                /*
                 * YES! I am being held!
                 */
                obj = holder->equipment[WIELD];

                /*
                 * remove charm 
                 */
                if (affected_by_spell(holder, SPELL_CHARM_PERSON)) {
                    affect_from_char(holder, SPELL_CHARM_PERSON);
                    sprintf(buf, "Due to the effects of %s, you feel less "
                                 "enthused about your master.\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove blindness 
                 */
                if (affected_by_spell(holder, SPELL_BLINDNESS)) {
                    affect_from_char(holder, SPELL_BLINDNESS);
                    sprintf(buf, "%s hums in your hands, you can see!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove chill touch 
                 */
                if (affected_by_spell(holder, SPELL_CHILL_TOUCH)) {
                    affect_from_char(holder, SPELL_CHILL_TOUCH);
                    sprintf(buf, "%s hums in your hands, you feel warm "
                                 "again!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove slow 
                 */
                if (affected_by_spell(holder, SPELL_SLOW)) {
                    affect_from_char(holder, SPELL_SLOW);
                    sprintf(buf, "%s hums in your hands, you feel yourself "
                                 "speed back up!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * remove poison 
                 */
                if (affected_by_spell(holder, SPELL_POISON)) {
                    affect_from_char(holder, SPELL_POISON);
                    sprintf(buf, "%s hums in your hands, the sick feeling "
                                 "fades!\n\r", obj->short_description);
                    send_to_char(buf, holder);
                }

                /*
                 * wielder is hurt, heal them 
                 */
                if (number(1, 101) > 90 && 
                    GET_HIT(holder) < GET_MAX_HIT(holder) / 2) {
                    act("You get a gentle warm pulse from $p, you feel MUCH "
                        "better!", FALSE, holder, obj, 0, TO_CHAR);
                    act("$n smiles as $p pulses in $s hands!", FALSE,
                        holder, obj, 0, TO_ROOM);
                    GET_HIT(holder) = GET_MAX_HIT(holder) - number(1, 10);
                    return (FALSE);
                }

                if (holder->specials.fighting) {
                    sprintf(buf, "%s almost sings in your hand!\n\r",
                            obj->short_description);
                    send_to_char(buf, holder);

                    sprintf(buf, "You can hear %s almost sing with joy in $n's"
                                 " hands!", obj->short_description);
                    act(buf, FALSE, holder, 0, 0, TO_ROOM);

                    if (holder == ch && cmd == 151) {
                        if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
                            sprintf(buf, "You can feel %s attempt to stay in "
                                         "the fight!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        } else {
                            sprintf(buf, "%s laughs at your attempt to flee "
                                         "from a fight!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        }
                    }
                }

                if (cmd == 66 && holder == ch) {
                    arg = get_argument(arg, &arg1);
                    if( !arg1 ) {
                        send_to_char("Remove what?\n\r", ch);
                        return( FALSE );
                    }

                    if (strcmp(arg1, "all") == 0) {
                        if (!EgoBladeSave(ch)) {
                            sprintf(buf, "%s laughs at your attempt remove "
                                         "it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        } else {
                            sprintf(buf, "You can feel %s attempt to stay "
                                         "wielded!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            return (FALSE);
                        }
                    } else {
                        key = StringToKeywords(arg1, NULL);
                        if (KeywordsMatch(key, &obj->keywords)) {
                            FreeKeywords( key, TRUE );
                            if (EgoBladeSave(ch)) {
                                sprintf(buf, "You can feel %s attempt to stay "
                                             "wielded!\n\r",
                                        obj->short_description);
                                send_to_char(buf, ch);
                                return (FALSE);
                            }

                            sprintf(buf, "%s laughs at your attempt to remove"
                                         " it!\n\r", obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s gives you a little warning...\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "%s twists around and smacks you!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);

                            sprintf(buf, "Wow! $n's %s just whipped around and"
                                         " smacked $m one!",
                                    obj->short_description);
                            act(buf, FALSE, ch, 0, 0, TO_ROOM);

                            GET_HIT(ch) -= 10;
                            if (GET_HIT(ch) < 0) {
                                GET_HIT(ch) = 0;
                                GET_POS(ch) = POSITION_STUNNED;
                            }
                            return (TRUE);
                        }
                        FreeKeywords( key, TRUE );
                    }
                }

                for (joe = real_roomp(holder->in_room)->people; joe;
                     joe = joe->next_in_room) {
                    if (GET_ALIGNMENT(joe) <= -350 && IS_MOB(joe) && 
                        CAN_SEE(holder, joe) && holder != joe) {
                        if (lowjoe) {
                            if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)) {
                                lowjoe = joe;
                            }
                        } else
                            lowjoe = joe;
                    }
                }

                if (lowjoe) {
                    if (!EgoBladeSave(holder)) {
                        if (GET_POS(holder) != POSITION_STANDING) {
                            sprintf(buf, "%s yanks you yo your feet!\n\r",
                                    obj->short_description);
                            send_to_char(buf, ch);
                            GET_POS(holder) = POSITION_STANDING;
                        }

                        sprintf(buf, "%s leaps out of control!!\n\r",
                                obj->short_description);
                        send_to_char(buf, holder);

                        sprintf(buf, "%s howls out for $n's neck!",
                                obj->short_description);
                        act(buf, FALSE, lowjoe, 0, 0, TO_ROOM);

                        sprintf(buf, "hit %s", GET_NAME(lowjoe));
                        command_interpreter(holder, buf);
                        return (TRUE);
                    } else {
                        return (FALSE);
                    }
                }

                if (cmd == 70 && holder == ch) {
                    sprintf(buf, "%s almost sings in your hands!!\n\r",
                            obj->short_description);
                    send_to_char(buf, ch);

                    sprintf(buf, "You can hear $n's %s almost sing with joy!",
                            obj->short_description);
                    act(buf, FALSE, ch, 0, 0, TO_ROOM);
                    return (FALSE);
                }
            }
        }
    }

#endif
    return (FALSE);
}

int NeutralBlade(struct char_data *ch, int cmd, char *arg,
                 struct obj_data *tobj, int type)
{
    return (FALSE);
}




int magic_user_imp(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell;
    char            buf[254];

    if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    /*
     * might move this somewhere else later... 
     */

    SET_BIT(ch->player.class, CLASS_MAGIC_USER);
    ch->player.level[MAGE_LEVEL_IND] = GetMaxLevel(ch);

    if (!ch->specials.fighting && !IS_PC(ch)) {
        if (GetMaxLevel(ch) < 25) {
            return FALSE;
        } else {
            if (!ch->desc) {
                if (Summoner(ch, cmd, arg, mob, type)) {
                    return (TRUE);
                } else if (NumCharmedFollowersInRoom(ch) < 5 && 
                         IS_SET(ch->hatefield, HATE_CHAR)) {
                    act("$n utters the words 'Here boy!'.", 1, ch, 0,
                        0, TO_ROOM);
                    cast_mon_sum7(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                    command_interpreter(ch, "order followers guard on");
                    return (TRUE);
                }
            }
            return FALSE;
        }
    }

    if (!ch->specials.fighting) {
        return FALSE;
    }
    if (!IS_PC(ch) && GET_POS(ch) > POSITION_STUNNED &&
        GET_POS(ch) < POSITION_FIGHTING) {
        StandUp(ch);
        if (GET_HIT(ch) <= GET_HIT(ch->specials.fighting) / 2) {
            do_flee(ch, NULL, 0);
        }
        WAIT_STATE(ch, PULSE_VIOLENCE * 3);
        return (TRUE);
    }

    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (number(0, 1) && UseViolentHeldItem(ch)) {
        return (TRUE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    /*
     * Find a dude to to evil things upon ! 
     */

    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    lspell = number(0, GetMaxLevel(ch));
    if (!IS_PC(ch)) {
        /* weight it towards the upper levels of the mages range */
        lspell += GetMaxLevel(ch) / 5;
    }
    lspell = MIN(GetMaxLevel(ch), lspell);

    /*
     **  check your own problems:
     */

    if (lspell < 1) {
        lspell = 1;
    }
    /*
     ** only problem I can see with this new spell casting is if the mobs
     ** name is the same as the victim....
     */

    if (IS_AFFECTED(ch, AFF_BLIND) && lspell > 15) {
        sprintf(buf, "cast 'remove blind' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 10 &&
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        sprintf(buf, "cast 'dispel magic' %s", GET_NAME(vict));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 10 &&
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        sprintf(buf, "cast 'dispel magic' %s", GET_NAME(vict));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 28 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {

#if 0
        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
            return (TRUE);
        }
#endif
        sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 15 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
        command_interpreter(ch, buf);
        return (FALSE);
    }

    if (GET_HIT(ch) > GET_MAX_HIT(ch) / 2 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
        GetMaxLevel(vict) < GetMaxLevel(ch) && number(0, 1)) {

        /*
         **  Non-damaging case:
         */

        if (lspell > 8 && lspell < 50 && !number(0, 6)) {
            sprintf(buf, "cast 'web' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 6)) {
            sprintf(buf, "cast 'weakness' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 7)) {
            sprintf(buf, "cast 'armor' %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 12 && lspell < 20 && !number(0, 7)) {
            sprintf(buf, "cast 'curse' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 10 && lspell < 20 && !number(0, 5)) {
            sprintf(buf, "cast 'blind' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            return TRUE;
        }

        if (lspell > 8 && lspell < 40 && !number(0, 5) &&
            vict->specials.fighting != ch) {
            sprintf(buf, "cast 'charm monster' %s", GET_NAME(vict));
            command_interpreter(ch, buf);
            if (IS_AFFECTED(vict, AFF_CHARM)) {
                if (!vict->specials.fighting) {
                    sprintf(buf, "order %s kill %s",
                            GET_NAME(vict), GET_NAME(ch->specials.fighting));
                } else {
                    sprintf(buf, "order %s remove all", GET_NAME(vict));
                }
                command_interpreter(ch, buf);
            }
        }

        /*
         **  The really nifty case:
         */
        switch (lspell) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            command_interpreter(ch, "cast 'monsum one'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 11:
        case 12:
        case 13:
            command_interpreter(ch, "cast 'monsum two'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 14:
        case 15:
            command_interpreter(ch, "cast 'monsum three'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 16:
        case 17:
        case 18:
            command_interpreter(ch, "cast 'monsum four'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 19:
        case 20:
        case 21:
        case 22:
            command_interpreter(ch, "cast 'monsum five'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 23:
        case 24:
        case 25:
            command_interpreter(ch, "cast 'monsum six'");
            command_interpreter(ch, "order followers guard on");
            break;
        case 26:
        default:
            command_interpreter(ch, "cast 'monsum seven'");
            command_interpreter(ch, "order followers guard on");
            break;
        }
        return (TRUE);
    } else {
        buf[0] = '\0';
        switch (lspell) {
        case 1:
        case 2:
            sprintf(buf, "cast 'magic missle' %s", GET_NAME(vict));
            break;
        case 3:
        case 4:
        case 5:
            sprintf(buf, "cast 'shocking grasp' %s", GET_NAME(vict));
            break;
        case 6:
        case 7:
        case 8:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'web' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'burning hands' %s", GET_NAME(vict));
            }
            break;
        case 9:
        case 10:
            sprintf(buf, "cast 'acid blast' %s", GET_NAME(vict));
            break;
        case 11:
        case 12:
        case 13:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'lightning bolt' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'ice storm' %s", GET_NAME(vict));
            }
            break;
        case 14:
        case 15:
            sprintf(buf, "cast 'teleport' %s", GET_NAME(ch));
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'colour spray' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'cone of cold' %s", GET_NAME(vict));
            }
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
            sprintf(buf, "cast 'fireball' %s", GET_NAME(vict));
            break;
        case 38:
        case 39:
        case 40:
        case 41:
            if (IS_EVIL(ch)) {
                sprintf(buf, "cast 'energy drain' %s", GET_NAME(vict));
            }
            break;
        default:
            if (ch->attackers <= 2) {
                sprintf(buf, "cast 'meteor swarm' %s", GET_NAME(vict));
            } else {
                sprintf(buf, "cast 'fireball' %s", GET_NAME(vict));
            }
            break;
        }
        if( *buf ) {
            command_interpreter(ch, buf);
        }
    }
    return TRUE;
}

int cleric_imp(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell,
                    healperc = 0;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) != POSITION_FIGHTING) {
        if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }
        return FALSE;
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (!ch->specials.fighting && GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
        if ((lspell = GetMaxLevel(ch)) >= 20) {
            act("$n utters the words 'What a Rush!'.", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else if (lspell > 12) {
            act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1,
                ch, 0, 0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else if (lspell > 8) {
            act("$n utters the words 'I feel much better now!'.", 1,
                ch, 0, 0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        } else {
            act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        }
    }

    /*
     * Find a dude to to evil things upon ! 
     */

    if ((vict = FindAHatee(ch)) == NULL) {
        vict = FindVictim(ch);
    }
    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    /*
     * gen number from 0 to level 
     */

    lspell = number(0, GetMaxLevel(ch));
    lspell += GetMaxLevel(ch) / 5;
    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 31 &&
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n utters the words 'Woah! I'm outta here!'",
            1, ch, 0, 0, TO_ROOM);

        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            return (TRUE);
        }

        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    /*
     * first -- hit a foe, or help yourself? 
     */

    if (ch->points.hit < (ch->points.max_hit / 2)) {
        healperc = 7;
    } else if (ch->points.hit < (ch->points.max_hit / 4)) {
        healperc = 5;
    } else if (ch->points.hit < (ch->points.max_hit / 8)) {
        healperc = 3;
    }
    if (number(1, healperc + 2) > 3) {
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 &&
            !number(0, 5)) {
            act("$n whistles.", 1, ch, 0, 0, TO_ROOM);
            act("$n utters the words 'Here Lightning!'.", 1, ch, 0, 0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
        case 3:
            act("$n utters the words 'Moo ha ha!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            act("$n utters the words 'Hocus Pocus!'.", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 7:
            act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 8:
            act("$n utters the words 'Urgle Blurg'.", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 11:
            act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0, 0, TO_ROOM);
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!IS_SET(vict->M_immune, IMM_FIRE)) {
                act("$n utters the words 'Burn Baby Burn'.", 1, ch, 0,
                    0, TO_ROOM);
                cast_flamestrike(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_SANCTUARY) && 
                       GetMaxLevel(ch) > GetMaxLevel(vict)) {
                act("$n utters the words 'Va-Voom!'.", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Take That!'.", 1, ch, 0, 0, TO_ROOM);
                cast_cause_critic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 17:
        case 18:
        case 19:
        default:
            act("$n utters the words 'Hurts, doesn't it?'.", 1, ch, 0, 0,
                TO_ROOM);
            cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        }

        return (TRUE);
    } else {
        if (IS_AFFECTED(ch, AFF_BLIND) && lspell >= 4 && !number(0, 3)) {
            act("$n utters the words 'Praise <Deity Name>, I can SEE!'.",
                1, ch, 0, 0, TO_ROOM);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_CURSE) && lspell >= 6 && !number(0, 6)) {
            act("$n utters the words 'I'm rubber, you're glue.", 1, ch, 0,
                0, TO_ROOM);
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_POISON) && lspell >= 5 && !number(0, 6)) {
            act("$n utters the words 'Praise <Deity Name> I don't feel sick "
                "no more!'.", 1, ch, 0, 0, TO_ROOM);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'Abrazak'.", 1, ch, 0, 0, TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            act("$n utters the words 'I feel much better now!'.", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
        case 18:
            act("$n utters the words 'What a Rush!'.", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            act("$n utters the words 'Oooh, pretty!'.", 1, ch, 0, 0, TO_ROOM);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        }

        return (TRUE);
    }
}





/*
 * well, this paladin will give a tough battle.. imho paladins just can't
 * be stupid and do _random_ things on yourself and on opponents.. grin
 * 
 */

int Paladin(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *vict,
                   *tch;
    char            buf[255];

    if (!ch->skills || GET_LEVEL(ch, PALADIN_LEVEL_IND) <= 0) { 
        /* init skills */
        SET_BIT(ch->player.class, CLASS_PALADIN);
        if (!ch->skills) {
            SpaceForSkills(ch);
        }
        GET_LEVEL(ch, PALADIN_LEVEL_IND) = GetMaxLevel(ch);
        /*
         * set skill levels 
         */
        ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_HOLY_WARCRY].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_LAY_ON_HANDS].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_BASH].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_KICK].learned = GetMaxLevel(ch) + 40;
        ch->skills[SKILL_BLESSING].learned = GetMaxLevel(ch) + 40;
    }

    if (cmd) {
        /*
         * we will not give free blesses or lay on hands.. we will just
         * ignore all commands.. yah. 
         */
        return (FALSE);
    }

    if (affected_by_spell(ch, SPELL_PARALYSIS)) {
        /* poor guy.. */
        return (FALSE);
    }

    if (!AWAKE(mob) && !affected_by_spell(mob, SPELL_SLEEP)) {
        /*
         * hey, why you sleeping guy? STAND AND FIGHT! 
         */
        command_interpreter(mob, "wake");
        command_interpreter(mob, "stand");
        return (TRUE);
    }

    if (ch->specials.fighting && ch->specials.fighting != ch) {
        if (GET_POS(ch) == POSITION_SITTING || 
            GET_POS(ch) == POSITION_RESTING) {
            do_stand(ch, NULL, 0);
            return (TRUE);
        }

        vict = ch->specials.fighting;
        if (!vict) {
            Log("!vict in paladin");
            return (FALSE);
        }

        /*
         * well, if we in battle, do some nice things on ourself..  
         */
        if (!affected_by_spell(ch, SKILL_BLESSING)) {
            /* bless myself */
            sprintf(buf, "blessing %s", GET_NAME(ch)); 
            command_interpreter(ch, buf);
            return (TRUE);
        }

        if (!affected_by_spell(ch, SKILL_LAY_ON_HANDS) && 
            GET_HIT(ch) < GET_MAX_HIT(ch) / 2) {
            sprintf(buf, "lay %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return (TRUE);
        }

        switch (number(1, 6)) {
        case 1:
            /* intellegent kick/bash.. hmm */
            if (HasClass(vict, CLASS_SORCERER | CLASS_MAGIC_USER) || 
                HasClass(vict, CLASS_CLERIC | CLASS_PSI)) {
                do_bash(ch, NULL, 0);
            } else {
                do_kick(ch, NULL, 0);
            }
            return (TRUE);
            break;
        case 2:
            do_bash(ch, NULL, 0);
            return (TRUE);
            break;
        case 3:
            do_kick(ch, NULL, 0);
            return (TRUE);
            break;
        case 4:
            do_holy_warcry(ch, NULL, 0);
            return (TRUE);
            break;
        default:
            return (fighter(ch, cmd, arg, mob, type));
            break;
        }
    } else {
        /*
         * check our hps 
         */
        if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && 
            !affected_by_spell(ch, SKILL_LAY_ON_HANDS)) {
            sprintf(buf, "lay %s", GET_NAME(ch));
            command_interpreter(ch, buf);
            return (TRUE);
        }

        if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && 
            GET_MANA(ch) > GET_MANA(ch) / 2 && number(1, 6) > 4) {
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        /*
         * lets check some spells on us.. 
         */
        if (IS_AFFECTED(ch, AFF_POISON) && !number(0, 6) &&
            GET_LEVEL(ch, PALADIN_LEVEL_IND) > 10) {
            act("$n asks $s deity to remove poison from $s blood!", 1, ch,
                0, 0, TO_ROOM);
            if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 40) {
                cast_slow_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            } else {
                cast_remove_poison(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
            }
            return (TRUE);
        }

        if (!IS_AFFECTED2(ch, AFF2_PROTECT_FROM_EVIL) && !number(0, 6)) {
            act("$n prays to $s deity to protect $m from evil.", 1, ch, 0,
                0, TO_ROOM);
            cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (!IS_NPC(tch) && !number(0, 4)) {
                if (IS_SET(ch->specials.act, ACT_GREET) && 
                    GetMaxLevel(tch) > 5 && CAN_SEE(ch, tch)) {
                    if (GET_ALIGNMENT(tch) >= 900) {
                        sprintf(buf, "bow %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        if (tch->player.sex == SEX_FEMALE) {
                            command_interpreter(ch, "say Greetings, noble "
                                                    "lady!");
                        } else {
                            command_interpreter(ch, "say Greetings, noble "
                                                    "sir!");
                        }
                    } else if (GET_ALIGNMENT(tch) >= 350) {
                        sprintf(buf, "smile %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say Greetings, adventurer.");
                    } else if (GET_ALIGNMENT(tch) >= -350) {
                        sprintf(buf, "wink %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say You're doing well on "
                                                "your path of Neutrality");
                    } else if (GET_ALIGNMENT(tch) >= -750) {
                        sprintf(buf, "nod %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say May the prophet smile "
                                                "upon you");
                    } else if (GET_ALIGNMENT(tch) >= -900) {
                        sprintf(buf, "frown %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say You're falling in hands "
                                                "of evil, beware!");
                    } else {
                        /*
                         * hmm, not nice guy.. 
                         */
                        sprintf(buf, "glare %s", GET_NAME(tch));
                        command_interpreter(ch, buf);
                        command_interpreter(ch, "say I sense great evil here!");
                    }
                    SET_BIT(ch->specials.act, ACT_GREET);
                    break;
                }
            } else if (IsUndead(tch) || IsDiabolic(tch)) {
                command_interpreter(ch, "say Praise the Light, meet thine "
                                        "maker!");
                sprintf(buf, "warcry %s", GET_NAME(tch));
                command_interpreter(ch, buf);
            }
        }
    }
    return (FALSE);
}

/*
 * PSI_CAN is a shorthand for me, use ONLY for PSIs
 */
#define PSI_CAN(skill,level) (spell_index[(skill)] == -1 ? FALSE : \
                     spell_info[spell_index[(skill)]].min_level_psi <= (level))

/*
 * NOTAFF_N_LEARNED more shorthand, this one is general
 */
#define NOTAFF_N_LEARNED(ch,spell,skill) (!affected_by_spell((ch),(spell)) && \
                                          ((ch)->skills[(skill)].learned))

/*
 * SP(el)L_(and target's)N(a)ME :: puts spell name and target's name
 * together
 */
#define SPL_NME(tname,spell) sprintf( buf, "'%s' %s", spells[(spell)], (tname))

/*
 * (Psychic)C(rush)_OR_B(last) if psi can do a crush he does, if not
 * blast!
 */
#define C_OR_B(ch,vict) ((ch)->skills[SKILL_PSYCHIC_CRUSH].learned ? \
        (mind_psychic_crush(PML,(ch),(vict),NULL)) : \
        (do_blast((ch),(vict)->player.name,1)) )

/*
 * CAST_OR_BLAST checks to see if psi can cast special attack spell, if
 * not it will either do a psyhic crush or it will do a psionic blast 
 */
#define CAST_OR_BLAST(ch,vict,spell) \
    SPL_NME((vict)->player.name,(spell-1)); \
    ((ch)->skills[(spell)].learned ? do_cast((ch),buf,370) : \
     C_OR_B((ch),(vict)))

/*
 * Bugs: if 2 mobs of same name in room, may cause problems. fix add macro 
 * to add those "-" between parts and check for number of mobs with same
 * name in room and figure out which one you are!!! * add teleport, prob
 * travel, and disguise 
 */

int Psionist(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    int             PML;        /* psi mob's level */
    int             Qmana;      /* psi mob's 1/4 mana */
    int             cmana;      /* psi mob's current mana level */
    int             hpcan;      /* safe to canibalize/summon */
    char            buf[MAX_STRING_LENGTH];   /* all purpose buffer */
    struct char_data *targ;     /* who the psi_mob is fighting */
    int             group;      /* does targ have followers or is grouped? 
                                 */

    if (cmd) {
        return (FALSE);
    }
    if (!AWAKE(mob) && !affected_by_spell(mob, SKILL_MEDITATE)) {
        /*
         * no sleeping on the job!!!!
         * NOTE: this also prevents mobs who are incap from casting
         */
        command_interpreter(mob, "wake");
        command_interpreter(mob, "stand");
        return (TRUE);
    }

    if (!MobCastCheck(ch, 1)) {
        return (TRUE);
    }

    if (!IS_SET(mob->player.class, CLASS_PSI)) {
        SET_BIT(mob->player.class, CLASS_PSI);
        PML = GET_LEVEL(mob, PSI_LEVEL_IND) = GetMaxLevel(mob);
        SpaceForSkills(mob);

        /*
         * SetSkillLevels first defensive, general spells. 
         * I set all of these so that gods running quests can use the psi
         * mobs skills, unlike most other special procedure mobs
         */
        if (PSI_CAN(SKILL_CANIBALIZE, PML)) {
            mob->skills[SKILL_CANIBALIZE].learned =
                MIN(95, 10 + dice(10, (int) PML / 2));
        }

        if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML)) {
            mob->skills[SKILL_CELL_ADJUSTMENT].learned =
                MIN(95, 10 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_CHAMELEON, PML)) {
            mob->skills[SKILL_CHAMELEON].learned = MIN(95, 50 + number(2, PML));
        }

        if (PSI_CAN(SKILL_FLAME_SHROUD, PML)) {
            mob->skills[SKILL_FLAME_SHROUD].learned =
                MIN(95, 50 + dice(5, PML));
        }

        if (PSI_CAN(SKILL_GREAT_SIGHT, PML)) {
            mob->skills[SKILL_GREAT_SIGHT].learned = MIN(95, 33 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_INVIS, PML)) {
            mob->skills[SKILL_INVIS].learned = MIN(95, 50 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_MEDITATE, PML)) {
            mob->skills[SKILL_MEDITATE].learned =
                MIN(95, 33 + dice((int) PML / 5, 20));
        }

        if (PSI_CAN(SKILL_MINDBLANK, PML)) {
            mob->skills[SKILL_MINDBLANK].learned = MIN(95, 50 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_PORTAL, PML)) {
            mob->skills[SKILL_PORTAL].learned = MIN(95, 40 + dice(4, PML));
        }

        if (PSI_CAN(SKILL_PROBABILITY_TRAVEL, PML)) {
            mob->skills[SKILL_PROBABILITY_TRAVEL].learned =
                MIN(95, 35 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_PSI_SHIELD, PML)) {
            mob->skills[SKILL_PSI_SHIELD].learned =
                MIN(95, 66 + dice(10, (int) PML / 3));
        }

        if (PSI_CAN(SKILL_PSI_STRENGTH, PML)) {
            mob->skills[SKILL_PSI_STRENGTH].learned =
                MIN(95, dice((int) PML / 5, 20));
        }

        if (PSI_CAN(SKILL_PSYCHIC_IMPERSONATION, PML)) {
            mob->skills[SKILL_PSYCHIC_IMPERSONATION].learned =
                MIN(95, 66 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_SUMMON, PML)) {
            mob->skills[SKILL_SUMMON].learned = MIN(95, 66 + PML);
        }

        if (PSI_CAN(SKILL_TOWER_IRON_WILL, PML)) {
            mob->skills[SKILL_TOWER_IRON_WILL].learned =
                MIN(95, 50 + dice(4, PML));
        }

        /*
         * ATTACK -- TYPE SPELLS BEGIN HERE
         */
        if (PSI_CAN(SKILL_MIND_BURN, PML)) {
            mob->skills[SKILL_MIND_BURN].learned = MIN(95, 33 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_DISINTEGRATE, PML)) {
            mob->skills[SKILL_DISINTEGRATE].learned =
                MIN(95, 33 + dice(3, PML));
        }

        if (PSI_CAN(SKILL_MIND_WIPE, PML)) {
            mob->skills[SKILL_MIND_WIPE].learned = MIN(95, 45 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_PSIONIC_BLAST, PML)) {
            mob->skills[SKILL_PSIONIC_BLAST].learned =
                MIN(99, 33 + dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_PSYCHIC_CRUSH, PML)) {
            mob->skills[SKILL_PSYCHIC_CRUSH].learned =
                MIN(99, 66 + dice(2, PML));
        }

        if (PSI_CAN(SKILL_TELEKINESIS, PML)) {
            mob->skills[SKILL_TELEKINESIS].learned =
                MIN(95, dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_PSI_TELEPORT, PML)) {
            mob->skills[SKILL_PSI_TELEPORT].learned =
                MIN(95, dice((int) PML / 10, 30));
        }

        if (PSI_CAN(SKILL_ULTRA_BLAST, PML)) {
            mob->skills[SKILL_ULTRA_BLAST].learned =
                MIN(99, 66 + dice(3, PML));
        }

        mob->points.mana = 100;
    }

    /*
     * Aarcerak's little bug fix.. sitting mobs can't cast/mind/etc.. 
     */
    if (mob->specials.fighting && GET_POS(mob) < POSITION_FIGHTING && 
        GET_POS(mob) > POSITION_STUNNED) {
        StandUp(mob);
        return (TRUE);
    }

    if (affected_by_spell(mob, SPELL_FEEBLEMIND)) {
        if (!IS_AFFECTED(mob, AFF_HIDE)) {
            act("$n waits for $s death blow impatiently.", FALSE, mob, 0,
                0, TO_ROOM);
        }
        return (TRUE);
    }

    if (GET_POS(mob) == POSITION_SITTING ||
        GET_POS(mob) == POSITION_RESTING) {
        do_stand(mob, NULL, 0);
        return (TRUE);
    }

    PML = GET_LEVEL(mob, PSI_LEVEL_IND);

    if (NOTAFF_N_LEARNED(mob, SPELL_FIRESHIELD, SKILL_FLAME_SHROUD)) {
        command_interpreter(mob, "flame");
        return (TRUE);
    }

    if (affected_by_spell(mob, SKILL_MEDITATE)) {
        if (mob->points.mana <= .75 * mob->points.max_mana) {
            /* regaining mana */
            return (TRUE);
        }  else {
            command_interpreter(mob, "stand");
            return (TRUE);
        }
    }

    if (IS_AFFECTED(mob, AFF_HIDE)) {
        /* hiding, break fer flame shrd */
        return (FALSE);
    }

    Qmana = 51;
    hpcan = (int) (.75 * mob->points.max_hit);
    cmana = mob->points.mana;

    if (!mob->specials.fighting) {
        if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML) && cmana > Qmana &&
            mob->points.hit < hpcan) {
            command_interpreter(mob, "say That was too close for comfort.");
            mind_teleport(PML, mob, mob, NULL);
            mind_cell_adjustment(PML, mob, mob, NULL);
            return (TRUE);
        }

        if (cmana <= Qmana) {
            if (mob->points.hit > hpcan + 1 && 
                mob->skills[SKILL_CANIBALIZE].learned) {
                if (cmana + 2 * (mob->points.hit - hpcan) >=
                    mob->points.max_mana) {
                    /* Qmana=51>=cm, cm+(2*24) <= 99 */
                    sprintf(buf, "canibalize 24"); 
                } else {
                    sprintf(buf, "canibalize %d", 
                            (mob->points.hit - hpcan - 1));
                }
                command_interpreter(mob, buf);
            } else if (mob->skills[SKILL_MEDITATE].learned) {
                command_interpreter(mob, "meditate");
            }
            return (TRUE);
        }

        if (NOTAFF_N_LEARNED(mob, SKILL_PSI_SHIELD, SKILL_PSI_SHIELD)) {
            command_interpreter(mob, "shield");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_MINDBLANK, SKILL_MINDBLANK)) {
            command_interpreter(mob, "mind 'mindblank'");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_TOWER_IRON_WILL,
                                    SKILL_TOWER_IRON_WILL)) {
            command_interpreter(mob, "mind 'tower of iron will'");
        } else if (NOTAFF_N_LEARNED(mob, SPELL_SENSE_LIFE, SKILL_GREAT_SIGHT)) {
            command_interpreter(mob, "sight");
        } else if (NOTAFF_N_LEARNED(mob, SKILL_PSI_STRENGTH, 
                                    SKILL_PSI_STRENGTH)) {
            command_interpreter(mob, "mind 'psionic strength' self");
        } else if (IS_SET(mob->hatefield, HATE_CHAR) && 
                   mob->points.hit > hpcan) {
            command_interpreter(mob, "say It's payback time!");
            mob->points.mana = 100;
            if (PSI_CAN(SKILL_PORTAL, PML) || PSI_CAN(SKILL_SUMMON, PML)) {
                return (Summoner(mob, 0, NULL, mob, 0));
            }
        } else if (NOTAFF_N_LEARNED(mob, SPELL_INVISIBLE, SKILL_INVIS)) {
            command_interpreter(mob, "psionic");
        } else if (mob->skills[SKILL_CHAMELEON].learned) {
            command_interpreter(mob, "mind 'chameleon'");
        }
        return (TRUE);
    } else {
        /*
         * some psi combat spells still cost mana, set to max mana start of 
         * every round of combat 
         */
        mob->points.mana = 100; 
        targ = mob->specials.fighting;
        if (mob->points.max_hit - hpcan > 1.5 * mob->points.hit) {
            if (!mob->skills[SKILL_PSI_TELEPORT].learned || 
                !IsOnPmp(mob->in_room)) {
                act("$n looks around frantically.", 0, mob, 0, 0, TO_ROOM);
                command_interpreter(mob, "flee");
                return (TRUE);
            }

            act("$n screams defiantly, 'I'll get you yet, $N!'", 0, mob, 0,
                targ, TO_ROOM);
            command_interpreter(mob, "mind 'psionic teleport' self");
            return (TRUE);
        }

        group = (targ->followers || targ->master ? TRUE : FALSE);
        if (group && dice(1, 2) - 1) {
            group = FALSE;
        }

        if (!group) {
            /*
             * not fighting a group, or has selected person fighting, for 
             * spec 
             */
            if (dice(0, 1)) {
                /* do special attack 50% of time */
                if (IS_SET(targ->player.class, CLASS_MAGIC_USER | 
                                               CLASS_CLERIC)) {
                    if (dice(0, 1)) {
                        CAST_OR_BLAST(mob, targ, SKILL_TELEKINESIS);
                    } else {
                        CAST_OR_BLAST(mob, targ, SKILL_MIND_WIPE);
                    }
                } else if (GetMaxLevel(targ) < 20 && dice(0, 1)) {
                    CAST_OR_BLAST(mob, targ, SKILL_PSI_TELEPORT);
                } else {
                    CAST_OR_BLAST(mob, targ, SKILL_DISINTEGRATE);
                }
            } else {
                /* norm attack, psychic crush or psionic blast */
                C_OR_B(mob, targ);
            }
        } else if (mob->skills[SKILL_ULTRA_BLAST].learned) {
            mind_ultra_blast(PML, mob, targ, NULL);
        } else if (mob->skills[SKILL_MIND_BURN].learned) {
            mind_burn(PML, mob, targ, NULL);
        } else {
            sprintf(buf, "blast %s", GET_NAME(targ));
            command_interpreter(mob, buf);
        }
    }
    return (TRUE);
}




int AntiSunItem(struct char_data *ch, int cmd, char *arg,
                struct obj_data *obj, int type)
{
    if (type != PULSE_COMMAND) {
        return (FALSE);
    }
    if (OUTSIDE(ch) && weather_info.sunlight == SUN_LIGHT && 
        weather_info.sky <= SKY_CLOUDY && !IS_AFFECTED(ch, AFF_DARKNESS)) {
        /*
         * frag the item! 
         */
        act("The sun strikes $p, causing it to fall apart!", FALSE, ch,
            obj, 0, TO_CHAR);
        act("The sun strikes $p worn by $n, causing it to fall apart!",
            FALSE, ch, obj, 0, TO_ROOM);
        MakeScrap(ch, 0, obj);
        return (TRUE);
    }
    return (FALSE);
}



int fighter_cleric(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    if (number(1, 100) > 49) {
        return (fighter(ch, cmd, arg, mob, type));
    } else {
        return (cleric(ch, cmd, arg, mob, type));
    }
}

int cleric_mage(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (number(1, 100) > 49) {
        return (cleric(ch, cmd, arg, mob, type));
    } else {
        return (magic_user(ch, cmd, arg, mob, type));
    }
}

int Ranger(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct follow_type *fol;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if (check_soundproof(ch) || check_nomagic(ch, 0, 0)) {
        return (fighter(ch, cmd, arg, mob, type));
    }
    if (!ch->specials.fighting) {
        if (MobCastCheck(ch, 0)) {
            if (GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
                act("$n utters the words 'I feel good!'.", 1, ch, 0, 0,
                    TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
                return (TRUE);
            }

#ifdef PREP_SPELLS
            if (!ch->desc) {
                /* is it a mob? */
                if (!affected_by_spell(ch, SPELL_BARKSKIN)) {
                    act("$n utters the words 'oakey dokey'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_barkskin(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL) && 
                    !IS_EVIL(ch)) {
                    act("$n utters the words 'anti evil'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                              SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 19 &&
                    !affected_by_spell(ch, SPELL_GIANT_GROWTH)) {
                    act("$n utters the words 'The Blessings of Kane'.",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_giant_growth(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 29 &&
                    !affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                    OUTSIDE(ch) && !ch->followers && 
                    !IS_SET(real_roomp((ch)->in_room)->room_flags, TUNNEL)) {
                    /*
                     * let's give ranger some pets 
                     */
                    act("$n whistles loudly.", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_3(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);

                    if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1) && 
                        ch->followers) {
                        command_interpreter(ch, "order followers guard on");
                        command_interpreter(ch, "group all");
                        act("$n utters the words 'instant growth'.",
                            FALSE, ch, 0, 0, TO_ROOM);

                        for (fol = ch->followers; fol; fol = fol->next) {
                            if (!affected_by_spell(fol->follower, 
                                                   SPELL_ANIMAL_GROWTH)) {
                                cast_animal_growth(GetMaxLevel(ch), ch, "",
                                                   SPELL_TYPE_SPELL,
                                                   fol->follower, 0);
                                WAIT_STATE(ch, PULSE_VIOLENCE);
                            }
                        }
                    }
                    return (TRUE);
                }

                if (affected_by_spell(ch, SPELL_POISON)) {
                    act("$n utters the words 'remove poison'.", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_remove_poison(GetMaxLevel(ch), ch, "",
                                       SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) > 24 && 
                    affected_by_spell(ch, SPELL_CURSE)) {
                    act("$n utters the words 'neutralize'.", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_remove_curse(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GET_MOVE(ch) < GET_MAX_MOVE(ch) / 2) {
                    act("$n utters the words 'lemon aid'.", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
                    return (TRUE);
                }
            }
#endif
        }
    } else {
        return (fighter(ch, cmd, arg, mob, type));
    }
    return( TRUE );
}


/*
 *  From: sund_procs.c                          Part of Exile MUD
 *
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.
 *
 */



int marbles(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct obj_data *tobj = NULL;

    if (tobj->in_room == NOWHERE) {
        return FALSE;
    }
    if (cmd >= 1 && cmd <= 6 && GET_POS(ch) == POSITION_STANDING && 
        !IS_NPC(ch)) {
        if (number(1, 100) + GET_DEX(ch) > 50) {
            act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
            GET_POS(ch) = POSITION_SITTING;
            return TRUE;
        } else {
            act("You slip on $p, but manage to retain your balance.",
                FALSE, ch, tobj, 0, TO_CHAR);
            act("$n slips on $p, but manages to retain $s balance.", FALSE,
                ch, tobj, 0, TO_ROOM);
        }
    }
    return FALSE;
}





int bahamut_prayer(struct char_data *ch, struct char_data *vict)
{
    struct char_data *i;

    if (IS_PC(ch)) {
        return (0);
    }
    if (GET_HIT(ch) <= 2000) {
        if (ch->mult_att < 4) {
            ch->mult_att = 4;
        }

        for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
            GET_HIT(i) -= 25;
            send_to_char("$c0011A blinding holy light engulfs the room and" 
                         "sears your life away!\n\r", i);
            if (mob_index[i->nr].vnum == BAHAMUT) {
                GET_HIT(i) += 25;
            }
        }
        return (FALSE);
    }

    if (GET_HIT(ch) <= 3000) {
        if (ch->mult_att >= 4) {
            ch->mult_att = 1;
        } else if (ch->mult_att == 1) {
            act("$c0011$n bows down and prays to the dragon lord.", FALSE,
                ch, 0, 0, TO_ROOM);
        } else {
            ch->mult_att = 4;
        }

        return (FALSE);
    }

    return (FALSE);
}

int bahamut_armor(struct char_data *ch, struct char_data *vict)
{
    if (!ch || !vict) {
        return (FALSE);
    }

    switch (number(0, 30)) {
    case 1:
        act("$c0011A blinding beam of light bursts from you and sears your "
            "enemy's life away.", FALSE, ch, 0, 0, TO_CHAR);
        act("$c0011A blinding beam of light bursts from $n.", FALSE, ch, 0,
            0, TO_ROOM);
        GET_HIT(ch) += 15;
        GET_HIT(vict) -= 15;
        break;
    case 3:
    case 4:
    case 5:
        act("$c0011A bright light flickers around you briefly.", FALSE, ch,
            0, 0, TO_CHAR);
        act("$c0011A bright light flickers around $n briefly.", FALSE, ch,
            0, 0, TO_ROOM);
        break;
    default:
        break;
    }
    return (TRUE);
}





/*
 * Toy for Banon.. a new say for him (GH) April 2002 
 */
int godsay(struct char_data *ch, int cmd, char *argument,
           struct obj_data *obj, int type)
{
    char            buf[MAX_INPUT_LENGTH + 80];

    if (cmd != 17) {
        return (FALSE);
    }
    dlog("in godsay");

    if (apply_soundproof(ch)) {
        return ( FALSE );
    }

    argument = skip_spaces(argument);
    if (!argument) {
        send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
        return( TRUE );
    }

    sprintf(buf, "$c0012-=$c0015$n$c0012=-$c0011 says '$c0014%s$c0011'",
            argument);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);

    if (IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
        sprintf(buf, "$c0015You say '$c0014%s$c0015'\n\r", argument);
        send_to_char(buf, ch);
    }
    return( TRUE );
}




int guardian_sin(struct char_data *ch, struct char_data *vict)
{
    struct affected_type af;
    struct obj_data *obj;
    char            buf[240];
    int             gold;

    obj = ch->equipment[WIELD];

    /*
     * I'm thinking 1% chance for each spec to happen Try 0-8 to test
     * specs. may have to tweak the rate 
     */
    switch (number(0, 100)) {
    case 1:
        if (!SET_BIT(ch->specials.affected_by2, AFF2_BERSERK)) {
            /* berserk ch */
            /*
             * cool! let's berserk the wielder of the mace, regardless of
             * class. 
             */
            act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
                FALSE, ch, 0, 0, TO_ROOM);
            act("$c0008Roaring at $n, he invokes the Wrath of Raiva.$c0007", 1,
                vict, 0, 0, TO_ROOM);
            act("$c0008Roaring at you, he invokes the Wrath of Raiva.$c0007", 
                FALSE, vict, 0, 0, TO_VICT);
            SET_BIT(ch->specials.affected_by2, AFF2_BERSERK);
            act("$c1012$n growls at $mself, and whirls into a killing frenzy!",
                FALSE, ch, 0, vict, TO_ROOM);
            act("$c1012The madness overtakes you quickly!", FALSE, ch, 0,
                0, TO_CHAR);

            if (obj->short_description) {
                free(obj->short_description);
                sprintf(buf, "%s", "Guardian of Wrath");
                obj->short_description = strdup(buf);
            }
        }
        break;
    case 2:
#if 0        
        if (IS_NPC(ch))  {
            /* 
             * since slow doesn't really affect mobs, check 
             * here if wielder is PC 
             */
            return(FALSE);
        }
#endif        
        if (IsImmune(ch, IMM_HOLD)) {
            /* 
             * immune PCs shouldn't be affected. 
             */
            return (FALSE);
        }

        if (!affected_by_spell(ch, SPELL_SLOW)) {
            act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
                FALSE, ch, 0, 0, TO_ROOM);
            act("$c0008He giggles at $n while invoking the Leviathon of "
                "Laethargio.$c0007", 1, ch, 0, 0, TO_ROOM);
            act("$c0008He giggles at you while invoking the Leviathon of "
                "Laethargio.$c0007", FALSE, ch, 0, 0, TO_CHAR);

            af.type = SPELL_SLOW;
            af.duration = 2;
            af.modifier = 1;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_SLOW;
            affect_to_char(ch, &af);

            act("$c0008$n seems very slow.$c0007", 1, ch, 0, 0, TO_ROOM);
            send_to_char("$c0008You feel very slow!$c0007\r\n", ch);
            
            if (obj->short_description) {
                free(obj->short_description);
                sprintf(buf, "%s", "Guardian of Sloth");
                obj->short_description = strdup(buf);
            }
        }
        break;
    case 3:
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Pointing, he sends the Essence of Neid towards $n.$c0007",
            1, vict, 0, 0, TO_ROOM);
        act("$c0008Pointing, he sends the Essence of Neid towards you.$c0007",
            FALSE, vict, 0, 0, TO_VICT);
        SET_BIT(vict->specials.affected_by, AFF_BLIND);
        act("$c0008$n's eyes glaze over.$c0007", 1, vict, 0, 0, TO_ROOM);
        send_to_char("$c0008You blink, and the world has turned "
                     "dark.$c0007\r\n", vict);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Envy");
            obj->short_description = strdup(buf);
        }
        break;
    case 4:
        /*
         * This one is funky: dispel magic, dependant on wielder's level.
         * Good stuff, in most cases. However, dispelling fsd mobs when
         * you got your own fs running, this may prove scrappish. Cool! 
         */
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Looking disdainfully at $n, he chants to Lord Orgulho."
            "$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008Looking disdainfully at you, he chants to Lord Orgulho."
            "$c0007", FALSE, vict, 0, 0, TO_VICT);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Pride");
            obj->short_description = strdup(buf);
        }

        break;
    case 5:
        if (IS_PC(vict)) {
            /* 
             * don't steal from other players, not even in arena. 
             */
            return (FALSE);
        }
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008Cackling gleefully, he sends Ginayro through $n's "
            "pockets.$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008Your pockets seem a little lighter as Ginayro's Spirit "
            "visits them.$c0007", FALSE, vict, 0, 0, TO_VICT);

        /*
         * steal monies from vict, spoils go to ch 
         */
        gold = (int) ((GET_GOLD(vict) * number(1, 10)) / 100);
        gold = MIN(number(10000, 30000), gold);
        if (gold > 0) {
            GET_GOLD(ch) += gold;
            GET_GOLD(vict) -= gold;
            sprintf(buf, "$c0008Ginayro rewarded you with %d gold coins."
                         "$c0007\r\n", gold);
            send_to_char(buf, ch);
        } else {
            send_to_char("$c0008Ginayro couldn't grab you any gold.$c0007\r\n",
                         ch);
        }

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Greed");
            obj->short_description = strdup(buf);
        }
        break;
    case 6:
        if (affected_by_spell(ch, SKILL_ADRENALIZE)) {
            /* 
             * don't get multiple adrenalizes 
             */
            return (FALSE);
        }
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008He bestows the Spirit of Luhuria upon $n, who gets an "
            "excited look in his eye.$c0007", 1, ch, 0, 0, TO_ROOM);
        act("$c0008He bestows the Spirit of Luhuria upon you, setting your "
            "loins afire.$c0007", FALSE, ch, 0, 0, TO_CHAR);
        /*
         * set adrenalize 
         */
        af.type = SKILL_ADRENALIZE;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.duration = 7;
        af.bitvector = 0;
        affect_to_char(ch, &af);

        af.location = APPLY_DAMROLL;
        af.modifier = 4;
        affect_to_char(ch, &af);

        af.location = APPLY_AC;
        af.modifier = 20;
        affect_to_char(ch, &af);

        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Lust");
            obj->short_description = strdup(buf);
        }
        break;
    case 7:
        act("$c0008Your Guardian of Sin leaps up to defend you.$c0007",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$c0008The Guardian of Sin leaps up to defend $n.$c0007",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008An enormous maw takes a big bite out of $n, sating Hambre's"
            " appetite.$c0007", 1, vict, 0, 0, TO_ROOM);
        act("$c0008You've just become life bait for Hambre, who sinks his "
            "teeth deep into your flesh.$c0007", FALSE, vict, 0, 0, TO_VICT);
        /*
         * take big bite outta vict: 20% of current hp damage, that's
         * gotta hurt! 
         */
        GET_HIT(vict) -= (GET_HIT(vict) / 5);
        if (obj->short_description) {
            free(obj->short_description);
            sprintf(buf, "%s", "Guardian of Gluttony");
            obj->short_description = strdup(buf);
        }
        break;
    default:
        break;
    }
    return (FALSE);
}



int vampiric_embrace(struct char_data *ch, struct char_data *vict)
{
    struct obj_data *obj;
    int             dam;

    if (IsImmune(vict, IMM_DRAIN)) {
        return (FALSE);
    }
    if (ch->equipment[WIELD]) {
        obj = ch->equipment[WIELD];
        act("$c0008The negative aura surrounding your $p lashes out at $N, "
            "draining some of $S life.", FALSE, ch, obj, vict, TO_CHAR);
        act("$c0008The negative aura surrounding $n's $p lashes out at $N, "
            "draining some of $S life.", FALSE, ch, obj, vict, TO_NOTVICT);
        act("$c0008The negative aura surrounding $n's $p lashes out at you, "
            "draining some of your life.", FALSE, ch, obj, vict, TO_VICT);
    } else {
        act("$c0008The negative aura surrounding your hands lashes out at $N, "
            "draining some of $S life.", FALSE, ch, 0, vict, TO_CHAR);
        act("$c0008The negative aura surrounding $n's hands lashes out at $N, "
            "draining some of $S life.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("$c0008The negative aura surrounding $n's hands lashes out at you, "
            "draining some of your life.", FALSE, ch, 0, vict, TO_VICT);
    }
    dam = dice(3, 8);
    if (IsResist(vict, IMM_DRAIN)) {     
        /* 
         * half damage for resist 
         */
        dam >>= 1;
    }
    GET_HIT(ch) += dam;
    GET_HIT(vict) -= dam;
    return (FALSE);
}

/*
 * procs for the King's Grove 
 */
#define LEGEND_STATUE 52851
#define LEGEND_PAINTING 52852
#define LEGEND_BIOGRAPHY 52853
/**
 * @todo reimplement using MySQL to do the searching...
 */
int generate_legend_statue(void)
{
    struct obj_data *obj;
    struct char_data *tmp;
    struct char_file_u player;
    struct extra_descr_data *ed;
    char            name[254],
                    shdesc[254],
                    desc[254],
                    exdesc[500];
    int             i = 0,
                    itype = 0,
                    rnum = 0;
    extern int      top_of_p_table;
    extern struct player_index_element *player_table;

    /*
     * Determine number of pfiles. Add one for last player made, though
     * that one isn't very likely to have enough kills. Still, we wanna be 
     * thorough. 
     */
    for (i = 0; i < top_of_p_table + 1; i++) {
        /*
         * load up each of them 
         */
        if (load_char((player_table + i)->name, &player) > -1) {
            /*
             * store to a tmp char that we can deal with 
             */
            CREATE(tmp, struct char_data, 1);
            clear_char(tmp);
            store_to_char(&player, tmp);
            /*
             * are they entitled to an item? 
             */
            if (tmp->specials.m_kills >= 10000) {
                /*
                 * coolness, the are! Determine the item. 
                 */
                if (tmp->specials.m_kills >= 40000) {
                    itype = LEGEND_BIOGRAPHY;
                    rnum = number(52892, 52895);
                    sprintf(name, "biography tome %s", GET_NAME(tmp));
                    sprintf(shdesc, "a biography of %s", GET_NAME(tmp));
                    sprintf(desc, "A large tome lies here, titled "
                                  "'The Biography of %s'.", GET_NAME(tmp));
                    sprintf(exdesc, "This book is a treatise on the life and "
                                    "accomplishments of %s.\n\rIt is an "
                                    "extensive volume, detailing many a feat. "
                                    "Most impressive.", GET_NAME(tmp));
                } else if (tmp->specials.m_kills >= 20000) {
                    itype = LEGEND_PAINTING;
                    rnum = number(52886, 52891);
                    sprintf(name, "painting %s", GET_NAME(tmp));
                    sprintf(shdesc, "a painting of %s", GET_NAME(tmp));
                    sprintf(desc, "On the wall, one can admire a painting of "
                                  "%s, slaying a fearsome beast.", 
                                  GET_NAME(tmp));
                    sprintf(exdesc, "%s is in the process of slaying a fearsome"
                                    " beast.\n\rTruly, %s is one of the "
                                    "greatest of these times.",
                            GET_NAME(tmp), GET_NAME(tmp));
                } else {
                    itype = LEGEND_STATUE;
                    rnum = number(52861, 52884);
                    sprintf(name, "statue %s", GET_NAME(tmp));
                    sprintf(shdesc, "a statue of %s", GET_NAME(tmp));
                    sprintf(desc, "A statue of the legendary %s has been "
                                  "erected here.", GET_NAME(tmp));
                    sprintf(exdesc, "This is a statue of %s, the legendary "
                                    "slayer.", GET_NAME(tmp));
                }
                if (itype == 0) {
                    Log("Oddness in statue generation, no type found");
                    return (TRUE);
                }
                if (rnum == 0) {
                    Log("Oddness in statue generation, no rnum found");
                    return (TRUE);
                }

                /*
                 * load the generic item 
                 */
                if ((obj = objectRead(itype))) {
                    /*
                     * and string it up a bit 
                     */
                    if (obj->short_description) {
                        free(obj->short_description);
                        obj->short_description = strdup(shdesc);
                    }

                    if (obj->description) {
                        free(obj->description);
                        obj->description = strdup(desc);
                    }

                    FreeKeywords(&obj->keywords, FALSE);
                    StringToKeywords(name, &obj->keywords);

                    if (obj->ex_description) {
                        Log("trying to string invalid item in statue "
                            "generation");
                        return (TRUE);
                    } 
                    
                    /*
                     * create an extra desc structure for the object 
                     */
                    CREATE(obj->description, Keywords_t, 1);
                    StringToKeywords( name, obj->description );
                    obj->description->description = strdup(exdesc);

                    /*
                     * and finally place it in a room 
                     */
                    objectPutInRoom(obj, rnum);
                }
            }
            free(tmp);
        } else {
            Log("screw up bigtime in load_char");
            return (TRUE);
        }
    }

    Log("processed %d pfiles for legend statue check", top_of_p_table + 1);
    return( TRUE );
}





#define SHARPENING_STONE 52887
void do_sharpen(struct char_data *ch, char *argument, int cmd)
{
    struct obj_data *obj,
                   *cmp,
                   *stone;
    char            buf[254];
    char           *arg;
    int             w_type = 0;

    if (!ch || !cmd || cmd != 602) {
        /* 
         * sharpen 
         */
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("In the middle of a fight?! Hah.\n\r", ch);
        return;
    }

    if (ch->equipment && 
        (!(stone = ch->equipment[HOLD]) || 
         stone->item_number != SHARPENING_STONE)) {
        send_to_char("How can you sharpen stuff if you're not holding a "
                     "sharpening stone?\n\r", ch);
        return;
    }

    /*
     * is holding the stone 
     */
    if (!argument) {
        send_to_char("Sharpen what?\n\r", ch);
        return;
    }

    argument = get_argument(argument, &arg);
    if (!arg) {
        send_to_char("Sharpen what?\n\r", ch);
        return;
    }

    if ((obj = objectGetOnChar(ch, arg, ch))) {
        if ((ITEM_TYPE(obj) == ITEM_TYPE_WEAPON)) {
            /*
             * can only sharpen edged weapons 
             */
            switch (obj->value[3]) {
            case 0:
                w_type = TYPE_SMITE;
                break;
            case 1:
                w_type = TYPE_STAB;
                break;
            case 2:
                w_type = TYPE_WHIP;
                break;
            case 3:
                w_type = TYPE_SLASH;
                break;
            case 4:
                w_type = TYPE_SMASH;
                break;
            case 5:
                w_type = TYPE_CLEAVE;
                break;
            case 6:
                w_type = TYPE_CRUSH;
                break;
            case 7:
                w_type = TYPE_BLUDGEON;
                break;
            case 8:
                w_type = TYPE_CLAW;
                break;
            case 9:
                w_type = TYPE_BITE;
                break;
            case 10:
                w_type = TYPE_STING;
                break;
            case 11:
                w_type = TYPE_PIERCE;
                break;
            case 12:
                w_type = TYPE_BLAST;
                break;
            case 13:
                w_type = TYPE_IMPALE;
                break;
            case 14:
                w_type = TYPE_RANGE_WEAPON;
                break;
            default:
                w_type = TYPE_HIT;
                break;
            }

            if ((w_type >= TYPE_PIERCE && w_type <= TYPE_STING) || 
                (w_type >= TYPE_CLEAVE && w_type <= TYPE_STAB) || 
                w_type == TYPE_IMPALE) {

                if (obj->value[2] == 0) {
                    Log("%s tried to sharpen a weapon with invalid value: %s, "
                        "vnum %d.", GET_NAME(ch), obj->short_description,
                        obj->item_number);
                    return;
                }

                if (!(cmp = objectRead(obj->item_number))) {
                    Log("Could not load comparison weapon in do_sharpen");
                    return;
                }

                if (cmp->value[2] == 0) {
                    Log("%s tried to sharpen a weapon with invalid value: %s, "
                        "vnum %d.", GET_NAME(ch), obj->short_description,
                        obj->item_number);
                    objectExtract(cmp);
                    return;
                }

                if (cmp->value[2] == obj->value[2]) {
                    send_to_char("That item has no need of your attention.\n\r",
                                 ch);
                    objectExtract(cmp);
                    return;
                } 
                
                obj->value[2] = cmp->value[2];
                if (GET_POS(ch) > POSITION_RESTING) {
                    do_rest(ch, NULL, -1);
                }
                sprintf(buf, "%s diligently starts to sharpen %s.",
                        GET_NAME(ch), obj->short_description);
                act(buf, FALSE, ch, 0, 0, TO_ROOM);

                sprintf(buf, "You diligently sharpen %s.\n\r",
                        obj->short_description);
                send_to_char(buf, ch);

                objectExtract(cmp);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
            } else {
                send_to_char("You can only sharpen edged or pointy "
                             "weapons.\n\r", ch);
            }
        } else {
            send_to_char("You can only sharpen weapons.\n\r", ch);
        }
    } else {
        send_to_char("You don't seem to have that.\n\r", ch);
    }
}



/*
 * start shopkeeper .. this to make shops easier to build -Lennya 20030731
 */
int shopkeeper(struct char_data *ch, int cmd, char *arg,
               struct char_data *shopkeep, int type)
{
    struct room_data *rp;
    struct obj_data *obj = NULL,
                   *cond_ptr[50],
                   *store_obj = NULL;
    char           *itemname,
                    newarg[100];
    float           modifier = 1.0;
    int             cost = 0,
                    chr = 1,
                    k,
                    i = 0,
                    stop = 0,
                    num = 1,
                    rnum = 0;
    int             tot_cost = 0,
                    cond_top = 0,
                    cond_tot[50],
                    found = FALSE;

    extern struct str_app_type str_app[];

    if (!ch) {
        return (FALSE);
    }
    if (!AWAKE(ch) || IS_NPC(ch)) {
        return (FALSE);
    }
    /*
     * define the shopkeep 
     */
    if (ch->in_room) {
        rp = real_roomp(ch->in_room);
    } else {
        Log("weirdness in shopkeeper, char not in a room");
        return (FALSE);
    }

    if (!rp) {
        Log("weirdness in shopkeeper, char's room does not exist");
        return (FALSE);
    }

    /*
     * let's make sure shopkeepers don't get killed or robbed 
     */
    if (!IS_SET(rp->room_flags, PEACEFUL)) {
        SET_BIT(rp->room_flags, PEACEFUL);
    }
    if (cmd != 59 && cmd != 56 && cmd != 93 && cmd != 57) {
        /* 
         * list  buy  offer  sell 
         */
        return (FALSE);
    }
    shopkeep = FindMobInRoomWithFunction(ch->in_room, shopkeeper);

    if (!shopkeep) {
        Log("weirdness in shopkeeper, shopkeeper assigned but not found");
        return (FALSE);
    }

    if (!IS_NPC(shopkeep)) {
        Log("weirdness in shopkeeper, shopkeeper is not a mob");
        return (FALSE);
    }

    if (!IS_SET(shopkeep->specials.act, ACT_SENTINEL)) {
        SET_BIT(shopkeep->specials.act, ACT_SENTINEL);
    }
    /*
     * players with 14 chr pay avg price 
     */
    chr = GET_CHR(ch);
    if (chr < 1) {
        chr = 1;
    }
    modifier = (float) 14 / chr;

    /*
     * list 
     */
    switch (cmd) {
    case 59:
        oldSendOutput(ch, "This is what %s currently has on store:\n\r\n\r",
                  shopkeep->player.short_descr);
        send_to_char("  Count  Item                                       "
                     "Price\n\r", ch);
        send_to_char("$c0008*---------------------------------------------"
                     "------------*\n\r", ch);
        obj = shopkeep->carrying;
        if (!obj) {
            send_to_char("$c0008|$c0007        Nothing.                    "
                         "                     $c0008|\n\r", ch);
            send_to_char("$c0008*------------------------------------------"
                         "---------------*\n\r", ch);
            break;
        }
        
        for (obj = shopkeep->carrying; obj; obj = obj->next_content) {
            if (CAN_SEE_OBJ(ch, obj)) {
                if (cond_top < 50) {
                    found = FALSE;
                    for (k = 0; (k < cond_top && !found); k++) {
                        if (cond_top > 0 &&
                            obj->item_number == cond_ptr[k]->item_number &&
                            obj->short_description && 
                            cond_ptr[k]->short_description && 
                            !strcmp(obj->short_description, 
                                    cond_ptr[k]->short_description)) {
                            cond_tot[k] += 1;
                            found = TRUE;
                        }
                    }

                    if (!found) {
                        cond_ptr[cond_top] = obj;
                        cond_tot[cond_top] = 1;
                        cond_top += 1;
                    }
                } else {
                    cost = (int) obj->cost * modifier;
                    if (cost < 0) {
                        cost = 0;
                    }
                    cost += 1000;   /* Trader's fee = 1000 */
                    oldSendOutput(ch, "$c0008|$c0007    1   %-41s %6d "
                                  "$c0008|\n\r", obj->short_description,
                              cost);
                }
            }
        }

        if (cond_top) {
            for (k = 0; k < cond_top; k++) {
                cost = (int) cond_ptr[k]->cost * modifier;
                if (cost < 0) {
                    cost = 0;
                }
                cost += 1000;   /* Trader's fee = 1000 */
                oldSendOutput(ch, "$c0008|$c0007 %4d   %-41s %6d $c0008|\n\r",
                          (cond_tot[k] > 1 ?  cond_tot[k] : 1),
                          cond_ptr[k]->short_description, cost);
            }
        }
        send_to_char("$c0008*------------------------------------------------"
                     "---------*\n\r", ch);
        break;
    case 56:
        /*
         * buy 
         */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            send_to_char("Buy what?\n\r", ch);
            return (TRUE);
        } 
        
        if ((num = getabunch(itemname, newarg)) != FALSE) {
            strcpy(itemname, newarg);
        }

        if (num < 1) {
            num = 1;
        }
        rnum = 0;
        stop = 0;
        i = 1;

        while (i <= num && stop == 0) {
            if ((obj = objectGetOnChar(ch, itemname, shopkeep))) {
                cost = (int) obj->cost * modifier;
                if (cost < 0) {
                    cost = 0;
                }
                cost += 1000;       /* Trader's Fee is 1000 */

                if (GET_GOLD(ch) < cost) {
                    send_to_char("Alas, you cannot afford that.\n\r", ch);
                    stop = 1;
                } else if ((IS_CARRYING_N(ch) + 1) > (CAN_CARRY_N(ch))) {
                    oldSendOutput(ch, "%s : You can't carry that many items.\n\r",
                              obj->short_description);
                    stop = 1;
                } else
                    if ((IS_CARRYING_W(ch) + (obj->weight)) >
                        CAN_CARRY_W(ch)) {
                    oldSendOutput(ch, "%s : You can't carry that much weight.\n\r",
                              obj->short_description);
                    stop = 1;
                } else if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) != 0 &&
                           anti_barbarian_stuff(obj) && 
                           !IS_IMMORTAL(ch)) {
                    send_to_char("You sense magic on the object and think "
                                 "better of buying it.\n\r", ch);
                    stop = 1;
                } else {
                    objectTakeFromChar(obj);
                    objectGiveToChar(obj, ch);
                    GET_GOLD(ch) -= cost;
                    GET_GOLD(shopkeep) += cost;
                    store_obj = obj;
                    i++;
                    tot_cost += cost;
                    rnum++;
                }
            } else if (rnum > 0) {
                oldSendOutput(ch, "Alas, %s only seems to have %d %ss on "
                              "store.\n\r",
                          shopkeep->player.short_descr, rnum, itemname);
                stop = 1;
            } else {
                oldSendOutput(ch, "Alas, %s doesn't seem to stock any %ss..\n\r",
                          shopkeep->player.short_descr, itemname);
                stop = 1;

            }
        }

        if (rnum == 1) {
            oldSendOutput(ch, "You just bought %s for %d coins.\n\r",
                      store_obj->short_description, cost);
            act("$n buys $p from $N.", FALSE, ch, obj, shopkeep, TO_ROOM);
        } else if (rnum > 1) {
            oldSendOutput(ch, "You just bought %d items for %d coins.\n\r",
                      rnum, tot_cost);
            act("$n buys some stuff from $N.", FALSE, ch, obj, shopkeep,
                TO_ROOM);
        }
        break;
    case 57:
    /*
     * sell 
     */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            send_to_char("Sell what?\n\r", ch);
            return (TRUE);
        }
        
        if ((obj = objectGetOnChar(ch, itemname, ch))) {
            cost = (int) obj->cost / (3 * modifier);
            /*
             * lets not have shops buying non-rentables
             */
            if (obj->cost_per_day == -1) {
                oldSendOutput(ch, "%s doesn't buy items that cannot be "
                                  "rented.\n\r", shopkeep->player.short_descr);
                return (TRUE);
            }

            if (cost < 400) {
                oldSendOutput(ch, "%s doesn't buy worthless junk like that.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }
            
            if (GET_GOLD(shopkeep) < cost) {
                oldSendOutput(ch, "Alas, %s cannot afford that right now.\n\r",
                          shopkeep->player.short_descr);
                return (TRUE);
            }
            
            objectTakeFromChar(obj);
            objectGiveToChar(obj, shopkeep);
            oldSendOutput(ch, "You just sold %s for %d coins.\n\r",
                      obj->short_description, cost);
            act("$n sells $p to $N.", FALSE, ch, obj, shopkeep, TO_ROOM);
            GET_GOLD(ch) += cost;
            GET_GOLD(shopkeep) -= cost;
            return (TRUE);
        }
        
        oldSendOutput(ch, "Alas, you don't seem to have the %s to sell.\n\r",
                  itemname);
        break;
    case 93:
    /*
     * offer 
     */
        arg = get_argument(arg, &itemname);
        if (!itemname) {
            oldSendOutput(ch, "What would you like to offer to %s?\n\r",
                      shopkeep->player.short_descr);
            return (TRUE);
        }
        
        if ((obj = objectGetOnChar(ch, itemname, ch))) {
            cost = (int) obj->cost / (3 * modifier);
            if (cost < 400) {
                oldSendOutput(ch, "%s doesn't buy worthless junk like "
                                  "that.\n\r", shopkeep->player.short_descr);
                return (TRUE);
            }
            
            oldSendOutput(ch, "%s is willing to pay you %d coins for %s.\n\r",
                          shopkeep->player.short_descr, cost,
                          obj->short_description);
            return (TRUE);
        }
        
        oldSendOutput(ch, "You don't seem to have any %ss.\n\r", itemname);
        break;
    default:
        return (FALSE);
    }
    return (TRUE);
}





int troll_regen(struct char_data *ch)
{
    assert(ch);

    if (GET_HIT(ch) >= GET_MAX_HIT(ch)) {
        return (FALSE);
    }
    if (number(0, 2)) {
        return( FALSE );
    }
    GET_HIT(ch) += number(1, 3);
    if (GET_HIT(ch) > GET_MAX_HIT(ch)) {
        GET_HIT(ch) = GET_MAX_HIT(ch);
    }
    act("$n's wounds seem to close of their own.", FALSE, ch, 0, 0, TO_ROOM);
    act("Your wounds close of their own accord.", FALSE, ch, 0, 0, TO_CHAR);
    return( TRUE );
}

int disembark_ship(struct char_data *ch, int cmd, char *argument,
                   struct obj_data *obj, int type)
{
    int             x = 0;

    if (cmd == 621) {
        /* 
         * disembark 
         */

        if (oceanmap[GET_XCOORD(obj)][GET_YCOORD(obj)] == '@') {
            if (ch->specials.fighting) {
                send_to_char("You can't while your fighting!", ch);
                return (TRUE);
            }

            /*
             * lets find out where they are going 
             */
            while (map_coords[x].x != -1) {
                if (map_coords[x].x == GET_XCOORD(obj) &&
                    map_coords[x].x == GET_YCOORD(obj)) {
                    char_from_room(ch);
                    char_to_room(ch, map_coords[x].room);
                    do_look(ch, NULL, 0);
                    return (TRUE);
                }

                x++;
            }
            send_to_char("Ahh.. maybe we don't have a place to disembark!\n\r",
                         ch);
        } else {
            send_to_char("There is no place around to disembark your ship!\n\r",
                         ch);
        }
        return (TRUE);
    }
    return (FALSE);
}

int steer_ship(struct char_data *ch, int cmd, char *argument,
               struct obj_data *obj, int type)
{
    static char    *keywords[] = {
        "north",
        "east",
        "south",
        "west",
        "up",
        "down"
    };
    int             keyword_no = 0;
    char           *buf;

    argument = get_argument(argument, &buf);
    if (!buf) {
        /* 
         * No arguments?? so which direction anyway? 
         */
        send_to_char("Sail in which direction?", ch);
        return (TRUE);
    }

    keyword_no = search_block(buf, keywords, FALSE);

    if ((keyword_no == -1)) {
        send_to_char("Sail in which direction?", ch);
        return (TRUE);
    }

    if (!CanSail(obj, keyword_no)) {
        send_to_char("You can't sail that way, you'd be bound to sink the "
                     "ship!!\n\r", ch);
        return (TRUE);
    }

    oldSendOutput(ch, "You sail %sward.\n\r", keywords[keyword_no]);

    switch (keyword_no) {
    case 0:
        /* 
         * North
         */
        GET_XCOORD(obj)--;
        break;
    case 1:
        /*
         * East
         */
        GET_YCOORD(obj)++;
        break;
    case 2:
        /*
         * south
         */
        GET_XCOORD(obj)++;
        break;
    case 3:
        /* 
         * west
         */
        GET_YCOORD(obj)--;
        break;
    default:
        break;
    }

    printmap(ch, GET_XCOORD(obj), GET_YCOORD(obj), 5, 10);
    return (TRUE);
}


/*
 * can they sail in that directioN?? 
 */
int CanSail(struct obj_data *obj, int direction)
{
    int             x = 0,
                    y = 0;

    x = GET_XCOORD(obj);
    y = GET_YCOORD(obj);

    switch (direction) {
    case 0:
        /* 
         * North
         */
        x--;
        break;
    case 1:
        /* 
         * East
         */
        y++;
        break;
    case 2:
        /* 
         * south
         */
        x++;
        break;
    case 3:
        /* 
         * west
         */
        y--;
        break;
    default:
        break;
    }

    if (oceanmap[x][y] == '~') {
        return (TRUE);
    } else if (oceanmap[x][y] == '@') {
        /* 
         * Entered port city.. lets place a ship in the board city
         */
        return (TRUE);
    } else if (oceanmap[x][y] == ':') {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/*
 * lets print the map to the screen 
 */
void printmap(struct char_data *ch, int x, int y, int sizex, int sizey)
{
    int             loop = 0;
    void            printColors(struct char_data *ch, char *buf);
    char            buf[256];
    char            buf2[256];
#if 0
    printf("Displaying map at coord X%d-Y%d with display size of "
           "%d by %d.\n\r\n\r",x,y,sizex, sizey);
#endif
    oldSendOutput(ch, "Coords: %d-%d.\n\r", x, y);
    sprintf(buf, "\n\r$c000B]$c000W");

    for (loop = 0; loop < sizey * 2 + 1; loop++) {
        sprintf(buf, "%s=", buf);
    }

    sprintf(buf, "%s$c000B[$c000w\n\r", buf);
    send_to_char(buf, ch);

    for (loop = 0; loop < sizex * 2 + 1; loop++) {
        /* 
         * move that row of the ocean map into buf 
         */
        sprintf(buf, "%s", oceanmap[x - sizex + loop]);

        if (loop == sizex) {
            buf[y] = 'X';
        }
        /* 
         * mark of the end of where they should see 
         */
        buf[y + sizey + 1] = '\0';
        /* 
         * move to the start of where they should see on that row 
         * Print that mofo out 
         */
        sprintf(buf2, "|%s|\n\r", &buf[y - sizey]);
#if 0
        send_to_char(buf,ch);
        printmapcolors(buf);
#endif
        printColors(ch, buf2);
    }

    sprintf(buf, "$c000B]$c000W");
    for (loop = 0; loop < sizey * 2 + 1; loop++) {
        sprintf(buf, "%s=", buf);
    }

    sprintf(buf, "%s$c000B[$c000w", buf);
    send_to_char(buf, ch);
#if 0
    printColors(ch, buf);
#endif
}

/*
 * Lets go through and see what terrain needs what color 
 */
void printColors(struct char_data *ch, char *buf)
{
    int             x = 0;
#if 0
    int last=0;
#endif
    char            buffer[2048];
    char            last = ' ';

    buffer[0] = '\0';

    while (buf[x] != '\0') {
        switch (buf[x]) {
        case '~':
            if (last == '~') {
                strcat(buffer, "~");
            } else {
                strcat(buffer, "$c000b~");
                last = '~';
            }
            break;
        case '+':
            if (last == '+') {
                strcat(buffer, "+");
            } else {
                strcat(buffer, "$c000G+");
                last = '+';
            }
            break;
        case '^':
            if (last == '^') {
                strcat(buffer, "^");
            } else {
                strcat(buffer, "$c000Y^");
                last = '^';
            }
            break;
        case '.':
            if (last == '.') {
                strcat(buffer, ".");
            } else {
                strcat(buffer, "$c000y.");
                last = '.';
            }
            break;
        default:
            if (last == buf[x]) {
                sprintf(buffer, "%s%c", buffer, buf[x]);
            } else {
                sprintf(buffer, "%s$c000w%c", buffer, buf[x]);
                last = buf[x];
            }
            break;
        }
        x++;
    }

    send_to_char(buffer, ch);
}

int embark_ship(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             j;
    char           *buf;
    struct char_data *ship;

    if (cmd != 620) {
        /* 
         * board ship 
         */
        return (FALSE);  
    }

    arg = get_argument(arg, &buf);
    if (buf && !strcasecmp("ship", buf) &&
        (ship = get_char_room("", ch->in_room))) {
        j = mob_index[ship->nr].vnum;

        send_to_char("You enter the ship.\n\r", ch);
        act("$n enters the ship.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, j);

        act("Walks onto the ship.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }

    return (FALSE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
