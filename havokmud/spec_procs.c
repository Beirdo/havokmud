#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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


/*************************************/
/*
 * predicates for find_path function
 */

int is_target_room_p(int room, void *tgt_room)
{
    /* AMD64: Fix me */
    return( room == (int)(long) tgt_room );
}

int named_object_on_ground(int room, void *c_data)
{
    char           *name = c_data;
    return( 0 != get_obj_in_list(name, real_roomp(room)->contents) );
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
            *thing = get_obj_in_list_vis(npc, arg,
                                         real_roomp(npc->in_room)->contents);
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

        temp1 = get_obj_in_list_vis(zmaster, "corpse",
                                    real_roomp(zmaster->in_room)->contents);

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
    static int      chl = -1,
                    achl = -1;

    /*
     * 222 is the normal chalice, 223 is chalice-on-altar
     */

    if (chl < 1) {
        chl = real_object(222);
        achl = real_object(223);
    }

    switch (cmd) {
    case 10:
        /*
         * get
         */
        if (!(chalice =
                get_obj_in_list_num(chl, real_roomp(ch->in_room)-> contents)) &&
            CAN_SEE_OBJ(ch, chalice)) {
            if (!(chalice =
                    get_obj_in_list_num(achl,
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
            extract_obj(chalice);
            chalice = read_object(chl, VIRTUAL);
            obj_to_char(chalice, ch);
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
            extract_obj(chalice);
            chalice = read_object(achl, VIRTUAL);
            obj_to_room(chalice, ch->in_room);
            send_to_char("Ok.\n\r", ch);
        }
        return (TRUE);
        break;
    case 176:
        /*
         * pray
         */
        if (!(chalice =
                get_obj_in_list_num(achl, real_roomp(ch->in_room)->contents))) {
            return (FALSE);
        }

        do_action(ch, arg, cmd);
        /*
         * pray
         */
        send_to_char(CHAL_ACT, ch);
        extract_obj(chalice);
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
                    save_room;
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

    save_obj(ch, &cost, 1);
    save_room = ch->in_room;

    if (ch->specials.start_room != 2) {
        ch->specials.start_room = save_room;
    }
    /*
     * CHARACTERS aren't freed by this
     */
    extract_char(ch);
    save_char(ch, save_room);
    ch->in_room = save_room;
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
            o = read_object(3012, VIRTUAL);
            obj_to_char(o, ch);
        } else {
            o = read_object(3013, VIRTUAL);
            obj_to_char(o, ch);
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




void ThrowChar(struct char_data *ch, struct char_data *v, int dir)
{
    struct room_data *rp;
    int             or;
    char            buf[200];

    rp = real_roomp(v->in_room);
    if (rp && rp->dir_option[dir] && rp->dir_option[dir]->to_room &&
        EXIT(v, dir)->to_room != NOWHERE) {
        if (v->specials.fighting) {
            stop_fighting(v);
        }

        sprintf(buf, "%s picks you up and throws you %s\n\r",
                ch->player.short_descr, direction[dir].dir);
        send_to_char(buf, v);

        or = v->in_room;
        char_from_room(v);
        char_to_room(v, (real_roomp(or))->dir_option[dir]->to_room);
        do_look(v, NULL, 15);

        if (IS_SET(RM_FLAGS(v->in_room), DEATH) && !IS_IMMORTAL(v)) {
            NailThisSucker(v);
        }
    }
}






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
                        obj = read_object(PostKey, VIRTUAL);
                        if ((IS_CARRYING_N(t) + 1) < CAN_CARRY_N(t)) {
                            act("$N emerges with $p, and gives it to you.",
                                FALSE, t, obj, ch, TO_CHAR);
                            act("$n emerges with $p, and gives it to $N.",
                                FALSE, ch, obj, t, TO_ROOM);
                            obj_to_char(obj, t);
                        } else {
                            act("$n emerges with $p, and drops it for $N.",
                                FALSE, ch, obj, t, TO_ROOM);
                            obj_to_room(obj, ch->in_room);
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
        if (!obj_name ||
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
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

            switch (obj->obj_flags.type_flag) {

            case ITEM_FOOD:
                if (obj->obj_flags.value[3]) {
                    act("$n sniffs $p, then discards it with disgust.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    obj_from_char(obj);
                    obj_to_room(obj, ch->in_room);
                    if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                        mem->index = affect_status(mem, latt, ch, -5);
                    } else {
                        return (TRUE);
                    }
                } else {
                    act("$n takes $p and hungrily wolfs it down.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    extract_obj(obj);
                    if (!IS_MOB(ch) && CAN_SEE(latt, ch)) {
                        mem->index = affect_status(mem, latt, ch, 4);
                    } else {
                        return (TRUE);
                    }
                }
                break;
            case ITEM_KEY:
                /*
                 * What he really wants
                 */
                if (obj_index[obj->item_number].virtual == CrowBar) {
                    act("$n takes $p and jumps up and down in joy.",
                        TRUE, latt, obj, 0, TO_ROOM);
                    obj_from_char(obj);
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
        if (!obj_name ||
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
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

            if (obj_index[obj->item_number].virtual == Necklace) {
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
                obj_from_char(obj);
                extract_obj(obj);

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
                if (real_roomp(ch->in_room)
                    && (EXIT(ch, 2)->to_room != NOWHERE)) {
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
            ++j;
        }
        return (FALSE);
    }
    return (FALSE);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
